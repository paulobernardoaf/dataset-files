






















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_filter.h>

#include <ctype.h>

#include "dynamicoverlay.h"





int BufferInit( buffer_t *p_buffer )
{
memset( p_buffer, 0, sizeof( buffer_t ) );
p_buffer->p_memory = NULL;
p_buffer->p_begin = NULL;

return VLC_SUCCESS;
}

int BufferDestroy( buffer_t *p_buffer )
{
free( p_buffer->p_memory );
p_buffer->p_memory = NULL;
p_buffer->p_begin = NULL;

return VLC_SUCCESS;
}

char *BufferGetToken( buffer_t *p_buffer )
{
char *p_char = p_buffer->p_begin;

while( isspace( (unsigned char)p_char[0] ) || p_char[0] == '\0' )
{
if( p_char <= (p_buffer->p_begin + p_buffer->i_length) )
p_char++;
else
return NULL;
}
return p_char;
}

int BufferAdd( buffer_t *p_buffer, const char *p_data, size_t i_len )
{
if( ( p_buffer->i_size - p_buffer->i_length -
( p_buffer->p_begin - p_buffer->p_memory ) ) < i_len )
{

if( ( p_buffer->i_size - p_buffer->i_length ) >= i_len )
{

memmove( p_buffer->p_memory, p_buffer->p_begin,
p_buffer->i_length );
p_buffer->p_begin = p_buffer->p_memory;
}
else
{

size_t i_newsize = 1024;
while( i_newsize < p_buffer->i_length + i_len )
i_newsize *= 2;




char *p_newdata = malloc( i_newsize );
if( p_newdata == NULL )
return VLC_ENOMEM;
if( p_buffer->p_begin != NULL )
{
memcpy( p_newdata, p_buffer->p_begin, p_buffer->i_length );
free( p_buffer->p_memory );
}
p_buffer->p_memory = p_buffer->p_begin = p_newdata;
p_buffer->i_size = i_newsize;
}
}


memcpy( p_buffer->p_begin + p_buffer->i_length, p_data, i_len );
p_buffer->i_length += i_len;
return VLC_SUCCESS;
}

int BufferPrintf( buffer_t *p_buffer, const char *p_fmt, ... )
{
int i_len;
int status;
char *psz_data;

va_list va;
va_start( va, p_fmt );

i_len = vasprintf( &psz_data, p_fmt, va );
va_end( va );
if( i_len == -1 )
return VLC_ENOMEM;

status = BufferAdd( p_buffer, psz_data, i_len );
free( psz_data );
return status;
}

int BufferDel( buffer_t *p_buffer, int i_len )
{
p_buffer->i_length -= i_len;
if( p_buffer->i_length == 0 )
{

p_buffer->p_begin = p_buffer->p_memory;
}
else
{
p_buffer->p_begin += i_len;
}
return VLC_SUCCESS;
}
