























#if !defined(RATEBASEDADAPTATIONLOGIC_H_)
#define RATEBASEDADAPTATIONLOGIC_H_

#include "AbstractAdaptationLogic.h"
#include "../tools/MovingAverage.hpp"

namespace adaptive
{
namespace logic
{

class RateBasedAdaptationLogic : public AbstractAdaptationLogic
{
public:
RateBasedAdaptationLogic (vlc_object_t *);
virtual ~RateBasedAdaptationLogic ();

BaseRepresentation *getNextRepresentation(BaseAdaptationSet *, BaseRepresentation *);
virtual void updateDownloadRate(const ID &, size_t, vlc_tick_t); 
virtual void trackerEvent(const SegmentTrackerEvent &); 

private:
size_t bpsAvg;
size_t currentBps;
size_t usedBps;

MovingAverage<size_t> average;

size_t dlsize;
vlc_tick_t dllength;

mutable vlc_mutex_t lock;
};

class FixedRateAdaptationLogic : public AbstractAdaptationLogic
{
public:
FixedRateAdaptationLogic(vlc_object_t *, size_t);

BaseRepresentation *getNextRepresentation(BaseAdaptationSet *, BaseRepresentation *);

private:
size_t currentBps;
};
}
}

#endif 
