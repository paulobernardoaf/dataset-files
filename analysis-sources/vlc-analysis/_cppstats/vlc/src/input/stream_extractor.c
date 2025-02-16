#if !defined(STREAM_EXTRACTOR_H)
#define STREAM_EXTRACTOR_H
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_stream.h>
#include <vlc_stream_extractor.h>
#include <vlc_modules.h>
#include <vlc_url.h>
#include <vlc_memstream.h>
#include <libvlc.h>
#include <assert.h>
#include "stream.h"
#include "mrl_helpers.h"
struct stream_extractor_private {
union {
stream_extractor_t extractor;
stream_directory_t directory;
};
int (*pf_init)( struct stream_extractor_private*, stream_t* );
void (*pf_clean)( struct stream_extractor_private* );
stream_t* wrapper; 
stream_t* source; 
module_t* module; 
vlc_object_t* object; 
};
static char*
StreamExtractorCreateMRL( char const* base, char const* subentry )
{
struct vlc_memstream buffer;
char* escaped;
if( mrl_EscapeFragmentIdentifier( &escaped, subentry ) )
return NULL;
if( vlc_memstream_open( &buffer ) )
{
free( escaped );
return NULL;
}
vlc_memstream_puts( &buffer, base );
if( !strstr( base, "#" ) )
vlc_memstream_putc( &buffer, '#' );
vlc_memstream_printf( &buffer, "!/%s", escaped );
free( escaped );
return vlc_memstream_close( &buffer ) ? NULL : buffer.ptr;
}
static void se_Release( struct stream_extractor_private* priv )
{
if( priv->pf_clean )
priv->pf_clean( priv );
if( priv->module )
{
module_unneed( priv->object, priv->module );
if( priv->source )
vlc_stream_Delete( priv->source );
}
vlc_object_delete(priv->object);
}
static void
se_StreamDelete( stream_t* stream )
{
se_Release( stream->p_sys );
}
static ssize_t
se_StreamRead( stream_t* stream, void* buf, size_t len )
{
struct stream_extractor_private* priv = stream->p_sys;
return priv->extractor.pf_read( &priv->extractor, buf, len );
}
static block_t*
se_StreamBlock( stream_t* stream, bool* eof )
{
struct stream_extractor_private* priv = stream->p_sys;
return priv->extractor.pf_block( &priv->extractor, eof );
}
static int
se_StreamSeek( stream_t* stream, uint64_t offset )
{
struct stream_extractor_private* priv = stream->p_sys;
return priv->extractor.pf_seek( &priv->extractor, offset );
}
static int
se_ReadDir( stream_t* stream, input_item_node_t* node )
{
struct stream_extractor_private* priv = stream->p_sys;
return priv->directory.pf_readdir( &priv->directory, node );
}
static int
se_StreamControl( stream_t* stream, int req, va_list args )
{
struct stream_extractor_private* priv = stream->p_sys;
return priv->extractor.pf_control( &priv->extractor, req, args );
}
static int
se_DirControl( stream_t* stream, int req, va_list args )
{
(void)stream;
(void)req;
(void)args;
return VLC_EGENERIC;
}
static int
se_InitStream( struct stream_extractor_private* priv, stream_t* s )
{
if( priv->extractor.pf_read ) s->pf_read = se_StreamRead;
else s->pf_block = se_StreamBlock;
s->pf_seek = se_StreamSeek;
s->pf_control = se_StreamControl;
s->psz_url = StreamExtractorCreateMRL( priv->extractor.source->psz_url,
priv->extractor.identifier );
if( unlikely( !s->psz_url ) )
return VLC_ENOMEM;
return VLC_SUCCESS;
}
static void
se_CleanStream( struct stream_extractor_private* priv )
{
free( (char*)priv->extractor.identifier );
}
static int
se_InitDirectory( struct stream_extractor_private* priv, stream_t* s )
{
stream_directory_t* directory = &priv->directory;
s->pf_readdir = se_ReadDir;
s->pf_control = se_DirControl;
s->psz_url = strdup( directory->source->psz_url );
if( unlikely( !s->psz_url ) )
return VLC_EGENERIC;
return VLC_SUCCESS;
}
static int
se_AttachWrapper( struct stream_extractor_private* priv, stream_t* source )
{
stream_t* s = vlc_stream_CommonNew( vlc_object_parent(source),
se_StreamDelete );
if( unlikely( !s ) )
return VLC_ENOMEM;
if( priv->pf_init( priv, s ) )
{
stream_CommonDelete( s );
return VLC_EGENERIC;
}
priv->wrapper = s;
priv->wrapper->p_input_item = source->p_input_item;
priv->wrapper->p_sys = priv;
priv->source = source;
priv->wrapper = stream_FilterChainNew( priv->wrapper, "cache" );
return VLC_SUCCESS;
}
static int
StreamExtractorAttach( stream_t** source, char const* identifier,
char const* module_name )
{
const bool extractor = identifier != NULL;
char const* capability = extractor ? "stream_extractor"
: "stream_directory";
struct stream_extractor_private* priv = vlc_custom_create(
vlc_object_parent(*source), sizeof( *priv ), capability );
if( unlikely( !priv ) )
return VLC_ENOMEM;
if( extractor )
{
priv->object = VLC_OBJECT( &priv->extractor );
priv->pf_init = se_InitStream;
priv->pf_clean = se_CleanStream;
priv->extractor.source = *source;
priv->extractor.identifier = strdup( identifier );
if( unlikely( !priv->extractor.identifier ) )
goto error;
}
else
{
priv->object = VLC_OBJECT( &priv->directory );
priv->pf_init = se_InitDirectory;
priv->pf_clean = NULL;
priv->directory.source = *source;
}
priv->module = module_need( priv->object, capability, module_name, true );
if( !priv->module || se_AttachWrapper( priv, *source ) )
goto error;
*source = priv->wrapper;
return VLC_SUCCESS;
error:
se_Release( priv );
return VLC_EGENERIC;
}
int
vlc_stream_directory_Attach( stream_t** source, char const* module_name )
{
return StreamExtractorAttach( source, NULL, module_name );
}
int
vlc_stream_extractor_Attach( stream_t** source, char const* identifier,
char const* module_name )
{
return StreamExtractorAttach( source, identifier, module_name );
}
int
stream_extractor_AttachParsed( stream_t** source, char const* data,
char const** out_extra )
{
vlc_array_t identifiers;
if( mrl_FragmentSplit( &identifiers, out_extra, data ) )
return VLC_EGENERIC;
size_t count = vlc_array_count( &identifiers );
size_t idx = 0;
while( idx < count )
{
char* id = vlc_array_item_at_index( &identifiers, idx );
if( vlc_stream_extractor_Attach( source, id, NULL ) )
break;
++idx;
}
for( size_t i = 0; i < count; ++i )
free( vlc_array_item_at_index( &identifiers, i ) );
vlc_array_clear( &identifiers );
return idx == count ? VLC_SUCCESS : VLC_EGENERIC;
}
char*
vlc_stream_extractor_CreateMRL( stream_directory_t* directory,
char const* subentry )
{
return StreamExtractorCreateMRL( directory->source->psz_url, subentry );
}
#endif 
