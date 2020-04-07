
















#if !defined(__al_included_allegro5_astdbool_h)
#define __al_included_allegro5_astdbool_h

#if !defined(__cplusplus)
#if defined(ALLEGRO_HAVE_STDBOOL_H)
#include <stdbool.h>
#else
#if !defined(ALLEGRO_HAVE__BOOL)
typedef unsigned char _Bool;
#endif
#define bool _Bool
#define false 0
#define true 1
#define __bool_true_false_are_defined 1
#endif
#endif

#endif
