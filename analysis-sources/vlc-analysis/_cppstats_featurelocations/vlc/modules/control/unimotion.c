





































































#if defined(__APPLE__)
#include "TargetConditionals.h"
#if !TARGET_OS_IPHONE
#define HAVE_MACOS_UNIMOTION
#endif
#endif

#if defined(HAVE_MACOS_UNIMOTION)

#include "unimotion.h"
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdint.h>

enum data_type {
PB_IB,
MBP
};

struct pb_ib_data {
int8_t x;
int8_t y;
int8_t z;
int8_t pad[57];
};

struct mbp_data {
int16_t x;
int16_t y;
int16_t z;
int8_t pad[34];
};

union motion_data {
struct pb_ib_data pb_ib;
struct mbp_data mbp;
};


static int set_values(int type, int *kernFunc, const char **servMatch, int *dataType)
{
switch ( type ) {
case powerbook:
*kernFunc = 21;
*servMatch = "IOI2CMotionSensor";
*dataType = PB_IB;
break;
case ibook:
*kernFunc = 21;
*servMatch = "IOI2CMotionSensor";
*dataType = PB_IB;
break;
case highrespb:
*kernFunc = 21;
*servMatch = "PMUMotionSensor";
*dataType = PB_IB;
break;
case macbookpro:
*kernFunc = 5;
*servMatch = "SMCMotionSensor";
*dataType = MBP;
break;
default:
return 0;
}

return 1;
}

static int probe_sms(int kernFunc, const char *servMatch, int dataType, void *data)
{
kern_return_t result;
mach_port_t masterPort;
io_iterator_t iterator;
io_object_t aDevice;
io_connect_t dataPort;

size_t structureInputSize;
size_t structureOutputSize;

union motion_data inputStructure;
union motion_data *outputStructure;

outputStructure = (union motion_data *)data;

result = IOMasterPort(MACH_PORT_NULL, &masterPort);

CFMutableDictionaryRef matchingDictionary = IOServiceMatching(servMatch);

result = IOServiceGetMatchingServices(masterPort, matchingDictionary, &iterator);

if (result != KERN_SUCCESS) {

return 0;
}

aDevice = IOIteratorNext(iterator);
IOObjectRelease(iterator);

if (aDevice == 0) {

return 0;
}

result = IOServiceOpen(aDevice, mach_task_self(), 0, &dataPort);
IOObjectRelease(aDevice);

if (result != KERN_SUCCESS) {

return 0;
}

switch ( dataType ) {
case PB_IB:
structureInputSize = sizeof(struct pb_ib_data);
structureOutputSize = sizeof(struct pb_ib_data);
break;
case MBP:
structureInputSize = sizeof(struct mbp_data);
structureOutputSize = sizeof(struct mbp_data);
break;
default:
return 0;
}

memset(&inputStructure, 0, sizeof(union motion_data));
memset(outputStructure, 0, sizeof(union motion_data));

result = IOConnectCallStructMethod(dataPort, kernFunc, &inputStructure,
structureInputSize, outputStructure, &structureOutputSize );

IOServiceClose(dataPort);

if (result != KERN_SUCCESS) {

return 0;
}
return 1;
}

int detect_sms()
{
int kernFunc;
const char *servMatch;
int dataType;
union motion_data data;
int i;

for ( i = 1; ; i++ ) {
if ( !set_values(i, &kernFunc, &servMatch, &dataType) )
break;
if ( probe_sms(kernFunc, servMatch, dataType, &data) )
return i;
}

return unknown;
}

int read_sms_raw(int type, int *x, int *y, int *z)
{
int kernFunc;
const char *servMatch;
int dataType;
union motion_data data;

if ( !set_values(type, &kernFunc, &servMatch, &dataType) )
return 0;
if ( probe_sms(kernFunc, servMatch, dataType, &data) ) {
switch ( dataType ) {
case PB_IB:
if ( x ) *x = data.pb_ib.x;
if ( y ) *y = data.pb_ib.y;
if ( z ) *z = data.pb_ib.z;
break;
case MBP:
if ( x ) *x = data.mbp.x;
if ( y ) *y = data.mbp.y;
if ( z ) *z = data.mbp.z;
break;
default:
return 0;
}
return 1;
}
return 0;
}

int read_sms(int type, int *x, int *y, int *z)
{
int _x, _y, _z;
int xoff, yoff, zoff;
Boolean ok;
int ret;

ret = read_sms_raw(type, &_x, &_y, &_z);
if ( !ret )
return 0;

static CFStringRef app = CFSTR("com.ramsayl.UniMotion");
static CFStringRef xoffstr = CFSTR("x_offset");
static CFStringRef yoffstr = CFSTR("y_offset");
static CFStringRef zoffstr = CFSTR("z_offset");
xoff = CFPreferencesGetAppIntegerValue(xoffstr, app, &ok);
if ( ok ) _x += xoff;
yoff = CFPreferencesGetAppIntegerValue(yoffstr, app, &ok);
if ( ok ) _y += yoff;
zoff = CFPreferencesGetAppIntegerValue(zoffstr, app, &ok);
if ( ok ) _z += zoff;

*x = _x;
*y = _y;
*z = _z;

return ret;
}

int read_sms_real(int type, double *x, double *y, double *z)
{
int _x, _y, _z;
int xscale, yscale, zscale;
int ret;
Boolean ok;

ret = read_sms_raw(type, &_x, &_y, &_z);
if ( !ret )
return 0;

static CFStringRef app = CFSTR("com.ramsayl.UniMotion");
static CFStringRef xscalestr = CFSTR("x_scale");
static CFStringRef yscalestr = CFSTR("y_scale");
static CFStringRef zscalestr = CFSTR("z_scale");
xscale = CFPreferencesGetAppIntegerValue(xscalestr, app, &ok);
if ( !ok ) return 0;
yscale = CFPreferencesGetAppIntegerValue(yscalestr, app, &ok);
if ( !ok ) return 0;
zscale = CFPreferencesGetAppIntegerValue(zscalestr, app, &ok);
if ( !ok ) return 0;

*x = _x / (double)xscale;
*y = _y / (double)yscale;
*z = _z / (double)zscale;

return 1;
}

#endif
