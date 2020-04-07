#if !defined(NVIM_FILEIO_H)
#define NVIM_FILEIO_H

#include "nvim/buffer_defs.h"
#include "nvim/os/os.h"


#define READ_NEW 0x01 
#define READ_FILTER 0x02 
#define READ_STDIN 0x04 
#define READ_BUFFER 0x08 
#define READ_DUMMY 0x10 
#define READ_KEEP_UNDO 0x20 
#define READ_FIFO 0x40 

#define READ_STRING(x, y) (char_u *)read_string((x), (size_t)(y))





typedef struct {
buf_T *save_curbuf; 
int use_aucmd_win; 
win_T *save_curwin; 
win_T *save_prevwin; 
win_T *new_curwin; 
bufref_T new_curbuf; 
char_u *globaldir; 
} aco_save_T;

#if defined(INCLUDE_GENERATED_DECLARATIONS)

#include "auevents_enum.generated.h"
#include "fileio.h.generated.h"
#endif
#endif 
