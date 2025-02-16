























#if !defined(HTTPCONNECTIONMANAGER_H_)
#define HTTPCONNECTIONMANAGER_H_

#include "../logic/IDownloadRateObserver.h"

#include <vlc_common.h>

#include <vector>
#include <string>

namespace adaptive
{
namespace http
{
class ConnectionParams;
class AbstractConnectionFactory;
class AbstractConnection;
class AuthStorage;
class Downloader;
class AbstractChunkSource;

class AbstractConnectionManager : public IDownloadRateObserver
{
public:
AbstractConnectionManager(vlc_object_t *);
~AbstractConnectionManager();
virtual void closeAllConnections () = 0;
virtual AbstractConnection * getConnection(ConnectionParams &) = 0;
virtual void start(AbstractChunkSource *) = 0;
virtual void cancel(AbstractChunkSource *) = 0;

virtual void updateDownloadRate(const ID &, size_t, vlc_tick_t); 
void setDownloadRateObserver(IDownloadRateObserver *);

protected:
vlc_object_t *p_object;

private:
IDownloadRateObserver *rateObserver;
};

class HTTPConnectionManager : public AbstractConnectionManager
{
public:
HTTPConnectionManager (vlc_object_t *p_object, AuthStorage *);
virtual ~HTTPConnectionManager ();

virtual void closeAllConnections () ;
virtual AbstractConnection * getConnection(ConnectionParams &) ;

virtual void start(AbstractChunkSource *) ;
virtual void cancel(AbstractChunkSource *) ;
void setLocalConnectionsAllowed();

private:
void releaseAllConnections ();
Downloader *downloader;
vlc_mutex_t lock;
std::vector<AbstractConnection *> connectionPool;
AbstractConnectionFactory *factory;
bool localAllowed;
AbstractConnection * reuseConnection(ConnectionParams &);
};
}
}

#endif 
