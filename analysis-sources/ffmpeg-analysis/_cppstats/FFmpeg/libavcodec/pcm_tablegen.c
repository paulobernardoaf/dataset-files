#include <stdlib.h>
#define CONFIG_HARDCODED_TABLES 0
#include "pcm_tablegen.h"
#include "tableprint.h"
int main(void)
{
pcm_alaw_tableinit();
pcm_ulaw_tableinit();
pcm_vidc_tableinit();
write_fileheader();
WRITE_ARRAY("static const", uint8_t, linear_to_alaw);
WRITE_ARRAY("static const", uint8_t, linear_to_ulaw);
WRITE_ARRAY("static const", uint8_t, linear_to_vidc);
return 0;
}
