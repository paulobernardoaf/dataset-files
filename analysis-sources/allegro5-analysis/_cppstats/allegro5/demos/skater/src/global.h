#define mouse_event WIN32_mouse_event
#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#undef mouse_event
#include "defines.h"
#include "keyboard.h"
#if defined(ALLEGRO_MSVC)
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif
extern int fullscreen; 
extern int bit_depth; 
extern int screen_width; 
extern int screen_height; 
extern int screen_orientation;
extern int window_width; 
extern int window_height; 
extern int screen_samples; 
extern int use_vsync; 
extern int logic_framerate; 
extern int max_frame_skip; 
extern int limit_framerate; 
extern int display_framerate; 
extern int reduce_cpu_usage; 
extern int sound_volume; 
extern int music_volume; 
extern int controller_id; 
extern int shadow_offset;
extern VCONTROLLER *controller[2];
extern char config_path[DEMO_PATH_LENGTH];
extern char data_path[DEMO_PATH_LENGTH];
#define demo_font ((ALLEGRO_FONT *)demo_data[DEMO_FONT].dat)
#define demo_font_logo ((ALLEGRO_FONT *)demo_data[DEMO_FONT_LOGO].dat)
#define plain_font demo_font
extern ALLEGRO_DISPLAY *screen;
extern const char *demo_error(int id);
extern int change_gfx_mode(void);
extern void read_global_config(const char *config);
extern void write_global_config(const char *config);
extern void unload_data(void);
extern void demo_textprintf_ex(const ALLEGRO_FONT * font, int x, int y,
ALLEGRO_COLOR col, int align, const char *format, ...);
extern void demo_textprintf(const ALLEGRO_FONT * font, int x, int y,
ALLEGRO_COLOR col, const char *format, ...);
extern void demo_textprintf_right(const ALLEGRO_FONT * font, int x, int y,
ALLEGRO_COLOR col, const char *format, ...);
extern void demo_textprintf_centre(const ALLEGRO_FONT * font, int x, int y,
ALLEGRO_COLOR col, const char *format, ...);
extern void demo_textout(const ALLEGRO_FONT *f, const char *s, int x,
int y, ALLEGRO_COLOR color);
extern void demo_textout_right(const ALLEGRO_FONT *f, const char *s,
int x, int y, ALLEGRO_COLOR color);
extern void demo_textout_centre(const ALLEGRO_FONT *f, const char *s,
int x, int y, ALLEGRO_COLOR color);
extern void shadow_textprintf(ALLEGRO_FONT * font, int x, int y,
ALLEGRO_COLOR col, int align, const char *text, ...);
typedef struct DATA_ENTRY {
int id;
char const *type;
char *path;
char *subfolder;
char *name;
char *ext;
int size;
void *dat;
} DATA_ENTRY;
extern DATA_ENTRY *demo_data;
void unload_data_entries(DATA_ENTRY *data);
int get_config_int(const ALLEGRO_CONFIG *cfg, const char *section,
const char *name, int def);
void set_config_int(ALLEGRO_CONFIG *cfg, const char *section, const char *name,
int val);
int my_stricmp(const char *s1, const char *s2);
