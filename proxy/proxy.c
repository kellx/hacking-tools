#include <proxy.h>

int
proxy_loop(proxy_t *proxy)
{
   fd_set set;
   int n, sockfd;
   char buf[1500];

   sockfd = proxy_wait(proxy);
   
   for(;;) {
      FD_ZERO(&set);
      FD_SET(sockfd, &set);
      FD_SET(proxy->sockfd[1], &set);

      memset(buf, 0, sizeof(buf));

      n = select(sockfd + 1, &set, NULL, NULL, NULL);

      if(n == -1) {
         perror("select()");
         return(-1);
      }

      if(FD_ISSET(sockfd, &set)) {
         proxy_forward(proxy, sockfd, proxy->sockfd[1]);
      }

      if(FD_ISSET(proxy->sockfd[1], &set)) {
         proxy_forward(proxy, proxy->sockfd[1], sockfd);
      }
   }
   return(0);
}

int 
proxy_forward(proxy_t *proxy, int from, int to)
{
   int n, m;
   char data[1500], desc[1024];

   memset(desc, 0, sizeof(desc));
   snprintf(desc, sizeof(desc) - 1, "From %d to %d", from, to);
   
   n = read(from, data, sizeof(data));
   if (!n)
      return (-1);
   if (n == -1)
      exit(-1);

   proxy->callback_func(data, n);
   
   m = write(to, data, n);
   if (m != n)
      exit(-1);
             
   return (0);
}

int
proxy_wait(proxy_t *proxy)
{
   int sockfd, size;
   struct sockaddr_in sin;
  
   size = sizeof(sin);
   
   if((sockfd = accept(proxy->sockfd[0], (struct sockaddr *)&sin, &size)) < 0) {
      perror("sockfd()");
      return(-1);
   }

   return(sockfd);
}

proxy_t *
proxy_init(int port_listen, int port_forward, long ip_forward, void (*callback_func)(char *, int))
{
   proxy_t *proxy;
   struct sockaddr_in sin;

   proxy = (proxy_t *)malloc(sizeof(proxy_t));

   proxy->port_listen = port_listen;
   proxy->port_forward = port_forward;
   proxy->ip_forward = ip_forward;
   proxy->callback_func = callback_func;

   // initialize listening socket descriptor
   if((proxy->sockfd[0] = proxy_bind_socket(proxy->port_listen)) < 0)
      return(NULL);

   // initialize forwarding socket descriptor
   if((proxy->sockfd[1] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket()");
      return(NULL);
   }

   sin.sin_port = htons(port_forward);
   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = ip_forward;

   // make connection
   if(connect(proxy->sockfd[1], (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      perror("connect()");
      return(NULL);
   }

   return(proxy);
}

int 
proxy_bind_socket(int port)
{
   int opt = 1;
   int sockfd;              /* bound socket descriptor   */
   struct sockaddr_in sin;  /* socket information struct */

  /*
   * initialize socket and bind to specified port
   */

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket()");
    return(-1);
  }

  sin.sin_port = htons(port);
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;

  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
     perror("setsockopt()");
     return(-1);
  }

  if(bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("bind()");
    return(-1);
  }

  if(listen(sockfd, 100) < 0) {
    perror("listen()");
    return(-1);
  }
  return(sockfd);
}
