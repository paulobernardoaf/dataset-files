#include "tool_setup.h"
typedef enum {
TOOLMIME_NONE = 0,
TOOLMIME_PARTS,
TOOLMIME_DATA,
TOOLMIME_FILE,
TOOLMIME_FILEDATA,
TOOLMIME_STDIN,
TOOLMIME_STDINDATA
} toolmimekind;
typedef struct tool_mime tool_mime;
struct tool_mime {
toolmimekind kind; 
tool_mime *parent; 
tool_mime *prev; 
const char *data; 
const char *name; 
const char *filename; 
const char *type; 
const char *encoder; 
struct curl_slist *headers; 
tool_mime *subparts; 
curl_off_t origin; 
curl_off_t size; 
curl_off_t curpos; 
struct GlobalConfig *config; 
};
size_t tool_mime_stdin_read(char *buffer,
size_t size, size_t nitems, void *arg);
int tool_mime_stdin_seek(void *instream, curl_off_t offset, int whence);
int formparse(struct OperationConfig *config,
const char *input,
tool_mime **mimeroot,
tool_mime **mimecurrent,
bool literal_value);
CURLcode tool2curlmime(CURL *curl, tool_mime *m, curl_mime **mime);
void tool_mime_free(tool_mime *mime);
