#ifndef _LIB_PROXY

#define _LIB_PROXY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct proxy {
   int sockfd[2];                   // 2 socket descriptors, one listening, one forwarding
   int port_listen;                 // the port to listen on
   int port_forward;                // the port to forward to
   long ip_forward;                 // the host to forward to
   void (*callback_func)(char *, int);   // callback function when processing packets
};

typedef struct proxy proxy_t;

proxy_t *proxy_init(int, int, long, void (*callback_func)(char *, int));
int proxy_loop(proxy_t *);
int proxy_wait(proxy_t *);
int proxy_forward(proxy_t *, int, int);
int proxy_bind_socket(int);


#endif
