



















#include "libavutil/timer.h"

#include <string.h>

#include "libavutil/log.h"
#include "libavutil/adler32.h"

#define LEN 7001

static volatile int checksum;

int main(int argc, char **argv)
{
int i;
uint8_t data[LEN];

av_log_set_level(AV_LOG_DEBUG);

for (i = 0; i < LEN; i++)
data[i] = ((i * i) >> 3) + 123 * i;

if (argc > 1 && !strcmp(argv[1], "-t")) {
for (i = 0; i < 1000; i++) {
START_TIMER;
checksum = av_adler32_update(1, data, LEN);
STOP_TIMER("adler");
}
} else {
checksum = av_adler32_update(1, data, LEN);
}

av_log(NULL, AV_LOG_DEBUG, "%X (expected 50E6E508)\n", checksum);
return checksum == 0x50e6e508 ? 0 : 1;
}

