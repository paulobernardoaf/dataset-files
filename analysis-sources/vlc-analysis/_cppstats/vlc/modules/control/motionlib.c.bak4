






















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <math.h>
#include <unistd.h>

#include <vlc_common.h>

#if defined(__APPLE__)
#include "TargetConditionals.h"
#if !TARGET_OS_IPHONE
#define HAVE_MACOS_UNIMOTION
#endif
#endif

#if defined(HAVE_MACOS_UNIMOTION)
#include "unimotion.h"
#endif

#include "motionlib.h"

struct motion_sensors_t
{
enum { HDAPS_SENSOR, AMS_SENSOR, APPLESMC_SENSOR,
UNIMOTION_SENSOR } sensor;
#if defined(HAVE_MACOS_UNIMOTION)
enum sms_hardware unimotion_hw;
#endif
int i_calibrate;

int p_oldx[16];
int i;
int i_sum;
};

motion_sensors_t *motion_create( vlc_object_t *obj )
{
FILE *f;
int i_x = 0, i_y = 0;

motion_sensors_t *motion = malloc( sizeof( motion_sensors_t ) );
if( unlikely( motion == NULL ) )
{
return NULL;
}

if( access( "/sys/devices/platform/hdaps/position", R_OK ) == 0
&& ( f = fopen( "/sys/devices/platform/hdaps/calibrate", "re" ) ) )
{

motion->i_calibrate = fscanf( f, "(%d,%d)", &i_x, &i_y ) == 2 ? i_x: 0;
fclose( f );
motion->sensor = HDAPS_SENSOR;
msg_Dbg( obj, "HDAPS motion detection correctly loaded" );
}
else if( access( "/sys/devices/ams/x", R_OK ) == 0 )
{

motion->sensor = AMS_SENSOR;
msg_Dbg( obj, "AMS motion detection correctly loaded" );
}
else if( access( "/sys/devices/platform/applesmc.768/position", R_OK ) == 0
&& ( f = fopen( "/sys/devices/platform/applesmc.768/calibrate", "re" ) ) )
{


motion->i_calibrate = fscanf( f, "(%d,%d)", &i_x, &i_y ) == 2 ? i_x: 0;
fclose( f );
motion->sensor = APPLESMC_SENSOR;
msg_Dbg( obj, "Apple SMC motion detection correctly loaded" );
}
#if defined(HAVE_MACOS_UNIMOTION)
else if( (motion->unimotion_hw = detect_sms()) )
{
motion->sensor = UNIMOTION_SENSOR;
msg_Dbg( obj, "UniMotion motion detection correctly loaded" );
}
#endif
else
{

msg_Err( obj, "No motion sensor available" );
free( motion );
return NULL;
}

memset( motion->p_oldx, 0, sizeof( motion->p_oldx ) );
motion->i = 0;
motion->i_sum = 0;
return motion;
}

void motion_destroy( motion_sensors_t *motion )
{
free( motion );
}




static int GetOrientation( motion_sensors_t *motion )
{
FILE *f;
int i_x = 0, i_y = 0, i_z = 0;
int i_ret;

switch( motion->sensor )
{
case HDAPS_SENSOR:
f = fopen( "/sys/devices/platform/hdaps/position", "re" );
if( !f )
{
return 0;
}

i_ret = fscanf( f, "(%d,%d)", &i_x, &i_y );
fclose( f );

if( i_ret < 2 )
return 0;
else
return ( i_x - motion->i_calibrate ) * 10;

case AMS_SENSOR:
f = fopen( "/sys/devices/ams/x", "re" );
if( !f )
{
return 0;
}

i_ret = fscanf( f, "%d", &i_x);
fclose( f );

if( i_ret < 1 )
return 0;
else
return - i_x * 30; 

case APPLESMC_SENSOR:
f = fopen( "/sys/devices/platform/applesmc.768/position", "re" );
if( !f )
{
return 0;
}

i_ret = fscanf( f, "(%d,%d,%d)", &i_x, &i_y, &i_z );
fclose( f );

if( i_ret < 3 )
return 0;
else
return ( i_x - motion->i_calibrate ) * 10;

#if defined(HAVE_MACOS_UNIMOTION)
case UNIMOTION_SENSOR:
if( read_sms_raw( motion->unimotion_hw, &i_x, &i_y, &i_z ) )
{
double d_norm = sqrt( i_x*i_x+i_z*i_z );
if( d_norm < 100 )
return 0;
double d_x = i_x / d_norm;
if( i_z > 0 )
return -asin(d_x)*3600/3.141;
else
return 3600 + asin(d_x)*3600/3.141;
}
else
return 0;
#endif
default:
vlc_assert_unreachable();
}
}




int motion_get_angle( motion_sensors_t *motion )
{
const int filter_length = ARRAY_SIZE( motion->p_oldx );
int i_x = GetOrientation( motion );

motion->i_sum += i_x - motion->p_oldx[motion->i];
motion->p_oldx[motion->i] = i_x;
motion->i = ( motion->i + 1 ) % filter_length;
i_x = motion->i_sum / filter_length;

return i_x;
}

