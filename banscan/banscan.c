/*
 * banscan - a fast banner enumeration program
 * written by bind <bind@insidiae.org>
 *
 * usage: ./banscan [options] [-f <infile>] [-c <addr>] [-b <addr>] [-p <port>]
 *        -f       file containing list of ip's
 *        -c       class c ip range
 *        -b       class b ip range
 *        -p       banner enumeration port
 *        -s       source ip to perform scan from
 *        -i       default packet capture interface
 *        -o       output file for banner enumeration
 *        -P       perform portscan only
 *
 * copyright (c) 2002 bind@insidiae.org
 * http://www.insidiae.org/~bind
 */
 
#include <banscan.h>

FILE *output;
char *src_ip;
int src_port;

int main(int argc, char **argv)
{
  int port;			/* IN: port to get banner on          */
  int pscan_only;		/* IN: portscan only?		      */
  char *class_c;		/* IN: class c ip address range       */
  char *class_b;		/* IN: class b ip address range       */
  char *ip_file;		/* IN: file to read IP addresses from */
  char *out_file;		/* IN: file to output to              */
  char *nudge;			/* IN: nudge string                   */
  char *iface;			/* IN: network interface              */
  
  int n;			/* PROG: general return value         */	
  int opt;			/* PROG: getopt option character      */
  struct list_t ip_list;	/* PROG: linked list of ip's          */
  struct list_t *open_list;	/* PROG: ip's with open ports         */

  /* 
   * initialize banscan
   */

  port = pscan_only = 0;
  src_ip = ip_file = class_c = class_b = out_file = nudge = NULL;
  output = NULL;
  iface = "eth0";

  srand(time(NULL));
  src_port = rand() % 65535;

  list_init(&ip_list);

  setvbuf(stdout, NULL, _IONBF, 0);

  /*
   * display application banner 
   */

  printf("banscan - a fast banner enumeration program\n"
         "written by bind <bind@insidiae.org>\n\n");

  /*
   * parse and check command line arguments
   */

  opterr = 0;
  while((opt = getopt(argc, argv, "p:f:n:i:o:s:Pc:b:")) != EOF) {
    switch(opt) {
      case 'p':
        port = atoi(optarg);
        break;
      case 'f':
        ip_file = optarg;
        break;
      case 'n':
        nudge = optarg;
        break;
      case 'i':
        iface = optarg;
        break;
      case 'o':
        out_file = optarg;
        break;
      case 's':
        src_ip = optarg;
        break;
      case 'P':
        pscan_only = 1;
        break;
      case 'c':
        class_c = optarg;
        break;
      case 'b':
        class_b = optarg;
        break;
      default:
        usage(argv[0]);
    }
  }

  if(!src_ip)
    src_ip = getifipaddr(iface);

  if(!src_ip) {
    printf("FATAL: unable to get ip address from %s\n", iface);
    exit(-1);
  }

  if(!port)
    usage(argv[0]);

  if(!class_c && !ip_file && !class_b)
    usage(argv[0]);

  /*
   * check for root access 
   */
 
  if(geteuid()) {
    printf("FATAL: uid is not 0\n");
    exit(-1);
  }

  /*
   * if output file exists, open file for writing 
   */

  if(out_file) {
    if(!(output = fopen(out_file, "w"))) {
      perror("fopen()");
      exit(-1);
    }
  }

  if(ip_file) {

  /*
   * read ip's from file and place in linked list 
   */

    if((n = read_list(ip_file, &ip_list)) <= 0) {
      printf("FATAL: read_list() failed\n");
      exit(-1);
    }
  }

  /*
   * generate list of ip's based off class c addr
   */

  if(class_c) {
    int i;
    char *ptr;
    char buf[32];
    struct sockaddr_in addr;

    snprintf(buf, sizeof(buf), "%s.255", class_c);

    if(!inet_aton(buf, &addr.sin_addr))
      usage(argv[0]);

    for(i = 1; i <= 255; i++) {
      memset(buf, 0, sizeof(buf));
      snprintf(buf, sizeof(buf), "%s.%d", class_c, i);

      ptr = (char *) malloc(strlen(buf) + 1);
      memcpy(ptr, buf, strlen(buf));

     list_insert(ptr, &ip_list);
    }
  }

  /*
   * generate list of ip's based off class b addr
   */

  if(class_b) {
    int i, j;
    char *ptr;
    char buf[32];
    struct sockaddr_in addr;

    snprintf(buf, sizeof(buf), "%s.255.255", class_b);

    if(!inet_aton(buf, &addr.sin_addr))
      usage(argv[0]);

    for(i = 1; i <= 255; i++) {
      for(j = 1; j <= 255; j++) {
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "%s.%d.%d", class_b, i, j);

        ptr = (char *) malloc(strlen(buf) + 1);
        memcpy(ptr, buf, strlen(buf));

       list_insert(ptr, &ip_list);
      }
    }
  }

  printf("scanning %d host(s) for port %d\n", ip_list.list_size, port);

  /*
   * perform syn scan against all ip addresses
   */

  if(!(open_list = synsweep(&ip_list, port, iface))) {
    printf("FATAL: syn_sweep() failed\n");
    exit(-1);
  }

  printf("\ndiscovered %d hosts with %d open\n", open_list->list_size, port);

  /*
   * if portscan only was selected, display results
   */

  if(pscan_only) {
    char *ip_addr = NULL;

    while(open_list->list_size > 0) {
      list_remove((void **)&ip_addr, open_list);
      printf("  %s,%d\n", ip_addr, port);
   
      if(output) {
        fprintf(output,"%s,%d\n", ip_addr, port);
        fflush(output);
      }
    }
    exit(0);
  }

  /*
   * enumerate application banners
   */

  enumeration(open_list, port);

  exit(0);
}

/*
 * function : getifipaddr()
 * purpose  : get ip address from interface
 * arguments: interface name
 * returns  : ip address string, NULL on error
 */

char *getifipaddr (char *ifname)
{
   int sd;
   struct in_addr ipaddr;
   struct ifreq ifr;
   char *ip;

   if (!ifname || !*ifname)
      return (NULL);

   sd = socket (AF_INET, SOCK_DGRAM, 0);

   /* this isnt super good, because if your ip address * is 255.255.255.255
      this is bad */
   if (sd == -1)
      return (NULL);

   strncpy (ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

   if (ioctl (sd, SIOCGIFADDR, &ifr) == -1)
      goto failure;

   /* does this interface have an ip address? */

   /* if its not AF_INET then just go away cause we're done */
   if (ifr.ifr_addr.sa_family != AF_INET)
      goto failure;

   /* save the ip address */
   ipaddr.s_addr = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr;

   ip = inet_ntoa(ipaddr);
   return (ip);

 failure:
   close (sd);
   return (NULL);
}

/*
 * function : usage()
 * purpose  : display the program usage
 * arguments: the name of the program binary
 * returns  : void
 */

void usage(char *name)
{
  printf("usage: %s [options] [-f <infile>] [-c <addr>] [-b <addr>] [-p <port>]"
         "\n       -f\tfile containing list of ip's\n"
	 "       -c\tclass c ip range\n"
	 "       -b\tclass b ip range\n"
         "       -p\tbanner enumeration port\n"
         "       -s\tsource ip to perform scan from\n"
         "       -i\tdefault packet capture interface\n"
         "       -o\toutput file for banner enumeration\n"
         "       -P\tperform portscan only\n", name);
  exit(-1);
}
