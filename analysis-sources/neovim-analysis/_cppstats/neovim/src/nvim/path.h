#include "nvim/func_attr.h"
#include "nvim/types.h"
#include "nvim/garray.h"
#define EW_DIR 0x01 
#define EW_FILE 0x02 
#define EW_NOTFOUND 0x04 
#define EW_ADDSLASH 0x08 
#define EW_KEEPALL 0x10 
#define EW_SILENT 0x20 
#define EW_EXEC 0x40 
#define EW_PATH 0x80 
#define EW_ICASE 0x100 
#define EW_NOERROR 0x200 
#define EW_NOTWILD 0x400 
#define EW_KEEPDOLLAR 0x800 
#define EW_ALLLINKS 0x1000 
#define EW_SHELLCMD 0x2000 
#define EW_DODOT 0x4000 
#define EW_EMPTYOK 0x8000 
#define EW_NOTENV 0x10000 
typedef enum file_comparison {
kEqualFiles = 1, 
kDifferentFiles = 2, 
kBothFilesMissing = 4, 
kOneFileMissing = 6, 
kEqualFileNames = 7 
} FileComparison;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "path.h.generated.h"
#endif
