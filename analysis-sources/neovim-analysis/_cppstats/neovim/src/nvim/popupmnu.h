#include "nvim/vim.h"
#include "nvim/macros.h"
#include "nvim/grid_defs.h"
#include "nvim/types.h"
typedef struct {
char_u *pum_text; 
char_u *pum_kind; 
char_u *pum_extra; 
char_u *pum_info; 
} pumitem_T;
EXTERN ScreenGrid pum_grid INIT(= SCREEN_GRID_INIT);
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "popupmnu.h.generated.h"
#endif
