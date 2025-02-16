#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "libavutil/avstring.h"
#include "libavutil/dict.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "libavutil/mem.h"
#include "libavutil/replaygain.h"
#include "avformat.h"
#include "internal.h"
#include "replaygain.h"
static int32_t parse_value(const char *value, int32_t min)
{
char *fraction;
int scale = 10000;
int32_t mb = 0;
int sign = 1;
int db;
if (!value)
return min;
value += strspn(value, " \t");
if (*value == '-')
sign = -1;
db = strtol(value, &fraction, 0);
if (*fraction++ == '.') {
while (av_isdigit(*fraction) && scale) {
mb += scale * (*fraction - '0');
scale /= 10;
fraction++;
}
}
if (abs(db) > (INT32_MAX - mb) / 100000)
return min;
return db * 100000 + sign * mb;
}
int ff_replaygain_export_raw(AVStream *st, int32_t tg, uint32_t tp,
int32_t ag, uint32_t ap)
{
AVReplayGain *replaygain;
if (tg == INT32_MIN && ag == INT32_MIN)
return 0;
replaygain = (AVReplayGain*)av_stream_new_side_data(st, AV_PKT_DATA_REPLAYGAIN,
sizeof(*replaygain));
if (!replaygain)
return AVERROR(ENOMEM);
replaygain->track_gain = tg;
replaygain->track_peak = tp;
replaygain->album_gain = ag;
replaygain->album_peak = ap;
return 0;
}
int ff_replaygain_export(AVStream *st, AVDictionary *metadata)
{
const AVDictionaryEntry *tg, *tp, *ag, *ap;
tg = av_dict_get(metadata, "REPLAYGAIN_TRACK_GAIN", NULL, 0);
tp = av_dict_get(metadata, "REPLAYGAIN_TRACK_PEAK", NULL, 0);
ag = av_dict_get(metadata, "REPLAYGAIN_ALBUM_GAIN", NULL, 0);
ap = av_dict_get(metadata, "REPLAYGAIN_ALBUM_PEAK", NULL, 0);
return ff_replaygain_export_raw(st,
parse_value(tg ? tg->value : NULL, INT32_MIN),
parse_value(tp ? tp->value : NULL, 0),
parse_value(ag ? ag->value : NULL, INT32_MIN),
parse_value(ap ? ap->value : NULL, 0));
}
