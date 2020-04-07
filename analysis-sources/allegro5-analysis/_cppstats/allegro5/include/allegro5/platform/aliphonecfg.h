#include <fcntl.h>
#include <unistd.h>
#define ALLEGRO_PLATFORM_STR "IPHONE"
#define ALLEGRO_EXTRA_HEADER "allegro5/platform/aliphone.h"
#define ALLEGRO_INTERNAL_HEADER "allegro5/platform/aintiphone.h"
#define ALLEGRO_INTERNAL_THREAD_HEADER "allegro5/platform/aintuthr.h"
#define ALLEGRO_EXCLUDE_GLX
#if !defined(AL_INLINE)
#define AL_INLINE(type, name, args, code) static __inline__ type name args; static __inline__ type name args code
#endif
