






















#if !defined(BASEPERIOD_H_)
#define BASEPERIOD_H_

#include <vector>

#include "BaseAdaptationSet.h"
#include "SegmentInformation.hpp"
#include "../tools/Properties.hpp"

namespace adaptive
{
namespace playlist
{
class BasePeriod : public SegmentInformation
{
public:
BasePeriod(AbstractPlaylist *);
virtual ~BasePeriod ();

const std::vector<BaseAdaptationSet *>& getAdaptationSets () const;
BaseAdaptationSet * getAdaptationSetByID(const ID &);
void addAdaptationSet (BaseAdaptationSet *AdaptationSet);
void debug (vlc_object_t *,int = 0) const;

virtual vlc_tick_t getPeriodStart() const; 
virtual AbstractPlaylist *getPlaylist() const; 

Property<vlc_tick_t> duration;
Property<vlc_tick_t> startTime;

private:
std::vector<BaseAdaptationSet *> adaptationSets;
AbstractPlaylist *playlist;
};
}
}

#endif 
