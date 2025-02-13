#include "allegro5/allegro.h"
#if defined(__cplusplus)
extern "C" {
#endif
#if (defined ALLEGRO_MINGW32) || (defined ALLEGRO_MSVC) || (defined ALLEGRO_BCC32)
#if !defined(ALLEGRO_STATICLINK)
#if defined(ALLEGRO_MEMFILE_SRC)
#define _ALLEGRO_MEMFILE_DLL __declspec(dllexport)
#else
#define _ALLEGRO_MEMFILE_DLL __declspec(dllimport)
#endif
#else
#define _ALLEGRO_MEMFILE_DLL
#endif
#endif
#if defined ALLEGRO_MSVC
#define ALLEGRO_MEMFILE_FUNC(type, name, args) _ALLEGRO_MEMFILE_DLL type __cdecl name args
#elif defined ALLEGRO_MINGW32
#define ALLEGRO_MEMFILE_FUNC(type, name, args) extern type name args
#elif defined ALLEGRO_BCC32
#define ALLEGRO_MEMFILE_FUNC(type, name, args) extern _ALLEGRO_MEMFILE_DLL type name args
#else
#define ALLEGRO_MEMFILE_FUNC AL_FUNC
#endif
ALLEGRO_MEMFILE_FUNC(ALLEGRO_FILE *, al_open_memfile, (void *mem, int64_t size, const char *mode));
ALLEGRO_MEMFILE_FUNC(uint32_t, al_get_allegro_memfile_version, (void));
#if defined(__cplusplus)
}
#endif
