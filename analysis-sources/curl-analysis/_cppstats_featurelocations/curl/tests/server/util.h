#if !defined(HEADER_CURL_SERVER_UTIL_H)
#define HEADER_CURL_SERVER_UTIL_H





















#include "server_setup.h"

char *data_to_hex(char *data, size_t len);
void logmsg(const char *msg, ...);
long timediff(struct timeval newer, struct timeval older);

#define TEST_DATA_PATH "%s/data/test%ld"

#define SERVERLOGS_LOCK "log/serverlogs.lock"


extern const char *path;


extern const char *serverlogfile;

#if defined(WIN32) || defined(_WIN32)
#include <process.h>
#include <fcntl.h>

#define sleep(sec) Sleep ((sec)*1000)

#undef perror
#define perror(m) win32_perror(m)
void win32_perror(const char *msg);
#endif 

#if defined(USE_WINSOCK)
void win32_init(void);
void win32_cleanup(void);
#endif 


char *test2file(long testno);

int wait_ms(int timeout_ms);

int write_pidfile(const char *filename);

void set_advisor_read_lock(const char *filename);

void clear_advisor_read_lock(const char *filename);

int strncasecompare(const char *first, const char *second, size_t max);

#endif 
