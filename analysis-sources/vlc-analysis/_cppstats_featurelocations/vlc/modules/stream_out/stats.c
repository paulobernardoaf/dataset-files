
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_sout.h>
#include <vlc_block.h>
#include <vlc_md5.h>
#include <vlc_fs.h>




typedef struct
{
FILE *output;
char *prefix;
} sout_stream_sys_t;

typedef struct
{
int id;
uint64_t segment_number;
void *next_id;
const char *type;
vlc_tick_t previous_dts,track_duration;
struct md5_s hash;
} sout_stream_id_sys_t;

static void *Add( sout_stream_t *p_stream, const es_format_t *p_fmt )
{
sout_stream_sys_t *p_sys = (sout_stream_sys_t *)p_stream->p_sys;
sout_stream_id_sys_t *id;

id = malloc( sizeof( sout_stream_id_sys_t ) );
if( unlikely( !id ) )
return NULL;

id->id = p_fmt->i_id;
switch( p_fmt->i_cat)
{
case VIDEO_ES:
id->type = "Video";
break;
case AUDIO_ES:
id->type = "Audio";
break;
case SPU_ES:
id->type = "SPU";
break;
default:
id->type = "Data";
break;
}
id->next_id = NULL;
id->segment_number = 0;
id->previous_dts = VLC_TICK_INVALID;
id->track_duration = 0;
InitMD5( &id->hash );

msg_Dbg( p_stream, "%s: Adding track type:%s id:%d", p_sys->prefix, id->type, id->id);
return id;
}

static void Del( sout_stream_t *p_stream, void *_id )
{
sout_stream_sys_t *p_sys = (sout_stream_sys_t *)p_stream->p_sys;
sout_stream_id_sys_t *id = (sout_stream_id_sys_t *)_id;

EndMD5( &id->hash );
char *outputhash = psz_md5_hash( &id->hash );
unsigned int num,den;
vlc_ureduce( &num, &den, id->track_duration, id->segment_number, 0 );
msg_Dbg( p_stream, "%s: Removing track type:%s id:%d", p_sys->prefix, id->type, id->id );
if( p_sys->output )
{
fprintf( p_sys->output,"#%s: final type:%s id:%d segments:%"PRIu64" total_duration:%"PRId64" avg_track:%d/%d md5:%16s\n",
p_sys->prefix, id->type, id->id, id->segment_number, id->track_duration, num, den, outputhash );
} else {
msg_Info( p_stream, "%s: final type:%s id:%d segments:%"PRIu64" total_duration:%"PRId64" avg_track:%d/%d md5:%16s",
p_sys->prefix, id->type, id->id, id->segment_number, id->track_duration, num, den, outputhash );
}
free( outputhash );
free( id );
}

static int Send( sout_stream_t *p_stream, void *_id, block_t *p_buffer )
{
sout_stream_sys_t *p_sys = (sout_stream_sys_t *)p_stream->p_sys;
sout_stream_id_sys_t *id = (sout_stream_id_sys_t *)_id;
struct md5_s hash;

block_t *p_block = p_buffer;
while ( p_block != NULL )
{
InitMD5( &hash );
AddMD5( &hash, p_block->p_buffer, p_block->i_buffer );
AddMD5( &id->hash, p_block->p_buffer, p_block->i_buffer );
EndMD5( &hash );
char *outputhash = psz_md5_hash( &hash );




vlc_tick_t dts_difference = VLC_TICK_INVALID;
if( likely( id->previous_dts != VLC_TICK_INVALID ) )
dts_difference = p_block->i_dts - id->previous_dts;
if( p_sys->output )
{

fprintf( p_sys->output, "%s\t%d\t%s\t%"PRIu64"\t%"PRId64"\t%"PRId64"\t%16s\n",
p_sys->prefix, id->id, id->type, ++id->segment_number, dts_difference,
p_block->i_length, outputhash );

} else {
msg_Dbg( p_stream, "%s: track:%d type:%s segment_number:%"PRIu64" dts_difference:%"PRId64" length:%"PRId64" md5:%16s",
p_sys->prefix, id->id, id->type, ++id->segment_number, dts_difference,
p_block->i_length, outputhash );
}
id->track_duration += p_block->i_length ? p_block->i_length : dts_difference;
free( outputhash );
id->previous_dts = p_block->i_dts;
p_block = p_block->p_next;
}
return VLC_SUCCESS;
}




static const char *ppsz_sout_options[] = {
"output", "prefix", NULL
};

#define SOUT_CFG_PREFIX "sout-stats-"

static int Open(sout_stream_t *p_stream)
{
sout_stream_sys_t *p_sys;
char *outputFile;

p_sys = calloc( 1, sizeof( sout_stream_sys_t ) );
if( !p_sys )
return VLC_ENOMEM;


config_ChainParse( p_stream, SOUT_CFG_PREFIX, ppsz_sout_options,
p_stream->p_cfg );


outputFile = var_InheritString( p_stream, SOUT_CFG_PREFIX "output" );

if( outputFile )
{
p_sys->output = vlc_fopen( outputFile, "wt" );
if( !p_sys->output )
{
msg_Err( p_stream, "Unable to open file '%s' for writing", outputFile );
free( p_sys );
free( outputFile );
return VLC_EGENERIC;
} else {
fprintf( p_sys->output,"#prefix\ttrack\ttype\tsegment_number\tdts_difference\tlength\tmd5\n");
}
free( outputFile );
}
p_sys->prefix = var_InheritString( p_stream, SOUT_CFG_PREFIX "prefix" );

p_stream->p_sys = p_sys;
return VLC_SUCCESS;
}

static int OutputSend(sout_stream_t *stream, void *id, block_t *block)
{
Send(stream, id, block);
block_Release(block);
return VLC_SUCCESS;
}

static int OutputOpen(vlc_object_t *obj)
{
sout_stream_t *stream = (sout_stream_t *)obj;

if (stream->p_next != NULL)
return VLC_EGENERIC;

int val = Open(stream);

if (val == VLC_SUCCESS)
{
stream->pf_add = Add;
stream->pf_del = Del;
stream->pf_send = OutputSend;
}

return val;
}

static void *FilterAdd(sout_stream_t *stream, const es_format_t *fmt)
{
sout_stream_id_sys_t *id = Add(stream, fmt);

if (likely(id != NULL))
id->next_id = sout_StreamIdAdd(stream->p_next, fmt);

return id;
}

static void FilterDel(sout_stream_t *stream, void *opaque)
{
sout_stream_id_sys_t *id = opaque;

if (id->next_id != NULL)
sout_StreamIdDel(stream->p_next, id->next_id);
Del(stream, id);
}

static int FilterSend(sout_stream_t *stream, void *opaque, block_t *block)
{
sout_stream_id_sys_t *id = opaque;

Send(stream, id, block);
return sout_StreamIdSend(stream->p_next, id->next_id, block);
}

static int FilterOpen(vlc_object_t *obj)
{
sout_stream_t *stream = (sout_stream_t *)obj;

if (stream->p_next == NULL)
return VLC_EGENERIC;

int val = Open(stream);

if (val == VLC_SUCCESS)
{
stream->pf_add = FilterAdd;
stream->pf_del = FilterDel;
stream->pf_send = FilterSend;
}

return val;
}




static void Close( vlc_object_t * p_this )
{
sout_stream_t *p_stream = (sout_stream_t*)p_this;
sout_stream_sys_t *p_sys = (sout_stream_sys_t *)p_stream->p_sys;

if( p_sys->output )
fclose( p_sys->output );

free( p_sys->prefix );
free( p_sys );
}




#define OUTPUT_TEXT N_("Output file")
#define OUTPUT_LONGTEXT N_( "Writes stats to file instead of stdout" )

#define PREFIX_TEXT N_("Prefix to show on output line")

vlc_module_begin()
set_shortname( N_("Stats"))
set_description( N_("Writes statistic info about stream"))
set_capability( "sout output", 0 )
add_shortcut( "stats" )
set_category( CAT_SOUT )
set_subcategory( SUBCAT_SOUT_STREAM )
set_callbacks( OutputOpen, Close )
add_string( SOUT_CFG_PREFIX "output", "", OUTPUT_TEXT,OUTPUT_LONGTEXT, false );
add_string( SOUT_CFG_PREFIX "prefix", "stats", PREFIX_TEXT,PREFIX_TEXT, false );
add_submodule()
set_capability( "sout filter", 0 )
add_shortcut( "stats" )
set_callbacks( FilterOpen, Close )
vlc_module_end()
