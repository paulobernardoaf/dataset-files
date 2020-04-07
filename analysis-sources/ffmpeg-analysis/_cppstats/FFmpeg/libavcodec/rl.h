#include <stdint.h>
#include "vlc.h"
#define MAX_RUN 64
#define MAX_LEVEL 64
typedef struct RLTable {
int n; 
int last; 
const uint16_t (*table_vlc)[2];
const int8_t *table_run;
const int8_t *table_level;
uint8_t *index_run[2]; 
int8_t *max_level[2]; 
int8_t *max_run[2]; 
RL_VLC_ELEM *rl_vlc[32]; 
} RLTable;
int ff_rl_init(RLTable *rl, uint8_t static_store[2][2*MAX_RUN + MAX_LEVEL + 3]);
void ff_rl_init_vlc(RLTable *rl, unsigned static_size);
void ff_rl_free(RLTable *rl);
#define INIT_VLC_RL(rl, static_size){int q;static RL_VLC_ELEM rl_vlc_table[32][static_size];if(!rl.rl_vlc[0]){for(q=0; q<32; q++)rl.rl_vlc[q]= rl_vlc_table[q];ff_rl_init_vlc(&rl, static_size);}}
static inline int get_rl_index(const RLTable *rl, int last, int run, int level)
{
int index;
index = rl->index_run[last][run];
if (index >= rl->n)
return rl->n;
if (level > rl->max_level[last][run])
return rl->n;
return index + level - 1;
}
