

























#include "ass.h"
#include "libavutil/avstring.h"
#include "libavutil/bprint.h"
#include "htmlsubtitles.h"

typedef struct {
AVBPrint source;
AVBPrint content;
AVBPrint encoded_source;
AVBPrint encoded_content;
AVBPrint full;
int readorder;
} SAMIContext;

static int sami_paragraph_to_ass(AVCodecContext *avctx, const char *src)
{
SAMIContext *sami = avctx->priv_data;
int ret = 0;
char *tag = NULL;
char *dupsrc = av_strdup(src);
char *p = dupsrc;
AVBPrint *dst_content = &sami->encoded_content;
AVBPrint *dst_source = &sami->encoded_source;

if (!dupsrc)
return AVERROR(ENOMEM);

av_bprint_clear(&sami->encoded_content);
av_bprint_clear(&sami->content);
av_bprint_clear(&sami->encoded_source);
for (;;) {
char *saveptr = NULL;
int prev_chr_is_space = 0;
AVBPrint *dst = &sami->content;


p = av_stristr(p, "<P");
if (!p)
break;
if (p[2] != '>' && !av_isspace(p[2])) { 
p++;
continue;
}
if (dst->len) 
av_bprintf(dst, "\\N");
tag = av_strtok(p, ">", &saveptr);
if (!tag || !saveptr)
break;
p = saveptr;


if (av_stristr(tag, "ID=Source") || av_stristr(tag, "ID=\"Source\"")) {
dst = &sami->source;
av_bprint_clear(dst);
}


while (av_isspace(*p))
p++;
if (!strncmp(p, "&nbsp;", 6)) {
ret = -1;
goto end;
}


while (*p) {
if (*p == '<') {
if (!av_strncasecmp(p, "<P", 2) && (p[2] == '>' || av_isspace(p[2])))
break;
}
if (!av_strncasecmp(p, "<BR", 3)) {
av_bprintf(dst, "\\N");
p++;
while (*p && *p != '>')
p++;
if (!*p)
break;
if (*p == '>')
p++;
continue;
}
if (!av_isspace(*p))
av_bprint_chars(dst, *p, 1);
else if (!prev_chr_is_space)
av_bprint_chars(dst, ' ', 1);
prev_chr_is_space = av_isspace(*p);
p++;
}
}

av_bprint_clear(&sami->full);
if (sami->source.len) {
ret = ff_htmlmarkup_to_ass(avctx, dst_source, sami->source.str);
if (ret < 0)
goto end;
av_bprintf(&sami->full, "{\\i1}%s{\\i0}\\N", sami->encoded_source.str);
}
ret = ff_htmlmarkup_to_ass(avctx, dst_content, sami->content.str);
if (ret < 0)
goto end;
av_bprintf(&sami->full, "%s", sami->encoded_content.str);

end:
av_free(dupsrc);
return ret;
}

static int sami_decode_frame(AVCodecContext *avctx,
void *data, int *got_sub_ptr, AVPacket *avpkt)
{
AVSubtitle *sub = data;
const char *ptr = avpkt->data;
SAMIContext *sami = avctx->priv_data;

if (ptr && avpkt->size > 0) {
int ret = sami_paragraph_to_ass(avctx, ptr);
if (ret < 0)
return ret;

ret = ff_ass_add_rect(sub, sami->full.str, sami->readorder++, 0, NULL, NULL);
if (ret < 0)
return ret;
}
*got_sub_ptr = sub->num_rects > 0;
return avpkt->size;
}

static av_cold int sami_init(AVCodecContext *avctx)
{
SAMIContext *sami = avctx->priv_data;
av_bprint_init(&sami->source, 0, 2048);
av_bprint_init(&sami->content, 0, 2048);
av_bprint_init(&sami->encoded_source, 0, 2048);
av_bprint_init(&sami->encoded_content, 0, 2048);
av_bprint_init(&sami->full, 0, 2048);
return ff_ass_subtitle_header_default(avctx);
}

static av_cold int sami_close(AVCodecContext *avctx)
{
SAMIContext *sami = avctx->priv_data;
av_bprint_finalize(&sami->source, NULL);
av_bprint_finalize(&sami->content, NULL);
av_bprint_finalize(&sami->encoded_source, NULL);
av_bprint_finalize(&sami->encoded_content, NULL);
av_bprint_finalize(&sami->full, NULL);
return 0;
}

static void sami_flush(AVCodecContext *avctx)
{
SAMIContext *sami = avctx->priv_data;
if (!(avctx->flags2 & AV_CODEC_FLAG2_RO_FLUSH_NOOP))
sami->readorder = 0;
}

AVCodec ff_sami_decoder = {
.name = "sami",
.long_name = NULL_IF_CONFIG_SMALL("SAMI subtitle"),
.type = AVMEDIA_TYPE_SUBTITLE,
.id = AV_CODEC_ID_SAMI,
.priv_data_size = sizeof(SAMIContext),
.init = sami_init,
.close = sami_close,
.decode = sami_decode_frame,
.flush = sami_flush,
};
