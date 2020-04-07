#if !defined(NVIM_MEMFILE_DEFS_H)
#define NVIM_MEMFILE_DEFS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "nvim/types.h"
#include "nvim/pos.h"






typedef int64_t blocknr_T;








typedef struct mf_hashitem {
struct mf_hashitem *mhi_next;
struct mf_hashitem *mhi_prev;
blocknr_T mhi_key;
} mf_hashitem_T;


#define MHT_INIT_SIZE 64







typedef struct mf_hashtab {
size_t mht_mask; 

size_t mht_count; 
mf_hashitem_T **mht_buckets; 


mf_hashitem_T *mht_small_buckets[MHT_INIT_SIZE]; 
} mf_hashtab_T;














typedef struct bhdr {
mf_hashitem_T bh_hashitem; 
#define bh_bnum bh_hashitem.mhi_key 

struct bhdr *bh_next; 
struct bhdr *bh_prev; 
void *bh_data; 
unsigned bh_page_count; 

#define BH_DIRTY 1U
#define BH_LOCKED 2U
unsigned bh_flags; 
} bhdr_T;







typedef struct mf_blocknr_trans_item {
mf_hashitem_T nt_hashitem; 
#define nt_old_bnum nt_hashitem.mhi_key 
blocknr_T nt_new_bnum; 
} mf_blocknr_trans_item_T;


typedef struct memfile {
char_u *mf_fname; 
char_u *mf_ffname; 
int mf_fd; 
bhdr_T *mf_free_first; 
bhdr_T *mf_used_first; 
bhdr_T *mf_used_last; 
mf_hashtab_T mf_hash; 
mf_hashtab_T mf_trans; 
blocknr_T mf_blocknr_max; 
blocknr_T mf_blocknr_min; 
blocknr_T mf_neg_count; 
blocknr_T mf_infile_count; 
unsigned mf_page_size; 
bool mf_dirty; 
} memfile_T;

#endif 
