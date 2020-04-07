typedef long linenr_T; 
#define PRIdLINENR "ld"
typedef int colnr_T;
#define PRIdCOLNR "d"
enum { MAXLNUM = 0x7fffffff };
enum { MAXCOL = 0x7fffffff };
enum { MINLNUM = 1 };
enum { MINCOL = 1 };
typedef struct {
linenr_T lnum; 
colnr_T col; 
colnr_T coladd;
} pos_T;
typedef struct {
linenr_T lnum; 
colnr_T col; 
} lpos_T;
