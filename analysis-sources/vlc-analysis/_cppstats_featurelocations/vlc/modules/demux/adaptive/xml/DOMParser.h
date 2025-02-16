























#if !defined(DOMPARSER_H_)
#define DOMPARSER_H_

#include <vlc_common.h>
#include <vlc_stream.h>

#include "Node.h"

namespace adaptive
{
namespace xml
{
class DOMParser
{
public:
DOMParser ();
DOMParser (stream_t *stream);
virtual ~DOMParser ();

bool parse (bool);
bool reset (stream_t *);
Node* getRootNode ();
void print ();

private:
Node *root;
stream_t *stream;

xml_reader_t *vlc_reader;

Node* processNode (bool);
void addAttributesToNode (Node *node);
void print (Node *node, int offset);
};
}
}

#endif 
