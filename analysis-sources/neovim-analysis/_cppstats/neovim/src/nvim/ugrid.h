#include "nvim/ui.h"
#include "nvim/globals.h"
typedef struct ucell UCell;
typedef struct ugrid UGrid;
#define CELLBYTES (sizeof(schar_T))
struct ucell {
char data[CELLBYTES + 1];
sattr_T attr;
};
struct ugrid {
int row, col;
int width, height;
UCell **cells;
};
#define UGRID_FOREACH_CELL(grid, row, startcol, endcol, code) do { UCell *row_cells = (grid)->cells[row]; for (int curcol = startcol; curcol < endcol; curcol++) { UCell *cell = row_cells + curcol; (void)(cell); code; } } while (0)
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "ugrid.h.generated.h"
#endif
