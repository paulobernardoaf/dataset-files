#include <vlc_common.h>
#include <vlc_demux.h>
#include "stream.h"
demux_t *demux_NewAdvanced( vlc_object_t *p_obj, input_thread_t *p_parent_input,
const char *psz_demux, const char *url,
stream_t *s, es_out_t *out, bool );
unsigned demux_TestAndClearFlags( demux_t *, unsigned );
int demux_GetTitle( demux_t * );
int demux_GetSeekpoint( demux_t * );
demux_t *demux_FilterChainNew( demux_t *source, const char *list ) VLC_USED;
bool demux_FilterEnable( demux_t *p_demux_chain, const char* psz_demux );
bool demux_FilterDisable( demux_t *p_demux_chain, const char* psz_demux );
