#include "SegmentInfoCommon.h"
namespace adaptive
{
namespace playlist
{
class SegmentInformation;
class Segment;
class SegmentList : public SegmentInfoCommon,
public TimescaleAble
{
public:
SegmentList ( SegmentInformation * = NULL );
virtual ~SegmentList ();
const std::vector<ISegment *>& getSegments() const;
ISegment * getSegmentByNumber(uint64_t);
void addSegment(ISegment *seg);
void updateWith(SegmentList *, bool = false);
void pruneBySegmentNumber(uint64_t);
void pruneByPlaybackTime(vlc_tick_t);
bool getSegmentNumberByScaledTime(stime_t, uint64_t *) const;
bool getPlaybackTimeDurationBySegmentNumber(uint64_t, vlc_tick_t *, vlc_tick_t *) const;
stime_t getTotalLength() const;
private:
std::vector<ISegment *> segments;
stime_t totalLength;
};
}
}
