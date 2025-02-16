














#if !defined(__al_included_allegro5_timer_h)
#define __al_included_allegro5_timer_h

#include "allegro5/base.h"
#include "allegro5/events.h"

#if defined(__cplusplus)
extern "C" {
#endif




#define ALLEGRO_USECS_TO_SECS(x) ((x) / 1000000.0)



#define ALLEGRO_MSECS_TO_SECS(x) ((x) / 1000.0)



#define ALLEGRO_BPS_TO_SECS(x) (1.0 / (x))



#define ALLEGRO_BPM_TO_SECS(x) (60.0 / (x))




typedef struct ALLEGRO_TIMER ALLEGRO_TIMER;


AL_FUNC(ALLEGRO_TIMER*, al_create_timer, (double speed_secs));
AL_FUNC(void, al_destroy_timer, (ALLEGRO_TIMER *timer));
AL_FUNC(void, al_start_timer, (ALLEGRO_TIMER *timer));
AL_FUNC(void, al_stop_timer, (ALLEGRO_TIMER *timer));
AL_FUNC(void, al_resume_timer, (ALLEGRO_TIMER *timer));
AL_FUNC(bool, al_get_timer_started, (const ALLEGRO_TIMER *timer));
AL_FUNC(double, al_get_timer_speed, (const ALLEGRO_TIMER *timer));
AL_FUNC(void, al_set_timer_speed, (ALLEGRO_TIMER *timer, double speed_secs));
AL_FUNC(int64_t, al_get_timer_count, (const ALLEGRO_TIMER *timer));
AL_FUNC(void, al_set_timer_count, (ALLEGRO_TIMER *timer, int64_t count));
AL_FUNC(void, al_add_timer_count, (ALLEGRO_TIMER *timer, int64_t diff));
AL_FUNC(ALLEGRO_EVENT_SOURCE *, al_get_timer_event_source, (ALLEGRO_TIMER *timer));


#if defined(__cplusplus)
}
#endif

#endif
