

















#if !defined(__al_included_allegro5_fmaths_h)
#define __al_included_allegro5_fmaths_h

#include "allegro5/base.h"
#include "allegro5/fixed.h"

#if defined(__cplusplus)
extern "C" {
#endif

AL_FUNC(al_fixed, al_fixsqrt, (al_fixed x));
AL_FUNC(al_fixed, al_fixhypot, (al_fixed x, al_fixed y));
AL_FUNC(al_fixed, al_fixatan, (al_fixed x));
AL_FUNC(al_fixed, al_fixatan2, (al_fixed y, al_fixed x));

AL_ARRAY(al_fixed, _al_fix_cos_tbl);
AL_ARRAY(al_fixed, _al_fix_tan_tbl);
AL_ARRAY(al_fixed, _al_fix_acos_tbl);

#if defined(__cplusplus)
}
#endif

#include "allegro5/inline/fmaths.inl"

#endif


