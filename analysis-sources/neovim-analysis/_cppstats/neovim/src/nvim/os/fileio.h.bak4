#if !defined(NVIM_OS_FILEIO_H)
#define NVIM_OS_FILEIO_H

#include <stdbool.h>
#include <stddef.h>

#include "nvim/func_attr.h"
#include "nvim/rbuffer.h"


typedef struct {
int fd; 
int _error; 
RBuffer *rv; 
bool wr; 
bool eof; 
bool non_blocking; 
} FileDescriptor;


typedef enum {
kFileReadOnly = 1, 
kFileCreate = 2, 

kFileWriteOnly = 4, 

kFileNoSymlink = 8, 
kFileCreateOnly = 16, 


kFileTruncate = 32, 


kFileAppend = 64, 

kFileNonBlocking = 128, 

} FileOpenFlags;

static inline bool file_eof(const FileDescriptor *const fp)
REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT REAL_FATTR_NONNULL_ALL;







static inline bool file_eof(const FileDescriptor *const fp)
{
return fp->eof && rbuffer_size(fp->rv) == 0;
}

static inline int file_fd(const FileDescriptor *const fp)
REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT REAL_FATTR_NONNULL_ALL;






static inline int file_fd(const FileDescriptor *const fp)
{
return fp->fd;
}

enum {



kRWBufferSize = 1024
};

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "os/fileio.h.generated.h"
#endif
#endif 
