



















#include "avformat.h"
#include "metadata.h"
#include "libavutil/dict.h"
#include "libavutil/avstring.h"

void ff_metadata_conv(AVDictionary **pm, const AVMetadataConv *d_conv,
const AVMetadataConv *s_conv)
{


const AVMetadataConv *sc, *dc;
AVDictionaryEntry *mtag = NULL;
AVDictionary *dst = NULL;
const char *key;

if (d_conv == s_conv || !pm)
return;

while ((mtag = av_dict_get(*pm, "", mtag, AV_DICT_IGNORE_SUFFIX))) {
key = mtag->key;
if (s_conv)
for (sc=s_conv; sc->native; sc++)
if (!av_strcasecmp(key, sc->native)) {
key = sc->generic;
break;
}
if (d_conv)
for (dc=d_conv; dc->native; dc++)
if (!av_strcasecmp(key, dc->generic)) {
key = dc->native;
break;
}
av_dict_set(&dst, key, mtag->value, 0);
}
av_dict_free(pm);
*pm = dst;
}

void ff_metadata_conv_ctx(AVFormatContext *ctx, const AVMetadataConv *d_conv,
const AVMetadataConv *s_conv)
{
int i;
ff_metadata_conv(&ctx->metadata, d_conv, s_conv);
for (i=0; i<ctx->nb_streams ; i++)
ff_metadata_conv(&ctx->streams [i]->metadata, d_conv, s_conv);
for (i=0; i<ctx->nb_chapters; i++)
ff_metadata_conv(&ctx->chapters[i]->metadata, d_conv, s_conv);
for (i=0; i<ctx->nb_programs; i++)
ff_metadata_conv(&ctx->programs[i]->metadata, d_conv, s_conv);
}
