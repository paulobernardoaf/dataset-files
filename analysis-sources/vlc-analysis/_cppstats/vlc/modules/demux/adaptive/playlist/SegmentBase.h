#include "Segment.h"
#include "SegmentInfoCommon.h"
#include "../tools/Properties.hpp"
namespace adaptive
{
namespace playlist
{
class SegmentBase : public Segment,
public Initializable<Segment>,
public Indexable<Segment>
{
public:
SegmentBase (ICanonicalUrl *);
virtual ~SegmentBase ();
};
}
}
