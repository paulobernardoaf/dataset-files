#include "config.h"
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/rational.h"
#include "libavutil/time_internal.h"
#include "avc.h"
#include "avformat.h"
#include "avio_internal.h"
#include "internal.h"
#include "isom.h"
#include "os_support.h"
#include "url.h"
#include "dash.h"
static DASHTmplId dash_read_tmpl_id(const char *identifier, char *format_tag,
size_t format_tag_size, const char **ptr) {
const char *next_ptr;
DASHTmplId id_type = DASH_TMPL_ID_UNDEFINED;
if (av_strstart(identifier, "$$", &next_ptr)) {
id_type = DASH_TMPL_ID_ESCAPE;
*ptr = next_ptr;
} else if (av_strstart(identifier, "$RepresentationID$", &next_ptr)) {
id_type = DASH_TMPL_ID_REP_ID;
av_strlcpy(format_tag, "%d", format_tag_size);
*ptr = next_ptr;
} else { 
if (av_strstart(identifier, "$Number", &next_ptr))
id_type = DASH_TMPL_ID_NUMBER;
else if (av_strstart(identifier, "$Bandwidth", &next_ptr))
id_type = DASH_TMPL_ID_BANDWIDTH;
else if (av_strstart(identifier, "$Time", &next_ptr))
id_type = DASH_TMPL_ID_TIME;
else
id_type = DASH_TMPL_ID_UNDEFINED;
if (id_type != DASH_TMPL_ID_UNDEFINED) {
const char *number_format = (id_type == DASH_TMPL_ID_TIME) ? PRId64 : "d";
if (next_ptr[0] == '$') { 
snprintf(format_tag, format_tag_size, "%%%s", number_format);
*ptr = &next_ptr[1];
} else {
const char *width_ptr;
if (av_strstart(next_ptr, "%0", &width_ptr) &&
av_isdigit(width_ptr[0]) &&
av_strstart(&width_ptr[1], "d$", &next_ptr)) {
snprintf(format_tag, format_tag_size, "%s%c%s", "%0", width_ptr[0], number_format);
*ptr = next_ptr;
} else {
av_log(NULL, AV_LOG_WARNING, "Failed to parse format-tag beginning with %s. Expected either a "
"closing '$' character or a format-string like '%%0[width]d', "
"where width must be a single digit\n", next_ptr);
id_type = DASH_TMPL_ID_UNDEFINED;
}
}
}
}
return id_type;
}
void ff_dash_fill_tmpl_params(char *dst, size_t buffer_size,
const char *template, int rep_id,
int number, int bit_rate,
int64_t time) {
int dst_pos = 0;
const char *t_cur = template;
while (dst_pos < buffer_size - 1 && *t_cur) {
char format_tag[7]; 
int n = 0;
DASHTmplId id_type;
const char *t_next = strchr(t_cur, '$'); 
if (t_next) {
int num_copy_bytes = FFMIN(t_next - t_cur, buffer_size - dst_pos - 1);
av_strlcpy(&dst[dst_pos], t_cur, num_copy_bytes + 1);
dst_pos += num_copy_bytes;
t_cur = t_next;
} else { 
av_strlcpy(&dst[dst_pos], t_cur, buffer_size - dst_pos);
break;
}
if (dst_pos >= buffer_size - 1 || !*t_cur)
break;
id_type = dash_read_tmpl_id(t_cur, format_tag, sizeof(format_tag), &t_next);
switch (id_type) {
case DASH_TMPL_ID_ESCAPE:
av_strlcpy(&dst[dst_pos], "$", 2);
n = 1;
break;
case DASH_TMPL_ID_REP_ID:
n = snprintf(&dst[dst_pos], buffer_size - dst_pos, format_tag, rep_id);
break;
case DASH_TMPL_ID_NUMBER:
n = snprintf(&dst[dst_pos], buffer_size - dst_pos, format_tag, number);
break;
case DASH_TMPL_ID_BANDWIDTH:
n = snprintf(&dst[dst_pos], buffer_size - dst_pos, format_tag, bit_rate);
break;
case DASH_TMPL_ID_TIME:
n = snprintf(&dst[dst_pos], buffer_size - dst_pos, format_tag, time);
break;
case DASH_TMPL_ID_UNDEFINED:
av_strlcpy(&dst[dst_pos], t_cur, 2);
n = 1;
t_next = &t_cur[1];
break;
}
dst_pos += FFMIN(n, buffer_size - dst_pos - 1);
t_cur = t_next;
}
}
