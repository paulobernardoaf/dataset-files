#include <string>
#include <list>
#include "CommonAttributesElements.h"
#include "SegmentInformation.hpp"
#include "../StreamFormat.hpp"
namespace adaptive
{
class SharedResources;
namespace playlist
{
class BaseAdaptationSet;
class AbstractPlaylist;
class BaseSegmentTemplate;
class BaseRepresentation : public CommonAttributesElements,
public SegmentInformation
{
public:
BaseRepresentation( BaseAdaptationSet * );
virtual ~BaseRepresentation ();
virtual StreamFormat getStreamFormat() const;
BaseAdaptationSet* getAdaptationSet();
uint64_t getBandwidth () const;
void setBandwidth ( uint64_t bandwidth );
const std::list<std::string> & getCodecs () const;
void addCodecs (const std::string &);
bool consistentSegmentNumber () const;
virtual void pruneByPlaybackTime (vlc_tick_t);
virtual vlc_tick_t getMinAheadTime (uint64_t) const;
virtual bool needsUpdate () const;
virtual bool runLocalUpdates (SharedResources *);
virtual void scheduleNextUpdate (uint64_t);
virtual void debug (vlc_object_t *,int = 0) const;
virtual std::string contextualize(size_t, const std::string &,
const BaseSegmentTemplate *) const;
static bool bwCompare(const BaseRepresentation *a,
const BaseRepresentation *b);
protected:
virtual bool validateCodec(const std::string &) const;
BaseAdaptationSet *adaptationSet;
uint64_t bandwidth;
std::list<std::string> codecs;
bool b_consistent;
};
}
}
