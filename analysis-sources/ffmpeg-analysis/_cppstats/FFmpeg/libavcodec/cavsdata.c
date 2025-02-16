#include "cavs.h"
const uint8_t ff_cavs_partition_flags[30] = {
0, 
0, 
0, 
SPLITH, 
SPLITV, 
SPLITH|SPLITV, 
SPLITH|SPLITV, 
SPLITH|SPLITV, 
0, 
0, 
0, 
FWD0|FWD1 |SPLITH,
FWD0|FWD1 |SPLITV,
BWD0|BWD1 |SPLITH,
BWD0|BWD1 |SPLITV,
FWD0|BWD1 |SPLITH,
FWD0|BWD1 |SPLITV,
BWD0|FWD1 |SPLITH,
BWD0|FWD1 |SPLITV,
FWD0|FWD1 |SYM1|SPLITH,
FWD0|FWD1 |SYM1 |SPLITV,
BWD0|FWD1 |SYM1|SPLITH,
BWD0|FWD1 |SYM1 |SPLITV,
FWD0|FWD1|SYM0 |SPLITH,
FWD0|FWD1|SYM0 |SPLITV,
FWD0|BWD1|SYM0 |SPLITH,
FWD0|BWD1|SYM0 |SPLITV,
FWD0|FWD1|SYM0|SYM1|SPLITH,
FWD0|FWD1|SYM0|SYM1 |SPLITV,
SPLITH|SPLITV, 
};
const uint8_t ff_cavs_chroma_qp[64] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 42, 43, 43, 44, 44,
45, 45, 46, 46, 47, 47, 48, 48, 48, 49, 49, 49, 50, 50, 50, 51
};
const cavs_vector ff_cavs_dir_mv = {0,0,1,REF_DIR};
const cavs_vector ff_cavs_intra_mv = {0,0,1,REF_INTRA};
