



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdbool.h>
#undef NDEBUG
#include <assert.h>

#include <vlc_common.h>
#include <vlc_es.h>
#include <vlc_picture_pool.h>

#define PICTURES 10

const char vlc_module_name[] = "test_picture_pool";

static video_format_t fmt;
static picture_pool_t *pool, *reserve;

static void test(bool zombie)
{
picture_t *pics[PICTURES];

pool = picture_pool_NewFromFormat(&fmt, PICTURES);
assert(pool != NULL);

for (unsigned i = 0; i < PICTURES; i++) {
pics[i] = picture_pool_Get(pool);
assert(pics[i] != NULL);
}

for (unsigned i = 0; i < PICTURES; i++)
assert(picture_pool_Get(pool) == NULL);




for (unsigned i = 0; i < PICTURES / 2; i++)
picture_Hold(pics[i]);

for (unsigned i = 0; i < PICTURES / 2; i++)
picture_Release(pics[i]);

for (unsigned i = 0; i < PICTURES; i++) {
void *plane = pics[i]->p[0].p_pixels;
assert(plane != NULL);
picture_Release(pics[i]);

pics[i] = picture_pool_Get(pool);
assert(pics[i] != NULL);
assert(pics[i]->p[0].p_pixels == plane);
}

for (unsigned i = 0; i < PICTURES; i++)
picture_Release(pics[i]);

for (unsigned i = 0; i < PICTURES; i++) {
pics[i] = picture_pool_Wait(pool);
assert(pics[i] != NULL);
}

for (unsigned i = 0; i < PICTURES; i++)
picture_Release(pics[i]);

reserve = picture_pool_Reserve(pool, PICTURES / 2);
assert(reserve != NULL);

for (unsigned i = 0; i < PICTURES / 2; i++) {
pics[i] = picture_pool_Get(pool);
assert(pics[i] != NULL);
}

for (unsigned i = PICTURES / 2; i < PICTURES; i++) {
assert(picture_pool_Get(pool) == NULL);
pics[i] = picture_pool_Get(reserve);
assert(pics[i] != NULL);
}

if (!zombie)
for (unsigned i = 0; i < PICTURES; i++)
picture_Release(pics[i]);

picture_pool_Release(reserve);
picture_pool_Release(pool);

if (zombie)
for (unsigned i = 0; i < PICTURES; i++)
picture_Release(pics[i]);
}

int main(void)
{
video_format_Setup(&fmt, VLC_CODEC_I420, 320, 200, 320, 200, 1, 1);

pool = picture_pool_NewFromFormat(&fmt, PICTURES);
assert(pool != NULL);
assert(picture_pool_GetSize(pool) == PICTURES);

reserve = picture_pool_Reserve(pool, PICTURES / 2);
assert(reserve != NULL);

picture_pool_Release(reserve);
picture_pool_Release(pool);

test(false);
test(true);

return 0;
}
