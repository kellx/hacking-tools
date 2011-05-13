#include <banscan.h>

/*
 * function : read_list()
 * purpose  : read ip's from a file and place in a linked list
 * arguments: filename, pointer to linked list
 * returns  : number of ip's added to list
 */

int read_list(char *ip_file, list_t *list)
{
  FILE *file;			/* PROG: pointer to file on disk         */
  char buf[1024];		/* PROG: general purpose buffer          */  
  char *parse;			/* PROG: used to sanitize file input     */
  char *ptr;			/* PROG: general purpose pointer         */
  struct sockaddr_in addr;      /* PROG: to check if ip address is valid */

  /*
   * open ip file for reading
   */

  if(!(file = fopen(ip_file, "r"))) 
    return(-1);

  /* 
   * traverse ip file, find valid ips, & put in linked list
   */

  for(;;) {
    memset(buf, 0, sizeof(buf));

    if(feof(file))
      break;

    fgets(buf, sizeof(buf), file);
    parse = strtok(buf, "\n");

    if(!parse)
      continue;

    if(!inet_aton(parse, &addr.sin_addr))
      continue;

    ptr = (char *) malloc(strlen(parse) + 1);
    memcpy(ptr, parse, strlen(parse));

    list_insert(ptr, list);
  }

  /* 
   * close out disk file & return
   */

  fclose(file);
  return(list->list_size);
}
