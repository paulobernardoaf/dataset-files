#include <vector>
#include <string>
#include <map>
#include "../../adaptive/playlist/BaseAdaptationSet.h"
#include "DASHCommonAttributesElements.h"
namespace dash
{
namespace mpd
{
class Period;
class Representation;
using namespace adaptive;
class AdaptationSet : public adaptive::playlist::BaseAdaptationSet,
public DASHCommonAttributesElements
{
public:
AdaptationSet(Period *);
virtual ~AdaptationSet();
virtual StreamFormat getStreamFormat() const; 
bool getSubsegmentAlignmentFlag() const;
void setSubsegmentAlignmentFlag( bool alignment );
private:
bool subsegmentAlignmentFlag;
};
}
}
