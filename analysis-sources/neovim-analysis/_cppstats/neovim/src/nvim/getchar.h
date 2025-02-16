#include "nvim/os/fileio.h"
#include "nvim/types.h"
#include "nvim/buffer_defs.h"
#include "nvim/ex_cmds_defs.h"
#include "nvim/vim.h"
enum RemapValues {
REMAP_YES = 0, 
REMAP_NONE = -1, 
REMAP_SCRIPT = -2, 
REMAP_SKIP = -3, 
};
typedef enum {
FLUSH_MINIMAL,
FLUSH_TYPEAHEAD, 
FLUSH_INPUT 
} flush_buffers_T;
struct map_arguments {
bool buffer;
bool expr;
bool noremap;
bool nowait;
bool script;
bool silent;
bool unique;
char_u lhs[MAXMAPLEN + 1];
size_t lhs_len;
char_u *rhs; 
size_t rhs_len;
bool rhs_is_noop; 
char_u *orig_rhs; 
size_t orig_rhs_len;
};
typedef struct map_arguments MapArguments;
#define KEYLEN_PART_KEY -1 
#define KEYLEN_PART_MAP -2 
enum { NSCRIPT = 15 };
extern FileDescriptor *scriptin[NSCRIPT];
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "getchar.h.generated.h"
#endif
