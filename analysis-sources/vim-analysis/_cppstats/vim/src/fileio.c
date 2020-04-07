#include "vim.h"
#if defined(__TANDEM) || defined(__MINT__)
#include <limits.h> 
#endif
#define USE_MCH_ACCESS
static char_u *next_fenc(char_u **pp, int *alloced);
#if defined(FEAT_EVAL)
static char_u *readfile_charconvert(char_u *fname, char_u *fenc, int *fdp);
#endif
#if defined(FEAT_CRYPT)
static char_u *check_for_cryptkey(char_u *cryptkey, char_u *ptr, long *sizep, off_T *filesizep, int newfile, char_u *fname, int *did_ask);
#endif
static linenr_T readfile_linenr(linenr_T linecnt, char_u *p, char_u *endp);
static char_u *check_for_bom(char_u *p, long size, int *lenp, int flags);
static char *e_auchangedbuf = N_("E812: Autocommands changed buffer or buffer name");
void
filemess(
buf_T *buf,
char_u *name,
char_u *s,
int attr)
{
int msg_scroll_save;
int prev_msg_col = msg_col;
if (msg_silent != 0)
return;
msg_add_fname(buf, name); 
if (STRLEN(IObuff) > IOSIZE - 80)
IObuff[IOSIZE - 80] = NUL;
STRCAT(IObuff, s);
msg_scroll_save = msg_scroll;
if (shortmess(SHM_OVERALL) && !exiting && p_verbose == 0)
msg_scroll = FALSE;
if (!msg_scroll) 
check_for_delay(FALSE);
msg_start();
if (prev_msg_col != 0 && msg_col == 0)
msg_putchar('\r'); 
msg_scroll = msg_scroll_save;
msg_scrolled_ign = TRUE;
msg_outtrans_attr(msg_may_trunc(FALSE, IObuff), attr);
msg_clr_eos();
out_flush();
msg_scrolled_ign = FALSE;
}
int
readfile(
char_u *fname,
char_u *sfname,
linenr_T from,
linenr_T lines_to_skip,
linenr_T lines_to_read,
exarg_T *eap, 
int flags)
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
char_u *p;
off_T filesize = 0;
int skip_read = FALSE;
#if defined(FEAT_CRYPT)
char_u *cryptkey = NULL;
int did_ask_for_key = FALSE;
#endif
#if defined(FEAT_PERSISTENT_UNDO)
context_sha256_T sha_ctx;
int read_undo_file = FALSE;
#endif
int split = 0; 
#define UNKNOWN 0x0fffffff 
linenr_T linecnt;
int error = FALSE; 
int ff_error = EOL_UNKNOWN; 
long linerest = 0; 
#if defined(UNIX)
int perm = 0;
int swap_mode = -1; 
#else
int perm;
#endif
int fileformat = 0; 
int keep_fileformat = FALSE;
stat_T st;
int file_readonly;
linenr_T skip_count = 0;
linenr_T read_count = 0;
int msg_save = msg_scroll;
linenr_T read_no_eol_lnum = 0; 
int try_mac;
int try_dos;
int try_unix;
int file_rewind = FALSE;
int can_retry;
linenr_T conv_error = 0; 
linenr_T illegal_byte = 0; 
int keep_dest_enc = FALSE; 
int bad_char_behavior = BAD_REPLACE;
char_u *tmpname = NULL; 
int fio_flags = 0;
char_u *fenc; 
int fenc_alloced; 
char_u *fenc_next = NULL; 
int advance_fenc = FALSE;
long real_size = 0;
#if defined(USE_ICONV)
iconv_t iconv_fd = (iconv_t)-1; 
#if defined(FEAT_EVAL)
int did_iconv = FALSE; 
#endif
#endif
int converted = FALSE; 
int notconverted = FALSE; 
char_u conv_rest[CONV_RESTLEN];
int conv_restlen = 0; 
pos_T orig_start;
buf_T *old_curbuf;
char_u *old_b_ffname;
char_u *old_b_fname;
int using_b_ffname;
int using_b_fname;
au_did_filetype = FALSE; 
curbuf->b_no_eol_lnum = 0; 
if (curbuf->b_ffname == NULL
&& !filtering
&& fname != NULL
&& vim_strchr(p_cpo, CPO_FNAMER) != NULL
&& !(flags & READ_DUMMY))
{
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
if (sfname == NULL)
sfname = fname;
#if defined(UNIX)
fname = sfname;
#endif
if (!filtering && !read_stdin && !read_buffer)
{
orig_start = curbuf->b_op_start;
curbuf->b_op_start.lnum = ((from == 0) ? 1 : from);
curbuf->b_op_start.col = 0;
if (newfile)
{
if (apply_autocmds_exarg(EVENT_BUFREADCMD, NULL, sfname,
FALSE, curbuf, eap))
{
int status = OK;
#if defined(FEAT_EVAL)
if (aborting())
status = FAIL;
#endif
if (status == OK)
curbuf->b_flags &= ~BF_NOTEDITED;
return status;
}
}
else if (apply_autocmds_exarg(EVENT_FILEREADCMD, sfname, sfname,
FALSE, NULL, eap))
#if defined(FEAT_EVAL)
return aborting() ? FAIL : OK;
#else
return OK;
#endif
curbuf->b_op_start = orig_start;
}
if ((shortmess(SHM_OVER) || curbuf->b_help) && p_verbose == 0)
msg_scroll = FALSE; 
else
msg_scroll = TRUE; 
if (fname != NULL && *fname != NUL)
{
p = fname + STRLEN(fname);
if (after_pathsep(fname, p) || STRLEN(fname) >= MAXPATHL)
{
filemess(curbuf, fname, (char_u *)_("Illegal file name"), 0);
msg_end();
msg_scroll = msg_save;
return FAIL;
}
}
if (!read_stdin && !read_buffer && !read_fifo)
{
#if defined(UNIX)
perm = mch_getperm(fname);
if (perm >= 0 && !S_ISREG(perm) 
&& !S_ISFIFO(perm) 
&& !S_ISSOCK(perm) 
#if defined(OPEN_CHR_FILES)
&& !(S_ISCHR(perm) && is_dev_fd_file(fname))
#endif
)
{
int retval = FAIL;
if (S_ISDIR(perm))
{
filemess(curbuf, fname, (char_u *)_("is a directory"), 0);
retval = NOTDONE;
}
else
filemess(curbuf, fname, (char_u *)_("is not a file"), 0);
msg_end();
msg_scroll = msg_save;
return retval;
}
#endif
#if defined(MSWIN)
if (!p_odev && mch_nodetype(fname) == NODE_WRITABLE)
{
filemess(curbuf, fname, (char_u *)_("is a device (disabled with 'opendevice' option)"), 0);
msg_end();
msg_scroll = msg_save;
return FAIL;
}
#endif
}
set_file_options(set_options, eap);
check_readonly = (newfile && (curbuf->b_flags & BF_CHECK_RO));
if (check_readonly && !readonlymode)
curbuf->b_p_ro = FALSE;
if (newfile && !read_stdin && !read_buffer && !read_fifo)
{
if (mch_stat((char *)fname, &st) >= 0)
{
buf_store_time(curbuf, &st, fname);
curbuf->b_mtime_read = curbuf->b_mtime;
#if defined(UNIX)
swap_mode = (st.st_mode & 0644) | 0600;
#endif
#if defined(FEAT_CW_EDITOR)
(void)GetFSSpecFromPath(curbuf->b_ffname, &curbuf->b_FSSpec);
#endif
#if defined(VMS)
curbuf->b_fab_rfm = st.st_fab_rfm;
curbuf->b_fab_rat = st.st_fab_rat;
curbuf->b_fab_mrs = st.st_fab_mrs;
#endif
}
else
{
curbuf->b_mtime = 0;
curbuf->b_mtime_read = 0;
curbuf->b_orig_size = 0;
curbuf->b_orig_mode = 0;
}
curbuf->b_flags &= ~(BF_NEW | BF_NEW_W);
}
file_readonly = FALSE;
if (read_stdin)
{
#if defined(MSWIN)
setmode(0, O_BINARY);
#endif
}
else if (!read_buffer)
{
#if defined(USE_MCH_ACCESS)
if (
#if defined(UNIX)
!(perm & 0222) ||
#endif
mch_access((char *)fname, W_OK))
file_readonly = TRUE;
fd = mch_open((char *)fname, O_RDONLY | O_EXTRA, 0);
#else
if (!newfile
|| readonlymode
|| (fd = mch_open((char *)fname, O_RDWR | O_EXTRA, 0)) < 0)
{
file_readonly = TRUE;
fd = mch_open((char *)fname, O_RDONLY | O_EXTRA, 0);
}
#endif
}
if (fd < 0) 
{
#if !defined(UNIX)
int isdir_f;
#endif
msg_scroll = msg_save;
#if !defined(UNIX)
isdir_f = (mch_isdir(fname));
perm = mch_getperm(fname); 
if (isdir_f)
{
filemess(curbuf, sfname, (char_u *)_("is a directory"), 0);
curbuf->b_p_ro = TRUE; 
}
else
#endif
if (newfile)
{
if (perm < 0
#if defined(ENOENT)
&& errno == ENOENT
#endif
)
{
curbuf->b_flags |= BF_NEW;
#if defined(FEAT_QUICKFIX)
if (!bt_dontwrite(curbuf))
#endif
{
check_need_swap(newfile);
if (curbuf != old_curbuf
|| (using_b_ffname
&& (old_b_ffname != curbuf->b_ffname))
|| (using_b_fname
&& (old_b_fname != curbuf->b_fname)))
{
emsg(_(e_auchangedbuf));
return FAIL;
}
}
if (dir_of_file_exists(fname))
filemess(curbuf, sfname, (char_u *)_("[New File]"), 0);
else
filemess(curbuf, sfname,
(char_u *)_("[New DIRECTORY]"), 0);
#if defined(FEAT_VIMINFO)
check_marks_read();
#endif
if (eap != NULL)
set_forced_fenc(eap);
apply_autocmds_exarg(EVENT_BUFNEWFILE, sfname, sfname,
FALSE, curbuf, eap);
save_file_ff(curbuf);
#if defined(FEAT_EVAL)
if (aborting()) 
return FAIL;
#endif
return OK; 
}
else
{
filemess(curbuf, sfname, (char_u *)(
#if defined(EFBIG)
(errno == EFBIG) ? _("[File too big]") :
#endif
#if defined(EOVERFLOW)
(errno == EOVERFLOW) ? _("[File too big]") :
#endif
_("[Permission Denied]")), 0);
curbuf->b_p_ro = TRUE; 
}
}
return FAIL;
}
if ((check_readonly && file_readonly) || curbuf->b_help)
curbuf->b_p_ro = TRUE;
if (set_options)
{
if (!read_buffer)
{
curbuf->b_p_eol = TRUE;
curbuf->b_start_eol = TRUE;
}
curbuf->b_p_bomb = FALSE;
curbuf->b_start_bomb = FALSE;
}
#if defined(FEAT_QUICKFIX)
if (!bt_dontwrite(curbuf))
#endif
{
check_need_swap(newfile);
if (!read_stdin && (curbuf != old_curbuf
|| (using_b_ffname && (old_b_ffname != curbuf->b_ffname))
|| (using_b_fname && (old_b_fname != curbuf->b_fname))))
{
emsg(_(e_auchangedbuf));
if (!read_buffer)
close(fd);
return FAIL;
}
#if defined(UNIX)
if (swap_mode > 0 && curbuf->b_ml.ml_mfp != NULL
&& curbuf->b_ml.ml_mfp->mf_fname != NULL)
{
char_u *swap_fname = curbuf->b_ml.ml_mfp->mf_fname;
if ((swap_mode & 044) == 040)
{
stat_T swap_st;
if (mch_stat((char *)swap_fname, &swap_st) >= 0
&& st.st_gid != swap_st.st_gid
#if defined(HAVE_FCHOWN)
&& fchown(curbuf->b_ml.ml_mfp->mf_fd, -1, st.st_gid)
== -1
#endif
)
swap_mode &= 0600;
}
(void)mch_setperm(swap_fname, (long)swap_mode);
}
#endif
}
if (swap_exists_action == SEA_QUIT)
{
if (!read_buffer && !read_stdin)
close(fd);
return FAIL;
}
++no_wait_return; 
orig_start = curbuf->b_op_start;
curbuf->b_op_start.lnum = ((from == 0) ? 1 : from);
curbuf->b_op_start.col = 0;
try_mac = (vim_strchr(p_ffs, 'm') != NULL);
try_dos = (vim_strchr(p_ffs, 'd') != NULL);
try_unix = (vim_strchr(p_ffs, 'x') != NULL);
if (!read_buffer)
{
int m = msg_scroll;
int n = msg_scrolled;
if (!read_stdin)
close(fd); 
msg_scroll = TRUE;
if (filtering)
apply_autocmds_exarg(EVENT_FILTERREADPRE, NULL, sfname,
FALSE, curbuf, eap);
else if (read_stdin)
apply_autocmds_exarg(EVENT_STDINREADPRE, NULL, sfname,
FALSE, curbuf, eap);
else if (newfile)
apply_autocmds_exarg(EVENT_BUFREADPRE, NULL, sfname,
FALSE, curbuf, eap);
else
apply_autocmds_exarg(EVENT_FILEREADPRE, sfname, sfname,
FALSE, NULL, eap);
try_mac = (vim_strchr(p_ffs, 'm') != NULL);
try_dos = (vim_strchr(p_ffs, 'd') != NULL);
try_unix = (vim_strchr(p_ffs, 'x') != NULL);
curbuf->b_op_start = orig_start;
if (msg_scrolled == n)
msg_scroll = m;
#if defined(FEAT_EVAL)
if (aborting()) 
{
--no_wait_return;
msg_scroll = msg_save;
curbuf->b_p_ro = TRUE; 
return FAIL;
}
#endif
if (!read_stdin && (curbuf != old_curbuf
|| (using_b_ffname && (old_b_ffname != curbuf->b_ffname))
|| (using_b_fname && (old_b_fname != curbuf->b_fname))
|| (fd = mch_open((char *)fname, O_RDONLY | O_EXTRA, 0)) < 0))
{
--no_wait_return;
msg_scroll = msg_save;
if (fd < 0)
emsg(_("E200: *ReadPre autocommands made the file unreadable"));
else
emsg(_("E201: *ReadPre autocommands must not change current buffer"));
curbuf->b_p_ro = TRUE; 
return FAIL;
}
}
wasempty = (curbuf->b_ml.ml_flags & ML_EMPTY);
if (!recoverymode && !filtering && !(flags & READ_DUMMY))
{
if (read_stdin)
{
if (!is_not_a_term())
{
#if !defined(ALWAYS_USE_GUI)
#if defined(VIMDLL)
if (!gui.in_use)
#endif
mch_msg(_("Vim: Reading from stdin...\n"));
#endif
#if defined(FEAT_GUI)
if (gui.in_use && !gui.dying && !gui.starting)
{
p = (char_u *)_("Reading from stdin...");
gui_write(p, (int)STRLEN(p));
}
#endif
}
}
else if (!read_buffer)
filemess(curbuf, sfname, (char_u *)"", 0);
}
msg_scroll = FALSE; 
linecnt = curbuf->b_ml.ml_line_count;
if (eap != NULL && eap->bad_char != 0)
{
bad_char_behavior = eap->bad_char;
if (set_options)
curbuf->b_bad_char = eap->bad_char;
}
else
curbuf->b_bad_char = 0;
if (eap != NULL && eap->force_enc != 0)
{
fenc = enc_canonize(eap->cmd + eap->force_enc);
fenc_alloced = TRUE;
keep_dest_enc = TRUE;
}
else if (curbuf->b_p_bin)
{
fenc = (char_u *)""; 
fenc_alloced = FALSE;
}
else if (curbuf->b_help)
{
char_u firstline[80];
int fc;
fenc = (char_u *)"latin1";
c = enc_utf8;
if (!c && !read_stdin)
{
fc = fname[STRLEN(fname) - 1];
if (TOLOWER_ASC(fc) == 'x')
{
len = read_eintr(fd, firstline, 80);
vim_lseek(fd, (off_T)0L, SEEK_SET);
for (p = firstline; p < firstline + len; ++p)
if (*p >= 0x80)
{
c = TRUE;
break;
}
}
}
if (c)
{
fenc_next = fenc;
fenc = (char_u *)"utf-8";
if (!enc_utf8)
keep_dest_enc = TRUE;
}
fenc_alloced = FALSE;
}
else if (*p_fencs == NUL)
{
fenc = curbuf->b_p_fenc; 
fenc_alloced = FALSE;
}
else
{
fenc_next = p_fencs; 
fenc = next_fenc(&fenc_next, &fenc_alloced);
}
retry:
if (file_rewind)
{
if (read_buffer)
{
read_buf_lnum = 1;
read_buf_col = 0;
}
else if (read_stdin || vim_lseek(fd, (off_T)0L, SEEK_SET) != 0)
{
error = TRUE;
goto failed;
}
while (lnum > from)
ml_delete(lnum--, FALSE);
file_rewind = FALSE;
if (set_options)
{
curbuf->b_p_bomb = FALSE;
curbuf->b_start_bomb = FALSE;
}
conv_error = 0;
}
if (keep_fileformat)
keep_fileformat = FALSE;
else
{
if (eap != NULL && eap->force_ff != 0)
{
fileformat = get_fileformat_force(curbuf, eap);
try_unix = try_dos = try_mac = FALSE;
}
else if (curbuf->b_p_bin)
fileformat = EOL_UNIX; 
else if (*p_ffs == NUL)
fileformat = get_fileformat(curbuf);
else
fileformat = EOL_UNKNOWN; 
}
#if defined(USE_ICONV)
if (iconv_fd != (iconv_t)-1)
{
iconv_close(iconv_fd);
iconv_fd = (iconv_t)-1;
}
#endif
if (advance_fenc)
{
advance_fenc = FALSE;
if (eap != NULL && eap->force_enc != 0)
{
notconverted = TRUE;
conv_error = 0;
if (fenc_alloced)
vim_free(fenc);
fenc = (char_u *)"";
fenc_alloced = FALSE;
}
else
{
if (fenc_alloced)
vim_free(fenc);
if (fenc_next != NULL)
{
fenc = next_fenc(&fenc_next, &fenc_alloced);
}
else
{
fenc = (char_u *)"";
fenc_alloced = FALSE;
}
}
if (tmpname != NULL)
{
mch_remove(tmpname); 
VIM_CLEAR(tmpname);
}
}
fio_flags = 0;
converted = need_conversion(fenc);
if (converted)
{
if (STRCMP(fenc, ENC_UCSBOM) == 0)
fio_flags = FIO_UCSBOM;
else if (enc_utf8 || STRCMP(p_enc, "latin1") == 0)
fio_flags = get_fio_flags(fenc);
#if defined(MSWIN)
if (fio_flags == 0)
fio_flags = get_win_fio_flags(fenc);
#endif
#if defined(MACOS_CONVERT)
if (fio_flags == 0)
fio_flags = get_mac_fio_flags(fenc);
#endif
#if defined(USE_ICONV)
if (fio_flags == 0
#if defined(FEAT_EVAL)
&& !did_iconv
#endif
)
iconv_fd = (iconv_t)my_iconv_open(
enc_utf8 ? (char_u *)"utf-8" : p_enc, fenc);
#endif
#if defined(FEAT_EVAL)
if (fio_flags == 0 && !read_stdin && !read_buffer && *p_ccv != NUL
&& !read_fifo
#if defined(USE_ICONV)
&& iconv_fd == (iconv_t)-1
#endif
)
{
#if defined(USE_ICONV)
did_iconv = FALSE;
#endif
if (tmpname == NULL)
{
tmpname = readfile_charconvert(fname, fenc, &fd);
if (tmpname == NULL)
{
advance_fenc = TRUE;
if (fd < 0)
{
emsg(_("E202: Conversion made file unreadable!"));
error = TRUE;
goto failed;
}
goto retry;
}
}
}
else
#endif
{
if (fio_flags == 0
#if defined(USE_ICONV)
&& iconv_fd == (iconv_t)-1
#endif
)
{
advance_fenc = TRUE;
goto retry;
}
}
}
can_retry = (*fenc != NUL && !read_stdin && !read_fifo && !keep_dest_enc);
if (!skip_read)
{
linerest = 0;
filesize = 0;
skip_count = lines_to_skip;
read_count = lines_to_read;
conv_restlen = 0;
#if defined(FEAT_PERSISTENT_UNDO)
read_undo_file = (newfile && (flags & READ_KEEP_UNDO) == 0
&& curbuf->b_ffname != NULL
&& curbuf->b_p_udf
&& !filtering
&& !read_fifo
&& !read_stdin
&& !read_buffer);
if (read_undo_file)
sha256_start(&sha_ctx);
#endif
#if defined(FEAT_CRYPT)
if (curbuf->b_cryptstate != NULL)
{
crypt_free_state(curbuf->b_cryptstate);
curbuf->b_cryptstate = NULL;
}
#endif
}
while (!error && !got_int)
{
if (!skip_read)
{
#if defined(SSIZE_MAX) && (SSIZE_MAX < 0x10000L)
size = SSIZE_MAX; 
#else
size = 0x10000L + linerest;
if (size > 0x100000L)
size = 0x100000L;
#endif
}
if (size < 0 || size + linerest + 1 < 0 || linerest >= MAXCOL)
{
++split;
*ptr = NL; 
size = 1;
}
else
{
if (!skip_read)
{
for ( ; size >= 10; size = (long)((long_u)size >> 1))
{
if ((new_buffer = lalloc(size + linerest + 1,
FALSE)) != NULL)
break;
}
if (new_buffer == NULL)
{
do_outofmem_msg((long_u)(size * 2 + linerest + 1));
error = TRUE;
break;
}
if (linerest) 
mch_memmove(new_buffer, ptr - linerest, (size_t)linerest);
vim_free(buffer);
buffer = new_buffer;
ptr = buffer + linerest;
line_start = buffer;
real_size = (int)size;
#if defined(USE_ICONV)
if (iconv_fd != (iconv_t)-1)
size = size / ICONV_MULT;
else
#endif
if (fio_flags & FIO_LATIN1)
size = size / 2;
else if (fio_flags & (FIO_UCS2 | FIO_UTF16))
size = (size * 2 / 3) & ~1;
else if (fio_flags & FIO_UCS4)
size = (size * 2 / 3) & ~3;
else if (fio_flags == FIO_UCSBOM)
size = size / ICONV_MULT; 
#if defined(MSWIN)
else if (fio_flags & FIO_CODEPAGE)
size = size / ICONV_MULT; 
#endif
#if defined(MACOS_CONVERT)
else if (fio_flags & FIO_MACROMAN)
size = size / ICONV_MULT; 
#endif
if (conv_restlen > 0)
{
mch_memmove(ptr, conv_rest, conv_restlen);
ptr += conv_restlen;
size -= conv_restlen;
}
if (read_buffer)
{
if (read_buf_lnum > from)
size = 0;
else
{
int n, ni;
long tlen;
tlen = 0;
for (;;)
{
p = ml_get(read_buf_lnum) + read_buf_col;
n = (int)STRLEN(p);
if ((int)tlen + n + 1 > size)
{
n = (int)(size - tlen);
for (ni = 0; ni < n; ++ni)
{
if (p[ni] == NL)
ptr[tlen++] = NUL;
else
ptr[tlen++] = p[ni];
}
read_buf_col += n;
break;
}
else
{
for (ni = 0; ni < n; ++ni)
{
if (p[ni] == NL)
ptr[tlen++] = NUL;
else
ptr[tlen++] = p[ni];
}
ptr[tlen++] = NL;
read_buf_col = 0;
if (++read_buf_lnum > from)
{
if (!curbuf->b_p_eol)
--tlen;
size = tlen;
break;
}
}
}
}
}
else
{
size = read_eintr(fd, ptr, size);
}
#if defined(FEAT_CRYPT)
if (filesize == 0 && size > 0)
cryptkey = check_for_cryptkey(cryptkey, ptr, &size,
&filesize, newfile, sfname,
&did_ask_for_key);
if (cryptkey != NULL && curbuf->b_cryptstate != NULL
&& size > 0)
{
#if defined(CRYPT_NOT_INPLACE)
if (crypt_works_inplace(curbuf->b_cryptstate))
{
#endif
crypt_decode_inplace(curbuf->b_cryptstate, ptr, size);
#if defined(CRYPT_NOT_INPLACE)
}
else
{
char_u *newptr = NULL;
int decrypted_size;
decrypted_size = crypt_decode_alloc(
curbuf->b_cryptstate, ptr, size, &newptr);
if (decrypted_size == 0)
continue;
if (linerest == 0)
{
new_buffer = newptr;
}
else
{
long_u new_size;
new_size = (long_u)(decrypted_size + linerest + 1);
new_buffer = lalloc(new_size, FALSE);
if (new_buffer == NULL)
{
do_outofmem_msg(new_size);
error = TRUE;
break;
}
mch_memmove(new_buffer, buffer, linerest);
if (newptr != NULL)
mch_memmove(new_buffer + linerest, newptr,
decrypted_size);
}
if (new_buffer != NULL)
{
vim_free(buffer);
buffer = new_buffer;
new_buffer = NULL;
line_start = buffer;
ptr = buffer + linerest;
}
size = decrypted_size;
}
#endif
}
#endif
if (size <= 0)
{
if (size < 0) 
error = TRUE;
else if (conv_restlen > 0)
{
if (fio_flags != 0
#if defined(USE_ICONV)
|| iconv_fd != (iconv_t)-1
#endif
)
{
if (can_retry)
goto rewind_retry;
if (conv_error == 0)
conv_error = curbuf->b_ml.ml_line_count
- linecnt + 1;
}
else if (illegal_byte == 0)
illegal_byte = curbuf->b_ml.ml_line_count
- linecnt + 1;
if (bad_char_behavior == BAD_DROP)
{
*(ptr - conv_restlen) = NUL;
conv_restlen = 0;
}
else
{
if (bad_char_behavior != BAD_KEEP && (fio_flags != 0
#if defined(USE_ICONV)
|| iconv_fd != (iconv_t)-1
#endif
))
{
while (conv_restlen > 0)
{
*(--ptr) = bad_char_behavior;
--conv_restlen;
}
}
fio_flags = 0; 
#if defined(USE_ICONV)
if (iconv_fd != (iconv_t)-1)
{
iconv_close(iconv_fd);
iconv_fd = (iconv_t)-1;
}
#endif
}
}
}
}
skip_read = FALSE;
if ((filesize == 0
#if defined(FEAT_CRYPT)
|| (cryptkey != NULL
&& filesize == crypt_get_header_len(
crypt_get_method_nr(curbuf)))
#endif
)
&& (fio_flags == FIO_UCSBOM
|| (!curbuf->b_p_bomb
&& tmpname == NULL
&& (*fenc == 'u' || (*fenc == NUL && enc_utf8)))))
{
char_u *ccname;
int blen;
if (size < 2 || curbuf->b_p_bin)
ccname = NULL;
else
ccname = check_for_bom(ptr, size, &blen,
fio_flags == FIO_UCSBOM ? FIO_ALL : get_fio_flags(fenc));
if (ccname != NULL)
{
filesize += blen;
size -= blen;
mch_memmove(ptr, ptr + blen, (size_t)size);
if (set_options)
{
curbuf->b_p_bomb = TRUE;
curbuf->b_start_bomb = TRUE;
}
}
if (fio_flags == FIO_UCSBOM)
{
if (ccname == NULL)
{
advance_fenc = TRUE;
}
else
{
if (fenc_alloced)
vim_free(fenc);
fenc = ccname;
fenc_alloced = FALSE;
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
#if defined(USE_ICONV)
if (iconv_fd != (iconv_t)-1)
{
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
|| from_size > CONV_RESTLEN)
{
if (can_retry)
goto rewind_retry;
if (conv_error == 0)
conv_error = readfile_linenr(linecnt,
ptr, (char_u *)top);
++fromp;
--from_size;
if (bad_char_behavior == BAD_KEEP)
{
*top++ = *(fromp - 1);
--to_size;
}
else if (bad_char_behavior != BAD_DROP)
{
*top++ = bad_char_behavior;
--to_size;
}
}
if (from_size > 0)
{
mch_memmove(conv_rest, (char_u *)fromp, from_size);
conv_restlen = (int)from_size;
}
line_start = ptr - linerest;
mch_memmove(line_start, buffer, (size_t)linerest);
size = (long)((char_u *)top - ptr);
}
#endif
#if defined(MSWIN)
if (fio_flags & FIO_CODEPAGE)
{
char_u *src, *dst;
WCHAR ucs2buf[3];
int ucs2len;
int codepage = FIO_GET_CP(fio_flags);
int bytelen;
int found_bad;
char replstr[2];
if (bad_char_behavior > 0)
replstr[0] = bad_char_behavior;
else
replstr[0] = '?';
replstr[1] = NUL;
src = ptr + real_size - size;
mch_memmove(src, ptr, size);
dst = ptr;
size = size;
while (size > 0)
{
found_bad = FALSE;
#if defined(CP_UTF8)
if (codepage == CP_UTF8)
{
bytelen = (int)utf_ptr2len_len(src, size);
if (bytelen > size)
{
if (bytelen <= CONV_RESTLEN)
break;
bytelen = size;
found_bad = TRUE;
}
else
{
int u8c = utf_ptr2char(src);
if (u8c > 0xffff || (*src >= 0x80 && bytelen == 1))
found_bad = TRUE;
ucs2buf[0] = u8c;
ucs2len = 1;
}
}
else
#endif
{
for (bytelen = 1; bytelen <= size && bytelen <= 3;
++bytelen)
{
ucs2len = MultiByteToWideChar(codepage,
MB_ERR_INVALID_CHARS,
(LPCSTR)src, bytelen,
ucs2buf, 3);
if (ucs2len > 0)
break;
}
if (ucs2len == 0)
{
if (size == 1)
break;
found_bad = TRUE;
bytelen = 1;
}
}
if (!found_bad)
{
int i;
if (enc_utf8)
{
for (i = 0; i < ucs2len; ++i)
dst += utf_char2bytes(ucs2buf[i], dst);
}
else
{
BOOL bad = FALSE;
int dstlen;
dstlen = WideCharToMultiByte(enc_codepage, 0,
(LPCWSTR)ucs2buf, ucs2len,
(LPSTR)dst, (int)(src - dst),
replstr, &bad);
if (bad)
found_bad = TRUE;
else
dst += dstlen;
}
}
if (found_bad)
{
if (can_retry)
goto rewind_retry;
if (conv_error == 0)
conv_error = readfile_linenr(linecnt, ptr, dst);
if (bad_char_behavior != BAD_DROP)
{
if (bad_char_behavior == BAD_KEEP)
{
mch_memmove(dst, src, bytelen);
dst += bytelen;
}
else
*dst++ = bad_char_behavior;
}
}
src += bytelen;
size -= bytelen;
}
if (size > 0)
{
mch_memmove(conv_rest, src, size);
conv_restlen = size;
}
size = (long)(dst - ptr);
}
else
#endif
#if defined(MACOS_CONVERT)
if (fio_flags & FIO_MACROMAN)
{
if (macroman2enc(ptr, &size, real_size) == FAIL)
goto rewind_retry;
}
else
#endif
if (fio_flags != 0)
{
int u8c;
char_u *dest;
char_u *tail = NULL;
dest = ptr + real_size;
if (fio_flags == FIO_LATIN1 || fio_flags == FIO_UTF8)
{
p = ptr + size;
if (fio_flags == FIO_UTF8)
{
tail = ptr + size - 1;
while (tail > ptr && (*tail & 0xc0) == 0x80)
--tail;
if (tail + utf_byte2len(*tail) <= ptr + size)
tail = NULL;
else
p = tail;
}
}
else if (fio_flags & (FIO_UCS2 | FIO_UTF16))
{
p = ptr + (size & ~1);
if (size & 1)
tail = p;
if ((fio_flags & FIO_UTF16) && p > ptr)
{
if (fio_flags & FIO_ENDIAN_L)
{
u8c = (*--p << 8);
u8c += *--p;
}
else
{
u8c = *--p;
u8c += (*--p << 8);
}
if (u8c >= 0xd800 && u8c <= 0xdbff)
tail = p;
else
p += 2;
}
}
else 
{
p = ptr + (size & ~3);
if (size & 3)
tail = p;
}
if (tail != NULL)
{
conv_restlen = (int)((ptr + size) - tail);
mch_memmove(conv_rest, (char_u *)tail, conv_restlen);
size -= conv_restlen;
}
while (p > ptr)
{
if (fio_flags & FIO_LATIN1)
u8c = *--p;
else if (fio_flags & (FIO_UCS2 | FIO_UTF16))
{
if (fio_flags & FIO_ENDIAN_L)
{
u8c = (*--p << 8);
u8c += *--p;
}
else
{
u8c = *--p;
u8c += (*--p << 8);
}
if ((fio_flags & FIO_UTF16)
&& u8c >= 0xdc00 && u8c <= 0xdfff)
{
int u16c;
if (p == ptr)
{
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
if (fio_flags & FIO_ENDIAN_L)
{
u16c = (*--p << 8);
u16c += *--p;
}
else
{
u16c = *--p;
u16c += (*--p << 8);
}
u8c = 0x10000 + ((u16c & 0x3ff) << 10)
+ (u8c & 0x3ff);
if (u16c < 0xd800 || u16c > 0xdbff)
{
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
else if (fio_flags & FIO_UCS4)
{
if (fio_flags & FIO_ENDIAN_L)
{
u8c = (unsigned)*--p << 24;
u8c += (unsigned)*--p << 16;
u8c += (unsigned)*--p << 8;
u8c += *--p;
}
else 
{
u8c = *--p;
u8c += (unsigned)*--p << 8;
u8c += (unsigned)*--p << 16;
u8c += (unsigned)*--p << 24;
}
}
else 
{
if (*--p < 0x80)
u8c = *p;
else
{
len = utf_head_off(ptr, p);
p -= len;
u8c = utf_ptr2char(p);
if (len == 0)
{
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
if (enc_utf8) 
{
dest -= utf_char2len(u8c);
(void)utf_char2bytes(u8c, dest);
}
else 
{
--dest;
if (u8c >= 0x100)
{
if (can_retry)
goto rewind_retry;
if (conv_error == 0)
conv_error = readfile_linenr(linecnt, ptr, p);
if (bad_char_behavior == BAD_DROP)
++dest;
else if (bad_char_behavior == BAD_KEEP)
*dest = u8c;
else if (eap != NULL && eap->bad_char != 0)
*dest = bad_char_behavior;
else
*dest = 0xBF;
}
else
*dest = u8c;
}
}
line_start = dest - linerest;
mch_memmove(line_start, buffer, (size_t)linerest);
size = (long)((ptr + real_size) - dest);
ptr = dest;
}
else if (enc_utf8 && !curbuf->b_p_bin)
{
int incomplete_tail = FALSE;
for (p = ptr; ; ++p)
{
int todo = (int)((ptr + size) - p);
int l;
if (todo <= 0)
break;
if (*p >= 0x80)
{
l = utf_ptr2len_len(p, todo);
if (l > todo && !incomplete_tail)
{
if (p > ptr || filesize > 0)
incomplete_tail = TRUE;
if (p > ptr)
{
conv_restlen = todo;
mch_memmove(conv_rest, p, conv_restlen);
size -= conv_restlen;
break;
}
}
if (l == 1 || l > todo)
{
if (can_retry && !incomplete_tail)
break;
#if defined(USE_ICONV)
if (iconv_fd != (iconv_t)-1 && conv_error == 0)
conv_error = readfile_linenr(linecnt, ptr, p);
#endif
if (conv_error == 0 && illegal_byte == 0)
illegal_byte = readfile_linenr(linecnt, ptr, p);
if (bad_char_behavior == BAD_DROP)
{
mch_memmove(p, p + 1, todo - 1);
--p;
--size;
}
else if (bad_char_behavior != BAD_KEEP)
*p = bad_char_behavior;
}
else
p += l - 1;
}
}
if (p < ptr + size && !incomplete_tail)
{
rewind_retry:
#if defined(FEAT_EVAL) && defined(USE_ICONV)
if (*p_ccv != NUL && iconv_fd != (iconv_t)-1)
did_iconv = TRUE;
else
#endif
advance_fenc = TRUE;
file_rewind = TRUE;
goto retry;
}
}
filesize += size;
if (fileformat == EOL_UNKNOWN)
{
if (try_dos || try_unix)
{
if (try_mac)
try_mac = 1;
for (p = ptr; p < ptr + size; ++p)
{
if (*p == NL)
{
if (!try_unix
|| (try_dos && p > ptr && p[-1] == CAR))
fileformat = EOL_DOS;
else
fileformat = EOL_UNIX;
break;
}
else if (*p == CAR && try_mac)
try_mac++;
}
if (fileformat == EOL_UNIX && try_mac)
{
try_mac = 1;
try_unix = 1;
for (; p >= ptr && *p != CAR; p--)
;
if (p >= ptr)
{
for (p = ptr; p < ptr + size; ++p)
{
if (*p == NL)
try_unix++;
else if (*p == CAR)
try_mac++;
}
if (try_mac > try_unix)
fileformat = EOL_MAC;
}
}
else if (fileformat == EOL_UNKNOWN && try_mac == 1)
fileformat = default_fileformat();
}
if (fileformat == EOL_UNKNOWN && try_mac)
fileformat = EOL_MAC;
if (fileformat == EOL_UNKNOWN)
fileformat = default_fileformat();
if (set_options)
set_fileformat(fileformat, OPT_LOCAL);
}
}
if (fileformat == EOL_MAC)
{
--ptr;
while (++ptr, --size >= 0)
{
if ((c = *ptr) != NUL && c != CAR && c != NL)
continue;
if (c == NUL)
*ptr = NL; 
else if (c == NL)
*ptr = CAR; 
else
{
if (skip_count == 0)
{
*ptr = NUL; 
len = (colnr_T) (ptr - line_start + 1);
if (ml_append(lnum, line_start, len, newfile) == FAIL)
{
error = TRUE;
break;
}
#if defined(FEAT_PERSISTENT_UNDO)
if (read_undo_file)
sha256_update(&sha_ctx, line_start, len);
#endif
++lnum;
if (--read_count == 0)
{
error = TRUE; 
line_start = ptr; 
break;
}
}
else
--skip_count;
line_start = ptr + 1;
}
}
}
else
{
--ptr;
while (++ptr, --size >= 0)
{
if ((c = *ptr) != NUL && c != NL) 
continue;
if (c == NUL)
*ptr = NL; 
else
{
if (skip_count == 0)
{
*ptr = NUL; 
len = (colnr_T)(ptr - line_start + 1);
if (fileformat == EOL_DOS)
{
if (ptr > line_start && ptr[-1] == CAR)
{
ptr[-1] = NUL;
--len;
}
else if (ff_error != EOL_DOS)
{
if ( try_unix
&& !read_stdin
&& (read_buffer
|| vim_lseek(fd, (off_T)0L, SEEK_SET)
== 0))
{
fileformat = EOL_UNIX;
if (set_options)
set_fileformat(EOL_UNIX, OPT_LOCAL);
file_rewind = TRUE;
keep_fileformat = TRUE;
goto retry;
}
ff_error = EOL_DOS;
}
}
if (ml_append(lnum, line_start, len, newfile) == FAIL)
{
error = TRUE;
break;
}
#if defined(FEAT_PERSISTENT_UNDO)
if (read_undo_file)
sha256_update(&sha_ctx, line_start, len);
#endif
++lnum;
if (--read_count == 0)
{
error = TRUE; 
line_start = ptr; 
break;
}
}
else
--skip_count;
line_start = ptr + 1;
}
}
}
linerest = (long)(ptr - line_start);
ui_breakcheck();
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
&& ptr == line_start + 1))
{
if (set_options)
curbuf->b_p_eol = FALSE;
*ptr = NUL;
len = (colnr_T)(ptr - line_start + 1);
if (ml_append(lnum, line_start, len, newfile) == FAIL)
error = TRUE;
else
{
#if defined(FEAT_PERSISTENT_UNDO)
if (read_undo_file)
sha256_update(&sha_ctx, line_start, len);
#endif
read_no_eol_lnum = ++lnum;
}
}
if (set_options)
save_file_ff(curbuf); 
#if defined(FEAT_CRYPT)
if (curbuf->b_cryptstate != NULL)
{
crypt_free_state(curbuf->b_cryptstate);
curbuf->b_cryptstate = NULL;
}
if (cryptkey != NULL && cryptkey != curbuf->b_p_key)
crypt_free_key(cryptkey);
#endif
if (set_options)
set_string_option_direct((char_u *)"fenc", -1, fenc,
OPT_FREE|OPT_LOCAL, 0);
if (fenc_alloced)
vim_free(fenc);
#if defined(USE_ICONV)
if (iconv_fd != (iconv_t)-1)
iconv_close(iconv_fd);
#endif
if (!read_buffer && !read_stdin)
close(fd); 
#if defined(HAVE_FD_CLOEXEC)
else
{
int fdflags = fcntl(fd, F_GETFD);
if (fdflags >= 0 && (fdflags & FD_CLOEXEC) == 0)
(void)fcntl(fd, F_SETFD, fdflags | FD_CLOEXEC);
}
#endif
vim_free(buffer);
#if defined(HAVE_DUP)
if (read_stdin)
{
close(0);
vim_ignored = dup(2);
}
#endif
if (tmpname != NULL)
{
mch_remove(tmpname); 
vim_free(tmpname);
}
--no_wait_return; 
if (!recoverymode)
{
if (newfile && wasempty && !(curbuf->b_ml.ml_flags & ML_EMPTY))
{
#if defined(FEAT_NETBEANS_INTG)
netbeansFireChanges = 0;
#endif
ml_delete(curbuf->b_ml.ml_line_count, FALSE);
#if defined(FEAT_NETBEANS_INTG)
netbeansFireChanges = 1;
#endif
--linecnt;
}
linecnt = curbuf->b_ml.ml_line_count - linecnt;
if (filesize == 0)
linecnt = 0;
if (newfile || read_buffer)
{
redraw_curbuf_later(NOT_VALID);
#if defined(FEAT_DIFF)
diff_invalidate(curbuf);
#endif
#if defined(FEAT_FOLDING)
foldUpdateAll(curwin);
#endif
}
else if (linecnt) 
appended_lines_mark(from, linecnt);
#if !defined(ALWAYS_USE_GUI)
if (read_stdin)
{
settmode(TMODE_RAW); 
starttermcap();
screenclear();
}
#endif
if (got_int)
{
if (!(flags & READ_DUMMY))
{
filemess(curbuf, sfname, (char_u *)_(e_interr), 0);
if (newfile)
curbuf->b_p_ro = TRUE; 
}
msg_scroll = msg_save;
#if defined(FEAT_VIMINFO)
check_marks_read();
#endif
return OK; 
}
if (!filtering && !(flags & READ_DUMMY))
{
msg_add_fname(curbuf, sfname); 
c = FALSE;
#if defined(UNIX)
if (S_ISFIFO(perm)) 
{
STRCAT(IObuff, _("[fifo]"));
c = TRUE;
}
if (S_ISSOCK(perm)) 
{
STRCAT(IObuff, _("[socket]"));
c = TRUE;
}
#if defined(OPEN_CHR_FILES)
if (S_ISCHR(perm)) 
{
STRCAT(IObuff, _("[character special]"));
c = TRUE;
}
#endif
#endif
if (curbuf->b_p_ro)
{
STRCAT(IObuff, shortmess(SHM_RO) ? _("[RO]") : _("[readonly]"));
c = TRUE;
}
if (read_no_eol_lnum)
{
msg_add_eol();
c = TRUE;
}
if (ff_error == EOL_DOS)
{
STRCAT(IObuff, _("[CR missing]"));
c = TRUE;
}
if (split)
{
STRCAT(IObuff, _("[long lines split]"));
c = TRUE;
}
if (notconverted)
{
STRCAT(IObuff, _("[NOT converted]"));
c = TRUE;
}
else if (converted)
{
STRCAT(IObuff, _("[converted]"));
c = TRUE;
}
#if defined(FEAT_CRYPT)
if (cryptkey != NULL)
{
crypt_append_msg(curbuf);
c = TRUE;
}
#endif
if (conv_error != 0)
{
sprintf((char *)IObuff + STRLEN(IObuff),
_("[CONVERSION ERROR in line %ld]"), (long)conv_error);
c = TRUE;
}
else if (illegal_byte > 0)
{
sprintf((char *)IObuff + STRLEN(IObuff),
_("[ILLEGAL BYTE in line %ld]"), (long)illegal_byte);
c = TRUE;
}
else if (error)
{
STRCAT(IObuff, _("[READ ERRORS]"));
c = TRUE;
}
if (msg_add_fileformat(fileformat))
c = TRUE;
#if defined(FEAT_CRYPT)
if (cryptkey != NULL)
msg_add_lines(c, (long)linecnt, filesize
- crypt_get_header_len(crypt_get_method_nr(curbuf)));
else
#endif
msg_add_lines(c, (long)linecnt, filesize);
VIM_CLEAR(keep_msg);
msg_scrolled_ign = TRUE;
#if defined(ALWAYS_USE_GUI)
if (read_stdin || read_buffer)
p = msg_may_trunc(FALSE, IObuff);
else
#endif
{
if (msg_col > 0)
msg_putchar('\r'); 
p = (char_u *)msg_trunc_attr((char *)IObuff, FALSE, 0);
}
if (read_stdin || read_buffer || restart_edit != 0
|| (msg_scrolled != 0 && !need_wait_return))
set_keep_msg(p, 0);
msg_scrolled_ign = FALSE;
}
if (newfile && (error
|| conv_error != 0
|| (illegal_byte > 0 && bad_char_behavior != BAD_KEEP)))
curbuf->b_p_ro = TRUE;
u_clearline(); 
if (exmode_active)
curwin->w_cursor.lnum = from + linecnt;
else
curwin->w_cursor.lnum = from + 1;
check_cursor_lnum();
beginline(BL_WHITE | BL_FIX); 
if (!cmdmod.lockmarks)
{
curbuf->b_op_start.lnum = from + 1;
curbuf->b_op_start.col = 0;
curbuf->b_op_end.lnum = from + linecnt;
curbuf->b_op_end.col = 0;
}
#if defined(MSWIN)
if (newfile && !read_stdin && !read_buffer
&& mch_stat((char *)fname, &st) >= 0)
{
buf_store_time(curbuf, &st, fname);
curbuf->b_mtime_read = curbuf->b_mtime;
}
#endif
}
msg_scroll = msg_save;
#if defined(FEAT_VIMINFO)
check_marks_read();
#endif
curbuf->b_no_eol_lnum = read_no_eol_lnum;
if (flags & READ_KEEP_UNDO)
u_find_first_changed();
#if defined(FEAT_PERSISTENT_UNDO)
if (read_undo_file)
{
char_u hash[UNDO_HASH_SIZE];
sha256_finish(&sha_ctx, hash);
u_read_undo(NULL, hash, fname);
}
#endif
if (!read_stdin && !read_fifo && (!read_buffer || sfname != NULL))
{
int m = msg_scroll;
int n = msg_scrolled;
if (set_options)
save_file_ff(curbuf);
msg_scroll = TRUE;
if (filtering)
apply_autocmds_exarg(EVENT_FILTERREADPOST, NULL, sfname,
FALSE, curbuf, eap);
else if (newfile || (read_buffer && sfname != NULL))
{
apply_autocmds_exarg(EVENT_BUFREADPOST, NULL, sfname,
FALSE, curbuf, eap);
if (!au_did_filetype && *curbuf->b_p_ft != NUL)
apply_autocmds(EVENT_FILETYPE, curbuf->b_p_ft, curbuf->b_fname,
TRUE, curbuf);
}
else
apply_autocmds_exarg(EVENT_FILEREADPOST, sfname, sfname,
FALSE, NULL, eap);
if (msg_scrolled == n)
msg_scroll = m;
#if defined(FEAT_EVAL)
if (aborting()) 
return FAIL;
#endif
}
if (recoverymode && error)
return FAIL;
return OK;
}
#if defined(OPEN_CHR_FILES) || defined(PROTO)
int
is_dev_fd_file(char_u *fname)
{
return (STRNCMP(fname, "/dev/fd/", 8) == 0
&& VIM_ISDIGIT(fname[8])
&& *skipdigits(fname + 9) == NUL
&& (fname[9] != NUL
|| (fname[8] != '0' && fname[8] != '1' && fname[8] != '2')));
}
#endif
static linenr_T
readfile_linenr(
linenr_T linecnt, 
char_u *p, 
char_u *endp) 
{
char_u *s;
linenr_T lnum;
lnum = curbuf->b_ml.ml_line_count - linecnt + 1;
for (s = p; s < endp; ++s)
if (*s == '\n')
++lnum;
return lnum;
}
int
prep_exarg(exarg_T *eap, buf_T *buf)
{
eap->cmd = alloc(15 + (unsigned)STRLEN(buf->b_p_fenc));
if (eap->cmd == NULL)
return FAIL;
sprintf((char *)eap->cmd, "e ++enc=%s", buf->b_p_fenc);
eap->force_enc = 8;
eap->bad_char = buf->b_bad_char;
eap->force_ff = *buf->b_p_ff;
eap->force_bin = buf->b_p_bin ? FORCE_BIN : FORCE_NOBIN;
eap->read_edit = FALSE;
eap->forceit = FALSE;
return OK;
}
void
set_file_options(int set_options, exarg_T *eap)
{
if (set_options)
{
if (eap != NULL && eap->force_ff != 0)
set_fileformat(get_fileformat_force(curbuf, eap), OPT_LOCAL);
else if (*p_ffs != NUL)
set_fileformat(default_fileformat(), OPT_LOCAL);
}
if (eap != NULL && eap->force_bin != 0)
{
int oldval = curbuf->b_p_bin;
curbuf->b_p_bin = (eap->force_bin == FORCE_BIN);
set_options_bin(oldval, curbuf->b_p_bin, OPT_LOCAL);
}
}
void
set_forced_fenc(exarg_T *eap)
{
if (eap->force_enc != 0)
{
char_u *fenc = enc_canonize(eap->cmd + eap->force_enc);
if (fenc != NULL)
set_string_option_direct((char_u *)"fenc", -1,
fenc, OPT_FREE|OPT_LOCAL, 0);
vim_free(fenc);
}
}
static char_u *
next_fenc(char_u **pp, int *alloced)
{
char_u *p;
char_u *r;
*alloced = FALSE;
if (**pp == NUL)
{
*pp = NULL;
return (char_u *)"";
}
p = vim_strchr(*pp, ',');
if (p == NULL)
{
r = enc_canonize(*pp);
*pp += STRLEN(*pp);
}
else
{
r = vim_strnsave(*pp, (int)(p - *pp));
*pp = p + 1;
if (r != NULL)
{
p = enc_canonize(r);
vim_free(r);
r = p;
}
}
if (r != NULL)
*alloced = TRUE;
else
{
r = (char_u *)"";
*pp = NULL;
}
return r;
}
#if defined(FEAT_EVAL)
static char_u *
readfile_charconvert(
char_u *fname, 
char_u *fenc, 
int *fdp) 
{
char_u *tmpname;
char *errmsg = NULL;
tmpname = vim_tempname('r', FALSE);
if (tmpname == NULL)
errmsg = _("Can't find temp file for conversion");
else
{
close(*fdp); 
*fdp = -1;
if (eval_charconvert(fenc, enc_utf8 ? (char_u *)"utf-8" : p_enc,
fname, tmpname) == FAIL)
errmsg = _("Conversion with 'charconvert' failed");
if (errmsg == NULL && (*fdp = mch_open((char *)tmpname,
O_RDONLY | O_EXTRA, 0)) < 0)
errmsg = _("can't read output of 'charconvert'");
}
if (errmsg != NULL)
{
msg(errmsg);
if (tmpname != NULL)
{
mch_remove(tmpname); 
VIM_CLEAR(tmpname);
}
}
if (*fdp < 0)
*fdp = mch_open((char *)fname, O_RDONLY | O_EXTRA, 0);
return tmpname;
}
#endif
#if defined(FEAT_CRYPT) || defined(PROTO)
static char_u *
check_for_cryptkey(
char_u *cryptkey, 
char_u *ptr, 
long *sizep, 
off_T *filesizep, 
int newfile, 
char_u *fname, 
int *did_ask) 
{
int method = crypt_method_nr_from_magic((char *)ptr, *sizep);
int b_p_ro = curbuf->b_p_ro;
if (method >= 0)
{
curbuf->b_p_ro = TRUE;
crypt_set_cm_option(curbuf, method);
if (cryptkey == NULL && !*did_ask)
{
if (*curbuf->b_p_key)
cryptkey = curbuf->b_p_key;
else
{
smsg(_(need_key_msg), fname);
msg_scroll = TRUE;
crypt_check_method(method);
cryptkey = crypt_get_key(newfile, FALSE);
*did_ask = TRUE;
if (cryptkey != NULL && *cryptkey == NUL)
{
if (cryptkey != curbuf->b_p_key)
vim_free(cryptkey);
cryptkey = NULL;
}
}
}
if (cryptkey != NULL)
{
int header_len;
curbuf->b_cryptstate = crypt_create_from_header(
method, cryptkey, ptr);
crypt_set_cm_option(curbuf, method);
header_len = crypt_get_header_len(method);
if (*sizep <= header_len)
return NULL;
*filesizep += header_len;
*sizep -= header_len;
mch_memmove(ptr, ptr + header_len, (size_t)*sizep);
curbuf->b_p_ro = b_p_ro;
}
}
else if (newfile && *curbuf->b_p_key != NUL && !starting)
set_option_value((char_u *)"key", 0L, (char_u *)"", OPT_LOCAL);
return cryptkey;
}
#endif 
int
check_file_readonly(
char_u *fname, 
int perm UNUSED) 
{
#if !defined(USE_MCH_ACCESS)
int fd = 0;
#endif
return (
#if defined(USE_MCH_ACCESS)
#if defined(UNIX)
(perm & 0222) == 0 ||
#endif
mch_access((char *)fname, W_OK)
#else
(fd = mch_open((char *)fname, O_RDWR | O_EXTRA, 0)) < 0
? TRUE : (close(fd), FALSE)
#endif
);
}
#if defined(HAVE_FSYNC) || defined(PROTO)
int
vim_fsync(int fd)
{
int r;
#if defined(MACOS_X)
r = fcntl(fd, F_FULLFSYNC);
if (r != 0) 
#endif
r = fsync(fd);
return r;
}
#endif
int
set_rw_fname(char_u *fname, char_u *sfname)
{
buf_T *buf = curbuf;
if (curbuf->b_p_bl)
apply_autocmds(EVENT_BUFDELETE, NULL, NULL, FALSE, curbuf);
apply_autocmds(EVENT_BUFWIPEOUT, NULL, NULL, FALSE, curbuf);
#if defined(FEAT_EVAL)
if (aborting()) 
return FAIL;
#endif
if (curbuf != buf)
{
emsg(_(e_auchangedbuf));
return FAIL;
}
if (setfname(curbuf, fname, sfname, FALSE) == OK)
curbuf->b_flags |= BF_NOTEDITED;
apply_autocmds(EVENT_BUFNEW, NULL, NULL, FALSE, curbuf);
if (curbuf->b_p_bl)
apply_autocmds(EVENT_BUFADD, NULL, NULL, FALSE, curbuf);
#if defined(FEAT_EVAL)
if (aborting()) 
return FAIL;
#endif
if (*curbuf->b_p_ft == NUL)
{
if (au_has_group((char_u *)"filetypedetect"))
(void)do_doautocmd((char_u *)"filetypedetect BufRead", FALSE, NULL);
do_modelines(0);
}
return OK;
}
void
msg_add_fname(buf_T *buf, char_u *fname)
{
if (fname == NULL)
fname = (char_u *)"-stdin-";
home_replace(buf, fname, IObuff + 1, IOSIZE - 4, TRUE);
IObuff[0] = '"';
STRCAT(IObuff, "\" ");
}
int
msg_add_fileformat(int eol_type)
{
#if !defined(USE_CRNL)
if (eol_type == EOL_DOS)
{
STRCAT(IObuff, shortmess(SHM_TEXT) ? _("[dos]") : _("[dos format]"));
return TRUE;
}
#endif
if (eol_type == EOL_MAC)
{
STRCAT(IObuff, shortmess(SHM_TEXT) ? _("[mac]") : _("[mac format]"));
return TRUE;
}
#if defined(USE_CRNL)
if (eol_type == EOL_UNIX)
{
STRCAT(IObuff, shortmess(SHM_TEXT) ? _("[unix]") : _("[unix format]"));
return TRUE;
}
#endif
return FALSE;
}
void
msg_add_lines(
int insert_space,
long lnum,
off_T nchars)
{
char_u *p;
p = IObuff + STRLEN(IObuff);
if (insert_space)
*p++ = ' ';
if (shortmess(SHM_LINES))
vim_snprintf((char *)p, IOSIZE - (p - IObuff),
"%ldL, %lldC", lnum, (varnumber_T)nchars);
else
{
sprintf((char *)p, NGETTEXT("%ld line, ", "%ld lines, ", lnum), lnum);
p += STRLEN(p);
vim_snprintf((char *)p, IOSIZE - (p - IObuff),
NGETTEXT("%lld character", "%lld characters", nchars),
(varnumber_T)nchars);
}
}
void
msg_add_eol(void)
{
STRCAT(IObuff, shortmess(SHM_LAST) ? _("[noeol]") : _("[Incomplete last line]"));
}
int
time_differs(long t1, long t2)
{
#if defined(__linux__) || defined(MSWIN)
return (t1 - t2 > 1 || t2 - t1 > 1);
#else
return (t1 != t2);
#endif
}
int
need_conversion(char_u *fenc)
{
int same_encoding;
int enc_flags;
int fenc_flags;
if (*fenc == NUL || STRCMP(p_enc, fenc) == 0)
{
same_encoding = TRUE;
fenc_flags = 0;
}
else
{
enc_flags = get_fio_flags(p_enc);
fenc_flags = get_fio_flags(fenc);
same_encoding = (enc_flags != 0 && fenc_flags == enc_flags);
}
if (same_encoding)
{
return enc_unicode != 0;
}
return !(enc_utf8 && fenc_flags == FIO_UTF8);
}
int
get_fio_flags(char_u *ptr)
{
int prop;
if (*ptr == NUL)
ptr = p_enc;
prop = enc_canon_props(ptr);
if (prop & ENC_UNICODE)
{
if (prop & ENC_2BYTE)
{
if (prop & ENC_ENDIAN_L)
return FIO_UCS2 | FIO_ENDIAN_L;
return FIO_UCS2;
}
if (prop & ENC_4BYTE)
{
if (prop & ENC_ENDIAN_L)
return FIO_UCS4 | FIO_ENDIAN_L;
return FIO_UCS4;
}
if (prop & ENC_2WORD)
{
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
#if defined(MSWIN) || defined(PROTO)
int
get_win_fio_flags(char_u *ptr)
{
int cp;
if (!enc_utf8 && enc_codepage <= 0)
return 0;
cp = encname2codepage(ptr);
if (cp == 0)
{
#if defined(CP_UTF8)
if (STRCMP(ptr, "utf-8") == 0)
cp = CP_UTF8;
else
#endif
return 0;
}
return FIO_PUT_CP(cp) | FIO_CODEPAGE;
}
#endif
#if defined(MACOS_CONVERT) || defined(PROTO)
int
get_mac_fio_flags(char_u *ptr)
{
if ((enc_utf8 || STRCMP(p_enc, "latin1") == 0)
&& (enc_canon_props(ptr) & ENC_MACROMAN))
return FIO_MACROMAN;
return 0;
}
#endif
static char_u *
check_for_bom(
char_u *p,
long size,
int *lenp,
int flags)
{
char *name = NULL;
int len = 2;
if (p[0] == 0xef && p[1] == 0xbb && size >= 3 && p[2] == 0xbf
&& (flags == FIO_ALL || flags == FIO_UTF8 || flags == 0))
{
name = "utf-8"; 
len = 3;
}
else if (p[0] == 0xff && p[1] == 0xfe)
{
if (size >= 4 && p[2] == 0 && p[3] == 0
&& (flags == FIO_ALL || flags == (FIO_UCS4 | FIO_ENDIAN_L)))
{
name = "ucs-4le"; 
len = 4;
}
else if (flags == (FIO_UCS2 | FIO_ENDIAN_L))
name = "ucs-2le"; 
else if (flags == FIO_ALL || flags == (FIO_UTF16 | FIO_ENDIAN_L))
name = "utf-16le"; 
}
else if (p[0] == 0xfe && p[1] == 0xff
&& (flags == FIO_ALL || flags == FIO_UCS2 || flags == FIO_UTF16))
{
if (flags == FIO_UCS2)
name = "ucs-2"; 
else
name = "utf-16"; 
}
else if (size >= 4 && p[0] == 0 && p[1] == 0 && p[2] == 0xfe
&& p[3] == 0xff && (flags == FIO_ALL || flags == FIO_UCS4))
{
name = "ucs-4"; 
len = 4;
}
*lenp = len;
return (char_u *)name;
}
char_u *
shorten_fname1(char_u *full_path)
{
char_u *dirname;
char_u *p = full_path;
dirname = alloc(MAXPATHL);
if (dirname == NULL)
return full_path;
if (mch_dirname(dirname, MAXPATHL) == OK)
{
p = shorten_fname(full_path, dirname);
if (p == NULL || *p == NUL)
p = full_path;
}
vim_free(dirname);
return p;
}
char_u *
shorten_fname(char_u *full_path, char_u *dir_name)
{
int len;
char_u *p;
if (full_path == NULL)
return NULL;
len = (int)STRLEN(dir_name);
if (fnamencmp(dir_name, full_path, len) == 0)
{
p = full_path + len;
#if defined(MSWIN)
if (!((len > 2) && (*(p - 2) == ':')))
#endif
{
if (vim_ispathsep(*p))
++p;
#if !defined(VMS)
else
p = NULL;
#endif
}
}
#if defined(MSWIN)
else if (len > 3
&& TOUPPER_LOC(full_path[0]) == TOUPPER_LOC(dir_name[0])
&& full_path[1] == ':'
&& vim_ispathsep(full_path[2]))
p = full_path + 2;
#endif
else
p = NULL;
return p;
}
void
shorten_buf_fname(buf_T *buf, char_u *dirname, int force)
{
char_u *p;
if (buf->b_fname != NULL
#if defined(FEAT_QUICKFIX)
&& !bt_nofilename(buf)
#endif
&& !path_with_url(buf->b_fname)
&& (force
|| buf->b_sfname == NULL
|| mch_isFullName(buf->b_sfname)))
{
if (buf->b_sfname != buf->b_ffname)
VIM_CLEAR(buf->b_sfname);
p = shorten_fname(buf->b_ffname, dirname);
if (p != NULL)
{
buf->b_sfname = vim_strsave(p);
buf->b_fname = buf->b_sfname;
}
if (p == NULL || buf->b_fname == NULL)
buf->b_fname = buf->b_ffname;
}
}
void
shorten_fnames(int force)
{
char_u dirname[MAXPATHL];
buf_T *buf;
mch_dirname(dirname, MAXPATHL);
FOR_ALL_BUFFERS(buf)
{
shorten_buf_fname(buf, dirname, force);
mf_fullname(buf->b_ml.ml_mfp);
}
status_redraw_all();
redraw_tabline = TRUE;
#if defined(FEAT_PROP_POPUP) && defined(FEAT_QUICKFIX)
popup_update_preview_title();
#endif
}
#if (defined(FEAT_DND) && defined(FEAT_GUI_GTK)) || defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_MAC) || defined(FEAT_GUI_HAIKU) || defined(PROTO)
void
shorten_filenames(char_u **fnames, int count)
{
int i;
char_u dirname[MAXPATHL];
char_u *p;
if (fnames == NULL || count < 1)
return;
mch_dirname(dirname, sizeof(dirname));
for (i = 0; i < count; ++i)
{
if ((p = shorten_fname(fnames[i], dirname)) != NULL)
{
p = vim_strsave(p);
vim_free(fnames[i]);
fnames[i] = p;
}
}
}
#endif
char_u *
modname(
char_u *fname,
char_u *ext,
int prepend_dot) 
{
return buf_modname((curbuf->b_p_sn || curbuf->b_shortname),
fname, ext, prepend_dot);
}
char_u *
buf_modname(
int shortname, 
char_u *fname,
char_u *ext,
int prepend_dot) 
{
char_u *retval;
char_u *s;
char_u *e;
char_u *ptr;
int fnamelen, extlen;
extlen = (int)STRLEN(ext);
if (fname == NULL || *fname == NUL)
{
retval = alloc(MAXPATHL + extlen + 3);
if (retval == NULL)
return NULL;
if (mch_dirname(retval, MAXPATHL) == FAIL ||
(fnamelen = (int)STRLEN(retval)) == 0)
{
vim_free(retval);
return NULL;
}
if (!after_pathsep(retval, retval + fnamelen))
{
retval[fnamelen++] = PATHSEP;
retval[fnamelen] = NUL;
}
prepend_dot = FALSE; 
}
else
{
fnamelen = (int)STRLEN(fname);
retval = alloc(fnamelen + extlen + 3);
if (retval == NULL)
return NULL;
STRCPY(retval, fname);
#if defined(VMS)
vms_remove_version(retval); 
#endif
}
for (ptr = retval + fnamelen; ptr > retval; MB_PTR_BACK(retval, ptr))
{
if (*ext == '.' && shortname)
if (*ptr == '.') 
*ptr = '_';
if (vim_ispathsep(*ptr))
{
++ptr;
break;
}
}
if (STRLEN(ptr) > (unsigned)BASENAMELEN)
ptr[BASENAMELEN] = '\0';
s = ptr + STRLEN(ptr);
if (shortname)
{
if (fname == NULL || *fname == NUL
|| vim_ispathsep(fname[STRLEN(fname) - 1]))
{
if (*ext == '.')
*s++ = '_';
}
else if (*ext == '.')
{
if ((size_t)(s - ptr) > (size_t)8)
{
s = ptr + 8;
*s = '\0';
}
}
else if ((e = vim_strchr(ptr, '.')) == NULL)
*s++ = '.';
else if ((int)STRLEN(e) + extlen > 4)
s = e + 4 - extlen;
}
#if defined(MSWIN)
else if ((fname == NULL || *fname == NUL) && *ext == '.')
*s++ = '_';
#endif
STRCPY(s, ext);
if (prepend_dot && !shortname && *(e = gettail(retval)) != '.')
{
STRMOVE(e + 1, e);
*e = '.';
}
if (fname != NULL && STRCMP(fname, retval) == 0)
{
while (--s >= ptr)
{
if (*s != '_')
{
*s = '_';
break;
}
}
if (s < ptr) 
*ptr = 'v';
}
return retval;
}
int
vim_fgets(char_u *buf, int size, FILE *fp)
{
char *eof;
#define FGETS_SIZE 200
char tbuf[FGETS_SIZE];
buf[size - 2] = NUL;
eof = fgets((char *)buf, size, fp);
if (buf[size - 2] != NUL && buf[size - 2] != '\n')
{
buf[size - 1] = NUL; 
do
{
tbuf[FGETS_SIZE - 2] = NUL;
vim_ignoredp = fgets((char *)tbuf, FGETS_SIZE, fp);
} while (tbuf[FGETS_SIZE - 2] != NUL && tbuf[FGETS_SIZE - 2] != '\n');
}
return (eof == NULL);
}
int
vim_rename(char_u *from, char_u *to)
{
int fd_in;
int fd_out;
int n;
char *errmsg = NULL;
char *buffer;
#if defined(AMIGA)
BPTR flock;
#endif
stat_T st;
long perm;
#if defined(HAVE_ACL)
vim_acl_T acl; 
#endif
int use_tmp_file = FALSE;
if (fnamecmp(from, to) == 0)
{
if (p_fic && STRCMP(gettail(from), gettail(to)) != 0)
use_tmp_file = TRUE;
else
return 0;
}
if (mch_stat((char *)from, &st) < 0)
return -1;
#if defined(UNIX)
{
stat_T st_to;
if (mch_stat((char *)to, &st_to) >= 0
&& st.st_dev == st_to.st_dev
&& st.st_ino == st_to.st_ino)
use_tmp_file = TRUE;
}
#endif
#if defined(MSWIN)
{
BY_HANDLE_FILE_INFORMATION info1, info2;
if (win32_fileinfo(from, &info1) == FILEINFO_OK
&& win32_fileinfo(to, &info2) == FILEINFO_OK
&& info1.dwVolumeSerialNumber == info2.dwVolumeSerialNumber
&& info1.nFileIndexHigh == info2.nFileIndexHigh
&& info1.nFileIndexLow == info2.nFileIndexLow)
use_tmp_file = TRUE;
}
#endif
if (use_tmp_file)
{
char tempname[MAXPATHL + 1];
if (STRLEN(from) >= MAXPATHL - 5)
return -1;
STRCPY(tempname, from);
for (n = 123; n < 99999; ++n)
{
sprintf((char *)gettail((char_u *)tempname), "%d", n);
if (mch_stat(tempname, &st) < 0)
{
if (mch_rename((char *)from, tempname) == 0)
{
if (mch_rename(tempname, (char *)to) == 0)
return 0;
mch_rename(tempname, (char *)from);
return -1;
}
return -1;
}
}
return -1;
}
#if defined(AMIGA)
flock = Lock((UBYTE *)from, (long)ACCESS_READ);
#endif
mch_remove(to);
#if defined(AMIGA)
if (flock)
UnLock(flock);
#endif
if (mch_rename((char *)from, (char *)to) == 0)
return 0;
perm = mch_getperm(from);
#if defined(HAVE_ACL)
acl = mch_get_acl(from);
#endif
fd_in = mch_open((char *)from, O_RDONLY|O_EXTRA, 0);
if (fd_in == -1)
{
#if defined(HAVE_ACL)
mch_free_acl(acl);
#endif
return -1;
}
fd_out = mch_open((char *)to,
O_CREAT|O_EXCL|O_WRONLY|O_EXTRA|O_NOFOLLOW, (int)perm);
if (fd_out == -1)
{
close(fd_in);
#if defined(HAVE_ACL)
mch_free_acl(acl);
#endif
return -1;
}
buffer = alloc(WRITEBUFSIZE);
if (buffer == NULL)
{
close(fd_out);
close(fd_in);
#if defined(HAVE_ACL)
mch_free_acl(acl);
#endif
return -1;
}
while ((n = read_eintr(fd_in, buffer, WRITEBUFSIZE)) > 0)
if (write_eintr(fd_out, buffer, n) != n)
{
errmsg = _("E208: Error writing to \"%s\"");
break;
}
vim_free(buffer);
close(fd_in);
if (close(fd_out) < 0)
errmsg = _("E209: Error closing \"%s\"");
if (n < 0)
{
errmsg = _("E210: Error reading \"%s\"");
to = from;
}
#if !defined(UNIX)
mch_setperm(to, perm);
#endif
#if defined(HAVE_ACL)
mch_set_acl(to, acl);
mch_free_acl(acl);
#endif
#if defined(HAVE_SELINUX) || defined(HAVE_SMACK)
mch_copy_sec(from, to);
#endif
if (errmsg != NULL)
{
semsg(errmsg, to);
return -1;
}
mch_remove(from);
return 0;
}
static int already_warned = FALSE;
int
check_timestamps(
int focus) 
{
buf_T *buf;
int didit = 0;
int n;
if (no_check_timestamps > 0)
return FALSE;
if (focus && did_check_timestamps)
{
need_check_timestamps = TRUE;
return FALSE;
}
if (!stuff_empty() || global_busy || !typebuf_typed()
|| autocmd_busy || curbuf_lock > 0 || allbuf_lock > 0)
need_check_timestamps = TRUE; 
else
{
++no_wait_return;
did_check_timestamps = TRUE;
already_warned = FALSE;
FOR_ALL_BUFFERS(buf)
{
if (buf->b_nwindows > 0)
{
bufref_T bufref;
set_bufref(&bufref, buf);
n = buf_check_timestamp(buf, focus);
if (didit < n)
didit = n;
if (n > 0 && !bufref_valid(&bufref))
{
buf = firstbuf;
continue;
}
}
}
--no_wait_return;
need_check_timestamps = FALSE;
if (need_wait_return && didit == 2)
{
msg_puts("\n");
out_flush();
}
}
return didit;
}
static int
move_lines(buf_T *frombuf, buf_T *tobuf)
{
buf_T *tbuf = curbuf;
int retval = OK;
linenr_T lnum;
char_u *p;
curbuf = tobuf;
for (lnum = 1; lnum <= frombuf->b_ml.ml_line_count; ++lnum)
{
p = vim_strsave(ml_get_buf(frombuf, lnum, FALSE));
if (p == NULL || ml_append(lnum - 1, p, 0, FALSE) == FAIL)
{
vim_free(p);
retval = FAIL;
break;
}
vim_free(p);
}
if (retval != FAIL)
{
curbuf = frombuf;
for (lnum = curbuf->b_ml.ml_line_count; lnum > 0; --lnum)
if (ml_delete(lnum, FALSE) == FAIL)
{
retval = FAIL;
break;
}
}
curbuf = tbuf;
return retval;
}
int
buf_check_timestamp(
buf_T *buf,
int focus UNUSED) 
{
stat_T st;
int stat_res;
int retval = 0;
char_u *path;
char *tbuf;
char *mesg = NULL;
char *mesg2 = "";
int helpmesg = FALSE;
int reload = FALSE;
char *reason;
#if defined(FEAT_CON_DIALOG) || defined(FEAT_GUI_DIALOG)
int can_reload = FALSE;
#endif
off_T orig_size = buf->b_orig_size;
int orig_mode = buf->b_orig_mode;
#if defined(FEAT_GUI)
int save_mouse_correct = need_mouse_correct;
#endif
static int busy = FALSE;
int n;
#if defined(FEAT_EVAL)
char_u *s;
#endif
bufref_T bufref;
set_bufref(&bufref, buf);
if (buf->b_ffname == NULL
|| buf->b_ml.ml_mfp == NULL
|| !bt_normal(buf)
|| buf->b_saving
|| busy
#if defined(FEAT_NETBEANS_INTG)
|| isNetbeansBuffer(buf)
#endif
#if defined(FEAT_TERMINAL)
|| buf->b_term != NULL
#endif
)
return 0;
if ( !(buf->b_flags & BF_NOTEDITED)
&& buf->b_mtime != 0
&& ((stat_res = mch_stat((char *)buf->b_ffname, &st)) < 0
|| time_differs((long)st.st_mtime, buf->b_mtime)
|| st.st_size != buf->b_orig_size
#if defined(HAVE_ST_MODE)
|| (int)st.st_mode != buf->b_orig_mode
#else
|| mch_getperm(buf->b_ffname) != buf->b_orig_mode
#endif
))
{
long prev_b_mtime = buf->b_mtime;
retval = 1;
if (stat_res < 0)
{
buf->b_mtime = -1;
buf->b_orig_size = 0;
buf->b_orig_mode = 0;
}
else
buf_store_time(buf, &st, buf->b_ffname);
if (mch_isdir(buf->b_fname))
;
else if ((buf->b_p_ar >= 0 ? buf->b_p_ar : p_ar)
&& !bufIsChanged(buf) && stat_res >= 0)
reload = TRUE;
else
{
if (stat_res < 0)
reason = "deleted";
else if (bufIsChanged(buf))
reason = "conflict";
else if (orig_size != buf->b_orig_size || buf_contents_changed(buf))
reason = "changed";
else if (orig_mode != buf->b_orig_mode)
reason = "mode";
else
reason = "time";
busy = TRUE;
#if defined(FEAT_EVAL)
set_vim_var_string(VV_FCS_REASON, (char_u *)reason, -1);
set_vim_var_string(VV_FCS_CHOICE, (char_u *)"", -1);
#endif
++allbuf_lock;
n = apply_autocmds(EVENT_FILECHANGEDSHELL,
buf->b_fname, buf->b_fname, FALSE, buf);
--allbuf_lock;
busy = FALSE;
if (n)
{
if (!bufref_valid(&bufref))
emsg(_("E246: FileChangedShell autocommand deleted buffer"));
#if defined(FEAT_EVAL)
s = get_vim_var_str(VV_FCS_CHOICE);
if (STRCMP(s, "reload") == 0 && *reason != 'd')
reload = TRUE;
else if (STRCMP(s, "ask") == 0)
n = FALSE;
else
#endif
return 2;
}
if (!n)
{
if (*reason == 'd')
{
if (prev_b_mtime != -1)
mesg = _("E211: File \"%s\" no longer available");
}
else
{
helpmesg = TRUE;
#if defined(FEAT_CON_DIALOG) || defined(FEAT_GUI_DIALOG)
can_reload = TRUE;
#endif
if (reason[2] == 'n')
{
mesg = _("W12: Warning: File \"%s\" has changed and the buffer was changed in Vim as well");
mesg2 = _("See \":help W12\" for more info.");
}
else if (reason[1] == 'h')
{
mesg = _("W11: Warning: File \"%s\" has changed since editing started");
mesg2 = _("See \":help W11\" for more info.");
}
else if (*reason == 'm')
{
mesg = _("W16: Warning: Mode of file \"%s\" has changed since editing started");
mesg2 = _("See \":help W16\" for more info.");
}
else
buf->b_mtime_read = buf->b_mtime;
}
}
}
}
else if ((buf->b_flags & BF_NEW) && !(buf->b_flags & BF_NEW_W)
&& vim_fexists(buf->b_ffname))
{
retval = 1;
mesg = _("W13: Warning: File \"%s\" has been created after editing started");
buf->b_flags |= BF_NEW_W;
#if defined(FEAT_CON_DIALOG) || defined(FEAT_GUI_DIALOG)
can_reload = TRUE;
#endif
}
if (mesg != NULL)
{
path = home_replace_save(buf, buf->b_fname);
if (path != NULL)
{
if (!helpmesg)
mesg2 = "";
tbuf = alloc(STRLEN(path) + STRLEN(mesg) + STRLEN(mesg2) + 2);
sprintf(tbuf, mesg, path);
#if defined(FEAT_EVAL)
set_vim_var_string(VV_WARNINGMSG, (char_u *)tbuf, -1);
#endif
#if defined(FEAT_CON_DIALOG) || defined(FEAT_GUI_DIALOG)
if (can_reload)
{
if (*mesg2 != NUL)
{
STRCAT(tbuf, "\n");
STRCAT(tbuf, mesg2);
}
if (do_dialog(VIM_WARNING, (char_u *)_("Warning"),
(char_u *)tbuf,
(char_u *)_("&OK\n&Load File"), 1, NULL, TRUE) == 2)
reload = TRUE;
}
else
#endif
if (State > NORMAL_BUSY || (State & CMDLINE) || already_warned)
{
if (*mesg2 != NUL)
{
STRCAT(tbuf, "; ");
STRCAT(tbuf, mesg2);
}
emsg(tbuf);
retval = 2;
}
else
{
if (!autocmd_busy)
{
msg_start();
msg_puts_attr(tbuf, HL_ATTR(HLF_E) + MSG_HIST);
if (*mesg2 != NUL)
msg_puts_attr(mesg2, HL_ATTR(HLF_W) + MSG_HIST);
msg_clr_eos();
(void)msg_end();
if (emsg_silent == 0)
{
out_flush();
#if defined(FEAT_GUI)
if (!focus)
#endif
ui_delay(1004L, TRUE);
redraw_cmdline = FALSE;
}
}
already_warned = TRUE;
}
vim_free(path);
vim_free(tbuf);
}
}
if (reload)
{
buf_reload(buf, orig_mode);
#if defined(FEAT_PERSISTENT_UNDO)
if (buf->b_p_udf && buf->b_ffname != NULL)
{
char_u hash[UNDO_HASH_SIZE];
buf_T *save_curbuf = curbuf;
curbuf = buf;
u_compute_hash(hash);
u_write_undo(NULL, FALSE, buf, hash);
curbuf = save_curbuf;
}
#endif
}
if (bufref_valid(&bufref) && retval != 0)
(void)apply_autocmds(EVENT_FILECHANGEDSHELLPOST,
buf->b_fname, buf->b_fname, FALSE, buf);
#if defined(FEAT_GUI)
need_mouse_correct = save_mouse_correct;
#endif
return retval;
}
void
buf_reload(buf_T *buf, int orig_mode)
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
if (prep_exarg(&ea, buf) == OK)
{
old_cursor = curwin->w_cursor;
old_topline = curwin->w_topline;
if (p_ur < 0 || curbuf->b_ml.ml_line_count <= p_ur)
{
u_sync(FALSE);
saved = u_savecommon(0, curbuf->b_ml.ml_line_count + 1, 0, TRUE);
flags |= READ_KEEP_UNDO;
}
if (BUFEMPTY() || saved == FAIL)
savebuf = NULL;
else
{
savebuf = buflist_new(NULL, NULL, (linenr_T)1, BLN_DUMMY);
set_bufref(&bufref, savebuf);
if (savebuf != NULL && buf == curbuf)
{
curbuf = savebuf;
curwin->w_buffer = savebuf;
saved = ml_open(curbuf);
curbuf = buf;
curwin->w_buffer = buf;
}
if (savebuf == NULL || saved == FAIL || buf != curbuf
|| move_lines(buf, savebuf) == FAIL)
{
semsg(_("E462: Could not prepare for reloading \"%s\""),
buf->b_fname);
saved = FAIL;
}
}
if (saved == OK)
{
curbuf->b_flags |= BF_CHECK_RO; 
keep_filetype = TRUE; 
if (readfile(buf->b_ffname, buf->b_fname, (linenr_T)0,
(linenr_T)0,
(linenr_T)MAXLNUM, &ea, flags) != OK)
{
#if defined(FEAT_EVAL)
if (!aborting())
#endif
semsg(_("E321: Could not reload \"%s\""), buf->b_fname);
if (savebuf != NULL && bufref_valid(&bufref) && buf == curbuf)
{
while (!BUFEMPTY())
if (ml_delete(buf->b_ml.ml_line_count, FALSE) == FAIL)
break;
(void)move_lines(savebuf, buf);
}
}
else if (buf == curbuf) 
{
unchanged(buf, TRUE, TRUE);
if ((flags & READ_KEEP_UNDO) == 0)
{
u_blockfree(buf);
u_clearall(buf);
}
else
{
u_unchanged(curbuf);
}
}
}
vim_free(ea.cmd);
if (savebuf != NULL && bufref_valid(&bufref))
wipe_buffer(savebuf, FALSE);
#if defined(FEAT_DIFF)
diff_invalidate(curbuf);
#endif
if (old_topline > curbuf->b_ml.ml_line_count)
curwin->w_topline = curbuf->b_ml.ml_line_count;
else
curwin->w_topline = old_topline;
curwin->w_cursor = old_cursor;
check_cursor();
update_topline();
keep_filetype = FALSE;
#if defined(FEAT_FOLDING)
{
win_T *wp;
tabpage_T *tp;
FOR_ALL_TAB_WINDOWS(tp, wp)
if (wp->w_buffer == curwin->w_buffer
&& !foldmethodIsManual(wp))
foldUpdateAll(wp);
}
#endif
if (orig_mode == curbuf->b_orig_mode)
curbuf->b_p_ro |= old_ro;
do_modelines(0);
}
aucmd_restbuf(&aco);
}
void
buf_store_time(buf_T *buf, stat_T *st, char_u *fname UNUSED)
{
buf->b_mtime = (long)st->st_mtime;
buf->b_orig_size = st->st_size;
#if defined(HAVE_ST_MODE)
buf->b_orig_mode = (int)st->st_mode;
#else
buf->b_orig_mode = mch_getperm(fname);
#endif
}
void
write_lnum_adjust(linenr_T offset)
{
if (curbuf->b_no_eol_lnum != 0) 
curbuf->b_no_eol_lnum += offset;
}
#if defined(TEMPDIRNAMES) || defined(FEAT_EVAL) || defined(PROTO)
int
readdir_core(
garray_T *gap,
char_u *path,
void *context,
int (*checkitem)(void *context, char_u *name))
{
int failed = FALSE;
char_u *p;
ga_init2(gap, (int)sizeof(char *), 20);
#if defined(MSWIN)
{
char_u *buf;
int ok;
HANDLE hFind = INVALID_HANDLE_VALUE;
WIN32_FIND_DATAW wfb;
WCHAR *wn = NULL; 
buf = alloc(MAXPATHL);
if (buf == NULL)
return FAIL;
STRNCPY(buf, path, MAXPATHL-5);
p = buf + STRLEN(buf);
MB_PTR_BACK(buf, p);
if (*p == '\\' || *p == '/')
*p = NUL;
STRCAT(buf, "\\*");
wn = enc_to_utf16(buf, NULL);
if (wn != NULL)
hFind = FindFirstFileW(wn, &wfb);
ok = (hFind != INVALID_HANDLE_VALUE);
if (!ok)
{
failed = TRUE;
smsg(_(e_notopen), path);
}
else
{
while (ok)
{
int ignore;
p = utf16_to_enc(wfb.cFileName, NULL); 
if (p == NULL)
break; 
ignore = p[0] == '.' && (p[1] == NUL
|| (p[1] == '.' && p[2] == NUL));
if (!ignore && checkitem != NULL)
{
int r = checkitem(context, p);
if (r < 0)
{
vim_free(p);
break;
}
if (r == 0)
ignore = TRUE;
}
if (!ignore)
{
if (ga_grow(gap, 1) == OK)
((char_u**)gap->ga_data)[gap->ga_len++] = vim_strsave(p);
else
{
failed = TRUE;
vim_free(p);
break;
}
}
vim_free(p);
ok = FindNextFileW(hFind, &wfb);
}
FindClose(hFind);
}
vim_free(buf);
vim_free(wn);
}
#else
{
DIR *dirp;
struct dirent *dp;
dirp = opendir((char *)path);
if (dirp == NULL)
{
failed = TRUE;
smsg(_(e_notopen), path);
}
else
{
for (;;)
{
int ignore;
dp = readdir(dirp);
if (dp == NULL)
break;
p = (char_u *)dp->d_name;
ignore = p[0] == '.' &&
(p[1] == NUL ||
(p[1] == '.' && p[2] == NUL));
if (!ignore && checkitem != NULL)
{
int r = checkitem(context, p);
if (r < 0)
break;
if (r == 0)
ignore = TRUE;
}
if (!ignore)
{
if (ga_grow(gap, 1) == OK)
((char_u**)gap->ga_data)[gap->ga_len++] = vim_strsave(p);
else
{
failed = TRUE;
break;
}
}
}
closedir(dirp);
}
}
#endif
if (!failed && gap->ga_len > 0)
sort_strings((char_u **)gap->ga_data, gap->ga_len);
return failed ? FAIL : OK;
}
int
delete_recursive(char_u *name)
{
int result = 0;
int i;
char_u *exp;
garray_T ga;
if (
#if defined(UNIX) || defined(MSWIN)
mch_isrealdir(name)
#else
mch_isdir(name)
#endif
)
{
exp = vim_strsave(name);
if (exp == NULL)
return -1;
if (readdir_core(&ga, exp, NULL, NULL) == OK)
{
for (i = 0; i < ga.ga_len; ++i)
{
vim_snprintf((char *)NameBuff, MAXPATHL, "%s/%s", exp,
((char_u **)ga.ga_data)[i]);
if (delete_recursive(NameBuff) != 0)
result = -1;
}
ga_clear_strings(&ga);
}
else
result = -1;
(void)mch_rmdir(exp);
vim_free(exp);
}
else
result = mch_remove(name) == 0 ? 0 : -1;
return result;
}
#endif
#if defined(TEMPDIRNAMES) || defined(PROTO)
static long temp_count = 0; 
void
vim_deltempdir(void)
{
if (vim_tempdir != NULL)
{
gettail(vim_tempdir)[-1] = NUL;
delete_recursive(vim_tempdir);
VIM_CLEAR(vim_tempdir);
}
}
static void
vim_settempdir(char_u *tempdir)
{
char_u *buf;
buf = alloc(MAXPATHL + 2);
if (buf != NULL)
{
if (vim_FullName(tempdir, buf, MAXPATHL, FALSE) == FAIL)
STRCPY(buf, tempdir);
add_pathsep(buf);
vim_tempdir = vim_strsave(buf);
vim_free(buf);
}
}
#endif
char_u *
vim_tempname(
int extra_char UNUSED, 
int keep UNUSED)
{
#if defined(USE_TMPNAM)
char_u itmp[L_tmpnam]; 
#elif defined(MSWIN)
WCHAR itmp[TEMPNAMELEN];
#else
char_u itmp[TEMPNAMELEN];
#endif
#if defined(TEMPDIRNAMES)
static char *(tempdirs[]) = {TEMPDIRNAMES};
int i;
#if !defined(EEXIST)
stat_T st;
#endif
if (vim_tempdir == NULL)
{
for (i = 0; i < (int)(sizeof(tempdirs) / sizeof(char *)); ++i)
{
#if !defined(HAVE_MKDTEMP)
size_t itmplen;
long nr;
long off;
#endif
expand_env((char_u *)tempdirs[i], itmp, TEMPNAMELEN - 20);
if (itmp[0] != '$' && mch_isdir(itmp))
{
add_pathsep(itmp);
#if defined(HAVE_MKDTEMP)
{
#if defined(UNIX) || defined(VMS)
mode_t umask_save = umask(077);
#endif
STRCAT(itmp, "vXXXXXX");
if (mkdtemp((char *)itmp) != NULL)
vim_settempdir(itmp);
#if defined(UNIX) || defined(VMS)
(void)umask(umask_save);
#endif
}
#else
nr = (mch_get_pid() + (long)time(NULL)) % 1000000L;
itmplen = STRLEN(itmp);
for (off = 0; off < 10000L; ++off)
{
int r;
#if defined(UNIX) || defined(VMS)
mode_t umask_save;
#endif
sprintf((char *)itmp + itmplen, "v%ld", nr + off);
#if !defined(EEXIST)
if (mch_stat((char *)itmp, &st) >= 0)
continue;
#endif
#if defined(UNIX) || defined(VMS)
umask_save = umask(077);
#endif
r = vim_mkdir(itmp, 0700);
#if defined(UNIX) || defined(VMS)
(void)umask(umask_save);
#endif
if (r == 0)
{
vim_settempdir(itmp);
break;
}
#if defined(EEXIST)
if (errno != EEXIST)
#endif
break;
}
#endif 
if (vim_tempdir != NULL)
break;
}
}
}
if (vim_tempdir != NULL)
{
sprintf((char *)itmp, "%s%ld", vim_tempdir, temp_count++);
return vim_strsave(itmp);
}
return NULL;
#else 
#if defined(MSWIN)
WCHAR wszTempFile[_MAX_PATH + 1];
WCHAR buf4[4];
char_u *retval;
char_u *p;
wcscpy(itmp, L"");
if (GetTempPathW(_MAX_PATH, wszTempFile) == 0)
{
wszTempFile[0] = L'.'; 
wszTempFile[1] = NUL;
}
wcscpy(buf4, L"VIM");
buf4[2] = extra_char; 
if (GetTempFileNameW(wszTempFile, buf4, 0, itmp) == 0)
return NULL;
if (!keep)
(void)DeleteFileW(itmp);
retval = utf16_to_enc(itmp, NULL);
if (*p_shcf == '-' || p_ssl)
for (p = retval; *p; ++p)
if (*p == '\\')
*p = '/';
return retval;
#else 
#if defined(USE_TMPNAM)
char_u *p;
p = tmpnam((char *)itmp);
if (p == NULL || *p == NUL)
return NULL;
#else
char_u *p;
#if defined(VMS_TEMPNAM)
sprintf((char *)itmp, "VIM%c", extra_char);
p = (char_u *)tempnam("tmp:", (char *)itmp);
if (p != NULL)
{
STRCPY(itmp, p);
STRCAT(itmp, ".txt");
free(p);
}
else
return NULL;
#else
STRCPY(itmp, TEMPNAME);
if ((p = vim_strchr(itmp, '?')) != NULL)
*p = extra_char;
if (mktemp((char *)itmp) == NULL)
return NULL;
#endif
#endif
return vim_strsave(itmp);
#endif 
#endif 
}
#if defined(BACKSLASH_IN_FILENAME) || defined(PROTO)
void
forward_slash(char_u *fname)
{
char_u *p;
if (path_with_url(fname))
return;
for (p = fname; *p != NUL; ++p)
if (enc_dbcs != 0 && (*mb_ptr2len)(p) > 1)
++p;
else if (*p == '\\')
*p = '/';
}
#endif
int
match_file_pat(
char_u *pattern, 
regprog_T **prog, 
char_u *fname, 
char_u *sfname, 
char_u *tail, 
int allow_dirs) 
{
regmatch_T regmatch;
int result = FALSE;
regmatch.rm_ic = p_fic; 
if (prog != NULL)
regmatch.regprog = *prog;
else
regmatch.regprog = vim_regcomp(pattern, RE_MAGIC);
if (regmatch.regprog != NULL
&& ((allow_dirs
&& (vim_regexec(&regmatch, fname, (colnr_T)0)
|| (sfname != NULL
&& vim_regexec(&regmatch, sfname, (colnr_T)0))))
|| (!allow_dirs && vim_regexec(&regmatch, tail, (colnr_T)0))))
result = TRUE;
if (prog != NULL)
*prog = regmatch.regprog;
else
vim_regfree(regmatch.regprog);
return result;
}
#if defined(FEAT_WILDIGN) || defined(PROTO)
int
match_file_list(char_u *list, char_u *sfname, char_u *ffname)
{
char_u buf[100];
char_u *tail;
char_u *regpat;
char allow_dirs;
int match;
char_u *p;
tail = gettail(sfname);
p = list;
while (*p)
{
copy_option_part(&p, buf, 100, ",");
regpat = file_pat_to_reg_pat(buf, NULL, &allow_dirs, FALSE);
if (regpat == NULL)
break;
match = match_file_pat(regpat, NULL, ffname, sfname,
tail, (int)allow_dirs);
vim_free(regpat);
if (match)
return TRUE;
}
return FALSE;
}
#endif
char_u *
file_pat_to_reg_pat(
char_u *pat,
char_u *pat_end, 
char *allow_dirs, 
int no_bslash UNUSED) 
{
int size = 2; 
char_u *endp;
char_u *reg_pat;
char_u *p;
int i;
int nested = 0;
int add_dollar = TRUE;
if (allow_dirs != NULL)
*allow_dirs = FALSE;
if (pat_end == NULL)
pat_end = pat + STRLEN(pat);
for (p = pat; p < pat_end; p++)
{
switch (*p)
{
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
if (enc_dbcs != 0 && (*mb_ptr2len)(p) > 1)
{
++p;
++size;
}
break;
}
}
reg_pat = alloc(size + 1);
if (reg_pat == NULL)
return NULL;
i = 0;
if (pat[0] == '*')
while (pat[0] == '*' && pat < pat_end - 1)
pat++;
else
reg_pat[i++] = '^';
endp = pat_end - 1;
if (endp >= pat && *endp == '*')
{
while (endp - pat > 0 && *endp == '*')
endp--;
add_dollar = FALSE;
}
for (p = pat; *p && nested >= 0 && p <= endp; p++)
{
switch (*p)
{
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
if (!no_bslash)
{
if ((vim_isfilec(p[1]) || p[1] == '*' || p[1] == '?')
&& p[1] != '+')
{
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
)
reg_pat[i++] = '?';
else
if (*p == ',' || *p == '%' || *p == '#'
|| vim_isspace(*p) || *p == '{' || *p == '}')
reg_pat[i++] = *p;
else if (*p == '\\' && p[1] == '\\' && p[2] == '{')
{
reg_pat[i++] = '\\';
reg_pat[i++] = '{';
p += 2;
}
else
{
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
if (nested)
{
reg_pat[i++] = '\\';
reg_pat[i++] = '|';
}
else
reg_pat[i++] = ',';
break;
default:
if (enc_dbcs != 0 && (*mb_ptr2len)(p) > 1)
reg_pat[i++] = *p++;
else if (allow_dirs != NULL && vim_ispathsep(*p))
*allow_dirs = TRUE;
reg_pat[i++] = *p;
break;
}
}
if (add_dollar)
reg_pat[i++] = '$';
reg_pat[i] = NUL;
if (nested != 0)
{
if (nested < 0)
emsg(_("E219: Missing {."));
else
emsg(_("E220: Missing }."));
VIM_CLEAR(reg_pat);
}
return reg_pat;
}
#if defined(EINTR) || defined(PROTO)
long
read_eintr(int fd, void *buf, size_t bufsize)
{
long ret;
for (;;)
{
ret = vim_read(fd, buf, bufsize);
if (ret >= 0 || errno != EINTR)
break;
}
return ret;
}
long
write_eintr(int fd, void *buf, size_t bufsize)
{
long ret = 0;
long wlen;
while (ret < (long)bufsize)
{
wlen = vim_write(fd, (char *)buf + ret, bufsize - ret);
if (wlen < 0)
{
if (errno != EINTR)
break;
}
else
ret += wlen;
}
return ret;
}
#endif
