



















#include "config.h"
#if HAVE_UNISTD_H
#include <unistd.h> 
#endif

#include "libavutil/pixdesc.h"
#include "libavcodec/avcodec.h"
#include "libavutil/common.h"
#include "libavcodec/raw.h"

#undef printf
#undef fprintf

#if !HAVE_GETOPT
#include "compat/getopt.c"
#endif

static void usage(void)
{
printf("Show the relationships between rawvideo pixel formats and FourCC tags.\n");
printf("usage: fourcc2pixfmt [OPTIONS]\n");
printf("\n"
"Options:\n"
"-l list the pixel format for each fourcc\n"
"-L list the fourccs for each pixel format\n"
"-p PIX_FMT given a pixel format, print the list of associated fourccs (one per line)\n"
"-h print this help\n");
}

static void print_pix_fmt_fourccs(enum AVPixelFormat pix_fmt, const PixelFormatTag *pix_fmt_tags, char sep)
{
int i;

for (i = 0; pix_fmt_tags[i].pix_fmt != AV_PIX_FMT_NONE; i++)
if (pix_fmt_tags[i].pix_fmt == pix_fmt)
printf("%s%c", av_fourcc2str(pix_fmt_tags[i].fourcc), sep);
}

int main(int argc, char **argv)
{
int i, list_fourcc_pix_fmt = 0, list_pix_fmt_fourccs = 0;
const PixelFormatTag *pix_fmt_tags = avpriv_get_raw_pix_fmt_tags();
const char *pix_fmt_name = NULL;
char c;

if (argc == 1) {
usage();
return 0;
}

while ((c = getopt(argc, argv, "hp:lL")) != -1) {
switch (c) {
case 'h':
usage();
return 0;
case 'l':
list_fourcc_pix_fmt = 1;
break;
case 'L':
list_pix_fmt_fourccs = 1;
break;
case 'p':
pix_fmt_name = optarg;
break;
case '?':
usage();
return 1;
}
}

if (list_fourcc_pix_fmt)
for (i = 0; pix_fmt_tags[i].pix_fmt != AV_PIX_FMT_NONE; i++)
printf("%s: %s\n", av_fourcc2str(pix_fmt_tags[i].fourcc),
av_get_pix_fmt_name(pix_fmt_tags[i].pix_fmt));

if (list_pix_fmt_fourccs) {
for (i = 0; av_pix_fmt_desc_get(i); i++) {
const AVPixFmtDescriptor *pix_desc = av_pix_fmt_desc_get(i);
if (!pix_desc->name || pix_desc->flags & AV_PIX_FMT_FLAG_HWACCEL)
continue;
printf("%s: ", pix_desc->name);
print_pix_fmt_fourccs(i, pix_fmt_tags, ' ');
printf("\n");
}
}

if (pix_fmt_name) {
enum AVPixelFormat pix_fmt = av_get_pix_fmt(pix_fmt_name);
if (pix_fmt == AV_PIX_FMT_NONE) {
fprintf(stderr, "Invalid pixel format selected '%s'\n", pix_fmt_name);
return 1;
}
print_pix_fmt_fourccs(pix_fmt, pix_fmt_tags, '\n');
}

return 0;
}
