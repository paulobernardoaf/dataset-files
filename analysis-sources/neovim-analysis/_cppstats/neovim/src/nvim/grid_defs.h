#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "nvim/types.h"
#define MAX_MCO 6 
typedef char_u schar_T[(MAX_MCO+1) * 4 + 1];
typedef int16_t sattr_T;
typedef struct {
handle_T handle;
schar_T *chars;
sattr_T *attrs;
unsigned *line_offset;
char_u *line_wraps;
int *dirty_col;
int Rows;
int Columns;
bool valid;
bool throttled;
int row_offset;
int col_offset;
bool blending;
bool focusable;
int comp_row;
int comp_col;
size_t comp_index;
bool comp_disabled;
} ScreenGrid;
#define SCREEN_GRID_INIT { 0, NULL, NULL, NULL, NULL, NULL, 0, 0, false, false, 0, 0, false, true, 0, 0, 0, false }
