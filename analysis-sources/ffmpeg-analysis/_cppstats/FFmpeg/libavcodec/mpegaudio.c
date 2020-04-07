#include "mpegaudio.h"
int ff_mpa_l2_select_table(int bitrate, int nb_channels, int freq, int lsf)
{
int ch_bitrate, table;
ch_bitrate = bitrate / nb_channels;
if (!lsf) {
if ((freq == 48000 && ch_bitrate >= 56) ||
(ch_bitrate >= 56 && ch_bitrate <= 80))
table = 0;
else if (freq != 48000 && ch_bitrate >= 96)
table = 1;
else if (freq != 32000 && ch_bitrate <= 48)
table = 2;
else
table = 3;
} else {
table = 4;
}
return table;
}
