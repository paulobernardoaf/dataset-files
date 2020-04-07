







































#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>

#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/memfile.h"
#include "nvim/fileio.h"
#include "nvim/memline.h"
#include "nvim/message.h"
#include "nvim/memory.h"
#include "nvim/os_unix.h"
#include "nvim/path.h"
#include "nvim/assert.h"
#include "nvim/os/os.h"
#include "nvim/os/input.h"

#define MEMFILE_PAGE_SIZE 4096 


#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "memfile.c.generated.h"
#endif














memfile_T *mf_open(char_u *fname, int flags)
{
memfile_T *mfp = xmalloc(sizeof(memfile_T));

if (fname == NULL) { 
mfp->mf_fname = NULL;
mfp->mf_ffname = NULL;
mfp->mf_fd = -1;
} else { 
if (!mf_do_open(mfp, fname, flags)) {
xfree(mfp);
return NULL; 
}
}

mfp->mf_free_first = NULL; 
mfp->mf_used_first = NULL; 
mfp->mf_used_last = NULL;
mfp->mf_dirty = false;
mf_hash_init(&mfp->mf_hash);
mf_hash_init(&mfp->mf_trans);
mfp->mf_page_size = MEMFILE_PAGE_SIZE;


FileInfo file_info;
if (mfp->mf_fd >= 0 && os_fileinfo_fd(mfp->mf_fd, &file_info)) {
uint64_t blocksize = os_fileinfo_blocksize(&file_info);
if (blocksize >= MIN_SWAP_PAGE_SIZE && blocksize <= MAX_SWAP_PAGE_SIZE) {
STATIC_ASSERT(MAX_SWAP_PAGE_SIZE <= UINT_MAX,
"MAX_SWAP_PAGE_SIZE must fit into an unsigned");
mfp->mf_page_size = (unsigned)blocksize;
}
}

off_T size;




if (mfp->mf_fd < 0
|| (flags & (O_TRUNC|O_EXCL))
|| (size = vim_lseek(mfp->mf_fd, 0L, SEEK_END)) <= 0) {

mfp->mf_blocknr_max = 0;
} else {
assert(sizeof(off_T) <= sizeof(blocknr_T)
&& mfp->mf_page_size > 0
&& mfp->mf_page_size - 1 <= INT64_MAX - size);
mfp->mf_blocknr_max = (((blocknr_T)size + mfp->mf_page_size - 1)
/ mfp->mf_page_size);
}
mfp->mf_blocknr_min = -1;
mfp->mf_neg_count = 0;
mfp->mf_infile_count = mfp->mf_blocknr_max;

return mfp;
}














int mf_open_file(memfile_T *mfp, char_u *fname)
{
if (mf_do_open(mfp, fname, O_RDWR | O_CREAT | O_EXCL)) {
mfp->mf_dirty = true;
return OK;
}

return FAIL;
}




void mf_close(memfile_T *mfp, bool del_file)
{
if (mfp == NULL) { 
return;
}
if (mfp->mf_fd >= 0 && close(mfp->mf_fd) < 0) {
EMSG(_(e_swapclose));
}
if (del_file && mfp->mf_fname != NULL) {
os_remove((char *)mfp->mf_fname);
}


for (bhdr_T *hp = mfp->mf_used_first, *nextp; hp != NULL; hp = nextp) {
nextp = hp->bh_next;
mf_free_bhdr(hp);
}
while (mfp->mf_free_first != NULL) { 
xfree(mf_rem_free(mfp));
}
mf_hash_free(&mfp->mf_hash);
mf_hash_free_all(&mfp->mf_trans); 
mf_free_fnames(mfp);
xfree(mfp);
}




void mf_close_file(buf_T *buf, bool getlines)
{
memfile_T *mfp = buf->b_ml.ml_mfp;
if (mfp == NULL || mfp->mf_fd < 0) { 
return;
}

if (getlines) {

for (linenr_T lnum = 1; lnum <= buf->b_ml.ml_line_count; lnum++) {
(void)ml_get_buf(buf, lnum, false);
}
}

if (close(mfp->mf_fd) < 0) { 
EMSG(_(e_swapclose));
}
mfp->mf_fd = -1;

if (mfp->mf_fname != NULL) {
os_remove((char *)mfp->mf_fname); 
mf_free_fnames(mfp);
}
}



void mf_new_page_size(memfile_T *mfp, unsigned new_size)
{
mfp->mf_page_size = new_size;
}





bhdr_T *mf_new(memfile_T *mfp, bool negative, unsigned page_count)
{
bhdr_T *hp = NULL;





bhdr_T *freep = mfp->mf_free_first; 
if (!negative && freep != NULL && freep->bh_page_count >= page_count) {
if (freep->bh_page_count > page_count) {


hp = mf_alloc_bhdr(mfp, page_count);
hp->bh_bnum = freep->bh_bnum;
freep->bh_bnum += page_count;
freep->bh_page_count -= page_count;
} else { 


void *p = xmalloc(mfp->mf_page_size * page_count);
hp = mf_rem_free(mfp);
hp->bh_data = p;
}
} else { 
hp = mf_alloc_bhdr(mfp, page_count);
if (negative) {
hp->bh_bnum = mfp->mf_blocknr_min--;
mfp->mf_neg_count++;
} else {
hp->bh_bnum = mfp->mf_blocknr_max;
mfp->mf_blocknr_max += page_count;
}
}
hp->bh_flags = BH_LOCKED | BH_DIRTY; 
mfp->mf_dirty = true;
hp->bh_page_count = page_count;
mf_ins_used(mfp, hp);
mf_ins_hash(mfp, hp);



(void)memset(hp->bh_data, 0, mfp->mf_page_size * page_count);

return hp;
}






bhdr_T *mf_get(memfile_T *mfp, blocknr_T nr, unsigned page_count)
{

if (nr >= mfp->mf_blocknr_max || nr <= mfp->mf_blocknr_min)
return NULL;


bhdr_T *hp = mf_find_hash(mfp, nr);
if (hp == NULL) { 
if (nr < 0 || nr >= mfp->mf_infile_count) 
return NULL;



hp = mf_alloc_bhdr(mfp, page_count);

hp->bh_bnum = nr;
hp->bh_flags = 0;
hp->bh_page_count = page_count;
if (mf_read(mfp, hp) == FAIL) { 
mf_free_bhdr(hp);
return NULL;
}
} else {
mf_rem_used(mfp, hp); 
mf_rem_hash(mfp, hp);
}

hp->bh_flags |= BH_LOCKED;
mf_ins_used(mfp, hp); 
mf_ins_hash(mfp, hp); 

return hp;
}





void mf_put(memfile_T *mfp, bhdr_T *hp, bool dirty, bool infile)
{
unsigned flags = hp->bh_flags;

if ((flags & BH_LOCKED) == 0) {
IEMSG(_("E293: block was not locked"));
}
flags &= ~BH_LOCKED;
if (dirty) {
flags |= BH_DIRTY;
mfp->mf_dirty = true;
}
hp->bh_flags = flags;
if (infile)
mf_trans_add(mfp, hp); 
}


void mf_free(memfile_T *mfp, bhdr_T *hp)
{
xfree(hp->bh_data); 
mf_rem_hash(mfp, hp); 
mf_rem_used(mfp, hp); 
if (hp->bh_bnum < 0) {
xfree(hp); 
mfp->mf_neg_count--;
} else {
mf_ins_free(mfp, hp); 
}
}















int mf_sync(memfile_T *mfp, int flags)
{
int got_int_save = got_int;

if (mfp->mf_fd < 0) { 
mfp->mf_dirty = false;
return FAIL;
}


got_int = false;





int status = OK;
bhdr_T *hp;
for (hp = mfp->mf_used_last; hp != NULL; hp = hp->bh_prev)
if (((flags & MFS_ALL) || hp->bh_bnum >= 0)
&& (hp->bh_flags & BH_DIRTY)
&& (status == OK || (hp->bh_bnum >= 0
&& hp->bh_bnum < mfp->mf_infile_count))) {
if ((flags & MFS_ZERO) && hp->bh_bnum != 0)
continue;
if (mf_write(mfp, hp) == FAIL) {
if (status == FAIL) 
break;
status = FAIL;
}
if (flags & MFS_STOP) { 
if (os_char_avail())
break;
} else {
os_breakcheck();
}
if (got_int)
break;
}



if (hp == NULL || status == FAIL)
mfp->mf_dirty = false;

if (flags & MFS_FLUSH) {
if (os_fsync(mfp->mf_fd)) {
status = FAIL;
}
}

got_int |= got_int_save;

return status;
}



void mf_set_dirty(memfile_T *mfp)
{
for (bhdr_T *hp = mfp->mf_used_last; hp != NULL; hp = hp->bh_prev) {
if (hp->bh_bnum > 0) {
hp->bh_flags |= BH_DIRTY;
}
}
mfp->mf_dirty = true;
}


static void mf_ins_hash(memfile_T *mfp, bhdr_T *hp)
{
mf_hash_add_item(&mfp->mf_hash, (mf_hashitem_T *)hp);
}


static void mf_rem_hash(memfile_T *mfp, bhdr_T *hp)
{
mf_hash_rem_item(&mfp->mf_hash, (mf_hashitem_T *)hp);
}


static bhdr_T *mf_find_hash(memfile_T *mfp, blocknr_T nr)
{
return (bhdr_T *)mf_hash_find(&mfp->mf_hash, nr);
}


static void mf_ins_used(memfile_T *mfp, bhdr_T *hp)
{
hp->bh_next = mfp->mf_used_first;
mfp->mf_used_first = hp;
hp->bh_prev = NULL;
if (hp->bh_next == NULL) { 
mfp->mf_used_last = hp;
} else {
hp->bh_next->bh_prev = hp;
}
}


static void mf_rem_used(memfile_T *mfp, bhdr_T *hp)
{
if (hp->bh_next == NULL) 
mfp->mf_used_last = hp->bh_prev;
else
hp->bh_next->bh_prev = hp->bh_prev;

if (hp->bh_prev == NULL) 
mfp->mf_used_first = hp->bh_next;
else
hp->bh_prev->bh_next = hp->bh_next;
}






bool mf_release_all(void)
{
bool retval = false;
FOR_ALL_BUFFERS(buf) {
memfile_T *mfp = buf->b_ml.ml_mfp;
if (mfp != NULL) {

if (mfp->mf_fd < 0 && buf->b_may_swap) {
ml_open_file(buf);
}


if (mfp->mf_fd >= 0) {
for (bhdr_T *hp = mfp->mf_used_last; hp != NULL; ) {
if (!(hp->bh_flags & BH_LOCKED)
&& (!(hp->bh_flags & BH_DIRTY)
|| mf_write(mfp, hp) != FAIL)) {
mf_rem_used(mfp, hp);
mf_rem_hash(mfp, hp);
mf_free_bhdr(hp);
hp = mfp->mf_used_last; 
retval = true;
} else {
hp = hp->bh_prev;
}
}
}
}
}
return retval;
}


static bhdr_T *mf_alloc_bhdr(memfile_T *mfp, unsigned page_count)
{
bhdr_T *hp = xmalloc(sizeof(bhdr_T));
hp->bh_data = xmalloc(mfp->mf_page_size * page_count);
hp->bh_page_count = page_count;
return hp;
}


static void mf_free_bhdr(bhdr_T *hp)
{
xfree(hp->bh_data);
xfree(hp);
}


static void mf_ins_free(memfile_T *mfp, bhdr_T *hp)
{
hp->bh_next = mfp->mf_free_first;
mfp->mf_free_first = hp;
}




static bhdr_T *mf_rem_free(memfile_T *mfp)
{
bhdr_T *hp = mfp->mf_free_first;
mfp->mf_free_first = hp->bh_next;
return hp;
}







static int mf_read(memfile_T *mfp, bhdr_T *hp)
{
if (mfp->mf_fd < 0) 
return FAIL;

unsigned page_size = mfp->mf_page_size;

off_T offset = (off_T)(page_size * hp->bh_bnum);
if (vim_lseek(mfp->mf_fd, offset, SEEK_SET) != offset) {
PERROR(_("E294: Seek error in swap file read"));
return FAIL;
}

assert(hp->bh_page_count <= UINT_MAX / page_size);
unsigned size = page_size * hp->bh_page_count;
if ((unsigned)read_eintr(mfp->mf_fd, hp->bh_data, size) != size) {
PERROR(_("E295: Read error in swap file"));
return FAIL;
}

return OK;
}









static int mf_write(memfile_T *mfp, bhdr_T *hp)
{
off_T offset; 
blocknr_T nr; 
bhdr_T *hp2;
unsigned page_size; 
unsigned page_count; 
unsigned size; 

if (mfp->mf_fd < 0) 
return FAIL;

if (hp->bh_bnum < 0) 
if (mf_trans_add(mfp, hp) == FAIL)
return FAIL;

page_size = mfp->mf_page_size;





for (;;) {
nr = hp->bh_bnum;
if (nr > mfp->mf_infile_count) { 
nr = mfp->mf_infile_count;
hp2 = mf_find_hash(mfp, nr); 
} else {
hp2 = hp;
}


offset = (off_T)(page_size * nr);
if (vim_lseek(mfp->mf_fd, offset, SEEK_SET) != offset) {
PERROR(_("E296: Seek error in swap file write"));
return FAIL;
}
if (hp2 == NULL) 
page_count = 1;
else
page_count = hp2->bh_page_count;
size = page_size * page_count;
void *data = (hp2 == NULL) ? hp->bh_data : hp2->bh_data;
if ((unsigned)write_eintr(mfp->mf_fd, data, size) != size) {




if (!did_swapwrite_msg)
EMSG(_("E297: Write error in swap file"));
did_swapwrite_msg = true;
return FAIL;
}
did_swapwrite_msg = false;
if (hp2 != NULL) 
hp2->bh_flags &= ~BH_DIRTY;
if (nr + (blocknr_T)page_count > mfp->mf_infile_count) 
mfp->mf_infile_count = nr + page_count;
if (nr == hp->bh_bnum) 
break;
}
return OK;
}





static int mf_trans_add(memfile_T *mfp, bhdr_T *hp)
{
if (hp->bh_bnum >= 0) 
return OK;

mf_blocknr_trans_item_T *np = xmalloc(sizeof(mf_blocknr_trans_item_T));




blocknr_T new_bnum;
bhdr_T *freep = mfp->mf_free_first;
unsigned page_count = hp->bh_page_count;
if (freep != NULL && freep->bh_page_count >= page_count) {
new_bnum = freep->bh_bnum;


if (freep->bh_page_count > page_count) {
freep->bh_bnum += page_count;
freep->bh_page_count -= page_count;
} else {
freep = mf_rem_free(mfp);
xfree(freep);
}
} else {
new_bnum = mfp->mf_blocknr_max;
mfp->mf_blocknr_max += page_count;
}

np->nt_old_bnum = hp->bh_bnum; 
np->nt_new_bnum = new_bnum;

mf_rem_hash(mfp, hp); 
hp->bh_bnum = new_bnum;
mf_ins_hash(mfp, hp); 


mf_hash_add_item(&mfp->mf_trans, (mf_hashitem_T *)np);

return OK;
}





blocknr_T mf_trans_del(memfile_T *mfp, blocknr_T old_nr)
{
mf_blocknr_trans_item_T *np =
(mf_blocknr_trans_item_T *)mf_hash_find(&mfp->mf_trans, old_nr);

if (np == NULL) 
return old_nr;

mfp->mf_neg_count--;
blocknr_T new_bnum = np->nt_new_bnum;


mf_hash_rem_item(&mfp->mf_trans, (mf_hashitem_T *)np);

xfree(np);

return new_bnum;
}


void mf_free_fnames(memfile_T *mfp)
{
XFREE_CLEAR(mfp->mf_fname);
XFREE_CLEAR(mfp->mf_ffname);
}






void mf_set_fnames(memfile_T *mfp, char_u *fname)
{
mfp->mf_fname = fname;
mfp->mf_ffname = (char_u *)FullName_save((char *)mfp->mf_fname, false);
}




void mf_fullname(memfile_T *mfp)
{
if (mfp != NULL && mfp->mf_fname != NULL && mfp->mf_ffname != NULL) {
xfree(mfp->mf_fname);
mfp->mf_fname = mfp->mf_ffname;
mfp->mf_ffname = NULL;
}
}


bool mf_need_trans(memfile_T *mfp)
{
return mfp->mf_fname != NULL && mfp->mf_neg_count > 0;
}







static bool mf_do_open(memfile_T *mfp, char_u *fname, int flags)
{

mf_set_fnames(mfp, fname);
assert(mfp->mf_fname != NULL);



FileInfo file_info;
if ((flags & O_CREAT)
&& os_fileinfo_link((char *)mfp->mf_fname, &file_info)) {
mfp->mf_fd = -1;
EMSG(_("E300: Swap file already exists (symlink attack?)"));
} else {

mfp->mf_fd = mch_open_rw((char *)mfp->mf_fname, flags | O_NOFOLLOW);
}


if (mfp->mf_fd < 0) {
mf_free_fnames(mfp);
return false;
}

(void)os_set_cloexec(mfp->mf_fd);

return true;
}








#define MHT_LOG_LOAD_FACTOR 6
#define MHT_GROWTH_FACTOR 2 


static void mf_hash_init(mf_hashtab_T *mht)
{
memset(mht, 0, sizeof(mf_hashtab_T));
mht->mht_buckets = mht->mht_small_buckets;
mht->mht_mask = MHT_INIT_SIZE - 1;
}



static void mf_hash_free(mf_hashtab_T *mht)
{
if (mht->mht_buckets != mht->mht_small_buckets) {
xfree(mht->mht_buckets);
}
}


static void mf_hash_free_all(mf_hashtab_T *mht)
{
for (size_t idx = 0; idx <= mht->mht_mask; idx++) {
mf_hashitem_T *next;
for (mf_hashitem_T *mhi = mht->mht_buckets[idx]; mhi != NULL; mhi = next) {
next = mhi->mhi_next;
xfree(mhi);
}
}

mf_hash_free(mht);
}




static mf_hashitem_T *mf_hash_find(mf_hashtab_T *mht, blocknr_T key)
{
mf_hashitem_T *mhi = mht->mht_buckets[(size_t)key & mht->mht_mask];
while (mhi != NULL && mhi->mhi_key != key)
mhi = mhi->mhi_next;
return mhi;
}


static void mf_hash_add_item(mf_hashtab_T *mht, mf_hashitem_T *mhi)
{
size_t idx = (size_t)mhi->mhi_key & mht->mht_mask;
mhi->mhi_next = mht->mht_buckets[idx];
mhi->mhi_prev = NULL;
if (mhi->mhi_next != NULL)
mhi->mhi_next->mhi_prev = mhi;
mht->mht_buckets[idx] = mhi;

mht->mht_count++;



if ((mht->mht_count >> MHT_LOG_LOAD_FACTOR) > mht->mht_mask) {
mf_hash_grow(mht);
}
}


static void mf_hash_rem_item(mf_hashtab_T *mht, mf_hashitem_T *mhi)
{
if (mhi->mhi_prev == NULL)
mht->mht_buckets[(size_t)mhi->mhi_key & mht->mht_mask] =
mhi->mhi_next;
else
mhi->mhi_prev->mhi_next = mhi->mhi_next;

if (mhi->mhi_next != NULL)
mhi->mhi_next->mhi_prev = mhi->mhi_prev;

mht->mht_count--;



}



static void mf_hash_grow(mf_hashtab_T *mht)
{
size_t size = (mht->mht_mask + 1) * MHT_GROWTH_FACTOR * sizeof(void *);
mf_hashitem_T **buckets = xcalloc(1, size);

int shift = 0;
while ((mht->mht_mask >> shift) != 0)
shift++;

for (size_t i = 0; i <= mht->mht_mask; i++) {









mf_hashitem_T *tails[MHT_GROWTH_FACTOR];
memset(tails, 0, sizeof(tails));

for (mf_hashitem_T *mhi = mht->mht_buckets[i];
mhi != NULL; mhi = mhi->mhi_next) {
size_t j = (mhi->mhi_key >> shift) & (MHT_GROWTH_FACTOR - 1);
if (tails[j] == NULL) {
buckets[i + (j << shift)] = mhi;
tails[j] = mhi;
mhi->mhi_prev = NULL;
} else {
tails[j]->mhi_next = mhi;
mhi->mhi_prev = tails[j];
tails[j] = mhi;
}
}

for (size_t j = 0; j < MHT_GROWTH_FACTOR; j++)
if (tails[j] != NULL)
tails[j]->mhi_next = NULL;
}

if (mht->mht_buckets != mht->mht_small_buckets)
xfree(mht->mht_buckets);

mht->mht_buckets = buckets;
mht->mht_mask = (mht->mht_mask + 1) * MHT_GROWTH_FACTOR - 1;
}
