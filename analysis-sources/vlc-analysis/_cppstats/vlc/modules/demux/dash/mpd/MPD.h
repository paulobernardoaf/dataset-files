#include "../../adaptive/playlist/AbstractPlaylist.hpp"
#include "../../adaptive/StreamFormat.hpp"
#include "Profile.hpp"
namespace dash
{
namespace mpd
{
using namespace adaptive::playlist;
using namespace adaptive;
class ProgramInformation;
class MPD : public AbstractPlaylist
{
friend class IsoffMainParser;
public:
MPD(vlc_object_t *, Profile);
virtual ~MPD();
Profile getProfile() const;
virtual bool isLive() const;
virtual bool isLowLatency() const;
void setLowLatency(bool);
virtual void debug();
Property<ProgramInformation *> programInfo;
private:
Profile profile;
bool lowLatency;
};
}
}
