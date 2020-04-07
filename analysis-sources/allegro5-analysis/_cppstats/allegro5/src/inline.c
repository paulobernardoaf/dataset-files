#include "allegro5/platform/alplatf.h"
#if !defined(ALLEGRO_IPHONE)
#define AL_INLINE(type, name, args, code) extern type name args; type name args code
#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#if defined(ALLEGRO_INTERNAL_HEADER)
#include ALLEGRO_INTERNAL_HEADER
#endif
#include "allegro5/internal/aintern_float.h"
#include "allegro5/internal/aintern_vector.h"
#endif
