#include <vlc_common.h>
#include <vlc_stream.h>
#include "input_internal.h"
stream_t *vlc_stream_CustomNew(vlc_object_t *parent,
void (*destroy)(stream_t *), size_t extra_size,
const char *type_name);
void *vlc_stream_Private(stream_t *stream);
void stream_CommonDelete( stream_t *s );
stream_t *vlc_stream_AttachmentNew(vlc_object_t *p_this,
input_attachment_t *attachement);
stream_t *stream_AccessNew(vlc_object_t *, input_thread_t *, es_out_t *, bool,
const char *);
stream_t *stream_FilterAutoNew( stream_t *source ) VLC_USED;
stream_t *stream_FilterChainNew( stream_t *source, const char *list ) VLC_USED;
int stream_extractor_AttachParsed( stream_t** stream, const char* psz_data,
char const** out_extra );
char *get_path(const char *location);
