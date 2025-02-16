





#if !defined(__DEMO_FPS_H__)
#define __DEMO_FPS_H__

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#if defined(__cplusplus)
extern "C" {
#endif






typedef struct FPS {
int *samples;
int nSamples;
int curSample;
int frameCounter;
int framesPerSecond;
} FPS;












FPS *create_fps(int fps);











void destroy_fps(FPS * fps);













void fps_tick(FPS * fps);











void fps_frame(FPS * fps);











int get_fps(FPS * fps);





















void draw_fps(FPS * fps, ALLEGRO_FONT * font, int x, int y,
ALLEGRO_COLOR color, char *format);


#if defined(__cplusplus)
}
#endif
#endif
