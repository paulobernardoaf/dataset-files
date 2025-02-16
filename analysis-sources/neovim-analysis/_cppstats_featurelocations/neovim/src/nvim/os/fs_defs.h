#if !defined(NVIM_OS_FS_DEFS_H)
#define NVIM_OS_FS_DEFS_H

#include <uv.h>


typedef struct {
uv_stat_t stat; 
} FileInfo;


typedef struct {
uint64_t inode; 
uint64_t device_id; 
} FileID;

#define FILE_ID_EMPTY (FileID) { .inode = 0, .device_id = 0 }

typedef struct {
uv_fs_t request; 
uv_dirent_t ent; 
} Directory;


#define NODE_NORMAL 0 
#define NODE_WRITABLE 1 

#define NODE_OTHER 2 

#endif 
