


























#if !defined(SEG_SERVER_H_)
#define SEG_SERVER_H_

#include "Dependencies.h"
#include "SegImage.h"

#define USE_DEFAULT_PROPERTY_VALUE -1

class SegServer
{
public:
enum ServiceStatus
{
COM_LIB_INIT_ERROR = -1,
SERVICE_INIT_ERROR = -2,
SERVICE_REINIT_ERROR = -3,
SERVICE_FUNC_ERROR = -4,
SHARED_MEMORY_ERROR = -5,
ALLOCATION_FAILURE = -6,
SERVICE_NO_ERROR = 0,
SERVICE_NOT_READY = 1
};

static SegServer* CreateServer();
static SegServer* GetServerInstance();

virtual ServiceStatus Init() = 0;
virtual ServiceStatus Stop() = 0;

virtual ServiceStatus GetFrame(SegImage** image) = 0;

virtual void SetFps(int fps) = 0;
virtual int GetFps() = 0;




virtual void SetIVCAMMotionRangeTradeOff(int value) = 0;
virtual int GetIVCAMMotionRangeTradeOff() = 0;

virtual ~SegServer() {};
};

#endif 
