#include <stdbool.h>
#include "nvim/types.h"
#include "nvim/buffer_defs.h"
#include "nvim/grid_defs.h"
#include "nvim/pos.h"
#define VALID 10 
#define INVERTED 20 
#define INVERTED_ALL 25 
#define REDRAW_TOP 30 
#define SOME_VALID 35 
#define NOT_VALID 40 
#define CLEAR 50 
EXTERN ScreenGrid default_grid INIT(= SCREEN_GRID_INIT);
#define DEFAULT_GRID_HANDLE 1 
typedef struct {
enum {
kStlClickDisabled = 0, 
kStlClickTabSwitch, 
kStlClickTabClose, 
kStlClickFuncRun, 
} type; 
int tabnr; 
char *func; 
} StlClickDefinition;
typedef struct {
StlClickDefinition def; 
const char *start; 
} StlClickRecord;
extern StlClickDefinition *tab_page_click_defs;
extern long tab_page_click_defs_size;
#define W_ENDCOL(wp) (wp->w_wincol + wp->w_width)
#define W_ENDROW(wp) (wp->w_winrow + wp->w_height)
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "screen.h.generated.h"
#endif
