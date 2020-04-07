






















#if !defined(VLC_AOUT_MIXER_H)
#define VLC_AOUT_MIXER_H 1

#if defined(__cplusplus)
extern "C" {
#endif









typedef struct audio_volume audio_volume_t;




struct audio_volume
{
struct vlc_object_t obj;

vlc_fourcc_t format; 
void (*amplify)(audio_volume_t *, block_t *, float); 
};



#if defined(__cplusplus)
}
#endif

#endif
