





























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_access.h>
#include <vlc_interrupt.h>
#include <vlc_dialog.h>

#include <sys/types.h>
#include <poll.h>

#include <errno.h>

#include "dvb.h"
#include "scan.h"

typedef struct
{
demux_handle_t p_demux_handles[MAX_DEMUX];
dvb_sys_t dvb;


struct scan_t *scan;
bool done;
} access_sys_t;




static int Open( vlc_object_t *p_this );
static void Close( vlc_object_t *p_this );

#define PROBE_TEXT N_("Probe DVB card for capabilities")
#define PROBE_LONGTEXT N_("Some DVB cards do not like to be probed for their capabilities, you can disable this feature if you experience some trouble.")


#define SATELLITE_TEXT N_("Satellite scanning config")
#define SATELLITE_LONGTEXT N_("Filename of config file in share/dvb/dvb-s.")

#define SCANLIST_TEXT N_("Scan tuning list")
#define SCANLIST_LONGTEXT N_("Filename containing initial scan tuning data.")

#define SCAN_NIT_TEXT N_("Use NIT for scanning services")

vlc_module_begin ()
set_shortname( N_("DVB") )
set_description( N_("DVB input with v4l2 support") )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_ACCESS )

add_bool( "dvb-probe", true, PROBE_TEXT, PROBE_LONGTEXT, true )

add_string( "dvb-satellite", NULL, SATELLITE_TEXT, SATELLITE_LONGTEXT,
true )
add_string( "dvb-scanlist", NULL, SCANLIST_TEXT, SCANLIST_LONGTEXT,
true )
add_bool( "dvb-scan-nit", true, SCAN_NIT_TEXT, NULL, true )

set_capability( "access", 0 )
add_shortcut( "dvb", 
"dvb-s", "qpsk", "satellite", 
"dvb-c", "cable", 
"dvb-t", "terrestrial" ) 

set_callbacks( Open, Close )

vlc_module_end ()





static int Control( stream_t *, int, va_list );

static block_t *BlockScan( stream_t *, bool * );

#define DVB_SCAN_MAX_LOCK_TIME VLC_TICK_FROM_SEC(2)

static void FilterUnset( stream_t *, int i_max );
static void FilterSet( stream_t *, int i_pid, int i_type );

static void VarInit( stream_t * );
static int ParseMRL( stream_t * );

static int ScanFrontendTuningHandler( scan_t *, void *, const scan_tuner_config_t * );
static int ScanFilterHandler( scan_t *, void *, uint16_t, bool );
static int ScanStatsCallback( scan_t *p_scan, void *p_privdata, int *pi_snr );
static int ScanReadCallback( scan_t *, void *, unsigned, size_t, uint8_t *, size_t *);




static int Open( vlc_object_t *p_this )
{
stream_t *p_access = (stream_t*)p_this;
access_sys_t *p_sys;

if( p_access->b_preparsing )
return VLC_EGENERIC;

p_access->p_sys = p_sys = calloc( 1, sizeof( access_sys_t ) );
if( !p_sys )
return VLC_ENOMEM;


VarInit( p_access );


if( ParseMRL( p_access ) )
{
free( p_sys );
return VLC_EGENERIC;
}

bool b_scan_mode = var_GetInteger( p_access, "dvb-frequency" ) == 0;
if( b_scan_mode )
{
msg_Dbg( p_access, "DVB scan mode selected" );
p_access->pf_block = BlockScan;
}
else
{
free( p_sys );
return VLC_EGENERIC; 
}


if( FrontendOpen( p_this, &p_sys->dvb, p_access->psz_name ) )
{
free( p_sys );
return VLC_EGENERIC;
}


if( DVROpen( p_this, &p_sys->dvb ) < 0 )
{
FrontendClose( p_this, &p_sys->dvb );
free( p_sys );
return VLC_EGENERIC;
}

scan_parameter_t parameter;
scan_t *p_scan;

scan_parameter_Init( &parameter );

parameter.b_use_nit = var_InheritBool( p_access, "dvb-scan-nit" );

if( FrontendFillScanParameter( p_this, &p_sys->dvb, &parameter ) ||
(p_scan = scan_New( VLC_OBJECT(p_access), &parameter,
ScanFrontendTuningHandler,
ScanStatsCallback,
ScanFilterHandler,
ScanReadCallback,
p_access )) == NULL )
{
scan_parameter_Clean( &parameter );
Close( VLC_OBJECT(p_access) );
return VLC_EGENERIC;
}

scan_parameter_Clean( &parameter );
p_sys->scan = p_scan;



p_access->pf_control = Control;

return VLC_SUCCESS;
}




static void Close( vlc_object_t *p_this )
{
stream_t *p_access = (stream_t*)p_this;
access_sys_t *p_sys = p_access->p_sys;

FilterUnset( p_access, MAX_DEMUX );

DVRClose( p_this, &p_sys->dvb );
FrontendClose( p_this, &p_sys->dvb );
scan_Destroy( p_sys->scan );

free( p_sys );
}

static int ScanFrontendTuningHandler( scan_t *p_scan, void *p_privdata,
const scan_tuner_config_t *p_cfg )
{
stream_t *p_access = (stream_t *) p_privdata;
access_sys_t *sys = p_access->p_sys;
VLC_UNUSED(p_scan);

var_SetInteger( p_access, "dvb-frequency", p_cfg->i_frequency );
var_SetInteger( p_access, "dvb-bandwidth", p_cfg->i_bandwidth );

if ( p_cfg->polarization != SCAN_POLARIZATION_NONE )
var_SetInteger( p_access, "dvb-voltage", p_cfg->polarization == SCAN_POLARIZATION_HORIZONTAL ? 18 : 13 );

if ( p_cfg->i_symbolrate )
var_SetInteger( p_access, "dvb-srate", p_cfg->i_symbolrate );

msg_Dbg( p_access, "Scanning frequency %d", p_cfg->i_frequency );
msg_Dbg( p_access, " bandwidth %d", p_cfg->i_bandwidth );


if( FrontendSet( VLC_OBJECT(p_access), &sys->dvb ) < 0 )
{
msg_Err( p_access, "Failed to tune the frontend" );
return VLC_EGENERIC;
}

return VLC_SUCCESS;
}

static int ScanStatsCallback( scan_t *p_scan, void *p_privdata, int *pi_snr )
{
stream_t *p_access = (stream_t *) p_privdata;
access_sys_t *sys = p_access->p_sys;
VLC_UNUSED(p_scan);

frontend_statistic_t stat;
if( !FrontendGetStatistic( &sys->dvb, &stat ) )
{
*pi_snr = stat.i_snr;
return VLC_SUCCESS;
}

return VLC_EGENERIC;
}

static int ScanFilterHandler( scan_t *p_scan, void *p_privdata, uint16_t i_pid, bool b_set )
{
stream_t *p_access = (stream_t *) p_privdata;
VLC_UNUSED(p_scan);

if( b_set )
FilterSet( p_access, i_pid, OTHER_TYPE );

return VLC_SUCCESS;
}

static int ScanReadCallback( scan_t *p_scan, void *p_privdata,
unsigned i_probe_timeout, size_t i_packets_max,
uint8_t *p_packet, size_t *pi_count )
{
stream_t *p_access = (stream_t *) p_privdata;
access_sys_t *p_sys = p_access->p_sys;
*pi_count = 0;


struct pollfd ufds[2];

ufds[0].fd = p_sys->dvb.i_handle;
ufds[0].events = POLLIN;
ufds[1].fd = p_sys->dvb.i_frontend_handle;
ufds[1].events = POLLPRI;

frontend_status_t status;
FrontendGetStatus( &p_sys->dvb, &status );
bool b_has_lock = status.b_has_lock;

vlc_tick_t i_scan_start = vlc_tick_now();

for( ; *pi_count == 0; )
{

int i_ret;

vlc_tick_t i_timeout = b_has_lock ? i_probe_timeout:
DVB_SCAN_MAX_LOCK_TIME;

do
{
vlc_tick_t i_poll_timeout = i_scan_start - vlc_tick_now() + i_timeout;

i_ret = 0;

if( vlc_killed() || scan_IsCancelled( p_scan ) )
break;

if( i_poll_timeout >= 0 )
i_ret = vlc_poll_i11e( ufds, 2, i_poll_timeout / 1000 );
}
while( i_ret < 0 && errno == EINTR );

if( i_ret < 0 )
{
return VLC_EGENERIC;
}
else if( i_ret == 0 )
{
return VLC_ENOITEM;
}

if( ufds[1].revents )
{
FrontendPoll( VLC_OBJECT(p_access), &p_sys->dvb );

FrontendGetStatus( &p_sys->dvb, &status );
if( status.b_has_lock && !b_has_lock )
{
i_scan_start = vlc_tick_now();
b_has_lock = true;
}
}

if ( ufds[0].revents )
{
ssize_t i_read = read( p_sys->dvb.i_handle, p_packet, TS_PACKET_SIZE * i_packets_max );
if( i_read < 0 )
{
msg_Warn( p_access, "read failed: %s", vlc_strerror_c(errno) );
break;
}
else
{
*pi_count = i_read / TS_PACKET_SIZE;
}
}
}

return VLC_SUCCESS;
}




static block_t *BlockScan( stream_t *p_access, bool *restrict eof )
{
access_sys_t *p_sys = p_access->p_sys;
scan_t *p_scan = p_sys->scan;
block_t *p_block = NULL;

if( scan_Run( p_scan ) != VLC_SUCCESS )
{
if( !p_sys->done )
{
msg_Info( p_access, "Scanning finished" );
p_sys->done = true;
p_block = scan_GetM3U( p_scan );
}
*eof = true;
}

return p_block;
}




static int Control( stream_t *p_access, int i_query, va_list args )
{
access_sys_t *sys = p_access->p_sys;
bool *pb_bool;
double *pf1, *pf2;
frontend_statistic_t stat;

switch( i_query )
{
case STREAM_CAN_SEEK:
case STREAM_CAN_FASTSEEK:
case STREAM_CAN_PAUSE:
case STREAM_CAN_CONTROL_PACE:
pb_bool = va_arg( args, bool * );
*pb_bool = false;
break;

case STREAM_GET_CONTENT_TYPE:
*va_arg( args, char** ) = strdup("application/vnd.apple.mpegurl"); 
return VLC_SUCCESS;

case STREAM_GET_PTS_DELAY:
*va_arg( args, vlc_tick_t * ) = DEFAULT_PTS_DELAY;
break;

case STREAM_GET_SIGNAL:
pf1 = va_arg( args, double * );
pf2 = va_arg( args, double * );

*pf1 = *pf2 = 0;
if( !FrontendGetStatistic( &sys->dvb, &stat ) )
{
*pf1 = (double)stat.i_snr / 65535.0;
*pf2 = (double)stat.i_signal_strenth / 65535.0;
}
break;

default:
return VLC_EGENERIC;

}
return VLC_SUCCESS;
}




static void FilterSet( stream_t *p_access, int i_pid, int i_type )
{
access_sys_t *p_sys = p_access->p_sys;
int i;


for( i = 0; i < MAX_DEMUX; i++ )
{
if( !p_sys->p_demux_handles[i].i_type )
break;

if( p_sys->p_demux_handles[i].i_pid == i_pid )
return; 
}

if( i >= MAX_DEMUX )
{
msg_Err( p_access, "no free p_demux_handles !" );
return;
}

if( DMXSetFilter( VLC_OBJECT(p_access), i_pid,
&p_sys->p_demux_handles[i].i_handle, i_type ) )
{
msg_Err( p_access, "DMXSetFilter failed" );
return;
}
p_sys->p_demux_handles[i].i_type = i_type;
p_sys->p_demux_handles[i].i_pid = i_pid;
}

static void FilterUnset( stream_t *p_access, int i_max )
{
access_sys_t *p_sys = p_access->p_sys;

for( int i = 0; i < i_max; i++ )
{
if( p_sys->p_demux_handles[i].i_type )
{
DMXUnsetFilter( VLC_OBJECT(p_access), p_sys->p_demux_handles[i].i_handle );
p_sys->p_demux_handles[i].i_type = 0;
}
}
}




static void VarInit( stream_t *p_access )
{
var_Create( p_access, "dvb-adapter", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT );
var_Create( p_access, "dvb-device", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT );
var_Create( p_access, "dvb-frequency", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT );
var_Create( p_access, "dvb-inversion", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT );
var_Create( p_access, "dvb-probe", VLC_VAR_BOOL | VLC_VAR_DOINHERIT );


var_Create( p_access, "dvb-satellite", VLC_VAR_STRING | VLC_VAR_DOINHERIT );
var_Create( p_access, "dvb-satno", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT );
var_Create( p_access, "dvb-voltage", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT );
var_Create( p_access, "dvb-high-voltage", VLC_VAR_BOOL | VLC_VAR_DOINHERIT );
var_Create( p_access, "dvb-tone", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT );
var_Create( p_access, "dvb-srate", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT );
var_Create( p_access, "dvb-lnb-lof1", VLC_VAR_INTEGER );
var_Create( p_access, "dvb-lnb-lof2", VLC_VAR_INTEGER );
var_Create( p_access, "dvb-lnb-slof", VLC_VAR_INTEGER );


var_Create( p_access, "dvb-bandwidth", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT );
var_Create( p_access, "dvb-transmission", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT );
var_Create( p_access, "dvb-hierarchy", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT );
}


static int ParseMRL( stream_t *p_access )
{
char *psz_dup = strdup( p_access->psz_location );
char *psz_parser = psz_dup;
vlc_value_t val;

#define GET_OPTION_INT( option ) if ( !strncmp( psz_parser, option "=", strlen(option "=") ) ) { val.i_int = strtol( psz_parser + strlen(option "="), &psz_parser, 0 ); var_Set( p_access, "dvb-" option, val ); }







#define GET_OPTION_BOOL( option ) if ( !strncmp( psz_parser, option "=", strlen(option "=") ) ) { val.b_bool = strtol( psz_parser + strlen(option "="), &psz_parser, 0 ); var_Set( p_access, "dvb-" option, val ); }







#define GET_OPTION_STRING( option ) if ( !strncmp( psz_parser, option "=", strlen( option "=" ) ) ) { psz_parser += strlen( option "=" ); val.psz_string = psz_parser; char *p_save; char *tok = strtok_r(val.psz_string, ":", &p_save); val.psz_string[tok - val.psz_string - 1] = 0; var_Set( p_access, "dvb-" option, val ); psz_parser += strlen( val.psz_string ); }











while( *psz_parser )
{
GET_OPTION_INT("adapter")
else GET_OPTION_INT("device")
else GET_OPTION_INT("frequency")
else GET_OPTION_INT("inversion")
else GET_OPTION_BOOL("probe")
else GET_OPTION_BOOL("budget-mode")

else GET_OPTION_STRING("satellite")
else GET_OPTION_INT("voltage")
else GET_OPTION_BOOL("high-voltage")
else GET_OPTION_INT("tone")
else GET_OPTION_INT("satno")
else GET_OPTION_INT("srate")
else GET_OPTION_INT("lnb-lof1")
else GET_OPTION_INT("lnb-lof2")
else GET_OPTION_INT("lnb-slof")

else GET_OPTION_INT("bandwidth")
else GET_OPTION_INT("transmission")
else GET_OPTION_INT("hierarchy")


else if( !strncmp( psz_parser, "polarization=",
strlen( "polarization=" ) ) )
{
psz_parser += strlen( "polarization=" );
if ( *psz_parser == 'V' || *psz_parser == 'v' )
val.i_int = 13;
else if ( *psz_parser == 'H' || *psz_parser == 'h' )
val.i_int = 18;
else
{
msg_Warn( p_access, "illegal polarization %c", *psz_parser );
free( psz_dup );
return VLC_EGENERIC;
}
var_Set( p_access, "dvb-voltage", val );
}
else
{
msg_Warn( p_access, "unknown option (%s)", psz_parser );
free( psz_dup );
return VLC_EGENERIC;
}

if ( *psz_parser )
psz_parser++;
}
#undef GET_OPTION_INT
#undef GET_OPTION_BOOL
#undef GET_OPTION_STRING

free( psz_dup );
return VLC_SUCCESS;
}

