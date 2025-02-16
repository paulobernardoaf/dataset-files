#include "DASHCommonAttributesElements.h"
#include "../../adaptive/playlist/SegmentInfoCommon.h"
#include "../../adaptive/playlist/BaseRepresentation.h"
namespace dash
{
namespace mpd
{
class AdaptationSet;
class MPD;
using namespace adaptive;
using namespace adaptive::playlist;
class Representation : public BaseRepresentation,
public DASHCommonAttributesElements
{
public:
Representation( AdaptationSet * );
virtual ~Representation ();
virtual StreamFormat getStreamFormat() const; 
virtual std::string contextualize(size_t, const std::string &,
const BaseSegmentTemplate *) const; 
private:
stime_t getScaledTimeBySegmentNumber(uint64_t, const MediaSegmentTemplate *) const;
};
}
}
