












#include "vim.h"

#if defined(HAVE_UTIME) && defined(HAVE_UTIME_H)
#include <utime.h> 
#endif

#define SMALLBUFSIZE 256 




struct bw_info
{
int bw_fd; 
char_u *bw_buf; 
int bw_len; 
int bw_flags; 
#if defined(FEAT_CRYPT)
buf_T *bw_buffer; 
#endif
char_u bw_rest[CONV_RESTLEN]; 
int bw_restlen; 
int bw_first; 
char_u *bw_conv_buf; 
size_t bw_conv_buflen; 
int bw_conv_error; 
linenr_T bw_conv_error_lnum; 
linenr_T bw_start_lnum; 
#if defined(USE_ICONV)
iconv_t bw_iconv_fd; 
#endif
};





static int
ucs2bytes(
unsigned c, 
char_u **pp, 
int flags) 
{
char_u *p = *pp;
int error = FALSE;
int cc;


if (flags & FIO_UCS4)
{
if (flags & FIO_ENDIAN_L)
{
*p++ = c;
*p++ = (c >> 8);
*p++ = (c >> 16);
*p++ = (c >> 24);
}
else
{
*p++ = (c >> 24);
*p++ = (c >> 16);
*p++ = (c >> 8);
*p++ = c;
}
}
else if (flags & (FIO_UCS2 | FIO_UTF16))
{
if (c >= 0x10000)
{
if (flags & FIO_UTF16)
{


c -= 0x10000;
if (c >= 0x100000)
error = TRUE;
cc = ((c >> 10) & 0x3ff) + 0xd800;
if (flags & FIO_ENDIAN_L)
{
*p++ = cc;
*p++ = ((unsigned)cc >> 8);
}
else
{
*p++ = ((unsigned)cc >> 8);
*p++ = cc;
}
c = (c & 0x3ff) + 0xdc00;
}
else
error = TRUE;
}
if (flags & FIO_ENDIAN_L)
{
*p++ = c;
*p++ = (c >> 8);
}
else
{
*p++ = (c >> 8);
*p++ = c;
}
}
else 
{
if (c >= 0x100)
{
error = TRUE;
*p++ = 0xBF;
}
else
*p++ = c;
}

*pp = p;
return error;
}







static int
buf_write_bytes(struct bw_info *ip)
{
int wlen;
char_u *buf = ip->bw_buf; 
int len = ip->bw_len; 
int flags = ip->bw_flags; 


if (!(flags & FIO_NOCONVERT))
{
char_u *p;
unsigned c;
int n;

if (flags & FIO_UTF8)
{

p = ip->bw_conv_buf; 
for (wlen = 0; wlen < len; ++wlen)
p += utf_char2bytes(buf[wlen], p);
buf = ip->bw_conv_buf;
len = (int)(p - ip->bw_conv_buf);
}
else if (flags & (FIO_UCS4 | FIO_UTF16 | FIO_UCS2 | FIO_LATIN1))
{


if (flags & FIO_LATIN1)
p = buf; 
else
p = ip->bw_conv_buf; 
for (wlen = 0; wlen < len; wlen += n)
{
if (wlen == 0 && ip->bw_restlen != 0)
{
int l;




l = CONV_RESTLEN - ip->bw_restlen;
if (l > len)
l = len;
mch_memmove(ip->bw_rest + ip->bw_restlen, buf, (size_t)l);
n = utf_ptr2len_len(ip->bw_rest, ip->bw_restlen + l);
if (n > ip->bw_restlen + len)
{



if (ip->bw_restlen + len > CONV_RESTLEN)
return FAIL;
ip->bw_restlen += len;
break;
}
if (n > 1)
c = utf_ptr2char(ip->bw_rest);
else
c = ip->bw_rest[0];
if (n >= ip->bw_restlen)
{
n -= ip->bw_restlen;
ip->bw_restlen = 0;
}
else
{
ip->bw_restlen -= n;
mch_memmove(ip->bw_rest, ip->bw_rest + n,
(size_t)ip->bw_restlen);
n = 0;
}
}
else
{
n = utf_ptr2len_len(buf + wlen, len - wlen);
if (n > len - wlen)
{



if (len - wlen > CONV_RESTLEN)
return FAIL;
ip->bw_restlen = len - wlen;
mch_memmove(ip->bw_rest, buf + wlen,
(size_t)ip->bw_restlen);
break;
}
if (n > 1)
c = utf_ptr2char(buf + wlen);
else
c = buf[wlen];
}

if (ucs2bytes(c, &p, flags) && !ip->bw_conv_error)
{
ip->bw_conv_error = TRUE;
ip->bw_conv_error_lnum = ip->bw_start_lnum;
}
if (c == NL)
++ip->bw_start_lnum;
}
if (flags & FIO_LATIN1)
len = (int)(p - buf);
else
{
buf = ip->bw_conv_buf;
len = (int)(p - ip->bw_conv_buf);
}
}

#if defined(MSWIN)
else if (flags & FIO_CODEPAGE)
{


char_u *from;
size_t fromlen;
char_u *to;
int u8c;
BOOL bad = FALSE;
int needed;

if (ip->bw_restlen > 0)
{



fromlen = len + ip->bw_restlen;
from = ip->bw_conv_buf + ip->bw_conv_buflen - fromlen;
mch_memmove(from, ip->bw_rest, (size_t)ip->bw_restlen);
mch_memmove(from + ip->bw_restlen, buf, (size_t)len);
}
else
{
from = buf;
fromlen = len;
}

to = ip->bw_conv_buf;
if (enc_utf8)
{


while (fromlen > 0)
{
n = (int)utf_ptr2len_len(from, (int)fromlen);
if (n > (int)fromlen) 
break;
u8c = utf_ptr2char(from);
*to++ = (u8c & 0xff);
*to++ = (u8c >> 8);
fromlen -= n;
from += n;
}



if (fromlen > CONV_RESTLEN)
{

ip->bw_conv_error = TRUE;
return FAIL;
}
mch_memmove(ip->bw_rest, from, fromlen);
ip->bw_restlen = (int)fromlen;
}
else
{


ip->bw_restlen = 0;
needed = MultiByteToWideChar(enc_codepage,
MB_ERR_INVALID_CHARS, (LPCSTR)from, (int)fromlen,
NULL, 0);
if (needed == 0)
{

needed = MultiByteToWideChar(enc_codepage,
MB_ERR_INVALID_CHARS, (LPCSTR)from, (int)fromlen - 1,
NULL, 0);
if (needed == 0)
{

ip->bw_conv_error = TRUE;
return FAIL;
}

ip->bw_rest[0] = from[fromlen - 1];
ip->bw_restlen = 1;
}
needed = MultiByteToWideChar(enc_codepage, MB_ERR_INVALID_CHARS,
(LPCSTR)from, (int)(fromlen - ip->bw_restlen),
(LPWSTR)to, needed);
if (needed == 0)
{

ip->bw_conv_error = TRUE;
return FAIL;
}
to += needed * 2;
}

fromlen = to - ip->bw_conv_buf;
buf = to;
#if defined(CP_UTF8)
if (FIO_GET_CP(flags) == CP_UTF8)
{


for (from = ip->bw_conv_buf; fromlen > 1; fromlen -= 2)
{
u8c = *from++;
u8c += (*from++ << 8);
to += utf_char2bytes(u8c, to);
if (to + 6 >= ip->bw_conv_buf + ip->bw_conv_buflen)
{
ip->bw_conv_error = TRUE;
return FAIL;
}
}
len = (int)(to - buf);
}
else
#endif
{




len = WideCharToMultiByte(FIO_GET_CP(flags), 0,
(LPCWSTR)ip->bw_conv_buf, (int)fromlen / sizeof(WCHAR),
(LPSTR)to, (int)(ip->bw_conv_buflen - fromlen), 0,
&bad);
if (bad)
{
ip->bw_conv_error = TRUE;
return FAIL;
}
}
}
#endif

#if defined(MACOS_CONVERT)
else if (flags & FIO_MACROMAN)
{

char_u *from;
size_t fromlen;

if (ip->bw_restlen > 0)
{



fromlen = len + ip->bw_restlen;
from = ip->bw_conv_buf + ip->bw_conv_buflen - fromlen;
mch_memmove(from, ip->bw_rest, (size_t)ip->bw_restlen);
mch_memmove(from + ip->bw_restlen, buf, (size_t)len);
}
else
{
from = buf;
fromlen = len;
}

if (enc2macroman(from, fromlen,
ip->bw_conv_buf, &len, ip->bw_conv_buflen,
ip->bw_rest, &ip->bw_restlen) == FAIL)
{
ip->bw_conv_error = TRUE;
return FAIL;
}
buf = ip->bw_conv_buf;
}
#endif

#if defined(USE_ICONV)
if (ip->bw_iconv_fd != (iconv_t)-1)
{
const char *from;
size_t fromlen;
char *to;
size_t tolen;


if (ip->bw_restlen > 0)
{
char *fp;




fromlen = len + ip->bw_restlen;
fp = (char *)ip->bw_conv_buf + ip->bw_conv_buflen - fromlen;
mch_memmove(fp, ip->bw_rest, (size_t)ip->bw_restlen);
mch_memmove(fp + ip->bw_restlen, buf, (size_t)len);
from = fp;
tolen = ip->bw_conv_buflen - fromlen;
}
else
{
from = (const char *)buf;
fromlen = len;
tolen = ip->bw_conv_buflen;
}
to = (char *)ip->bw_conv_buf;

if (ip->bw_first)
{
size_t save_len = tolen;


(void)iconv(ip->bw_iconv_fd, NULL, NULL, &to, &tolen);



if (to == NULL)
{
to = (char *)ip->bw_conv_buf;
tolen = save_len;
}
ip->bw_first = FALSE;
}


if ((iconv(ip->bw_iconv_fd, (void *)&from, &fromlen, &to, &tolen)
== (size_t)-1 && ICONV_ERRNO != ICONV_EINVAL)
|| fromlen > CONV_RESTLEN)
{
ip->bw_conv_error = TRUE;
return FAIL;
}


if (fromlen > 0)
mch_memmove(ip->bw_rest, (void *)from, fromlen);
ip->bw_restlen = (int)fromlen;

buf = ip->bw_conv_buf;
len = (int)((char_u *)to - ip->bw_conv_buf);
}
#endif
}

if (ip->bw_fd < 0)

return OK;

#if defined(FEAT_CRYPT)
if (flags & FIO_ENCRYPTED)
{


#if defined(CRYPT_NOT_INPLACE)
if (crypt_works_inplace(ip->bw_buffer->b_cryptstate))
{
#endif
crypt_encode_inplace(ip->bw_buffer->b_cryptstate, buf, len);
#if defined(CRYPT_NOT_INPLACE)
}
else
{
char_u *outbuf;

len = crypt_encode_alloc(curbuf->b_cryptstate, buf, len, &outbuf);
if (len == 0)
return OK; 
wlen = write_eintr(ip->bw_fd, outbuf, len);
vim_free(outbuf);
return (wlen < len) ? FAIL : OK;
}
#endif
}
#endif

wlen = write_eintr(ip->bw_fd, buf, len);
return (wlen < len) ? FAIL : OK;
}






static int
check_mtime(buf_T *buf, stat_T *st)
{
if (buf->b_mtime_read != 0
&& time_differs((long)st->st_mtime, buf->b_mtime_read))
{
msg_scroll = TRUE; 
msg_silent = 0; 

msg_attr(_("WARNING: The file has been changed since reading it!!!"),
HL_ATTR(HLF_E));
if (ask_yesno((char_u *)_("Do you really want to write to it"),
TRUE) == 'n')
return FAIL;
msg_scroll = FALSE; 
}
return OK;
}





static int
make_bom(char_u *buf, char_u *name)
{
int flags;
char_u *p;

flags = get_fio_flags(name);


if (flags == FIO_LATIN1 || flags == 0)
return 0;

if (flags == FIO_UTF8) 
{
buf[0] = 0xef;
buf[1] = 0xbb;
buf[2] = 0xbf;
return 3;
}
p = buf;
(void)ucs2bytes(0xfeff, &p, flags);
return (int)(p - buf);
}

#if defined(UNIX)
static void
set_file_time(
char_u *fname,
time_t atime, 
time_t mtime) 
{
#if defined(HAVE_UTIME) && defined(HAVE_UTIME_H)
struct utimbuf buf;

buf.actime = atime;
buf.modtime = mtime;
(void)utime((char *)fname, &buf);
#else
#if defined(HAVE_UTIMES)
struct timeval tvp[2];

tvp[0].tv_sec = atime;
tvp[0].tv_usec = 0;
tvp[1].tv_sec = mtime;
tvp[1].tv_usec = 0;
#if defined(NeXT)
(void)utimes((char *)fname, tvp);
#else
(void)utimes((char *)fname, (const struct timeval *)&tvp);
#endif
#endif
#endif
}
#endif 

















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
int filtering)
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
char_u *errmsg = NULL;
int errmsg_allocated = FALSE;
char_u *errnum = NULL;
char_u *buffer;
char_u smallbuf[SMALLBUFSIZE];
char_u *backup_ext;
int bufsize;
long perm; 
int retval = OK;
int newfile = FALSE; 
int msg_save = msg_scroll;
int overwriting; 
int no_eol = FALSE; 
int device = FALSE; 
stat_T st_old;
int prev_got_int = got_int;
int checking_conversion;
int file_readonly = FALSE; 
static char *err_readonly = "is read-only (cannot override: \"W\" in 'cpoptions')";
#if defined(UNIX) 
int made_writable = FALSE; 
#endif

int whole = (start == 1 && end == buf->b_ml.ml_line_count);
linenr_T old_line_count = buf->b_ml.ml_line_count;
int attr;
int fileformat;
int write_bin;
struct bw_info write_info; 
int converted = FALSE;
int notconverted = FALSE;
char_u *fenc; 
char_u *fenc_tofree = NULL; 
int wb_flags = 0;
#if defined(HAVE_ACL)
vim_acl_T acl = NULL; 

#endif
#if defined(FEAT_PERSISTENT_UNDO)
int write_undo_file = FALSE;
context_sha256_T sha_ctx;
#endif
unsigned int bkc = get_bkc_value(buf);
pos_T orig_start = buf->b_op_start;
pos_T orig_end = buf->b_op_end;

if (fname == NULL || *fname == NUL) 
return FAIL;
if (buf->b_ml.ml_mfp == NULL)
{


emsg(_(e_emptybuf));
return FAIL;
}



if (check_secure())
return FAIL;


if (STRLEN(fname) >= MAXPATHL)
{
emsg(_(e_longname));
return FAIL;
}


write_info.bw_conv_buf = NULL;
write_info.bw_conv_error = FALSE;
write_info.bw_conv_error_lnum = 0;
write_info.bw_restlen = 0;
#if defined(USE_ICONV)
write_info.bw_iconv_fd = (iconv_t)-1;
#endif
#if defined(FEAT_CRYPT)
write_info.bw_buffer = buf;
#endif



ex_no_reprint = TRUE;






if (buf->b_ffname == NULL
&& reset_changed
&& whole
&& buf == curbuf
#if defined(FEAT_QUICKFIX)
&& !bt_nofilename(buf)
#endif
&& !filtering
&& (!append || vim_strchr(p_cpo, CPO_FNAMEAPP) != NULL)
&& vim_strchr(p_cpo, CPO_FNAMEW) != NULL)
{
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

if (exiting)
settmode(TMODE_COOK); 

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

if (append)
{
if (!(did_cmd = apply_autocmds_exarg(EVENT_FILEAPPENDCMD,
sfname, sfname, FALSE, curbuf, eap)))
{
#if defined(FEAT_QUICKFIX)
if (overwriting && bt_nofilename(curbuf))
nofile_err = TRUE;
else
#endif
apply_autocmds_exarg(EVENT_FILEAPPENDPRE,
sfname, sfname, FALSE, curbuf, eap);
}
}
else if (filtering)
{
apply_autocmds_exarg(EVENT_FILTERWRITEPRE,
NULL, sfname, FALSE, curbuf, eap);
}
else if (reset_changed && whole)
{
int was_changed = curbufIsChanged();

did_cmd = apply_autocmds_exarg(EVENT_BUFWRITECMD,
sfname, sfname, FALSE, curbuf, eap);
if (did_cmd)
{
if (was_changed && !curbufIsChanged())
{



u_unchanged(curbuf);
u_update_save_nr(curbuf);
}
}
else
{
#if defined(FEAT_QUICKFIX)
if (overwriting && bt_nofilename(curbuf))
nofile_err = TRUE;
else
#endif
apply_autocmds_exarg(EVENT_BUFWRITEPRE,
sfname, sfname, FALSE, curbuf, eap);
}
}
else
{
if (!(did_cmd = apply_autocmds_exarg(EVENT_FILEWRITECMD,
sfname, sfname, FALSE, curbuf, eap)))
{
#if defined(FEAT_QUICKFIX)
if (overwriting && bt_nofilename(curbuf))
nofile_err = TRUE;
else
#endif
apply_autocmds_exarg(EVENT_FILEWRITEPRE,
sfname, sfname, FALSE, curbuf, eap);
}
}


aucmd_restbuf(&aco);





if (!bufref_valid(&bufref))
buf = NULL;
if (buf == NULL || (buf->b_ml.ml_mfp == NULL && !empty_memline)
|| did_cmd || nofile_err
#if defined(FEAT_EVAL)
|| aborting()
#endif
)
{
if (buf != NULL && cmdmod.lockmarks)
{

buf->b_op_start = orig_start;
buf->b_op_end = orig_end;
}

--no_wait_return;
msg_scroll = msg_save;
if (nofile_err)
emsg(_("E676: No matching autocommands for acwrite buffer"));

if (nofile_err
#if defined(FEAT_EVAL)
|| aborting()
#endif
)


return FAIL;
if (did_cmd)
{
if (buf == NULL)


return OK;
if (overwriting)
{

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
#if defined(FEAT_EVAL)
if (!aborting())
#endif
emsg(_("E203: Autocommands deleted or unloaded buffer to be written"));
return FAIL;
}





if (buf->b_ml.ml_line_count != old_line_count)
{
if (whole) 
end = buf->b_ml.ml_line_count;
else if (buf->b_ml.ml_line_count > old_line_count) 
end += buf->b_ml.ml_line_count - old_line_count;
else 
{
end -= old_line_count - buf->b_ml.ml_line_count;
if (end < start)
{
--no_wait_return;
msg_scroll = msg_save;
emsg(_("E204: Autocommand changed number of lines in unexpected way"));
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

if (cmdmod.lockmarks)
{

buf->b_op_start = orig_start;
buf->b_op_end = orig_end;
}

#if defined(FEAT_NETBEANS_INTG)
if (netbeans_active() && isNetbeansBuffer(buf))
{
if (whole)
{


if (buf->b_changed || isNetbeansModified(buf))
{
--no_wait_return; 
msg_scroll = msg_save;
netbeans_save_buffer(buf); 
return retval;
}
else
{
errnum = (char_u *)"E656: ";
errmsg = (char_u *)_("NetBeans disallows writes of unmodified buffers");
buffer = NULL;
goto fail;
}
}
else
{
errnum = (char_u *)"E657: ";
errmsg = (char_u *)_("Partial writes disallowed for NetBeans buffers");
buffer = NULL;
goto fail;
}
}
#endif

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

buffer = alloc(WRITEBUFSIZE);
if (buffer == NULL) 


{
buffer = smallbuf;
bufsize = SMALLBUFSIZE;
}
else
bufsize = WRITEBUFSIZE;


#if defined(UNIX)
st_old.st_dev = 0;
st_old.st_ino = 0;
perm = -1;
if (mch_stat((char *)fname, &st_old) < 0)
newfile = TRUE;
else
{
perm = st_old.st_mode;
if (!S_ISREG(st_old.st_mode)) 
{
if (S_ISDIR(st_old.st_mode))
{
errnum = (char_u *)"E502: ";
errmsg = (char_u *)_("is a directory");
goto fail;
}
if (mch_nodetype(fname) != NODE_WRITABLE)
{
errnum = (char_u *)"E503: ";
errmsg = (char_u *)_("is not a file or writable device");
goto fail;
}


device = TRUE;
newfile = TRUE;
perm = -1;
}
}
#else 

c = mch_nodetype(fname);
if (c == NODE_OTHER)
{
errnum = (char_u *)"E503: ";
errmsg = (char_u *)_("is not a file or writable device");
goto fail;
}
if (c == NODE_WRITABLE)
{
#if defined(MSWIN)


if (!p_odev)
{
errnum = (char_u *)"E796: ";
errmsg = (char_u *)_("writing to device disabled with 'opendevice' option");
goto fail;
}
#endif
device = TRUE;
newfile = TRUE;
perm = -1;
}
else
{
perm = mch_getperm(fname);
if (perm < 0)
newfile = TRUE;
else if (mch_isdir(fname))
{
errnum = (char_u *)"E502: ";
errmsg = (char_u *)_("is a directory");
goto fail;
}
if (overwriting)
(void)mch_stat((char *)fname, &st_old);
}
#endif 

if (!device && !newfile)
{


file_readonly = check_file_readonly(fname, (int)perm);

if (!forceit && file_readonly)
{
if (vim_strchr(p_cpo, CPO_FWRITE) != NULL)
{
errnum = (char_u *)"E504: ";
errmsg = (char_u *)_(err_readonly);
}
else
{
errnum = (char_u *)"E505: ";
errmsg = (char_u *)_("is read-only (add ! to override)");
}
goto fail;
}


if (overwriting)
{
retval = check_mtime(buf, &st_old);
if (retval == FAIL)
goto fail;
}
}

#if defined(HAVE_ACL)

if (!newfile)
acl = mch_get_acl(fname);
#endif


dobackup = (p_wb || p_bk || *p_pm != NUL);
#if defined(FEAT_WILDIGN)
if (dobackup && *p_bsk != NUL && match_file_list(p_bsk, sfname, ffname))
dobackup = FALSE;
#endif




prev_got_int = got_int;
got_int = FALSE;


buf->b_saving = TRUE;







if (!(append && *p_pm == NUL) && !filtering && perm >= 0 && dobackup)
{
#if defined(UNIX) || defined(MSWIN)
stat_T st;
#endif

if ((bkc & BKC_YES) || append) 
backup_copy = TRUE;
#if defined(UNIX) || defined(MSWIN)
else if ((bkc & BKC_AUTO)) 
{
int i;

#if defined(UNIX)





if (st_old.st_nlink > 1
|| mch_lstat((char *)fname, &st) < 0
|| st.st_dev != st_old.st_dev
|| st.st_ino != st_old.st_ino
#if !defined(HAVE_FCHOWN)
|| st.st_uid != st_old.st_uid
|| st.st_gid != st_old.st_gid
#endif
)
backup_copy = TRUE;
else
#else
#if defined(MSWIN)

if (mch_is_linked(fname))
backup_copy = TRUE;
else
#endif
#endif
{




STRCPY(IObuff, fname);
for (i = 4913; ; i += 123)
{
sprintf((char *)gettail(IObuff), "%d", i);
if (mch_lstat((char *)IObuff, &st) < 0)
break;
}
fd = mch_open((char *)IObuff,
O_CREAT|O_WRONLY|O_EXCL|O_NOFOLLOW, perm);
if (fd < 0) 
backup_copy = TRUE;
else
{
#if defined(UNIX)
#if defined(HAVE_FCHOWN)
vim_ignored = fchown(fd, st_old.st_uid, st_old.st_gid);
#endif
if (mch_stat((char *)IObuff, &st) < 0
|| st.st_uid != st_old.st_uid
|| st.st_gid != st_old.st_gid
|| (long)st.st_mode != perm)
backup_copy = TRUE;
#endif


close(fd);
mch_remove(IObuff);
#if defined(MSWIN)



{
int try;

for (try = 0; try < 10; ++try)
{
if (mch_lstat((char *)IObuff, &st) < 0)
break;
ui_delay(50L, TRUE); 
mch_remove(IObuff);
}
}
#endif
}
}
}


if ((bkc & BKC_BREAKSYMLINK) || (bkc & BKC_BREAKHARDLINK))
{
#if defined(UNIX)
int lstat_res;

lstat_res = mch_lstat((char *)fname, &st);


if ((bkc & BKC_BREAKSYMLINK)
&& lstat_res == 0
&& st.st_ino != st_old.st_ino)
backup_copy = FALSE;


if ((bkc & BKC_BREAKHARDLINK)
&& st_old.st_nlink > 1
&& (lstat_res != 0 || st.st_ino == st_old.st_ino))
backup_copy = FALSE;
#else
#if defined(MSWIN)

if ((bkc & BKC_BREAKSYMLINK) && mch_is_symbolic_link(fname))
backup_copy = FALSE;


if ((bkc & BKC_BREAKHARDLINK) && mch_is_hard_link(fname))
backup_copy = FALSE;
#endif
#endif
}

#endif


if (*p_bex == NUL)
backup_ext = (char_u *)".bak";
else
backup_ext = p_bex;

if (backup_copy
&& (fd = mch_open((char *)fname, O_RDONLY | O_EXTRA, 0)) >= 0)
{
int bfd;
char_u *copybuf, *wp;
int some_error = FALSE;
stat_T st_new;
char_u *dirp;
char_u *rootname;
#if defined(UNIX) || defined(MSWIN)
char_u *p;
#endif
#if defined(UNIX)
int did_set_shortname;
mode_t umask_save;
#endif

copybuf = alloc(WRITEBUFSIZE + 1);
if (copybuf == NULL)
{
some_error = TRUE; 
goto nobackup;
}











dirp = p_bdir;
while (*dirp)
{
#if defined(UNIX)
st_new.st_ino = 0;
st_new.st_dev = 0;
st_new.st_gid = 0;
#endif


(void)copy_option_part(&dirp, copybuf, WRITEBUFSIZE, ",");

#if defined(UNIX) || defined(MSWIN)
p = copybuf + STRLEN(copybuf);
if (after_pathsep(copybuf, p) && p[-1] == p[-2])

if ((p = make_percent_swname(copybuf, fname)) != NULL)
{
backup = modname(p, backup_ext, FALSE);
vim_free(p);
}
#endif
rootname = get_file_in_dir(fname, copybuf);
if (rootname == NULL)
{
some_error = TRUE; 
goto nobackup;
}

#if defined(UNIX)
did_set_shortname = FALSE;
#endif


for (;;)
{

if (backup == NULL)
backup = buf_modname((buf->b_p_sn || buf->b_shortname),
rootname, backup_ext, FALSE);
if (backup == NULL)
{
vim_free(rootname);
some_error = TRUE; 
goto nobackup;
}


if (mch_stat((char *)backup, &st_new) >= 0)
{
#if defined(UNIX)





if (st_new.st_dev == st_old.st_dev
&& st_new.st_ino == st_old.st_ino)
{
VIM_CLEAR(backup); 

if (!(buf->b_shortname || buf->b_p_sn))
{
buf->b_shortname = TRUE;
did_set_shortname = TRUE;
continue;
}

if (did_set_shortname)
buf->b_shortname = FALSE;
break;
}
#endif




if (!p_bk)
{
wp = backup + STRLEN(backup) - 1
- STRLEN(backup_ext);
if (wp < backup) 
wp = backup;
*wp = 'z';
while (*wp > 'a'
&& mch_stat((char *)backup, &st_new) >= 0)
--*wp;

if (*wp == 'a')
VIM_CLEAR(backup);
}
}
break;
}
vim_free(rootname);


if (backup != NULL)
{

mch_remove(backup);



#if defined(UNIX)
umask_save = umask(0);
#endif
bfd = mch_open((char *)backup,
O_WRONLY|O_CREAT|O_EXTRA|O_EXCL|O_NOFOLLOW,
perm & 0777);
#if defined(UNIX)
(void)umask(umask_save);
#endif
if (bfd < 0)
VIM_CLEAR(backup);
else
{



#if !defined(UNIX)
(void)mch_setperm(backup, perm & 0777);
#else




if (st_new.st_gid != st_old.st_gid
#if defined(HAVE_FCHOWN)
&& fchown(bfd, (uid_t)-1, st_old.st_gid) != 0
#endif
)
mch_setperm(backup,
(perm & 0707) | ((perm & 07) << 3));
#if defined(HAVE_SELINUX) || defined(HAVE_SMACK)
mch_copy_sec(fname, backup);
#endif
#endif


write_info.bw_fd = bfd;
write_info.bw_buf = copybuf;
write_info.bw_flags = FIO_NOCONVERT;
while ((write_info.bw_len = read_eintr(fd, copybuf,
WRITEBUFSIZE)) > 0)
{
if (buf_write_bytes(&write_info) == FAIL)
{
errmsg = (char_u *)_("E506: Can't write to backup file (add ! to override)");
break;
}
ui_breakcheck();
if (got_int)
{
errmsg = (char_u *)_(e_interr);
break;
}
}

if (close(bfd) < 0 && errmsg == NULL)
errmsg = (char_u *)_("E507: Close error for backup file (add ! to override)");
if (write_info.bw_len < 0)
errmsg = (char_u *)_("E508: Can't read file for backup (add ! to override)");
#if defined(UNIX)
set_file_time(backup, st_old.st_atime, st_old.st_mtime);
#endif
#if defined(HAVE_ACL)
mch_set_acl(backup, acl);
#endif
#if defined(HAVE_SELINUX) || defined(HAVE_SMACK)
mch_copy_sec(fname, backup);
#endif
break;
}
}
}
nobackup:
close(fd); 
vim_free(copybuf);

if (backup == NULL && errmsg == NULL)
errmsg = (char_u *)_("E509: Cannot create backup file (add ! to override)");

if ((some_error || errmsg != NULL) && !forceit)
{
retval = FAIL;
goto fail;
}
errmsg = NULL;
}
else
{
char_u *dirp;
char_u *p;
char_u *rootname;






if (file_readonly && vim_strchr(p_cpo, CPO_FWRITE) != NULL)
{
errnum = (char_u *)"E504: ";
errmsg = (char_u *)_(err_readonly);
goto fail;
}




dirp = p_bdir;
while (*dirp)
{

(void)copy_option_part(&dirp, IObuff, IOSIZE, ",");

#if defined(UNIX) || defined(MSWIN)
p = IObuff + STRLEN(IObuff);
if (after_pathsep(IObuff, p) && p[-1] == p[-2])

if ((p = make_percent_swname(IObuff, fname)) != NULL)
{
backup = modname(p, backup_ext, FALSE);
vim_free(p);
}
#endif
if (backup == NULL)
{
rootname = get_file_in_dir(fname, IObuff);
if (rootname == NULL)
backup = NULL;
else
{
backup = buf_modname(
(buf->b_p_sn || buf->b_shortname),
rootname, backup_ext, FALSE);
vim_free(rootname);
}
}

if (backup != NULL)
{



if (!p_bk && mch_getperm(backup) >= 0)
{
p = backup + STRLEN(backup) - 1 - STRLEN(backup_ext);
if (p < backup) 
p = backup;
*p = 'z';
while (*p > 'a' && mch_getperm(backup) >= 0)
--*p;

if (*p == 'a')
VIM_CLEAR(backup);
}
}
if (backup != NULL)
{







if (vim_rename(fname, backup) == 0)
break;

VIM_CLEAR(backup); 
}
}
if (backup == NULL && !forceit)
{
errmsg = (char_u *)_("E510: Can't make backup file (add ! to override)");
goto fail;
}
}
}

#if defined(UNIX)

if (forceit && perm >= 0 && !(perm & 0200) && st_old.st_uid == getuid()
&& vim_strchr(p_cpo, CPO_FWRITE) == NULL)
{
perm |= 0200;
(void)mch_setperm(fname, perm);
made_writable = TRUE;
}
#endif



if (forceit && overwriting && vim_strchr(p_cpo, CPO_KEEPRO) == NULL)
{
buf->b_p_ro = FALSE;
#if defined(FEAT_TITLE)
need_maketitle = TRUE; 
#endif
status_redraw_all(); 
}

if (end > buf->b_ml.ml_line_count)
end = buf->b_ml.ml_line_count;
if (buf->b_ml.ml_flags & ML_EMPTY)
start = end + 1;






if (reset_changed && !newfile && overwriting
&& !(exiting && backup != NULL))
{
ml_preserve(buf, FALSE);
if (got_int)
{
errmsg = (char_u *)_(e_interr);
goto restore_backup;
}
}

#if defined(VMS)
vms_remove_version(fname); 
#endif


wfname = fname;


if (eap != NULL && eap->force_enc != 0)
{
fenc = eap->cmd + eap->force_enc;
fenc = enc_canonize(fenc);
fenc_tofree = fenc;
}
else
fenc = buf->b_p_fenc;


converted = need_conversion(fenc);




if (converted && (enc_utf8 || STRCMP(p_enc, "latin1") == 0))
{
wb_flags = get_fio_flags(fenc);
if (wb_flags & (FIO_UCS2 | FIO_UCS4 | FIO_UTF16 | FIO_UTF8))
{

if (wb_flags & (FIO_UCS2 | FIO_UTF16 | FIO_UTF8))
write_info.bw_conv_buflen = bufsize * 2;
else 
write_info.bw_conv_buflen = bufsize * 4;
write_info.bw_conv_buf = alloc(write_info.bw_conv_buflen);
if (write_info.bw_conv_buf == NULL)
end = 0;
}
}

#if defined(MSWIN)
if (converted && wb_flags == 0 && (wb_flags = get_win_fio_flags(fenc)) != 0)
{

write_info.bw_conv_buflen = bufsize * 4;
write_info.bw_conv_buf = alloc(write_info.bw_conv_buflen);
if (write_info.bw_conv_buf == NULL)
end = 0;
}
#endif

#if defined(MACOS_CONVERT)
if (converted && wb_flags == 0 && (wb_flags = get_mac_fio_flags(fenc)) != 0)
{
write_info.bw_conv_buflen = bufsize * 3;
write_info.bw_conv_buf = alloc(write_info.bw_conv_buflen);
if (write_info.bw_conv_buf == NULL)
end = 0;
}
#endif

#if defined(FEAT_EVAL) || defined(USE_ICONV)
if (converted && wb_flags == 0)
{
#if defined(USE_ICONV)


write_info.bw_iconv_fd = (iconv_t)my_iconv_open(fenc,
enc_utf8 ? (char_u *)"utf-8" : p_enc);
if (write_info.bw_iconv_fd != (iconv_t)-1)
{

write_info.bw_conv_buflen = bufsize * ICONV_MULT;
write_info.bw_conv_buf = alloc(write_info.bw_conv_buflen);
if (write_info.bw_conv_buf == NULL)
end = 0;
write_info.bw_first = TRUE;
}
#if defined(FEAT_EVAL)
else
#endif
#endif

#if defined(FEAT_EVAL)



if (*p_ccv != NUL)
{
wfname = vim_tempname('w', FALSE);
if (wfname == NULL) 
{
errmsg = (char_u *)_("E214: Can't find temp file for writing");
goto restore_backup;
}
}
#endif
}
#endif
if (converted && wb_flags == 0
#if defined(USE_ICONV)
&& write_info.bw_iconv_fd == (iconv_t)-1
#endif
#if defined(FEAT_EVAL)
&& wfname == fname
#endif
)
{
if (!forceit)
{
errmsg = (char_u *)_("E213: Cannot convert (add ! to write without conversion)");
goto restore_backup;
}
notconverted = TRUE;
}




for (checking_conversion = TRUE; ; checking_conversion = FALSE)
{




if (!converted || dobackup)
checking_conversion = FALSE;

if (checking_conversion)
{

fd = -1;
write_info.bw_fd = fd;
}
else
{
#if defined(HAVE_FTRUNCATE)
#define TRUNC_ON_OPEN 0
#else
#define TRUNC_ON_OPEN O_TRUNC
#endif








while ((fd = mch_open((char *)wfname, O_WRONLY | O_EXTRA | (append
? (forceit ? (O_APPEND | O_CREAT) : O_APPEND)
: (O_CREAT | TRUNC_ON_OPEN))
, perm < 0 ? 0666 : (perm & 0777))) < 0)
{



if (errmsg == NULL)
{
#if defined(UNIX)
stat_T st;


if ((!newfile && st_old.st_nlink > 1)
|| (mch_lstat((char *)fname, &st) == 0
&& (st.st_dev != st_old.st_dev
|| st.st_ino != st_old.st_ino)))
errmsg = (char_u *)_("E166: Can't open linked file for writing");
else
#endif
{
errmsg = (char_u *)_("E212: Can't open file for writing");
if (forceit && vim_strchr(p_cpo, CPO_FWRITE) == NULL
&& perm >= 0)
{
#if defined(UNIX)


if (!(perm & 0200))
made_writable = TRUE;
perm |= 0200;
if (st_old.st_uid != getuid()
|| st_old.st_gid != getgid())
perm &= 0777;
#endif
if (!append) 
mch_remove(wfname);
continue;
}
}
}

restore_backup:
{
stat_T st;




if (backup != NULL && wfname == fname)
{
if (backup_copy)
{







if (mch_stat((char *)fname, &st) < 0)
vim_rename(backup, fname);

if (mch_stat((char *)fname, &st) >= 0)
mch_remove(backup);
}
else
{

vim_rename(backup, fname);
}
}


if (!newfile && mch_stat((char *)fname, &st) < 0)
end = 0;
}

if (wfname != fname)
vim_free(wfname);
goto fail;
}
write_info.bw_fd = fd;

#if defined(UNIX)
{
stat_T st;



if (overwriting
&& (!dobackup || backup_copy)
&& fname == wfname
&& perm >= 0
&& mch_fstat(fd, &st) == 0
&& st.st_ino != st_old.st_ino)
{
close(fd);
errmsg = (char_u *)_("E949: File changed while writing");
goto fail;
}
}
#endif
#if defined(HAVE_FTRUNCATE)
if (!append)
vim_ignored = ftruncate(fd, (off_t)0);
#endif

#if defined(MSWIN)
if (backup != NULL && overwriting && !append)
{
if (backup_copy)
(void)mch_copy_file_attribute(wfname, backup);
else
(void)mch_copy_file_attribute(backup, wfname);
}

if (!overwriting && !append)
{
if (buf->b_ffname != NULL)
(void)mch_copy_file_attribute(buf->b_ffname, wfname);

}
#endif

#if defined(FEAT_CRYPT)
if (*buf->b_p_key != NUL && !filtering)
{
char_u *header;
int header_len;

buf->b_cryptstate = crypt_create_for_writing(
crypt_get_method_nr(buf),
buf->b_p_key, &header, &header_len);
if (buf->b_cryptstate == NULL || header == NULL)
end = 0;
else
{


write_info.bw_buf = header;
write_info.bw_len = header_len;
write_info.bw_flags = FIO_NOCONVERT;
if (buf_write_bytes(&write_info) == FAIL)
end = 0;
wb_flags |= FIO_ENCRYPTED;
vim_free(header);
}
}
#endif
}
errmsg = NULL;

write_info.bw_buf = buffer;
nchars = 0;


if (eap != NULL && eap->force_bin != 0)
write_bin = (eap->force_bin == FORCE_BIN);
else
write_bin = buf->b_p_bin;




if (buf->b_p_bomb && !write_bin && (!append || perm < 0))
{
write_info.bw_len = make_bom(buffer, fenc);
if (write_info.bw_len > 0)
{

write_info.bw_flags = FIO_NOCONVERT | wb_flags;
if (buf_write_bytes(&write_info) == FAIL)
end = 0;
else
nchars += write_info.bw_len;
}
}
write_info.bw_start_lnum = start;

#if defined(FEAT_PERSISTENT_UNDO)
write_undo_file = (buf->b_p_udf
&& overwriting
&& !append
&& !filtering
&& reset_changed
&& !checking_conversion);
if (write_undo_file)

sha256_start(&sha_ctx);
#endif

write_info.bw_len = bufsize;
write_info.bw_flags = wb_flags;
fileformat = get_fileformat_force(buf, eap);
s = buffer;
len = 0;
for (lnum = start; lnum <= end; ++lnum)
{


ptr = ml_get_buf(buf, lnum, FALSE) - 1;
#if defined(FEAT_PERSISTENT_UNDO)
if (write_undo_file)
sha256_update(&sha_ctx, ptr + 1,
(UINT32_T)(STRLEN(ptr + 1) + 1));
#endif
while ((c = *++ptr) != NUL)
{
if (c == NL)
*s = NUL; 
else if (c == CAR && fileformat == EOL_MAC)
*s = NL; 
else
*s = c;
++s;
if (++len != bufsize)
continue;
if (buf_write_bytes(&write_info) == FAIL)
{
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
|| (lnum == buf->b_ml.ml_line_count
&& !buf->b_p_eol))))
{
++lnum; 
no_eol = TRUE;
break;
}
if (fileformat == EOL_UNIX)
*s++ = NL;
else
{
*s++ = CAR; 
if (fileformat == EOL_DOS) 
{
if (++len == bufsize)
{
if (buf_write_bytes(&write_info) == FAIL)
{
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
if (++len == bufsize && end)
{
if (buf_write_bytes(&write_info) == FAIL)
{
end = 0; 
break;
}
nchars += bufsize;
s = buffer;
len = 0;

ui_breakcheck();
if (got_int)
{
end = 0; 
break;
}
}
#if defined(VMS)







#if !defined(MIN)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
if (buf->b_fab_rfm == FAB$C_VFC
|| ((buf->b_fab_rat & (FAB$M_FTN | FAB$M_CR)) != 0))
{
int b2write;

buf->b_fab_mrs = (buf->b_fab_mrs == 0
? MIN(4096, bufsize)
: MIN(buf->b_fab_mrs, bufsize));

b2write = len;
while (b2write > 0)
{
write_info.bw_len = MIN(b2write, buf->b_fab_mrs);
if (buf_write_bytes(&write_info) == FAIL)
{
end = 0;
break;
}
b2write -= MIN(b2write, buf->b_fab_mrs);
}
write_info.bw_len = bufsize;
nchars += len;
s = buffer;
len = 0;
}
#endif
}
if (len > 0 && end > 0)
{
write_info.bw_len = len;
if (buf_write_bytes(&write_info) == FAIL)
end = 0; 
nchars += len;
}


if (!checking_conversion || end == 0)
break;



}



if (!checking_conversion)
{
#if defined(UNIX) && defined(HAVE_FSYNC)








if (p_fs && vim_fsync(fd) != 0 && !device)
{
errmsg = (char_u *)_(e_fsync);
end = 0;
}
#endif

#if defined(HAVE_SELINUX) || defined(HAVE_SMACK)

if (!backup_copy)
mch_copy_sec(backup, wfname);
#endif

#if defined(UNIX)


if (backup != NULL && !backup_copy)
{
#if defined(HAVE_FCHOWN)
stat_T st;



if (mch_stat((char *)wfname, &st) < 0
|| st.st_uid != st_old.st_uid
|| st.st_gid != st_old.st_gid)
{

vim_ignored = fchown(fd, st_old.st_uid, -1);

if (fchown(fd, -1, st_old.st_gid) == -1 && perm > 0)
perm &= ~070;
}
#endif
buf_setino(buf);
}
else if (!buf->b_dev_valid)

buf_setino(buf);
#endif

#if defined(UNIX)
if (made_writable)
perm &= ~0200; 
#endif
#if defined(HAVE_FCHMOD)

if (perm >= 0)
(void)mch_fsetperm(fd, perm);
#endif
if (close(fd) != 0)
{
errmsg = (char_u *)_("E512: Close failed");
end = 0;
}

#if !defined(HAVE_FCHMOD)

if (perm >= 0)
(void)mch_setperm(wfname, perm);
#endif
#if defined(HAVE_ACL)






#if !defined(HAVE_SOLARIS_ZFS_ACL)
if (!backup_copy)
#endif
mch_set_acl(wfname, acl);
#endif
#if defined(FEAT_CRYPT)
if (buf->b_cryptstate != NULL)
{
crypt_free_state(buf->b_cryptstate);
buf->b_cryptstate = NULL;
}
#endif

#if defined(FEAT_EVAL)
if (wfname != fname)
{


if (end != 0)
{
if (eval_charconvert(enc_utf8 ? (char_u *)"utf-8" : p_enc,
fenc, wfname, fname) == FAIL)
{
write_info.bw_conv_error = TRUE;
end = 0;
}
}
mch_remove(wfname);
vim_free(wfname);
}
#endif
}

if (end == 0)
{

if (errmsg == NULL)
{
if (write_info.bw_conv_error)
{
if (write_info.bw_conv_error_lnum == 0)
errmsg = (char_u *)_("E513: write error, conversion failed (make 'fenc' empty to override)");
else
{
errmsg_allocated = TRUE;
errmsg = alloc(300);
vim_snprintf((char *)errmsg, 300, _("E513: write error, conversion failed in line %ld (make 'fenc' empty to override)"),
(long)write_info.bw_conv_error_lnum);
}
}
else if (got_int)
errmsg = (char_u *)_(e_interr);
else
errmsg = (char_u *)_("E514: write error (file system full?)");
}








if (backup != NULL)
{
if (backup_copy)
{


if (got_int)
{
msg(_(e_interr));
out_flush();
}
if ((fd = mch_open((char *)backup, O_RDONLY | O_EXTRA, 0)) >= 0)
{
if ((write_info.bw_fd = mch_open((char *)fname,
O_WRONLY | O_CREAT | O_TRUNC | O_EXTRA,
perm & 0777)) >= 0)
{

write_info.bw_buf = smallbuf;
write_info.bw_flags = FIO_NOCONVERT;
while ((write_info.bw_len = read_eintr(fd, smallbuf,
SMALLBUFSIZE)) > 0)
if (buf_write_bytes(&write_info) == FAIL)
break;

if (close(write_info.bw_fd) >= 0
&& write_info.bw_len == 0)
end = 1; 
}
close(fd); 
}
}
else
{
if (vim_rename(backup, fname) == 0)
end = 1;
}
}
goto fail;
}

lnum -= start; 
--no_wait_return; 

#if !(defined(UNIX) || defined(VMS))
fname = sfname; 
#endif
if (!filtering)
{
msg_add_fname(buf, fname); 
c = FALSE;
if (write_info.bw_conv_error)
{
STRCAT(IObuff, _(" CONVERSION ERROR"));
c = TRUE;
if (write_info.bw_conv_error_lnum != 0)
vim_snprintf_add((char *)IObuff, IOSIZE, _(" in line %ld;"),
(long)write_info.bw_conv_error_lnum);
}
else if (notconverted)
{
STRCAT(IObuff, _("[NOT converted]"));
c = TRUE;
}
else if (converted)
{
STRCAT(IObuff, _("[converted]"));
c = TRUE;
}
if (device)
{
STRCAT(IObuff, _("[Device]"));
c = TRUE;
}
else if (newfile)
{
STRCAT(IObuff, shortmess(SHM_NEW) ? _("[New]") : _("[New File]"));
c = TRUE;
}
if (no_eol)
{
msg_add_eol();
c = TRUE;
}

if (msg_add_fileformat(fileformat))
c = TRUE;
#if defined(FEAT_CRYPT)
if (wb_flags & FIO_ENCRYPTED)
{
crypt_append_msg(buf);
c = TRUE;
}
#endif
msg_add_lines(c, (long)lnum, nchars); 
if (!shortmess(SHM_WRITE))
{
if (append)
STRCAT(IObuff, shortmess(SHM_WRI) ? _(" [a]") : _(" appended"));
else
STRCAT(IObuff, shortmess(SHM_WRI) ? _(" [w]") : _(" written"));
}

set_keep_msg((char_u *)msg_trunc_attr((char *)IObuff, FALSE, 0), 0);
}



if (reset_changed && whole && !append
&& !write_info.bw_conv_error
&& (overwriting || vim_strchr(p_cpo, CPO_PLUS) != NULL))
{
unchanged(buf, TRUE, FALSE);


if (buf->b_last_changedtick + 1 == CHANGEDTICK(buf))
buf->b_last_changedtick = CHANGEDTICK(buf);
u_unchanged(buf);
u_update_save_nr(buf);
}



if (overwriting)
{
ml_timestamp(buf);
if (append)
buf->b_flags &= ~BF_NEW;
else
buf->b_flags &= ~BF_WRITE_MASK;
}



if (*p_pm && dobackup)
{
char *org = (char *)buf_modname((buf->b_p_sn || buf->b_shortname),
fname, p_pm, FALSE);

if (backup != NULL)
{
stat_T st;



if (org == NULL)
emsg(_("E205: Patchmode: can't save original file"));
else if (mch_stat(org, &st) < 0)
{
vim_rename(backup, (char_u *)org);
VIM_CLEAR(backup); 
#if defined(UNIX)
set_file_time((char_u *)org, st_old.st_atime, st_old.st_mtime);
#endif
}
}


else
{
int empty_fd;

if (org == NULL
|| (empty_fd = mch_open(org,
O_CREAT | O_EXTRA | O_EXCL | O_NOFOLLOW,
perm < 0 ? 0666 : (perm & 0777))) < 0)
emsg(_("E206: patchmode: can't touch empty original file"));
else
close(empty_fd);
}
if (org != NULL)
{
mch_setperm((char_u *)org, mch_getperm(fname) & 0777);
vim_free(org);
}
}



if (!p_bk && backup != NULL && !write_info.bw_conv_error
&& mch_remove(backup) != 0)
emsg(_("E207: Can't delete backup file"));

goto nofail;


fail:
--no_wait_return; 
nofail:


buf->b_saving = FALSE;

vim_free(backup);
if (buffer != smallbuf)
vim_free(buffer);
vim_free(fenc_tofree);
vim_free(write_info.bw_conv_buf);
#if defined(USE_ICONV)
if (write_info.bw_iconv_fd != (iconv_t)-1)
{
iconv_close(write_info.bw_iconv_fd);
write_info.bw_iconv_fd = (iconv_t)-1;
}
#endif
#if defined(HAVE_ACL)
mch_free_acl(acl);
#endif

if (errmsg != NULL)
{
int numlen = errnum != NULL ? (int)STRLEN(errnum) : 0;

attr = HL_ATTR(HLF_E); 
msg_add_fname(buf,
#if !defined(UNIX)
sfname
#else
fname
#endif
); 
if (STRLEN(IObuff) + STRLEN(errmsg) + numlen >= IOSIZE)
IObuff[IOSIZE - STRLEN(errmsg) - numlen - 1] = NUL;


if (errnum != NULL)
{
STRMOVE(IObuff + numlen, IObuff);
mch_memmove(IObuff, errnum, (size_t)numlen);
}
STRCAT(IObuff, errmsg);
emsg((char *)IObuff);
if (errmsg_allocated)
vim_free(errmsg);

retval = FAIL;
if (end == 0)
{
msg_puts_attr(_("\nWARNING: Original file may be lost or damaged\n"),
attr | MSG_HIST);
msg_puts_attr(_("don't quit the editor until the file is successfully written!"),
attr | MSG_HIST);



if (mch_stat((char *)fname, &st_old) >= 0)
{
buf_store_time(buf, &st_old, fname);
buf->b_mtime_read = buf->b_mtime;
}
}
}
msg_scroll = msg_save;

#if defined(FEAT_PERSISTENT_UNDO)


if (retval == OK && write_undo_file)
{
char_u hash[UNDO_HASH_SIZE];

sha256_finish(&sha_ctx, hash);
u_write_undo(NULL, FALSE, buf, hash);
}
#endif

#if defined(FEAT_EVAL)
if (!should_abort(retval))
#else
if (!got_int)
#endif
{
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

#if defined(FEAT_EVAL)
if (aborting()) 
retval = FALSE;
#endif
}

got_int |= prev_got_int;

return retval;
}
