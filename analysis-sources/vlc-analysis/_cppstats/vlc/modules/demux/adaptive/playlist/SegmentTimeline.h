#include "SegmentInfoCommon.h"
#include <vlc_common.h>
#include <list>
namespace adaptive
{
namespace playlist
{
class SegmentTimeline : public TimescaleAble
{
class Element;
public:
SegmentTimeline(TimescaleAble *);
SegmentTimeline(uint64_t);
virtual ~SegmentTimeline();
void addElement(uint64_t, stime_t d, uint64_t r = 0, stime_t t = 0);
uint64_t getElementNumberByScaledPlaybackTime(stime_t) const;
bool getScaledPlaybackTimeDurationBySegmentNumber(uint64_t, stime_t *, stime_t *) const;
stime_t getScaledPlaybackTimeByElementNumber(uint64_t) const;
stime_t getMinAheadScaledTime(uint64_t) const;
stime_t getTotalLength() const;
uint64_t maxElementNumber() const;
uint64_t minElementNumber() const;
void pruneByPlaybackTime(vlc_tick_t);
size_t pruneBySequenceNumber(uint64_t);
void updateWith(SegmentTimeline &);
void debug(vlc_object_t *, int = 0) const;
private:
std::list<Element *> elements;
stime_t totalLength;
class Element
{
public:
Element(uint64_t, stime_t, uint64_t, stime_t);
void debug(vlc_object_t *, int = 0) const;
bool contains(stime_t) const;
stime_t t;
stime_t d;
uint64_t r;
uint64_t number;
};
};
}
}
