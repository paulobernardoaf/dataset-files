#include "../../adaptive/playlist/BasePeriod.h"
#include "../../adaptive/playlist/SegmentInformation.hpp"
namespace dash
{
namespace mpd
{
class MPD;
using namespace adaptive;
using namespace adaptive::playlist;
class Period : public BasePeriod
{
public:
Period(MPD *);
virtual ~Period ();
};
}
}
