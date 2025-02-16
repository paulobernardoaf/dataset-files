#if !defined(NVIM_EXTMARK_DEFS_H)
#define NVIM_EXTMARK_DEFS_H

#include "nvim/pos.h" 
#include "nvim/lib/kvec.h"

typedef struct {
char *text;
int hl_id;
} VirtTextChunk;

typedef kvec_t(VirtTextChunk) VirtText;
#define VIRTTEXT_EMPTY ((VirtText)KV_INITIAL_VALUE)

typedef struct
{
uint64_t ns_id;
uint64_t mark_id;
int hl_id; 


VirtText virt_text;
} ExtmarkItem;

typedef struct undo_object ExtmarkUndoObject;
typedef kvec_t(ExtmarkUndoObject) extmark_undo_vec_t;



typedef enum {
kExtmarkNOOP, 
kExtmarkUndo, 
kExtmarkNoUndo, 
kExtmarkUndoNoRedo, 
} ExtmarkOp;

#endif 
