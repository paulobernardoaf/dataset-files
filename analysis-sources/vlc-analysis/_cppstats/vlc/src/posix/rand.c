#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_rand.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <vlc_fs.h>
#include <vlc_md5.h>
#define BLOCK_SIZE 64
static uint8_t okey[BLOCK_SIZE], ikey[BLOCK_SIZE];
static void vlc_rand_init (void)
{
uint8_t key[BLOCK_SIZE];
int fd = vlc_open ("/dev/urandom", O_RDONLY);
if (fd == -1)
return; 
for (size_t i = 0; i < sizeof (key);)
{
ssize_t val = read (fd, key + i, sizeof (key) - i);
if (val > 0)
i += val;
}
for (size_t i = 0; i < sizeof (key); i++)
{
okey[i] = key[i] ^ 0x5c;
ikey[i] = key[i] ^ 0x36;
}
vlc_close (fd);
}
void vlc_rand_bytes (void *buf, size_t len)
{
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static uint64_t counter = 0;
uint64_t stamp = NTPtime64 ();
while (len > 0)
{
uint64_t val;
struct md5_s mdi, mdo;
InitMD5 (&mdi);
InitMD5 (&mdo);
pthread_mutex_lock (&lock);
if (counter == 0)
vlc_rand_init ();
val = counter++;
AddMD5 (&mdi, ikey, sizeof (ikey));
AddMD5 (&mdo, okey, sizeof (okey));
pthread_mutex_unlock (&lock);
AddMD5 (&mdi, &stamp, sizeof (stamp));
AddMD5 (&mdi, &val, sizeof (val));
EndMD5 (&mdi);
AddMD5 (&mdo, mdi.buf, 16);
EndMD5 (&mdo);
if (len < 16)
{
memcpy (buf, mdo.buf, len);
break;
}
memcpy (buf, mdo.buf, 16);
len -= 16;
buf = ((uint8_t *)buf) + 16;
}
}
