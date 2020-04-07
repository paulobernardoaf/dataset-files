



















#include "mem.h"
#include "spherical.h"

AVSphericalMapping *av_spherical_alloc(size_t *size)
{
AVSphericalMapping *spherical = av_mallocz(sizeof(AVSphericalMapping));
if (!spherical)
return NULL;

if (size)
*size = sizeof(*spherical);

return spherical;
}

void av_spherical_tile_bounds(const AVSphericalMapping *map,
size_t width, size_t height,
size_t *left, size_t *top,
size_t *right, size_t *bottom)
{

uint64_t orig_width = (uint64_t) width * UINT32_MAX /
(UINT32_MAX - map->bound_right - map->bound_left);
uint64_t orig_height = (uint64_t) height * UINT32_MAX /
(UINT32_MAX - map->bound_bottom - map->bound_top);


*left = (orig_width * map->bound_left + UINT32_MAX - 1) / UINT32_MAX;
*top = (orig_height * map->bound_top + UINT32_MAX - 1) / UINT32_MAX;
*right = orig_width - width - *left;
*bottom = orig_height - height - *top;
}

static const char *spherical_projection_names[] = {
[AV_SPHERICAL_EQUIRECTANGULAR] = "equirectangular",
[AV_SPHERICAL_CUBEMAP] = "cubemap",
[AV_SPHERICAL_EQUIRECTANGULAR_TILE] = "tiled equirectangular",
};

const char *av_spherical_projection_name(enum AVSphericalProjection projection)
{
if ((unsigned)projection >= FF_ARRAY_ELEMS(spherical_projection_names))
return "unknown";

return spherical_projection_names[projection];
}

int av_spherical_from_name(const char *name)
{
int i;

for (i = 0; i < FF_ARRAY_ELEMS(spherical_projection_names); i++) {
size_t len = strlen(spherical_projection_names[i]);
if (!strncmp(spherical_projection_names[i], name, len))
return i;
}

return -1;
}
