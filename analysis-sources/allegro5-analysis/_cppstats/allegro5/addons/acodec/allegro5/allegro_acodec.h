#include "allegro5/allegro.h"
#include "allegro5/allegro_audio.h"
#if defined(__cplusplus)
extern "C" {
#endif
#if (defined ALLEGRO_MINGW32) || (defined ALLEGRO_MSVC)
#if !defined(ALLEGRO_STATICLINK)
#if defined(ALLEGRO_ACODEC_SRC)
#define _ALLEGRO_ACODEC_DLL __declspec(dllexport)
#else
#define _ALLEGRO_ACODEC_DLL __declspec(dllimport)
#endif
#else
#define _ALLEGRO_ACODEC_DLL
#endif
#endif
#if defined ALLEGRO_MSVC
#define ALLEGRO_ACODEC_FUNC(type, name, args) _ALLEGRO_ACODEC_DLL type __cdecl name args
#elif defined ALLEGRO_MINGW32
#define ALLEGRO_ACODEC_FUNC(type, name, args) extern type name args
#else
#define ALLEGRO_ACODEC_FUNC AL_FUNC
#endif
ALLEGRO_ACODEC_FUNC(bool, al_init_acodec_addon, (void));
ALLEGRO_ACODEC_FUNC(bool, al_is_acodec_addon_initialized, (void));
ALLEGRO_ACODEC_FUNC(uint32_t, al_get_allegro_acodec_version, (void));
#if defined(__cplusplus)
}
#endif
