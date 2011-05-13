/* $Id: rc4.h,v 1.1 2000/12/18 23:17:00 bind Exp $ */

typedef struct rc4 rc4_t;

int rc4close(rc4_t * rc4);
unsigned char rc4getc(rc4_t * rc4);
rc4_t *rc4open(const void *key, int len);
int rc4crypt(void *key, int keylen, void *buffer, int len);
