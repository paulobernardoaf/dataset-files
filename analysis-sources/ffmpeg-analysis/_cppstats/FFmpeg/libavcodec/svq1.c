#include "svq1.h"
#include "svq1_cb.h"
#include "svq1_vlc.h"
const uint16_t ff_svq1_frame_size_table[7][2] = {
{ 160, 120 }, { 128, 96 }, { 176, 144 }, { 352, 288 },
{ 704, 576 }, { 240, 180 }, { 320, 240 }
};
