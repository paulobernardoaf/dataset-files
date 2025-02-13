#if !defined(__al_included_allegro5_monitor_h)
#define __al_included_allegro5_monitor_h

#include "allegro5/base.h"

#if defined(__cplusplus)
extern "C" {
#endif




typedef struct ALLEGRO_MONITOR_INFO
{
int x1;
int y1;
int x2;
int y2;
} ALLEGRO_MONITOR_INFO;

enum {
ALLEGRO_DEFAULT_DISPLAY_ADAPTER = -1
};

AL_FUNC(int, al_get_num_video_adapters, (void));
AL_FUNC(bool, al_get_monitor_info, (int adapter, ALLEGRO_MONITOR_INFO *info));
AL_FUNC(int, al_get_monitor_dpi, (int adapter));


#if defined(__cplusplus)
}
#endif

#endif


