




#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>

#include "nvim/vim.h"
#include "nvim/api/private/handle.h"
#include "nvim/ascii.h"
#include "nvim/fileio.h"
#include "nvim/buffer.h"
#include "nvim/change.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/diff.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/ex_cmds.h"
#include "nvim/ex_docmd.h"
#include "nvim/ex_eval.h"
#include "nvim/fold.h"
#include "nvim/func_attr.h"
#include "nvim/getchar.h"
#include "nvim/hashtab.h"
#include "nvim/iconv.h"
#include "nvim/mbyte.h"
#include "nvim/memfile.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/garray.h"
#include "nvim/move.h"
#include "nvim/normal.h"
#include "nvim/option.h"
#include "nvim/os_unix.h"
#include "nvim/path.h"
#include "nvim/quickfix.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/sha256.h"
#include "nvim/state.h"
#include "nvim/strings.h"
#include "nvim/ui.h"
#include "nvim/ui_compositor.h"
#include "nvim/types.h"
#include "nvim/undo.h"
#include "nvim/window.h"
#include "nvim/shada.h"
#include "nvim/os/os.h"
#include "nvim/os/os_defs.h"
#include "nvim/os/time.h"
#include "nvim/os/input.h"

#define BUFSIZE 8192 
#define SMBUFSIZE 256 




































typedef struct AutoCmd {
char_u *cmd; 

bool once; 
char nested; 
char last; 
sctx_T script_ctx; 
struct AutoCmd *next; 
} AutoCmd;

typedef struct AutoPat {
struct AutoPat *next; 

char_u *pat; 

regprog_T *reg_prog; 
AutoCmd *cmds; 
int group; 
int patlen; 
int buflocal_nr; 
char allow_dirs; 
char last; 
} AutoPat;




typedef struct AutoPatCmd {
AutoPat *curpat; 
AutoCmd *nextcmd; 
int group; 
char_u *fname; 
char_u *sfname; 
char_u *tail; 
event_T event; 
int arg_bufnr; 

struct AutoPatCmd *next; 
} AutoPatCmd;

#define AUGROUP_DEFAULT -1 
#define AUGROUP_ERROR -2 
#define AUGROUP_ALL -3 

#define HAS_BW_FLAGS
#define FIO_LATIN1 0x01 
#define FIO_UTF8 0x02 
#define FIO_UCS2 0x04 
#define FIO_UCS4 0x08 
#define FIO_UTF16 0x10 
#define FIO_ENDIAN_L 0x80 
#define FIO_NOCONVERT 0x2000 
#define FIO_UCSBOM 0x4000 
#define FIO_ALL -1 



#define CONV_RESTLEN 30



#define ICONV_MULT 8




struct bw_info {
int bw_fd; 
char_u *bw_buf; 
int bw_len; 
#if defined(HAS_BW_FLAGS)
int bw_flags; 
#endif
char_u bw_rest[CONV_RESTLEN]; 
int bw_restlen; 
int bw_first; 
char_u *bw_conv_buf; 
int bw_conv_buflen; 
int bw_conv_error; 
linenr_T bw_conv_error_lnum; 
linenr_T bw_start_lnum; 
#if defined(HAVE_ICONV)
iconv_t bw_iconv_fd; 
#endif
};

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "fileio.c.generated.h"
#endif

static char *e_auchangedbuf = N_(
"E812: Autocommands changed buffer or buffer name");







static bool au_did_filetype INIT(= false);

void filemess(buf_T *buf, char_u *name, char_u *s, int attr)
{
int msg_scroll_save;

if (msg_silent != 0) {
return;
}
add_quoted_fname((char *)IObuff, IOSIZE - 80, buf, (const char *)name);
xstrlcat((char *)IObuff, (const char *)s, IOSIZE);



msg_scroll_save = msg_scroll;
if (shortmess(SHM_OVERALL) && !exiting && p_verbose == 0)
msg_scroll = FALSE;
if (!msg_scroll) 
check_for_delay(FALSE);
msg_start();
msg_scroll = msg_scroll_save;
msg_scrolled_ign = TRUE;

msg_outtrans_attr(msg_may_trunc(FALSE, IObuff), attr);
msg_clr_eos();
ui_flush();
msg_scrolled_ign = FALSE;
}

static AutoPat *last_autopat[NUM_EVENTS] = {
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};


























int
readfile(
char_u *fname,
char_u *sfname,
linenr_T from,
linenr_T lines_to_skip,
linenr_T lines_to_read,
exarg_T *eap, 
int flags
)
{
int fd = 0;
int newfile = (flags & READ_NEW);
int check_readonly;
int filtering = (flags & READ_FILTER);
int read_stdin = (flags & READ_STDIN);
int read_buffer = (flags & READ_BUFFER);
int read_fifo = (flags & READ_FIFO);
int set_options = newfile || read_buffer
|| (eap != NULL && eap->read_edit);
linenr_T read_buf_lnum = 1; 
colnr_T read_buf_col = 0; 
char_u c;
linenr_T lnum = from;
char_u *ptr = NULL; 
char_u *buffer = NULL; 
char_u *new_buffer = NULL; 
char_u *line_start = NULL; 
int wasempty; 
colnr_T len;
long size = 0;
uint8_t *p = NULL;
off_T filesize = 0;
int skip_read = false;
context_sha256_T sha_ctx;
int read_undo_file = false;
linenr_T linecnt;
int error = FALSE; 
int ff_error = EOL_UNKNOWN; 
long linerest = 0; 
int perm = 0;
#if defined(UNIX)
int swap_mode = -1; 
#endif
int fileformat = 0; 
bool keep_fileformat = false;
FileInfo file_info;
int file_readonly;
linenr_T skip_count = 0;
linenr_T read_count = 0;
int msg_save = msg_scroll;
linenr_T read_no_eol_lnum = 0; 

bool file_rewind = false;
int can_retry;
linenr_T conv_error = 0; 
linenr_T illegal_byte = 0; 
bool keep_dest_enc = false; 

int bad_char_behavior = BAD_REPLACE;


char_u *tmpname = NULL; 
int fio_flags = 0;
char_u *fenc; 
bool fenc_alloced; 
char_u *fenc_next = NULL; 
bool advance_fenc = false;
long real_size = 0;
#if defined(HAVE_ICONV)
iconv_t iconv_fd = (iconv_t)-1; 
int did_iconv = false; 

#endif
int converted = FALSE; 
int notconverted = FALSE; 

char_u conv_rest[CONV_RESTLEN];
int conv_restlen = 0; 
buf_T *old_curbuf;
char_u *old_b_ffname;
char_u *old_b_fname;
int using_b_ffname;
int using_b_fname;

au_did_filetype = false; 

curbuf->b_no_eol_lnum = 0; 







if (curbuf->b_ffname == NULL
&& !filtering
&& fname != NULL
&& vim_strchr(p_cpo, CPO_FNAMER) != NULL
&& !(flags & READ_DUMMY)) {
if (set_rw_fname(fname, sfname) == FAIL)
return FAIL;
}





old_curbuf = curbuf;
old_b_ffname = curbuf->b_ffname;
old_b_fname = curbuf->b_fname;
using_b_ffname = (fname == curbuf->b_ffname)
|| (sfname == curbuf->b_ffname);
using_b_fname = (fname == curbuf->b_fname) || (sfname == curbuf->b_fname);



ex_no_reprint = TRUE;


need_fileinfo = FALSE;





if (sfname == NULL) {
sfname = fname;
}
#if defined(UNIX)
fname = sfname;
#endif





if (!filtering && !read_stdin && !read_buffer) {
pos_T pos;

pos = curbuf->b_op_start;


curbuf->b_op_start.lnum = ((from == 0) ? 1 : from);
curbuf->b_op_start.col = 0;

if (newfile) {
if (apply_autocmds_exarg(EVENT_BUFREADCMD, NULL, sfname,
FALSE, curbuf, eap))
return aborting() ? FAIL : OK;
} else if (apply_autocmds_exarg(EVENT_FILEREADCMD, sfname, sfname,
FALSE, NULL, eap))
return aborting() ? FAIL : OK;

curbuf->b_op_start = pos;
}

if ((shortmess(SHM_OVER) || curbuf->b_help) && p_verbose == 0)
msg_scroll = FALSE; 
else
msg_scroll = TRUE; 




if (fname != NULL && *fname != NUL) {
if (STRLEN(fname) >= MAXPATHL) {
filemess(curbuf, fname, (char_u *)_("Illegal file name"), 0);
msg_end();
msg_scroll = msg_save;
return FAIL;
}
}

if (!read_buffer && !read_stdin && !read_fifo) {
perm = os_getperm((const char *)fname);
#if defined(UNIX)


if (perm >= 0 && !S_ISREG(perm) 
#if defined(S_ISFIFO)
&& !S_ISFIFO(perm) 
#endif
#if defined(S_ISSOCK)
&& !S_ISSOCK(perm) 
#endif
#if defined(OPEN_CHR_FILES)
&& !(S_ISCHR(perm) && is_dev_fd_file(fname))

#endif
) {
if (S_ISDIR(perm)) {
filemess(curbuf, fname, (char_u *)_("is a directory"), 0);
} else {
filemess(curbuf, fname, (char_u *)_("is not a file"), 0);
}
msg_end();
msg_scroll = msg_save;
return S_ISDIR(perm) ? NOTDONE : FAIL;
}
#endif
}


set_file_options(set_options, eap);







check_readonly = (newfile && (curbuf->b_flags & BF_CHECK_RO));
if (check_readonly && !readonlymode)
curbuf->b_p_ro = FALSE;

if (newfile && !read_stdin && !read_buffer && !read_fifo) {

if (os_fileinfo((char *)fname, &file_info)) {
buf_store_file_info(curbuf, &file_info);
curbuf->b_mtime_read = curbuf->b_mtime;
#if defined(UNIX)











swap_mode = (file_info.stat.st_mode & 0644) | 0600;
#endif
} else {
curbuf->b_mtime = 0;
curbuf->b_mtime_read = 0;
curbuf->b_orig_size = 0;
curbuf->b_orig_mode = 0;
}



curbuf->b_flags &= ~(BF_NEW | BF_NEW_W);
}


file_readonly = false;
if (!read_buffer && !read_stdin) {
if (!newfile || readonlymode || !(perm & 0222)
|| !os_file_is_writable((char *)fname)) {
file_readonly = true;
}
fd = os_open((char *)fname, O_RDONLY, 0);
}

if (fd < 0) { 
msg_scroll = msg_save;
#if !defined(UNIX)

if (os_isdir(fname)) {
filemess(curbuf, sfname, (char_u *)_("is a directory"), 0);
curbuf->b_p_ro = true; 
} else {
#endif
if (!newfile) {
return FAIL;
}
if (perm == UV_ENOENT) { 


curbuf->b_flags |= BF_NEW;




if (!bt_dontwrite(curbuf)) {
check_need_swap(newfile);

if (curbuf != old_curbuf
|| (using_b_ffname
&& (old_b_ffname != curbuf->b_ffname))
|| (using_b_fname
&& (old_b_fname != curbuf->b_fname))) {
EMSG(_(e_auchangedbuf));
return FAIL;
}
}
if (dir_of_file_exists(fname))
filemess(curbuf, sfname, (char_u *)_("[New File]"), 0);
else
filemess(curbuf, sfname,
(char_u *)_("[New DIRECTORY]"), 0);


check_marks_read();

if (eap != NULL)
set_forced_fenc(eap);
apply_autocmds_exarg(EVENT_BUFNEWFILE, sfname, sfname,
FALSE, curbuf, eap);

save_file_ff(curbuf);

if (aborting()) 
return FAIL;
return OK; 
} else {
filemess(curbuf, sfname, (char_u *)(
(fd == UV_EFBIG) ? _("[File too big]") :
#if defined(UNIX) && defined(EOVERFLOW)


(fd == -EOVERFLOW) ? _("[File too big]") :
#endif
_("[Permission Denied]")), 0);
curbuf->b_p_ro = TRUE; 
}

return FAIL;
}
#if !defined(UNIX)
}
#endif





if ((check_readonly && file_readonly) || curbuf->b_help)
curbuf->b_p_ro = TRUE;

if (set_options) {


if (!read_buffer) {
curbuf->b_p_eol = TRUE;
curbuf->b_start_eol = TRUE;
}
curbuf->b_p_bomb = FALSE;
curbuf->b_start_bomb = FALSE;
}




if (!bt_dontwrite(curbuf)) {
check_need_swap(newfile);
if (!read_stdin
&& (curbuf != old_curbuf
|| (using_b_ffname && (old_b_ffname != curbuf->b_ffname))
|| (using_b_fname && (old_b_fname != curbuf->b_fname)))) {
EMSG(_(e_auchangedbuf));
if (!read_buffer) {
close(fd);
}
return FAIL;
}
#if defined(UNIX)

if (swap_mode > 0 && curbuf->b_ml.ml_mfp != NULL
&& curbuf->b_ml.ml_mfp->mf_fname != NULL) {
const char *swap_fname = (const char *)curbuf->b_ml.ml_mfp->mf_fname;






if ((swap_mode & 044) == 040) {
FileInfo swap_info;

if (os_fileinfo(swap_fname, &swap_info)
&& file_info.stat.st_gid != swap_info.stat.st_gid
&& os_fchown(curbuf->b_ml.ml_mfp->mf_fd, -1, file_info.stat.st_gid)
== -1) {
swap_mode &= 0600;
}
}

(void)os_setperm(swap_fname, swap_mode);
}
#endif
}


if (swap_exists_action == SEA_QUIT) {
if (!read_buffer && !read_stdin)
close(fd);
return FAIL;
}

++no_wait_return; 




curbuf->b_op_start.lnum = ((from == 0) ? 1 : from);
curbuf->b_op_start.col = 0;

int try_mac = (vim_strchr(p_ffs, 'm') != NULL);
int try_dos = (vim_strchr(p_ffs, 'd') != NULL);
int try_unix = (vim_strchr(p_ffs, 'x') != NULL);

if (!read_buffer) {
int m = msg_scroll;
int n = msg_scrolled;



if (!read_stdin) {
close(fd); 
}




msg_scroll = true;
if (filtering) {
apply_autocmds_exarg(EVENT_FILTERREADPRE, NULL, sfname,
false, curbuf, eap);
} else if (read_stdin) {
apply_autocmds_exarg(EVENT_STDINREADPRE, NULL, sfname,
false, curbuf, eap);
} else if (newfile) {
apply_autocmds_exarg(EVENT_BUFREADPRE, NULL, sfname,
false, curbuf, eap);
} else {
apply_autocmds_exarg(EVENT_FILEREADPRE, sfname, sfname,
false, NULL, eap);
}


try_mac = (vim_strchr(p_ffs, 'm') != NULL);
try_dos = (vim_strchr(p_ffs, 'd') != NULL);
try_unix = (vim_strchr(p_ffs, 'x') != NULL);

if (msg_scrolled == n) {
msg_scroll = m;
}

if (aborting()) { 
--no_wait_return;
msg_scroll = msg_save;
curbuf->b_p_ro = TRUE; 
return FAIL;
}







if (!read_stdin && (curbuf != old_curbuf
|| (using_b_ffname && (old_b_ffname != curbuf->b_ffname))
|| (using_b_fname && (old_b_fname != curbuf->b_fname))
|| (fd = os_open((char *)fname, O_RDONLY, 0)) < 0)) {
--no_wait_return;
msg_scroll = msg_save;
if (fd < 0)
EMSG(_("E200: *ReadPre autocommands made the file unreadable"));
else
EMSG(_("E201: *ReadPre autocommands must not change current buffer"));
curbuf->b_p_ro = TRUE; 
return FAIL;
}
}


wasempty = (curbuf->b_ml.ml_flags & ML_EMPTY);

if (!recoverymode && !filtering && !(flags & READ_DUMMY)) {
if (!read_stdin && !read_buffer) {
filemess(curbuf, sfname, (char_u *)"", 0);
}
}

msg_scroll = FALSE; 





linecnt = curbuf->b_ml.ml_line_count;


if (eap != NULL && eap->bad_char != 0) {
bad_char_behavior = eap->bad_char;
if (set_options)
curbuf->b_bad_char = eap->bad_char;
} else
curbuf->b_bad_char = 0;




if (eap != NULL && eap->force_enc != 0) {
fenc = enc_canonize(eap->cmd + eap->force_enc);
fenc_alloced = true;
keep_dest_enc = true;
} else if (curbuf->b_p_bin) {
fenc = (char_u *)""; 
fenc_alloced = false;
} else if (curbuf->b_help) {




fenc_next = (char_u *)"latin1";
fenc = (char_u *)"utf-8";

fenc_alloced = false;
} else if (*p_fencs == NUL) {
fenc = curbuf->b_p_fenc; 
fenc_alloced = false;
} else {
fenc_next = p_fencs; 
fenc = next_fenc(&fenc_next, &fenc_alloced);
}




















retry:

if (file_rewind) {
if (read_buffer) {
read_buf_lnum = 1;
read_buf_col = 0;
} else if (read_stdin || vim_lseek(fd, (off_T)0L, SEEK_SET) != 0) {

error = true;
goto failed;
}

while (lnum > from) {
ml_delete(lnum--, false);
}
file_rewind = false;
if (set_options) {
curbuf->b_p_bomb = FALSE;
curbuf->b_start_bomb = FALSE;
}
conv_error = 0;
}





if (keep_fileformat) {
keep_fileformat = false;
} else {
if (eap != NULL && eap->force_ff != 0) {
fileformat = get_fileformat_force(curbuf, eap);
try_unix = try_dos = try_mac = FALSE;
} else if (curbuf->b_p_bin)
fileformat = EOL_UNIX; 
else if (*p_ffs == NUL)
fileformat = get_fileformat(curbuf); 
else
fileformat = EOL_UNKNOWN; 
}

#if defined(HAVE_ICONV)
if (iconv_fd != (iconv_t)-1) {

iconv_close(iconv_fd);
iconv_fd = (iconv_t)-1;
}
#endif

if (advance_fenc) {



advance_fenc = false;

if (eap != NULL && eap->force_enc != 0) {


notconverted = TRUE;
conv_error = 0;
if (fenc_alloced)
xfree(fenc);
fenc = (char_u *)"";
fenc_alloced = false;
} else {
if (fenc_alloced)
xfree(fenc);
if (fenc_next != NULL) {
fenc = next_fenc(&fenc_next, &fenc_alloced);
} else {
fenc = (char_u *)"";
fenc_alloced = false;
}
}
if (tmpname != NULL) {
os_remove((char *)tmpname); 
XFREE_CLEAR(tmpname);
}
}





fio_flags = 0;
converted = need_conversion(fenc);
if (converted) {



if (STRCMP(fenc, ENC_UCSBOM) == 0)
fio_flags = FIO_UCSBOM;










else if (enc_utf8 || STRCMP(p_enc, "latin1") == 0)
fio_flags = get_fio_flags(fenc);



#if defined(HAVE_ICONV)

if (fio_flags == 0
&& !did_iconv
) {
iconv_fd = (iconv_t)my_iconv_open(
enc_utf8 ? (char_u *)"utf-8" : p_enc, fenc);
}
#endif





if (fio_flags == 0 && !read_stdin && !read_buffer && *p_ccv != NUL
&& !read_fifo
#if defined(HAVE_ICONV)
&& iconv_fd == (iconv_t)-1
#endif
) {
#if defined(HAVE_ICONV)
did_iconv = false;
#endif


if (tmpname == NULL) {
tmpname = readfile_charconvert(fname, fenc, &fd);
if (tmpname == NULL) {

advance_fenc = true;
if (fd < 0) {

EMSG(_("E202: Conversion made file unreadable!"));
error = TRUE;
goto failed;
}
goto retry;
}
}
} else {
if (fio_flags == 0
#if defined(HAVE_ICONV)
&& iconv_fd == (iconv_t)-1
#endif
) {


advance_fenc = true;
goto retry;
}
}
}




can_retry = (*fenc != NUL && !read_stdin && !keep_dest_enc && !read_fifo);

if (!skip_read) {
linerest = 0;
filesize = 0;
skip_count = lines_to_skip;
read_count = lines_to_read;
conv_restlen = 0;
read_undo_file = (newfile && (flags & READ_KEEP_UNDO) == 0
&& curbuf->b_ffname != NULL
&& curbuf->b_p_udf
&& !filtering
&& !read_fifo
&& !read_stdin
&& !read_buffer);
if (read_undo_file)
sha256_start(&sha_ctx);
}

while (!error && !got_int) {






{
if (!skip_read) {
size = 0x10000L; 

for (; size >= 10; size /= 2) {
new_buffer = verbose_try_malloc((size_t)size + (size_t)linerest + 1);
if (new_buffer) {
break;
}
}
if (new_buffer == NULL) {
error = TRUE;
break;
}
if (linerest) 
memmove(new_buffer, ptr - linerest, (size_t)linerest);
xfree(buffer);
buffer = new_buffer;
ptr = buffer + linerest;
line_start = buffer;











real_size = (int)size;
#if defined(HAVE_ICONV)
if (iconv_fd != (iconv_t)-1) {
size = size / ICONV_MULT;
} else {
#endif
if (fio_flags & FIO_LATIN1) {
size = size / 2;
} else if (fio_flags & (FIO_UCS2 | FIO_UTF16)) {
size = (size * 2 / 3) & ~1;
} else if (fio_flags & FIO_UCS4) {
size = (size * 2 / 3) & ~3;
} else if (fio_flags == FIO_UCSBOM) {
size = size / ICONV_MULT; 
}
#if defined(HAVE_ICONV)
}
#endif
if (conv_restlen > 0) {

memmove(ptr, conv_rest, conv_restlen); 
ptr += conv_restlen;
size -= conv_restlen;
}

if (read_buffer) {




if (read_buf_lnum > from)
size = 0;
else {
int n, ni;
long tlen;

tlen = 0;
for (;; ) {
p = ml_get(read_buf_lnum) + read_buf_col;
n = (int)STRLEN(p);
if ((int)tlen + n + 1 > size) {



n = (int)(size - tlen);
for (ni = 0; ni < n; ++ni) {
if (p[ni] == NL)
ptr[tlen++] = NUL;
else
ptr[tlen++] = p[ni];
}
read_buf_col += n;
break;
} else {


for (ni = 0; ni < n; ++ni) {
if (p[ni] == NL)
ptr[tlen++] = NUL;
else
ptr[tlen++] = p[ni];
}
ptr[tlen++] = NL;
read_buf_col = 0;
if (++read_buf_lnum > from) {


if (!curbuf->b_p_eol)
--tlen;
size = tlen;
break;
}
}
}
}
} else {



size = read_eintr(fd, ptr, size);
}

if (size <= 0) {
if (size < 0) 
error = TRUE;
else if (conv_restlen > 0) {






if (fio_flags != 0
#if defined(HAVE_ICONV)
|| iconv_fd != (iconv_t)-1
#endif
) {
if (can_retry)
goto rewind_retry;
if (conv_error == 0)
conv_error = curbuf->b_ml.ml_line_count
- linecnt + 1;
}

else if (illegal_byte == 0)
illegal_byte = curbuf->b_ml.ml_line_count
- linecnt + 1;
if (bad_char_behavior == BAD_DROP) {
*(ptr - conv_restlen) = NUL;
conv_restlen = 0;
} else {




if (bad_char_behavior != BAD_KEEP && (fio_flags != 0
#if defined(HAVE_ICONV)
|| iconv_fd != (iconv_t)-1
#endif
)) {
while (conv_restlen > 0) {
*(--ptr) = bad_char_behavior;
--conv_restlen;
}
}
fio_flags = 0; 
#if defined(HAVE_ICONV)
if (iconv_fd != (iconv_t)-1) {
iconv_close(iconv_fd);
iconv_fd = (iconv_t)-1;
}
#endif
}
}
}
}

skip_read = FALSE;







if ((filesize == 0)
&& (fio_flags == FIO_UCSBOM
|| (!curbuf->b_p_bomb
&& tmpname == NULL
&& (*fenc == 'u' || (*fenc == NUL && enc_utf8))))) {
char_u *ccname;
int blen;


if (size < 2 || curbuf->b_p_bin)
ccname = NULL;
else
ccname = check_for_bom(ptr, size, &blen,
fio_flags == FIO_UCSBOM ? FIO_ALL : get_fio_flags(fenc));
if (ccname != NULL) {

filesize += blen;
size -= blen;
memmove(ptr, ptr + blen, (size_t)size);
if (set_options) {
curbuf->b_p_bomb = TRUE;
curbuf->b_start_bomb = TRUE;
}
}

if (fio_flags == FIO_UCSBOM) {
if (ccname == NULL) {

advance_fenc = true;
} else {

if (fenc_alloced)
xfree(fenc);
fenc = ccname;
fenc_alloced = false;
}

skip_read = TRUE;
goto retry;
}
}


ptr -= conv_restlen;
size += conv_restlen;
conv_restlen = 0;



if (size <= 0)
break;

#if defined(HAVE_ICONV)
if (iconv_fd != (iconv_t)-1) {




const char *fromp;
char *top;
size_t from_size;
size_t to_size;

fromp = (char *)ptr;
from_size = size;
ptr += size;
top = (char *)ptr;
to_size = real_size - size;






while ((iconv(iconv_fd, (void *)&fromp, &from_size,
&top, &to_size)
== (size_t)-1 && ICONV_ERRNO != ICONV_EINVAL)
|| from_size > CONV_RESTLEN) {
if (can_retry)
goto rewind_retry;
if (conv_error == 0)
conv_error = readfile_linenr(linecnt,
ptr, (char_u *)top);


++fromp;
--from_size;
if (bad_char_behavior == BAD_KEEP) {
*top++ = *(fromp - 1);
--to_size;
} else if (bad_char_behavior != BAD_DROP) {
*top++ = bad_char_behavior;
--to_size;
}
}

if (from_size > 0) {


memmove(conv_rest, (char_u *)fromp, from_size);
conv_restlen = (int)from_size;
}


line_start = ptr - linerest;
memmove(line_start, buffer, (size_t)linerest);
size = (long)((char_u *)top - ptr);
}
#endif

if (fio_flags != 0) {
unsigned int u8c;
char_u *dest;
char_u *tail = NULL;









dest = ptr + real_size;
if (fio_flags == FIO_LATIN1 || fio_flags == FIO_UTF8) {
p = ptr + size;
if (fio_flags == FIO_UTF8) {

tail = ptr + size - 1;
while (tail > ptr && (*tail & 0xc0) == 0x80)
--tail;
if (tail + utf_byte2len(*tail) <= ptr + size)
tail = NULL;
else
p = tail;
}
} else if (fio_flags & (FIO_UCS2 | FIO_UTF16)) {

p = ptr + (size & ~1);
if (size & 1)
tail = p;
if ((fio_flags & FIO_UTF16) && p > ptr) {

if (fio_flags & FIO_ENDIAN_L) {
u8c = (*--p << 8);
u8c += *--p;
} else {
u8c = *--p;
u8c += (*--p << 8);
}
if (u8c >= 0xd800 && u8c <= 0xdbff)
tail = p;
else
p += 2;
}
} else { 

p = ptr + (size & ~3);
if (size & 3)
tail = p;
}



if (tail != NULL) {
conv_restlen = (int)((ptr + size) - tail);
memmove(conv_rest, tail, conv_restlen);
size -= conv_restlen;
}


while (p > ptr) {
if (fio_flags & FIO_LATIN1)
u8c = *--p;
else if (fio_flags & (FIO_UCS2 | FIO_UTF16)) {
if (fio_flags & FIO_ENDIAN_L) {
u8c = (*--p << 8);
u8c += *--p;
} else {
u8c = *--p;
u8c += (*--p << 8);
}
if ((fio_flags & FIO_UTF16)
&& u8c >= 0xdc00 && u8c <= 0xdfff) {
int u16c;

if (p == ptr) {

if (can_retry)
goto rewind_retry;
if (conv_error == 0)
conv_error = readfile_linenr(linecnt,
ptr, p);
if (bad_char_behavior == BAD_DROP)
continue;
if (bad_char_behavior != BAD_KEEP)
u8c = bad_char_behavior;
}



if (fio_flags & FIO_ENDIAN_L) {
u16c = (*--p << 8);
u16c += *--p;
} else {
u16c = *--p;
u16c += (*--p << 8);
}
u8c = 0x10000 + ((u16c & 0x3ff) << 10)
+ (u8c & 0x3ff);


if (u16c < 0xd800 || u16c > 0xdbff) {
if (can_retry)
goto rewind_retry;
if (conv_error == 0)
conv_error = readfile_linenr(linecnt,
ptr, p);
if (bad_char_behavior == BAD_DROP)
continue;
if (bad_char_behavior != BAD_KEEP)
u8c = bad_char_behavior;
}
}
} else if (fio_flags & FIO_UCS4) {
if (fio_flags & FIO_ENDIAN_L) {
u8c = (unsigned)(*--p) << 24;
u8c += (unsigned)(*--p) << 16;
u8c += (unsigned)(*--p) << 8;
u8c += *--p;
} else { 
u8c = *--p;
u8c += (unsigned)(*--p) << 8;
u8c += (unsigned)(*--p) << 16;
u8c += (unsigned)(*--p) << 24;
}
} else { 
if (*--p < 0x80)
u8c = *p;
else {
len = utf_head_off(ptr, p);
p -= len;
u8c = utf_ptr2char(p);
if (len == 0) {



if (can_retry)
goto rewind_retry;
if (conv_error == 0)
conv_error = readfile_linenr(linecnt,
ptr, p);
if (bad_char_behavior == BAD_DROP)
continue;
if (bad_char_behavior != BAD_KEEP)
u8c = bad_char_behavior;
}
}
}
assert(u8c <= INT_MAX);

dest -= utf_char2len((int)u8c);
(void)utf_char2bytes((int)u8c, dest);
}


line_start = dest - linerest;
memmove(line_start, buffer, (size_t)linerest);
size = (long)((ptr + real_size) - dest);
ptr = dest;
} else if (enc_utf8 && !curbuf->b_p_bin) {
int incomplete_tail = FALSE;


for (p = ptr;; p++) {
int todo = (int)((ptr + size) - p);
int l;

if (todo <= 0) {
break;
}
if (*p >= 0x80) {




l = utf_ptr2len_len(p, todo);
if (l > todo && !incomplete_tail) {




if (p > ptr || filesize > 0)
incomplete_tail = TRUE;



if (p > ptr) {
conv_restlen = todo;
memmove(conv_rest, p, conv_restlen);
size -= conv_restlen;
break;
}
}
if (l == 1 || l > todo) {



if (can_retry && !incomplete_tail)
break;
#if defined(HAVE_ICONV)

if (iconv_fd != (iconv_t)-1 && conv_error == 0) {
conv_error = readfile_linenr(linecnt, ptr, p);
}
#endif

if (conv_error == 0 && illegal_byte == 0)
illegal_byte = readfile_linenr(linecnt, ptr, p);


if (bad_char_behavior == BAD_DROP) {
memmove(p, p + 1, todo - 1);
--p;
--size;
} else if (bad_char_behavior != BAD_KEEP)
*p = bad_char_behavior;
} else
p += l - 1;
}
}
if (p < ptr + size && !incomplete_tail) {

rewind_retry:

#if defined(HAVE_ICONV)
if (*p_ccv != NUL && iconv_fd != (iconv_t)-1) {

did_iconv = true;
} else {
#endif

advance_fenc = true;
#if defined(HAVE_ICONV)
}
#endif
file_rewind = true;
goto retry;
}
}


filesize += size;




if (fileformat == EOL_UNKNOWN) {

if (try_dos || try_unix) {

if (try_mac) {
try_mac = 1;
}

for (p = ptr; p < ptr + size; ++p) {
if (*p == NL) {
if (!try_unix
|| (try_dos && p > ptr && p[-1] == CAR))
fileformat = EOL_DOS;
else
fileformat = EOL_UNIX;
break;
} else if (*p == CAR && try_mac) {
try_mac++;
}
}


if (fileformat == EOL_UNIX && try_mac) {

try_mac = 1;
try_unix = 1;
for (; p >= ptr && *p != CAR; p--)
;
if (p >= ptr) {
for (p = ptr; p < ptr + size; ++p) {
if (*p == NL)
try_unix++;
else if (*p == CAR)
try_mac++;
}
if (try_mac > try_unix)
fileformat = EOL_MAC;
}
} else if (fileformat == EOL_UNKNOWN && try_mac == 1) {


fileformat = default_fileformat();
}
}


if (fileformat == EOL_UNKNOWN && try_mac)
fileformat = EOL_MAC;


if (fileformat == EOL_UNKNOWN)
fileformat = default_fileformat();


if (set_options) {
set_fileformat(fileformat, OPT_LOCAL);
}
}
}





if (fileformat == EOL_MAC) {
--ptr;
while (++ptr, --size >= 0) {

if ((c = *ptr) != NUL && c != CAR && c != NL)
continue;
if (c == NUL)
*ptr = NL; 
else if (c == NL)
*ptr = CAR; 
else {
if (skip_count == 0) {
*ptr = NUL; 
len = (colnr_T) (ptr - line_start + 1);
if (ml_append(lnum, line_start, len, newfile) == FAIL) {
error = TRUE;
break;
}
if (read_undo_file)
sha256_update(&sha_ctx, line_start, len);
++lnum;
if (--read_count == 0) {
error = TRUE; 
line_start = ptr; 
break;
}
} else
--skip_count;
line_start = ptr + 1;
}
}
} else {
--ptr;
while (++ptr, --size >= 0) {
if ((c = *ptr) != NUL && c != NL) 
continue;
if (c == NUL)
*ptr = NL; 
else {
if (skip_count == 0) {
*ptr = NUL; 
len = (colnr_T)(ptr - line_start + 1);
if (fileformat == EOL_DOS) {
if (ptr > line_start && ptr[-1] == CAR) {

ptr[-1] = NUL;
len--;
} else if (ff_error != EOL_DOS) {




if (try_unix
&& !read_stdin
&& (read_buffer
|| vim_lseek(fd, (off_T)0L, SEEK_SET) == 0)) {
fileformat = EOL_UNIX;
if (set_options)
set_fileformat(EOL_UNIX, OPT_LOCAL);
file_rewind = true;
keep_fileformat = true;
goto retry;
}
ff_error = EOL_DOS;
}
}
if (ml_append(lnum, line_start, len, newfile) == FAIL) {
error = TRUE;
break;
}
if (read_undo_file)
sha256_update(&sha_ctx, line_start, len);
++lnum;
if (--read_count == 0) {
error = TRUE; 
line_start = ptr; 
break;
}
} else
--skip_count;
line_start = ptr + 1;
}
}
}
linerest = (long)(ptr - line_start);
os_breakcheck();
}

failed:

if (error && read_count == 0)
error = FALSE;






if (!error
&& !got_int
&& linerest != 0
&& !(!curbuf->b_p_bin
&& fileformat == EOL_DOS
&& *line_start == Ctrl_Z
&& ptr == line_start + 1)) {

if (set_options)
curbuf->b_p_eol = FALSE;
*ptr = NUL;
len = (colnr_T)(ptr - line_start + 1);
if (ml_append(lnum, line_start, len, newfile) == FAIL)
error = TRUE;
else {
if (read_undo_file)
sha256_update(&sha_ctx, line_start, len);
read_no_eol_lnum = ++lnum;
}
}

if (set_options) {

save_file_ff(curbuf);


set_string_option_direct((char_u *)"fenc", -1, fenc,
OPT_FREE | OPT_LOCAL, 0);
}
if (fenc_alloced)
xfree(fenc);
#if defined(HAVE_ICONV)
if (iconv_fd != (iconv_t)-1) {
iconv_close(iconv_fd);
#if !defined(__clang_analyzer__)
iconv_fd = (iconv_t)-1;
#endif
}
#endif

if (!read_buffer && !read_stdin) {
close(fd); 
} else {
(void)os_set_cloexec(fd);
}
xfree(buffer);

if (read_stdin) {
close(0);
#if !defined(WIN32)

vim_ignored = dup(2);
#else

HANDLE conin = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE,
FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL,
OPEN_EXISTING, 0, (HANDLE)NULL);
vim_ignored = _open_osfhandle(conin, _O_RDONLY);
#endif
}

if (tmpname != NULL) {
os_remove((char *)tmpname); 
xfree(tmpname);
}
--no_wait_return; 




if (!recoverymode) {

if (newfile && wasempty && !(curbuf->b_ml.ml_flags & ML_EMPTY)) {
ml_delete(curbuf->b_ml.ml_line_count, false);
linecnt--;
}
curbuf->deleted_bytes = 0;
curbuf->deleted_codepoints = 0;
curbuf->deleted_codeunits = 0;
linecnt = curbuf->b_ml.ml_line_count - linecnt;
if (filesize == 0)
linecnt = 0;
if (newfile || read_buffer) {
redraw_curbuf_later(NOT_VALID);


diff_invalidate(curbuf);


foldUpdateAll(curwin);
} else if (linecnt) 
appended_lines_mark(from, linecnt);






if (read_stdin) {
screenclear();
}

if (got_int) {
if (!(flags & READ_DUMMY)) {
filemess(curbuf, sfname, (char_u *)_(e_interr), 0);
if (newfile)
curbuf->b_p_ro = TRUE; 
}
msg_scroll = msg_save;
check_marks_read();
return OK; 
}

if (!filtering && !(flags & READ_DUMMY)) {
add_quoted_fname((char *)IObuff, IOSIZE, curbuf, (const char *)sfname);
c = false;

#if defined(UNIX)
#if defined(S_ISFIFO)
if (S_ISFIFO(perm)) { 
STRCAT(IObuff, _("[fifo/socket]"));
c = TRUE;
}
#else
#if defined(S_IFIFO)
if ((perm & S_IFMT) == S_IFIFO) { 
STRCAT(IObuff, _("[fifo]"));
c = TRUE;
}
#endif
#if defined(S_IFSOCK)
if ((perm & S_IFMT) == S_IFSOCK) { 
STRCAT(IObuff, _("[socket]"));
c = TRUE;
}
#endif
#endif
#if defined(OPEN_CHR_FILES)
if (S_ISCHR(perm)) { 
STRCAT(IObuff, _("[character special]"));
c = TRUE;
}
#endif
#endif
if (curbuf->b_p_ro) {
STRCAT(IObuff, shortmess(SHM_RO) ? _("[RO]") : _("[readonly]"));
c = TRUE;
}
if (read_no_eol_lnum) {
msg_add_eol();
c = TRUE;
}
if (ff_error == EOL_DOS) {
STRCAT(IObuff, _("[CR missing]"));
c = TRUE;
}
if (notconverted) {
STRCAT(IObuff, _("[NOT converted]"));
c = TRUE;
} else if (converted) {
STRCAT(IObuff, _("[converted]"));
c = TRUE;
}
if (conv_error != 0) {
sprintf((char *)IObuff + STRLEN(IObuff),
_("[CONVERSION ERROR in line %" PRId64 "]"), (int64_t)conv_error);
c = TRUE;
} else if (illegal_byte > 0) {
sprintf((char *)IObuff + STRLEN(IObuff),
_("[ILLEGAL BYTE in line %" PRId64 "]"), (int64_t)illegal_byte);
c = TRUE;
} else if (error) {
STRCAT(IObuff, _("[READ ERRORS]"));
c = TRUE;
}
if (msg_add_fileformat(fileformat))
c = TRUE;

msg_add_lines(c, (long)linecnt, filesize);

XFREE_CLEAR(keep_msg);
p = NULL;
msg_scrolled_ign = TRUE;

if (!read_stdin && !read_buffer) {
p = msg_trunc_attr(IObuff, FALSE, 0);
}

if (read_stdin || read_buffer || restart_edit != 0
|| (msg_scrolled != 0 && !need_wait_return)) {





set_keep_msg(p, 0);
}
msg_scrolled_ign = FALSE;
}


if (newfile && (error
|| conv_error != 0
|| (illegal_byte > 0 && bad_char_behavior != BAD_KEEP)
))
curbuf->b_p_ro = TRUE;

u_clearline(); 





if (exmode_active)
curwin->w_cursor.lnum = from + linecnt;
else
curwin->w_cursor.lnum = from + 1;
check_cursor_lnum();
beginline(BL_WHITE | BL_FIX); 




curbuf->b_op_start.lnum = from + 1;
curbuf->b_op_start.col = 0;
curbuf->b_op_end.lnum = from + linecnt;
curbuf->b_op_end.col = 0;

}
msg_scroll = msg_save;




check_marks_read();







curbuf->b_no_eol_lnum = read_no_eol_lnum;



if (flags & READ_KEEP_UNDO)
u_find_first_changed();




if (read_undo_file) {
char_u hash[UNDO_HASH_SIZE];

sha256_finish(&sha_ctx, hash);
u_read_undo(NULL, hash, fname);
}

if (!read_stdin && !read_fifo && (!read_buffer || sfname != NULL)) {
int m = msg_scroll;
int n = msg_scrolled;



if (set_options)
save_file_ff(curbuf);






msg_scroll = true;
if (filtering) {
apply_autocmds_exarg(EVENT_FILTERREADPOST, NULL, sfname,
false, curbuf, eap);
} else if (newfile || (read_buffer && sfname != NULL)) {
apply_autocmds_exarg(EVENT_BUFREADPOST, NULL, sfname,
false, curbuf, eap);
if (!au_did_filetype && *curbuf->b_p_ft != NUL) {


apply_autocmds(EVENT_FILETYPE, curbuf->b_p_ft, curbuf->b_fname,
true, curbuf);
}
} else {
apply_autocmds_exarg(EVENT_FILEREADPOST, sfname, sfname,
false, NULL, eap);
}
if (msg_scrolled == n) {
msg_scroll = m;
}
if (aborting()) { 
return FAIL;
}
}

if (recoverymode && error)
return FAIL;
return OK;
}

#if defined(OPEN_CHR_FILES)






bool is_dev_fd_file(char_u *fname)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
return STRNCMP(fname, "/dev/fd/", 8) == 0
&& ascii_isdigit(fname[8])
&& *skipdigits(fname + 9) == NUL
&& (fname[9] != NUL
|| (fname[8] != '0' && fname[8] != '1' && fname[8] != '2'));
}
#endif







static linenr_T
readfile_linenr(
linenr_T linecnt, 
char_u *p, 
char_u *endp 
)
{
char_u *s;
linenr_T lnum;

lnum = curbuf->b_ml.ml_line_count - linecnt + 1;
for (s = p; s < endp; ++s)
if (*s == '\n')
++lnum;
return lnum;
}





void prep_exarg(exarg_T *eap, const buf_T *buf)
FUNC_ATTR_NONNULL_ALL
{
const size_t cmd_len = 15 + STRLEN(buf->b_p_fenc);
eap->cmd = xmalloc(cmd_len);

snprintf((char *)eap->cmd, cmd_len, "e ++enc=%s", buf->b_p_fenc);
eap->force_enc = 8;
eap->bad_char = buf->b_bad_char;
eap->force_ff = *buf->b_p_ff;

eap->force_bin = buf->b_p_bin ? FORCE_BIN : FORCE_NOBIN;
eap->read_edit = FALSE;
eap->forceit = FALSE;
}




void set_file_options(int set_options, exarg_T *eap)
{

if (set_options) {
if (eap != NULL && eap->force_ff != 0)
set_fileformat(get_fileformat_force(curbuf, eap), OPT_LOCAL);
else if (*p_ffs != NUL)
set_fileformat(default_fileformat(), OPT_LOCAL);
}


if (eap != NULL && eap->force_bin != 0) {
int oldval = curbuf->b_p_bin;

curbuf->b_p_bin = (eap->force_bin == FORCE_BIN);
set_options_bin(oldval, curbuf->b_p_bin, OPT_LOCAL);
}
}




void set_forced_fenc(exarg_T *eap)
{
if (eap->force_enc != 0) {
char_u *fenc = enc_canonize(eap->cmd + eap->force_enc);
set_string_option_direct((char_u *)"fenc", -1, fenc, OPT_FREE|OPT_LOCAL, 0);
xfree(fenc);
}
}







static char_u *next_fenc(char_u **pp, bool *alloced)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_NONNULL_RET
{
char_u *p;
char_u *r;

*alloced = false;
if (**pp == NUL) {
*pp = NULL;
return (char_u *)"";
}
p = vim_strchr(*pp, ',');
if (p == NULL) {
r = enc_canonize(*pp);
*pp += STRLEN(*pp);
} else {
r = vim_strnsave(*pp, (int)(p - *pp));
*pp = p + 1;
p = enc_canonize(r);
xfree(r);
r = p;
}
*alloced = true;
return r;
}









static char_u *
readfile_charconvert (
char_u *fname, 
char_u *fenc, 
int *fdp 
)
{
char_u *tmpname;
char_u *errmsg = NULL;

tmpname = vim_tempname();
if (tmpname == NULL)
errmsg = (char_u *)_("Can't find temp file for conversion");
else {
close(*fdp); 
*fdp = -1;
if (eval_charconvert((char *) fenc, enc_utf8 ? "utf-8" : (char *) p_enc,
(char *) fname, (char *) tmpname) == FAIL) {
errmsg = (char_u *)_("Conversion with 'charconvert' failed");
}
if (errmsg == NULL && (*fdp = os_open((char *)tmpname, O_RDONLY, 0)) < 0) {
errmsg = (char_u *)_("can't read output of 'charconvert'");
}
}

if (errmsg != NULL) {


MSG(errmsg);
if (tmpname != NULL) {
os_remove((char *)tmpname); 
XFREE_CLEAR(tmpname);
}
}


if (*fdp < 0) {
*fdp = os_open((char *)fname, O_RDONLY, 0);
}

return tmpname;
}






static void check_marks_read(void)
{
if (!curbuf->b_marks_read && get_shada_parameter('\'') > 0
&& curbuf->b_ffname != NULL) {
shada_read_marks();
}



curbuf->b_marks_read = true;
}

















int
buf_write(
buf_T *buf,
char_u *fname,
char_u *sfname,
linenr_T start,
linenr_T end,
exarg_T *eap, 

int append, 
int forceit,
int reset_changed,
int filtering
)
{
int fd;
char_u *backup = NULL;
int backup_copy = FALSE; 
int dobackup;
char_u *ffname;
char_u *wfname = NULL; 
char_u *s;
char_u *ptr;
char_u c;
int len;
linenr_T lnum;
long nchars;
#define SET_ERRMSG_NUM(num, msg) errnum = num, errmsg = msg, errmsgarg = 0

#define SET_ERRMSG_ARG(msg, error) errnum = NULL, errmsg = msg, errmsgarg = error

#define SET_ERRMSG(msg) errnum = NULL, errmsg = msg, errmsgarg = 0

const char *errnum = NULL;
char *errmsg = NULL;
int errmsgarg = 0;
bool errmsg_allocated = false;
char_u *buffer;
char_u smallbuf[SMBUFSIZE];
char_u *backup_ext;
int bufsize;
long perm; 
int retval = OK;
int newfile = false; 
int msg_save = msg_scroll;
int overwriting; 
int no_eol = false; 
int device = false; 
int prev_got_int = got_int;
int checking_conversion;
bool file_readonly = false; 
static char *err_readonly =
"is read-only (cannot override: \"W\" in 'cpoptions')";
#if defined(UNIX)
int made_writable = FALSE; 
#endif

int whole = (start == 1 && end == buf->b_ml.ml_line_count);
linenr_T old_line_count = buf->b_ml.ml_line_count;
int fileformat;
int write_bin;
struct bw_info write_info; 
int converted = FALSE;
int notconverted = FALSE;
char_u *fenc; 
char_u *fenc_tofree = NULL; 
#if defined(HAS_BW_FLAGS)
int wb_flags = 0;
#endif
#if defined(HAVE_ACL)
vim_acl_T acl = NULL; 

#endif
int write_undo_file = FALSE;
context_sha256_T sha_ctx;
unsigned int bkc = get_bkc_value(buf);

if (fname == NULL || *fname == NUL) 
return FAIL;
if (buf->b_ml.ml_mfp == NULL) {


EMSG(_(e_emptybuf));
return FAIL;
}





if (check_secure())
return FAIL;


if (STRLEN(fname) >= MAXPATHL) {
EMSG(_(e_longname));
return FAIL;
}


write_info.bw_conv_buf = NULL;
write_info.bw_conv_error = FALSE;
write_info.bw_conv_error_lnum = 0;
write_info.bw_restlen = 0;
#if defined(HAVE_ICONV)
write_info.bw_iconv_fd = (iconv_t)-1;
#endif



ex_no_reprint = TRUE;








if (buf->b_ffname == NULL
&& reset_changed
&& whole
&& buf == curbuf
&& !bt_nofile(buf)
&& !filtering
&& (!append || vim_strchr(p_cpo, CPO_FNAMEAPP) != NULL)
&& vim_strchr(p_cpo, CPO_FNAMEW) != NULL) {
if (set_rw_fname(fname, sfname) == FAIL)
return FAIL;
buf = curbuf; 
}

if (sfname == NULL)
sfname = fname;





ffname = fname; 
#if defined(UNIX)
fname = sfname;
#endif

if (buf->b_ffname != NULL && fnamecmp(ffname, buf->b_ffname) == 0)
overwriting = TRUE;
else
overwriting = FALSE;

++no_wait_return; 




buf->b_op_start.lnum = start;
buf->b_op_start.col = 0;
buf->b_op_end.lnum = end;
buf->b_op_end.col = 0;

{
aco_save_T aco;
int buf_ffname = FALSE;
int buf_sfname = FALSE;
int buf_fname_f = FALSE;
int buf_fname_s = FALSE;
int did_cmd = FALSE;
int nofile_err = FALSE;
int empty_memline = (buf->b_ml.ml_mfp == NULL);
bufref_T bufref;






if (ffname == buf->b_ffname)
buf_ffname = TRUE;
if (sfname == buf->b_sfname)
buf_sfname = TRUE;
if (fname == buf->b_ffname)
buf_fname_f = TRUE;
if (fname == buf->b_sfname)
buf_fname_s = TRUE;


aucmd_prepbuf(&aco, buf);
set_bufref(&bufref, buf);

if (append) {
if (!(did_cmd = apply_autocmds_exarg(EVENT_FILEAPPENDCMD,
sfname, sfname, FALSE, curbuf, eap))) {
if (overwriting && bt_nofile(curbuf))
nofile_err = TRUE;
else
apply_autocmds_exarg(EVENT_FILEAPPENDPRE,
sfname, sfname, FALSE, curbuf, eap);
}
} else if (filtering) {
apply_autocmds_exarg(EVENT_FILTERWRITEPRE,
NULL, sfname, FALSE, curbuf, eap);
} else if (reset_changed && whole) {
int was_changed = curbufIsChanged();

did_cmd = apply_autocmds_exarg(EVENT_BUFWRITECMD,
sfname, sfname, FALSE, curbuf, eap);
if (did_cmd) {
if (was_changed && !curbufIsChanged()) {



u_unchanged(curbuf);
u_update_save_nr(curbuf);
}
} else {
if (overwriting && bt_nofile(curbuf))
nofile_err = TRUE;
else
apply_autocmds_exarg(EVENT_BUFWRITEPRE,
sfname, sfname, FALSE, curbuf, eap);
}
} else {
if (!(did_cmd = apply_autocmds_exarg(EVENT_FILEWRITECMD,
sfname, sfname, FALSE, curbuf, eap))) {
if (overwriting && bt_nofile(curbuf))
nofile_err = TRUE;
else
apply_autocmds_exarg(EVENT_FILEWRITEPRE,
sfname, sfname, FALSE, curbuf, eap);
}
}


aucmd_restbuf(&aco);





if (!bufref_valid(&bufref)) {
buf = NULL;
}
if (buf == NULL || (buf->b_ml.ml_mfp == NULL && !empty_memline)
|| did_cmd || nofile_err
|| aborting()
) {
--no_wait_return;
msg_scroll = msg_save;
if (nofile_err)
EMSG(_("E676: No matching autocommands for acwrite buffer"));

if (nofile_err
|| aborting()
)


return FAIL;
if (did_cmd) {
if (buf == NULL)


return OK;
if (overwriting) {

ml_timestamp(buf);
if (append)
buf->b_flags &= ~BF_NEW;
else
buf->b_flags &= ~BF_WRITE_MASK;
}
if (reset_changed && buf->b_changed && !append
&& (overwriting || vim_strchr(p_cpo, CPO_PLUS) != NULL))


return FAIL;
return OK;
}
if (!aborting())
EMSG(_("E203: Autocommands deleted or unloaded buffer to be written"));
return FAIL;
}







if (buf->b_ml.ml_line_count != old_line_count) {
if (whole) 
end = buf->b_ml.ml_line_count;
else if (buf->b_ml.ml_line_count > old_line_count) 
end += buf->b_ml.ml_line_count - old_line_count;
else { 
end -= old_line_count - buf->b_ml.ml_line_count;
if (end < start) {
--no_wait_return;
msg_scroll = msg_save;
EMSG(_("E204: Autocommand changed number of lines in unexpected way"));
return FAIL;
}
}
}





if (buf_ffname)
ffname = buf->b_ffname;
if (buf_sfname)
sfname = buf->b_sfname;
if (buf_fname_f)
fname = buf->b_ffname;
if (buf_fname_s)
fname = buf->b_sfname;
}


if (shortmess(SHM_OVER) && !exiting)
msg_scroll = FALSE; 
else
msg_scroll = TRUE; 
if (!filtering)
filemess(buf,
#if !defined(UNIX)
sfname,
#else
fname,
#endif
(char_u *)"", 0); 
msg_scroll = FALSE; 

buffer = verbose_try_malloc(BUFSIZE);


if (buffer == NULL) {
buffer = smallbuf;
bufsize = SMBUFSIZE;
} else
bufsize = BUFSIZE;




FileInfo file_info_old;
#if defined(UNIX)
perm = -1;
if (!os_fileinfo((char *)fname, &file_info_old)) {
newfile = TRUE;
} else {
perm = file_info_old.stat.st_mode;
if (!S_ISREG(file_info_old.stat.st_mode)) { 
if (S_ISDIR(file_info_old.stat.st_mode)) {
SET_ERRMSG_NUM("E502", _("is a directory"));
goto fail;
}
if (os_nodetype((char *)fname) != NODE_WRITABLE) {
SET_ERRMSG_NUM("E503", _("is not a file or writable device"));
goto fail;
}


device = TRUE;
newfile = TRUE;
perm = -1;
}
}
#else 

c = fname == NULL ? NODE_OTHER : os_nodetype((char *)fname);
if (c == NODE_OTHER) {
SET_ERRMSG_NUM("E503", _("is not a file or writable device"));
goto fail;
}
if (c == NODE_WRITABLE) {
device = TRUE;
newfile = TRUE;
perm = -1;
} else {
perm = os_getperm((const char *)fname);
if (perm < 0) {
newfile = true;
} else if (os_isdir(fname)) {
SET_ERRMSG_NUM("E502", _("is a directory"));
goto fail;
}
if (overwriting) {
os_fileinfo((char *)fname, &file_info_old);
}
}
#endif 

if (!device && !newfile) {




file_readonly = !os_file_is_writable((char *)fname);

if (!forceit && file_readonly) {
if (vim_strchr(p_cpo, CPO_FWRITE) != NULL) {
SET_ERRMSG_NUM("E504", _(err_readonly));
} else {
SET_ERRMSG_NUM("E505", _("is read-only (add ! to override)"));
}
goto fail;
}




if (overwriting) {
retval = check_mtime(buf, &file_info_old);
if (retval == FAIL)
goto fail;
}
}

#if defined(HAVE_ACL)



if (!newfile)
acl = mch_get_acl(fname);
#endif




dobackup = (p_wb || p_bk || *p_pm != NUL);
if (dobackup && *p_bsk != NUL && match_file_list(p_bsk, sfname, ffname))
dobackup = FALSE;






prev_got_int = got_int;
got_int = FALSE;


buf->b_saving = true;









if (!(append && *p_pm == NUL) && !filtering && perm >= 0 && dobackup) {
FileInfo file_info;
const bool no_prepend_dot = false;

if ((bkc & BKC_YES) || append) { 
backup_copy = TRUE;
} else if ((bkc & BKC_AUTO)) { 
int i;







if (os_fileinfo_hardlinks(&file_info_old) > 1
|| !os_fileinfo_link((char *)fname, &file_info)
|| !os_fileinfo_id_equal(&file_info, &file_info_old)) {
backup_copy = TRUE;
} else {






STRCPY(IObuff, fname);
for (i = 4913;; i += 123) {
sprintf((char *)path_tail(IObuff), "%d", i);
if (!os_fileinfo_link((char *)IObuff, &file_info)) {
break;
}
}
fd = os_open((char *)IObuff,
O_CREAT|O_WRONLY|O_EXCL|O_NOFOLLOW, perm);
if (fd < 0) 
backup_copy = TRUE;
else {
#if defined(UNIX)
os_fchown(fd, file_info_old.stat.st_uid, file_info_old.stat.st_gid);
if (!os_fileinfo((char *)IObuff, &file_info)
|| file_info.stat.st_uid != file_info_old.stat.st_uid
|| file_info.stat.st_gid != file_info_old.stat.st_gid
|| (long)file_info.stat.st_mode != perm) {
backup_copy = TRUE;
}
#endif


close(fd);
os_remove((char *)IObuff);
}
}
}




if ((bkc & BKC_BREAKSYMLINK) || (bkc & BKC_BREAKHARDLINK)) {
#if defined(UNIX)
bool file_info_link_ok = os_fileinfo_link((char *)fname, &file_info);


if ((bkc & BKC_BREAKSYMLINK)
&& file_info_link_ok
&& !os_fileinfo_id_equal(&file_info, &file_info_old)) {
backup_copy = FALSE;
}


if ((bkc & BKC_BREAKHARDLINK)
&& os_fileinfo_hardlinks(&file_info_old) > 1
&& (!file_info_link_ok
|| os_fileinfo_id_equal(&file_info, &file_info_old))) {
backup_copy = FALSE;
}
#endif
}


if (*p_bex == NUL)
backup_ext = (char_u *)".bak";
else
backup_ext = p_bex;

if (backup_copy) {
char_u *wp;
int some_error = false;
char_u *dirp;
char_u *rootname;
char_u *p;













dirp = p_bdir;
while (*dirp) {



(void)copy_option_part(&dirp, IObuff, IOSIZE, ",");
p = IObuff + STRLEN(IObuff);
if (after_pathsep((char *)IObuff, (char *)p) && p[-1] == p[-2]) {

if ((p = (char_u *)make_percent_swname((char *)IObuff, (char *)fname))
!= NULL) {
backup = (char_u *)modname((char *)p, (char *)backup_ext,
no_prepend_dot);
xfree(p);
}
}

rootname = get_file_in_dir(fname, IObuff);
if (rootname == NULL) {
some_error = TRUE; 
goto nobackup;
}

FileInfo file_info_new;
{



if (backup == NULL) {
backup = (char_u *)modname((char *)rootname, (char *)backup_ext,
no_prepend_dot);
}

if (backup == NULL) {
xfree(rootname);
some_error = TRUE; 
goto nobackup;
}




if (os_fileinfo((char *)backup, &file_info_new)) {
if (os_fileinfo_id_equal(&file_info_new, &file_info_old)) {






XFREE_CLEAR(backup); 
} else if (!p_bk) {




wp = backup + STRLEN(backup) - 1 - STRLEN(backup_ext);
if (wp < backup) { 
wp = backup;
}
*wp = 'z';
while (*wp > 'a'
&& os_fileinfo((char *)backup, &file_info_new)) {
--*wp;
}

if (*wp == 'a') {
XFREE_CLEAR(backup);
}
}
}
}
xfree(rootname);




if (backup != NULL) {

os_remove((char *)backup);



(void)os_setperm((const char *)backup, perm & 0777);

#if defined(UNIX)





if (file_info_new.stat.st_gid != file_info_old.stat.st_gid
&& os_chown((char *)backup, -1, file_info_old.stat.st_gid) != 0) {
os_setperm((const char *)backup,
(perm & 0707) | ((perm & 07) << 3));
}
#endif


if (os_copy((char *)fname, (char *)backup, UV_FS_COPYFILE_FICLONE)
!= 0) {
SET_ERRMSG(_("E506: Can't write to backup file "
"(add ! to override)"));
}

#if defined(UNIX)
os_file_settime((char *)backup,
file_info_old.stat.st_atim.tv_sec,
file_info_old.stat.st_mtim.tv_sec);
#endif
#if defined(HAVE_ACL)
mch_set_acl(backup, acl);
#endif
break;
}
}

nobackup:
if (backup == NULL && errmsg == NULL) {
SET_ERRMSG(_(
"E509: Cannot create backup file (add ! to override)"));
}

if ((some_error || errmsg != NULL) && !forceit) {
retval = FAIL;
goto fail;
}
SET_ERRMSG(NULL);
} else {
char_u *dirp;
char_u *p;
char_u *rootname;









if (file_readonly && vim_strchr(p_cpo, CPO_FWRITE) != NULL) {
SET_ERRMSG_NUM("E504", _(err_readonly));
goto fail;
}







dirp = p_bdir;
while (*dirp) {



(void)copy_option_part(&dirp, IObuff, IOSIZE, ",");
p = IObuff + STRLEN(IObuff);
if (after_pathsep((char *)IObuff, (char *)p) && p[-1] == p[-2]) {

if ((p = (char_u *)make_percent_swname((char *)IObuff, (char *)fname))
!= NULL) {
backup = (char_u *)modname((char *)p, (char *)backup_ext,
no_prepend_dot);
xfree(p);
}
}

if (backup == NULL) {
rootname = get_file_in_dir(fname, IObuff);
if (rootname == NULL) {
backup = NULL;
} else {
backup = (char_u *)modname((char *)rootname, (char *)backup_ext,
no_prepend_dot);
xfree(rootname);
}
}

if (backup != NULL) {





if (!p_bk && os_path_exists(backup)) {
p = backup + STRLEN(backup) - 1 - STRLEN(backup_ext);
if (p < backup) 
p = backup;
*p = 'z';
while (*p > 'a' && os_path_exists(backup)) {
(*p)--;
}

if (*p == 'a') {
XFREE_CLEAR(backup);
}
}
}
if (backup != NULL) {








if (vim_rename(fname, backup) == 0) {
break;
}

XFREE_CLEAR(backup); 
}
}
if (backup == NULL && !forceit) {
SET_ERRMSG(_("E510: Can't make backup file (add ! to override)"));
goto fail;
}
}
}

#if defined(UNIX)

if (forceit && perm >= 0 && !(perm & 0200)
&& file_info_old.stat.st_uid == getuid()
&& vim_strchr(p_cpo, CPO_FWRITE) == NULL) {
perm |= 0200;
(void)os_setperm((const char *)fname, perm);
made_writable = true;
}
#endif



if (forceit && overwriting && vim_strchr(p_cpo, CPO_KEEPRO) == NULL) {
buf->b_p_ro = false;
need_maketitle = true; 
status_redraw_all(); 
}

if (end > buf->b_ml.ml_line_count)
end = buf->b_ml.ml_line_count;
if (buf->b_ml.ml_flags & ML_EMPTY)
start = end + 1;






if (reset_changed && !newfile && overwriting
&& !(exiting && backup != NULL)) {
ml_preserve(buf, false, !!p_fs);
if (got_int) {
SET_ERRMSG(_(e_interr));
goto restore_backup;
}
}




wfname = fname;


if (eap != NULL && eap->force_enc != 0) {
fenc = eap->cmd + eap->force_enc;
fenc = enc_canonize(fenc);
fenc_tofree = fenc;
} else {
fenc = buf->b_p_fenc;
}


converted = need_conversion(fenc);




if (converted && (enc_utf8 || STRCMP(p_enc, "latin1") == 0)) {
wb_flags = get_fio_flags(fenc);
if (wb_flags & (FIO_UCS2 | FIO_UCS4 | FIO_UTF16 | FIO_UTF8)) {

if (wb_flags & (FIO_UCS2 | FIO_UTF16 | FIO_UTF8)) {
write_info.bw_conv_buflen = bufsize * 2;
} else { 
write_info.bw_conv_buflen = bufsize * 4;
}
write_info.bw_conv_buf = verbose_try_malloc(write_info.bw_conv_buflen);
if (!write_info.bw_conv_buf) {
end = 0;
}
}
}



if (converted && wb_flags == 0) {
#if defined(HAVE_ICONV)


write_info.bw_iconv_fd = (iconv_t)my_iconv_open(fenc,
enc_utf8 ? (char_u *)"utf-8" : p_enc);
if (write_info.bw_iconv_fd != (iconv_t)-1) {

write_info.bw_conv_buflen = bufsize * ICONV_MULT;
write_info.bw_conv_buf = verbose_try_malloc(write_info.bw_conv_buflen);
if (!write_info.bw_conv_buf) {
end = 0;
}
write_info.bw_first = TRUE;
} else
#endif






if (*p_ccv != NUL) {
wfname = vim_tempname();
if (wfname == NULL) { 
SET_ERRMSG(_("E214: Can't find temp file for writing"));
goto restore_backup;
}
}
}
if (converted && wb_flags == 0
#if defined(HAVE_ICONV)
&& write_info.bw_iconv_fd == (iconv_t)-1
#endif
&& wfname == fname
) {
if (!forceit) {
SET_ERRMSG(_(
"E213: Cannot convert (add ! to write without conversion)"));
goto restore_backup;
}
notconverted = TRUE;
}




for (checking_conversion = true; ; checking_conversion = false) {




if (!converted || dobackup) {
checking_conversion = false;
}

if (checking_conversion) {

fd = -1;
write_info.bw_fd = fd;
} else {








while ((fd = os_open((char *)wfname,
O_WRONLY |
(append ?
(forceit ? (O_APPEND | O_CREAT) : O_APPEND)
: (O_CREAT | O_TRUNC))
, perm < 0 ? 0666 : (perm & 0777))) < 0) {



if (errmsg == NULL) {
#if defined(UNIX)
FileInfo file_info;


if ((!newfile && os_fileinfo_hardlinks(&file_info_old) > 1)
|| (os_fileinfo_link((char *)fname, &file_info)
&& !os_fileinfo_id_equal(&file_info, &file_info_old))) {
SET_ERRMSG(_("E166: Can't open linked file for writing"));
} else {
#endif
SET_ERRMSG_ARG(_("E212: Can't open file for writing: %s"), fd);
if (forceit && vim_strchr(p_cpo, CPO_FWRITE) == NULL
&& perm >= 0) {
#if defined(UNIX)


if (!(perm & 0200)) {
made_writable = true;
}
perm |= 0200;
if (file_info_old.stat.st_uid != getuid()
|| file_info_old.stat.st_gid != getgid()) {
perm &= 0777;
}
#endif
if (!append) { 
os_remove((char *)wfname);
}
continue;
}
#if defined(UNIX)
}
#endif
}

restore_backup:
{



if (backup != NULL && wfname == fname) {
if (backup_copy) {





if (!os_path_exists(fname)) {
vim_rename(backup, fname);
}

if (os_path_exists(fname)) {
os_remove((char *)backup);
}
} else {

vim_rename(backup, fname);
}
}


if (!newfile && !os_path_exists(fname)) {
end = 0;
}
}

if (wfname != fname) {
xfree(wfname);
}
goto fail;
}
write_info.bw_fd = fd;
}
SET_ERRMSG(NULL);

write_info.bw_buf = buffer;
nchars = 0;


if (eap != NULL && eap->force_bin != 0) {
write_bin = (eap->force_bin == FORCE_BIN);
} else {
write_bin = buf->b_p_bin;
}



if (buf->b_p_bomb && !write_bin && (!append || perm < 0)) {
write_info.bw_len = make_bom(buffer, fenc);
if (write_info.bw_len > 0) {

write_info.bw_flags = FIO_NOCONVERT | wb_flags;
if (buf_write_bytes(&write_info) == FAIL) {
end = 0;
} else {
nchars += write_info.bw_len;
}
}
}
write_info.bw_start_lnum = start;

write_undo_file = (buf->b_p_udf && overwriting && !append
&& !filtering && reset_changed && !checking_conversion);
if (write_undo_file) {

sha256_start(&sha_ctx);
}

write_info.bw_len = bufsize;
#if defined(HAS_BW_FLAGS)
write_info.bw_flags = wb_flags;
#endif
fileformat = get_fileformat_force(buf, eap);
s = buffer;
len = 0;
for (lnum = start; lnum <= end; lnum++) {


ptr = ml_get_buf(buf, lnum, false) - 1;
if (write_undo_file) {
sha256_update(&sha_ctx, ptr + 1, (uint32_t)(STRLEN(ptr + 1) + 1));
}
while ((c = *++ptr) != NUL) {
if (c == NL) {
*s = NUL; 
} else if (c == CAR && fileformat == EOL_MAC) {
*s = NL; 
} else {
*s = c;
}
s++;
if (++len != bufsize) {
continue;
}
if (buf_write_bytes(&write_info) == FAIL) {
end = 0; 
break;
}
nchars += bufsize;
s = buffer;
len = 0;
write_info.bw_start_lnum = lnum;
}

if (end == 0
|| (lnum == end
&& (write_bin || !buf->b_p_fixeol)
&& (lnum == buf->b_no_eol_lnum
|| (lnum == buf->b_ml.ml_line_count && !buf->b_p_eol)))) {
lnum++; 
no_eol = true;
break;
}
if (fileformat == EOL_UNIX) {
*s++ = NL;
} else {
*s++ = CAR; 
if (fileformat == EOL_DOS) { 
if (++len == bufsize) {
if (buf_write_bytes(&write_info) == FAIL) {
end = 0; 
break;
}
nchars += bufsize;
s = buffer;
len = 0;
}
*s++ = NL;
}
}
if (++len == bufsize) {
if (buf_write_bytes(&write_info) == FAIL) {
end = 0; 
break;
}
nchars += bufsize;
s = buffer;
len = 0;

os_breakcheck();
if (got_int) {
end = 0; 
break;
}
}
}
if (len > 0 && end > 0) {
write_info.bw_len = len;
if (buf_write_bytes(&write_info) == FAIL) {
end = 0; 
}
nchars += len;
}


if (!checking_conversion || end == 0) {
break;
}



}



if (!checking_conversion) {








int error;
if (p_fs && (error = os_fsync(fd)) != 0 && !device

&& error != UV_ENOTSUP) {
SET_ERRMSG_ARG(e_fsync, error);
end = 0;
}

#if defined(UNIX)


if (backup != NULL && !backup_copy) {


FileInfo file_info;
if (!os_fileinfo((char *)wfname, &file_info)
|| file_info.stat.st_uid != file_info_old.stat.st_uid
|| file_info.stat.st_gid != file_info_old.stat.st_gid) {
os_fchown(fd, file_info_old.stat.st_uid, file_info_old.stat.st_gid);
if (perm >= 0) { 
(void)os_setperm((const char *)wfname, perm);
}
}
buf_set_file_id(buf);
} else if (!buf->file_id_valid) {

buf_set_file_id(buf);
}
#endif

if ((error = os_close(fd)) != 0) {
SET_ERRMSG_ARG(_("E512: Close failed: %s"), error);
end = 0;
}

#if defined(UNIX)
if (made_writable) {
perm &= ~0200; 
}
#endif
if (perm >= 0) { 
(void)os_setperm((const char *)wfname, perm);
}
#if defined(HAVE_ACL)


if (!backup_copy) {
mch_set_acl(wfname, acl);
}
#endif

if (wfname != fname) {


if (end != 0) {
if (eval_charconvert(enc_utf8 ? "utf-8" : (char *)p_enc, (char *)fenc,
(char *)wfname, (char *)fname) == FAIL) {
write_info.bw_conv_error = true;
end = 0;
}
}
os_remove((char *)wfname);
xfree(wfname);
}
}

if (end == 0) {

if (errmsg == NULL) {
if (write_info.bw_conv_error) {
if (write_info.bw_conv_error_lnum == 0) {
SET_ERRMSG(_(
"E513: write error, conversion failed "
"(make 'fenc' empty to override)"));
} else {
errmsg_allocated = true;
SET_ERRMSG(xmalloc(300));
vim_snprintf(
errmsg, 300,
_("E513: write error, conversion failed in line %" PRIdLINENR
" (make 'fenc' empty to override)"),
write_info.bw_conv_error_lnum);
}
} else if (got_int) {
SET_ERRMSG(_(e_interr));
} else {
SET_ERRMSG(_("E514: write error (file system full?)"));
}
}








if (backup != NULL) {
if (backup_copy) {


if (got_int) {
MSG(_(e_interr));
ui_flush();
}


if (os_copy((char *)backup, (char *)fname, UV_FS_COPYFILE_FICLONE)
== 0) {
end = 1; 
}
} else {
if (vim_rename(backup, fname) == 0) {
end = 1;
}
}
}
goto fail;
}

lnum -= start; 
--no_wait_return; 

#if !defined(UNIX)
fname = sfname; 
#endif
if (!filtering) {
add_quoted_fname((char *)IObuff, IOSIZE, buf, (const char *)fname);
c = false;
if (write_info.bw_conv_error) {
STRCAT(IObuff, _(" CONVERSION ERROR"));
c = TRUE;
if (write_info.bw_conv_error_lnum != 0)
vim_snprintf_add((char *)IObuff, IOSIZE, _(" in line %" PRId64 ";"),
(int64_t)write_info.bw_conv_error_lnum);
} else if (notconverted) {
STRCAT(IObuff, _("[NOT converted]"));
c = TRUE;
} else if (converted) {
STRCAT(IObuff, _("[converted]"));
c = TRUE;
}
if (device) {
STRCAT(IObuff, _("[Device]"));
c = TRUE;
} else if (newfile) {
STRCAT(IObuff, shortmess(SHM_NEW) ? _("[New]") : _("[New File]"));
c = TRUE;
}
if (no_eol) {
msg_add_eol();
c = TRUE;
}

if (msg_add_fileformat(fileformat))
c = TRUE;
msg_add_lines(c, (long)lnum, nchars); 
if (!shortmess(SHM_WRITE)) {
if (append)
STRCAT(IObuff, shortmess(SHM_WRI) ? _(" [a]") : _(" appended"));
else
STRCAT(IObuff, shortmess(SHM_WRI) ? _(" [w]") : _(" written"));
}

set_keep_msg(msg_trunc_attr(IObuff, FALSE, 0), 0);
}



if (reset_changed && whole && !append
&& !write_info.bw_conv_error
&& (overwriting || vim_strchr(p_cpo, CPO_PLUS) != NULL)) {
unchanged(buf, true, false);
const varnumber_T changedtick = buf_get_changedtick(buf);
if (buf->b_last_changedtick + 1 == changedtick) {


buf->b_last_changedtick = changedtick;
}
u_unchanged(buf);
u_update_save_nr(buf);
}





if (overwriting) {
ml_timestamp(buf);
if (append)
buf->b_flags &= ~BF_NEW;
else
buf->b_flags &= ~BF_WRITE_MASK;
}





if (*p_pm && dobackup) {
char *org = modname((char *)fname, (char *)p_pm, FALSE);

if (backup != NULL) {




if (org == NULL) {
EMSG(_("E205: Patchmode: can't save original file"));
} else if (!os_path_exists((char_u *)org)) {
vim_rename(backup, (char_u *)org);
XFREE_CLEAR(backup); 
#if defined(UNIX)
os_file_settime(org,
file_info_old.stat.st_atim.tv_sec,
file_info_old.stat.st_mtim.tv_sec);
#endif
}
}




else {
int empty_fd;

if (org == NULL
|| (empty_fd = os_open(org,
O_CREAT | O_EXCL | O_NOFOLLOW,
perm < 0 ? 0666 : (perm & 0777))) < 0)
EMSG(_("E206: patchmode: can't touch empty original file"));
else
close(empty_fd);
}
if (org != NULL) {
os_setperm(org, os_getperm((const char *)fname) & 0777);
xfree(org);
}
}




if (!p_bk && backup != NULL
&& !write_info.bw_conv_error
&& os_remove((char *)backup) != 0) {
EMSG(_("E207: Can't delete backup file"));
}

goto nofail;




fail:
--no_wait_return; 
nofail:


buf->b_saving = false;

xfree(backup);
if (buffer != smallbuf)
xfree(buffer);
xfree(fenc_tofree);
xfree(write_info.bw_conv_buf);
#if defined(HAVE_ICONV)
if (write_info.bw_iconv_fd != (iconv_t)-1) {
iconv_close(write_info.bw_iconv_fd);
write_info.bw_iconv_fd = (iconv_t)-1;
}
#endif
#if defined(HAVE_ACL)
mch_free_acl(acl);
#endif

if (errmsg != NULL) {

#if !defined(UNIX)
add_quoted_fname((char *)IObuff, IOSIZE - 100, buf, (const char *)sfname);
#else
add_quoted_fname((char *)IObuff, IOSIZE - 100, buf, (const char *)fname);
#endif
if (errnum != NULL) {
if (errmsgarg != 0) {
emsgf("%s: %s%s: %s", errnum, IObuff, errmsg, os_strerror(errmsgarg));
} else {
emsgf("%s: %s%s", errnum, IObuff, errmsg);
}
} else if (errmsgarg != 0) {
emsgf(errmsg, os_strerror(errmsgarg));
} else {
EMSG(errmsg);
}
if (errmsg_allocated) {
xfree(errmsg);
}

retval = FAIL;
if (end == 0) {
const int attr = HL_ATTR(HLF_E); 
MSG_PUTS_ATTR(_("\nWARNING: Original file may be lost or damaged\n"),
attr | MSG_HIST);
MSG_PUTS_ATTR(_(
"don't quit the editor until the file is successfully written!"),
attr | MSG_HIST);



if (os_fileinfo((char *)fname, &file_info_old)) {
buf_store_file_info(buf, &file_info_old);
buf->b_mtime_read = buf->b_mtime;
}
}
}
msg_scroll = msg_save;





if (retval == OK && write_undo_file) {
char_u hash[UNDO_HASH_SIZE];

sha256_finish(&sha_ctx, hash);
u_write_undo(NULL, FALSE, buf, hash);
}

if (!should_abort(retval)) {
aco_save_T aco;

curbuf->b_no_eol_lnum = 0; 





aucmd_prepbuf(&aco, buf);

if (append)
apply_autocmds_exarg(EVENT_FILEAPPENDPOST, fname, fname,
FALSE, curbuf, eap);
else if (filtering)
apply_autocmds_exarg(EVENT_FILTERWRITEPOST, NULL, fname,
FALSE, curbuf, eap);
else if (reset_changed && whole)
apply_autocmds_exarg(EVENT_BUFWRITEPOST, fname, fname,
FALSE, curbuf, eap);
else
apply_autocmds_exarg(EVENT_FILEWRITEPOST, fname, fname,
FALSE, curbuf, eap);


aucmd_restbuf(&aco);

if (aborting()) 
retval = FALSE;
}

got_int |= prev_got_int;

return retval;
#undef SET_ERRMSG
#undef SET_ERRMSG_ARG
#undef SET_ERRMSG_NUM
}





static int set_rw_fname(char_u *fname, char_u *sfname)
{
buf_T *buf = curbuf;


if (curbuf->b_p_bl)
apply_autocmds(EVENT_BUFDELETE, NULL, NULL, FALSE, curbuf);
apply_autocmds(EVENT_BUFWIPEOUT, NULL, NULL, FALSE, curbuf);
if (aborting()) 
return FAIL;
if (curbuf != buf) {

EMSG(_(e_auchangedbuf));
return FAIL;
}

if (setfname(curbuf, fname, sfname, false) == OK) {
curbuf->b_flags |= BF_NOTEDITED;
}


apply_autocmds(EVENT_BUFNEW, NULL, NULL, FALSE, curbuf);
if (curbuf->b_p_bl)
apply_autocmds(EVENT_BUFADD, NULL, NULL, FALSE, curbuf);
if (aborting()) 
return FAIL;


if (*curbuf->b_p_ft == NUL) {
if (au_has_group((char_u *)"filetypedetect")) {
(void)do_doautocmd((char_u *)"filetypedetect BufRead", false, NULL);
}
do_modelines(0);
}

return OK;
}









static void add_quoted_fname(char *const ret_buf, const size_t buf_len,
const buf_T *const buf, const char *fname)
FUNC_ATTR_NONNULL_ARG(1)
{
if (fname == NULL) {
fname = "-stdin-";
}
ret_buf[0] = '"';
home_replace(buf, (const char_u *)fname, (char_u *)ret_buf + 1,
(int)buf_len - 4, true);
xstrlcat(ret_buf, "\" ", buf_len);
}






static bool msg_add_fileformat(int eol_type)
{
#if !defined(USE_CRNL)
if (eol_type == EOL_DOS) {
STRCAT(IObuff, shortmess(SHM_TEXT) ? _("[dos]") : _("[dos format]"));
return true;
}
#endif
if (eol_type == EOL_MAC) {
STRCAT(IObuff, shortmess(SHM_TEXT) ? _("[mac]") : _("[mac format]"));
return true;
}
#if defined(USE_CRNL)
if (eol_type == EOL_UNIX) {
STRCAT(IObuff, shortmess(SHM_TEXT) ? _("[unix]") : _("[unix format]"));
return true;
}
#endif
return false;
}




void msg_add_lines(int insert_space, long lnum, off_T nchars)
{
char_u *p;

p = IObuff + STRLEN(IObuff);

if (insert_space)
*p++ = ' ';
if (shortmess(SHM_LINES)) {
sprintf((char *)p, "%" PRId64 "L, %" PRId64 "C",
(int64_t)lnum, (int64_t)nchars);
}
else {
if (lnum == 1)
STRCPY(p, _("1 line, "));
else
sprintf((char *)p, _("%" PRId64 " lines, "), (int64_t)lnum);
p += STRLEN(p);
if (nchars == 1)
STRCPY(p, _("1 character"));
else {
sprintf((char *)p, _("%" PRId64 " characters"), (int64_t)nchars);
}
}
}




static void msg_add_eol(void)
{
STRCAT(IObuff,
shortmess(SHM_LAST) ? _("[noeol]") : _("[Incomplete last line]"));
}






static int check_mtime(buf_T *buf, FileInfo *file_info)
{
if (buf->b_mtime_read != 0
&& time_differs(file_info->stat.st_mtim.tv_sec,
buf->b_mtime_read)) {
msg_scroll = true; 
msg_silent = 0; 

msg_attr(_("WARNING: The file has been changed since reading it!!!"),
HL_ATTR(HLF_E));
if (ask_yesno(_("Do you really want to write to it"), true) == 'n') {
return FAIL;
}
msg_scroll = false; 
}
return OK;
}





static bool time_differs(long t1, long t2) FUNC_ATTR_CONST
{
#if defined(__linux__) || defined(MSWIN)



return t1 - t2 > 1 || t2 - t1 > 1;
#else
return t1 != t2;
#endif
}







static int buf_write_bytes(struct bw_info *ip)
{
int wlen;
char_u *buf = ip->bw_buf; 
int len = ip->bw_len; 
#if defined(HAS_BW_FLAGS)
int flags = ip->bw_flags; 
#endif




if (!(flags & FIO_NOCONVERT)) {
char_u *p;
unsigned c;
int n;

if (flags & FIO_UTF8) {



p = ip->bw_conv_buf; 
for (wlen = 0; wlen < len; ++wlen)
p += utf_char2bytes(buf[wlen], p);
buf = ip->bw_conv_buf;
len = (int)(p - ip->bw_conv_buf);
} else if (flags & (FIO_UCS4 | FIO_UTF16 | FIO_UCS2 | FIO_LATIN1)) {




if (flags & FIO_LATIN1)
p = buf; 
else
p = ip->bw_conv_buf; 
for (wlen = 0; wlen < len; wlen += n) {
if (wlen == 0 && ip->bw_restlen != 0) {
int l;




l = CONV_RESTLEN - ip->bw_restlen;
if (l > len)
l = len;
memmove(ip->bw_rest + ip->bw_restlen, buf, (size_t)l);
n = utf_ptr2len_len(ip->bw_rest, ip->bw_restlen + l);
if (n > ip->bw_restlen + len) {



if (ip->bw_restlen + len > CONV_RESTLEN)
return FAIL;
ip->bw_restlen += len;
break;
}
if (n > 1)
c = utf_ptr2char(ip->bw_rest);
else
c = ip->bw_rest[0];
if (n >= ip->bw_restlen) {
n -= ip->bw_restlen;
ip->bw_restlen = 0;
} else {
ip->bw_restlen -= n;
memmove(ip->bw_rest, ip->bw_rest + n,
(size_t)ip->bw_restlen);
n = 0;
}
} else {
n = utf_ptr2len_len(buf + wlen, len - wlen);
if (n > len - wlen) {



if (len - wlen > CONV_RESTLEN)
return FAIL;
ip->bw_restlen = len - wlen;
memmove(ip->bw_rest, buf + wlen,
(size_t)ip->bw_restlen);
break;
}
if (n > 1)
c = utf_ptr2char(buf + wlen);
else
c = buf[wlen];
}

if (ucs2bytes(c, &p, flags) && !ip->bw_conv_error) {
ip->bw_conv_error = TRUE;
ip->bw_conv_error_lnum = ip->bw_start_lnum;
}
if (c == NL)
++ip->bw_start_lnum;
}
if (flags & FIO_LATIN1)
len = (int)(p - buf);
else {
buf = ip->bw_conv_buf;
len = (int)(p - ip->bw_conv_buf);
}
}

#if defined(HAVE_ICONV)
if (ip->bw_iconv_fd != (iconv_t)-1) {
const char *from;
size_t fromlen;
char *to;
size_t tolen;


if (ip->bw_restlen > 0) {
char *fp;




fromlen = len + ip->bw_restlen;
fp = (char *)ip->bw_conv_buf + ip->bw_conv_buflen - fromlen;
memmove(fp, ip->bw_rest, (size_t)ip->bw_restlen);
memmove(fp + ip->bw_restlen, buf, (size_t)len);
from = fp;
tolen = ip->bw_conv_buflen - fromlen;
} else {
from = (const char *)buf;
fromlen = len;
tolen = ip->bw_conv_buflen;
}
to = (char *)ip->bw_conv_buf;

if (ip->bw_first) {
size_t save_len = tolen;


(void)iconv(ip->bw_iconv_fd, NULL, NULL, &to, &tolen);




if (to == NULL) {
to = (char *)ip->bw_conv_buf;
tolen = save_len;
}
ip->bw_first = FALSE;
}




if ((iconv(ip->bw_iconv_fd, (void *)&from, &fromlen, &to, &tolen)
== (size_t)-1 && ICONV_ERRNO != ICONV_EINVAL)
|| fromlen > CONV_RESTLEN) {
ip->bw_conv_error = TRUE;
return FAIL;
}


if (fromlen > 0)
memmove(ip->bw_rest, (void *)from, fromlen);
ip->bw_restlen = (int)fromlen;

buf = ip->bw_conv_buf;
len = (int)((char_u *)to - ip->bw_conv_buf);
}
#endif
}

if (ip->bw_fd < 0) {

return OK;
}
wlen = write_eintr(ip->bw_fd, buf, len);
return (wlen < len) ? FAIL : OK;
}








static bool ucs2bytes(unsigned c, char_u **pp, int flags) FUNC_ATTR_NONNULL_ALL
{
char_u *p = *pp;
bool error = false;
int cc;


if (flags & FIO_UCS4) {
if (flags & FIO_ENDIAN_L) {
*p++ = c;
*p++ = (c >> 8);
*p++ = (c >> 16);
*p++ = (c >> 24);
} else {
*p++ = (c >> 24);
*p++ = (c >> 16);
*p++ = (c >> 8);
*p++ = c;
}
} else if (flags & (FIO_UCS2 | FIO_UTF16)) {
if (c >= 0x10000) {
if (flags & FIO_UTF16) {


c -= 0x10000;
if (c >= 0x100000) {
error = true;
}
cc = ((c >> 10) & 0x3ff) + 0xd800;
if (flags & FIO_ENDIAN_L) {
*p++ = cc;
*p++ = ((unsigned)cc >> 8);
} else {
*p++ = ((unsigned)cc >> 8);
*p++ = cc;
}
c = (c & 0x3ff) + 0xdc00;
} else {
error = true;
}
}
if (flags & FIO_ENDIAN_L) {
*p++ = c;
*p++ = (c >> 8);
} else {
*p++ = (c >> 8);
*p++ = c;
}
} else { 
if (c >= 0x100) {
error = true;
*p++ = 0xBF;
} else
*p++ = c;
}

*pp = p;
return error;
}







static bool need_conversion(const char_u *fenc)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
int same_encoding;
int enc_flags;
int fenc_flags;

if (*fenc == NUL || STRCMP(p_enc, fenc) == 0) {
same_encoding = TRUE;
fenc_flags = 0;
} else {


enc_flags = get_fio_flags(p_enc);
fenc_flags = get_fio_flags(fenc);
same_encoding = (enc_flags != 0 && fenc_flags == enc_flags);
}
if (same_encoding) {

return false;
}



return !(enc_utf8 && fenc_flags == FIO_UTF8);
}






static int get_fio_flags(const char_u *name)
{
int prop;

if (*name == NUL) {
name = p_enc;
}
prop = enc_canon_props(name);
if (prop & ENC_UNICODE) {
if (prop & ENC_2BYTE) {
if (prop & ENC_ENDIAN_L)
return FIO_UCS2 | FIO_ENDIAN_L;
return FIO_UCS2;
}
if (prop & ENC_4BYTE) {
if (prop & ENC_ENDIAN_L)
return FIO_UCS4 | FIO_ENDIAN_L;
return FIO_UCS4;
}
if (prop & ENC_2WORD) {
if (prop & ENC_ENDIAN_L)
return FIO_UTF16 | FIO_ENDIAN_L;
return FIO_UTF16;
}
return FIO_UTF8;
}
if (prop & ENC_LATIN1)
return FIO_LATIN1;

return 0;
}









static char_u *check_for_bom(char_u *p, long size, int *lenp, int flags)
{
char *name = NULL;
int len = 2;

if (p[0] == 0xef && p[1] == 0xbb && size >= 3 && p[2] == 0xbf
&& (flags == FIO_ALL || flags == FIO_UTF8 || flags == 0)) {
name = "utf-8"; 
len = 3;
} else if (p[0] == 0xff && p[1] == 0xfe) {
if (size >= 4 && p[2] == 0 && p[3] == 0
&& (flags == FIO_ALL || flags == (FIO_UCS4 | FIO_ENDIAN_L))) {
name = "ucs-4le"; 
len = 4;
} else if (flags == (FIO_UCS2 | FIO_ENDIAN_L))
name = "ucs-2le"; 
else if (flags == FIO_ALL || flags == (FIO_UTF16 | FIO_ENDIAN_L))

name = "utf-16le"; 
} else if (p[0] == 0xfe && p[1] == 0xff
&& (flags == FIO_ALL || flags == FIO_UCS2 || flags ==
FIO_UTF16)) {

if (flags == FIO_UCS2)
name = "ucs-2"; 
else
name = "utf-16"; 
} else if (size >= 4 && p[0] == 0 && p[1] == 0 && p[2] == 0xfe
&& p[3] == 0xff && (flags == FIO_ALL || flags == FIO_UCS4)) {
name = "ucs-4"; 
len = 4;
}

*lenp = len;
return (char_u *)name;
}





static int make_bom(char_u *buf, char_u *name)
{
int flags;
char_u *p;

flags = get_fio_flags(name);


if (flags == FIO_LATIN1 || flags == 0)
return 0;

if (flags == FIO_UTF8) { 
buf[0] = 0xef;
buf[1] = 0xbb;
buf[2] = 0xbf;
return 3;
}
p = buf;
(void)ucs2bytes(0xfeff, &p, flags);
return (int)(p - buf);
}








void shorten_buf_fname(buf_T *buf, char_u *dirname, int force)
{
char_u *p;

if (buf->b_fname != NULL
&& !bt_nofile(buf)
&& !path_with_url((char *)buf->b_fname)
&& (force
|| buf->b_sfname == NULL
|| path_is_absolute(buf->b_sfname))) {
XFREE_CLEAR(buf->b_sfname);
p = path_shorten_fname(buf->b_ffname, dirname);
if (p != NULL) {
buf->b_sfname = vim_strsave(p);
buf->b_fname = buf->b_sfname;
}
if (p == NULL) {
buf->b_fname = buf->b_ffname;
}
}
}


void shorten_fnames(int force)
{
char_u dirname[MAXPATHL];

os_dirname(dirname, MAXPATHL);
FOR_ALL_BUFFERS(buf) {
shorten_buf_fname(buf, dirname, force);



mf_fullname(buf->b_ml.ml_mfp);
}
status_redraw_all();
redraw_tabline = TRUE;
}

























char *modname(const char *fname, const char *ext, bool prepend_dot)
FUNC_ATTR_NONNULL_ARG(2)
{
char *retval;
size_t fnamelen;
size_t extlen = strlen(ext);



if (fname == NULL || *fname == NUL) {
retval = xmalloc(MAXPATHL + extlen + 3); 
if (os_dirname((char_u *)retval, MAXPATHL) == FAIL
|| (fnamelen = strlen(retval)) == 0) {
xfree(retval);
return NULL;
}
add_pathsep(retval);
fnamelen = strlen(retval);
prepend_dot = FALSE; 
} else {
fnamelen = strlen(fname);
retval = xmalloc(fnamelen + extlen + 3);
strcpy(retval, fname);
}



char *ptr = NULL;
for (ptr = retval + fnamelen; ptr > retval; MB_PTR_BACK(retval, ptr)) {
if (vim_ispathsep(*ptr)) {
ptr++;
break;
}
}


if (strlen(ptr) > BASENAMELEN) {
ptr[BASENAMELEN] = '\0';
}

char *s;
s = ptr + strlen(ptr);



strcpy(s, ext);

char *e;

if (prepend_dot && *(e = (char *)path_tail((char_u *)retval)) != '.') {
STRMOVE(e + 1, e);
*e = '.';
}



if (fname != NULL && strcmp(fname, retval) == 0) {

while (--s >= ptr) {
if (*s != '_') {
*s = '_';
break;
}
}
if (s < ptr) { 
*ptr = 'v';
}
}
return retval;
}









bool vim_fgets(char_u *buf, int size, FILE *fp) FUNC_ATTR_NONNULL_ALL
{
char *retval;

assert(size > 0);
buf[size - 2] = NUL;

do {
errno = 0;
retval = fgets((char *)buf, size, fp);
} while (retval == NULL && errno == EINTR && ferror(fp));

if (buf[size - 2] != NUL && buf[size - 2] != '\n') {
char tbuf[200];

buf[size - 1] = NUL; 


do {
tbuf[sizeof(tbuf) - 2] = NUL;
errno = 0;
retval = fgets((char *)tbuf, sizeof(tbuf), fp);
if (retval == NULL && (feof(fp) || errno != EINTR)) {
break;
}
} while (tbuf[sizeof(tbuf) - 2] != NUL && tbuf[sizeof(tbuf) - 2] != '\n');
}
return retval == NULL;
}



int get2c(FILE *fd)
{
const int n = getc(fd);
if (n == EOF) {
return -1;
}
const int c = getc(fd);
if (c == EOF) {
return -1;
}
return (n << 8) + c;
}



int get3c(FILE *fd)
{
int n = getc(fd);
if (n == EOF) {
return -1;
}
int c = getc(fd);
if (c == EOF) {
return -1;
}
n = (n << 8) + c;
c = getc(fd);
if (c == EOF) {
return -1;
}
return (n << 8) + c;
}



int get4c(FILE *fd)
{


unsigned n;

int c = getc(fd);
if (c == EOF) {
return -1;
}
n = (unsigned)c;
c = getc(fd);
if (c == EOF) {
return -1;
}
n = (n << 8) + (unsigned)c;
c = getc(fd);
if (c == EOF) {
return -1;
}
n = (n << 8) + (unsigned)c;
c = getc(fd);
if (c == EOF) {
return -1;
}
n = (n << 8) + (unsigned)c;
return (int)n;
}



time_t get8ctime(FILE *fd)
{
time_t n = 0;

for (int i = 0; i < 8; i++) {
const int c = getc(fd);
if (c == EOF) {
return -1;
}
n = (n << 8) + c;
}
return n;
}



char *read_string(FILE *fd, size_t cnt)
{
char *str = xmallocz(cnt);
for (size_t i = 0; i < cnt; i++) {
int c = getc(fd);
if (c == EOF) {
xfree(str);
return NULL;
}
str[i] = (char)c;
}
return str;
}



bool put_bytes(FILE *fd, uintmax_t number, size_t len)
{
assert(len > 0);
for (size_t i = len - 1; i < len; i--) {
if (putc((int)(number >> (i * 8)), fd) == EOF) {
return false;
}
}
return true;
}



int put_time(FILE *fd, time_t time_)
{
uint8_t buf[8];
time_to_bytes(time_, buf);
return fwrite(buf, sizeof(uint8_t), ARRAY_SIZE(buf), fd) == 1 ? OK : FAIL;
}





int vim_rename(const char_u *from, const char_u *to)
FUNC_ATTR_NONNULL_ALL
{
int fd_in;
int fd_out;
int n;
char *errmsg = NULL;
char *buffer;
long perm;
#if defined(HAVE_ACL)
vim_acl_T acl; 
#endif
bool use_tmp_file = false;






if (fnamecmp(from, to) == 0) {
if (p_fic && (STRCMP(path_tail((char_u *)from), path_tail((char_u *)to))
!= 0)) {
use_tmp_file = true;
} else {
return 0;
}
}


FileInfo from_info;
if (!os_fileinfo((char *)from, &from_info)) {
return -1;
}




FileInfo to_info;
if (os_fileinfo((char *)to, &to_info)
&& os_fileinfo_id_equal(&from_info, &to_info)) {
use_tmp_file = true;
}

if (use_tmp_file) {
char_u tempname[MAXPATHL + 1];





if (STRLEN(from) >= MAXPATHL - 5)
return -1;
STRCPY(tempname, from);
for (n = 123; n < 99999; n++) {
char * tail = (char *)path_tail(tempname);
snprintf(tail, (MAXPATHL + 1) - (tail - (char *)tempname - 1), "%d", n);

if (!os_path_exists(tempname)) {
if (os_rename(from, tempname) == OK) {
if (os_rename(tempname, to) == OK)
return 0;


os_rename(tempname, from);
return -1;
}


return -1;
}
}
return -1;
}







os_remove((char *)to);




if (os_rename(from, to) == OK)
return 0;




perm = os_getperm((const char *)from);
#if defined(HAVE_ACL)

acl = mch_get_acl(from);
#endif
fd_in = os_open((char *)from, O_RDONLY, 0);
if (fd_in < 0) {
#if defined(HAVE_ACL)
mch_free_acl(acl);
#endif
return -1;
}


fd_out = os_open((char *)to,
O_CREAT|O_EXCL|O_WRONLY|O_NOFOLLOW, (int)perm);
if (fd_out < 0) {
close(fd_in);
#if defined(HAVE_ACL)
mch_free_acl(acl);
#endif
return -1;
}



buffer = try_malloc(BUFSIZE);
if (buffer == NULL) {
close(fd_out);
close(fd_in);
#if defined(HAVE_ACL)
mch_free_acl(acl);
#endif
return -1;
}

while ((n = read_eintr(fd_in, buffer, BUFSIZE)) > 0)
if (write_eintr(fd_out, buffer, n) != n) {
errmsg = _("E208: Error writing to \"%s\"");
break;
}

xfree(buffer);
close(fd_in);
if (close(fd_out) < 0)
errmsg = _("E209: Error closing \"%s\"");
if (n < 0) {
errmsg = _("E210: Error reading \"%s\"");
to = from;
}
#if !defined(UNIX)
os_setperm((const char *)to, perm);
#endif
#if defined(HAVE_ACL)
mch_set_acl(to, acl);
mch_free_acl(acl);
#endif
if (errmsg != NULL) {
EMSG2(errmsg, to);
return -1;
}
os_remove((char *)from);
return 0;
}

static int already_warned = FALSE;







int
check_timestamps(
int focus 
)
{
int didit = 0;
int n;



if (no_check_timestamps > 0)
return FALSE;




if (focus && did_check_timestamps) {
need_check_timestamps = TRUE;
return FALSE;
}

if (!stuff_empty() || global_busy || !typebuf_typed()
|| autocmd_busy || curbuf_lock > 0 || allbuf_lock > 0
) {
need_check_timestamps = true; 
} else {
no_wait_return++;
did_check_timestamps = true;
already_warned = false;
FOR_ALL_BUFFERS(buf) {

if (buf->b_nwindows > 0) {
bufref_T bufref;
set_bufref(&bufref, buf);
n = buf_check_timestamp(buf, focus);
if (didit < n) {
didit = n;
}
if (n > 0 && !bufref_valid(&bufref)) {

buf = firstbuf;
continue;
}
}
}
--no_wait_return;
need_check_timestamps = FALSE;
if (need_wait_return && didit == 2) {

msg_puts("\n");
ui_flush();
}
}
return didit;
}






static int move_lines(buf_T *frombuf, buf_T *tobuf)
{
buf_T *tbuf = curbuf;
int retval = OK;
linenr_T lnum;
char_u *p;


curbuf = tobuf;
for (lnum = 1; lnum <= frombuf->b_ml.ml_line_count; lnum++) {
p = vim_strsave(ml_get_buf(frombuf, lnum, false));
if (ml_append(lnum - 1, p, 0, false) == FAIL) {
xfree(p);
retval = FAIL;
break;
}
xfree(p);
}


if (retval != FAIL) {
curbuf = frombuf;
for (lnum = curbuf->b_ml.ml_line_count; lnum > 0; lnum--) {
if (ml_delete(lnum, false) == FAIL) {


retval = FAIL;
break;
}
}
}

curbuf = tbuf;
return retval;
}








int
buf_check_timestamp(
buf_T *buf,
int focus 
)
FUNC_ATTR_NONNULL_ALL
{
int retval = 0;
char_u *path;
char *mesg = NULL;
char *mesg2 = "";
bool helpmesg = false;
bool reload = false;
bool can_reload = false;
uint64_t orig_size = buf->b_orig_size;
int orig_mode = buf->b_orig_mode;
static bool busy = false;
char_u *s;
char *reason;

bufref_T bufref;
set_bufref(&bufref, buf);




if (buf->terminal
|| buf->b_ffname == NULL
|| buf->b_ml.ml_mfp == NULL
|| !bt_normal(buf)
|| buf->b_saving
|| busy
)
return 0;

FileInfo file_info;
bool file_info_ok;
if (!(buf->b_flags & BF_NOTEDITED)
&& buf->b_mtime != 0
&& (!(file_info_ok = os_fileinfo((char *)buf->b_ffname, &file_info))
|| time_differs(file_info.stat.st_mtim.tv_sec, buf->b_mtime)
|| (int)file_info.stat.st_mode != buf->b_orig_mode)) {
const long prev_b_mtime = buf->b_mtime;

retval = 1;



if (!file_info_ok) {

buf->b_mtime = -1;
buf->b_orig_size = 0;
buf->b_orig_mode = 0;
} else {
buf_store_file_info(buf, &file_info);
}



if (os_isdir(buf->b_fname)) {
} else if ((buf->b_p_ar >= 0 ? buf->b_p_ar : p_ar)
&& !bufIsChanged(buf) && file_info_ok) {



reload = true;
} else {
if (!file_info_ok) {
reason = "deleted";
} else if (bufIsChanged(buf)) {
reason = "conflict";
} else if (orig_size != buf->b_orig_size || buf_contents_changed(buf)) {
reason = "changed";
} else if (orig_mode != buf->b_orig_mode) {
reason = "mode";
} else {
reason = "time";
}




busy = true;
set_vim_var_string(VV_FCS_REASON, reason, -1);
set_vim_var_string(VV_FCS_CHOICE, "", -1);
allbuf_lock++;
bool n = apply_autocmds(EVENT_FILECHANGEDSHELL,
buf->b_fname, buf->b_fname, false, buf);
allbuf_lock--;
busy = false;
if (n) {
if (!bufref_valid(&bufref)) {
EMSG(_("E246: FileChangedShell autocommand deleted buffer"));
}
s = get_vim_var_str(VV_FCS_CHOICE);
if (STRCMP(s, "reload") == 0 && *reason != 'd') {
reload = true;
} else if (STRCMP(s, "ask") == 0) {
n = false;
} else {
return 2;
}
}
if (!n) {
if (*reason == 'd') {

if (prev_b_mtime != -1) {
mesg = _("E211: File \"%s\" no longer available");
}
} else {
helpmesg = true;
can_reload = true;





if (reason[2] == 'n') {
mesg = _(
"W12: Warning: File \"%s\" has changed and the buffer was changed in Vim as well");
mesg2 = _("See \":help W12\" for more info.");
} else if (reason[1] == 'h') {
mesg = _(
"W11: Warning: File \"%s\" has changed since editing started");
mesg2 = _("See \":help W11\" for more info.");
} else if (*reason == 'm') {
mesg = _(
"W16: Warning: Mode of file \"%s\" has changed since editing started");
mesg2 = _("See \":help W16\" for more info.");
} else


buf->b_mtime_read = buf->b_mtime;
}
}
}

} else if ((buf->b_flags & BF_NEW) && !(buf->b_flags & BF_NEW_W)
&& os_path_exists(buf->b_ffname)) {
retval = 1;
mesg = _("W13: Warning: File \"%s\" has been created after editing started");
buf->b_flags |= BF_NEW_W;
can_reload = true;
}

if (mesg != NULL) {
path = home_replace_save(buf, buf->b_fname);
if (!helpmesg) {
mesg2 = "";
}
const size_t tbuf_len = STRLEN(path) + STRLEN(mesg) + STRLEN(mesg2) + 2;
char *const tbuf = xmalloc(tbuf_len);
snprintf(tbuf, tbuf_len, mesg, path);


set_vim_var_string(VV_WARNINGMSG, tbuf, -1);
if (can_reload) {
if (*mesg2 != NUL) {
xstrlcat(tbuf, "\n", tbuf_len - 1);
xstrlcat(tbuf, mesg2, tbuf_len - 1);
}
if (do_dialog(VIM_WARNING, (char_u *) _("Warning"), (char_u *) tbuf,
(char_u *) _("&OK\n&Load File"), 1, NULL, true) == 2) {
reload = true;
}
} else if (State > NORMAL_BUSY || (State & CMDLINE) || already_warned) {
if (*mesg2 != NUL) {
xstrlcat(tbuf, "; ", tbuf_len - 1);
xstrlcat(tbuf, mesg2, tbuf_len - 1);
}
EMSG(tbuf);
retval = 2;
} else {
if (!autocmd_busy) {
msg_start();
msg_puts_attr(tbuf, HL_ATTR(HLF_E) + MSG_HIST);
if (*mesg2 != NUL) {
msg_puts_attr(mesg2, HL_ATTR(HLF_W) + MSG_HIST);
}
msg_clr_eos();
(void)msg_end();
if (emsg_silent == 0) {
ui_flush();

os_delay(1000L, true);


redraw_cmdline = FALSE;
}
}
already_warned = TRUE;
}

xfree(path);
xfree(tbuf);
}

if (reload) {

buf_reload(buf, orig_mode);
if (buf->b_p_udf && buf->b_ffname != NULL) {
char_u hash[UNDO_HASH_SIZE];
buf_T *save_curbuf = curbuf;


curbuf = buf;
u_compute_hash(hash);
u_write_undo(NULL, FALSE, buf, hash);
curbuf = save_curbuf;
}
}


if (bufref_valid(&bufref) && retval != 0) {
(void)apply_autocmds(EVENT_FILECHANGEDSHELLPOST, buf->b_fname, buf->b_fname,
false, buf);
}
return retval;
}







void buf_reload(buf_T *buf, int orig_mode)
{
exarg_T ea;
pos_T old_cursor;
linenr_T old_topline;
int old_ro = buf->b_p_ro;
buf_T *savebuf;
bufref_T bufref;
int saved = OK;
aco_save_T aco;
int flags = READ_NEW;


aucmd_prepbuf(&aco, buf);





prep_exarg(&ea, buf);
old_cursor = curwin->w_cursor;
old_topline = curwin->w_topline;

if (p_ur < 0 || curbuf->b_ml.ml_line_count <= p_ur) {


u_sync(FALSE);
saved = u_savecommon(0, curbuf->b_ml.ml_line_count + 1, 0, TRUE);
flags |= READ_KEEP_UNDO;
}






if (BUFEMPTY() || saved == FAIL) {
savebuf = NULL;
} else {

savebuf = buflist_new(NULL, NULL, (linenr_T)1, BLN_DUMMY);
set_bufref(&bufref, savebuf);
if (savebuf != NULL && buf == curbuf) {

curbuf = savebuf;
curwin->w_buffer = savebuf;
saved = ml_open(curbuf);
curbuf = buf;
curwin->w_buffer = buf;
}
if (savebuf == NULL || saved == FAIL || buf != curbuf
|| move_lines(buf, savebuf) == FAIL) {
EMSG2(_("E462: Could not prepare for reloading \"%s\""),
buf->b_fname);
saved = FAIL;
}
}

if (saved == OK) {
curbuf->b_flags |= BF_CHECK_RO; 
keep_filetype = true; 
if (readfile(buf->b_ffname, buf->b_fname, (linenr_T)0, (linenr_T)0,
(linenr_T)MAXLNUM, &ea, flags) != OK) {
if (!aborting()) {
EMSG2(_("E321: Could not reload \"%s\""), buf->b_fname);
}
if (savebuf != NULL && bufref_valid(&bufref) && buf == curbuf) {


while (!BUFEMPTY()) {
if (ml_delete(buf->b_ml.ml_line_count, false) == FAIL) {
break;
}
}
(void)move_lines(savebuf, buf);
}
} else if (buf == curbuf) { 

unchanged(buf, true, true);
if ((flags & READ_KEEP_UNDO) == 0) {
u_blockfree(buf);
u_clearall(buf);
} else {

u_unchanged(curbuf);
}
}
}
xfree(ea.cmd);

if (savebuf != NULL && bufref_valid(&bufref)) {
wipe_buffer(savebuf, false);
}


diff_invalidate(curbuf);



if (old_topline > curbuf->b_ml.ml_line_count)
curwin->w_topline = curbuf->b_ml.ml_line_count;
else
curwin->w_topline = old_topline;
curwin->w_cursor = old_cursor;
check_cursor();
update_topline();
keep_filetype = FALSE;


FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->w_buffer == curwin->w_buffer
&& !foldmethodIsManual(wp)) {
foldUpdateAll(wp);
}
}




if (orig_mode == curbuf->b_orig_mode)
curbuf->b_p_ro |= old_ro;


do_modelines(0);


aucmd_restbuf(&aco);

}

void buf_store_file_info(buf_T *buf, FileInfo *file_info)
FUNC_ATTR_NONNULL_ALL
{
buf->b_mtime = file_info->stat.st_mtim.tv_sec;
buf->b_orig_size = os_fileinfo_size(file_info);
buf->b_orig_mode = (int)file_info->stat.st_mode;
}





void write_lnum_adjust(linenr_T offset)
{
if (curbuf->b_no_eol_lnum != 0) 
curbuf->b_no_eol_lnum += offset;
}

#if defined(BACKSLASH_IN_FILENAME)


void forward_slash(char_u *fname)
{
char_u *p;

if (path_with_url((const char *)fname)) {
return;
}
for (p = fname; *p != NUL; p++) {

if (*p == '\\') {
*p = '/';
}
}
}
#endif


static char_u *vim_tempdir = NULL;






static void vim_maketempdir(void)
{
static const char *temp_dirs[] = TEMP_DIR_NAMES;

char_u template[TEMP_FILE_PATH_MAXLEN];
char_u path[TEMP_FILE_PATH_MAXLEN];



mode_t umask_save = umask(0077);
for (size_t i = 0; i < ARRAY_SIZE(temp_dirs); i++) {

expand_env((char_u *)temp_dirs[i], template, TEMP_FILE_PATH_MAXLEN - 22);
if (!os_isdir(template)) { 
continue;
}

add_pathsep((char *)template);

STRCAT(template, "nvimXXXXXX");

if (os_mkdtemp((const char *)template, (char *)path) != 0) {
continue;
}

if (vim_settempdir((char *)path)) {

break;
} else {

os_rmdir((char *)path);
}
}
(void)umask(umask_save);
}




int delete_recursive(const char *name)
{
int result = 0;

if (os_isrealdir(name)) {
snprintf((char *)NameBuff, MAXPATHL, "%s/*", name); 

char_u **files;
int file_count;
char_u *exp = vim_strsave(NameBuff);
if (gen_expand_wildcards(1, &exp, &file_count, &files,
EW_DIR | EW_FILE | EW_SILENT | EW_ALLLINKS
| EW_DODOT | EW_EMPTYOK) == OK) {
for (int i = 0; i < file_count; i++) {
if (delete_recursive((const char *)files[i]) != 0) {
result = -1;
}
}
FreeWild(file_count, files);
} else {
result = -1;
}

xfree(exp);
os_rmdir(name);
} else {
result = os_remove(name) == 0 ? 0 : -1;
}

return result;
}


void vim_deltempdir(void)
{
if (vim_tempdir != NULL) {

path_tail(vim_tempdir)[-1] = NUL;
delete_recursive((const char *)vim_tempdir);
XFREE_CLEAR(vim_tempdir);
}
}



char_u *vim_gettempdir(void)
{
if (vim_tempdir == NULL) {
vim_maketempdir();
}

return vim_tempdir;
}








static bool vim_settempdir(char *tempdir)
{
char *buf = verbose_try_malloc(MAXPATHL + 2);
if (!buf) {
return false;
}
vim_FullName(tempdir, buf, MAXPATHL, false);
add_pathsep(buf);
vim_tempdir = (char_u *)xstrdup(buf);
xfree(buf);
return true;
}







char_u *vim_tempname(void)
{

static uint64_t temp_count;

char_u *tempdir = vim_gettempdir();
if (!tempdir) {
return NULL;
}



char_u template[TEMP_FILE_PATH_MAXLEN];
snprintf((char *)template, TEMP_FILE_PATH_MAXLEN,
"%s%" PRIu64, tempdir, temp_count++);
return vim_strsave(template);
}





#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "auevents_name_map.generated.h"
#endif

static AutoPatCmd *active_apc_list = NULL; 


static garray_T augroups = { 0, 0, sizeof(char_u *), 10, NULL };
#define AUGROUP_NAME(i) (((char **)augroups.ga_data)[i])




static int current_augroup = AUGROUP_DEFAULT;

static int au_need_clean = FALSE; 



static event_T last_event;
static int last_group;
static int autocmd_blocked = 0; 


static const char *deleted_augroup = NULL;

static inline const char *get_deleted_augroup(void)
FUNC_ATTR_ALWAYS_INLINE
{
if (deleted_augroup == NULL) {
deleted_augroup = _("--Deleted--");
}
return deleted_augroup;
}




static void show_autocmd(AutoPat *ap, event_T event)
{
AutoCmd *ac;



if (got_int)
return;
if (ap->pat == NULL) 
return;

msg_putchar('\n');
if (got_int)
return;
if (event != last_event || ap->group != last_group) {
if (ap->group != AUGROUP_DEFAULT) {
if (AUGROUP_NAME(ap->group) == NULL) {
msg_puts_attr(get_deleted_augroup(), HL_ATTR(HLF_E));
} else {
msg_puts_attr(AUGROUP_NAME(ap->group), HL_ATTR(HLF_T));
}
msg_puts(" ");
}
msg_puts_attr(event_nr2name(event), HL_ATTR(HLF_T));
last_event = event;
last_group = ap->group;
msg_putchar('\n');
if (got_int)
return;
}
msg_col = 4;
msg_outtrans(ap->pat);

for (ac = ap->cmds; ac != NULL; ac = ac->next) {
if (ac->cmd == NULL) { 
continue;
}
if (msg_col >= 14) {
msg_putchar('\n');
}
msg_col = 14;
if (got_int) {
return;
}
msg_outtrans(ac->cmd);
if (p_verbose > 0) {
last_set_msg(ac->script_ctx);
}
if (got_int) {
return;
}
if (ac->next != NULL) {
msg_putchar('\n');
if (got_int) {
return;
}
}
}
}


static void au_remove_pat(AutoPat *ap)
{
XFREE_CLEAR(ap->pat);
ap->buflocal_nr = -1;
au_need_clean = true;
}


static void au_remove_cmds(AutoPat *ap)
{
for (AutoCmd *ac = ap->cmds; ac != NULL; ac = ac->next) {
XFREE_CLEAR(ac->cmd);
}
au_need_clean = true;
}


static void au_del_cmd(AutoCmd *ac)
{
XFREE_CLEAR(ac->cmd);
au_need_clean = true;
}



static void au_cleanup(void)
{
AutoPat *ap, **prev_ap;
AutoCmd *ac, **prev_ac;
event_T event;

if (autocmd_busy || !au_need_clean) {
return;
}


for (event = (event_T)0; (int)event < (int)NUM_EVENTS;
event = (event_T)((int)event + 1)) {

prev_ap = &(first_autopat[(int)event]);
for (ap = *prev_ap; ap != NULL; ap = *prev_ap) {

prev_ac = &(ap->cmds);
bool has_cmd = false;

for (ac = *prev_ac; ac != NULL; ac = *prev_ac) {


if (ap->pat == NULL || ac->cmd == NULL) {
*prev_ac = ac->next;
xfree(ac->cmd);
xfree(ac);
} else {
has_cmd = true;
prev_ac = &(ac->next);
}
}

if (ap->pat != NULL && !has_cmd) {


au_remove_pat(ap);
}


if (ap->pat == NULL) {
if (ap->next == NULL) {
if (prev_ap == &(first_autopat[(int)event])) {
last_autopat[(int)event] = NULL;
} else {


last_autopat[(int)event] = (AutoPat *)prev_ap;
}
}
*prev_ap = ap->next;
vim_regfree(ap->reg_prog);
xfree(ap);
} else {
prev_ap = &(ap->next);
}
}
}

au_need_clean = false;
}





void aubuflocal_remove(buf_T *buf)
{
AutoPat *ap;
event_T event;
AutoPatCmd *apc;


for (apc = active_apc_list; apc; apc = apc->next)
if (buf->b_fnum == apc->arg_bufnr)
apc->arg_bufnr = 0;


for (event = (event_T)0; (int)event < (int)NUM_EVENTS;
event = (event_T)((int)event + 1))

for (ap = first_autopat[(int)event]; ap != NULL; ap = ap->next)
if (ap->buflocal_nr == buf->b_fnum) {
au_remove_pat(ap);
if (p_verbose >= 6) {
verbose_enter();
smsg(_("auto-removing autocommand: %s <buffer=%d>"),
event_nr2name(event), buf->b_fnum);
verbose_leave();
}
}
au_cleanup();
}



static int au_new_group(char_u *name)
{
int i = au_find_group(name);
if (i == AUGROUP_ERROR) { 

for (i = 0; i < augroups.ga_len; i++) {
if (AUGROUP_NAME(i) == NULL) {
break;
}
}
if (i == augroups.ga_len) {
ga_grow(&augroups, 1);
}

AUGROUP_NAME(i) = xstrdup((char *)name);
if (i == augroups.ga_len) {
augroups.ga_len++;
}
}

return i;
}

static void au_del_group(char_u *name)
{
int i = au_find_group(name);
if (i == AUGROUP_ERROR) { 
EMSG2(_("E367: No such group: \"%s\""), name);
} else if (i == current_augroup) {
EMSG(_("E936: Cannot delete the current group"));
} else {
event_T event;
AutoPat *ap;
int in_use = false;

for (event = (event_T)0; (int)event < (int)NUM_EVENTS;
event = (event_T)((int)event + 1)) {
for (ap = first_autopat[(int)event]; ap != NULL; ap = ap->next) {
if (ap->group == i && ap->pat != NULL) {
give_warning((char_u *)
_("W19: Deleting augroup that is still in use"), true);
in_use = true;
event = NUM_EVENTS;
break;
}
}
}
xfree(AUGROUP_NAME(i));
if (in_use) {
AUGROUP_NAME(i) = (char *)get_deleted_augroup();
} else {
AUGROUP_NAME(i) = NULL;
}
}
}






static int au_find_group(const char_u *name)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
for (int i = 0; i < augroups.ga_len; i++) {
if (AUGROUP_NAME(i) != NULL && AUGROUP_NAME(i) != get_deleted_augroup()
&& STRCMP(AUGROUP_NAME(i), name) == 0) {
return i;
}
}
return AUGROUP_ERROR;
}




bool au_has_group(const char_u *name)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return au_find_group(name) != AUGROUP_ERROR;
}


void do_augroup(char_u *arg, int del_group)
{
if (del_group) {
if (*arg == NUL) {
EMSG(_(e_argreq));
} else {
au_del_group(arg);
}
} else if (STRICMP(arg, "end") == 0) { 
current_augroup = AUGROUP_DEFAULT;
} else if (*arg) { 
int i = au_new_group(arg);
if (i != AUGROUP_ERROR)
current_augroup = i;
} else { 
msg_start();
for (int i = 0; i < augroups.ga_len; ++i) {
if (AUGROUP_NAME(i) != NULL) {
msg_puts(AUGROUP_NAME(i));
msg_puts(" ");
}
}
msg_clr_eos();
msg_end();
}
}

#if defined(EXITFREE)
void free_all_autocmds(void)
{
for (current_augroup = -1; current_augroup < augroups.ga_len;
current_augroup++) {
do_autocmd((char_u *)"", true);
}

for (int i = 0; i < augroups.ga_len; i++) {
char *const s = ((char **)(augroups.ga_data))[i];
if ((const char *)s != get_deleted_augroup()) {
xfree(s);
}
}
ga_clear(&augroups);
}
#endif






static event_T event_name2nr(const char_u *start, char_u **end)
{
const char_u *p;
int i;
int len;


for (p = start; *p && !ascii_iswhite(*p) && *p != ',' && *p != '|'; p++) {
}
for (i = 0; event_names[i].name != NULL; i++) {
len = (int)event_names[i].len;
if (len == p - start && STRNICMP(event_names[i].name, start, len) == 0) {
break;
}
}
if (*p == ',') {
p++;
}
*end = (char_u *)p;
if (event_names[i].name == NULL) {
return NUM_EVENTS;
}
return event_names[i].event;
}






static const char *event_nr2name(event_T event)
FUNC_ATTR_NONNULL_RET FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_CONST
{
int i;

for (i = 0; event_names[i].name != NULL; i++) {
if (event_names[i].event == event) {
return event_names[i].name;
}
}
return "Unknown";
}




static char_u *
find_end_event (
char_u *arg,
int have_group 
)
{
char_u *pat;
char_u *p;

if (*arg == '*') {
if (arg[1] && !ascii_iswhite(arg[1])) {
EMSG2(_("E215: Illegal character after *: %s"), arg);
return NULL;
}
pat = arg + 1;
} else {
for (pat = arg; *pat && *pat != '|' && !ascii_iswhite(*pat); pat = p) {
if ((int)event_name2nr(pat, &p) >= (int)NUM_EVENTS) {
if (have_group)
EMSG2(_("E216: No such event: %s"), pat);
else
EMSG2(_("E216: No such group or event: %s"), pat);
return NULL;
}
}
}
return pat;
}




static bool event_ignored(event_T event)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
char_u *p = p_ei;

while (*p != NUL) {
if (STRNICMP(p, "all", 3) == 0 && (p[3] == NUL || p[3] == ',')) {
return true;
}
if (event_name2nr(p, &p) == event) {
return true;
}
}

return false;
}




int check_ei(void)
{
char_u *p = p_ei;

while (*p) {
if (STRNICMP(p, "all", 3) == 0 && (p[3] == NUL || p[3] == ',')) {
p += 3;
if (*p == ',')
++p;
} else if (event_name2nr(p, &p) == NUM_EVENTS)
return FAIL;
}

return OK;
}






char_u *au_event_disable(char *what)
{
char_u *new_ei;
char_u *save_ei;

save_ei = vim_strsave(p_ei);
new_ei = vim_strnsave(p_ei, (int)(STRLEN(p_ei) + STRLEN(what)));
if (*what == ',' && *p_ei == NUL)
STRCPY(new_ei, what + 1);
else
STRCAT(new_ei, what);
set_string_option_direct((char_u *)"ei", -1, new_ei, OPT_FREE, SID_NONE);
xfree(new_ei);

return save_ei;
}

void au_event_restore(char_u *old_ei)
{
if (old_ei != NULL) {
set_string_option_direct((char_u *)"ei", -1, old_ei,
OPT_FREE, SID_NONE);
xfree(old_ei);
}
}

































void do_autocmd(char_u *arg_in, int forceit)
{
char_u *arg = arg_in;
char_u *pat;
char_u *envpat = NULL;
char_u *cmd;
int need_free = false;
int nested = false;
bool once = false;
int group;

if (*arg == '|') {
arg = (char_u *)"";
group = AUGROUP_ALL; 
} else {

group = au_get_grouparg(&arg);
}





pat = find_end_event(arg, group != AUGROUP_ALL);
if (pat == NULL)
return;

pat = skipwhite(pat);
if (*pat == '|') {
pat = (char_u *)"";
cmd = (char_u *)"";
} else {

cmd = pat;
while (*cmd && (!ascii_iswhite(*cmd) || cmd[-1] == '\\')) {
cmd++;
}
if (*cmd) {
*cmd++ = NUL;
}



if (vim_strchr(pat, '$') != NULL || vim_strchr(pat, '~') != NULL) {
#if defined(BACKSLASH_IN_FILENAME)
int p_ssl_save = p_ssl;

p_ssl = true;
#endif
envpat = expand_env_save(pat);
#if defined(BACKSLASH_IN_FILENAME)
p_ssl = p_ssl_save;
#endif
if (envpat != NULL) {
pat = envpat;
}
}

cmd = skipwhite(cmd);
for (size_t i = 0; i < 2; i++) {
if (*cmd != NUL) {

if (STRNCMP(cmd, "++once", 6) == 0 && ascii_iswhite(cmd[6])) {
if (once) {
EMSG2(_(e_duparg2), "++once");
}
once = true;
cmd = skipwhite(cmd + 6);
}


if ((STRNCMP(cmd, "++nested", 8) == 0 && ascii_iswhite(cmd[8]))) {
if (nested) {
EMSG2(_(e_duparg2), "++nested");
}
nested = true;
cmd = skipwhite(cmd + 8);
}


if (STRNCMP(cmd, "nested", 6) == 0 && ascii_iswhite(cmd[6])) {
if (nested) {
EMSG2(_(e_duparg2), "nested");
}
nested = true;
cmd = skipwhite(cmd + 6);
}
}
}



if (*cmd != NUL) {
cmd = expand_sfile(cmd);
if (cmd == NULL) { 
return;
}
need_free = true;
}
}




if (!forceit && *cmd == NUL) {

MSG_PUTS_TITLE(_("\n--- Autocommands ---"));
}




last_event = (event_T)-1; 
last_group = AUGROUP_ERROR; 
if (*arg == '*' || *arg == NUL || *arg == '|') {
for (event_T event = (event_T)0; (int)event < (int)NUM_EVENTS;
event = (event_T)((int)event + 1)) {
if (do_autocmd_event(event, pat, once, nested, cmd, forceit, group)
== FAIL) {
break;
}
}
} else {
while (*arg && *arg != '|' && !ascii_iswhite(*arg)) {
event_T event = event_name2nr(arg, &arg);
assert(event < NUM_EVENTS);
if (do_autocmd_event(event, pat, once, nested, cmd, forceit, group)
== FAIL) {
break;
}
}
}

if (need_free)
xfree(cmd);
xfree(envpat);
}







static int au_get_grouparg(char_u **argp)
{
char_u *group_name;
char_u *p;
char_u *arg = *argp;
int group = AUGROUP_ALL;

for (p = arg; *p && !ascii_iswhite(*p) && *p != '|'; p++) {
}
if (p > arg) {
group_name = vim_strnsave(arg, (int)(p - arg));
group = au_find_group(group_name);
if (group == AUGROUP_ERROR)
group = AUGROUP_ALL; 
else
*argp = skipwhite(p); 
xfree(group_name);
}
return group;
}








static int do_autocmd_event(event_T event, char_u *pat, bool once, int nested,
char_u *cmd, int forceit, int group)
{
AutoPat *ap;
AutoPat **prev_ap;
AutoCmd *ac;
AutoCmd **prev_ac;
int brace_level;
char_u *endpat;
int findgroup;
int allgroups;
int patlen;
int is_buflocal;
int buflocal_nr;
char_u buflocal_pat[25]; 

if (group == AUGROUP_ALL)
findgroup = current_augroup;
else
findgroup = group;
allgroups = (group == AUGROUP_ALL && !forceit && *cmd == NUL);




if (*pat == NUL) {
for (ap = first_autopat[(int)event]; ap != NULL; ap = ap->next) {
if (forceit) { 
if (ap->group == findgroup)
au_remove_pat(ap);
} else if (group == AUGROUP_ALL || ap->group == group)
show_autocmd(ap, event);
}
}




for (; *pat; pat = (*endpat == ',' ? endpat + 1 : endpat)) {




endpat = pat;

if (*endpat == ',') {
continue;
}
brace_level = 0;
for (; *endpat && (*endpat != ',' || brace_level || endpat[-1] == '\\');
++endpat) {
if (*endpat == '{')
brace_level++;
else if (*endpat == '}')
brace_level--;
}
patlen = (int)(endpat - pat);




is_buflocal = FALSE;
buflocal_nr = 0;

if (patlen >= 8 && STRNCMP(pat, "<buffer", 7) == 0
&& pat[patlen - 1] == '>') {


is_buflocal = TRUE;
if (patlen == 8)

buflocal_nr = curbuf->b_fnum;
else if (patlen > 9 && pat[7] == '=') {
if (patlen == 13 && STRNICMP(pat, "<buffer=abuf>", 13) == 0)

buflocal_nr = autocmd_bufnr;
else if (skipdigits(pat + 8) == pat + patlen - 1)

buflocal_nr = atoi((char *)pat + 8);
}
}

if (is_buflocal) {

sprintf((char *)buflocal_pat, "<buffer=%d>", buflocal_nr);
pat = buflocal_pat; 
patlen = (int)STRLEN(buflocal_pat); 
}



if (!forceit && *cmd != NUL && last_autopat[(int)event] != NULL) {
prev_ap = &last_autopat[(int)event];
} else {
prev_ap = &first_autopat[(int)event];
}
while ((ap = *prev_ap) != NULL) {
if (ap->pat != NULL) {









if ((allgroups || ap->group == findgroup)
&& ap->patlen == patlen
&& STRNCMP(pat, ap->pat, patlen) == 0) {






if (forceit) {
if (*cmd != NUL && ap->next == NULL) {
au_remove_cmds(ap);
break;
}
au_remove_pat(ap);
}



else if (*cmd == NUL)
show_autocmd(ap, event);




else if (ap->next == NULL)
break;
}
}
prev_ap = &ap->next;
}




if (*cmd != NUL) {





if (ap == NULL) {

if (is_buflocal && (buflocal_nr == 0
|| buflist_findnr(buflocal_nr) == NULL)) {
emsgf(_("E680: <buffer=%d>: invalid buffer number "),
buflocal_nr);
return FAIL;
}

ap = xmalloc(sizeof(AutoPat));
ap->pat = vim_strnsave(pat, patlen);
ap->patlen = patlen;

if (is_buflocal) {
ap->buflocal_nr = buflocal_nr;
ap->reg_prog = NULL;
} else {
char_u *reg_pat;

ap->buflocal_nr = 0;
reg_pat = file_pat_to_reg_pat(pat, endpat,
&ap->allow_dirs, TRUE);
if (reg_pat != NULL)
ap->reg_prog = vim_regcomp(reg_pat, RE_MAGIC);
xfree(reg_pat);
if (reg_pat == NULL || ap->reg_prog == NULL) {
xfree(ap->pat);
xfree(ap);
return FAIL;
}
}
ap->cmds = NULL;
*prev_ap = ap;
last_autopat[(int)event] = ap;
ap->next = NULL;
if (group == AUGROUP_ALL)
ap->group = current_augroup;
else
ap->group = group;
}




prev_ac = &(ap->cmds);
while ((ac = *prev_ac) != NULL)
prev_ac = &ac->next;
ac = xmalloc(sizeof(AutoCmd));
ac->cmd = vim_strsave(cmd);
ac->script_ctx = current_sctx;
ac->script_ctx.sc_lnum += sourcing_lnum;
ac->next = NULL;
*prev_ac = ac;
ac->once = once;
ac->nested = nested;
}
}

au_cleanup(); 
return OK;
}



int
do_doautocmd(
char_u *arg,
int do_msg, 
bool *did_something
)
{
char_u *fname;
int nothing_done = TRUE;
int group;

if (did_something != NULL) {
*did_something = false;
}




group = au_get_grouparg(&arg);

if (*arg == '*') {
EMSG(_("E217: Can't execute autocommands for ALL events"));
return FAIL;
}





fname = find_end_event(arg, group != AUGROUP_ALL);
if (fname == NULL)
return FAIL;

fname = skipwhite(fname);


while (*arg && !ends_excmd(*arg) && !ascii_iswhite(*arg)) {
if (apply_autocmds_group(event_name2nr(arg, &arg), fname, NULL, true,
group, curbuf, NULL)) {
nothing_done = false;
}
}

if (nothing_done && do_msg) {
MSG(_("No matching autocommands"));
}
if (did_something != NULL) {
*did_something = !nothing_done;
}

return aborting() ? FAIL : OK;
}




void ex_doautoall(exarg_T *eap)
{
int retval;
aco_save_T aco;
char_u *arg = eap->arg;
int call_do_modelines = check_nomodeline(&arg);
bufref_T bufref;








FOR_ALL_BUFFERS(buf) {
if (buf->b_ml.ml_mfp == NULL) {
continue;
}

aucmd_prepbuf(&aco, buf);
set_bufref(&bufref, buf);

bool did_aucmd;

retval = do_doautocmd(arg, false, &did_aucmd);

if (call_do_modelines && did_aucmd) {



do_modelines(curwin == aucmd_win ? OPT_NOWIN : 0);
}


aucmd_restbuf(&aco);


if (retval == FAIL || !bufref_valid(&bufref)) {
break;
}
}

check_cursor(); 
}






bool check_nomodeline(char_u **argp)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
if (STRNCMP(*argp, "<nomodeline>", 12) == 0) {
*argp = skipwhite(*argp + 12);
return false;
}
return true;
}








void aucmd_prepbuf(aco_save_T *aco, buf_T *buf)
{
win_T *win;
bool need_append = true; 


if (buf == curbuf) { 
win = curwin;
} else {
win = NULL;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_buffer == buf) {
win = wp;
break;
}
}
}


if (win == NULL && aucmd_win == NULL) {
win_alloc_aucmd_win();
need_append = false;
}
if (win == NULL && aucmd_win_used)


win = curwin;

aco->save_curwin = curwin;
aco->save_prevwin = prevwin;
aco->save_curbuf = curbuf;
if (win != NULL) {



aco->use_aucmd_win = FALSE;
curwin = win;
} else {




aco->use_aucmd_win = TRUE;
aucmd_win_used = TRUE;
aucmd_win->w_buffer = buf;
aucmd_win->w_s = &buf->b_s;
++buf->b_nwindows;
win_init_empty(aucmd_win); 



XFREE_CLEAR(aucmd_win->w_localdir);
aco->globaldir = globaldir;
globaldir = NULL;

block_autocmds(); 
if (need_append) {
win_append(lastwin, aucmd_win);
handle_register_window(aucmd_win);
win_config_float(aucmd_win, aucmd_win->w_float_config);
}

int save_acd = p_acd;
p_acd = false;
win_enter(aucmd_win, false);
p_acd = save_acd;
unblock_autocmds();
curwin = aucmd_win;
}
curbuf = buf;
aco->new_curwin = curwin;
set_bufref(&aco->new_curbuf, curbuf);
}





void aucmd_restbuf(aco_save_T *aco)
{
if (aco->use_aucmd_win) {
curbuf->b_nwindows--;


block_autocmds();
if (curwin != aucmd_win) {
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp == aucmd_win) {
if (tp != curtab) {
goto_tabpage_tp(tp, true, true);
}
win_goto(aucmd_win);
goto win_found;
}
}
}
win_found:

win_remove(curwin, NULL);
handle_unregister_window(curwin);
if (curwin->w_grid.chars != NULL) {
ui_comp_remove_grid(&curwin->w_grid);
ui_call_win_hide(curwin->w_grid.handle);
grid_free(&curwin->w_grid);
}

aucmd_win_used = false;
last_status(false); 

if (!valid_tabpage_win(curtab)) {

close_tabpage(curtab);
}

unblock_autocmds();

if (win_valid(aco->save_curwin)) {
curwin = aco->save_curwin;
} else {

curwin = firstwin;
}
prevwin = win_valid(aco->save_prevwin) ? aco->save_prevwin
: firstwin; 
vars_clear(&aucmd_win->w_vars->dv_hashtab); 
hash_init(&aucmd_win->w_vars->dv_hashtab); 
curbuf = curwin->w_buffer;

xfree(globaldir);
globaldir = aco->globaldir;


check_cursor();
if (curwin->w_topline > curbuf->b_ml.ml_line_count) {
curwin->w_topline = curbuf->b_ml.ml_line_count;
curwin->w_topfill = 0;
}
} else {

if (win_valid(aco->save_curwin)) {


if (curwin == aco->new_curwin
&& curbuf != aco->new_curbuf.br_buf
&& bufref_valid(&aco->new_curbuf)
&& aco->new_curbuf.br_buf->b_ml.ml_mfp != NULL) {
if (curwin->w_s == &curbuf->b_s) {
curwin->w_s = &aco->new_curbuf.br_buf->b_s;
}
curbuf->b_nwindows--;
curbuf = aco->new_curbuf.br_buf;
curwin->w_buffer = curbuf;
curbuf->b_nwindows++;
}

curwin = aco->save_curwin;
prevwin = win_valid(aco->save_prevwin) ? aco->save_prevwin
: firstwin; 
curbuf = curwin->w_buffer;


check_cursor();
}
}
}

static int autocmd_nested = FALSE;










bool apply_autocmds(event_T event, char_u *fname, char_u *fname_io, bool force,
buf_T *buf)
{
return apply_autocmds_group(event, fname, fname_io, force,
AUGROUP_ALL, buf, NULL);
}












static bool apply_autocmds_exarg(event_T event, char_u *fname, char_u *fname_io,
bool force, buf_T *buf, exarg_T *eap)
{
return apply_autocmds_group(event, fname, fname_io, force,
AUGROUP_ALL, buf, eap);
}














bool apply_autocmds_retval(event_T event, char_u *fname, char_u *fname_io,
bool force, buf_T *buf, int *retval)
{
if (should_abort(*retval)) {
return false;
}

bool did_cmd = apply_autocmds_group(event, fname, fname_io, force,
AUGROUP_ALL, buf, NULL);
if (did_cmd && aborting()) {
*retval = FAIL;
}
return did_cmd;
}



bool has_cursorhold(void) FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return first_autopat[(int)(get_real_state() == NORMAL_BUSY
? EVENT_CURSORHOLD : EVENT_CURSORHOLDI)] != NULL;
}


bool trigger_cursorhold(void) FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
int state;

if (!did_cursorhold
&& has_cursorhold()
&& reg_recording == 0
&& typebuf.tb_len == 0
&& !ins_compl_active()
) {
state = get_real_state();
if (state == NORMAL_BUSY || (state & INSERT) != 0) {
return true;
}
}
return false;
}




bool has_event(event_T event) FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return first_autopat[event] != NULL;
}













static bool apply_autocmds_group(event_T event, char_u *fname, char_u *fname_io,
bool force, int group, buf_T *buf,
exarg_T *eap)
{
char_u *sfname = NULL; 
char_u *tail;
bool save_changed;
buf_T *old_curbuf;
bool retval = false;
char_u *save_sourcing_name;
linenr_T save_sourcing_lnum;
char_u *save_autocmd_fname;
int save_autocmd_bufnr;
char_u *save_autocmd_match;
int save_autocmd_busy;
int save_autocmd_nested;
static int nesting = 0;
AutoPatCmd patcmd;
AutoPat *ap;
void *save_funccalp;
char_u *save_cmdarg;
long save_cmdbang;
static int filechangeshell_busy = FALSE;
proftime_T wait_time;
bool did_save_redobuff = false;
save_redo_T save_redo;
const bool save_KeyTyped = KeyTyped;



if (event == NUM_EVENTS || first_autopat[(int)event] == NULL
|| autocmd_blocked > 0) {
goto BYPASS_AU;
}





if (autocmd_busy && !(force || autocmd_nested))
goto BYPASS_AU;





if (aborting())
goto BYPASS_AU;




if (filechangeshell_busy && (event == EVENT_FILECHANGEDSHELL
|| event == EVENT_FILECHANGEDSHELLPOST))
goto BYPASS_AU;




if (event_ignored(event))
goto BYPASS_AU;





if (nesting == 10) {
EMSG(_("E218: autocommand nesting too deep"));
goto BYPASS_AU;
}





if ( (autocmd_no_enter
&& (event == EVENT_WINENTER || event == EVENT_BUFENTER))
|| (autocmd_no_leave
&& (event == EVENT_WINLEAVE || event == EVENT_BUFLEAVE)))
goto BYPASS_AU;




save_autocmd_fname = autocmd_fname;
save_autocmd_bufnr = autocmd_bufnr;
save_autocmd_match = autocmd_match;
save_autocmd_busy = autocmd_busy;
save_autocmd_nested = autocmd_nested;
save_changed = curbuf->b_changed;
old_curbuf = curbuf;






if (fname_io == NULL) {
if (event == EVENT_COLORSCHEME
|| event == EVENT_COLORSCHEMEPRE
|| event == EVENT_OPTIONSET) {
autocmd_fname = NULL;
} else if (fname != NULL && !ends_excmd(*fname)) {
autocmd_fname = fname;
} else if (buf != NULL) {
autocmd_fname = buf->b_ffname;
} else {
autocmd_fname = NULL;
}
} else {
autocmd_fname = fname_io;
}
if (autocmd_fname != NULL) {

autocmd_fname = vim_strnsave(autocmd_fname, MAXPATHL);
}




if (buf == NULL)
autocmd_bufnr = 0;
else
autocmd_bufnr = buf->b_fnum;






if (fname == NULL || *fname == NUL) {
if (buf == NULL)
fname = NULL;
else {
if (event == EVENT_SYNTAX)
fname = buf->b_p_syn;
else if (event == EVENT_FILETYPE)
fname = buf->b_p_ft;
else {
if (buf->b_sfname != NULL)
sfname = vim_strsave(buf->b_sfname);
fname = buf->b_ffname;
}
}
if (fname == NULL)
fname = (char_u *)"";
fname = vim_strsave(fname); 
} else {
sfname = vim_strsave(fname);

if (event == EVENT_CMDLINECHANGED
|| event == EVENT_CMDLINEENTER
|| event == EVENT_CMDLINELEAVE
|| event == EVENT_CMDWINENTER
|| event == EVENT_CMDWINLEAVE
|| event == EVENT_CMDUNDEFINED
|| event == EVENT_COLORSCHEME
|| event == EVENT_COLORSCHEMEPRE
|| event == EVENT_DIRCHANGED
|| event == EVENT_FILETYPE
|| event == EVENT_FUNCUNDEFINED
|| event == EVENT_OPTIONSET
|| event == EVENT_QUICKFIXCMDPOST
|| event == EVENT_QUICKFIXCMDPRE
|| event == EVENT_REMOTEREPLY
|| event == EVENT_SPELLFILEMISSING
|| event == EVENT_SYNTAX
|| event == EVENT_SIGNAL
|| event == EVENT_TABCLOSED
|| event == EVENT_WINCLOSED) {
fname = vim_strsave(fname);
} else {
fname = (char_u *)FullName_save((char *)fname, false);
}
}
if (fname == NULL) { 
xfree(sfname);
retval = false;
goto BYPASS_AU;
}

#if defined(BACKSLASH_IN_FILENAME)


if (sfname != NULL) {
forward_slash(sfname);
}
forward_slash(fname);
#endif





autocmd_match = fname;



RedrawingDisabled++;
save_sourcing_name = sourcing_name;
sourcing_name = NULL; 
save_sourcing_lnum = sourcing_lnum;
sourcing_lnum = 0; 

const sctx_T save_current_sctx = current_sctx;

if (do_profiling == PROF_YES)
prof_child_enter(&wait_time); 


save_funccalp = save_funccal();




if (!autocmd_busy) {
save_search_patterns();
if (!ins_compl_active()) {
saveRedobuff(&save_redo);
did_save_redobuff = true;
}
did_filetype = keep_filetype;
}




autocmd_busy = TRUE;
filechangeshell_busy = (event == EVENT_FILECHANGEDSHELL);
++nesting; 


if (event == EVENT_FILETYPE)
did_filetype = TRUE;

tail = path_tail(fname);


patcmd.curpat = first_autopat[(int)event];
patcmd.nextcmd = NULL;
patcmd.group = group;
patcmd.fname = fname;
patcmd.sfname = sfname;
patcmd.tail = tail;
patcmd.event = event;
patcmd.arg_bufnr = autocmd_bufnr;
patcmd.next = NULL;
auto_next_pat(&patcmd, false);


if (patcmd.curpat != NULL) {

patcmd.next = active_apc_list;
active_apc_list = &patcmd;


save_cmdbang = (long)get_vim_var_nr(VV_CMDBANG);
if (eap != NULL) {
save_cmdarg = set_cmdarg(eap, NULL);
set_vim_var_nr(VV_CMDBANG, (long)eap->forceit);
} else {
save_cmdarg = NULL; 
}
retval = true;


for (ap = patcmd.curpat; ap->next != NULL; ap = ap->next) {
ap->last = false;
}
ap->last = true;
check_lnums(true); 


do_cmdline(NULL, getnextac, (void *)&patcmd,
DOCMD_NOWAIT|DOCMD_VERBOSE|DOCMD_REPEAT);

reset_lnums(); 

if (eap != NULL) {
(void)set_cmdarg(NULL, save_cmdarg);
set_vim_var_nr(VV_CMDBANG, save_cmdbang);
}

if (active_apc_list == &patcmd) 
active_apc_list = patcmd.next;
}

--RedrawingDisabled;
autocmd_busy = save_autocmd_busy;
filechangeshell_busy = FALSE;
autocmd_nested = save_autocmd_nested;
xfree(sourcing_name);
sourcing_name = save_sourcing_name;
sourcing_lnum = save_sourcing_lnum;
xfree(autocmd_fname);
autocmd_fname = save_autocmd_fname;
autocmd_bufnr = save_autocmd_bufnr;
autocmd_match = save_autocmd_match;
current_sctx = save_current_sctx;
restore_funccal(save_funccalp);
if (do_profiling == PROF_YES)
prof_child_exit(&wait_time);
KeyTyped = save_KeyTyped;
xfree(fname);
xfree(sfname);
--nesting; 




if (!autocmd_busy) {
restore_search_patterns();
if (did_save_redobuff) {
restoreRedobuff(&save_redo);
}
did_filetype = FALSE;
while (au_pending_free_buf != NULL) {
buf_T *b = au_pending_free_buf->b_next;
xfree(au_pending_free_buf);
au_pending_free_buf = b;
}
while (au_pending_free_win != NULL) {
win_T *w = au_pending_free_win->w_next;
xfree(au_pending_free_win);
au_pending_free_win = w;
}
}





if (curbuf == old_curbuf
&& (event == EVENT_BUFREADPOST
|| event == EVENT_BUFWRITEPOST
|| event == EVENT_FILEAPPENDPOST
|| event == EVENT_VIMLEAVE
|| event == EVENT_VIMLEAVEPRE)) {
if (curbuf->b_changed != save_changed)
need_maketitle = TRUE;
curbuf->b_changed = save_changed;
}

au_cleanup(); 

BYPASS_AU:


if (event == EVENT_BUFWIPEOUT && buf != NULL)
aubuflocal_remove(buf);

if (retval == OK && event == EVENT_FILETYPE) {
au_did_filetype = true;
}

return retval;
}

static char_u *old_termresponse = NULL;





void block_autocmds(void)
{

if (autocmd_blocked == 0)
old_termresponse = get_vim_var_str(VV_TERMRESPONSE);
++autocmd_blocked;
}

void unblock_autocmds(void)
{
--autocmd_blocked;




if (autocmd_blocked == 0
&& get_vim_var_str(VV_TERMRESPONSE) != old_termresponse)
apply_autocmds(EVENT_TERMRESPONSE, NULL, NULL, FALSE, curbuf);
}


static void
auto_next_pat(
AutoPatCmd *apc,
int stop_at_last 
)
{
AutoPat *ap;
AutoCmd *cp;
char *s;

XFREE_CLEAR(sourcing_name);

for (ap = apc->curpat; ap != NULL && !got_int; ap = ap->next) {
apc->curpat = NULL;




if (ap->pat != NULL && ap->cmds != NULL
&& (apc->group == AUGROUP_ALL || apc->group == ap->group)) {

if (ap->buflocal_nr == 0
? match_file_pat(NULL, &ap->reg_prog, apc->fname, apc->sfname,
apc->tail, ap->allow_dirs)
: ap->buflocal_nr == apc->arg_bufnr) {
const char *const name = event_nr2name(apc->event);
s = _("%s Autocommands for \"%s\"");
const size_t sourcing_name_len = (STRLEN(s) + strlen(name) + ap->patlen
+ 1);
sourcing_name = xmalloc(sourcing_name_len);
snprintf((char *)sourcing_name, sourcing_name_len, s, name,
(char *)ap->pat);
if (p_verbose >= 8) {
verbose_enter();
smsg(_("Executing %s"), sourcing_name);
verbose_leave();
}

apc->curpat = ap;
apc->nextcmd = ap->cmds;

for (cp = ap->cmds; cp->next != NULL; cp = cp->next)
cp->last = FALSE;
cp->last = TRUE;
}
line_breakcheck();
if (apc->curpat != NULL) 
break;
}
if (stop_at_last && ap->last)
break;
}
}




char_u *getnextac(int c, void *cookie, int indent, bool do_concat)
{
AutoPatCmd *acp = (AutoPatCmd *)cookie;
char_u *retval;
AutoCmd *ac;


if (acp->curpat == NULL)
return NULL;


for (;; ) {

while (acp->nextcmd != NULL && acp->nextcmd->cmd == NULL)
if (acp->nextcmd->last)
acp->nextcmd = NULL;
else
acp->nextcmd = acp->nextcmd->next;

if (acp->nextcmd != NULL)
break;


if (acp->curpat->last)
acp->curpat = NULL;
else
acp->curpat = acp->curpat->next;
if (acp->curpat != NULL)
auto_next_pat(acp, TRUE);
if (acp->curpat == NULL)
return NULL;
}

ac = acp->nextcmd;

if (p_verbose >= 9) {
verbose_enter_scroll();
smsg(_("autocommand %s"), ac->cmd);
msg_puts("\n"); 
verbose_leave_scroll();
}
retval = vim_strsave(ac->cmd);

if (ac->once) {
au_del_cmd(ac);
}
autocmd_nested = ac->nested;
current_sctx = ac->script_ctx;
if (ac->last) {
acp->nextcmd = NULL;
} else {
acp->nextcmd = ac->next;
}

return retval;
}








bool has_autocmd(event_T event, char_u *sfname, buf_T *buf)
FUNC_ATTR_WARN_UNUSED_RESULT
{
AutoPat *ap;
char_u *fname;
char_u *tail = path_tail(sfname);
bool retval = false;

fname = (char_u *)FullName_save((char *)sfname, false);
if (fname == NULL) {
return false;
}

#if defined(BACKSLASH_IN_FILENAME)


sfname = vim_strsave(sfname);
forward_slash(sfname);
forward_slash(fname);
#endif

for (ap = first_autopat[(int)event]; ap != NULL; ap = ap->next) {
if (ap->pat != NULL && ap->cmds != NULL
&& (ap->buflocal_nr == 0
? match_file_pat(NULL, &ap->reg_prog, fname, sfname, tail,
ap->allow_dirs)
: buf != NULL && ap->buflocal_nr == buf->b_fnum)) {
retval = true;
break;
}
}

xfree(fname);
#if defined(BACKSLASH_IN_FILENAME)
xfree(sfname);
#endif

return retval;
}





char_u *get_augroup_name(expand_T *xp, int idx)
{
if (idx == augroups.ga_len) { 
return (char_u *)"END";
}
if (idx >= augroups.ga_len) { 
return NULL;
}
if (AUGROUP_NAME(idx) == NULL || AUGROUP_NAME(idx) == get_deleted_augroup()) {

return (char_u *)"";
}
return (char_u *)AUGROUP_NAME(idx);
}

static int include_groups = FALSE;

char_u *
set_context_in_autocmd (
expand_T *xp,
char_u *arg,
int doautocmd 
)
{
char_u *p;
int group;


include_groups = FALSE;
p = arg;
group = au_get_grouparg(&arg);


if (*arg == NUL && group != AUGROUP_ALL && !ascii_iswhite(arg[-1])) {
arg = p;
group = AUGROUP_ALL;
}


for (p = arg; *p != NUL && !ascii_iswhite(*p); ++p)
if (*p == ',')
arg = p + 1;
if (*p == NUL) {
if (group == AUGROUP_ALL)
include_groups = TRUE;
xp->xp_context = EXPAND_EVENTS; 
xp->xp_pattern = arg;
return NULL;
}


arg = skipwhite(p);
while (*arg && (!ascii_iswhite(*arg) || arg[-1] == '\\'))
arg++;
if (*arg)
return arg; 

if (doautocmd)
xp->xp_context = EXPAND_FILES; 
else
xp->xp_context = EXPAND_NOTHING; 
return NULL;
}




char_u *get_event_name(expand_T *xp, int idx)
{
if (idx < augroups.ga_len) { 
if (!include_groups || AUGROUP_NAME(idx) == NULL
|| AUGROUP_NAME(idx) == get_deleted_augroup()) {
return (char_u *)""; 
}
return (char_u *)AUGROUP_NAME(idx);
}
return (char_u *)event_names[idx - augroups.ga_len].name;
}







bool autocmd_supported(const char *const event)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
char_u *p;
return event_name2nr((const char_u *)event, &p) != NUM_EVENTS;
}













bool au_exists(const char *const arg) FUNC_ATTR_WARN_UNUSED_RESULT
{
event_T event;
AutoPat *ap;
buf_T *buflocal_buf = NULL;
int group;
bool retval = false;


char *const arg_save = xstrdup(arg);
char *p = strchr(arg_save, '#');
if (p != NULL) {
*p++ = NUL;
}


group = au_find_group((char_u *)arg_save);
char *event_name;
if (group == AUGROUP_ERROR) {

group = AUGROUP_ALL;
event_name = arg_save;
} else {
if (p == NULL) {

retval = true;
goto theend;
}


event_name = p;
p = strchr(event_name, '#');
if (p != NULL) {
*p++ = NUL; 
}
}

char *pattern = p; 


event = event_name2nr((char_u *)event_name, (char_u **)&p);


if (event == NUM_EVENTS)
goto theend;




ap = first_autopat[(int)event];
if (ap == NULL)
goto theend;



if (pattern != NULL && STRICMP(pattern, "<buffer>") == 0)
buflocal_buf = curbuf;


for (; ap != NULL; ap = ap->next)


if (ap->pat != NULL && ap->cmds != NULL
&& (group == AUGROUP_ALL || ap->group == group)
&& (pattern == NULL
|| (buflocal_buf == NULL
? fnamecmp(ap->pat, (char_u *)pattern) == 0
: ap->buflocal_nr == buflocal_buf->b_fnum))) {
retval = true;
break;
}

theend:
xfree(arg_save);
return retval;
}















static bool match_file_pat(char_u *pattern, regprog_T **prog, char_u *fname,
char_u *sfname, char_u *tail, int allow_dirs)
{
regmatch_T regmatch;
bool result = false;

regmatch.rm_ic = p_fic; 
{
if (prog != NULL)
regmatch.regprog = *prog;
else
regmatch.regprog = vim_regcomp(pattern, RE_MAGIC);
}







if (regmatch.regprog != NULL
&& ((allow_dirs
&& (vim_regexec(&regmatch, fname, (colnr_T)0)
|| (sfname != NULL
&& vim_regexec(&regmatch, sfname, (colnr_T)0))))
|| (!allow_dirs && vim_regexec(&regmatch, tail, (colnr_T)0)))) {
result = true;
}

if (prog != NULL) {
*prog = regmatch.regprog;
} else {
vim_regfree(regmatch.regprog);
}
return result;
}










bool match_file_list(char_u *list, char_u *sfname, char_u *ffname)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ARG(1, 3)
{
char_u buf[100];
char_u *tail;
char_u *regpat;
char allow_dirs;
bool match;
char_u *p;

tail = path_tail(sfname);


p = list;
while (*p) {
copy_option_part(&p, buf, ARRAY_SIZE(buf), ",");
regpat = file_pat_to_reg_pat(buf, NULL, &allow_dirs, false);
if (regpat == NULL) {
break;
}
match = match_file_pat(regpat, NULL, ffname, sfname, tail, (int)allow_dirs);
xfree(regpat);
if (match) {
return true;
}
}
return false;
}








char_u * file_pat_to_reg_pat(
const char_u *pat,
const char_u *pat_end, 
char *allow_dirs, 
int no_bslash 
)
FUNC_ATTR_NONNULL_ARG(1)
{
const char_u *endp;
char_u *reg_pat;
const char_u *p;
int nested = 0;
int add_dollar = TRUE;

if (allow_dirs != NULL)
*allow_dirs = FALSE;
if (pat_end == NULL)
pat_end = pat + STRLEN(pat);

if (pat_end == pat) {
return (char_u *)xstrdup("^$");
}

size_t size = 2; 

for (p = pat; p < pat_end; p++) {
switch (*p) {
case '*':
case '.':
case ',':
case '{':
case '}':
case '~':
size += 2; 
break;
#if defined(BACKSLASH_IN_FILENAME)
case '\\':
case '/':
size += 4; 
break;
#endif
default:
size++;
break;
}
}
reg_pat = xmalloc(size + 1);

size_t i = 0;

if (pat[0] == '*')
while (pat[0] == '*' && pat < pat_end - 1)
pat++;
else
reg_pat[i++] = '^';
endp = pat_end - 1;
if (endp >= pat && *endp == '*') {
while (endp - pat > 0 && *endp == '*') {
endp--;
}
add_dollar = false;
}
for (p = pat; *p && nested >= 0 && p <= endp; p++) {
switch (*p) {
case '*':
reg_pat[i++] = '.';
reg_pat[i++] = '*';
while (p[1] == '*') 
++p;
break;
case '.':
case '~':
reg_pat[i++] = '\\';
reg_pat[i++] = *p;
break;
case '?':
reg_pat[i++] = '.';
break;
case '\\':
if (p[1] == NUL)
break;
#if defined(BACKSLASH_IN_FILENAME)
if (!no_bslash) {






if ((vim_isfilec(p[1]) || p[1] == '*' || p[1] == '?')
&& p[1] != '+') {
reg_pat[i++] = '[';
reg_pat[i++] = '\\';
reg_pat[i++] = '/';
reg_pat[i++] = ']';
if (allow_dirs != NULL)
*allow_dirs = TRUE;
break;
}
}
#endif










if (*++p == '?'
#if defined(BACKSLASH_IN_FILENAME)
&& no_bslash
#endif
) {
reg_pat[i++] = '?';
} else if (*p == ',' || *p == '%' || *p == '#'
|| ascii_isspace(*p) || *p == '{' || *p == '}') {
reg_pat[i++] = *p;
} else if (*p == '\\' && p[1] == '\\' && p[2] == '{') {
reg_pat[i++] = '\\';
reg_pat[i++] = '{';
p += 2;
} else {
if (allow_dirs != NULL && vim_ispathsep(*p)
#if defined(BACKSLASH_IN_FILENAME)
&& (!no_bslash || *p != '\\')
#endif
)
*allow_dirs = TRUE;
reg_pat[i++] = '\\';
reg_pat[i++] = *p;
}
break;
#if defined(BACKSLASH_IN_FILENAME)
case '/':
reg_pat[i++] = '[';
reg_pat[i++] = '\\';
reg_pat[i++] = '/';
reg_pat[i++] = ']';
if (allow_dirs != NULL)
*allow_dirs = TRUE;
break;
#endif
case '{':
reg_pat[i++] = '\\';
reg_pat[i++] = '(';
nested++;
break;
case '}':
reg_pat[i++] = '\\';
reg_pat[i++] = ')';
--nested;
break;
case ',':
if (nested) {
reg_pat[i++] = '\\';
reg_pat[i++] = '|';
} else
reg_pat[i++] = ',';
break;
default:
if (allow_dirs != NULL && vim_ispathsep(*p)) {
*allow_dirs = true;
}
reg_pat[i++] = *p;
break;
}
}
if (add_dollar)
reg_pat[i++] = '$';
reg_pat[i] = NUL;
if (nested != 0) {
if (nested < 0) {
EMSG(_("E219: Missing {."));
} else {
EMSG(_("E220: Missing }."));
}
XFREE_CLEAR(reg_pat);
}
return reg_pat;
}

#if defined(EINTR)




long read_eintr(int fd, void *buf, size_t bufsize)
{
long ret;

for (;; ) {
ret = read(fd, buf, bufsize);
if (ret >= 0 || errno != EINTR)
break;
}
return ret;
}





long write_eintr(int fd, void *buf, size_t bufsize)
{
long ret = 0;
long wlen;



while (ret < (long)bufsize) {
wlen = write(fd, (char *)buf + ret, bufsize - ret);
if (wlen < 0) {
if (errno != EINTR)
break;
} else
ret += wlen;
}
return ret;
}
#endif
