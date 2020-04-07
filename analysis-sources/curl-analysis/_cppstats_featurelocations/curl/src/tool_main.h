#if !defined(HEADER_CURL_TOOL_MAIN_H)
#define HEADER_CURL_TOOL_MAIN_H





















#include "tool_setup.h"

#define DEFAULT_MAXREDIRS 50L

#define RETRY_SLEEP_DEFAULT 1000L 
#define RETRY_SLEEP_MAX 600000L 

#define MAX_PARALLEL 300 
#define PARALLEL_DEFAULT 50

#if !defined(STDIN_FILENO)
#define STDIN_FILENO fileno(stdin)
#endif

#if !defined(STDOUT_FILENO)
#define STDOUT_FILENO fileno(stdout)
#endif

#if !defined(STDERR_FILENO)
#define STDERR_FILENO fileno(stderr)
#endif

#endif 
