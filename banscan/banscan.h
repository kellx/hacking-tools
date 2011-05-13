#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <time.h>
#include <pcap.h>

#include "list.h"
#include "tcp.h"

extern FILE *output;
extern char *src_ip;
extern int src_port;

/* 
 * prototypes for core functions
 */

void usage(char *);
int read_list(char *, list_t *);
list_t *synsweep(list_t *, int, char *);
void sendsyn(char *, char *, int);
int recvpkts(list_t *, char *);
void enumeration(list_t *, int);
void get_banner(char *, int);
int recvline(int, char *, int);
char *getifipaddr (char *);

/*
 * prototypes for helper functions
 */

void sighandler(int);
u_short in_cksum(u_short *, int);
u_short tcp_cksum(char *, int, struct in_addr, struct in_addr);
