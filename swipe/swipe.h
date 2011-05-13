#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <utmp.h>
#include <pwd.h>

#ifdef _HAVE_WTMPX
#include <utmpx.h>
#endif

/*
 * log wiping options
 */

#define WIPE_UTMP    (1)
#define WIPE_WTMP    (1 << 1)
#define WIPE_ACCT    (1 << 2)
#define WIPE_FILE    (1 << 3)
#define WIPE_LASTLOG (1 << 4)

/*
 * file attributes structure
 */

struct fstats {
  int uid;			/* owner of the file       */
  int gid;			/* group of the file       */
  int mode;			/* permissions of the file */
  struct utimbuf time;		/* timestamp of the file   */
};

/*
 * function prototypes
 */

void usage(char *); 
void wipe_wtmp(char *);
void wipe_lastlog(char *);
void wipe_utmp(char *, char *);
void wipe(char *);

/* 
 * global externs
 */

extern int wipe_num;
