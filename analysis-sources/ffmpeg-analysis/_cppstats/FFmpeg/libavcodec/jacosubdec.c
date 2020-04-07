#include <time.h>
#include "ass.h"
#include "jacosub.h"
#include "libavutil/avstring.h"
#include "libavutil/bprint.h"
#include "libavutil/time_internal.h"
#undef time
static int insert_text(AVBPrint *dst, const char *in, const char *arg)
{
av_bprintf(dst, "%s", arg);
return 0;
}
static int insert_datetime(AVBPrint *dst, const char *in, const char *arg)
{
char buf[16] = {0};
time_t now = time(0);
struct tm ltime;
localtime_r(&now, &ltime);
if (strftime(buf, sizeof(buf), arg, &ltime))
av_bprintf(dst, "%s", buf);
return 0;
}
static int insert_color(AVBPrint *dst, const char *in, const char *arg)
{
return 1; 
}
static int insert_font(AVBPrint *dst, const char *in, const char *arg)
{
return 1; 
}
static const struct {
const char *from;
const char *arg;
int (*func)(AVBPrint *dst, const char *in, const char *arg);
} ass_codes_map[] = {
{"\\~", "~", insert_text}, 
{"~", "{\\h}", insert_text}, 
{"\\n", "\\N", insert_text}, 
{"\\D", "%d %b %Y", insert_datetime}, 
{"\\T", "%H:%M", insert_datetime}, 
{"\\N", "{\\r}", insert_text}, 
{"\\I", "{\\i1}", insert_text}, 
{"\\i", "{\\i0}", insert_text}, 
{"\\B", "{\\b1}", insert_text}, 
{"\\b", "{\\b0}", insert_text}, 
{"\\U", "{\\u1}", insert_text}, 
{"\\u", "{\\u0}", insert_text}, 
{"\\C", "", insert_color}, 
{"\\F", "", insert_font}, 
};
enum {
ALIGN_VB = 1<<0, 
ALIGN_VM = 1<<1, 
ALIGN_VT = 1<<2, 
ALIGN_JC = 1<<3, 
ALIGN_JL = 1<<4, 
ALIGN_JR = 1<<5, 
};
static void jacosub_to_ass(AVCodecContext *avctx, AVBPrint *dst, const char *src)
{
int i, valign = 0, halign = 0;
char c = av_toupper(*src);
char directives[128] = {0};
if ((c >= 'A' && c <= 'Z') || c == '[') {
char *p = directives;
char *pend = directives + sizeof(directives) - 1;
do *p++ = av_toupper(*src++);
while (*src && !jss_whitespace(*src) && p < pend);
*p = 0;
src = jss_skip_whitespace(src);
}
if (strstr(directives, "VB")) valign = ALIGN_VB;
else if (strstr(directives, "VM")) valign = ALIGN_VM;
else if (strstr(directives, "VT")) valign = ALIGN_VT;
if (strstr(directives, "JC")) halign = ALIGN_JC;
else if (strstr(directives, "JL")) halign = ALIGN_JL;
else if (strstr(directives, "JR")) halign = ALIGN_JR;
if (valign || halign) {
if (!valign) valign = ALIGN_VB;
if (!halign) halign = ALIGN_JC;
switch (valign | halign) {
case ALIGN_VB | ALIGN_JL: av_bprintf(dst, "{\\an1}"); break; 
case ALIGN_VB | ALIGN_JC: av_bprintf(dst, "{\\an2}"); break; 
case ALIGN_VB | ALIGN_JR: av_bprintf(dst, "{\\an3}"); break; 
case ALIGN_VM | ALIGN_JL: av_bprintf(dst, "{\\an4}"); break; 
case ALIGN_VM | ALIGN_JC: av_bprintf(dst, "{\\an5}"); break; 
case ALIGN_VM | ALIGN_JR: av_bprintf(dst, "{\\an6}"); break; 
case ALIGN_VT | ALIGN_JL: av_bprintf(dst, "{\\an7}"); break; 
case ALIGN_VT | ALIGN_JC: av_bprintf(dst, "{\\an8}"); break; 
case ALIGN_VT | ALIGN_JR: av_bprintf(dst, "{\\an9}"); break; 
}
}
while (*src && *src != '\n') {
if (src[0] == '\\' && src[1] == '\n') {
src += 2;
while (jss_whitespace(*src))
src++;
continue;
}
for (i = 0; i < FF_ARRAY_ELEMS(ass_codes_map); i++) {
const char *from = ass_codes_map[i].from;
const char *arg = ass_codes_map[i].arg;
size_t codemap_len = strlen(from);
if (!strncmp(src, from, codemap_len)) {
src += codemap_len;
src += ass_codes_map[i].func(dst, src, arg);
break;
}
}
if (i == FF_ARRAY_ELEMS(ass_codes_map))
av_bprintf(dst, "%c", *src++);
}
}
static int jacosub_decode_frame(AVCodecContext *avctx,
void *data, int *got_sub_ptr, AVPacket *avpkt)
{
int ret;
AVSubtitle *sub = data;
const char *ptr = avpkt->data;
FFASSDecoderContext *s = avctx->priv_data;
if (avpkt->size <= 0)
goto end;
if (*ptr) {
AVBPrint buffer;
ptr = jss_skip_whitespace(ptr);
ptr = strchr(ptr, ' '); if (!ptr) goto end; ptr++;
ptr = strchr(ptr, ' '); if (!ptr) goto end; ptr++;
av_bprint_init(&buffer, JSS_MAX_LINESIZE, JSS_MAX_LINESIZE);
jacosub_to_ass(avctx, &buffer, ptr);
ret = ff_ass_add_rect(sub, buffer.str, s->readorder++, 0, NULL, NULL);
av_bprint_finalize(&buffer, NULL);
if (ret < 0)
return ret;
}
end:
*got_sub_ptr = sub->num_rects > 0;
return avpkt->size;
}
AVCodec ff_jacosub_decoder = {
.name = "jacosub",
.long_name = NULL_IF_CONFIG_SMALL("JACOsub subtitle"),
.type = AVMEDIA_TYPE_SUBTITLE,
.id = AV_CODEC_ID_JACOSUB,
.init = ff_ass_subtitle_header_default,
.decode = jacosub_decode_frame,
.flush = ff_ass_decoder_flush,
.priv_data_size = sizeof(FFASSDecoderContext),
};
