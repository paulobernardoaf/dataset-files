#include <string.h>
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"
static void print_usage(void)
{
fprintf(stderr, "Usage: enum_options type\n"
"type: format codec\n");
exit(1);
}
static void print_option(const AVClass *class, const AVOption *o)
{
printf("@item -%s @var{", o->name);
switch (o->type) {
case FF_OPT_TYPE_BINARY: printf("hexadecimal string"); break;
case FF_OPT_TYPE_STRING: printf("string"); break;
case FF_OPT_TYPE_INT:
case FF_OPT_TYPE_INT64: printf("integer"); break;
case FF_OPT_TYPE_FLOAT:
case FF_OPT_TYPE_DOUBLE: printf("float"); break;
case FF_OPT_TYPE_RATIONAL: printf("rational number"); break;
case FF_OPT_TYPE_FLAGS: printf("flags"); break;
default: printf("value"); break;
}
printf("} (@emph{");
if (o->flags & AV_OPT_FLAG_ENCODING_PARAM) {
printf("input");
if (o->flags & AV_OPT_FLAG_ENCODING_PARAM)
printf("/");
}
if (o->flags & AV_OPT_FLAG_ENCODING_PARAM)
printf("output");
printf("})\n");
if (o->help)
printf("%s\n", o->help);
if (o->unit) {
const AVOption *u = NULL;
printf("\nPossible values:\n@table @samp\n");
while ((u = av_next_option(&class, u)))
if (u->type == FF_OPT_TYPE_CONST && u->unit && !strcmp(u->unit, o->unit))
printf("@item %s\n%s\n", u->name, u->help ? u->help : "");
printf("@end table\n");
}
}
static void show_opts(const AVClass *class)
{
const AVOption *o = NULL;
printf("@table @option\n");
while ((o = av_next_option(&class, o)))
if (o->type != FF_OPT_TYPE_CONST)
print_option(class, o);
printf("@end table\n");
}
static void show_format_opts(void)
{
const AVInputFormat *iformat = NULL;
const AVOutputFormat *oformat = NULL;
void *iformat_opaque = NULL;
void *oformat_opaque = NULL;
printf("@section Generic format AVOptions\n");
show_opts(avformat_get_class());
printf("@section Format-specific AVOptions\n");
while ((iformat = av_demuxer_iterate(&iformat_opaque))) {
if (!iformat->priv_class)
continue;
printf("@subsection %s AVOptions\n", iformat->priv_class->class_name);
show_opts(iformat->priv_class);
}
while ((oformat = av_muxer_iterate(&oformat_opaque))) {
if (!oformat->priv_class)
continue;
printf("@subsection %s AVOptions\n", oformat->priv_class->class_name);
show_opts(oformat->priv_class);
}
}
static void show_codec_opts(void)
{
AVCodec *c = NULL;
printf("@section Generic codec AVOptions\n");
show_opts(avcodec_get_class());
printf("@section Codec-specific AVOptions\n");
while ((c = av_codec_next(c))) {
if (!c->priv_class)
continue;
printf("@subsection %s AVOptions\n", c->priv_class->class_name);
show_opts(c->priv_class);
}
}
int main(int argc, char **argv)
{
if (argc < 2)
print_usage();
if (!strcmp(argv[1], "format"))
show_format_opts();
else if (!strcmp(argv[1], "codec"))
show_codec_opts();
else
print_usage();
return 0;
}
