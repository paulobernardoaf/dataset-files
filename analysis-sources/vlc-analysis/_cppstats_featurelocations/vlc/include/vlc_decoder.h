






















#if !defined(VLC_DECODER_H)
#define VLC_DECODER_H 1






#include <vlc_subpicture.h>

typedef struct vlc_input_decoder_t vlc_input_decoder_t;




typedef struct input_resource_t input_resource_t;


VLC_API vlc_input_decoder_t *
vlc_input_decoder_Create( vlc_object_t *, const es_format_t *, input_resource_t * ) VLC_USED;
VLC_API void vlc_input_decoder_Delete( vlc_input_decoder_t * );
VLC_API void vlc_input_decoder_Decode( vlc_input_decoder_t *, block_t *, bool b_do_pace );
VLC_API void vlc_input_decoder_Drain( vlc_input_decoder_t * );
VLC_API void vlc_input_decoder_Flush( vlc_input_decoder_t * );
VLC_API int vlc_input_decoder_SetSpuHighlight( vlc_input_decoder_t *, const vlc_spu_highlight_t * );







VLC_API input_resource_t * input_resource_New( vlc_object_t * ) VLC_USED;




VLC_API void input_resource_Release( input_resource_t * );





VLC_API audio_output_t *input_resource_HoldAout( input_resource_t * );




VLC_API audio_output_t *input_resource_GetAout( input_resource_t * );




VLC_API void input_resource_PutAout( input_resource_t *, audio_output_t * );


#endif
