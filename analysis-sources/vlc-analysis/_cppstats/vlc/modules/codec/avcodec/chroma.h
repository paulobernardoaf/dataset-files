int FindFfmpegChroma( vlc_fourcc_t );
int GetFfmpegChroma( int *i_ffmpeg_chroma, const video_format_t *fmt );
vlc_fourcc_t FindVlcChroma( int );
int GetVlcChroma( video_format_t *fmt, int i_ffmpeg_chroma );
