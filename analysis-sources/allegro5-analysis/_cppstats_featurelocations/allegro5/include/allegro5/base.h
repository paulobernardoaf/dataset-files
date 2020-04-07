

















#if !defined(__al_included_allegro5_base_h)
#define __al_included_allegro5_base_h

#if !defined(ALLEGRO_NO_STD_HEADERS)
#include <errno.h>
#if defined(_MSC_VER)



#define _POSIX_
#include <limits.h>
#undef _POSIX_
#else
#include <limits.h>
#endif
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#endif

#if (defined DEBUGMODE) && (defined FORTIFY)
#include <fortify/fortify.h>
#endif

#if (defined DEBUGMODE) && (defined DMALLOC)
#include <dmalloc.h>
#endif

#include "allegro5/internal/alconfig.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define ALLEGRO_VERSION 5
#define ALLEGRO_SUB_VERSION 2
#define ALLEGRO_WIP_VERSION 7

#if defined(ALLEGRO_UNSTABLE)
#define ALLEGRO_UNSTABLE_BIT 1 << 31
#else
#define ALLEGRO_UNSTABLE_BIT 0
#endif









#define ALLEGRO_RELEASE_NUMBER 0

#define ALLEGRO_VERSION_STR "5.2.7 (GIT)"
#define ALLEGRO_DATE_STR "2020"
#define ALLEGRO_DATE 20200207 
#define ALLEGRO_VERSION_INT ((ALLEGRO_VERSION << 24) | (ALLEGRO_SUB_VERSION << 16) | (ALLEGRO_WIP_VERSION << 8) | ALLEGRO_RELEASE_NUMBER | ALLEGRO_UNSTABLE_BIT)




AL_FUNC(uint32_t, al_get_allegro_version, (void));
AL_FUNC(int, al_run_main, (int argc, char **argv, int (*)(int, char **)));







#define ALLEGRO_PI 3.14159265358979323846

#define AL_ID(a,b,c,d) (((a)<<24) | ((b)<<16) | ((c)<<8) | (d))



#if defined(__cplusplus)
}
#endif

#endif
