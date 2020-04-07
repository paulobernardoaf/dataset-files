#include <stdlib.h>
#define CONFIG_HARDCODED_TABLES 0
#include "libavutil/tablegen.h"
#include "mpegaudio_tablegen.h"
#include "tableprint.h"
int main(void)
{
mpegaudio_tableinit();
write_fileheader();
WRITE_ARRAY("static const", int8_t, table_4_3_exp);
WRITE_ARRAY("static const", uint32_t, table_4_3_value);
WRITE_ARRAY("static const", uint32_t, exp_table_fixed);
WRITE_ARRAY("static const", float, exp_table_float);
WRITE_2D_ARRAY("static const", uint32_t, expval_table_fixed);
WRITE_2D_ARRAY("static const", float, expval_table_float);
return 0;
}
