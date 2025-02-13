#if !defined(HEADER_CURL_TOOL_CB_PRG_H)
#define HEADER_CURL_TOOL_CB_PRG_H





















#include "tool_setup.h"

#define CURL_PROGRESS_STATS 0 
#define CURL_PROGRESS_BAR 1

struct ProgressData {
int calls;
curl_off_t prev;
struct timeval prevtime;
int width;
FILE *out; 
curl_off_t initial_size;
unsigned int tick;
int bar;
int barmove;
};

void progressbarinit(struct ProgressData *bar,
struct OperationConfig *config);





int tool_progress_cb(void *clientp,
curl_off_t dltotal, curl_off_t dlnow,
curl_off_t ultotal, curl_off_t ulnow);

#endif 
