























#if !defined(DASHMANAGER_H_)
#define DASHMANAGER_H_

#include "../adaptive/PlaylistManager.h"
#include "../adaptive/logic/AbstractAdaptationLogic.h"
#include "mpd/MPD.h"

namespace adaptive
{
namespace xml
{
class Node;
}
}

namespace dash
{
using namespace adaptive;

class DASHManager : public PlaylistManager
{
public:
DASHManager( demux_t *,
SharedResources *,
mpd::MPD *mpd,
AbstractStreamFactory *,
logic::AbstractAdaptationLogic::LogicType type);
virtual ~DASHManager ();

virtual bool needsUpdate() const; 
virtual bool updatePlaylist(); 
virtual void scheduleNextUpdate();
static bool isDASH(xml::Node *);
static bool mimeMatched(const std::string &);

protected:
virtual int doControl(int, va_list); 
};

}

#endif 
