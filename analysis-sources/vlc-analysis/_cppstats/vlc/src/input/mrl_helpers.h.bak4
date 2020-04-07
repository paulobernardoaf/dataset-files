



















#if !defined(INPUT_MRL_HELPERS_H)
#define INPUT_MRL_HELPERS_H

#include <string.h>
#include <stdlib.h>

#include <vlc_common.h>
#include <vlc_memstream.h>
#include <vlc_arrays.h>
#include <vlc_url.h>


























static inline int
mrl_EscapeFragmentIdentifier( char** out, char const* payload )
{
struct vlc_memstream mstream;

#define RFC3986_SUBDELIMS "!" "$" "&" "'" "(" ")" "*" "+" "," ";" "="

#define RFC3986_ALPHA "abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define RFC3986_DIGIT "0123456789"
#define RFC3986_UNRESERVED RFC3986_ALPHA RFC3986_DIGIT "-" "." "_" "~"
#define RFC3986_PCHAR RFC3986_UNRESERVED RFC3986_SUBDELIMS ":" "@"
#define RFC3986_FRAGMENT RFC3986_PCHAR "/" "?"

if( vlc_memstream_open( &mstream ) )
return VLC_EGENERIC;

for( char const* p = payload; *p; ++p )
{
vlc_memstream_printf( &mstream,
( strchr( "!?", *p ) == NULL &&
strchr( RFC3986_FRAGMENT, *p ) ? "%c" : "%%%02hhx"), *p );
}

#undef RFC3986_FRAGMENT
#undef RFC3986_PCHAR
#undef RFC3986_UNRESERVEd
#undef RFC3986_DIGIT
#undef RFC3986_ALPHA
#undef RFC3986_SUBDELIMS

if( vlc_memstream_close( &mstream ) )
return VLC_EGENERIC;

*out = mstream.ptr;
return VLC_SUCCESS;
}




















static inline int
mrl_FragmentSplit( vlc_array_t* out_items,
char const** out_extra,
char const* payload )
{
char const* extra = NULL;

vlc_array_init( out_items );

while( strncmp( payload, "!/", 2 ) == 0 )
{
payload += 2;

int len = strcspn( payload, "!?" );
char* decoded = strndup( payload, len );

if( unlikely( !decoded ) || !vlc_uri_decode( decoded ) )
goto error;

if( vlc_array_append( out_items, decoded ) )
{
free( decoded );
goto error;
}
payload += len;
}

if( *payload )
{
if( *payload == '!' )
goto error;

if( *payload == '?' && vlc_array_count( out_items ) )
++payload;

extra = payload;
}

*out_extra = extra;
return VLC_SUCCESS;

error:
for( size_t i = 0; i < vlc_array_count( out_items ); ++i )
free( vlc_array_item_at_index( out_items, i ) );
vlc_array_clear( out_items );
return VLC_EGENERIC;;
}





#endif 
