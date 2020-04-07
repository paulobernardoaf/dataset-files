#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_demux.h>
#include "ts_streams.h"
#include "ts_pid.h"
#include "ts_streams_private.h"
#include "ts_pes.h"
#include <assert.h>
static bool block_Split( block_t **pp_block, block_t **pp_remain, size_t i_offset )
{
block_t *p_block = *pp_block;
block_t *p_split = NULL;
*pp_remain = NULL;
size_t i_tocopy = p_block->i_buffer - i_offset;
if( i_tocopy > i_offset ) 
{
if( i_offset > 0 )
{
p_split = block_Alloc( i_offset );
if( p_split == NULL )
return false;
memcpy( p_split->p_buffer, p_block->p_buffer, i_offset );
p_block->p_buffer += i_offset;
p_block->i_buffer -= i_offset;
}
*pp_remain = p_block;
*pp_block = p_split;
}
else 
{
if( i_tocopy > 0 )
{
p_split = block_Alloc( i_tocopy );
if( p_split == NULL )
return false;
memcpy( p_split->p_buffer, &p_block->p_buffer[i_offset], i_tocopy );
p_block->i_buffer -= i_tocopy;
}
*pp_remain = p_split;
}
return true;
}
static const uint8_t pes_sync[] = { 0, 0, 1 };
static bool MayHaveStartCodeOnEnd( const uint8_t *p_buf, size_t i_buf )
{
assert(i_buf > 2);
return !( *(--p_buf) > 1 || *(--p_buf) > 0 || *(--p_buf) > 0 );
}
static uint8_t *FindNextPESHeader( uint8_t *p_buf, size_t i_buffer )
{
const uint8_t *p_end = &p_buf[i_buffer];
unsigned i_bitflow = 0;
for( ; p_buf != p_end; p_buf++ )
{
i_bitflow <<= 1;
if( !*p_buf )
{
i_bitflow |= 1;
}
else if( *p_buf == 0x01 && (i_bitflow & 0x06) == 0x06 ) 
{
return p_buf - 2;
}
}
return NULL;
}
static bool ts_pes_Push( ts_pes_parse_callback *cb,
ts_stream_t *p_pes, block_t *p_pkt, bool b_unit_start )
{
bool b_ret = false;
if ( b_unit_start && p_pes->gather.p_data )
{
block_t *p_datachain = p_pes->gather.p_data;
p_pes->gather.p_data = NULL;
p_pes->gather.i_data_size = 0;
p_pes->gather.i_gathered = 0;
p_pes->gather.pp_last = &p_pes->gather.p_data;
cb->pf_parse( cb->p_obj, cb->priv, p_datachain );
b_ret = true;
}
if( p_pkt == NULL )
return b_ret;
if( p_pkt->i_buffer == 0 )
{
block_Release( p_pkt );
return b_ret;
}
if( !b_unit_start && p_pes->gather.p_data == NULL )
{
block_Release( p_pkt );
return b_ret;
}
block_ChainLastAppend( &p_pes->gather.pp_last, p_pkt );
p_pes->gather.i_gathered += p_pkt->i_buffer;
if( p_pes->gather.i_data_size > 0 &&
p_pes->gather.i_gathered >= p_pes->gather.i_data_size )
{
assert(p_pes->gather.p_data);
return ts_pes_Push( cb, p_pes, NULL, true );
}
return b_ret;
}
bool ts_pes_Drain( ts_pes_parse_callback *cb, ts_stream_t *p_pes )
{
return ts_pes_Push( cb, p_pes, NULL, true );
}
bool ts_pes_Gather( ts_pes_parse_callback *cb,
ts_stream_t *p_pes, block_t *p_pkt,
bool b_unit_start, bool b_valid_scrambling )
{
bool b_ret = false;
bool b_single_payload = b_unit_start; 
bool b_aligned_ts_payload = true;
if( unlikely(p_pes->b_broken_PUSI_conformance) )
{
b_aligned_ts_payload = false;
b_single_payload = false;
}
if( (p_pkt->i_flags & BLOCK_FLAG_SCRAMBLED) && b_valid_scrambling )
{
block_Release( p_pkt );
return ts_pes_Push( cb, p_pes, NULL, true );
}
if( p_pkt->i_flags & BLOCK_FLAG_SOURCE_RANDOM_ACCESS )
{
p_pes->gather.i_saved = 0;
b_ret |= ts_pes_Push( cb, p_pes, NULL, true );
if( p_pes->p_es )
p_pes->p_es->i_next_block_flags |= BLOCK_FLAG_DISCONTINUITY;
}
else if( p_pkt->i_flags & BLOCK_FLAG_DISCONTINUITY )
{
p_pes->gather.i_data_size = 0;
p_pes->gather.i_saved = 0;
if( p_pes->p_es )
p_pes->p_es->i_next_block_flags |= BLOCK_FLAG_DISCONTINUITY|BLOCK_FLAG_CORRUPTED;
}
if ( unlikely(p_pes->gather.i_saved > 0) )
{
assert(p_pes->gather.i_saved < 6);
if( !b_aligned_ts_payload )
{
p_pkt = block_Realloc( p_pkt, p_pes->gather.i_saved, p_pkt->i_buffer );
if( p_pkt )
memcpy( p_pkt->p_buffer, p_pes->gather.saved, p_pes->gather.i_saved );
}
p_pes->gather.i_saved = 0;
}
for( bool b_first_sync_done = false; p_pkt; )
{
assert( p_pes->gather.i_saved == 0 );
if( p_pes->gather.p_data == NULL && b_unit_start && !b_first_sync_done && p_pkt->i_buffer >= 6 )
{
if( likely(b_aligned_ts_payload) )
{
if( memcmp( p_pkt->p_buffer, pes_sync, 3 ) )
{
block_Release( p_pkt );
return b_ret;
}
}
else
{
uint8_t *p_buf = FindNextPESHeader( p_pkt->p_buffer, p_pkt->i_buffer - 3 );
if( p_buf == NULL )
{
if( MayHaveStartCodeOnEnd( p_pkt->p_buffer, p_pkt->i_buffer ) )
{
p_pkt->p_buffer += p_pkt->i_buffer - 3;
p_pes->gather.i_saved = p_pkt->i_buffer = 3;
memcpy(p_pes->gather.saved, p_pkt->p_buffer, p_pkt->i_buffer);
}
block_Release( p_pkt );
return b_ret;
}
p_pkt->i_buffer -= p_buf - p_pkt->p_buffer;
p_pkt->p_buffer = p_buf;
}
p_pes->gather.i_data_size = GetWBE(&p_pkt->p_buffer[4]);
if( p_pes->gather.i_data_size > 0 )
p_pes->gather.i_data_size += 6;
b_first_sync_done = true; 
}
else
{
assert( p_pes->gather.i_data_size > p_pes->gather.i_gathered ||
p_pes->gather.i_data_size == 0 );
if( p_pes->gather.i_data_size > p_pes->gather.i_gathered && !b_single_payload )
{
const size_t i_remain = p_pes->gather.i_data_size - p_pes->gather.i_gathered;
if( likely(p_pkt->i_buffer <= i_remain) )
{
b_ret |= ts_pes_Push( cb, p_pes, p_pkt, p_pes->gather.p_data == NULL );
p_pkt = NULL;
}
else 
{
block_t *p_split;
if( !block_Split( &p_pkt, &p_split, i_remain ) )
{
block_Release( p_pkt );
return false;
}
b_ret |= ts_pes_Push( cb, p_pes, p_pkt, p_pes->gather.p_data == NULL );
p_pkt = p_split;
b_first_sync_done = false;
}
}
else 
{
if( likely(b_aligned_ts_payload) && b_unit_start )
{
b_ret |= ts_pes_Push( cb, p_pes, NULL, true );
if( p_pkt->i_buffer >= 6 )
{
p_pes->gather.i_data_size = GetWBE(&p_pkt->p_buffer[4]);
if( p_pes->gather.i_data_size > 0 )
p_pes->gather.i_data_size += 6;
}
}
b_ret |= ts_pes_Push( cb, p_pes, p_pkt, b_unit_start );
p_pkt = NULL;
}
}
if( unlikely(p_pkt && p_pkt->i_buffer < 6) )
{
assert(!b_single_payload);
assert(p_pes->gather.i_saved == 0);
p_pes->gather.i_saved = p_pkt->i_buffer;
memcpy(p_pes->gather.saved, p_pkt->p_buffer, p_pkt->i_buffer);
block_Release( p_pkt );
p_pkt = NULL;
}
}
return b_ret;
}
