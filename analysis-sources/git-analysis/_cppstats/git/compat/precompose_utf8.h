#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <iconv.h>
typedef struct dirent_prec_psx {
ino_t d_ino; 
size_t max_name_len; 
unsigned char d_type; 
char d_name[NAME_MAX+1];
} dirent_prec_psx;
typedef struct {
iconv_t ic_precompose;
DIR *dirp;
struct dirent_prec_psx *dirent_nfc;
} PREC_DIR;
void precompose_argv(int argc, const char **argv);
void probe_utf8_pathname_composition(void);
PREC_DIR *precompose_utf8_opendir(const char *dirname);
struct dirent_prec_psx *precompose_utf8_readdir(PREC_DIR *dirp);
int precompose_utf8_closedir(PREC_DIR *dirp);
#if !defined(PRECOMPOSE_UNICODE_C)
#define dirent dirent_prec_psx
#define opendir(n) precompose_utf8_opendir(n)
#define readdir(d) precompose_utf8_readdir(d)
#define closedir(d) precompose_utf8_closedir(d)
#define DIR PREC_DIR
#endif 
