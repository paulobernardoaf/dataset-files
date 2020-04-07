#include <vdpau/vdpau.h>
typedef struct AVVDPAUDeviceContext {
VdpDevice device;
VdpGetProcAddress *get_proc_address;
} AVVDPAUDeviceContext;
