



















#if !defined(AVCODEC_LIBXVID_H)
#define AVCODEC_LIBXVID_H






struct MpegEncContext;


int ff_xvid_rate_control_init(struct MpegEncContext *s);
void ff_xvid_rate_control_uninit(struct MpegEncContext *s);
float ff_xvid_rate_estimate_qscale(struct MpegEncContext *s, int dry_run);

#endif 
