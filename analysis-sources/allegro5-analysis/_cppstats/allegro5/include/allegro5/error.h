#include "allegro5/base.h"
#if defined(__cplusplus)
extern "C" {
#endif
AL_FUNC(int, al_get_errno, (void));
AL_FUNC(void, al_set_errno, (int errnum));
#if defined(__cplusplus)
}
#endif
