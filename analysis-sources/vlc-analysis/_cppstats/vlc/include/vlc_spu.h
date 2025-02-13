#include <vlc_subpicture.h>
#if defined(__cplusplus)
extern "C" {
#endif
typedef struct spu_private_t spu_private_t;
struct spu_t
{
struct vlc_object_t obj;
spu_private_t *p;
};
VLC_API spu_t * spu_Create( vlc_object_t *, vout_thread_t * );
#define spu_Create(a,b) spu_Create(VLC_OBJECT(a),b)
VLC_API void spu_Destroy( spu_t * );
VLC_API void spu_PutSubpicture( spu_t *, subpicture_t * );
VLC_API subpicture_t * spu_Render( spu_t *, const vlc_fourcc_t *p_chroma_list,
const video_format_t *p_fmt_dst, const video_format_t *p_fmt_src,
vlc_tick_t system_now, vlc_tick_t pts,
bool ignore_osd, bool external_scale );
VLC_API ssize_t spu_RegisterChannel( spu_t * );
VLC_API void spu_UnregisterChannel( spu_t *, size_t );
VLC_API void spu_ClearChannel( spu_t *, size_t );
VLC_API void spu_ChangeSources( spu_t *, const char * );
VLC_API void spu_ChangeFilters( spu_t *, const char * );
#if defined(__cplusplus)
}
#endif
