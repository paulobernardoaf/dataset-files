#include "nvim/buffer_defs.h"
#include "nvim/extmark_defs.h"
#include "nvim/marktree.h"
EXTERN int extmark_splice_pending INIT(= 0);
typedef struct
{
uint64_t ns_id;
uint64_t mark_id;
int row;
colnr_T col;
} ExtmarkInfo;
typedef kvec_t(ExtmarkInfo) ExtmarkArray;
typedef struct {
int start_row;
colnr_T start_col;
int oldextent_row;
colnr_T oldextent_col;
int newextent_row;
colnr_T newextent_col;
} ExtmarkSplice;
typedef struct {
int start_row;
int start_col;
int extent_row;
int extent_col;
int new_row;
int new_col;
} ExtmarkMove;
typedef struct {
uint64_t mark; 
int old_row;
colnr_T old_col;
int row;
colnr_T col;
} ExtmarkSavePos;
typedef enum {
kExtmarkSplice,
kExtmarkMove,
kExtmarkUpdate,
kExtmarkSavePos,
kExtmarkClear,
} UndoObjectType;
struct undo_object {
UndoObjectType type;
union {
ExtmarkSplice splice;
ExtmarkMove move;
ExtmarkSavePos savepos;
} data;
};
typedef struct {
int start_row;
int start_col;
int end_row;
int end_col;
int attr_id;
VirtText *virt_text;
} HlRange;
typedef struct {
MarkTreeIter itr[1];
kvec_t(HlRange) active;
int top_row;
int row;
int col_until;
int current;
VirtText *virt_text;
} DecorationRedrawState;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "extmark.h.generated.h"
#endif
