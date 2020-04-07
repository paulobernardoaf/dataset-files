inline static char * peek_Readline( stream_t *p_demuxstream, uint64_t *pi_offset )
{
uint8_t *p_peek;
ssize_t i_peek = vlc_stream_Peek( p_demuxstream, (const uint8_t **) &p_peek,
*pi_offset + 2048 );
if( i_peek < 0 || (uint64_t) i_peek < *pi_offset )
return NULL;
const uint64_t i_bufsize = (uint64_t) i_peek - *pi_offset;
char *psz_line = NULL;
stream_t *p_memorystream = vlc_stream_MemoryNew( p_demuxstream,
&p_peek[*pi_offset],
i_bufsize, true );
if( p_memorystream )
{
psz_line = vlc_stream_ReadLine( p_memorystream );
*pi_offset += vlc_stream_Tell( p_memorystream );
vlc_stream_Delete( p_memorystream );
}
return psz_line;
}
