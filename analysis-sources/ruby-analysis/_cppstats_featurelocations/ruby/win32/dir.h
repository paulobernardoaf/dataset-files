#if !defined(RUBY_WIN32_DIR_H)
#define RUBY_WIN32_DIR_H
#include <stdint.h> 
#include <esent.h> 
#include "ruby/encoding.h" 

#define DT_UNKNOWN 0
#define DT_DIR (S_IFDIR>>12)
#define DT_REG (S_IFREG>>12)
#define DT_LNK 10

struct direct
{
long d_namlen;
ino_t d_ino;
char *d_name;
char *d_altname; 
short d_altlen;
uint8_t d_type;
};
typedef struct {
WCHAR *start;
WCHAR *curr;
long size;
long nfiles;
long loc; 
struct direct dirstr;
char *bits; 
} DIR;


DIR* rb_w32_opendir(const char*);
DIR* rb_w32_uopendir(const char*);
struct direct* rb_w32_readdir(DIR *, rb_encoding *);
long rb_w32_telldir(DIR *);
void rb_w32_seekdir(DIR *, long);
void rb_w32_rewinddir(DIR *);
void rb_w32_closedir(DIR *);
char *rb_w32_ugetcwd(char *, int);

#define opendir(s) rb_w32_opendir((s))
#define readdir(d) rb_w32_readdir((d), 0)
#define telldir(d) rb_w32_telldir((d))
#define seekdir(d, l) rb_w32_seekdir((d), (l))
#define rewinddir(d) rb_w32_rewinddir((d))
#define closedir(d) rb_w32_closedir((d))

#endif 
