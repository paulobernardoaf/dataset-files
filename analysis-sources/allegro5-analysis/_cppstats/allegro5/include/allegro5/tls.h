#include "allegro5/base.h"
#if defined(__cplusplus)
extern "C" {
#endif
typedef enum ALLEGRO_STATE_FLAGS
{
ALLEGRO_STATE_NEW_DISPLAY_PARAMETERS = 0x0001,
ALLEGRO_STATE_NEW_BITMAP_PARAMETERS = 0x0002,
ALLEGRO_STATE_DISPLAY = 0x0004,
ALLEGRO_STATE_TARGET_BITMAP = 0x0008,
ALLEGRO_STATE_BLENDER = 0x0010,
ALLEGRO_STATE_NEW_FILE_INTERFACE = 0x0020,
ALLEGRO_STATE_TRANSFORM = 0x0040,
ALLEGRO_STATE_PROJECTION_TRANSFORM = 0x0100,
ALLEGRO_STATE_BITMAP = ALLEGRO_STATE_TARGET_BITMAP +\
ALLEGRO_STATE_NEW_BITMAP_PARAMETERS,
ALLEGRO_STATE_ALL = 0xffff
} ALLEGRO_STATE_FLAGS;
typedef struct ALLEGRO_STATE ALLEGRO_STATE;
struct ALLEGRO_STATE
{
char _tls[1024];
};
AL_FUNC(void, al_store_state, (ALLEGRO_STATE *state, int flags));
AL_FUNC(void, al_restore_state, (ALLEGRO_STATE const *state));
#if defined(__cplusplus)
}
#endif
