#if !defined(DIRENT_H)
#define DIRENT_H

typedef struct DIR DIR;

#define DT_UNKNOWN 0
#define DT_DIR 1
#define DT_REG 2
#define DT_LNK 3

struct dirent {
unsigned char d_type; 
char d_name[MAX_PATH * 3]; 
};

DIR *opendir(const char *dirname);
struct dirent *readdir(DIR *dir);
int closedir(DIR *dir);

#endif 
