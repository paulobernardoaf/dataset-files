#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <malloc.h>
#include "allegro5/platform/alplatf.h"
#if defined(ALLEGRO_STATICLINK)
#define ALLEGRO_PLATFORM_STR "MinGW32.s"
#else
#define ALLEGRO_PLATFORM_STR "MinGW32"
#endif
#define ALLEGRO_WINDOWS
#define ALLEGRO_I386
#define ALLEGRO_LITTLE_ENDIAN
#if defined(ALLEGRO_USE_CONSOLE)
#define ALLEGRO_NO_MAGIC_MAIN
#endif
#if (defined ALLEGRO_STATICLINK) || (defined ALLEGRO_SRC)
#define _AL_DLL
#else
#define _AL_DLL __declspec(dllimport)
#endif
#define AL_VAR(type, name) extern _AL_DLL type name
#define AL_ARRAY(type, name) extern _AL_DLL type name[]
#define AL_FUNC(type, name, args) extern type name args
#define AL_METHOD(type, name, args) type (*name) args
#define AL_FUNCPTR(type, name, args) extern _AL_DLL type (*name) args
#if (defined ALLEGRO_SRC)
#if defined(WINNT)
#undef WINNT
#endif
#if !defined(HMONITOR_DECLARED)
#define HMONITOR_DECLARED 1
#endif
#endif 
#if !defined(ENUM_CURRENT_SETTINGS)
#define ENUM_CURRENT_SETTINGS ((DWORD)-1)
#endif
#define ALLEGRO_EXTRA_HEADER "allegro5/platform/alwin.h"
#define ALLEGRO_INTERNAL_HEADER "allegro5/platform/aintwin.h"
#define ALLEGRO_INTERNAL_THREAD_HEADER "allegro5/platform/aintwthr.h"
