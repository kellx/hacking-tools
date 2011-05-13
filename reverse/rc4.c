#include <rc4.h>
#include <stdlib.h>

#define xorswap(a,b) do{a^=b;b^=a;a^=b;}while(0)

struct rc4 {
  unsigned char sbox[256];
  unsigned char kbox[256];
  int i, j;
};

rc4_t *
rc4open(const void *key, int len)
{
  rc4_t *rc4;

  if (!key)
    return (NULL);

  rc4 = (rc4_t *) malloc(sizeof(*rc4));
  if (!rc4)
    return (NULL);

  for (rc4->i = 256; rc4->i--;) {
    rc4->sbox[rc4->i] = rc4->i;
    rc4->kbox[rc4->i] = ((unsigned char *) key)[rc4->i % len];
  }

  rc4->j = 0;
  for (rc4->i = 0; rc4->i <= 255; rc4->i++) {
    rc4->j = (rc4->j + rc4->sbox[rc4->i] + rc4->kbox[rc4->i]) % 256;
    xorswap(rc4->sbox[rc4->i], rc4->sbox[rc4->j]);
  }

  return (rc4);

}

unsigned char 
rc4getc(rc4_t * rc4)
{
  int t;
  if (!rc4)
    return (-1);
  rc4->i++;
  rc4->i %= 256;
  rc4->j += rc4->sbox[rc4->i];
  rc4->j %= 256;
  xorswap(rc4->sbox[rc4->i], rc4->sbox[rc4->j]);
  t = (rc4->sbox[rc4->i] + rc4->sbox[rc4->j]) % 256;
  return (rc4->sbox[t]);
}

int 
rc4close(rc4_t * rc4)
{
  if (!rc4)
    return (-1);
  free(rc4);
  return (0);
}

int 
rc4crypt(void *key, int keylen, void *buffer, int len)
{
  int i;
  rc4_t *rc4;
  rc4 = rc4open(key, keylen);

  if (!rc4)
    return (-1);

  for (i = 0; i < len; i++)
    ((unsigned char *) buffer)[i] ^= rc4getc(rc4);

  return (rc4close(rc4));
}
