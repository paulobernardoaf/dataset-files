#include "BytesRange.hpp"
#include "ConnectionParams.hpp"
#include "../ID.hpp"
#include <vector>
#include <string>
#include <stdint.h>
typedef struct block_t block_t;
namespace adaptive
{
namespace http
{
class AbstractConnection;
class AbstractConnectionManager;
class AbstractChunk;
class AbstractChunkSource
{
public:
AbstractChunkSource();
virtual ~AbstractChunkSource();
virtual block_t * readBlock () = 0;
virtual block_t * read (size_t) = 0;
virtual bool hasMoreData () const = 0;
void setBytesRange (const BytesRange &);
const BytesRange & getBytesRange () const;
virtual std::string getContentType () const;
enum RequestStatus getRequestStatus() const;
protected:
enum RequestStatus requeststatus;
size_t contentLength;
BytesRange bytesRange;
};
class AbstractChunk
{
public:
virtual ~AbstractChunk();
std::string getContentType ();
enum RequestStatus getRequestStatus () const;
size_t getBytesRead () const;
uint64_t getStartByteInFile () const;
bool isEmpty () const;
virtual block_t * readBlock ();
virtual block_t * read (size_t);
protected:
AbstractChunk(AbstractChunkSource *);
AbstractChunkSource *source;
virtual void onDownload (block_t **) = 0;
private:
size_t bytesRead;
block_t * doRead(size_t, bool);
};
class HTTPChunkSource : public AbstractChunkSource,
public BackendPrefInterface
{
public:
HTTPChunkSource(const std::string &url, AbstractConnectionManager *,
const ID &, bool = false);
virtual ~HTTPChunkSource();
virtual block_t * readBlock (); 
virtual block_t * read (size_t); 
virtual bool hasMoreData () const; 
virtual std::string getContentType () const; 
static const size_t CHUNK_SIZE = 32768;
protected:
virtual bool prepare();
AbstractConnection *connection;
AbstractConnectionManager *connManager;
mutable vlc_mutex_t lock;
size_t consumed; 
bool prepared;
bool eof;
ID sourceid;
private:
bool init(const std::string &);
ConnectionParams params;
};
class HTTPChunkBufferedSource : public HTTPChunkSource
{
friend class Downloader;
public:
HTTPChunkBufferedSource(const std::string &url, AbstractConnectionManager *,
const ID &, bool = false);
virtual ~HTTPChunkBufferedSource();
virtual block_t * readBlock (); 
virtual block_t * read (size_t); 
virtual bool hasMoreData () const; 
void hold();
void release();
protected:
virtual bool prepare(); 
void bufferize(size_t);
bool isDone() const;
private:
block_t *p_head; 
block_t **pp_tail;
size_t buffered; 
bool done;
bool eof;
vlc_tick_t downloadstart;
vlc_cond_t avail;
bool held;
};
class HTTPChunk : public AbstractChunk
{
public:
HTTPChunk(const std::string &url, AbstractConnectionManager *,
const ID &, bool = false);
virtual ~HTTPChunk();
protected:
virtual void onDownload (block_t **) {} 
};
}
}
