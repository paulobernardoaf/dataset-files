#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include "hxxx_nal.h"
#include <vlc_block.h>
static bool block_WillRealloc( block_t *p_block, ssize_t i_prebody, size_t i_body )
{
if( i_prebody <= 0 && i_body <= (size_t)(-i_prebody) )
return false;
else
return ( i_prebody + i_body <= p_block->i_size );
}
static inline void hxxx_WritePrefix( uint8_t i_nal_length_size, uint8_t *p_dest, uint32_t i_payload )
{
if( i_nal_length_size == 4 )
SetDWBE( p_dest, i_payload );
else if( i_nal_length_size == 2 )
SetWBE( p_dest, i_payload );
else
*p_dest = i_payload;
}
block_t *hxxx_AnnexB_to_xVC( block_t *p_block, uint8_t i_nal_length_size )
{
unsigned i_nalcount = 0;
unsigned i_list = 16;
struct nalmoves_e
{
const uint8_t *p; 
uint8_t prefix; 
off_t move; 
} *p_list = NULL;
if(!p_block->i_buffer || p_block->p_buffer[0])
goto error;
if(! (p_list = vlc_alloc( i_list, sizeof(*p_list) )) )
goto error;
const uint8_t *p_buf = p_block->p_buffer;
const uint8_t *p_end = &p_block->p_buffer[p_block->i_buffer];
unsigned i_bitflow = 0;
off_t i_move = 0;
while( p_buf != p_end )
{
i_bitflow <<= 1;
if( !*p_buf )
{
i_bitflow |= 1;
}
else if( *p_buf == 0x01 && (i_bitflow & 0x06) == 0x06 ) 
{
if( i_bitflow & 0x08 ) 
{
p_list[i_nalcount].p = &p_buf[-3];
p_list[i_nalcount].prefix = 4;
}
else 
{
p_list[i_nalcount].p = &p_buf[-2];
p_list[i_nalcount].prefix = 3;
}
i_move += (off_t) i_nal_length_size - p_list[i_nalcount].prefix;
p_list[i_nalcount++].move = i_move;
if(i_nalcount == i_list)
{
i_list += 16;
struct nalmoves_e *p_new = realloc( p_list, sizeof(*p_new) * i_list );
if(unlikely(!p_new))
goto error;
p_list = p_new;
}
}
p_buf++;
}
if( !i_nalcount )
goto error;
if( i_nalcount == 1 && block_WillRealloc( p_block, p_list[0].move, p_block->i_buffer ) )
{
uint32_t i_payload = p_block->i_buffer - p_list[0].prefix;
block_t *p_newblock = block_Realloc( p_block, p_list[0].move, p_block->i_buffer );
if( unlikely(!p_newblock) )
goto error;
p_block = p_newblock;
hxxx_WritePrefix( i_nal_length_size, p_block->p_buffer , i_payload );
free( p_list );
return p_block;
}
block_t *p_release = NULL;
const uint8_t *p_source = NULL;
const uint8_t *p_sourceend = NULL;
uint8_t *p_dest = NULL;
const size_t i_dest = p_block->i_buffer + p_list[i_nalcount - 1].move;
if( p_list[i_nalcount - 1].move != 0 || i_nal_length_size != 4 ) 
{
if( p_list[i_nalcount - 1].move > 0 && block_WillRealloc( p_block, 0, i_dest ) )
{
uint32_t i_sizebackup = p_block->i_buffer;
block_t *p_newblock = block_Realloc( p_block, 0, i_dest );
if( unlikely(!p_newblock) )
goto error;
p_block = p_newblock;
p_sourceend = &p_block->p_buffer[i_sizebackup];
p_source = p_dest = p_block->p_buffer;
}
else
{
block_t *p_newblock = block_Alloc( i_dest );
if( unlikely(!p_newblock) )
goto error;
p_release = p_block; 
p_source = p_release->p_buffer;
p_sourceend = &p_release->p_buffer[p_release->i_buffer];
p_block = p_newblock;
p_dest = p_newblock->p_buffer;
}
}
else
{
p_source = p_dest = p_block->p_buffer;
p_sourceend = &p_block->p_buffer[p_block->i_buffer];
}
if(!p_dest)
goto error;
for( unsigned i=i_nalcount; i!=0; i-- )
{
const uint8_t *p_readstart = p_list[i - 1].p;
uint32_t i_payload = p_sourceend - p_readstart - p_list[i - 1].prefix;
off_t offset = p_list[i - 1].p - p_source + p_list[i - 1].prefix + p_list[i - 1].move;
memmove( &p_dest[ offset ], &p_list[i - 1].p[ p_list[i - 1].prefix ], i_payload );
hxxx_WritePrefix( i_nal_length_size, &p_dest[ offset - i_nal_length_size ] , i_payload );
p_sourceend = p_readstart;
}
if( p_release )
block_Release( p_release );
free( p_list );
return p_block;
error:
free( p_list );
block_Release( p_block );
return NULL;
}
