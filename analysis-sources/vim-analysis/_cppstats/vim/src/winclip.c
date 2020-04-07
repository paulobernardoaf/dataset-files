#include "vim.h"
#if defined(FEAT_CYGWIN_WIN32_CLIPBOARD)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "winclip.pro"
#endif
#if defined(PROTO)
#define WINAPI
#define WINBASEAPI
typedef int DWORD;
typedef int LPBOOL;
typedef int LPCSTR;
typedef int LPCWSTR;
typedef int LPSTR;
typedef int LPWSTR;
typedef int UINT;
#endif
int
utf8_to_utf16(char_u *instr, int inlen, short_u *outstr, int *unconvlenp)
{
int outlen = 0;
char_u *p = instr;
int todo = inlen;
int l;
int ch;
while (todo > 0)
{
l = utf_ptr2len_len(p, todo);
if (l > todo)
{
if (unconvlenp != NULL)
*unconvlenp = todo;
break;
}
ch = utf_ptr2char(p);
if (ch >= 0x10000)
{
++outlen;
if (outstr != NULL)
{
*outstr++ = (0xD800 - (0x10000 >> 10)) + (ch >> 10);
*outstr++ = 0xDC00 | (ch & 0x3FF);
}
}
else if (outstr != NULL)
*outstr++ = ch;
++outlen;
p += l;
todo -= l;
}
return outlen;
}
int
utf16_to_utf8(short_u *instr, int inlen, char_u *outstr)
{
int outlen = 0;
int todo = inlen;
short_u *p = instr;
int l;
int ch, ch2;
while (todo > 0)
{
ch = *p;
if (ch >= 0xD800 && ch <= 0xDBFF && todo > 1)
{
ch2 = p[1];
if (ch2 >= 0xDC00 && ch2 <= 0xDFFF)
{
ch = ((ch - 0xD800) << 10) + (ch2 & 0x3FF) + 0x10000;
++p;
--todo;
}
}
if (outstr != NULL)
{
l = utf_char2bytes(ch, outstr);
outstr += l;
}
else
l = utf_char2len(ch);
++p;
outlen += l;
--todo;
}
return outlen;
}
void
MultiByteToWideChar_alloc(UINT cp, DWORD flags,
LPCSTR in, int inlen,
LPWSTR *out, int *outlen)
{
*outlen = MultiByteToWideChar(cp, flags, in, inlen, 0, 0);
*out = ALLOC_MULT(WCHAR, *outlen + 1);
if (*out != NULL)
{
MultiByteToWideChar(cp, flags, in, inlen, *out, *outlen);
(*out)[*outlen] = 0;
}
}
void
WideCharToMultiByte_alloc(UINT cp, DWORD flags,
LPCWSTR in, int inlen,
LPSTR *out, int *outlen,
LPCSTR def, LPBOOL useddef)
{
*outlen = WideCharToMultiByte(cp, flags, in, inlen, NULL, 0, def, useddef);
*out = alloc(*outlen + 1);
if (*out != NULL)
{
WideCharToMultiByte(cp, flags, in, inlen, *out, *outlen, def, useddef);
(*out)[*outlen] = 0;
}
}
#if defined(FEAT_CLIPBOARD)
void
win_clip_init(void)
{
clip_init(TRUE);
clip_star.format = RegisterClipboardFormat("VimClipboard2");
clip_star.format_raw = RegisterClipboardFormat("VimRawBytes");
}
typedef struct
{
int type; 
int txtlen; 
int ucslen; 
int rawlen; 
} VimClipType_t;
int
clip_mch_own_selection(Clipboard_T *cbd UNUSED)
{
return FAIL;
}
void
clip_mch_lose_selection(Clipboard_T *cbd UNUSED)
{
}
static char_u *
crnl_to_nl(const char_u *str, int *size)
{
int pos = 0;
int str_len = *size;
char_u *ret;
char_u *retp;
ret = alloc(str_len == 0 ? 1 : str_len);
if (ret != NULL)
{
retp = ret;
for (pos = 0; pos < str_len; ++pos)
{
if (str[pos] == '\r' && str[pos + 1] == '\n')
{
++pos;
--(*size);
}
*retp++ = str[pos];
}
}
return ret;
}
static int
vim_open_clipboard(void)
{
int delay = 10;
while (!OpenClipboard(NULL))
{
if (delay > 500)
return FALSE; 
Sleep(delay);
delay *= 2; 
}
return TRUE;
}
void
clip_mch_request_selection(Clipboard_T *cbd)
{
VimClipType_t metadata = { -1, -1, -1, -1 };
HGLOBAL hMem = NULL;
char_u *str = NULL;
char_u *to_free = NULL;
HGLOBAL rawh = NULL;
int str_size = 0;
int maxlen;
size_t n;
if (!vim_open_clipboard())
return;
if (IsClipboardFormatAvailable(cbd->format))
{
VimClipType_t *meta_p;
HGLOBAL meta_h;
if ((meta_h = GetClipboardData(cbd->format)) != NULL
&& (meta_p = (VimClipType_t *)GlobalLock(meta_h)) != NULL)
{
n = sizeof(VimClipType_t);
if (GlobalSize(meta_h) < n)
n = GlobalSize(meta_h);
memcpy(&metadata, meta_p, n);
GlobalUnlock(meta_h);
}
}
if (IsClipboardFormatAvailable(cbd->format_raw)
&& metadata.rawlen > (int)STRLEN(p_enc))
{
if ((rawh = GetClipboardData(cbd->format_raw)) != NULL)
{
char_u *rawp;
rawp = (char_u *)GlobalLock(rawh);
if (rawp != NULL && STRCMP(p_enc, rawp) == 0)
{
n = STRLEN(p_enc) + 1;
str = rawp + n;
str_size = (int)(metadata.rawlen - n);
}
else
{
GlobalUnlock(rawh);
rawh = NULL;
}
}
}
if (str == NULL)
{
if (IsClipboardFormatAvailable(CF_UNICODETEXT) && metadata.ucslen != 0)
{
HGLOBAL hMemW;
if ((hMemW = GetClipboardData(CF_UNICODETEXT)) != NULL)
{
WCHAR *hMemWstr = (WCHAR *)GlobalLock(hMemW);
maxlen = (int)(GlobalSize(hMemW) / sizeof(WCHAR));
if (metadata.ucslen >= 0)
{
if (metadata.ucslen > maxlen)
str_size = maxlen;
else
str_size = metadata.ucslen;
}
else
{
for (str_size = 0; str_size < maxlen; ++str_size)
if (hMemWstr[str_size] == NUL)
break;
}
to_free = str = utf16_to_enc((short_u *)hMemWstr, &str_size);
GlobalUnlock(hMemW);
}
}
else if (IsClipboardFormatAvailable(CF_TEXT))
{
if ((hMem = GetClipboardData(CF_TEXT)) != NULL)
{
str = (char_u *)GlobalLock(hMem);
maxlen = (int)GlobalSize(hMem);
if (metadata.txtlen >= 0)
{
if (metadata.txtlen > maxlen)
str_size = maxlen;
else
str_size = metadata.txtlen;
}
else
{
for (str_size = 0; str_size < maxlen; ++str_size)
if (str[str_size] == NUL)
break;
}
acp_to_enc(str, str_size, &to_free, &maxlen);
if (to_free != NULL)
{
str_size = maxlen;
str = to_free;
}
}
}
}
if (str != NULL && *str != NUL)
{
char_u *temp_clipboard;
if (metadata.type == -1)
metadata.type = MAUTO;
temp_clipboard = crnl_to_nl(str, &str_size);
if (temp_clipboard != NULL)
{
clip_yank_selection(metadata.type, temp_clipboard, str_size, cbd);
vim_free(temp_clipboard);
}
}
if (hMem != NULL)
GlobalUnlock(hMem);
if (rawh != NULL)
GlobalUnlock(rawh);
CloseClipboard();
vim_free(to_free);
}
void
clip_mch_set_selection(Clipboard_T *cbd)
{
char_u *str = NULL;
VimClipType_t metadata;
long_u txtlen;
HGLOBAL hMemRaw = NULL;
HGLOBAL hMem = NULL;
HGLOBAL hMemVim = NULL;
HGLOBAL hMemW = NULL;
cbd->owned = TRUE;
clip_get_selection(cbd);
cbd->owned = FALSE;
metadata.type = clip_convert_selection(&str, &txtlen, cbd);
if (metadata.type < 0)
return;
metadata.txtlen = (int)txtlen;
metadata.ucslen = 0;
metadata.rawlen = 0;
{
LPSTR lpszMemRaw;
metadata.rawlen = (int)(txtlen + STRLEN(p_enc) + 1);
hMemRaw = (LPSTR)GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
metadata.rawlen + 1);
lpszMemRaw = (LPSTR)GlobalLock(hMemRaw);
if (lpszMemRaw != NULL)
{
STRCPY(lpszMemRaw, p_enc);
memcpy(lpszMemRaw + STRLEN(p_enc) + 1, str, txtlen + 1);
GlobalUnlock(hMemRaw);
}
else
metadata.rawlen = 0;
}
{
WCHAR *out;
int len = metadata.txtlen;
out = (WCHAR *)enc_to_utf16(str, &len);
if (out != NULL)
{
WCHAR *lpszMemW;
metadata.txtlen = WideCharToMultiByte(GetACP(), 0, out, len,
NULL, 0, 0, 0);
vim_free(str);
str = alloc(metadata.txtlen == 0 ? 1 : metadata.txtlen);
if (str == NULL)
{
vim_free(out);
return; 
}
WideCharToMultiByte(GetACP(), 0, out, len,
(LPSTR)str, metadata.txtlen, 0, 0);
hMemW = (LPSTR)GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
(len + 1) * sizeof(WCHAR));
lpszMemW = (WCHAR *)GlobalLock(hMemW);
if (lpszMemW != NULL)
{
memcpy(lpszMemW, out, len * sizeof(WCHAR));
lpszMemW[len] = NUL;
GlobalUnlock(hMemW);
}
vim_free(out);
metadata.ucslen = len;
}
}
hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, metadata.txtlen + 1);
{
LPSTR lpszMem = (LPSTR)GlobalLock(hMem);
if (lpszMem)
{
vim_strncpy((char_u *)lpszMem, str, metadata.txtlen);
GlobalUnlock(hMem);
}
}
{
VimClipType_t *lpszMemVim = NULL;
hMemVim = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,
sizeof(VimClipType_t));
lpszMemVim = (VimClipType_t *)GlobalLock(hMemVim);
memcpy(lpszMemVim, &metadata, sizeof(metadata));
GlobalUnlock(hMemVim);
}
if (vim_open_clipboard())
{
if (EmptyClipboard())
{
SetClipboardData(cbd->format, hMemVim);
hMemVim = 0;
if (hMemW != NULL)
{
if (SetClipboardData(CF_UNICODETEXT, hMemW) != NULL)
hMemW = NULL;
}
SetClipboardData(CF_TEXT, hMem);
hMem = 0;
}
CloseClipboard();
}
vim_free(str);
if (hMemRaw)
GlobalFree(hMemRaw);
if (hMem)
GlobalFree(hMem);
if (hMemW)
GlobalFree(hMemW);
if (hMemVim)
GlobalFree(hMemVim);
}
#endif 
short_u *
enc_to_utf16(char_u *str, int *lenp)
{
vimconv_T conv;
WCHAR *ret;
char_u *allocbuf = NULL;
int len_loc;
int length;
if (lenp == NULL)
{
len_loc = (int)STRLEN(str) + 1;
lenp = &len_loc;
}
if (enc_codepage > 0)
{
MultiByteToWideChar_alloc(enc_codepage, 0, (LPCSTR)str, *lenp,
&ret, &length);
}
else
{
conv.vc_type = CONV_NONE;
if (convert_setup(&conv, p_enc ? p_enc : (char_u *)"latin1",
(char_u *)"utf-8") == FAIL)
return NULL;
if (conv.vc_type != CONV_NONE)
{
str = allocbuf = string_convert(&conv, str, lenp);
if (str == NULL)
return NULL;
}
convert_setup(&conv, NULL, NULL);
length = utf8_to_utf16(str, *lenp, NULL, NULL);
ret = ALLOC_MULT(WCHAR, length + 1);
if (ret != NULL)
{
utf8_to_utf16(str, *lenp, (short_u *)ret, NULL);
ret[length] = 0;
}
vim_free(allocbuf);
}
*lenp = length;
return (short_u *)ret;
}
char_u *
utf16_to_enc(short_u *str, int *lenp)
{
vimconv_T conv;
char_u *utf8_str = NULL, *enc_str = NULL;
int len_loc;
if (lenp == NULL)
{
len_loc = (int)wcslen(str) + 1;
lenp = &len_loc;
}
if (enc_codepage > 0)
{
int length;
WideCharToMultiByte_alloc(enc_codepage, 0, str, *lenp,
(LPSTR *)&enc_str, &length, 0, 0);
*lenp = length;
return enc_str;
}
utf8_str = alloc(utf16_to_utf8(str, *lenp == 0 ? 1 : *lenp, NULL));
if (utf8_str != NULL)
{
*lenp = utf16_to_utf8(str, *lenp, utf8_str);
conv.vc_type = CONV_NONE;
convert_setup(&conv, (char_u *)"utf-8",
p_enc? p_enc: (char_u *)"latin1");
if (conv.vc_type == CONV_NONE)
{
enc_str = utf8_str;
}
else
{
enc_str = string_convert(&conv, utf8_str, lenp);
vim_free(utf8_str);
}
convert_setup(&conv, NULL, NULL);
}
return enc_str;
}
void
acp_to_enc(
char_u *str,
int str_size,
char_u **out,
int *outlen)
{
LPWSTR widestr;
MultiByteToWideChar_alloc(GetACP(), 0, (LPCSTR)str, str_size,
&widestr, outlen);
if (widestr != NULL)
{
++*outlen; 
*out = utf16_to_enc((short_u *)widestr, outlen);
vim_free(widestr);
}
}
void
enc_to_acp(
char_u *str,
int str_size,
char_u **out,
int *outlen)
{
LPWSTR widestr;
int len = str_size;
widestr = (WCHAR *)enc_to_utf16(str, &len);
if (widestr != NULL)
{
WideCharToMultiByte_alloc(GetACP(), 0, widestr, len,
(LPSTR *)out, outlen, 0, 0);
vim_free(widestr);
}
}
