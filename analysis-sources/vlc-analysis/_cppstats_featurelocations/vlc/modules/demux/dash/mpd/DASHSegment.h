























#if !defined(DASHSEGMENT_H_)
#define DASHSEGMENT_H_

#include "../../adaptive/playlist/Segment.h"
#include "../../adaptive/playlist/SegmentChunk.hpp"

namespace dash
{
namespace mpd
{
using namespace adaptive::playlist;
using namespace adaptive::http;

class DashIndexChunk : public SegmentChunk
{
public:
DashIndexChunk(AbstractChunkSource *, BaseRepresentation *);
~DashIndexChunk();
virtual void onDownload(block_t **); 
};

class DashIndexSegment : public IndexSegment
{
public:
DashIndexSegment( ICanonicalUrl *parent );
~DashIndexSegment();
virtual SegmentChunk* createChunk(AbstractChunkSource *, BaseRepresentation *); 
};

}
}

#endif 
