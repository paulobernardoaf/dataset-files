#include "config.h"
#include "libavutil/thread.h"
#include "libavformat/internal.h"
#include "avdevice.h"
extern AVInputFormat ff_alsa_demuxer;
extern AVOutputFormat ff_alsa_muxer;
extern AVInputFormat ff_android_camera_demuxer;
extern AVInputFormat ff_avfoundation_demuxer;
extern AVInputFormat ff_bktr_demuxer;
extern AVOutputFormat ff_caca_muxer;
extern AVInputFormat ff_decklink_demuxer;
extern AVOutputFormat ff_decklink_muxer;
extern AVInputFormat ff_dshow_demuxer;
extern AVInputFormat ff_fbdev_demuxer;
extern AVOutputFormat ff_fbdev_muxer;
extern AVInputFormat ff_gdigrab_demuxer;
extern AVInputFormat ff_iec61883_demuxer;
extern AVInputFormat ff_jack_demuxer;
extern AVInputFormat ff_kmsgrab_demuxer;
extern AVInputFormat ff_lavfi_demuxer;
extern AVInputFormat ff_openal_demuxer;
extern AVOutputFormat ff_opengl_muxer;
extern AVInputFormat ff_oss_demuxer;
extern AVOutputFormat ff_oss_muxer;
extern AVInputFormat ff_pulse_demuxer;
extern AVOutputFormat ff_pulse_muxer;
extern AVOutputFormat ff_sdl2_muxer;
extern AVInputFormat ff_sndio_demuxer;
extern AVOutputFormat ff_sndio_muxer;
extern AVInputFormat ff_v4l2_demuxer;
extern AVOutputFormat ff_v4l2_muxer;
extern AVInputFormat ff_vfwcap_demuxer;
extern AVInputFormat ff_xcbgrab_demuxer;
extern AVOutputFormat ff_xv_muxer;
extern AVInputFormat ff_libcdio_demuxer;
extern AVInputFormat ff_libdc1394_demuxer;
#include "libavdevice/outdev_list.c"
#include "libavdevice/indev_list.c"
void avdevice_register_all(void)
{
avpriv_register_devices(outdev_list, indev_list);
}
