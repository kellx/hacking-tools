/* 
 * taken from mike price's libiputil
 */

#include <banscan.h>

/*
 * function : recvline()
 * purpose  : read a line from a socket
 * arguments: socket descriptor, destination buffer, desired size
 * returns  : length read from socket, -1 on error
 */

int recvline(int sd, char *buffer, int buflen)
{
  int len = 0;			/* PROG: current length read */
  fd_set fdset;			/* PROG: file descriptor set */
  struct timeval tv;		/* PROG: for select timeout  */

  /*
   * set timeout to 10 seconds
   */

  tv.tv_sec  = 10;
  tv.tv_usec = 0;

  /*
   * when fd's are available, do appropriate actions
   */

  while(len < buflen - 1) {
    FD_ZERO(&fdset);
    FD_SET(sd, &fdset);

    if(select(sd + 1, &fdset, NULL, NULL, &tv) <= 0)
      break;

    if(read(sd, buffer + len, 1) <= 0)
      break;

    ++len;

    if(buffer[len - 1] == '\r')
      --len;

    if(buffer[len - 1] == '\n') {
      --len;
      break;
    }
  }

  if(len > 0)
    buffer[len] = '\0';

  return(len);
}
