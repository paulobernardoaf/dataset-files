#define CHECK(c, s) do { } while (0)
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include "nvim/ascii.h"
#include "nvim/vim.h"
#include "nvim/memline.h"
#include "nvim/buffer.h"
#include "nvim/change.h"
#include "nvim/cursor.h"
#include "nvim/eval.h"
#include "nvim/getchar.h"
#include "nvim/fileio.h"
#include "nvim/func_attr.h"
#include "nvim/main.h"
#include "nvim/mark.h"
#include "nvim/mbyte.h"
#include "nvim/memfile.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/option.h"
#include "nvim/os_unix.h"
#include "nvim/path.h"
#include "nvim/screen.h"
#include "nvim/sha256.h"
#include "nvim/spell.h"
#include "nvim/strings.h"
#include "nvim/ui.h"
#include "nvim/version.h"
#include "nvim/undo.h"
#include "nvim/window.h"
#include "nvim/os/os.h"
#include "nvim/os/process.h"
#include "nvim/os/input.h"
#if !defined(UNIX)
#include <time.h>
#endif
typedef struct block0 ZERO_BL; 
typedef struct pointer_block PTR_BL; 
typedef struct data_block DATA_BL; 
typedef struct pointer_entry PTR_EN; 
#define DATA_ID (('d' << 8) + 'a') 
#define PTR_ID (('p' << 8) + 't') 
#define BLOCK0_ID0 'b' 
#define BLOCK0_ID1 '0' 
struct pointer_entry {
blocknr_T pe_bnum; 
linenr_T pe_line_count; 
linenr_T pe_old_lnum; 
int pe_page_count; 
};
struct pointer_block {
uint16_t pb_id; 
uint16_t pb_count; 
uint16_t pb_count_max; 
PTR_EN pb_pointer[1]; 
};
struct data_block {
uint16_t db_id; 
unsigned db_free; 
unsigned db_txt_start; 
unsigned db_txt_end; 
linenr_T db_line_count; 
unsigned db_index[1]; 
};
#define DB_MARKED ((unsigned)1 << ((sizeof(unsigned) * 8) - 1))
#define DB_INDEX_MASK (~DB_MARKED)
#define INDEX_SIZE (sizeof(unsigned)) 
#define HEADER_SIZE (sizeof(DATA_BL) - INDEX_SIZE) 
#define B0_FNAME_SIZE_ORG 900 
#define B0_FNAME_SIZE_NOCRYPT 898 
#define B0_FNAME_SIZE_CRYPT 890 
#define B0_UNAME_SIZE 40
#define B0_HNAME_SIZE 40
#define B0_MAGIC_LONG 0x30313233L
#define B0_MAGIC_INT 0x20212223L
#define B0_MAGIC_SHORT 0x10111213L
#define B0_MAGIC_CHAR 0x55
struct block0 {
char_u b0_id[2]; 
char_u b0_version[10]; 
char_u b0_page_size[4]; 
char_u b0_mtime[4]; 
char_u b0_ino[4]; 
char_u b0_pid[4]; 
char_u b0_uname[B0_UNAME_SIZE]; 
char_u b0_hname[B0_HNAME_SIZE]; 
char_u b0_fname[B0_FNAME_SIZE_ORG]; 
long b0_magic_long; 
int b0_magic_int; 
short b0_magic_short; 
char_u b0_magic_char; 
};
#define B0_DIRTY 0x55
#define b0_dirty b0_fname[B0_FNAME_SIZE_ORG - 1]
#define b0_flags b0_fname[B0_FNAME_SIZE_ORG - 2]
#define B0_FF_MASK 3
#define B0_SAME_DIR 4
#define B0_HAS_FENC 8
#define STACK_INCR 5 
static linenr_T lowest_marked = 0;
#define ML_DELETE 0x11 
#define ML_INSERT 0x12 
#define ML_FIND 0x13 
#define ML_FLUSH 0x02 
#define ML_SIMPLE(x) (x & 0x10) 
typedef enum {
UB_FNAME = 0 
, UB_SAME_DIR 
} upd_block0_T;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "memline.c.generated.h"
#endif
int ml_open(buf_T *buf)
{
bhdr_T *hp = NULL;
ZERO_BL *b0p;
PTR_BL *pp;
DATA_BL *dp;
buf->b_ml.ml_stack_size = 0; 
buf->b_ml.ml_stack = NULL; 
buf->b_ml.ml_stack_top = 0; 
buf->b_ml.ml_locked = NULL; 
buf->b_ml.ml_line_lnum = 0; 
buf->b_ml.ml_chunksize = NULL;
if (cmdmod.noswapfile) {
buf->b_p_swf = false;
}
if (!buf->terminal && p_uc && buf->b_p_swf) {
buf->b_may_swap = true;
} else {
buf->b_may_swap = false;
}
memfile_T *mfp = mf_open(NULL, 0);
if (mfp == NULL) {
goto error;
}
buf->b_ml.ml_mfp = mfp;
buf->b_ml.ml_flags = ML_EMPTY;
buf->b_ml.ml_line_count = 1;
curwin->w_nrwidth_line_count = 0;
hp = mf_new(mfp, false, 1);
if (hp->bh_bnum != 0) {
IEMSG(_("E298: Didn't get block nr 0?"));
goto error;
}
b0p = hp->bh_data;
b0p->b0_id[0] = BLOCK0_ID0;
b0p->b0_id[1] = BLOCK0_ID1;
b0p->b0_magic_long = (long)B0_MAGIC_LONG;
b0p->b0_magic_int = (int)B0_MAGIC_INT;
b0p->b0_magic_short = (short)B0_MAGIC_SHORT;
b0p->b0_magic_char = B0_MAGIC_CHAR;
xstrlcpy(xstpcpy((char *) b0p->b0_version, "VIM "), Version, 6);
long_to_char((long)mfp->mf_page_size, b0p->b0_page_size);
if (!buf->b_spell) {
b0p->b0_dirty = buf->b_changed ? B0_DIRTY : 0;
b0p->b0_flags = get_fileformat(buf) + 1;
set_b0_fname(b0p, buf);
(void)os_get_user_name((char *)b0p->b0_uname, B0_UNAME_SIZE);
b0p->b0_uname[B0_UNAME_SIZE - 1] = NUL;
os_get_hostname((char *)b0p->b0_hname, B0_HNAME_SIZE);
b0p->b0_hname[B0_HNAME_SIZE - 1] = NUL;
long_to_char(os_get_pid(), b0p->b0_pid);
}
mf_put(mfp, hp, true, false);
if (!buf->b_help && !B_SPELL(buf))
(void)mf_sync(mfp, 0);
if ((hp = ml_new_ptr(mfp)) == NULL)
goto error;
if (hp->bh_bnum != 1) {
IEMSG(_("E298: Didn't get block nr 1?"));
goto error;
}
pp = hp->bh_data;
pp->pb_count = 1;
pp->pb_pointer[0].pe_bnum = 2;
pp->pb_pointer[0].pe_page_count = 1;
pp->pb_pointer[0].pe_old_lnum = 1;
pp->pb_pointer[0].pe_line_count = 1; 
mf_put(mfp, hp, true, false);
hp = ml_new_data(mfp, false, 1);
if (hp->bh_bnum != 2) {
IEMSG(_("E298: Didn't get block nr 2?"));
goto error;
}
dp = hp->bh_data;
dp->db_index[0] = --dp->db_txt_start; 
dp->db_free -= 1 + INDEX_SIZE;
dp->db_line_count = 1;
*((char_u *)dp + dp->db_txt_start) = NUL; 
return OK;
error:
if (mfp != NULL) {
if (hp) {
mf_put(mfp, hp, false, false);
}
mf_close(mfp, true); 
}
buf->b_ml.ml_mfp = NULL;
return FAIL;
}
void ml_setname(buf_T *buf)
{
int success = FALSE;
memfile_T *mfp;
char_u *fname;
char_u *dirp;
mfp = buf->b_ml.ml_mfp;
if (mfp->mf_fd < 0) { 
if (p_uc != 0 && !cmdmod.noswapfile) {
ml_open_file(buf); 
}
return;
}
dirp = p_dir;
bool found_existing_dir = false;
for (;; ) {
if (*dirp == NUL) 
break;
fname = (char_u *)findswapname(buf, (char **)&dirp, (char *)mfp->mf_fname,
&found_existing_dir);
if (dirp == NULL) 
break;
if (fname == NULL) 
continue;
if (fnamecmp(fname, mfp->mf_fname) == 0) {
xfree(fname);
success = TRUE;
break;
}
if (mfp->mf_fd >= 0) {
close(mfp->mf_fd);
mfp->mf_fd = -1;
}
if (vim_rename(mfp->mf_fname, fname) == 0) {
success = TRUE;
mf_free_fnames(mfp);
mf_set_fnames(mfp, fname);
ml_upd_block0(buf, UB_SAME_DIR);
break;
}
xfree(fname); 
}
if (mfp->mf_fd == -1) { 
mfp->mf_fd = os_open((char *)mfp->mf_fname, O_RDWR, 0);
if (mfp->mf_fd < 0) {
EMSG(_("E301: Oops, lost the swap file!!!"));
return;
}
(void)os_set_cloexec(mfp->mf_fd);
}
if (!success)
EMSG(_("E302: Could not rename swap file"));
}
void ml_open_files(void)
{
FOR_ALL_BUFFERS(buf) {
if (!buf->b_p_ro || buf->b_changed) {
ml_open_file(buf);
}
}
}
void ml_open_file(buf_T *buf)
{
memfile_T *mfp;
char_u *fname;
char_u *dirp;
mfp = buf->b_ml.ml_mfp;
if (mfp == NULL || mfp->mf_fd >= 0 || !buf->b_p_swf || cmdmod.noswapfile
|| buf->terminal) {
return; 
}
if (buf->b_spell) {
fname = vim_tempname();
if (fname != NULL)
(void)mf_open_file(mfp, fname); 
buf->b_may_swap = false;
return;
}
dirp = p_dir;
bool found_existing_dir = false;
for (;; ) {
if (*dirp == NUL)
break;
fname = (char_u *)findswapname(buf, (char **)&dirp, NULL,
&found_existing_dir);
if (dirp == NULL)
break; 
if (fname == NULL)
continue;
if (mf_open_file(mfp, fname) == OK) { 
ml_upd_block0(buf, UB_SAME_DIR);
if (mf_sync(mfp, MFS_ZERO) == OK) {
mf_set_dirty(mfp);
break;
}
mf_close_file(buf, false);
}
}
if (*p_dir != NUL && mfp->mf_fname == NULL) {
need_wait_return = true; 
no_wait_return++;
(void)EMSG2(_(
"E303: Unable to open swap file for \"%s\", recovery impossible"),
buf_spname(buf) != NULL ? buf_spname(buf) : buf->b_fname);
--no_wait_return;
}
buf->b_may_swap = false;
}
void check_need_swap(bool newfile)
{
int old_msg_silent = msg_silent; 
msg_silent = 0; 
if (curbuf->b_may_swap && (!curbuf->b_p_ro || !newfile)) {
ml_open_file(curbuf);
}
msg_silent = old_msg_silent;
}
void ml_close(buf_T *buf, int del_file)
{
if (buf->b_ml.ml_mfp == NULL) 
return;
mf_close(buf->b_ml.ml_mfp, del_file); 
if (buf->b_ml.ml_line_lnum != 0 && (buf->b_ml.ml_flags & ML_LINE_DIRTY))
xfree(buf->b_ml.ml_line_ptr);
xfree(buf->b_ml.ml_stack);
XFREE_CLEAR(buf->b_ml.ml_chunksize);
buf->b_ml.ml_mfp = NULL;
buf->b_flags &= ~BF_RECOVERED;
}
void ml_close_all(int del_file)
{
FOR_ALL_BUFFERS(buf) {
ml_close(buf, del_file && ((buf->b_flags & BF_PRESERVED) == 0));
}
spell_delete_wordlist(); 
vim_deltempdir(); 
}
void ml_close_notmod(void)
{
FOR_ALL_BUFFERS(buf) {
if (!bufIsChanged(buf)) {
ml_close(buf, TRUE); 
}
}
}
void ml_timestamp(buf_T *buf)
{
ml_upd_block0(buf, UB_FNAME);
}
static bool ml_check_b0_id(ZERO_BL *b0p)
FUNC_ATTR_NONNULL_ALL
{
return b0p->b0_id[0] == BLOCK0_ID0 && b0p->b0_id[1] == BLOCK0_ID1;
}
static bool ml_check_b0_strings(ZERO_BL *b0p)
FUNC_ATTR_NONNULL_ALL
{
return (memchr(b0p->b0_version, NUL, 10)
&& memchr(b0p->b0_uname, NUL, B0_UNAME_SIZE)
&& memchr(b0p->b0_hname, NUL, B0_HNAME_SIZE)
&& memchr(b0p->b0_fname, NUL, B0_FNAME_SIZE_CRYPT)); 
}
static void ml_upd_block0(buf_T *buf, upd_block0_T what)
{
memfile_T *mfp;
bhdr_T *hp;
ZERO_BL *b0p;
mfp = buf->b_ml.ml_mfp;
if (mfp == NULL || (hp = mf_get(mfp, 0, 1)) == NULL)
return;
b0p = hp->bh_data;
if (ml_check_b0_id(b0p) == FAIL) {
IEMSG(_("E304: ml_upd_block0(): Didn't get block 0??"));
} else {
if (what == UB_FNAME) {
set_b0_fname(b0p, buf);
} else { 
set_b0_dir_flag(b0p, buf);
}
}
mf_put(mfp, hp, true, false);
}
static void set_b0_fname(ZERO_BL *b0p, buf_T *buf)
{
if (buf->b_ffname == NULL)
b0p->b0_fname[0] = NUL;
else {
char uname[B0_UNAME_SIZE];
home_replace(NULL, buf->b_ffname, b0p->b0_fname,
B0_FNAME_SIZE_CRYPT, TRUE);
if (b0p->b0_fname[0] == '~') {
int retval = os_get_user_name(uname, B0_UNAME_SIZE);
size_t ulen = STRLEN(uname);
size_t flen = STRLEN(b0p->b0_fname);
if (retval == FAIL || ulen + flen > B0_FNAME_SIZE_CRYPT - 1) {
STRLCPY(b0p->b0_fname, buf->b_ffname, B0_FNAME_SIZE_CRYPT);
} else {
memmove(b0p->b0_fname + ulen + 1, b0p->b0_fname + 1, flen);
memmove(b0p->b0_fname + 1, uname, ulen);
}
}
FileInfo file_info;
if (os_fileinfo((char *)buf->b_ffname, &file_info)) {
long_to_char(file_info.stat.st_mtim.tv_sec, b0p->b0_mtime);
long_to_char((long)os_fileinfo_inode(&file_info), b0p->b0_ino);
buf_store_file_info(buf, &file_info);
buf->b_mtime_read = buf->b_mtime;
} else {
long_to_char(0L, b0p->b0_mtime);
long_to_char(0L, b0p->b0_ino);
buf->b_mtime = 0;
buf->b_mtime_read = 0;
buf->b_orig_size = 0;
buf->b_orig_mode = 0;
}
}
add_b0_fenc(b0p, curbuf);
}
static void set_b0_dir_flag(ZERO_BL *b0p, buf_T *buf)
{
if (same_directory(buf->b_ml.ml_mfp->mf_fname, buf->b_ffname))
b0p->b0_flags |= B0_SAME_DIR;
else
b0p->b0_flags &= ~B0_SAME_DIR;
}
static void add_b0_fenc(ZERO_BL *b0p, buf_T *buf)
{
int n;
int size = B0_FNAME_SIZE_NOCRYPT;
n = (int)STRLEN(buf->b_p_fenc);
if ((int)STRLEN(b0p->b0_fname) + n + 1 > size)
b0p->b0_flags &= ~B0_HAS_FENC;
else {
memmove((char *)b0p->b0_fname + size - n,
(char *)buf->b_p_fenc, (size_t)n);
*(b0p->b0_fname + size - n - 1) = NUL;
b0p->b0_flags |= B0_HAS_FENC;
}
}
void ml_recover(bool checkext)
{
buf_T *buf = NULL;
memfile_T *mfp = NULL;
char_u *fname;
char_u *fname_used = NULL;
bhdr_T *hp = NULL;
ZERO_BL *b0p;
int b0_ff;
char_u *b0_fenc = NULL;
PTR_BL *pp;
DATA_BL *dp;
infoptr_T *ip;
blocknr_T bnum;
int page_count;
int len;
int directly;
linenr_T lnum;
char_u *p;
int i;
long error;
int cannot_open;
linenr_T line_count;
bool has_error;
int idx;
int top;
int txt_start;
off_T size;
int called_from_main;
int serious_error = TRUE;
long mtime;
int attr;
int orig_file_status = NOTDONE;
recoverymode = TRUE;
called_from_main = (curbuf->b_ml.ml_mfp == NULL);
attr = HL_ATTR(HLF_E);
fname = curbuf->b_fname;
if (fname == NULL) 
fname = (char_u *)"";
len = (int)STRLEN(fname);
if (checkext && len >= 4
&& STRNICMP(fname + len - 4, ".s", 2) == 0
&& vim_strchr((char_u *)"abcdefghijklmnopqrstuvw",
TOLOWER_ASC(fname[len - 2])) != NULL
&& ASCII_ISALPHA(fname[len - 1])) {
directly = TRUE;
fname_used = vim_strsave(fname); 
} else {
directly = FALSE;
len = recover_names(fname, FALSE, 0, NULL);
if (len == 0) { 
EMSG2(_("E305: No swap file found for %s"), fname);
goto theend;
}
if (len == 1) 
i = 1;
else { 
(void)recover_names(fname, TRUE, 0, NULL);
msg_putchar('\n');
MSG_PUTS(_("Enter number of swap file to use (0 to quit): "));
i = get_number(FALSE, NULL);
if (i < 1 || i > len)
goto theend;
}
(void)recover_names(fname, FALSE, i, &fname_used);
}
if (fname_used == NULL)
goto theend; 
if (called_from_main && ml_open(curbuf) == FAIL)
getout(1);
buf = xmalloc(sizeof(buf_T));
buf->b_ml.ml_stack_size = 0; 
buf->b_ml.ml_stack = NULL; 
buf->b_ml.ml_stack_top = 0; 
buf->b_ml.ml_line_lnum = 0; 
buf->b_ml.ml_locked = NULL; 
buf->b_ml.ml_flags = 0;
p = vim_strsave(fname_used); 
mfp = mf_open(fname_used, O_RDONLY);
fname_used = p;
if (mfp == NULL || mfp->mf_fd < 0) {
EMSG2(_("E306: Cannot open %s"), fname_used);
goto theend;
}
buf->b_ml.ml_mfp = mfp;
mfp->mf_page_size = MIN_SWAP_PAGE_SIZE;
if ((hp = mf_get(mfp, 0, 1)) == NULL) {
msg_start();
MSG_PUTS_ATTR(_("Unable to read block 0 from "), attr | MSG_HIST);
msg_outtrans_attr(mfp->mf_fname, attr | MSG_HIST);
MSG_PUTS_ATTR(_(
"\nMaybe no changes were made or Vim did not update the swap file."),
attr | MSG_HIST);
msg_end();
goto theend;
}
b0p = hp->bh_data;
if (STRNCMP(b0p->b0_version, "VIM 3.0", 7) == 0) {
msg_start();
msg_outtrans_attr(mfp->mf_fname, MSG_HIST);
MSG_PUTS_ATTR(_(" cannot be used with this version of Vim.\n"),
MSG_HIST);
MSG_PUTS_ATTR(_("Use Vim version 3.0.\n"), MSG_HIST);
msg_end();
goto theend;
}
if (ml_check_b0_id(b0p) == FAIL) {
EMSG2(_("E307: %s does not look like a Vim swap file"), mfp->mf_fname);
goto theend;
}
if (b0_magic_wrong(b0p)) {
msg_start();
msg_outtrans_attr(mfp->mf_fname, attr | MSG_HIST);
MSG_PUTS_ATTR(_(" cannot be used on this computer.\n"),
attr | MSG_HIST);
MSG_PUTS_ATTR(_("The file was created on "), attr | MSG_HIST);
b0p->b0_fname[0] = NUL;
MSG_PUTS_ATTR(b0p->b0_hname, attr | MSG_HIST);
MSG_PUTS_ATTR(_(",\nor the file has been damaged."), attr | MSG_HIST);
msg_end();
goto theend;
}
if (mfp->mf_page_size != (unsigned)char_to_long(b0p->b0_page_size)) {
unsigned previous_page_size = mfp->mf_page_size;
mf_new_page_size(mfp, (unsigned)char_to_long(b0p->b0_page_size));
if (mfp->mf_page_size < previous_page_size) {
msg_start();
msg_outtrans_attr(mfp->mf_fname, attr | MSG_HIST);
MSG_PUTS_ATTR(_(
" has been damaged (page size is smaller than minimum value).\n"),
attr | MSG_HIST);
msg_end();
goto theend;
}
if ((size = vim_lseek(mfp->mf_fd, (off_T)0L, SEEK_END)) <= 0) {
mfp->mf_blocknr_max = 0; 
} else {
mfp->mf_blocknr_max = size / mfp->mf_page_size;
}
mfp->mf_infile_count = mfp->mf_blocknr_max;
p = xmalloc(mfp->mf_page_size);
memmove(p, hp->bh_data, previous_page_size);
xfree(hp->bh_data);
hp->bh_data = p;
b0p = hp->bh_data;
}
if (directly) {
expand_env(b0p->b0_fname, NameBuff, MAXPATHL);
if (setfname(curbuf, NameBuff, NULL, true) == FAIL) {
goto theend;
}
}
home_replace(NULL, mfp->mf_fname, NameBuff, MAXPATHL, TRUE);
smsg(_("Using swap file \"%s\""), NameBuff);
if (buf_spname(curbuf) != NULL)
STRLCPY(NameBuff, buf_spname(curbuf), MAXPATHL);
else
home_replace(NULL, curbuf->b_ffname, NameBuff, MAXPATHL, TRUE);
smsg(_("Original file \"%s\""), NameBuff);
msg_putchar('\n');
FileInfo org_file_info;
FileInfo swp_file_info;
mtime = char_to_long(b0p->b0_mtime);
if (curbuf->b_ffname != NULL
&& os_fileinfo((char *)curbuf->b_ffname, &org_file_info)
&& ((os_fileinfo((char *)mfp->mf_fname, &swp_file_info)
&& org_file_info.stat.st_mtim.tv_sec
> swp_file_info.stat.st_mtim.tv_sec)
|| org_file_info.stat.st_mtim.tv_sec != mtime)) {
EMSG(_("E308: Warning: Original file may have been changed"));
}
ui_flush();
b0_ff = (b0p->b0_flags & B0_FF_MASK);
if (b0p->b0_flags & B0_HAS_FENC) {
int fnsize = B0_FNAME_SIZE_NOCRYPT;
for (p = b0p->b0_fname + fnsize; p > b0p->b0_fname && p[-1] != NUL; --p)
;
b0_fenc = vim_strnsave(p, (int)(b0p->b0_fname + fnsize - p));
}
mf_put(mfp, hp, false, false); 
hp = NULL;
while (!(curbuf->b_ml.ml_flags & ML_EMPTY)) {
ml_delete((linenr_T)1, false);
}
if (curbuf->b_ffname != NULL)
orig_file_status = readfile(curbuf->b_ffname, NULL, (linenr_T)0,
(linenr_T)0, (linenr_T)MAXLNUM, NULL, READ_NEW);
if (b0_ff != 0)
set_fileformat(b0_ff - 1, OPT_LOCAL);
if (b0_fenc != NULL) {
set_option_value("fenc", 0L, (char *)b0_fenc, OPT_LOCAL);
xfree(b0_fenc);
}
unchanged(curbuf, true, true);
bnum = 1; 
page_count = 1; 
lnum = 0; 
line_count = 0;
idx = 0; 
error = 0;
buf->b_ml.ml_stack_top = 0;
buf->b_ml.ml_stack = NULL;
buf->b_ml.ml_stack_size = 0; 
if (curbuf->b_ffname == NULL)
cannot_open = TRUE;
else
cannot_open = FALSE;
serious_error = FALSE;
for (; !got_int; line_breakcheck()) {
if (hp != NULL)
mf_put(mfp, hp, false, false); 
if ((hp = mf_get(mfp, bnum, page_count)) == NULL) {
if (bnum == 1) {
EMSG2(_("E309: Unable to read block 1 from %s"), mfp->mf_fname);
goto theend;
}
++error;
ml_append(lnum++, (char_u *)_("???MANY LINES MISSING"),
(colnr_T)0, true);
} else { 
pp = hp->bh_data;
if (pp->pb_id == PTR_ID) { 
if (idx == 0 && line_count != 0) {
for (i = 0; i < (int)pp->pb_count; ++i)
line_count -= pp->pb_pointer[i].pe_line_count;
if (line_count != 0) {
++error;
ml_append(lnum++, (char_u *)_("???LINE COUNT WRONG"),
(colnr_T)0, true);
}
}
if (pp->pb_count == 0) {
ml_append(lnum++, (char_u *)_("???EMPTY BLOCK"),
(colnr_T)0, true);
error++;
} else if (idx < (int)pp->pb_count) { 
if (pp->pb_pointer[idx].pe_bnum < 0) {
if (!cannot_open) {
line_count = pp->pb_pointer[idx].pe_line_count;
if (readfile(curbuf->b_ffname, NULL, lnum,
pp->pb_pointer[idx].pe_old_lnum - 1, line_count,
NULL, 0) != OK) {
cannot_open = true;
} else {
lnum += line_count;
}
}
if (cannot_open) {
++error;
ml_append(lnum++, (char_u *)_("???LINES MISSING"),
(colnr_T)0, true);
}
++idx; 
continue;
}
top = ml_add_stack(buf); 
ip = &(buf->b_ml.ml_stack[top]);
ip->ip_bnum = bnum;
ip->ip_index = idx;
bnum = pp->pb_pointer[idx].pe_bnum;
line_count = pp->pb_pointer[idx].pe_line_count;
page_count = pp->pb_pointer[idx].pe_page_count;
idx = 0;
continue;
}
} else { 
dp = hp->bh_data;
if (dp->db_id != DATA_ID) { 
if (bnum == 1) {
EMSG2(_("E310: Block 1 ID wrong (%s not a .swp file?)"),
mfp->mf_fname);
goto theend;
}
++error;
ml_append(lnum++, (char_u *)_("???BLOCK MISSING"),
(colnr_T)0, true);
} else {
has_error = false;
if (page_count * mfp->mf_page_size != dp->db_txt_end) {
ml_append(
lnum++,
(char_u *)_("??? from here until ???END lines"
" may be messed up"),
(colnr_T)0, true);
error++;
has_error = true;
dp->db_txt_end = page_count * mfp->mf_page_size;
}
*((char_u *)dp + dp->db_txt_end - 1) = NUL;
if (line_count != dp->db_line_count) {
ml_append(
lnum++,
(char_u *)_("??? from here until ???END lines"
" may have been inserted/deleted"),
(colnr_T)0, true);
error++;
has_error = true;
}
for (i = 0; i < dp->db_line_count; ++i) {
txt_start = (dp->db_index[i] & DB_INDEX_MASK);
if (txt_start <= (int)HEADER_SIZE
|| txt_start >= (int)dp->db_txt_end) {
p = (char_u *)"???";
++error;
} else
p = (char_u *)dp + txt_start;
ml_append(lnum++, p, (colnr_T)0, true);
}
if (has_error) {
ml_append(lnum++, (char_u *)_("???END"), (colnr_T)0, true);
}
}
}
}
if (buf->b_ml.ml_stack_top == 0) 
break;
ip = &(buf->b_ml.ml_stack[--(buf->b_ml.ml_stack_top)]);
bnum = ip->ip_bnum;
idx = ip->ip_index + 1; 
page_count = 1;
}
if (orig_file_status != OK || curbuf->b_ml.ml_line_count != lnum * 2 + 1) {
if (!(curbuf->b_ml.ml_line_count == 2 && *ml_get(1) == NUL)) {
changed_internal();
buf_inc_changedtick(curbuf);
}
} else {
for (idx = 1; idx <= lnum; ++idx) {
p = vim_strsave(ml_get(idx));
i = STRCMP(p, ml_get(idx + lnum));
xfree(p);
if (i != 0) {
changed_internal();
buf_inc_changedtick(curbuf);
break;
}
}
}
while (curbuf->b_ml.ml_line_count > lnum
&& !(curbuf->b_ml.ml_flags & ML_EMPTY))
ml_delete(curbuf->b_ml.ml_line_count, false);
curbuf->b_flags |= BF_RECOVERED;
recoverymode = FALSE;
if (got_int)
EMSG(_("E311: Recovery Interrupted"));
else if (error) {
++no_wait_return;
MSG(">>>>>>>>>>>>>");
EMSG(_(
"E312: Errors detected while recovering; look for lines starting with ???"));
--no_wait_return;
MSG(_("See \":help E312\" for more information."));
MSG(">>>>>>>>>>>>>");
} else {
if (curbuf->b_changed) {
MSG(_("Recovery completed. You should check if everything is OK."));
MSG_PUTS(_(
"\n(You might want to write out this file under another name\n"));
MSG_PUTS(_("and run diff with the original file to check for changes)"));
} else
MSG(_("Recovery completed. Buffer contents equals file contents."));
MSG_PUTS(_("\nYou may want to delete the .swp file now.\n\n"));
cmdline_row = msg_row;
}
redraw_curbuf_later(NOT_VALID);
theend:
xfree(fname_used);
recoverymode = FALSE;
if (mfp != NULL) {
if (hp != NULL)
mf_put(mfp, hp, false, false);
mf_close(mfp, false); 
}
if (buf != NULL) { 
xfree(buf->b_ml.ml_stack);
xfree(buf);
}
if (serious_error && called_from_main)
ml_close(curbuf, TRUE);
else {
apply_autocmds(EVENT_BUFREADPOST, NULL, curbuf->b_fname, FALSE, curbuf);
apply_autocmds(EVENT_BUFWINENTER, NULL, curbuf->b_fname, FALSE, curbuf);
}
return;
}
int 
recover_names (
char_u *fname, 
int list, 
int nr, 
char_u **fname_out 
)
{
int num_names;
char_u *(names[6]);
char_u *tail;
char_u *p;
int num_files;
int file_count = 0;
char_u **files;
char_u *dirp;
char_u *dir_name;
char_u *fname_res = NULL;
#if defined(HAVE_READLINK)
char_u fname_buf[MAXPATHL];
#endif
if (fname != NULL) {
#if defined(HAVE_READLINK)
if (resolve_symlink(fname, fname_buf) == OK)
fname_res = fname_buf;
else
#endif
fname_res = fname;
}
if (list) {
msg((char_u *)_("Swap files found:"));
msg_putchar('\n');
}
dir_name = xmalloc(STRLEN(p_dir) + 1);
dirp = p_dir;
while (*dirp) {
(void)copy_option_part(&dirp, dir_name, 31000, ",");
if (dir_name[0] == '.' && dir_name[1] == NUL) { 
if (fname == NULL) {
names[0] = vim_strsave((char_u *)"*.sw?");
names[1] = vim_strsave((char_u *)".*.sw?");
names[2] = vim_strsave((char_u *)".sw?");
num_names = 3;
} else
num_names = recov_file_names(names, fname_res, TRUE);
} else { 
if (fname == NULL) {
names[0] = (char_u *)concat_fnames((char *)dir_name, "*.sw?", TRUE);
names[1] = (char_u *)concat_fnames((char *)dir_name, ".*.sw?", TRUE);
names[2] = (char_u *)concat_fnames((char *)dir_name, ".sw?", TRUE);
num_names = 3;
} else {
int len = (int)STRLEN(dir_name);
p = dir_name + len;
if (after_pathsep((char *)dir_name, (char *)p)
&& len > 1
&& p[-1] == p[-2]) {
tail = (char_u *)make_percent_swname((char *)dir_name,
(char *)fname_res);
} else {
tail = path_tail(fname_res);
tail = (char_u *)concat_fnames((char *)dir_name, (char *)tail, TRUE);
}
num_names = recov_file_names(names, tail, FALSE);
xfree(tail);
}
}
if (num_names == 0)
num_files = 0;
else if (expand_wildcards(num_names, names, &num_files, &files,
EW_KEEPALL|EW_FILE|EW_SILENT) == FAIL)
num_files = 0;
if (*dirp == NUL && file_count + num_files == 0 && fname != NULL) {
char_u *swapname = (char_u *)modname((char *)fname_res, ".swp", TRUE);
if (swapname != NULL) {
if (os_path_exists(swapname)) {
files = xmalloc(sizeof(char_u *));
files[0] = swapname;
swapname = NULL;
num_files = 1;
}
xfree(swapname);
}
}
if (curbuf->b_ml.ml_mfp != NULL
&& (p = curbuf->b_ml.ml_mfp->mf_fname) != NULL) {
for (int i = 0; i < num_files; i++) {
if (path_full_compare(p, files[i], true, false) & kEqualFiles) {
xfree(files[i]);
if (--num_files == 0)
xfree(files);
else
for (; i < num_files; ++i)
files[i] = files[i + 1];
}
}
}
if (nr > 0) {
file_count += num_files;
if (nr <= file_count) {
*fname_out = vim_strsave(
files[nr - 1 + num_files - file_count]);
dirp = (char_u *)""; 
}
} else if (list) {
if (dir_name[0] == '.' && dir_name[1] == NUL) {
if (fname == NULL)
MSG_PUTS(_(" In current directory:\n"));
else
MSG_PUTS(_(" Using specified name:\n"));
} else {
MSG_PUTS(_(" In directory "));
msg_home_replace(dir_name);
MSG_PUTS(":\n");
}
if (num_files) {
for (int i = 0; i < num_files; ++i) {
msg_outnum((long)++file_count);
msg_puts(". ");
msg_puts((const char *)path_tail(files[i]));
msg_putchar('\n');
(void)swapfile_info(files[i]);
}
} else
MSG_PUTS(_(" -- none --\n"));
ui_flush();
} else
file_count += num_files;
for (int i = 0; i < num_names; ++i)
xfree(names[i]);
if (num_files > 0)
FreeWild(num_files, files);
}
xfree(dir_name);
return file_count;
}
char *make_percent_swname(const char *dir, char *name)
FUNC_ATTR_NONNULL_ARG(1)
{
char *d = NULL;
char *f = fix_fname(name != NULL ? name : "");
if (f != NULL) {
char *s = xstrdup(f);
for (d = s; *d != NUL; MB_PTR_ADV(d)) {
if (vim_ispathsep(*d)) {
*d = '%';
}
}
d = concat_fnames(dir, s, TRUE);
xfree(s);
xfree(f);
}
return d;
}
static bool process_still_running;
void get_b0_dict(const char *fname, dict_T *d)
{
int fd;
struct block0 b0;
if ((fd = os_open(fname, O_RDONLY, 0)) >= 0) {
if (read_eintr(fd, &b0, sizeof(b0)) == sizeof(b0)) {
if (ml_check_b0_id(&b0) == FAIL) {
tv_dict_add_str(d, S_LEN("error"), "Not a swap file");
} else if (b0_magic_wrong(&b0)) {
tv_dict_add_str(d, S_LEN("error"), "Magic number mismatch");
} else {
tv_dict_add_str_len(d, S_LEN("version"), (char *)b0.b0_version, 10);
tv_dict_add_str_len(d, S_LEN("user"), (char *)b0.b0_uname,
B0_UNAME_SIZE);
tv_dict_add_str_len(d, S_LEN("host"), (char *)b0.b0_hname,
B0_HNAME_SIZE);
tv_dict_add_str_len(d, S_LEN("fname"), (char *)b0.b0_fname,
B0_FNAME_SIZE_ORG);
tv_dict_add_nr(d, S_LEN("pid"), char_to_long(b0.b0_pid));
tv_dict_add_nr(d, S_LEN("mtime"), char_to_long(b0.b0_mtime));
tv_dict_add_nr(d, S_LEN("dirty"), b0.b0_dirty ? 1 : 0);
tv_dict_add_nr(d, S_LEN("inode"), char_to_long(b0.b0_ino));
}
} else {
tv_dict_add_str(d, S_LEN("error"), "Cannot read file");
}
close(fd);
} else {
tv_dict_add_str(d, S_LEN("error"), "Cannot open file");
}
}
static time_t swapfile_info(char_u *fname)
{
assert(fname != NULL);
int fd;
struct block0 b0;
time_t x = (time_t)0;
char *p;
#if defined(UNIX)
char uname[B0_UNAME_SIZE];
#endif
FileInfo file_info;
if (os_fileinfo((char *)fname, &file_info)) {
#if defined(UNIX)
if (os_get_uname(file_info.stat.st_uid, uname, B0_UNAME_SIZE) == OK) {
MSG_PUTS(_(" owned by: "));
msg_outtrans((char_u *)uname);
MSG_PUTS(_(" dated: "));
} else
#endif
MSG_PUTS(_(" dated: "));
x = file_info.stat.st_mtim.tv_sec;
p = ctime(&x); 
if (p == NULL)
MSG_PUTS("(invalid)\n");
else
MSG_PUTS(p);
}
fd = os_open((char *)fname, O_RDONLY, 0);
if (fd >= 0) {
if (read_eintr(fd, &b0, sizeof(b0)) == sizeof(b0)) {
if (STRNCMP(b0.b0_version, "VIM 3.0", 7) == 0) {
MSG_PUTS(_(" [from Vim version 3.0]"));
} else if (ml_check_b0_id(&b0) == FAIL) {
MSG_PUTS(_(" [does not look like a Vim swap file]"));
} else if (!ml_check_b0_strings(&b0)) {
MSG_PUTS(_(" [garbled strings (not nul terminated)]"));
} else {
MSG_PUTS(_(" file name: "));
if (b0.b0_fname[0] == NUL)
MSG_PUTS(_("[No Name]"));
else
msg_outtrans(b0.b0_fname);
MSG_PUTS(_("\n modified: "));
MSG_PUTS(b0.b0_dirty ? _("YES") : _("no"));
if (*(b0.b0_uname) != NUL) {
MSG_PUTS(_("\n user name: "));
msg_outtrans(b0.b0_uname);
}
if (*(b0.b0_hname) != NUL) {
if (*(b0.b0_uname) != NUL)
MSG_PUTS(_(" host name: "));
else
MSG_PUTS(_("\n host name: "));
msg_outtrans(b0.b0_hname);
}
if (char_to_long(b0.b0_pid) != 0L) {
MSG_PUTS(_("\n process ID: "));
msg_outnum(char_to_long(b0.b0_pid));
if (os_proc_running((int)char_to_long(b0.b0_pid))) {
MSG_PUTS(_(" (STILL RUNNING)"));
process_still_running = true;
}
}
if (b0_magic_wrong(&b0)) {
MSG_PUTS(_("\n [not usable on this computer]"));
}
}
} else
MSG_PUTS(_(" [cannot be read]"));
close(fd);
} else
MSG_PUTS(_(" [cannot be opened]"));
msg_putchar('\n');
return x;
}
static time_t swapfile_unchanged(char *fname)
{
struct block0 b0;
int ret = true;
if (!os_path_exists((char_u *)fname)) {
return false;
}
int fd = os_open(fname, O_RDONLY, 0);
if (fd < 0) {
return false;
}
if (read_eintr(fd, &b0, sizeof(b0)) != sizeof(b0)) {
close(fd);
return false;
}
if (ml_check_b0_id(&b0) == FAIL|| b0_magic_wrong(&b0)) {
ret = false;
}
if (b0.b0_dirty) {
ret = false;
}
long pid = char_to_long(b0.b0_pid);
if (pid == 0L || os_proc_running((int)pid)) {
ret = false;
}
close(fd);
return ret;
}
static int recov_file_names(char_u **names, char_u *path, int prepend_dot)
FUNC_ATTR_NONNULL_ALL
{
int num_names = 0;
if (prepend_dot) {
names[num_names] = (char_u *)modname((char *)path, ".sw?", TRUE);
if (names[num_names] == NULL)
return num_names;
++num_names;
}
names[num_names] = (char_u *)concat_fnames((char *)path, ".sw?", FALSE);
if (num_names >= 1) { 
char_u *p = names[num_names - 1];
int i = (int)STRLEN(names[num_names - 1]) - (int)STRLEN(names[num_names]);
if (i > 0)
p += i; 
if (STRCMP(p, names[num_names]) != 0)
++num_names;
else
xfree(names[num_names]);
} else
++num_names;
return num_names;
}
void ml_sync_all(int check_file, int check_char, bool do_fsync)
{
FOR_ALL_BUFFERS(buf) {
if (buf->b_ml.ml_mfp == NULL || buf->b_ml.ml_mfp->mf_fname == NULL)
continue; 
ml_flush_line(buf); 
(void)ml_find_line(buf, (linenr_T)0, ML_FLUSH);
if (bufIsChanged(buf) && check_file && mf_need_trans(buf->b_ml.ml_mfp)
&& buf->b_ffname != NULL) {
FileInfo file_info;
if (!os_fileinfo((char *)buf->b_ffname, &file_info)
|| file_info.stat.st_mtim.tv_sec != buf->b_mtime_read
|| os_fileinfo_size(&file_info) != buf->b_orig_size) {
ml_preserve(buf, false, do_fsync);
did_check_timestamps = false;
need_check_timestamps = true; 
}
}
if (buf->b_ml.ml_mfp->mf_dirty) {
(void)mf_sync(buf->b_ml.ml_mfp, (check_char ? MFS_STOP : 0)
| (do_fsync && bufIsChanged(buf) ? MFS_FLUSH : 0));
if (check_char && os_char_avail()) { 
break;
}
}
}
}
void ml_preserve(buf_T *buf, int message, bool do_fsync)
{
bhdr_T *hp;
linenr_T lnum;
memfile_T *mfp = buf->b_ml.ml_mfp;
int status;
int got_int_save = got_int;
if (mfp == NULL || mfp->mf_fname == NULL) {
if (message)
EMSG(_("E313: Cannot preserve, there is no swap file"));
return;
}
got_int = FALSE;
ml_flush_line(buf); 
(void)ml_find_line(buf, (linenr_T)0, ML_FLUSH); 
status = mf_sync(mfp, MFS_ALL | (do_fsync ? MFS_FLUSH : 0));
buf->b_ml.ml_stack_top = 0;
if (mf_need_trans(mfp) && !got_int) {
lnum = 1;
while (mf_need_trans(mfp) && lnum <= buf->b_ml.ml_line_count) {
hp = ml_find_line(buf, lnum, ML_FIND);
if (hp == NULL) {
status = FAIL;
goto theend;
}
CHECK(buf->b_ml.ml_locked_low != lnum, "low != lnum");
lnum = buf->b_ml.ml_locked_high + 1;
}
(void)ml_find_line(buf, (linenr_T)0, ML_FLUSH); 
if (mf_sync(mfp, MFS_ALL | (do_fsync ? MFS_FLUSH : 0)) == FAIL) {
status = FAIL;
}
buf->b_ml.ml_stack_top = 0; 
}
theend:
got_int |= got_int_save;
if (message) {
if (status == OK)
MSG(_("File preserved"));
else
EMSG(_("E314: Preserve failed"));
}
}
char_u *ml_get(linenr_T lnum)
{
return ml_get_buf(curbuf, lnum, FALSE);
}
char_u *ml_get_pos(const pos_T *pos)
FUNC_ATTR_NONNULL_ALL
{
return ml_get_buf(curbuf, pos->lnum, false) + pos->col;
}
char_u *
ml_get_buf (
buf_T *buf,
linenr_T lnum,
bool will_change 
)
{
bhdr_T *hp;
DATA_BL *dp;
char_u *ptr;
static int recursive = 0;
if (lnum > buf->b_ml.ml_line_count) { 
if (recursive == 0) {
recursive++;
IEMSGN(_("E315: ml_get: invalid lnum: %" PRId64), lnum);
recursive--;
}
errorret:
STRCPY(IObuff, "???");
return IObuff;
}
if (lnum <= 0) 
lnum = 1;
if (buf->b_ml.ml_mfp == NULL) 
return (char_u *)"";
if (buf->b_ml.ml_line_lnum != lnum) {
ml_flush_line(buf);
if ((hp = ml_find_line(buf, lnum, ML_FIND)) == NULL) {
if (recursive == 0) {
recursive++;
get_trans_bufname(buf);
shorten_dir(NameBuff);
iemsgf(_("E316: ml_get: cannot find line %" PRId64 " in buffer %d %s"),
lnum, buf->b_fnum, NameBuff);
recursive--;
}
goto errorret;
}
dp = hp->bh_data;
ptr = (char_u *)dp +
((dp->db_index[lnum - buf->b_ml.ml_locked_low]) & DB_INDEX_MASK);
buf->b_ml.ml_line_ptr = ptr;
buf->b_ml.ml_line_lnum = lnum;
buf->b_ml.ml_flags &= ~ML_LINE_DIRTY;
}
if (will_change)
buf->b_ml.ml_flags |= (ML_LOCKED_DIRTY | ML_LOCKED_POS);
return buf->b_ml.ml_line_ptr;
}
int ml_line_alloced(void)
{
return curbuf->b_ml.ml_flags & ML_LINE_DIRTY;
}
int ml_append(
linenr_T lnum, 
char_u *line, 
colnr_T len, 
bool newfile 
)
{
if (curbuf->b_ml.ml_mfp == NULL && open_buffer(FALSE, NULL, 0) == FAIL)
return FAIL;
if (curbuf->b_ml.ml_line_lnum != 0)
ml_flush_line(curbuf);
return ml_append_int(curbuf, lnum, line, len, newfile, FALSE);
}
int ml_append_buf(
buf_T *buf,
linenr_T lnum, 
char_u *line, 
colnr_T len, 
bool newfile 
)
FUNC_ATTR_NONNULL_ARG(1)
{
if (buf->b_ml.ml_mfp == NULL)
return FAIL;
if (buf->b_ml.ml_line_lnum != 0)
ml_flush_line(buf);
return ml_append_int(buf, lnum, line, len, newfile, FALSE);
}
static int ml_append_int(
buf_T *buf,
linenr_T lnum, 
char_u *line, 
colnr_T len, 
bool newfile, 
int mark 
)
{
int i;
int line_count; 
int offset;
int from, to;
int space_needed; 
int page_size;
int page_count;
int db_idx; 
bhdr_T *hp;
memfile_T *mfp;
DATA_BL *dp;
PTR_BL *pp;
infoptr_T *ip;
if (lnum > buf->b_ml.ml_line_count || buf->b_ml.ml_mfp == NULL)
return FAIL;
if (lowest_marked && lowest_marked > lnum)
lowest_marked = lnum + 1;
if (len == 0)
len = (colnr_T)STRLEN(line) + 1; 
space_needed = len + INDEX_SIZE; 
mfp = buf->b_ml.ml_mfp;
page_size = mfp->mf_page_size;
if ((hp = ml_find_line(buf, lnum == 0 ? (linenr_T)1 : lnum,
ML_INSERT)) == NULL)
return FAIL;
buf->b_ml.ml_flags &= ~ML_EMPTY;
if (lnum == 0) 
db_idx = -1; 
else
db_idx = lnum - buf->b_ml.ml_locked_low;
line_count = buf->b_ml.ml_locked_high - buf->b_ml.ml_locked_low;
dp = hp->bh_data;
if ((int)dp->db_free < space_needed && db_idx == line_count - 1
&& lnum < buf->b_ml.ml_line_count) {
--(buf->b_ml.ml_locked_lineadd);
--(buf->b_ml.ml_locked_high);
if ((hp = ml_find_line(buf, lnum + 1, ML_INSERT)) == NULL)
return FAIL;
db_idx = -1; 
line_count = buf->b_ml.ml_locked_high - buf->b_ml.ml_locked_low;
CHECK(buf->b_ml.ml_locked_low != lnum + 1, "locked_low != lnum + 1");
dp = hp->bh_data;
}
++buf->b_ml.ml_line_count;
if ((int)dp->db_free >= space_needed) { 
dp->db_txt_start -= len;
dp->db_free -= space_needed;
++(dp->db_line_count);
if (line_count > db_idx + 1) { 
if (db_idx < 0)
offset = dp->db_txt_end;
else
offset = ((dp->db_index[db_idx]) & DB_INDEX_MASK);
memmove((char *)dp + dp->db_txt_start,
(char *)dp + dp->db_txt_start + len,
(size_t)(offset - (dp->db_txt_start + len)));
for (i = line_count - 1; i > db_idx; --i)
dp->db_index[i + 1] = dp->db_index[i] - len;
dp->db_index[db_idx + 1] = offset - len;
} else 
dp->db_index[db_idx + 1] = dp->db_txt_start;
memmove((char *)dp + dp->db_index[db_idx + 1], line, (size_t)len);
if (mark)
dp->db_index[db_idx + 1] |= DB_MARKED;
buf->b_ml.ml_flags |= ML_LOCKED_DIRTY;
if (!newfile)
buf->b_ml.ml_flags |= ML_LOCKED_POS;
} else { 
long line_count_left, line_count_right;
int page_count_left, page_count_right;
bhdr_T *hp_left;
bhdr_T *hp_right;
bhdr_T *hp_new;
int lines_moved;
int data_moved = 0; 
int total_moved = 0; 
DATA_BL *dp_right, *dp_left;
int stack_idx;
int in_left;
int lineadd;
blocknr_T bnum_left, bnum_right;
linenr_T lnum_left, lnum_right;
int pb_idx;
PTR_BL *pp_new;
if (db_idx < 0) { 
lines_moved = 0;
in_left = TRUE;
} else { 
lines_moved = line_count - db_idx - 1;
if (lines_moved == 0)
in_left = FALSE; 
else {
data_moved = ((dp->db_index[db_idx]) & DB_INDEX_MASK) -
dp->db_txt_start;
total_moved = data_moved + lines_moved * INDEX_SIZE;
if ((int)dp->db_free + total_moved >= space_needed) {
in_left = TRUE; 
space_needed = total_moved;
} else {
in_left = FALSE; 
space_needed += total_moved;
}
}
}
page_count = ((space_needed + HEADER_SIZE) + page_size - 1) / page_size;
hp_new = ml_new_data(mfp, newfile, page_count);
if (db_idx < 0) { 
hp_left = hp_new;
hp_right = hp;
line_count_left = 0;
line_count_right = line_count;
} else { 
hp_left = hp;
hp_right = hp_new;
line_count_left = line_count;
line_count_right = 0;
}
dp_right = hp_right->bh_data;
dp_left = hp_left->bh_data;
bnum_left = hp_left->bh_bnum;
bnum_right = hp_right->bh_bnum;
page_count_left = hp_left->bh_page_count;
page_count_right = hp_right->bh_page_count;
if (!in_left) {
dp_right->db_txt_start -= len;
dp_right->db_free -= len + INDEX_SIZE;
dp_right->db_index[0] = dp_right->db_txt_start;
if (mark)
dp_right->db_index[0] |= DB_MARKED;
memmove((char *)dp_right + dp_right->db_txt_start,
line, (size_t)len);
++line_count_right;
}
if (lines_moved) {
dp_right->db_txt_start -= data_moved;
dp_right->db_free -= total_moved;
memmove((char *)dp_right + dp_right->db_txt_start,
(char *)dp_left + dp_left->db_txt_start,
(size_t)data_moved);
offset = dp_right->db_txt_start - dp_left->db_txt_start;
dp_left->db_txt_start += data_moved;
dp_left->db_free += total_moved;
for (to = line_count_right, from = db_idx + 1;
from < line_count_left; ++from, ++to)
dp_right->db_index[to] = dp->db_index[from] + offset;
line_count_right += lines_moved;
line_count_left -= lines_moved;
}
if (in_left) {
dp_left->db_txt_start -= len;
dp_left->db_free -= len + INDEX_SIZE;
dp_left->db_index[line_count_left] = dp_left->db_txt_start;
if (mark)
dp_left->db_index[line_count_left] |= DB_MARKED;
memmove((char *)dp_left + dp_left->db_txt_start,
line, (size_t)len);
++line_count_left;
}
if (db_idx < 0) { 
lnum_left = lnum + 1;
lnum_right = 0;
} else { 
lnum_left = 0;
if (in_left)
lnum_right = lnum + 2;
else
lnum_right = lnum + 1;
}
dp_left->db_line_count = line_count_left;
dp_right->db_line_count = line_count_right;
if (lines_moved || in_left)
buf->b_ml.ml_flags |= ML_LOCKED_DIRTY;
if (!newfile && db_idx >= 0 && in_left)
buf->b_ml.ml_flags |= ML_LOCKED_POS;
mf_put(mfp, hp_new, true, false);
lineadd = buf->b_ml.ml_locked_lineadd;
buf->b_ml.ml_locked_lineadd = 0;
ml_find_line(buf, (linenr_T)0, ML_FLUSH); 
for (stack_idx = buf->b_ml.ml_stack_top - 1; stack_idx >= 0;
--stack_idx) {
ip = &(buf->b_ml.ml_stack[stack_idx]);
pb_idx = ip->ip_index;
if ((hp = mf_get(mfp, ip->ip_bnum, 1)) == NULL)
return FAIL;
pp = hp->bh_data; 
if (pp->pb_id != PTR_ID) {
IEMSG(_("E317: pointer block id wrong 3"));
mf_put(mfp, hp, false, false);
return FAIL;
}
if (pp->pb_count < pp->pb_count_max) {
if (pb_idx + 1 < (int)pp->pb_count)
memmove(&pp->pb_pointer[pb_idx + 2],
&pp->pb_pointer[pb_idx + 1],
(size_t)(pp->pb_count - pb_idx - 1) * sizeof(PTR_EN));
++pp->pb_count;
pp->pb_pointer[pb_idx].pe_line_count = line_count_left;
pp->pb_pointer[pb_idx].pe_bnum = bnum_left;
pp->pb_pointer[pb_idx].pe_page_count = page_count_left;
pp->pb_pointer[pb_idx + 1].pe_line_count = line_count_right;
pp->pb_pointer[pb_idx + 1].pe_bnum = bnum_right;
pp->pb_pointer[pb_idx + 1].pe_page_count = page_count_right;
if (lnum_left != 0)
pp->pb_pointer[pb_idx].pe_old_lnum = lnum_left;
if (lnum_right != 0)
pp->pb_pointer[pb_idx + 1].pe_old_lnum = lnum_right;
mf_put(mfp, hp, true, false);
buf->b_ml.ml_stack_top = stack_idx + 1; 
if (lineadd) {
--(buf->b_ml.ml_stack_top);
ml_lineadd(buf, lineadd);
buf->b_ml.ml_stack[buf->b_ml.ml_stack_top].ip_high +=
lineadd;
++(buf->b_ml.ml_stack_top);
}
break;
} else { 
for (;; ) { 
hp_new = ml_new_ptr(mfp);
if (hp_new == NULL) 
return FAIL;
pp_new = hp_new->bh_data;
if (hp->bh_bnum != 1)
break;
memmove(pp_new, pp, (size_t)page_size);
pp->pb_count = 1;
pp->pb_pointer[0].pe_bnum = hp_new->bh_bnum;
pp->pb_pointer[0].pe_line_count = buf->b_ml.ml_line_count;
pp->pb_pointer[0].pe_old_lnum = 1;
pp->pb_pointer[0].pe_page_count = 1;
mf_put(mfp, hp, true, false); 
hp = hp_new; 
pp = pp_new;
CHECK(stack_idx != 0, _("stack_idx should be 0"));
ip->ip_index = 0;
++stack_idx; 
}
total_moved = pp->pb_count - pb_idx - 1;
if (total_moved) {
memmove(&pp_new->pb_pointer[0],
&pp->pb_pointer[pb_idx + 1],
(size_t)(total_moved) * sizeof(PTR_EN));
pp_new->pb_count = total_moved;
pp->pb_count -= total_moved - 1;
pp->pb_pointer[pb_idx + 1].pe_bnum = bnum_right;
pp->pb_pointer[pb_idx + 1].pe_line_count = line_count_right;
pp->pb_pointer[pb_idx + 1].pe_page_count = page_count_right;
if (lnum_right)
pp->pb_pointer[pb_idx + 1].pe_old_lnum = lnum_right;
} else {
pp_new->pb_count = 1;
pp_new->pb_pointer[0].pe_bnum = bnum_right;
pp_new->pb_pointer[0].pe_line_count = line_count_right;
pp_new->pb_pointer[0].pe_page_count = page_count_right;
pp_new->pb_pointer[0].pe_old_lnum = lnum_right;
}
pp->pb_pointer[pb_idx].pe_bnum = bnum_left;
pp->pb_pointer[pb_idx].pe_line_count = line_count_left;
pp->pb_pointer[pb_idx].pe_page_count = page_count_left;
if (lnum_left)
pp->pb_pointer[pb_idx].pe_old_lnum = lnum_left;
lnum_left = 0;
lnum_right = 0;
line_count_right = 0;
for (i = 0; i < (int)pp_new->pb_count; ++i)
line_count_right += pp_new->pb_pointer[i].pe_line_count;
line_count_left = 0;
for (i = 0; i < (int)pp->pb_count; ++i)
line_count_left += pp->pb_pointer[i].pe_line_count;
bnum_left = hp->bh_bnum;
bnum_right = hp_new->bh_bnum;
page_count_left = 1;
page_count_right = 1;
mf_put(mfp, hp, true, false);
mf_put(mfp, hp_new, true, false);
}
}
if (stack_idx < 0) {
IEMSG(_("E318: Updated too many blocks?"));
buf->b_ml.ml_stack_top = 0; 
}
}
ml_updatechunk(buf, lnum + 1, (long)len, ML_CHNK_ADDLINE);
return OK;
}
void ml_add_deleted_len(char_u *ptr, ssize_t len)
{
if (inhibit_delete_count) {
return;
}
if (len == -1) {
len = STRLEN(ptr);
}
curbuf->deleted_bytes += len+1;
if (curbuf->update_need_codepoints) {
mb_utflen(ptr, len, &curbuf->deleted_codepoints,
&curbuf->deleted_codeunits);
curbuf->deleted_codepoints++; 
curbuf->deleted_codeunits++;
}
}
int ml_replace(linenr_T lnum, char_u *line, bool copy)
{
if (line == NULL) 
return FAIL;
if (curbuf->b_ml.ml_mfp == NULL && open_buffer(FALSE, NULL, 0) == FAIL)
return FAIL;
bool readlen = true;
if (copy) {
line = vim_strsave(line);
}
if (curbuf->b_ml.ml_line_lnum != lnum) { 
ml_flush_line(curbuf); 
} else if (curbuf->b_ml.ml_flags & ML_LINE_DIRTY) { 
ml_add_deleted_len(curbuf->b_ml.ml_line_ptr, -1);
readlen = false; 
xfree(curbuf->b_ml.ml_line_ptr); 
}
if (readlen && kv_size(curbuf->update_callbacks)) {
ml_add_deleted_len(ml_get_buf(curbuf, lnum, false), -1);
}
curbuf->b_ml.ml_line_ptr = line;
curbuf->b_ml.ml_line_lnum = lnum;
curbuf->b_ml.ml_flags = (curbuf->b_ml.ml_flags | ML_LINE_DIRTY) & ~ML_EMPTY;
return OK;
}
int ml_delete(linenr_T lnum, bool message)
{
ml_flush_line(curbuf);
return ml_delete_int(curbuf, lnum, message);
}
static int ml_delete_int(buf_T *buf, linenr_T lnum, bool message)
{
bhdr_T *hp;
memfile_T *mfp;
DATA_BL *dp;
PTR_BL *pp;
infoptr_T *ip;
int count; 
int idx;
int stack_idx;
int text_start;
int line_start;
long line_size;
int i;
if (lnum < 1 || lnum > buf->b_ml.ml_line_count)
return FAIL;
if (lowest_marked && lowest_marked > lnum)
lowest_marked--;
if (buf->b_ml.ml_line_count == 1) { 
if (message
)
set_keep_msg((char_u *)_(no_lines_msg), 0);
i = ml_replace((linenr_T)1, (char_u *)"", true);
buf->b_ml.ml_flags |= ML_EMPTY;
return i;
}
mfp = buf->b_ml.ml_mfp;
if (mfp == NULL)
return FAIL;
if ((hp = ml_find_line(buf, lnum, ML_DELETE)) == NULL)
return FAIL;
dp = hp->bh_data;
count = (long)(buf->b_ml.ml_locked_high)
- (long)(buf->b_ml.ml_locked_low) + 2;
idx = lnum - buf->b_ml.ml_locked_low;
--buf->b_ml.ml_line_count;
line_start = ((dp->db_index[idx]) & DB_INDEX_MASK);
if (idx == 0) 
line_size = dp->db_txt_end - line_start;
else
line_size = ((dp->db_index[idx - 1]) & DB_INDEX_MASK) - line_start;
assert(line_size >= 1);
ml_add_deleted_len((char_u *)dp + line_start, line_size-1);
if (count == 1) {
mf_free(mfp, hp); 
buf->b_ml.ml_locked = NULL;
for (stack_idx = buf->b_ml.ml_stack_top - 1; stack_idx >= 0;
--stack_idx) {
buf->b_ml.ml_stack_top = 0; 
ip = &(buf->b_ml.ml_stack[stack_idx]);
idx = ip->ip_index;
if ((hp = mf_get(mfp, ip->ip_bnum, 1)) == NULL)
return FAIL;
pp = hp->bh_data; 
if (pp->pb_id != PTR_ID) {
IEMSG(_("E317: pointer block id wrong 4"));
mf_put(mfp, hp, false, false);
return FAIL;
}
count = --(pp->pb_count);
if (count == 0) 
mf_free(mfp, hp);
else {
if (count != idx) 
memmove(&pp->pb_pointer[idx], &pp->pb_pointer[idx + 1],
(size_t)(count - idx) * sizeof(PTR_EN));
mf_put(mfp, hp, true, false);
buf->b_ml.ml_stack_top = stack_idx; 
if (buf->b_ml.ml_locked_lineadd != 0) {
ml_lineadd(buf, buf->b_ml.ml_locked_lineadd);
buf->b_ml.ml_stack[buf->b_ml.ml_stack_top].ip_high +=
buf->b_ml.ml_locked_lineadd;
}
++(buf->b_ml.ml_stack_top);
break;
}
}
CHECK(stack_idx < 0, _("deleted block 1?"));
} else {
text_start = dp->db_txt_start;
memmove((char *)dp + text_start + line_size,
(char *)dp + text_start, (size_t)(line_start - text_start));
for (i = idx; i < count - 1; ++i)
dp->db_index[i] = dp->db_index[i + 1] + line_size;
dp->db_free += line_size + INDEX_SIZE;
dp->db_txt_start += line_size;
--(dp->db_line_count);
buf->b_ml.ml_flags |= (ML_LOCKED_DIRTY | ML_LOCKED_POS);
}
ml_updatechunk(buf, lnum, line_size, ML_CHNK_DELLINE);
return OK;
}
void ml_setmarked(linenr_T lnum)
{
bhdr_T *hp;
DATA_BL *dp;
if (lnum < 1 || lnum > curbuf->b_ml.ml_line_count
|| curbuf->b_ml.ml_mfp == NULL)
return; 
if (lowest_marked == 0 || lowest_marked > lnum)
lowest_marked = lnum;
if ((hp = ml_find_line(curbuf, lnum, ML_FIND)) == NULL)
return; 
dp = hp->bh_data;
dp->db_index[lnum - curbuf->b_ml.ml_locked_low] |= DB_MARKED;
curbuf->b_ml.ml_flags |= ML_LOCKED_DIRTY;
}
linenr_T ml_firstmarked(void)
{
bhdr_T *hp;
DATA_BL *dp;
linenr_T lnum;
int i;
if (curbuf->b_ml.ml_mfp == NULL)
return (linenr_T) 0;
for (lnum = lowest_marked; lnum <= curbuf->b_ml.ml_line_count; ) {
if ((hp = ml_find_line(curbuf, lnum, ML_FIND)) == NULL)
return (linenr_T)0; 
dp = hp->bh_data;
for (i = lnum - curbuf->b_ml.ml_locked_low;
lnum <= curbuf->b_ml.ml_locked_high; ++i, ++lnum)
if ((dp->db_index[i]) & DB_MARKED) {
(dp->db_index[i]) &= DB_INDEX_MASK;
curbuf->b_ml.ml_flags |= ML_LOCKED_DIRTY;
lowest_marked = lnum + 1;
return lnum;
}
}
return (linenr_T) 0;
}
void ml_clearmarked(void)
{
bhdr_T *hp;
DATA_BL *dp;
linenr_T lnum;
int i;
if (curbuf->b_ml.ml_mfp == NULL) 
return;
for (lnum = lowest_marked; lnum <= curbuf->b_ml.ml_line_count; ) {
if ((hp = ml_find_line(curbuf, lnum, ML_FIND)) == NULL)
return; 
dp = hp->bh_data;
for (i = lnum - curbuf->b_ml.ml_locked_low;
lnum <= curbuf->b_ml.ml_locked_high; ++i, ++lnum)
if ((dp->db_index[i]) & DB_MARKED) {
(dp->db_index[i]) &= DB_INDEX_MASK;
curbuf->b_ml.ml_flags |= ML_LOCKED_DIRTY;
}
}
lowest_marked = 0;
return;
}
size_t ml_flush_deleted_bytes(buf_T *buf, size_t *codepoints, size_t *codeunits)
{
size_t ret = buf->deleted_bytes;
*codepoints = buf->deleted_codepoints;
*codeunits = buf->deleted_codeunits;
buf->deleted_bytes = 0;
buf->deleted_codepoints = 0;
buf->deleted_codeunits = 0;
return ret;
}
static void ml_flush_line(buf_T *buf)
{
bhdr_T *hp;
DATA_BL *dp;
linenr_T lnum;
char_u *new_line;
char_u *old_line;
colnr_T new_len;
int old_len;
int extra;
int idx;
int start;
int count;
int i;
static int entered = FALSE;
if (buf->b_ml.ml_line_lnum == 0 || buf->b_ml.ml_mfp == NULL)
return; 
if (buf->b_ml.ml_flags & ML_LINE_DIRTY) {
if (entered)
return;
entered = TRUE;
buf->flush_count++;
lnum = buf->b_ml.ml_line_lnum;
new_line = buf->b_ml.ml_line_ptr;
hp = ml_find_line(buf, lnum, ML_FIND);
if (hp == NULL) {
IEMSGN(_("E320: Cannot find line %" PRId64), lnum);
} else {
dp = hp->bh_data;
idx = lnum - buf->b_ml.ml_locked_low;
start = ((dp->db_index[idx]) & DB_INDEX_MASK);
old_line = (char_u *)dp + start;
if (idx == 0) 
old_len = dp->db_txt_end - start;
else 
old_len = (dp->db_index[idx - 1] & DB_INDEX_MASK) - start;
new_len = (colnr_T)STRLEN(new_line) + 1;
extra = new_len - old_len; 
if ((int)dp->db_free >= extra) {
count = buf->b_ml.ml_locked_high - buf->b_ml.ml_locked_low + 1;
if (extra != 0 && idx < count - 1) {
memmove((char *)dp + dp->db_txt_start - extra,
(char *)dp + dp->db_txt_start,
(size_t)(start - dp->db_txt_start));
for (i = idx + 1; i < count; ++i)
dp->db_index[i] -= extra;
}
dp->db_index[idx] -= extra;
dp->db_free -= extra;
dp->db_txt_start -= extra;
memmove(old_line - extra, new_line, (size_t)new_len);
buf->b_ml.ml_flags |= (ML_LOCKED_DIRTY | ML_LOCKED_POS);
ml_updatechunk(buf, lnum, (long)extra, ML_CHNK_UPDLINE);
} else {
(void)ml_append_int(buf, lnum, new_line, new_len, false,
(dp->db_index[idx] & DB_MARKED));
(void)ml_delete_int(buf, lnum, false);
}
}
xfree(new_line);
entered = FALSE;
}
buf->b_ml.ml_line_lnum = 0;
}
static bhdr_T *ml_new_data(memfile_T *mfp, bool negative, int page_count)
{
assert(page_count >= 0);
bhdr_T *hp = mf_new(mfp, negative, (unsigned)page_count);
DATA_BL *dp = hp->bh_data;
dp->db_id = DATA_ID;
dp->db_txt_start = dp->db_txt_end = page_count * mfp->mf_page_size;
dp->db_free = dp->db_txt_start - HEADER_SIZE;
dp->db_line_count = 0;
return hp;
}
static bhdr_T *ml_new_ptr(memfile_T *mfp)
{
bhdr_T *hp = mf_new(mfp, false, 1);
PTR_BL *pp = hp->bh_data;
pp->pb_id = PTR_ID;
pp->pb_count = 0;
pp->pb_count_max = (mfp->mf_page_size - sizeof(PTR_BL)) / sizeof(PTR_EN) + 1;
return hp;
}
static bhdr_T *ml_find_line(buf_T *buf, linenr_T lnum, int action)
{
DATA_BL *dp;
PTR_BL *pp;
infoptr_T *ip;
bhdr_T *hp;
memfile_T *mfp;
linenr_T t;
blocknr_T bnum, bnum2;
int dirty;
linenr_T low, high;
int top;
int page_count;
int idx;
mfp = buf->b_ml.ml_mfp;
if (buf->b_ml.ml_locked) {
if (ML_SIMPLE(action)
&& buf->b_ml.ml_locked_low <= lnum
&& buf->b_ml.ml_locked_high >= lnum) {
if (action == ML_INSERT) {
++(buf->b_ml.ml_locked_lineadd);
++(buf->b_ml.ml_locked_high);
} else if (action == ML_DELETE) {
--(buf->b_ml.ml_locked_lineadd);
--(buf->b_ml.ml_locked_high);
}
return buf->b_ml.ml_locked;
}
mf_put(mfp, buf->b_ml.ml_locked, buf->b_ml.ml_flags & ML_LOCKED_DIRTY,
buf->b_ml.ml_flags & ML_LOCKED_POS);
buf->b_ml.ml_locked = NULL;
if (buf->b_ml.ml_locked_lineadd != 0)
ml_lineadd(buf, buf->b_ml.ml_locked_lineadd);
}
if (action == ML_FLUSH) 
return NULL;
bnum = 1; 
page_count = 1;
low = 1;
high = buf->b_ml.ml_line_count;
if (action == ML_FIND) { 
for (top = buf->b_ml.ml_stack_top - 1; top >= 0; --top) {
ip = &(buf->b_ml.ml_stack[top]);
if (ip->ip_low <= lnum && ip->ip_high >= lnum) {
bnum = ip->ip_bnum;
low = ip->ip_low;
high = ip->ip_high;
buf->b_ml.ml_stack_top = top; 
break;
}
}
if (top < 0)
buf->b_ml.ml_stack_top = 0; 
} else 
buf->b_ml.ml_stack_top = 0; 
for (;; ) {
if ((hp = mf_get(mfp, bnum, page_count)) == NULL)
goto error_noblock;
if (action == ML_INSERT)
++high;
else if (action == ML_DELETE)
--high;
dp = hp->bh_data;
if (dp->db_id == DATA_ID) { 
buf->b_ml.ml_locked = hp;
buf->b_ml.ml_locked_low = low;
buf->b_ml.ml_locked_high = high;
buf->b_ml.ml_locked_lineadd = 0;
buf->b_ml.ml_flags &= ~(ML_LOCKED_DIRTY | ML_LOCKED_POS);
return hp;
}
pp = (PTR_BL *)(dp); 
if (pp->pb_id != PTR_ID) {
IEMSG(_("E317: pointer block id wrong"));
goto error_block;
}
top = ml_add_stack(buf); 
ip = &(buf->b_ml.ml_stack[top]);
ip->ip_bnum = bnum;
ip->ip_low = low;
ip->ip_high = high;
ip->ip_index = -1; 
dirty = FALSE;
for (idx = 0; idx < (int)pp->pb_count; ++idx) {
t = pp->pb_pointer[idx].pe_line_count;
CHECK(t == 0, _("pe_line_count is zero"));
if ((low += t) > lnum) {
ip->ip_index = idx;
bnum = pp->pb_pointer[idx].pe_bnum;
page_count = pp->pb_pointer[idx].pe_page_count;
high = low - 1;
low -= t;
if (bnum < 0) {
bnum2 = mf_trans_del(mfp, bnum);
if (bnum != bnum2) {
bnum = bnum2;
pp->pb_pointer[idx].pe_bnum = bnum;
dirty = TRUE;
}
}
break;
}
}
if (idx >= (int)pp->pb_count) { 
if (lnum > buf->b_ml.ml_line_count) {
IEMSGN(_("E322: line number out of range: %" PRId64 " past the end"),
lnum - buf->b_ml.ml_line_count);
} else {
IEMSGN(_("E323: line count wrong in block %" PRId64), bnum);
}
goto error_block;
}
if (action == ML_DELETE) {
pp->pb_pointer[idx].pe_line_count--;
dirty = TRUE;
} else if (action == ML_INSERT) {
pp->pb_pointer[idx].pe_line_count++;
dirty = TRUE;
}
mf_put(mfp, hp, dirty, false);
}
error_block:
mf_put(mfp, hp, false, false);
error_noblock:
if (action == ML_DELETE)
ml_lineadd(buf, 1);
else if (action == ML_INSERT)
ml_lineadd(buf, -1);
buf->b_ml.ml_stack_top = 0;
return NULL;
}
static int ml_add_stack(buf_T *buf)
{
int top = buf->b_ml.ml_stack_top;
if (top == buf->b_ml.ml_stack_size) {
CHECK(top > 0, _("Stack size increases")); 
buf->b_ml.ml_stack_size += STACK_INCR;
size_t new_size = sizeof(infoptr_T) * buf->b_ml.ml_stack_size;
buf->b_ml.ml_stack = xrealloc(buf->b_ml.ml_stack, new_size);
}
buf->b_ml.ml_stack_top++;
return top;
}
static void ml_lineadd(buf_T *buf, int count)
{
int idx;
infoptr_T *ip;
PTR_BL *pp;
memfile_T *mfp = buf->b_ml.ml_mfp;
bhdr_T *hp;
for (idx = buf->b_ml.ml_stack_top - 1; idx >= 0; --idx) {
ip = &(buf->b_ml.ml_stack[idx]);
if ((hp = mf_get(mfp, ip->ip_bnum, 1)) == NULL)
break;
pp = hp->bh_data; 
if (pp->pb_id != PTR_ID) {
mf_put(mfp, hp, false, false);
IEMSG(_("E317: pointer block id wrong 2"));
break;
}
pp->pb_pointer[ip->ip_index].pe_line_count += count;
ip->ip_high += count;
mf_put(mfp, hp, true, false);
}
}
#if defined(HAVE_READLINK)
int resolve_symlink(const char_u *fname, char_u *buf)
{
char_u tmp[MAXPATHL];
int ret;
int depth = 0;
if (fname == NULL)
return FAIL;
STRLCPY(tmp, fname, MAXPATHL);
for (;; ) {
if (++depth == 100) {
EMSG2(_("E773: Symlink loop for \"%s\""), fname);
return FAIL;
}
ret = readlink((char *)tmp, (char *)buf, MAXPATHL - 1);
if (ret <= 0) {
if (errno == EINVAL || errno == ENOENT) {
if (depth == 1)
return FAIL;
break;
}
return FAIL;
}
buf[ret] = NUL;
if (path_is_absolute(buf)) {
STRCPY(tmp, buf);
} else {
char_u *tail = path_tail(tmp);
if (STRLEN(tail) + STRLEN(buf) >= MAXPATHL) {
return FAIL;
}
STRCPY(tail, buf);
}
}
return vim_FullName((char *)tmp, (char *)buf, MAXPATHL, TRUE);
}
#endif
char_u *makeswapname(char_u *fname, char_u *ffname, buf_T *buf, char_u *dir_name)
{
char_u *r, *s;
char_u *fname_res = fname;
#if defined(HAVE_READLINK)
char_u fname_buf[MAXPATHL];
#endif
int len = (int)STRLEN(dir_name);
s = dir_name + len;
if (after_pathsep((char *)dir_name, (char *)s)
&& len > 1
&& s[-1] == s[-2]) { 
r = NULL;
if ((s = (char_u *)make_percent_swname((char *)dir_name, (char *)fname)) != NULL) {
r = (char_u *)modname((char *)s, ".swp", FALSE);
xfree(s);
}
return r;
}
#if defined(HAVE_READLINK)
if (resolve_symlink(fname, fname_buf) == OK)
fname_res = fname_buf;
#endif
r = (char_u *)modname((char *)fname_res, ".swp",
dir_name[0] == '.' && dir_name[1] == NUL);
if (r == NULL) 
return NULL;
s = get_file_in_dir(r, dir_name);
xfree(r);
return s;
}
char_u *
get_file_in_dir (
char_u *fname,
char_u *dname 
)
{
char_u *t;
char_u *tail;
char_u *retval;
int save_char;
tail = path_tail(fname);
if (dname[0] == '.' && dname[1] == NUL)
retval = vim_strsave(fname);
else if (dname[0] == '.' && vim_ispathsep(dname[1])) {
if (tail == fname) 
retval = (char_u *)concat_fnames((char *)dname + 2, (char *)tail, TRUE);
else {
save_char = *tail;
*tail = NUL;
t = (char_u *)concat_fnames((char *)fname, (char *)dname + 2, TRUE);
*tail = save_char;
retval = (char_u *)concat_fnames((char *)t, (char *)tail, TRUE);
xfree(t);
}
} else {
retval = (char_u *)concat_fnames((char *)dname, (char *)tail, TRUE);
}
return retval;
}
static void 
attention_message (
buf_T *buf, 
char_u *fname 
)
{
assert(buf->b_fname != NULL);
time_t x, sx;
char *p;
++no_wait_return;
(void)EMSG(_("E325: ATTENTION"));
MSG_PUTS(_("\nFound a swap file by the name \""));
msg_home_replace(fname);
MSG_PUTS("\"\n");
sx = swapfile_info(fname);
MSG_PUTS(_("While opening file \""));
msg_outtrans(buf->b_fname);
MSG_PUTS("\"\n");
FileInfo file_info;
if (!os_fileinfo((char *)buf->b_fname, &file_info)) {
MSG_PUTS(_(" CANNOT BE FOUND"));
} else {
MSG_PUTS(_(" dated: "));
x = file_info.stat.st_mtim.tv_sec;
p = ctime(&x); 
if (p == NULL)
MSG_PUTS("(invalid)\n");
else
MSG_PUTS(p);
if (sx != 0 && x > sx)
MSG_PUTS(_(" NEWER than swap file!\n"));
}
MSG_PUTS(_("\n(1) Another program may be editing the same file. If this is"
" the case,\n be careful not to end up with two different"
" instances of the same\n file when making changes."
" Quit, or continue with caution.\n"));
MSG_PUTS(_("(2) An edit session for this file crashed.\n"));
MSG_PUTS(_(" If this is the case, use \":recover\" or \"vim -r "));
msg_outtrans(buf->b_fname);
MSG_PUTS(_("\"\n to recover the changes (see \":help recovery\").\n"));
MSG_PUTS(_(" If you did this already, delete the swap file \""));
msg_outtrans(fname);
MSG_PUTS(_("\"\n to avoid this message.\n"));
cmdline_row = msg_row;
--no_wait_return;
}
static int do_swapexists(buf_T *buf, char_u *fname)
{
set_vim_var_string(VV_SWAPNAME, (char *) fname, -1);
set_vim_var_string(VV_SWAPCHOICE, NULL, -1);
++allbuf_lock;
apply_autocmds(EVENT_SWAPEXISTS, buf->b_fname, NULL, FALSE, NULL);
--allbuf_lock;
set_vim_var_string(VV_SWAPNAME, NULL, -1);
switch (*get_vim_var_str(VV_SWAPCHOICE)) {
case 'o': return 1;
case 'e': return 2;
case 'r': return 3;
case 'd': return 4;
case 'q': return 5;
case 'a': return 6;
}
return 0;
}
static char *findswapname(buf_T *buf, char **dirp, char *old_fname,
bool *found_existing_dir)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ARG(1, 2, 4)
{
char *fname;
size_t n;
char *dir_name;
char *buf_fname = (char *) buf->b_fname;
const size_t dir_len = strlen(*dirp) + 1;
dir_name = xmalloc(dir_len);
(void)copy_option_part((char_u **) dirp, (char_u *) dir_name, dir_len, ",");
fname = (char *)makeswapname((char_u *)buf_fname, buf->b_ffname, buf,
(char_u *)dir_name);
for (;; ) {
if (fname == NULL) { 
break;
}
if ((n = strlen(fname)) == 0) { 
XFREE_CLEAR(fname);
break;
}
FileInfo file_info;
bool file_or_link_found = os_fileinfo_link(fname, &file_info);
if (!file_or_link_found) {
break;
}
if (old_fname != NULL && fnamecmp(fname, old_fname) == 0) {
break;
}
if (fname[n - 2] == 'w' && fname[n - 1] == 'p') { 
if (!recoverymode && buf_fname != NULL
&& !buf->b_help && !(buf->b_flags & BF_DUMMY)) {
int fd;
struct block0 b0;
int differ = FALSE;
fd = os_open(fname, O_RDONLY, 0);
if (fd >= 0) {
if (read_eintr(fd, &b0, sizeof(b0)) == sizeof(b0)) {
if (b0.b0_flags & B0_SAME_DIR) {
if (fnamecmp(path_tail(buf->b_ffname),
path_tail(b0.b0_fname)) != 0
|| !same_directory((char_u *)fname, buf->b_ffname)) {
expand_env(b0.b0_fname, NameBuff, MAXPATHL);
if (fnamecmp_ino(buf->b_ffname, NameBuff,
char_to_long(b0.b0_ino))) {
differ = TRUE;
}
}
} else {
expand_env(b0.b0_fname, NameBuff, MAXPATHL);
if (fnamecmp_ino(buf->b_ffname, NameBuff,
char_to_long(b0.b0_ino))) {
differ = TRUE;
}
}
}
close(fd);
}
if (differ == false && !(curbuf->b_flags & BF_RECOVERED)
&& vim_strchr(p_shm, SHM_ATTENTION) == NULL) {
int choice = 0;
process_still_running = false;
if (os_path_exists(buf->b_fname) && swapfile_unchanged(fname)) {
choice = 4;
if (p_verbose > 0) {
verb_msg(_("Found a swap file that is not useful, deleting it"));
}
}
if (choice == 0
&& swap_exists_action != SEA_NONE
&& has_autocmd(EVENT_SWAPEXISTS, (char_u *)buf_fname, buf)) {
choice = do_swapexists(buf, (char_u *)fname);
}
if (choice == 0) {
attention_message(buf, (char_u *)fname);
got_int = false;
flush_buffers(FLUSH_TYPEAHEAD);
}
if (swap_exists_action != SEA_NONE && choice == 0) {
const char *const sw_msg_1 = _("Swap file \"");
const char *const sw_msg_2 = _("\" already exists!");
const size_t fname_len = strlen(fname);
const size_t sw_msg_1_len = strlen(sw_msg_1);
const size_t sw_msg_2_len = strlen(sw_msg_2);
const size_t name_len = sw_msg_1_len + fname_len + sw_msg_2_len + 5;
char *const name = xmalloc(name_len);
memcpy(name, sw_msg_1, sw_msg_1_len + 1);
home_replace(NULL, (char_u *)fname, (char_u *)&name[sw_msg_1_len],
fname_len, true);
xstrlcat(name, sw_msg_2, name_len);
choice = do_dialog(VIM_WARNING, (char_u *)_("VIM - ATTENTION"),
(char_u *)name,
process_still_running
? (char_u *)_(
"&Open Read-Only\n&Edit anyway\n&Recover"
"\n&Quit\n&Abort") :
(char_u *)_(
"&Open Read-Only\n&Edit anyway\n&Recover"
"\n&Delete it\n&Quit\n&Abort"),
1, NULL, false);
if (process_still_running && choice >= 4) {
choice++; 
}
xfree(name);
msg_reset_scroll();
}
if (choice > 0) {
switch (choice) {
case 1:
buf->b_p_ro = TRUE;
break;
case 2:
break;
case 3:
swap_exists_action = SEA_RECOVER;
break;
case 4:
os_remove(fname);
break;
case 5:
swap_exists_action = SEA_QUIT;
break;
case 6:
swap_exists_action = SEA_QUIT;
got_int = TRUE;
break;
}
if (!os_path_exists((char_u *)fname)) {
break;
}
} else {
MSG_PUTS("\n");
if (msg_silent == 0)
need_wait_return = TRUE;
}
}
}
}
if (fname[n - 1] == 'a') { 
if (fname[n - 2] == 'a') { 
EMSG(_("E326: Too many swap files found"));
XFREE_CLEAR(fname);
break;
}
--fname[n - 2]; 
fname[n - 1] = 'z' + 1;
}
--fname[n - 1]; 
}
if (os_isdir((char_u *) dir_name)) {
*found_existing_dir = true;
} else if (!*found_existing_dir && **dirp == NUL) {
int ret;
char *failed_dir;
if ((ret = os_mkdir_recurse(dir_name, 0755, &failed_dir)) != 0) {
EMSG3(_("E303: Unable to create directory \"%s\" for swap file, "
"recovery impossible: %s"),
failed_dir, os_strerror(ret));
xfree(failed_dir);
}
}
xfree(dir_name);
return fname;
}
static int b0_magic_wrong(ZERO_BL *b0p)
{
return b0p->b0_magic_long != (long)B0_MAGIC_LONG
|| b0p->b0_magic_int != (int)B0_MAGIC_INT
|| b0p->b0_magic_short != (short)B0_MAGIC_SHORT
|| b0p->b0_magic_char != B0_MAGIC_CHAR;
}
static bool fnamecmp_ino(
char_u *fname_c, 
char_u *fname_s, 
long ino_block0
)
{
uint64_t ino_c = 0; 
uint64_t ino_s; 
char_u buf_c[MAXPATHL]; 
char_u buf_s[MAXPATHL]; 
int retval_c; 
int retval_s; 
FileInfo file_info;
if (os_fileinfo((char *)fname_c, &file_info)) {
ino_c = os_fileinfo_inode(&file_info);
}
if (os_fileinfo((char *)fname_s, &file_info)) {
ino_s = os_fileinfo_inode(&file_info);
} else {
ino_s = (uint64_t)ino_block0;
}
if (ino_c && ino_s)
return ino_c != ino_s;
retval_c = vim_FullName((char *)fname_c, (char *)buf_c, MAXPATHL, TRUE);
retval_s = vim_FullName((char *)fname_s, (char *)buf_s, MAXPATHL, TRUE);
if (retval_c == OK && retval_s == OK)
return STRCMP(buf_c, buf_s) != 0;
if (ino_s == 0 && ino_c == 0 && retval_c == FAIL && retval_s == FAIL) {
return STRCMP(fname_c, fname_s) != 0;
}
return true;
}
static void long_to_char(long n, char_u *s)
{
s[0] = (char_u)(n & 0xff);
n = (unsigned)n >> 8;
s[1] = (char_u)(n & 0xff);
n = (unsigned)n >> 8;
s[2] = (char_u)(n & 0xff);
n = (unsigned)n >> 8;
s[3] = (char_u)(n & 0xff);
}
static long char_to_long(char_u *s)
{
long retval;
retval = s[3];
retval <<= 8;
retval |= s[2];
retval <<= 8;
retval |= s[1];
retval <<= 8;
retval |= s[0];
return retval;
}
void ml_setflags(buf_T *buf)
{
bhdr_T *hp;
ZERO_BL *b0p;
if (!buf->b_ml.ml_mfp)
return;
for (hp = buf->b_ml.ml_mfp->mf_used_last; hp != NULL; hp = hp->bh_prev) {
if (hp->bh_bnum == 0) {
b0p = hp->bh_data;
b0p->b0_dirty = buf->b_changed ? B0_DIRTY : 0;
b0p->b0_flags = (b0p->b0_flags & ~B0_FF_MASK)
| (get_fileformat(buf) + 1);
add_b0_fenc(b0p, buf);
hp->bh_flags |= BH_DIRTY;
mf_sync(buf->b_ml.ml_mfp, MFS_ZERO);
break;
}
}
}
#define MLCS_MAXL 800 
#define MLCS_MINL 400 
static void ml_updatechunk(buf_T *buf, linenr_T line, long len, int updtype)
{
static buf_T *ml_upd_lastbuf = NULL;
static linenr_T ml_upd_lastline;
static linenr_T ml_upd_lastcurline;
static int ml_upd_lastcurix;
linenr_T curline = ml_upd_lastcurline;
int curix = ml_upd_lastcurix;
long size;
chunksize_T *curchnk;
int rest;
bhdr_T *hp;
DATA_BL *dp;
if (buf->b_ml.ml_usedchunks == -1 || len == 0)
return;
if (buf->b_ml.ml_chunksize == NULL) {
buf->b_ml.ml_chunksize = xmalloc(sizeof(chunksize_T) * 100);
buf->b_ml.ml_numchunks = 100;
buf->b_ml.ml_usedchunks = 1;
buf->b_ml.ml_chunksize[0].mlcs_numlines = 1;
buf->b_ml.ml_chunksize[0].mlcs_totalsize = 1;
}
if (updtype == ML_CHNK_UPDLINE && buf->b_ml.ml_line_count == 1) {
buf->b_ml.ml_usedchunks = 1;
buf->b_ml.ml_chunksize[0].mlcs_numlines = 1;
buf->b_ml.ml_chunksize[0].mlcs_totalsize =
(long)STRLEN(buf->b_ml.ml_line_ptr) + 1;
return;
}
if (buf != ml_upd_lastbuf || line != ml_upd_lastline + 1
|| updtype != ML_CHNK_ADDLINE) {
for (curline = 1, curix = 0;
curix < buf->b_ml.ml_usedchunks - 1
&& line >= curline +
buf->b_ml.ml_chunksize[curix].mlcs_numlines;
curix++) {
curline += buf->b_ml.ml_chunksize[curix].mlcs_numlines;
}
} else if (curix < buf->b_ml.ml_usedchunks - 1
&& line >= curline + buf->b_ml.ml_chunksize[curix].mlcs_numlines) {
curline += buf->b_ml.ml_chunksize[curix].mlcs_numlines;
curix++;
}
curchnk = buf->b_ml.ml_chunksize + curix;
if (updtype == ML_CHNK_DELLINE)
len = -len;
curchnk->mlcs_totalsize += len;
if (updtype == ML_CHNK_ADDLINE) {
curchnk->mlcs_numlines++;
if (buf->b_ml.ml_usedchunks + 1 >= buf->b_ml.ml_numchunks) {
buf->b_ml.ml_numchunks = buf->b_ml.ml_numchunks * 3 / 2;
buf->b_ml.ml_chunksize = (chunksize_T *)
xrealloc(buf->b_ml.ml_chunksize,
sizeof(chunksize_T) * buf->b_ml.ml_numchunks);
}
if (buf->b_ml.ml_chunksize[curix].mlcs_numlines >= MLCS_MAXL) {
int count; 
int idx;
int text_end;
int linecnt;
memmove(buf->b_ml.ml_chunksize + curix + 1,
buf->b_ml.ml_chunksize + curix,
(buf->b_ml.ml_usedchunks - curix) *
sizeof(chunksize_T));
size = 0;
linecnt = 0;
while (curline < buf->b_ml.ml_line_count
&& linecnt < MLCS_MINL) {
if ((hp = ml_find_line(buf, curline, ML_FIND)) == NULL) {
buf->b_ml.ml_usedchunks = -1;
return;
}
dp = hp->bh_data;
count = (long)(buf->b_ml.ml_locked_high) -
(long)(buf->b_ml.ml_locked_low) + 1;
idx = curline - buf->b_ml.ml_locked_low;
curline = buf->b_ml.ml_locked_high + 1;
if (idx == 0) 
text_end = dp->db_txt_end;
else
text_end = ((dp->db_index[idx - 1]) & DB_INDEX_MASK);
rest = count - idx;
if (linecnt + rest > MLCS_MINL) {
idx += MLCS_MINL - linecnt - 1;
linecnt = MLCS_MINL;
} else {
idx = count - 1;
linecnt += rest;
}
size += text_end - ((dp->db_index[idx]) & DB_INDEX_MASK);
}
buf->b_ml.ml_chunksize[curix].mlcs_numlines = linecnt;
buf->b_ml.ml_chunksize[curix + 1].mlcs_numlines -= linecnt;
buf->b_ml.ml_chunksize[curix].mlcs_totalsize = size;
buf->b_ml.ml_chunksize[curix + 1].mlcs_totalsize -= size;
buf->b_ml.ml_usedchunks++;
ml_upd_lastbuf = NULL; 
return;
} else if (buf->b_ml.ml_chunksize[curix].mlcs_numlines >= MLCS_MINL
&& curix == buf->b_ml.ml_usedchunks - 1
&& buf->b_ml.ml_line_count - line <= 1) {
curchnk = buf->b_ml.ml_chunksize + curix + 1;
buf->b_ml.ml_usedchunks++;
if (line == buf->b_ml.ml_line_count) {
curchnk->mlcs_numlines = 0;
curchnk->mlcs_totalsize = 0;
} else {
hp = ml_find_line(buf, buf->b_ml.ml_line_count, ML_FIND);
if (hp == NULL) {
buf->b_ml.ml_usedchunks = -1;
return;
}
dp = hp->bh_data;
if (dp->db_line_count == 1)
rest = dp->db_txt_end - dp->db_txt_start;
else
rest =
((dp->db_index[dp->db_line_count - 2]) & DB_INDEX_MASK)
- dp->db_txt_start;
curchnk->mlcs_totalsize = rest;
curchnk->mlcs_numlines = 1;
curchnk[-1].mlcs_totalsize -= rest;
curchnk[-1].mlcs_numlines -= 1;
}
}
} else if (updtype == ML_CHNK_DELLINE) {
curchnk->mlcs_numlines--;
ml_upd_lastbuf = NULL; 
if (curix < (buf->b_ml.ml_usedchunks - 1)
&& (curchnk->mlcs_numlines + curchnk[1].mlcs_numlines)
<= MLCS_MINL) {
curix++;
curchnk = buf->b_ml.ml_chunksize + curix;
} else if (curix == 0 && curchnk->mlcs_numlines <= 0) {
buf->b_ml.ml_usedchunks--;
memmove(buf->b_ml.ml_chunksize, buf->b_ml.ml_chunksize + 1,
buf->b_ml.ml_usedchunks * sizeof(chunksize_T));
return;
} else if (curix == 0 || (curchnk->mlcs_numlines > 10
&& (curchnk->mlcs_numlines +
curchnk[-1].mlcs_numlines)
> MLCS_MINL)) {
return;
}
curchnk[-1].mlcs_numlines += curchnk->mlcs_numlines;
curchnk[-1].mlcs_totalsize += curchnk->mlcs_totalsize;
buf->b_ml.ml_usedchunks--;
if (curix < buf->b_ml.ml_usedchunks) {
memmove(buf->b_ml.ml_chunksize + curix,
buf->b_ml.ml_chunksize + curix + 1,
(buf->b_ml.ml_usedchunks - curix) *
sizeof(chunksize_T));
}
return;
}
ml_upd_lastbuf = buf;
ml_upd_lastline = line;
ml_upd_lastcurline = curline;
ml_upd_lastcurix = curix;
}
long ml_find_line_or_offset(buf_T *buf, linenr_T lnum, long *offp, bool no_ff)
{
linenr_T curline;
int curix;
long size;
bhdr_T *hp;
DATA_BL *dp;
int count; 
int idx;
int start_idx;
int text_end;
long offset;
int len;
int ffdos = !no_ff && (get_fileformat(buf) == EOL_DOS);
int extra = 0;
ml_flush_line(curbuf);
if (buf->b_ml.ml_usedchunks == -1
|| buf->b_ml.ml_chunksize == NULL
|| lnum < 0)
return -1;
if (offp == NULL)
offset = 0;
else
offset = *offp;
if (lnum == 0 && offset <= 0)
return 1; 
curline = 1;
curix = size = 0;
while (curix < buf->b_ml.ml_usedchunks - 1
&& ((lnum != 0
&& lnum >= curline + buf->b_ml.ml_chunksize[curix].mlcs_numlines)
|| (offset != 0
&& offset > size +
buf->b_ml.ml_chunksize[curix].mlcs_totalsize
+ ffdos * buf->b_ml.ml_chunksize[curix].mlcs_numlines))) {
curline += buf->b_ml.ml_chunksize[curix].mlcs_numlines;
size += buf->b_ml.ml_chunksize[curix].mlcs_totalsize;
if (offset && ffdos)
size += buf->b_ml.ml_chunksize[curix].mlcs_numlines;
curix++;
}
while ((lnum != 0 && curline < lnum) || (offset != 0 && size < offset)) {
if (curline > buf->b_ml.ml_line_count
|| (hp = ml_find_line(buf, curline, ML_FIND)) == NULL)
return -1;
dp = hp->bh_data;
count = (long)(buf->b_ml.ml_locked_high) -
(long)(buf->b_ml.ml_locked_low) + 1;
start_idx = idx = curline - buf->b_ml.ml_locked_low;
if (idx == 0) 
text_end = dp->db_txt_end;
else
text_end = ((dp->db_index[idx - 1]) & DB_INDEX_MASK);
if (lnum != 0) {
if (curline + (count - idx) >= lnum)
idx += lnum - curline - 1;
else
idx = count - 1;
} else {
extra = 0;
while (offset >= size
+ text_end - (int)((dp->db_index[idx]) & DB_INDEX_MASK)
+ ffdos) {
if (ffdos)
size++;
if (idx == count - 1) {
extra = 1;
break;
}
idx++;
}
}
len = text_end - ((dp->db_index[idx]) & DB_INDEX_MASK);
size += len;
if (offset != 0 && size >= offset) {
if (size + ffdos == offset)
*offp = 0;
else if (idx == start_idx)
*offp = offset - size + len;
else
*offp = offset - size + len
- (text_end - ((dp->db_index[idx - 1]) & DB_INDEX_MASK));
curline += idx - start_idx + extra;
if (curline > buf->b_ml.ml_line_count)
return -1; 
return curline;
}
curline = buf->b_ml.ml_locked_high + 1;
}
if (lnum != 0) {
if (ffdos)
size += lnum - 1;
if ((!buf->b_p_fixeol || buf->b_p_bin) && !buf->b_p_eol
&& lnum > buf->b_ml.ml_line_count) {
size -= ffdos + 1;
}
}
return size;
}
void goto_byte(long cnt)
{
long boff = cnt;
linenr_T lnum;
ml_flush_line(curbuf); 
setpcmark();
if (boff) {
boff--;
}
lnum = ml_find_line_or_offset(curbuf, (linenr_T)0, &boff, false);
if (lnum < 1) { 
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
curwin->w_curswant = MAXCOL;
coladvance((colnr_T)MAXCOL);
} else {
curwin->w_cursor.lnum = lnum;
curwin->w_cursor.col = (colnr_T)boff;
curwin->w_cursor.coladd = 0;
curwin->w_set_curswant = TRUE;
}
check_cursor();
if (has_mbyte) {
mb_adjust_cursor();
}
}
int inc(pos_T *lp)
{
if (lp->col != MAXCOL) {
const char_u *const p = ml_get_pos(lp);
if (*p != NUL) { 
const int l = utfc_ptr2len(p);
lp->col += l;
return ((p[l] != NUL) ? 0 : 2);
}
}
if (lp->lnum != curbuf->b_ml.ml_line_count) { 
lp->col = 0;
lp->lnum++;
lp->coladd = 0;
return 1;
}
return -1;
}
int incl(pos_T *lp)
{
int r;
if ((r = inc(lp)) >= 1 && lp->col) {
r = inc(lp);
}
return r;
}
int dec(pos_T *lp)
{
lp->coladd = 0;
if (lp->col == MAXCOL) {
char_u *p = ml_get(lp->lnum);
lp->col = (colnr_T)STRLEN(p);
lp->col -= utf_head_off(p, p + lp->col);
return 0;
}
if (lp->col > 0) {
lp->col--;
char_u *p = ml_get(lp->lnum);
lp->col -= utf_head_off(p, p + lp->col);
return 0;
}
if (lp->lnum > 1) {
lp->lnum--;
char_u *p = ml_get(lp->lnum);
lp->col = (colnr_T)STRLEN(p);
lp->col -= utf_head_off(p, p + lp->col);
return 1;
}
return -1;
}
int decl(pos_T *lp)
{
int r;
if ((r = dec(lp)) == 1 && lp->col) {
r = dec(lp);
}
return r;
}
