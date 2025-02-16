





















#if !defined(SEGMENTTEMPLATE_H)
#define SEGMENTTEMPLATE_H

#include "Segment.h"
#include "../tools/Properties.hpp"
#include "SegmentInfoCommon.h"

namespace adaptive
{
namespace playlist
{
class ICanonicalUrl;
class InitSegmentTemplate;
class SegmentInformation;
class SegmentTimeline;

class BaseSegmentTemplate : public Segment
{
public:
BaseSegmentTemplate( ICanonicalUrl * = NULL );
virtual ~BaseSegmentTemplate();
virtual void setSourceUrl( const std::string &url ); 
};

class MediaSegmentTemplate : public BaseSegmentTemplate,
public Initializable<InitSegmentTemplate>,
public TimescaleAble
{
public:
MediaSegmentTemplate( SegmentInformation * = NULL );
virtual ~MediaSegmentTemplate();
void setStartNumber( uint64_t );
void setSegmentTimeline( SegmentTimeline * );
void updateWith( MediaSegmentTemplate * );
virtual uint64_t getSequenceNumber() const; 
uint64_t getLiveTemplateNumber(vlc_tick_t, bool = true) const;
stime_t getMinAheadScaledTime(uint64_t) const;
void pruneByPlaybackTime(vlc_tick_t);
size_t pruneBySequenceNumber(uint64_t);
virtual Timescale inheritTimescale() const; 
virtual uint64_t inheritStartNumber() const;
stime_t inheritDuration() const;
SegmentTimeline * inheritSegmentTimeline() const;
virtual void debug(vlc_object_t *, int = 0) const; 

protected:
uint64_t startNumber;
SegmentTimeline *segmentTimeline;
SegmentInformation *parentSegmentInformation;
};

class InitSegmentTemplate : public BaseSegmentTemplate
{
public:
InitSegmentTemplate( ICanonicalUrl * = NULL );
};
}
}
#endif 
