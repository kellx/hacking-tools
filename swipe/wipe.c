#include <swipe.h>

/*
 * function : wipe()
 * purpose  : perform secure deletion of a file
 * arguments: filename
 * returns  : void 
 */

void wipe(char *filename)
{
  int i; 			/* PROG: general purpose counters     */
  int j; 			/* PROG: general purpose counters     */
  int rfd;			/* PROG: /dev/urandom file descriptor */
  int fd;			/* PROG: fd for overwritten file      */
  int bwrites;			/* PROG: number of 1024 writes        */
  int swrites;			/* PROG: number of one byte writes    */
  FILE *file;			/* PROG: for fdopen() on fd           */
  char rand[1024];		/* PROG: random bytes of data         */
  struct stat st_stat;		/* PROG: file attribute information   */

  /*
   * open /dev/urandom 
   */

  if((rfd = open("/dev/urandom", O_RDONLY)) < 0) {
    printf("FATAL: unable to open /dev/urandom\n");
    exit(-1);
  }

  /*
   * open file to be overwritten
   */

  if((fd = open(filename, O_WRONLY)) < 0) {
    printf("FATAL: unable to open %s for writing\n", filename);
    exit(-1);
  }

  if(!(file = fdopen(fd, "w"))) {
    printf("FATAL: unable to fdopen() fd for writing\n");
    exit(-1);
  }

  setvbuf(file, NULL, _IONBF, 0);

  /*
   * get the file's attributes
   */

  if(fstat(fd, &st_stat) < 0) {
    printf("FATAL: fstat() failed\n");
    exit(-1);
  }

  /*
   * calculate number of large and small writes
   */

  bwrites = st_stat.st_size / 1024;
  swrites = st_stat.st_size % 1024;

  /*
   * perform n number of overwrites
   */

  for(j = 0; j < wipe_num; j++) {

    /*
     * perform large, 1024 byte overwrites
     */

    for(i = 0; i < bwrites; i++) {
      read(rfd, rand, sizeof(rand));
      fwrite(rand, 1, 1024, file);
    }

    /*
     * perform small, one byte overwrites
     */

    for(i = 0; i < swrites; i++) {
      read(rfd, rand, 1);
      fwrite(rand, 1, 1, file);
    }

    /* 
     * rewind the file for another round
     */

    lseek(fd, -(long)(st_stat.st_size), SEEK_END);
  }

  /*
   * close files and unlink()
   */

  close(rfd);
  close(fd);
  fclose(file);

  unlink(filename);
}
