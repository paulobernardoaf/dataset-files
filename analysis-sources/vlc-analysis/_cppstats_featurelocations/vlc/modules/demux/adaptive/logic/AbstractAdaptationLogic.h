























#if !defined(ABSTRACTADAPTATIONLOGIC_H_)
#define ABSTRACTADAPTATIONLOGIC_H_

#include "IDownloadRateObserver.h"
#include "../SegmentTracker.hpp"

namespace adaptive
{
namespace playlist
{
class BaseRepresentation;
class BaseAdaptationSet;
}

namespace logic
{
using namespace playlist;

class AbstractAdaptationLogic : public IDownloadRateObserver,
public SegmentTrackerListenerInterface
{
public:
AbstractAdaptationLogic (vlc_object_t *);
virtual ~AbstractAdaptationLogic ();

virtual BaseRepresentation* getNextRepresentation(BaseAdaptationSet *, BaseRepresentation *) = 0;
virtual void updateDownloadRate (const ID &, size_t, vlc_tick_t);
virtual void trackerEvent (const SegmentTrackerEvent &) {}
void setMaxDeviceResolution (int, int);

enum LogicType
{
Default = 0,
AlwaysBest,
AlwaysLowest,
RateBased,
FixedRate,
Predictive,
NearOptimal,
};

protected:
vlc_object_t *p_obj;
int maxwidth;
int maxheight;
};
}
}

#endif 
