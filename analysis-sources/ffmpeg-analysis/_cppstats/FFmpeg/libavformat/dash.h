#include "avformat.h"
typedef enum {
DASH_TMPL_ID_UNDEFINED = -1,
DASH_TMPL_ID_ESCAPE,
DASH_TMPL_ID_REP_ID,
DASH_TMPL_ID_NUMBER,
DASH_TMPL_ID_BANDWIDTH,
DASH_TMPL_ID_TIME,
} DASHTmplId;
void ff_dash_fill_tmpl_params(char *dst, size_t buffer_size, const char *template, int rep_id, int number, int bit_rate, int64_t time);
