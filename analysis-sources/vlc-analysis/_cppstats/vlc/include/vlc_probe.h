#include <stdlib.h>
#if defined(__cplusplus)
extern "C" {
#endif
void *vlc_probe (vlc_object_t *, const char *, size_t *);
#define vlc_probe(obj, cap, pcount) vlc_probe(VLC_OBJECT(obj), cap, pcount)
struct vlc_probe_t
{
struct vlc_object_t obj;
void *list;
size_t count;
};
typedef struct vlc_probe_t vlc_probe_t;
static inline int vlc_probe_add(vlc_probe_t *obj, const void *data,
size_t len)
{
char *tab = (char *)realloc (obj->list, (obj->count + 1) * len);
if (unlikely(tab == NULL))
return VLC_ENOMEM;
memcpy(tab + (obj->count * len), data, len);
obj->list = tab;
obj->count++;
return VLC_SUCCESS;
}
#define VLC_PROBE_CONTINUE VLC_EGENERIC
#define VLC_PROBE_STOP VLC_SUCCESS
#if defined(__cplusplus)
}
#endif
