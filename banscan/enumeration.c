#include <banscan.h>

/*
 * function : enumeration()
 * purpose  : enumerate application banners on a specified port
 * arguments: list of ip addresses & port
 * returns  : void
 */

void enumeration(list_t *list, int port)
{
  char *ip;		/* PROG: ip address with open port */

  /*
   * traverse through list and gather the banner for each host
   */

  while(list->list_size > 0) {
    ip = NULL;
    list_remove((void **)&ip, list);
    get_banner(ip, port);
  }
}

/*
 * function : get_banner()
 * purpose  : print the application banner of a specified port
 * arguments: ip address and port
 * returns  : void
 */

void get_banner(char *ip, int port)
{
  int sockfd;			/* PROG: socket descriptor */
  char buf[1024];		/* PROG: recieve buffer    */
  struct sockaddr_in sin;	/* PROG: socket structure  */

  memset(buf, 0, sizeof(buf));

  /*
   * initialize socket descriptor & signal handlers
   */

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    puts("HERE!");
    return;
  }

  sin.sin_port = htons(port);
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr(ip);

  signal(SIGALRM, sighandler);

  /*
   * connect to remote host
   */

  alarm(3);
  if(connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    close(sockfd);
    alarm(0);
    return;
  }
  alarm(0);

  /*
   * read the application banner
   */

  recvline(sockfd, buf, sizeof(buf));

  printf("  %s,%d,%.110s\n", ip, port, strtok(buf, "\n"));
  
  if(output) {
    fprintf(output, "%s,%d,%.110s\n", ip, port, strtok(buf, "\n"));
    fflush(output);
  }

  close(sockfd);
}
