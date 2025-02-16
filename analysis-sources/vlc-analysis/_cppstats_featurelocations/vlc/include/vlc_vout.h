























#if !defined(VLC_VOUT_H_)
#define VLC_VOUT_H_ 1

#include <vlc_es.h>
#include <vlc_picture.h>
#include <vlc_subpicture.h>



















typedef struct vout_thread_sys_t vout_thread_sys_t;








struct vout_thread_t {
struct vlc_object_t obj;


vout_thread_sys_t *p;
};


#define VOUT_ALIGN_LEFT 0x0001
#define VOUT_ALIGN_RIGHT 0x0002
#define VOUT_ALIGN_HMASK 0x0003
#define VOUT_ALIGN_TOP 0x0004
#define VOUT_ALIGN_BOTTOM 0x0008
#define VOUT_ALIGN_VMASK 0x000C




enum vlc_vout_order
{
VLC_VOUT_ORDER_NONE,





VLC_VOUT_ORDER_PRIMARY,







VLC_VOUT_ORDER_SECONDARY,
};












VLC_API void vout_Close(vout_thread_t *vout);
















VLC_API int vout_GetSnapshot( vout_thread_t *p_vout,
block_t **pp_image, picture_t **pp_picture,
video_format_t *p_fmt,
const char *psz_format, vlc_tick_t i_timeout );


VLC_API picture_t * vout_GetPicture( vout_thread_t * );
VLC_API void vout_PutPicture( vout_thread_t *, picture_t * );


#define VOUT_SPU_CHANNEL_INVALID (-1) 
#define VOUT_SPU_CHANNEL_OSD 0 
#define VOUT_SPU_CHANNEL_OSD_HSLIDER 1
#define VOUT_SPU_CHANNEL_OSD_VSLIDER 2
#define VOUT_SPU_CHANNEL_OSD_COUNT 3


VLC_API void vout_PutSubpicture( vout_thread_t *, subpicture_t * );
VLC_API ssize_t vout_RegisterSubpictureChannel( vout_thread_t * );
VLC_API void vout_UnregisterSubpictureChannel( vout_thread_t *, size_t );
VLC_API void vout_FlushSubpictureChannel( vout_thread_t *, size_t );




VLC_API void vout_Flush( vout_thread_t *p_vout, vlc_tick_t i_date );




#define vout_FlushAll( vout ) vout_Flush( vout, VLC_TICK_INVALID )



#endif 
