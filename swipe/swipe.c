/*
 * secure wipe (swipe) - secure log wiping program
 * written by bind <bind@insidiae.org>
 *
 * swipe performs secure disk overwrites and removes the entries
 * completely from the log files, rather than zero'ing out entries 
 * which can lead to detection.
 *
 * usage: ./swipe [-U|-W|-L|-A] [-n <rounds>] [-t <tty>] <username>
 *        -U       erase username from UTMP
 *        -W       erase username from WTMP
 *        -L       erase username from LASTLOG
 *        -A       erase username from ACCT
 *        -n       number of rounds, 0 for none
 *        -t       specific tty to erase from logs
 *        username user to remove from logs
 *
 * copyright (c) 2002 bind@insidiae.org
 * http://www.insidiae.org/~bind
 */

#include <swipe.h>

/*
 * number of file wipes to perform
 */

int wipe_num = 20;

int main(int argc, char **argv)
{
  int opt;			/* IN: getopt() option character       */
  int swipe_opt;		/* IN: log wiping options              */ 
  char *tty;			/* IN: specific tty to erase from logs */
  char *username;		/* IN: username to erase from logs     */

  char *prog;			/* PROG: saved argv[0]                 */

  /* 
   * initialize swipe
   */

  swipe_opt = 0;
  username = tty = NULL;
  prog = argv[0]; 

  setvbuf(stdout, NULL, _IONBF, 0);

  /*
   * display application banner
   */

  printf("secure wipe (swipe) - secure log wiping program\n"
         "written by bind <bind@insidiae.org>\n");

  /*
   * parse and check command line arguments
   */

  opterr = 0;
  while((opt = getopt(argc, argv, "UWLAr:t:")) != EOF) {
    switch(opt) {
      case 'U':
        swipe_opt |= WIPE_UTMP;
        break;
      case 'W':
        swipe_opt |= WIPE_WTMP;
        break;
      case 'L':
        swipe_opt |= WIPE_LASTLOG;
        break;
      case 'A':
        swipe_opt |= WIPE_ACCT;
        break;
      case 'r':
        wipe_num = atoi(optarg);
        break;
      case 't':
        tty = optarg;
        break;
      default:
        usage(prog); 
    }
  }

  argv += optind;
  username = argv[0];

  if(!swipe_opt || !username)
    usage(prog);

  putchar('\n');

  /*
   * check for root access
   */

  if(geteuid()) {
    printf("FATAL: uid is not 0\n");
    exit(-1);
  } 

  /*
   * perform WTMP and WTMPX wipe
   */

  if(swipe_opt & WIPE_WTMP) {
    wipe_wtmp(username);

#ifdef _HAVE_WTMPX
    wipe_wtmpx(username);
#endif
  }

  /*
   * perform UTMP wipe
   */

  if(swipe_opt & WIPE_UTMP)
    wipe_utmp(username, tty);

  /*
   * perform LASTLOG wipe
   */

  if(swipe_opt & WIPE_LASTLOG)
    wipe_lastlog(username);

  exit(0);
}

/*
 * function : usage()
 * purpose  : display the program usage
 * arguments: the name of the program binary
 * returns  : void
 */

void usage(char *name)
{
  printf("usage: %s [-U|-W|-L|-A] [-r <rounds>] [-t <tty>] <username>\n"
         "       -U\terase username from UTMP\n"
         "       -W\terase username from WTMP\n"
         "       -L\terase username from LASTLOG\n"
         "       -A\terase username from ACCT\n"
         "       -r\tnumber of rounds, 0 for none\n"
	 "       -t\tspecific tty to erase from logs\n"
         "       username\tuser to remove from logs\n", name);
  exit(-1);
}
