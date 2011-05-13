#include <proxy.h>

void callback(char *buf, int size);
void hexdump(char *desc, unsigned char *data, unsigned int amount);

int
main(int argc, char **argv)
{
   proxy_t *proxy;

   if(argc < 4) {
      printf("usage: %s <listen port> <forward port> <forward ip>\n", argv[0]);
      exit(-1);
   }

   if(!(proxy = proxy_init(atoi(argv[1]), atoi(argv[2]), inet_addr(argv[3]), &callback)))
      exit(-1);

   proxy_loop(proxy);

   exit(0);
}

void callback(char *buf, int size)
{
   hexdump("heh", buf, size);
}

void
hexdump(char *desc, unsigned char *data, unsigned int amount)
{
   unsigned int dp, p;  /* data pointer */
   const char trans[] =
   "................................ !\"#$%&'()*+,-./0123456789"
   ":;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklm"
   "nopqrstuvwxyz{|}~...................................."
   "....................................................."
   "........................................";

   printf("/* %s */\n", desc);

   for (dp = 1; dp <= amount; dp++) {
      fprintf(stderr, "%02x ", data[dp - 1]);
      if ((dp % 8) == 0)
         fprintf(stderr, " ");
      if ((dp % 16) == 0) {
         fprintf(stderr, "| ");
         p = dp;
         for (dp -= 16; dp < p; dp++)
            fprintf(stderr, "%c", trans[data[dp]]);
         fflush(stderr);
         fprintf(stderr, "\n");
      }
      fflush(stderr);
   }
   if ((amount % 16) != 0) {
      p = dp = 16 - (amount % 16);
      for (dp = p; dp > 0; dp--) {
         fprintf(stderr, "   ");
         if (((dp % 8) == 0) && (p != 8))
            fprintf(stderr, " ");
         fflush(stderr);
      }
      fprintf(stderr, " | ");
      for (dp = (amount - (16 - p)); dp < amount; dp++)
         fprintf(stderr, "%c", trans[data[dp]]);
      fflush(stderr);
   }
   fprintf(stderr, "\n");

   return;
}
