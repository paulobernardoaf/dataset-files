#if !defined(NVIM_HASHTAB_H)
#define NVIM_HASHTAB_H

#include <stddef.h>

#include "nvim/types.h"




extern char hash_removed;


typedef size_t hash_T;



#define HI_KEY_REMOVED ((char_u *)&hash_removed)
#define HASHITEM_EMPTY(hi) ((hi)->hi_key == NULL || (hi)->hi_key == (char_u *)&hash_removed)


















typedef struct hashitem_S {

hash_T hi_hash;







char_u *hi_key;
} hashitem_T;




#define HT_INIT_SIZE 16







typedef struct hashtable_S {
hash_T ht_mask; 

size_t ht_used; 
size_t ht_filled; 
int ht_locked; 
hashitem_T *ht_array; 

hashitem_T ht_smallarray[HT_INIT_SIZE]; 
} hashtab_T;






#define HASHTAB_ITER(ht, hi, code) do { hashtab_T *const hi##ht_ = (ht); size_t hi##todo_ = hi##ht_->ht_used; for (hashitem_T *hi = hi##ht_->ht_array; hi##todo_; hi++) { if (!HASHITEM_EMPTY(hi)) { hi##todo_--; { code } } } } while (0)













#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "hashtab.h.generated.h"
#endif

#endif 
