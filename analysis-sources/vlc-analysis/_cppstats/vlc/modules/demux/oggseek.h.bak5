




























#if defined(OGG_SEEK_DEBUG)
#define OggDebug(code) code
#define OggNoDebug(code)
#else
#define OggDebug(code)
#define OggNoDebug(code) code
#endif

#define PAGE_HEADER_BYTES 27

#define OGGSEEK_BYTES_TO_READ 8500







struct oggseek_index_entry
{
demux_index_entry_t *p_next;
demux_index_entry_t *p_prev;


vlc_tick_t i_value;
int64_t i_pagepos;


int64_t i_pagepos_end;
};

int Oggseek_BlindSeektoAbsoluteTime ( demux_t *, logical_stream_t *, vlc_tick_t, bool );
int Oggseek_BlindSeektoPosition ( demux_t *, logical_stream_t *, double f, bool );
int Oggseek_SeektoAbsolutetime ( demux_t *, logical_stream_t *, vlc_tick_t );
const demux_index_entry_t *OggSeek_IndexAdd ( logical_stream_t *, vlc_tick_t, int64_t );
void Oggseek_ProbeEnd( demux_t * );

void oggseek_index_entries_free ( demux_index_entry_t * );

int64_t oggseek_read_page ( demux_t * );
