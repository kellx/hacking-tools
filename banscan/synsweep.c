#include <banscan.h>

/*
 * the finish flag 
 */

int finish_flag = 0;

/*
 * function : synsweep()
 * purpose  : scan a list of ip's for an open port
 * arguments: list of ip's and port to scan 
 * returns  : list of ip's with open port, NULL on error
 */

list_t *synsweep(list_t *list, int port, char *iface)
{
  int i;			/* PROG: general purpose counter         */
  int pid;			/* PROG: pid of child process            */
  int total;			/* PROG: total number of hosts           */
  int percent;  		/* PROG: percentage of scanning complete */
  char *ip_addr;		/* PROG: ip address to send syns to      */
  list_t *open_list;		/* PROG/OUT: list of ip's with open port */

  /*
   * initialize linked list and IPC
   */

  open_list = (list_t *) malloc(sizeof(list_t));
  list_init(open_list);

  signal(SIGALRM, sighandler);

  /*
   * fork() sendsyn into background & process responses
   */

  switch((pid = fork())) {
    case -1:
      return(NULL);
    case 0:
      sleep(1);

      total = list->list_size;

      for(i = 1; list->list_size > 0; i++) {
        usleep(100);

        list_remove((void **)&ip_addr, list);
        sendsyn(ip_addr, src_ip, port);

        percent = 100 * i / total;

        printf("  (%d%%) %d/%d completed\r", percent, i, total);
      }

      printf("\r\n");

      for(i = 10; i >= 0; i--) {
        printf("  %d second(s) remaining\r", i);
        sleep(1);
      }

      printf("\r\n");

      kill(getppid(), SIGALRM);

      exit(0);
    default:
      if(recvpkts(open_list, iface) < 0) {
        kill(pid, SIGKILL);
        return(NULL);
      }
      break;
  }

  wait(NULL);

  alarm(0);
  return(open_list);
}

/* 
 * function : sendsyn()
 * purpose  : construct a raw tcp syn packet and send it
 * arguments: ip address, src ip address & port
 * returns  : void
 */

void sendsyn(char *dst_ip, char *src_ip, int dst_port)
{
  int one;			/* PROG: for setsockopt()        */
  int sockfd;			/* PROG: raw socket descriptor   */
  u_char *pkt;			/* PROG: constructed packet      */
  struct ip iphdr;		/* PROG: constructed ip header   */
  struct tcphdr tcphdr;		/* PROG: constructed tcp header  */
  struct sockaddr_in sin;	/* PROG: socket structure        */

  /*
   * initialize raw socket descriptor 
   */

  if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) < 0)
     return;

  if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, (void *)&one, sizeof(one)) < 0)
    return;

  /*
   * initialize packet
   */

  memset(&iphdr, 0, sizeof(iphdr));
  memset(&tcphdr, 0, sizeof(tcphdr));

  pkt = (u_char *) malloc(sizeof(tcphdr) + sizeof(iphdr));

  /*
   * begin construction of packet, starting with ip header
   */

  iphdr.ip_v = 4;				/* ip version 4              */
  iphdr.ip_hl = 5;				/* ip header len: 5 * 4 = 20 */
  iphdr.ip_tos = 0;				/* ip type of service        */
  iphdr.ip_len = htons(40);			/* total length of ip pkt    */
  iphdr.ip_id = rand() % 65535;                 /* random ip identifier      */
  iphdr.ip_off = 0;                             /* data offset               */
  iphdr.ip_ttl = 64;                            /* ip time to live value     */
  iphdr.ip_p = IPPROTO_TCP;                     /* specify tcp packet        */
  iphdr.ip_sum = 0;                             /* zero out checksum for now */
  iphdr.ip_src.s_addr = inet_addr(src_ip);	/* source ip address         */
  iphdr.ip_dst.s_addr = inet_addr(dst_ip);	/* destination ip address    */

  /*
   * calculate checksum for the ip header
   */

  iphdr.ip_sum = in_cksum((u_short *)&iphdr, sizeof(iphdr));

  /*
   * construct the tcp header 
   */

  tcphdr.th_sport = htons(src_port);		/* tcp source port        */
  tcphdr.th_dport = htons(dst_port);		/* tcp destination port   */
  tcphdr.th_seq = (rand() * getpid());		/* random sequence number */ 
  tcphdr.th_ack = htons(0);			/* this is not an ack pkt */
  tcphdr.th_flags = TH_SYN;			/* specify tcp syn        */
  tcphdr.th_x2 = 0;				/* unused		  */
  tcphdr.th_off = 5;				/* data offset		  */
  tcphdr.th_win = htons(512);			/* tcp advertised window  */
  tcphdr.th_sum = 0;				/* zero checksum for now  */
  tcphdr.th_urp = 0;				/* no need for urgent     */

  /*
   * calculate checksum for tcp header
   */

  tcphdr.th_sum = tcp_cksum((char *)&tcphdr, sizeof(tcphdr), 
    iphdr.ip_src, iphdr.ip_dst);

  /*
   * copy in ip and tcp headers 
   */

  memcpy(pkt, &iphdr, sizeof(iphdr));
  memcpy(pkt + sizeof(iphdr), &tcphdr, sizeof(tcphdr));

  /*
   * initialize socket structure
   */

  sin.sin_family = AF_INET;
  sin.sin_port = htons(dst_port);
  sin.sin_addr.s_addr = inet_addr(dst_ip);

  /*
   * send packet to the network
   */

  sendto(sockfd, pkt, 40, 0, (struct sockaddr *)&sin, sizeof(sin));

  /* 
   * free memory and return
   */

  free(pkt);
  close(sockfd);
}

/*
 * function : in_cksum()
 * purpose  : perform checksum on packet header
 * arguments: ptr to ip header, length of ip header
 * returns  : computed checksum
 */

u_short in_cksum(u_short *addr, int len)
{
  int nleft = len;		/* PROG: number of bytes left to compute  */
  int sum = 0;			/* PROG: checksum to compute              */
  u_short *w = addr;		/* PROG: pointer to protocol header       */

  u_short answer = 0;           /* OUT: checksum to return                */

  while (nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }

  if (nleft == 1) {
    *(unsigned char *) (&answer) = *(unsigned char *) w;
    sum += answer;
  }
  sum = (sum >> 16) + (sum & 0xffff);   /* add hi 16 to low 16 */
  sum += (sum >> 16);           /* add carry */
  answer = ~sum;                /* truncate to 16 bits */
  return (answer);
}

/*
 * function : tcp_cksum()
 * purpose  : perform checksum on tcp packet header
 * arguments: ptr to tcp header, length of tcp header, src ip & dest ip
 * returns  : computed checksum
 */

u_short tcp_cksum(char *packet, int len, struct in_addr src_addr,
  struct in_addr dst_addr)
{
  char *temppkt;		/* PROG: pointer to temporary packet */
  u_short cksum;		/* PROG: computed checksum           */

  /*
   * definition of temp packet to checksum
   */

  struct temphdr {
    struct in_addr src;
    struct in_addr dst;
    unsigned char zero;
    unsigned char pr;
    u_short len;
  } temphdr;

  temphdr.pr = IPPROTO_TCP;
  temphdr.len = htons(len);
  temphdr.zero = 0;

  temphdr.src = src_addr;
  temphdr.dst = dst_addr;

  if((temppkt = malloc(sizeof(temphdr) + len)) == NULL) {
    perror("cannot allocate memory\n");
    exit(-1);
  }

  memcpy(temppkt, &temphdr, sizeof(struct temphdr));
  memcpy((temppkt + sizeof(temphdr)), packet, len);

  cksum = in_cksum((u_short *)temppkt, (len + sizeof(temphdr)));
  free(temppkt);
  return(cksum);
}

/*
 * function : recvpkts()
 * purpose  : recieve packets, find open ports and place in list
 * arguments: pointer to list
 * returns  : 0 on success, -1 on error
 */

int recvpkts(list_t *list, char *iface)
{
  u_char *pkt;				/* PROG: pointer to recieved packet */
  u_char *ptr;				/* PROG: general purpose pointer    */
  u_char *ip_addr;			/* PROG: responding ip address      */
  pcap_t *pd;				/* PROG: pcap descriptor            */
  char errbuf[1024];			/* PROG: errbuf for libpcap         */
  struct pcap_pkthdr pcap_h;		/* PROG: pcap packet header         */
  struct ip *iphdr;			/* PROG: parsed ip header	    */
  struct tcphdr *tcphdr;		/* PROG: parsed tcp header          */

  /*
   * open network interface
   */

  if(!(pd = pcap_open_live(iface, 1024, 0, 1, errbuf)))
    return(-1);

  /*
   * loop and gather packets
   */

  for(;;) {
    ptr = ip_addr = NULL;

    if(finish_flag)
      break;

    if(!(pkt = (u_char *)pcap_next(pd, &pcap_h)))
      continue;

    if((pcap_h.caplen - 14) < 40)
      continue;

    iphdr = (struct ip *)(pkt + 14);
    tcphdr = (struct tcphdr *)(pkt + 14 + 20);

    if(tcphdr->th_flags != (TH_SYN|TH_ACK))
      continue;

    if(ntohs(tcphdr->th_dport) != src_port)
      continue;

    ptr = inet_ntoa(iphdr->ip_src);

    ip_addr = (u_char *) malloc(strlen(ptr) + 1);
    memset(ip_addr, 0, strlen(ptr) + 1);
    memcpy(ip_addr, ptr, strlen(ptr));

    list_insert(ip_addr, list);
  }

  pcap_close(pd);    
  return(0);
}

void sighandler(int sig)
{
  finish_flag = 1;
}
