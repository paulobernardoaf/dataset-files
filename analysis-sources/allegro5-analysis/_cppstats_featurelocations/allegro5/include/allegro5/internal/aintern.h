

















#if !defined(__al_included_allegro5_aintern_h)
#define __al_included_allegro5_aintern_h

#if !defined(__al_included_allegro5_allegro_h)
#error must include allegro5/allegro.h first
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#define _ALLEGRO_MIN(x,y) (((x) < (y)) ? (x) : (y))
#define _ALLEGRO_MAX(x,y) (((x) > (y)) ? (x) : (y))
#define _ALLEGRO_CLAMP(x,y,z) _ALLEGRO_MAX((x), _ALLEGRO_MIN((y), (z)))

int _al_get_least_multiple(int val, int mul);


#define ALLEGRO_MESSAGE_SIZE 4096


AL_FUNC(void *, _al_sane_realloc, (void *ptr, size_t size));
AL_FUNC(char *, _al_sane_strncpy, (char *dest, const char *src, size_t n));


#define _AL_RAND_MAX 0xFFFF
AL_FUNC(void, _al_srand, (int seed));
AL_FUNC(int, _al_rand, (void));

AL_FUNC(int, _al_stricmp, (const char *s1, const char *s2));

#define _AL_INCHES_PER_MM 0.039370

#if defined(__cplusplus)
}
#endif

#endif
