

















#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "allegro5/allegro.h"
#include "allegro5/internal/bstrlib.h"

#define bstr__alloc(x) al_malloc(x)
#define bstr__free(p) al_free(p)
#define bstr__realloc(p, x) al_realloc((p), (x))



#if defined(MEMORY_DEBUG) || defined(BSTRLIB_MEMORY_DEBUG)
#include "memdbg.h"
#endif

#if !defined(bstr__alloc)
#define bstr__alloc(x) malloc (x)
#endif

#if !defined(bstr__free)
#define bstr__free(p) free (p)
#endif

#if !defined(bstr__realloc)
#define bstr__realloc(p,x) realloc ((p), (x))
#endif

#if !defined(bstr__memcpy)
#define bstr__memcpy(d,s,l) memcpy ((d), (s), (l))
#endif

#if !defined(bstr__memmove)
#define bstr__memmove(d,s,l) memmove ((d), (s), (l))
#endif

#if !defined(bstr__memset)
#define bstr__memset(d,c,l) memset ((d), (c), (l))
#endif

#if !defined(bstr__memcmp)
#define bstr__memcmp(d,c,l) memcmp ((d), (c), (l))
#endif

#if !defined(bstr__memchr)
#define bstr__memchr(s,c,l) memchr ((s), (c), (l))
#endif



#define bBlockCopy(D,S,L) { if ((L) > 0) bstr__memmove ((D),(S),(L)); }



static int snapUpSize (int i) {
if (i < 8) {
i = 8;
} else {
unsigned int j;
j = (unsigned int) i;

j |= (j >> 1);
j |= (j >> 2);
j |= (j >> 4);
j |= (j >> 8); 
#if (UINT_MAX != 0xffff)
j |= (j >> 16); 
#if (UINT_MAX > 0xffffffffUL)
j |= (j >> 32); 
#endif
#endif

j++;
if ((int) j >= i) i = (int) j;
}
return i;
}





int _al_balloc (_al_bstring b, int olen) {
int len;
if (b == NULL || b->data == NULL || b->slen < 0 || b->mlen <= 0 || 
b->mlen < b->slen || olen <= 0) {
return _AL_BSTR_ERR;
}

if (olen >= b->mlen) {
unsigned char * x;

if ((len = snapUpSize (olen)) <= b->mlen) return _AL_BSTR_OK;


if (7 * b->mlen < 8 * b->slen) {




reallocStrategy:;

x = (unsigned char *) bstr__realloc (b->data, (size_t) len);
if (x == NULL) {




if (NULL == (x = (unsigned char *) bstr__realloc (b->data, (size_t) (len = olen)))) {
return _AL_BSTR_ERR;
}
}
} else {





if (NULL == (x = (unsigned char *) bstr__alloc ((size_t) len))) {




goto reallocStrategy;

} else {
if (b->slen) bstr__memcpy ((char *) x, (char *) b->data, (size_t) b->slen);
bstr__free (b->data);
}
}
b->data = x;
b->mlen = len;
b->data[b->slen] = (unsigned char) '\0';
}

return _AL_BSTR_OK;
}







int _al_ballocmin (_al_bstring b, int len) {
unsigned char * s;

if (b == NULL || b->data == NULL || (b->slen+1) < 0 || b->mlen <= 0 || 
b->mlen < b->slen || len <= 0) {
return _AL_BSTR_ERR;
}

if (len < b->slen + 1) len = b->slen + 1;

if (len != b->mlen) {
s = (unsigned char *) bstr__realloc (b->data, (size_t) len);
if (NULL == s) return _AL_BSTR_ERR;
s[b->slen] = (unsigned char) '\0';
b->data = s;
b->mlen = len;
}

return _AL_BSTR_OK;
}






_al_bstring _al_bfromcstr (const char * str) {
_al_bstring b;
int i;
size_t j;

if (str == NULL) return NULL;
j = (strlen) (str);
i = snapUpSize ((int) (j + (2 - (j != 0))));
if (i <= (int) j) return NULL;

b = (_al_bstring) bstr__alloc (sizeof (struct _al_tagbstring));
if (NULL == b) return NULL;
b->slen = (int) j;
if (NULL == (b->data = (unsigned char *) bstr__alloc (b->mlen = i))) {
bstr__free (b);
return NULL;
}

bstr__memcpy (b->data, str, j+1);
return b;
}







_al_bstring _al_bfromcstralloc (int mlen, const char * str) {
_al_bstring b;
int i;
size_t j;

if (str == NULL) return NULL;
j = (strlen) (str);
i = snapUpSize ((int) (j + (2 - (j != 0))));
if (i <= (int) j) return NULL;

b = (_al_bstring) bstr__alloc (sizeof (struct _al_tagbstring));
if (b == NULL) return NULL;
b->slen = (int) j;
if (i < mlen) i = mlen;

if (NULL == (b->data = (unsigned char *) bstr__alloc (b->mlen = i))) {
bstr__free (b);
return NULL;
}

bstr__memcpy (b->data, str, j+1);
return b;
}






_al_bstring _al_blk2bstr (const void * blk, int len) {
_al_bstring b;
int i;

if (blk == NULL || len < 0) return NULL;
b = (_al_bstring) bstr__alloc (sizeof (struct _al_tagbstring));
if (b == NULL) return NULL;
b->slen = len;

i = len + (2 - (len != 0));
i = snapUpSize (i);

b->mlen = i;

b->data = (unsigned char *) bstr__alloc ((size_t) b->mlen);
if (b->data == NULL) {
bstr__free (b);
return NULL;
}

if (len > 0) bstr__memcpy (b->data, blk, (size_t) len);
b->data[len] = (unsigned char) '\0';

return b;
}








char * _al_bstr2cstr (_al_const_bstring b, char z) {
int i, l;
char * r;

if (b == NULL || b->slen < 0 || b->data == NULL) return NULL;
l = b->slen;
r = (char *) bstr__alloc ((size_t) (l + 1));
if (r == NULL) return r;

for (i=0; i < l; i ++) {
r[i] = (char) ((b->data[i] == '\0') ? z : (char) (b->data[i]));
}

r[l] = (unsigned char) '\0';

return r;
}












int _al_bcstrfree (char * s) {
if (s) {
bstr__free (s);
return _AL_BSTR_OK;
}
return _AL_BSTR_ERR;
}





int _al_bconcat (_al_bstring b0, _al_const_bstring b1) {
int len, d;
_al_bstring aux = (_al_bstring) b1;

if (b0 == NULL || b1 == NULL || b0->data == NULL || b1->data == NULL) return _AL_BSTR_ERR;

d = b0->slen;
len = b1->slen;
if ((d | (b0->mlen - d) | len | (d + len)) < 0) return _AL_BSTR_ERR;

if (b0->mlen <= d + len + 1) {
ptrdiff_t pd = b1->data - b0->data;
if (0 <= pd && pd < b0->mlen) {
if (NULL == (aux = _al_bstrcpy (b1))) return _AL_BSTR_ERR;
}
if (_al_balloc (b0, d + len + 1) != _AL_BSTR_OK) {
if (aux != b1) _al_bdestroy (aux);
return _AL_BSTR_ERR;
}
}

bBlockCopy (&b0->data[d], &aux->data[0], (size_t) len);
b0->data[d + len] = (unsigned char) '\0';
b0->slen = d + len;
if (aux != b1) _al_bdestroy (aux);
return _AL_BSTR_OK;
}





int _al_bconchar (_al_bstring b, char c) {
int d;

if (b == NULL) return _AL_BSTR_ERR;
d = b->slen;
if ((d | (b->mlen - d)) < 0 || _al_balloc (b, d + 2) != _AL_BSTR_OK) return _AL_BSTR_ERR;
b->data[d] = (unsigned char) c;
b->data[d + 1] = (unsigned char) '\0';
b->slen++;
return _AL_BSTR_OK;
}





int _al_bcatcstr (_al_bstring b, const char * s) {
char * d;
int i, l;

if (b == NULL || b->data == NULL || b->slen < 0 || b->mlen < b->slen
|| b->mlen <= 0 || s == NULL) return _AL_BSTR_ERR;


l = b->mlen - b->slen;
d = (char *) &b->data[b->slen];
for (i=0; i < l; i++) {
if ((*d++ = *s++) == '\0') {
b->slen += i;
return _AL_BSTR_OK;
}
}
b->slen += i;


return _al_bcatblk (b, (const void *) s, (int) strlen (s));
}





int _al_bcatblk (_al_bstring b, const void * s, int len) {
int nl;

if (b == NULL || b->data == NULL || b->slen < 0 || b->mlen < b->slen
|| b->mlen <= 0 || s == NULL || len < 0) return _AL_BSTR_ERR;

if (0 > (nl = b->slen + len)) return _AL_BSTR_ERR; 
if (b->mlen <= nl && 0 > _al_balloc (b, nl + 1)) return _AL_BSTR_ERR;

bBlockCopy (&b->data[b->slen], s, (size_t) len);
b->slen = nl;
b->data[nl] = (unsigned char) '\0';
return _AL_BSTR_OK;
}





_al_bstring _al_bstrcpy (_al_const_bstring b) {
_al_bstring b0;
int i,j;


if (b == NULL || b->slen < 0 || b->data == NULL) return NULL;

b0 = (_al_bstring) bstr__alloc (sizeof (struct _al_tagbstring));
if (b0 == NULL) {

return NULL;
}

i = b->slen;
j = snapUpSize (i + 1);

b0->data = (unsigned char *) bstr__alloc (j);
if (b0->data == NULL) {
j = i + 1;
b0->data = (unsigned char *) bstr__alloc (j);
if (b0->data == NULL) {

bstr__free (b0);
return NULL;
}
}

b0->mlen = j;
b0->slen = i;

if (i) bstr__memcpy ((char *) b0->data, (char *) b->data, i);
b0->data[b0->slen] = (unsigned char) '\0';

return b0;
}





int _al_bassign (_al_bstring a, _al_const_bstring b) {
if (b == NULL || b->data == NULL || b->slen < 0)
return _AL_BSTR_ERR;
if (b->slen != 0) {
if (_al_balloc (a, b->slen) != _AL_BSTR_OK) return _AL_BSTR_ERR;
bstr__memmove (a->data, b->data, b->slen);
} else {
if (a == NULL || a->data == NULL || a->mlen < a->slen || 
a->slen < 0 || a->mlen == 0) 
return _AL_BSTR_ERR;
}
a->data[b->slen] = (unsigned char) '\0';
a->slen = b->slen;
return _AL_BSTR_OK;
}







int _al_bassignmidstr (_al_bstring a, _al_const_bstring b, int left, int len) {
if (b == NULL || b->data == NULL || b->slen < 0)
return _AL_BSTR_ERR;

if (left < 0) {
len += left;
left = 0;
}

if (len > b->slen - left) len = b->slen - left;

if (a == NULL || a->data == NULL || a->mlen < a->slen ||
a->slen < 0 || a->mlen == 0)
return _AL_BSTR_ERR;

if (len > 0) {
if (_al_balloc (a, len) != _AL_BSTR_OK) return _AL_BSTR_ERR;
bstr__memmove (a->data, b->data + left, len);
a->slen = len;
} else {
a->slen = 0;
}
a->data[a->slen] = (unsigned char) '\0';
return _AL_BSTR_OK;
}







int _al_bassigncstr (_al_bstring a, const char * str) {
int i;
size_t len;
if (a == NULL || a->data == NULL || a->mlen < a->slen ||
a->slen < 0 || a->mlen == 0 || NULL == str) 
return _AL_BSTR_ERR;

for (i=0; i < a->mlen; i++) {
if ('\0' == (a->data[i] = str[i])) {
a->slen = i;
return _AL_BSTR_OK;
}
}

a->slen = i;
len = strlen (str + i);
if (len > INT_MAX || i + len + 1 > INT_MAX ||
0 > _al_balloc (a, (int) (i + len + 1))) return _AL_BSTR_ERR;
bBlockCopy (a->data + i, str + i, (size_t) len + 1);
a->slen += (int) len;
return _AL_BSTR_OK;
}







int _al_bassignblk (_al_bstring a, const void * s, int len) {
if (a == NULL || a->data == NULL || a->mlen < a->slen ||
a->slen < 0 || a->mlen == 0 || NULL == s || len + 1 < 1) 
return _AL_BSTR_ERR;
if (len + 1 > a->mlen && 0 > _al_balloc (a, len + 1)) return _AL_BSTR_ERR;
bBlockCopy (a->data, s, (size_t) len);
a->data[len] = (unsigned char) '\0';
a->slen = len;
return _AL_BSTR_OK;
}





int _al_btrunc (_al_bstring b, int n) {
if (n < 0 || b == NULL || b->data == NULL || b->mlen < b->slen ||
b->slen < 0 || b->mlen <= 0) return _AL_BSTR_ERR;
if (b->slen > n) {
b->slen = n;
b->data[n] = (unsigned char) '\0';
}
return _AL_BSTR_OK;
}

#define upcase(c) (toupper ((unsigned char) c))
#define downcase(c) (tolower ((unsigned char) c))
#define wspace(c) (isspace ((unsigned char) c))





int _al_btoupper (_al_bstring b) {
int i, len;
if (b == NULL || b->data == NULL || b->mlen < b->slen ||
b->slen < 0 || b->mlen <= 0) return _AL_BSTR_ERR;
for (i=0, len = b->slen; i < len; i++) {
b->data[i] = (unsigned char) upcase (b->data[i]);
}
return _AL_BSTR_OK;
}





int _al_btolower (_al_bstring b) {
int i, len;
if (b == NULL || b->data == NULL || b->mlen < b->slen ||
b->slen < 0 || b->mlen <= 0) return _AL_BSTR_ERR;
for (i=0, len = b->slen; i < len; i++) {
b->data[i] = (unsigned char) downcase (b->data[i]);
}
return _AL_BSTR_OK;
}










int _al_bstricmp (_al_const_bstring b0, _al_const_bstring b1) {
int i, v, n;

if (_al_bdata (b0) == NULL || b0->slen < 0 || 
_al_bdata (b1) == NULL || b1->slen < 0) return SHRT_MIN;
if ((n = b0->slen) > b1->slen) n = b1->slen;
else if (b0->slen == b1->slen && b0->data == b1->data) return _AL_BSTR_OK;

for (i = 0; i < n; i ++) {
v = (char) downcase (b0->data[i])
- (char) downcase (b1->data[i]);
if (0 != v) return v;
}

if (b0->slen > n) {
v = (char) downcase (b0->data[n]);
if (v) return v;
return UCHAR_MAX + 1;
}
if (b1->slen > n) {
v = - (char) downcase (b1->data[n]);
if (v) return v;
return - (int) (UCHAR_MAX + 1);
}
return _AL_BSTR_OK;
}











int _al_bstrnicmp (_al_const_bstring b0, _al_const_bstring b1, int n) {
int i, v, m;

if (_al_bdata (b0) == NULL || b0->slen < 0 || 
_al_bdata (b1) == NULL || b1->slen < 0 || n < 0) return SHRT_MIN;
m = n;
if (m > b0->slen) m = b0->slen;
if (m > b1->slen) m = b1->slen;

if (b0->data != b1->data) {
for (i = 0; i < m; i ++) {
v = (char) downcase (b0->data[i]);
v -= (char) downcase (b1->data[i]);
if (v != 0) return b0->data[i] - b1->data[i];
}
}

if (n == m || b0->slen == b1->slen) return _AL_BSTR_OK;

if (b0->slen > m) {
v = (char) downcase (b0->data[m]);
if (v) return v;
return UCHAR_MAX + 1;
}

v = - (char) downcase (b1->data[m]);
if (v) return v;
return - (int) (UCHAR_MAX + 1);
}









int _al_biseqcaseless (_al_const_bstring b0, _al_const_bstring b1) {
int i, n;

if (_al_bdata (b0) == NULL || b0->slen < 0 || 
_al_bdata (b1) == NULL || b1->slen < 0) return _AL_BSTR_ERR;
if (b0->slen != b1->slen) return _AL_BSTR_OK;
if (b0->data == b1->data || b0->slen == 0) return 1;
for (i=0, n=b0->slen; i < n; i++) {
if (b0->data[i] != b1->data[i]) {
unsigned char c = (unsigned char) downcase (b0->data[i]);
if (c != (unsigned char) downcase (b1->data[i])) return 0;
}
}
return 1;
}










int _al_bisstemeqcaselessblk (_al_const_bstring b0, const void * blk, int len) {
int i;

if (_al_bdata (b0) == NULL || b0->slen < 0 || NULL == blk || len < 0)
return _AL_BSTR_ERR;
if (b0->slen < len) return _AL_BSTR_OK;
if (b0->data == (const unsigned char *) blk || len == 0) return 1;

for (i = 0; i < len; i ++) {
if (b0->data[i] != ((const unsigned char *) blk)[i]) {
if (downcase (b0->data[i]) != 
downcase (((const unsigned char *) blk)[i])) return 0;
}
}
return 1;
}






int _al_bltrimws (_al_bstring b) {
int i, len;

if (b == NULL || b->data == NULL || b->mlen < b->slen ||
b->slen < 0 || b->mlen <= 0) return _AL_BSTR_ERR;

for (len = b->slen, i = 0; i < len; i++) {
if (!wspace (b->data[i])) {
return _al_bdelete (b, 0, i);
}
}

b->data[0] = (unsigned char) '\0';
b->slen = 0;
return _AL_BSTR_OK;
}






int _al_brtrimws (_al_bstring b) {
int i;

if (b == NULL || b->data == NULL || b->mlen < b->slen ||
b->slen < 0 || b->mlen <= 0) return _AL_BSTR_ERR;

for (i = b->slen - 1; i >= 0; i--) {
if (!wspace (b->data[i])) {
if (b->mlen > i) b->data[i+1] = (unsigned char) '\0';
b->slen = i + 1;
return _AL_BSTR_OK;
}
}

b->data[0] = (unsigned char) '\0';
b->slen = 0;
return _AL_BSTR_OK;
}






int _al_btrimws (_al_bstring b) {
int i, j;

if (b == NULL || b->data == NULL || b->mlen < b->slen ||
b->slen < 0 || b->mlen <= 0) return _AL_BSTR_ERR;

for (i = b->slen - 1; i >= 0; i--) {
if (!wspace (b->data[i])) {
if (b->mlen > i) b->data[i+1] = (unsigned char) '\0';
b->slen = i + 1;
for (j = 0; wspace (b->data[j]); j++) {}
return _al_bdelete (b, 0, j);
}
}

b->data[0] = (unsigned char) '\0';
b->slen = 0;
return _AL_BSTR_OK;
}








int _al_biseq (_al_const_bstring b0, _al_const_bstring b1) {
if (b0 == NULL || b1 == NULL || b0->data == NULL || b1->data == NULL ||
b0->slen < 0 || b1->slen < 0) return _AL_BSTR_ERR;
if (b0->slen != b1->slen) return _AL_BSTR_OK;
if (b0->data == b1->data || b0->slen == 0) return 1;
return !bstr__memcmp (b0->data, b1->data, b0->slen);
}









int _al_bisstemeqblk (_al_const_bstring b0, const void * blk, int len) {
int i;

if (_al_bdata (b0) == NULL || b0->slen < 0 || NULL == blk || len < 0)
return _AL_BSTR_ERR;
if (b0->slen < len) return _AL_BSTR_OK;
if (b0->data == (const unsigned char *) blk || len == 0) return 1;

for (i = 0; i < len; i ++) {
if (b0->data[i] != ((const unsigned char *) blk)[i]) return _AL_BSTR_OK;
}
return 1;
}












int _al_biseqcstr (_al_const_bstring b, const char * s) {
int i;
if (b == NULL || s == NULL || b->data == NULL || b->slen < 0) return _AL_BSTR_ERR;
for (i=0; i < b->slen; i++) {
if (s[i] == '\0' || b->data[i] != (unsigned char) s[i]) return _AL_BSTR_OK;
}
return s[i] == '\0';
}













int _al_biseqcstrcaseless (_al_const_bstring b, const char * s) {
int i;
if (b == NULL || s == NULL || b->data == NULL || b->slen < 0) return _AL_BSTR_ERR;
for (i=0; i < b->slen; i++) {
if (s[i] == '\0' || 
(b->data[i] != (unsigned char) s[i] && 
downcase (b->data[i]) != (unsigned char) downcase (s[i])))
return _AL_BSTR_OK;
}
return s[i] == '\0';
}















int _al_bstrcmp (_al_const_bstring b0, _al_const_bstring b1) {
int i, v, n;

if (b0 == NULL || b1 == NULL || b0->data == NULL || b1->data == NULL ||
b0->slen < 0 || b1->slen < 0) return SHRT_MIN;
n = b0->slen; if (n > b1->slen) n = b1->slen;
if (b0->slen == b1->slen && (b0->data == b1->data || b0->slen == 0))
return _AL_BSTR_OK;

for (i = 0; i < n; i ++) {
v = ((char) b0->data[i]) - ((char) b1->data[i]);
if (v != 0) return v;
if (b0->data[i] == (unsigned char) '\0') return _AL_BSTR_OK;
}

if (b0->slen > n) return 1;
if (b1->slen > n) return -1;
return _AL_BSTR_OK;
}











int _al_bstrncmp (_al_const_bstring b0, _al_const_bstring b1, int n) {
int i, v, m;

if (b0 == NULL || b1 == NULL || b0->data == NULL || b1->data == NULL ||
b0->slen < 0 || b1->slen < 0) return SHRT_MIN;
m = n;
if (m > b0->slen) m = b0->slen;
if (m > b1->slen) m = b1->slen;

if (b0->data != b1->data) {
for (i = 0; i < m; i ++) {
v = ((char) b0->data[i]) - ((char) b1->data[i]);
if (v != 0) return v;
if (b0->data[i] == (unsigned char) '\0') return _AL_BSTR_OK;
}
}

if (n == m || b0->slen == b1->slen) return _AL_BSTR_OK;

if (b0->slen > m) return 1;
return -1;
}








_al_bstring _al_bmidstr (_al_const_bstring b, int left, int len) {

if (b == NULL || b->slen < 0 || b->data == NULL) return NULL;

if (left < 0) {
len += left;
left = 0;
}

if (len > b->slen - left) len = b->slen - left;

if (len <= 0) return _al_bfromcstr ("");
return _al_blk2bstr (b->data + left, len);
}








int _al_bdelete (_al_bstring b, int pos, int len) {

if (pos < 0) {
len += pos;
pos = 0;
}

if (len < 0 || b == NULL || b->data == NULL || b->slen < 0 || 
b->mlen < b->slen || b->mlen <= 0) 
return _AL_BSTR_ERR;
if (len > 0 && pos < b->slen) {
if (pos + len >= b->slen) {
b->slen = pos;
} else {
bBlockCopy ((char *) (b->data + pos),
(char *) (b->data + pos + len), 
b->slen - (pos+len));
b->slen -= len;
}
b->data[b->slen] = (unsigned char) '\0';
}
return _AL_BSTR_OK;
}








int _al_bdestroy (_al_bstring b) {
if (b == NULL || b->slen < 0 || b->mlen <= 0 || b->mlen < b->slen ||
b->data == NULL)
return _AL_BSTR_ERR;

bstr__free (b->data);




b->slen = -1;
b->mlen = -__LINE__;
b->data = NULL;

bstr__free (b);
return _AL_BSTR_OK;
}










int _al_binstr (_al_const_bstring b1, int pos, _al_const_bstring b2) {
int j, ii, ll, lf;
unsigned char * d0;
unsigned char c0;
register unsigned char * d1;
register unsigned char c1;
register int i;

if (b1 == NULL || b1->data == NULL || b1->slen < 0 ||
b2 == NULL || b2->data == NULL || b2->slen < 0) return _AL_BSTR_ERR;
if (b1->slen == pos) return (b2->slen == 0)?pos:_AL_BSTR_ERR;
if (b1->slen < pos || pos < 0) return _AL_BSTR_ERR;
if (b2->slen == 0) return pos;


if ((lf = b1->slen - b2->slen + 1) <= pos) return _AL_BSTR_ERR;


if (b1->data == b2->data && pos == 0) return 0;

i = pos;

d0 = b2->data;
d1 = b1->data;
ll = b2->slen;


c0 = d0[0];
if (1 == ll) {
for (;i < lf; i++) if (c0 == d1[i]) return i;
return _AL_BSTR_ERR;
}

c1 = c0;
j = 0;
lf = b1->slen - 1;

ii = -1;
if (i < lf) do {

if (c1 != d1[i]) {
if (c1 != d1[1+i]) {
i += 2;
continue;
}
i++;
}


if (0 == j) ii = i;


j++;
i++;


if (j < ll) {
c1 = d0[j];
continue;
}

N0:;


if (i == ii+j) return ii;


i -= j;
j = 0;
c1 = c0;
} while (i < lf);


if (i == lf && ll == j+1 && c1 == d1[i]) goto N0;

return _AL_BSTR_ERR;
}










int _al_binstrr (_al_const_bstring b1, int pos, _al_const_bstring b2) {
int j, i, l;
unsigned char * d0, * d1;

if (b1 == NULL || b1->data == NULL || b1->slen < 0 ||
b2 == NULL || b2->data == NULL || b2->slen < 0) return _AL_BSTR_ERR;
if (b1->slen == pos && b2->slen == 0) return pos;
if (b1->slen < pos || pos < 0) return _AL_BSTR_ERR;
if (b2->slen == 0) return pos;


if (b1->data == b2->data && pos == 0 && b2->slen <= b1->slen) return 0;

i = pos;
if ((l = b1->slen - b2->slen) < 0) return _AL_BSTR_ERR;


if (l + 1 <= i) i = l;
j = 0;

d0 = b2->data;
d1 = b1->data;
l = b2->slen;

for (;;) {
if (d0[j] == d1[i + j]) {
j ++;
if (j >= l) return i;
} else {
i --;
if (i < 0) break;
j=0;
}
}

return _AL_BSTR_ERR;
}










int _al_binstrcaseless (_al_const_bstring b1, int pos, _al_const_bstring b2) {
int j, i, l, ll;
unsigned char * d0, * d1;

if (b1 == NULL || b1->data == NULL || b1->slen < 0 ||
b2 == NULL || b2->data == NULL || b2->slen < 0) return _AL_BSTR_ERR;
if (b1->slen == pos) return (b2->slen == 0)?pos:_AL_BSTR_ERR;
if (b1->slen < pos || pos < 0) return _AL_BSTR_ERR;
if (b2->slen == 0) return pos;

l = b1->slen - b2->slen + 1;


if (l <= pos) return _AL_BSTR_ERR;


if (b1->data == b2->data && pos == 0) return _AL_BSTR_OK;

i = pos;
j = 0;

d0 = b2->data;
d1 = b1->data;
ll = b2->slen;

for (;;) {
if (d0[j] == d1[i + j] || downcase (d0[j]) == downcase (d1[i + j])) {
j ++;
if (j >= ll) return i;
} else {
i ++;
if (i >= l) break;
j=0;
}
}

return _AL_BSTR_ERR;
}










int _al_binstrrcaseless (_al_const_bstring b1, int pos, _al_const_bstring b2) {
int j, i, l;
unsigned char * d0, * d1;

if (b1 == NULL || b1->data == NULL || b1->slen < 0 ||
b2 == NULL || b2->data == NULL || b2->slen < 0) return _AL_BSTR_ERR;
if (b1->slen == pos && b2->slen == 0) return pos;
if (b1->slen < pos || pos < 0) return _AL_BSTR_ERR;
if (b2->slen == 0) return pos;


if (b1->data == b2->data && pos == 0 && b2->slen <= b1->slen) return _AL_BSTR_OK;

i = pos;
if ((l = b1->slen - b2->slen) < 0) return _AL_BSTR_ERR;


if (l + 1 <= i) i = l;
j = 0;

d0 = b2->data;
d1 = b1->data;
l = b2->slen;

for (;;) {
if (d0[j] == d1[i + j] || downcase (d0[j]) == downcase (d1[i + j])) {
j ++;
if (j >= l) return i;
} else {
i --;
if (i < 0) break;
j=0;
}
}

return _AL_BSTR_ERR;
}







int _al_bstrchrp (_al_const_bstring b, int c, int pos) {
unsigned char * p;

if (b == NULL || b->data == NULL || b->slen <= pos || pos < 0) return _AL_BSTR_ERR;
p = (unsigned char *) bstr__memchr ((b->data + pos), (unsigned char) c, (b->slen - pos));
if (p) return (int) (p - b->data);
return _AL_BSTR_ERR;
}






int _al_bstrrchrp (_al_const_bstring b, int c, int pos) {
int i;

if (b == NULL || b->data == NULL || b->slen <= pos || pos < 0) return _AL_BSTR_ERR;
for (i=pos; i >= 0; i--) {
if (b->data[i] == (unsigned char) c) return i;
}
return _AL_BSTR_ERR;
}

#if !defined (BSTRLIB_AGGRESSIVE_MEMORY_FOR_SPEED_TRADEOFF)
#define LONG_LOG_BITS_QTY (3)
#define LONG_BITS_QTY (1 << LONG_LOG_BITS_QTY)
#define LONG_TYPE unsigned char

#define CFCLEN ((1 << CHAR_BIT) / LONG_BITS_QTY)
struct charField { LONG_TYPE content[CFCLEN]; };
#define testInCharField(cf,c) ((cf)->content[(c) >> LONG_LOG_BITS_QTY] & (((long)1) << ((c) & (LONG_BITS_QTY-1))))
#define setInCharField(cf,idx) { unsigned int c = (unsigned int) (idx); (cf)->content[c >> LONG_LOG_BITS_QTY] |= (LONG_TYPE) (1ul << (c & (LONG_BITS_QTY-1))); }




#else

#define CFCLEN (1 << CHAR_BIT)
struct charField { unsigned char content[CFCLEN]; };
#define testInCharField(cf,c) ((cf)->content[(unsigned char) (c)])
#define setInCharField(cf,idx) (cf)->content[(unsigned int) (idx)] = ~0

#endif


static int buildCharField (struct charField * cf, _al_const_bstring b) {
int i;
if (b == NULL || b->data == NULL || b->slen <= 0) return _AL_BSTR_ERR;
memset ((void *) cf->content, 0, sizeof (struct charField));
for (i=0; i < b->slen; i++) {
setInCharField (cf, b->data[i]);
}
return _AL_BSTR_OK;
}

static void invertCharField (struct charField * cf) {
int i;
for (i=0; i < CFCLEN; i++) cf->content[i] = ~cf->content[i];
}


static int binchrCF (const unsigned char * data, int len, int pos, const struct charField * cf) {
int i;
for (i=pos; i < len; i++) {
unsigned char c = (unsigned char) data[i];
if (testInCharField (cf, c)) return i;
}
return _AL_BSTR_ERR;
}







int _al_binchr (_al_const_bstring b0, int pos, _al_const_bstring b1) {
struct charField chrs;
if (pos < 0 || b0 == NULL || b0->data == NULL ||
b0->slen <= pos) return _AL_BSTR_ERR;
if (1 == b1->slen) return _al_bstrchrp (b0, b1->data[0], pos);
if (0 > buildCharField (&chrs, b1)) return _AL_BSTR_ERR;
return binchrCF (b0->data, b0->slen, pos, &chrs);
}


static int binchrrCF (const unsigned char * data, int pos, const struct charField * cf) {
int i;
for (i=pos; i >= 0; i--) {
unsigned int c = (unsigned int) data[i];
if (testInCharField (cf, c)) return i;
}
return _AL_BSTR_ERR;
}







int _al_binchrr (_al_const_bstring b0, int pos, _al_const_bstring b1) {
struct charField chrs;
if (pos < 0 || b0 == NULL || b0->data == NULL || b1 == NULL ||
b0->slen < pos) return _AL_BSTR_ERR;
if (pos == b0->slen) pos--;
if (1 == b1->slen) return _al_bstrrchrp (b0, b1->data[0], pos);
if (0 > buildCharField (&chrs, b1)) return _AL_BSTR_ERR;
return binchrrCF (b0->data, pos, &chrs);
}







int _al_bninchr (_al_const_bstring b0, int pos, _al_const_bstring b1) {
struct charField chrs;
if (pos < 0 || b0 == NULL || b0->data == NULL || 
b0->slen <= pos) return _AL_BSTR_ERR;
if (buildCharField (&chrs, b1) < 0) return _AL_BSTR_ERR;
invertCharField (&chrs);
return binchrCF (b0->data, b0->slen, pos, &chrs);
}







int _al_bninchrr (_al_const_bstring b0, int pos, _al_const_bstring b1) {
struct charField chrs;
if (pos < 0 || b0 == NULL || b0->data == NULL || 
b0->slen < pos) return _AL_BSTR_ERR;
if (pos == b0->slen) pos--;
if (buildCharField (&chrs, b1) < 0) return _AL_BSTR_ERR;
invertCharField (&chrs);
return binchrrCF (b0->data, pos, &chrs);
}








int _al_bsetstr (_al_bstring b0, int pos, _al_const_bstring b1, unsigned char fill) {
int d, newlen;
ptrdiff_t pd;
_al_bstring aux = (_al_bstring) b1;

if (pos < 0 || b0 == NULL || b0->slen < 0 || NULL == b0->data || 
b0->mlen < b0->slen || b0->mlen <= 0) return _AL_BSTR_ERR;
if (b1 != NULL && (b1->slen < 0 || b1->data == NULL)) return _AL_BSTR_ERR;

d = pos;


if (NULL != aux) {
if ((pd = (ptrdiff_t) (b1->data - b0->data)) >= 0 && pd < (ptrdiff_t) b0->mlen) {
if (NULL == (aux = _al_bstrcpy (b1))) return _AL_BSTR_ERR;
}
d += aux->slen;
}


if (_al_balloc (b0, d + 1) != _AL_BSTR_OK) {
if (aux != b1) _al_bdestroy (aux);
return _AL_BSTR_ERR;
}

newlen = b0->slen;


if (pos > newlen) {
bstr__memset (b0->data + b0->slen, (int) fill, (size_t) (pos - b0->slen));
newlen = pos;
}


if (aux != NULL) {
bBlockCopy ((char *) (b0->data + pos), (char *) aux->data, aux->slen);
if (aux != b1) _al_bdestroy (aux);
}


if (d > newlen) newlen = d;

b0->slen = newlen;
b0->data[newlen] = (unsigned char) '\0';

return _AL_BSTR_OK;
}








int _al_binsert (_al_bstring b1, int pos, _al_const_bstring b2, unsigned char fill) {
int d, l;
ptrdiff_t pd;
_al_bstring aux = (_al_bstring) b2;

if (pos < 0 || b1 == NULL || b2 == NULL || b1->slen < 0 || 
b2->slen < 0 || b1->mlen < b1->slen || b1->mlen <= 0) return _AL_BSTR_ERR;


if ((pd = (ptrdiff_t) (b2->data - b1->data)) >= 0 && pd < (ptrdiff_t) b1->mlen) {
if (NULL == (aux = _al_bstrcpy (b2))) return _AL_BSTR_ERR;
}


d = b1->slen + aux->slen;
l = pos + aux->slen;
if ((d|l) < 0) return _AL_BSTR_ERR;

if (l > d) {

if (_al_balloc (b1, l + 1) != _AL_BSTR_OK) {
if (aux != b2) _al_bdestroy (aux);
return _AL_BSTR_ERR;
}
bstr__memset (b1->data + b1->slen, (int) fill, (size_t) (pos - b1->slen));
b1->slen = l;
} else {

if (_al_balloc (b1, d + 1) != _AL_BSTR_OK) {
if (aux != b2) _al_bdestroy (aux);
return _AL_BSTR_ERR;
}
bBlockCopy (b1->data + l, b1->data + pos, d - l);
b1->slen = d;
}
bBlockCopy (b1->data + pos, aux->data, aux->slen);
b1->data[b1->slen] = (unsigned char) '\0';
if (aux != b2) _al_bdestroy (aux);
return _AL_BSTR_OK;
}







int _al_breplace (_al_bstring b1, int pos, int len, _al_const_bstring b2, 
unsigned char fill) {
int pl, ret;
ptrdiff_t pd;
_al_bstring aux = (_al_bstring) b2;

if (pos < 0 || len < 0 || (pl = pos + len) < 0 || b1 == NULL || 
b2 == NULL || b1->data == NULL || b2->data == NULL || 
b1->slen < 0 || b2->slen < 0 || b1->mlen < b1->slen ||
b1->mlen <= 0) return _AL_BSTR_ERR;


if (pl >= b1->slen) {
if ((ret = _al_bsetstr (b1, pos, b2, fill)) < 0) return ret;
if (pos + b2->slen < b1->slen) {
b1->slen = pos + b2->slen;
b1->data[b1->slen] = (unsigned char) '\0';
}
return ret;
}


if ((pd = (ptrdiff_t) (b2->data - b1->data)) >= 0 && pd < (ptrdiff_t) b1->slen) {
if (NULL == (aux = _al_bstrcpy (b2))) return _AL_BSTR_ERR;
}

if (aux->slen > len) {
if (_al_balloc (b1, b1->slen + aux->slen - len) != _AL_BSTR_OK) {
if (aux != b2) _al_bdestroy (aux);
return _AL_BSTR_ERR;
}
}

if (aux->slen != len) bstr__memmove (b1->data + pos + aux->slen, b1->data + pos + len, b1->slen - (pos + len));
bstr__memcpy (b1->data + pos, aux->data, aux->slen);
b1->slen += aux->slen - len;
b1->data[b1->slen] = (unsigned char) '\0';
if (aux != b2) _al_bdestroy (aux);
return _AL_BSTR_OK;
}








typedef int (*instr_fnptr) (_al_const_bstring s1, int pos, _al_const_bstring s2);

static int findreplaceengine (_al_bstring b, _al_const_bstring find, _al_const_bstring repl, int pos, instr_fnptr instr) {
int i, ret, slen, mlen, delta, acc;
int * d;
int static_d[32];
ptrdiff_t pd;
_al_bstring auxf = (_al_bstring) find;
_al_bstring auxr = (_al_bstring) repl;

if (b == NULL || b->data == NULL || find == NULL ||
find->data == NULL || repl == NULL || repl->data == NULL || 
pos < 0 || find->slen <= 0 || b->mlen < 0 || b->slen > b->mlen || 
b->mlen <= 0 || b->slen < 0 || repl->slen < 0) return _AL_BSTR_ERR;
if (pos > b->slen - find->slen) return _AL_BSTR_OK;


pd = (ptrdiff_t) (find->data - b->data);
if ((ptrdiff_t) (pos - find->slen) < pd && pd < (ptrdiff_t) b->slen) {
if (NULL == (auxf = _al_bstrcpy (find))) return _AL_BSTR_ERR;
}


pd = (ptrdiff_t) (repl->data - b->data);
if ((ptrdiff_t) (pos - repl->slen) < pd && pd < (ptrdiff_t) b->slen) {
if (NULL == (auxr = _al_bstrcpy (repl))) {
if (auxf != find) _al_bdestroy (auxf);
return _AL_BSTR_ERR;
}
}

delta = auxf->slen - auxr->slen;



if (delta == 0) {
while ((pos = instr (b, pos, auxf)) >= 0) {
bstr__memcpy (b->data + pos, auxr->data, auxr->slen);
pos += auxf->slen;
}
if (auxf != find) _al_bdestroy (auxf);
if (auxr != repl) _al_bdestroy (auxr);
return _AL_BSTR_OK;
}


if (delta > 0) {
acc = 0;

while ((i = instr (b, pos, auxf)) >= 0) {
if (acc && i > pos)
bstr__memmove (b->data + pos - acc, b->data + pos, i - pos);
if (auxr->slen)
bstr__memcpy (b->data + i - acc, auxr->data, auxr->slen);
acc += delta;
pos = i + auxf->slen;
}

if (acc) {
i = b->slen;
if (i > pos)
bstr__memmove (b->data + pos - acc, b->data + pos, i - pos);
b->slen -= acc;
b->data[b->slen] = (unsigned char) '\0';
}

if (auxf != find) _al_bdestroy (auxf);
if (auxr != repl) _al_bdestroy (auxr);
return _AL_BSTR_OK;
}




mlen = 32;
d = (int *) static_d; 
acc = slen = 0;

while ((pos = instr (b, pos, auxf)) >= 0) {
if (slen + 1 >= mlen) {
int sl;
int * t;
mlen += mlen;
sl = sizeof (int *) * mlen;
if (static_d == d) d = NULL;
if (sl < mlen || NULL == (t = (int *) bstr__realloc (d, sl))) {
ret = _AL_BSTR_ERR;
goto done;
}
if (NULL == d) bstr__memcpy (t, static_d, sizeof (static_d));
d = t;
}
d[slen] = pos;
slen++;
acc -= delta;
pos += auxf->slen;
if (pos < 0 || acc < 0) {
ret = _AL_BSTR_ERR;
goto done;
}
}
d[slen] = b->slen;

if (_AL_BSTR_OK == (ret = _al_balloc (b, b->slen + acc + 1))) {
b->slen += acc;
for (i = slen-1; i >= 0; i--) {
int s, l;
s = d[i] + auxf->slen;
l = d[i+1] - s;
if (l) {
bstr__memmove (b->data + s + acc, b->data + s, l);
}
if (auxr->slen) {
bstr__memmove (b->data + s + acc - auxr->slen, 
auxr->data, auxr->slen);
}
acc += delta; 
}
b->data[b->slen] = (unsigned char) '\0';
}

done:;
if (static_d == d) d = NULL;
bstr__free (d);
if (auxf != find) _al_bdestroy (auxf);
if (auxr != repl) _al_bdestroy (auxr);
return ret;
}







int _al_bfindreplace (_al_bstring b, _al_const_bstring find, _al_const_bstring repl, int pos) {
return findreplaceengine (b, find, repl, pos, _al_binstr);
}







int _al_bfindreplacecaseless (_al_bstring b, _al_const_bstring find, _al_const_bstring repl, int pos) {
return findreplaceengine (b, find, repl, pos, _al_binstrcaseless);
}








int _al_binsertch (_al_bstring b, int pos, int len, unsigned char fill) {
int d, l, i;

if (pos < 0 || b == NULL || b->slen < 0 || b->mlen < b->slen ||
b->mlen <= 0 || len < 0) return _AL_BSTR_ERR;


d = b->slen + len;
l = pos + len;
if ((d|l) < 0) return _AL_BSTR_ERR;

if (l > d) {

if (_al_balloc (b, l + 1) != _AL_BSTR_OK) return _AL_BSTR_ERR;
pos = b->slen;
b->slen = l;
} else {

if (_al_balloc (b, d + 1) != _AL_BSTR_OK) return _AL_BSTR_ERR;
for (i = d - 1; i >= l; i--) {
b->data[i] = b->data[i - len];
}
b->slen = d;
}

for (i=pos; i < l; i++) b->data[i] = fill;
b->data[b->slen] = (unsigned char) '\0';
return _AL_BSTR_OK;
}








int _al_bpattern (_al_bstring b, int len) {
int i, d;

d = _al_blength (b);
if (d <= 0 || len < 0 || _al_balloc (b, len + 1) != _AL_BSTR_OK) return _AL_BSTR_ERR;
if (len > 0) {
if (d == 1) return _al_bsetstr (b, len, NULL, b->data[0]);
for (i = d; i < len; i++) b->data[i] = b->data[i - d];
}
b->data[len] = (unsigned char) '\0';
b->slen = len;
return _AL_BSTR_OK;
}

#define BS_BUFF_SZ (1024)







int _al_breada (_al_bstring b, _al_bNread readPtr, void * parm) {
int i, l, n;

if (b == NULL || b->mlen <= 0 || b->slen < 0 || b->mlen < b->slen ||
b->mlen <= 0 || readPtr == NULL) return _AL_BSTR_ERR;

i = b->slen;
for (n=i+16; ; n += ((n < BS_BUFF_SZ) ? n : BS_BUFF_SZ)) {
if (_AL_BSTR_OK != _al_balloc (b, n + 1)) return _AL_BSTR_ERR;
l = (int) readPtr ((void *) (b->data + i), 1, n - i, parm);
i += l;
b->slen = i;
if (i < n) break;
}

b->data[i] = (unsigned char) '\0';
return _AL_BSTR_OK;
}







_al_bstring _al_bread (_al_bNread readPtr, void * parm) {
_al_bstring buff;

if (0 > _al_breada (buff = _al_bfromcstr (""), readPtr, parm)) {
_al_bdestroy (buff);
return NULL;
}
return buff;
}














int _al_bassigngets (_al_bstring b, _al_bNgetc getcPtr, void * parm, char terminator) {
int c, d, e;

if (b == NULL || b->mlen <= 0 || b->slen < 0 || b->mlen < b->slen ||
b->mlen <= 0 || getcPtr == NULL) return _AL_BSTR_ERR;
d = 0;
e = b->mlen - 2;

while ((c = getcPtr (parm)) >= 0) {
if (d > e) {
b->slen = d;
if (_al_balloc (b, d + 2) != _AL_BSTR_OK) return _AL_BSTR_ERR;
e = b->mlen - 2;
}
b->data[d] = (unsigned char) c;
d++;
if (c == terminator) break;
}

b->data[d] = (unsigned char) '\0';
b->slen = d;

return d == 0 && c < 0;
}














int _al_bgetsa (_al_bstring b, _al_bNgetc getcPtr, void * parm, char terminator) {
int c, d, e;

if (b == NULL || b->mlen <= 0 || b->slen < 0 || b->mlen < b->slen ||
b->mlen <= 0 || getcPtr == NULL) return _AL_BSTR_ERR;
d = b->slen;
e = b->mlen - 2;

while ((c = getcPtr (parm)) >= 0) {
if (d > e) {
b->slen = d;
if (_al_balloc (b, d + 2) != _AL_BSTR_OK) return _AL_BSTR_ERR;
e = b->mlen - 2;
}
b->data[d] = (unsigned char) c;
d++;
if (c == terminator) break;
}

b->data[d] = (unsigned char) '\0';
b->slen = d;

return d == 0 && c < 0;
}












_al_bstring _al_bgets (_al_bNgetc getcPtr, void * parm, char terminator) {
_al_bstring buff;

if (0 > _al_bgetsa (buff = _al_bfromcstr (""), getcPtr, parm, terminator) || 0 >= buff->slen) {
_al_bdestroy (buff);
buff = NULL;
}
return buff;
}

struct _al_bStream {
_al_bstring buff; 
void * parm; 
_al_bNread readFnPtr; 
int isEOF; 
int maxBuffSz;
};







struct _al_bStream * _al_bsopen (_al_bNread readPtr, void * parm) {
struct _al_bStream * s;

if (readPtr == NULL) return NULL;
s = (struct _al_bStream *) bstr__alloc (sizeof (struct _al_bStream));
if (s == NULL) return NULL;
s->parm = parm;
s->buff = _al_bfromcstr ("");
s->readFnPtr = readPtr;
s->maxBuffSz = BS_BUFF_SZ;
s->isEOF = 0;
return s;
}






int _al_bsbufflength (struct _al_bStream * s, int sz) {
int oldSz;
if (s == NULL || sz < 0) return _AL_BSTR_ERR;
oldSz = s->maxBuffSz;
if (sz > 0) s->maxBuffSz = sz;
return oldSz;
}

int _al_bseof (const struct _al_bStream * s) {
if (s == NULL || s->readFnPtr == NULL) return _AL_BSTR_ERR;
return s->isEOF && (s->buff->slen == 0);
}






void * _al_bsclose (struct _al_bStream * s) {
void * parm;
if (s == NULL) return NULL;
s->readFnPtr = NULL;
if (s->buff) _al_bdestroy (s->buff);
s->buff = NULL;
parm = s->parm;
s->parm = NULL;
s->isEOF = 1;
bstr__free (s);
return parm;
}








int _al_bsreadlna (_al_bstring r, struct _al_bStream * s, char terminator) {
int i, l, ret, rlo;
char * b;
struct _al_tagbstring x;

if (s == NULL || s->buff == NULL || r == NULL || r->mlen <= 0 ||
r->slen < 0 || r->mlen < r->slen) return _AL_BSTR_ERR;
l = s->buff->slen;
if (_AL_BSTR_OK != _al_balloc (s->buff, s->maxBuffSz + 1)) return _AL_BSTR_ERR;
b = (char *) s->buff->data;
x.data = (unsigned char *) b;


b[l] = terminator; 
for (i=0; b[i] != terminator; i++) ;
if (i < l) {
x.slen = i + 1;
ret = _al_bconcat (r, &x);
s->buff->slen = l;
if (_AL_BSTR_OK == ret) _al_bdelete (s->buff, 0, i + 1);
return _AL_BSTR_OK;
}

rlo = r->slen;


x.slen = l;
if (_AL_BSTR_OK != _al_bconcat (r, &x)) return _AL_BSTR_ERR;



for (;;) {
if (_AL_BSTR_OK != _al_balloc (r, r->slen + s->maxBuffSz + 1)) return _AL_BSTR_ERR;
b = (char *) (r->data + r->slen);
l = (int) s->readFnPtr (b, 1, s->maxBuffSz, s->parm);
if (l <= 0) {
r->data[r->slen] = (unsigned char) '\0';
s->buff->slen = 0;
s->isEOF = 1;

return _AL_BSTR_ERR & -(r->slen == rlo);
}
b[l] = terminator; 
for (i=0; b[i] != terminator; i++) ;
if (i < l) break;
r->slen += l;
}


i++;
r->slen += i;
s->buff->slen = l - i;
bstr__memcpy (s->buff->data, b + i, l - i);
r->data[r->slen] = (unsigned char) '\0';
return _AL_BSTR_OK;
}








int _al_bsreadlnsa (_al_bstring r, struct _al_bStream * s, _al_const_bstring term) {
int i, l, ret, rlo;
unsigned char * b;
struct _al_tagbstring x;
struct charField cf;

if (s == NULL || s->buff == NULL || r == NULL || term == NULL ||
term->data == NULL || r->mlen <= 0 || r->slen < 0 ||
r->mlen < r->slen) return _AL_BSTR_ERR;
if (term->slen == 1) return _al_bsreadlna (r, s, term->data[0]);
if (term->slen < 1 || buildCharField (&cf, term)) return _AL_BSTR_ERR;

l = s->buff->slen;
if (_AL_BSTR_OK != _al_balloc (s->buff, s->maxBuffSz + 1)) return _AL_BSTR_ERR;
b = (unsigned char *) s->buff->data;
x.data = b;


b[l] = term->data[0]; 
for (i=0; !testInCharField (&cf, b[i]); i++) ;
if (i < l) {
x.slen = i + 1;
ret = _al_bconcat (r, &x);
s->buff->slen = l;
if (_AL_BSTR_OK == ret) _al_bdelete (s->buff, 0, i + 1);
return _AL_BSTR_OK;
}

rlo = r->slen;


x.slen = l;
if (_AL_BSTR_OK != _al_bconcat (r, &x)) return _AL_BSTR_ERR;



for (;;) {
if (_AL_BSTR_OK != _al_balloc (r, r->slen + s->maxBuffSz + 1)) return _AL_BSTR_ERR;
b = (unsigned char *) (r->data + r->slen);
l = (int) s->readFnPtr (b, 1, s->maxBuffSz, s->parm);
if (l <= 0) {
r->data[r->slen] = (unsigned char) '\0';
s->buff->slen = 0;
s->isEOF = 1;

return _AL_BSTR_ERR & -(r->slen == rlo);
}

b[l] = term->data[0]; 
for (i=0; !testInCharField (&cf, b[i]); i++) ;
if (i < l) break;
r->slen += l;
}


i++;
r->slen += i;
s->buff->slen = l - i;
bstr__memcpy (s->buff->data, b + i, l - i);
r->data[r->slen] = (unsigned char) '\0';
return _AL_BSTR_OK;
}









int _al_bsreada (_al_bstring r, struct _al_bStream * s, int n) {
int l, ret, orslen;
char * b;
struct _al_tagbstring x;

if (s == NULL || s->buff == NULL || r == NULL || r->mlen <= 0
|| r->slen < 0 || r->mlen < r->slen || n <= 0) return _AL_BSTR_ERR;

n += r->slen;
if (n <= 0) return _AL_BSTR_ERR;

l = s->buff->slen;

orslen = r->slen;

if (0 == l) {
if (s->isEOF) return _AL_BSTR_ERR;
if (r->mlen > n) {
l = (int) s->readFnPtr (r->data + r->slen, 1, n - r->slen, s->parm);
if (0 >= l || l > n - r->slen) {
s->isEOF = 1;
return _AL_BSTR_ERR;
}
r->slen += l;
r->data[r->slen] = (unsigned char) '\0';
return 0;
}
}

if (_AL_BSTR_OK != _al_balloc (s->buff, s->maxBuffSz + 1)) return _AL_BSTR_ERR;
b = (char *) s->buff->data;
x.data = (unsigned char *) b;

do {
if (l + r->slen >= n) {
x.slen = n - r->slen;
ret = _al_bconcat (r, &x);
s->buff->slen = l;
if (_AL_BSTR_OK == ret) _al_bdelete (s->buff, 0, x.slen);
return _AL_BSTR_ERR & -(r->slen == orslen);
}

x.slen = l;
if (_AL_BSTR_OK != _al_bconcat (r, &x)) break;

l = n - r->slen;
if (l > s->maxBuffSz) l = s->maxBuffSz;

l = (int) s->readFnPtr (b, 1, l, s->parm);

} while (l > 0);
if (l < 0) l = 0;
if (l == 0) s->isEOF = 1;
s->buff->slen = l;
return _AL_BSTR_ERR & -(r->slen == orslen);
}








int _al_bsreadln (_al_bstring r, struct _al_bStream * s, char terminator) {
if (s == NULL || s->buff == NULL || r == NULL || r->mlen <= 0)
return _AL_BSTR_ERR;
if (_AL_BSTR_OK != _al_balloc (s->buff, s->maxBuffSz + 1)) return _AL_BSTR_ERR;
r->slen = 0;
return _al_bsreadlna (r, s, terminator);
}








int _al_bsreadlns (_al_bstring r, struct _al_bStream * s, _al_const_bstring term) {
if (s == NULL || s->buff == NULL || r == NULL || term == NULL 
|| term->data == NULL || r->mlen <= 0) return _AL_BSTR_ERR;
if (term->slen == 1) return _al_bsreadln (r, s, term->data[0]);
if (term->slen < 1) return _AL_BSTR_ERR;
if (_AL_BSTR_OK != _al_balloc (s->buff, s->maxBuffSz + 1)) return _AL_BSTR_ERR;
r->slen = 0;
return _al_bsreadlnsa (r, s, term);
}









int _al_bsread (_al_bstring r, struct _al_bStream * s, int n) {
if (s == NULL || s->buff == NULL || r == NULL || r->mlen <= 0
|| n <= 0) return _AL_BSTR_ERR;
if (_AL_BSTR_OK != _al_balloc (s->buff, s->maxBuffSz + 1)) return _AL_BSTR_ERR;
r->slen = 0;
return _al_bsreada (r, s, n);
}







int _al_bsunread (struct _al_bStream * s, _al_const_bstring b) {
if (s == NULL || s->buff == NULL) return _AL_BSTR_ERR;
return _al_binsert (s->buff, 0, b, (unsigned char) '?');
}






int _al_bspeek (_al_bstring r, const struct _al_bStream * s) {
if (s == NULL || s->buff == NULL) return _AL_BSTR_ERR;
return _al_bassign (r, s->buff);
}







_al_bstring _al_bjoin (const struct _al_bstrList * bl, _al_const_bstring sep) {
_al_bstring b;
int i, c, v;

if (bl == NULL || bl->qty < 0) return NULL;
if (sep != NULL && (sep->slen < 0 || sep->data == NULL)) return NULL;

for (i = 0, c = 1; i < bl->qty; i++) {
v = bl->entry[i]->slen;
if (v < 0) return NULL; 
c += v;
if (c < 0) return NULL; 
}

if (sep != NULL) c += (bl->qty - 1) * sep->slen;

b = (_al_bstring) bstr__alloc (sizeof (struct _al_tagbstring));
if (NULL == b) return NULL; 
b->data = (unsigned char *) bstr__alloc (c);
if (b->data == NULL) {
bstr__free (b);
return NULL;
}

b->mlen = c;
b->slen = c-1;

for (i = 0, c = 0; i < bl->qty; i++) {
if (i > 0 && sep != NULL) {
bstr__memcpy (b->data + c, sep->data, sep->slen);
c += sep->slen;
}
v = bl->entry[i]->slen;
bstr__memcpy (b->data + c, bl->entry[i]->data, v);
c += v;
}
b->data[c] = (unsigned char) '\0';
return b;
}

#define BSSSC_BUFF_LEN (256)


















int _al_bssplitscb (struct _al_bStream * s, _al_const_bstring splitStr, 
int (* cb) (void * parm, int ofs, _al_const_bstring entry), void * parm) {
struct charField chrs;
_al_bstring buff;
int i, p, ret;

if (cb == NULL || s == NULL || s->readFnPtr == NULL 
|| splitStr == NULL || splitStr->slen < 0) return _AL_BSTR_ERR;

if (NULL == (buff = _al_bfromcstr (""))) return _AL_BSTR_ERR;

if (splitStr->slen == 0) {
while (_al_bsreada (buff, s, BSSSC_BUFF_LEN) >= 0) ;
if ((ret = cb (parm, 0, buff)) > 0) 
ret = 0;
} else {
buildCharField (&chrs, splitStr);
ret = p = i = 0;
for (;;) {
if (i >= buff->slen) {
_al_bsreada (buff, s, BSSSC_BUFF_LEN);
if (i >= buff->slen) {
if (0 < (ret = cb (parm, p, buff))) ret = 0;
break;
}
}
if (testInCharField (&chrs, buff->data[i])) {
struct _al_tagbstring t;
unsigned char c;

_al_blk2tbstr (t, buff->data + i + 1, buff->slen - (i + 1));
if ((ret = _al_bsunread (s, &t)) < 0) break;
buff->slen = i;
c = buff->data[i];
buff->data[i] = (unsigned char) '\0';
if ((ret = cb (parm, p, buff)) < 0) break;
buff->data[i] = c;
buff->slen = 0;
p += i + 1;
i = -1;
}
i++;
}
}

_al_bdestroy (buff);
return ret;
}


















int _al_bssplitstrcb (struct _al_bStream * s, _al_const_bstring splitStr, 
int (* cb) (void * parm, int ofs, _al_const_bstring entry), void * parm) {
_al_bstring buff;
int i, p, ret;

if (cb == NULL || s == NULL || s->readFnPtr == NULL 
|| splitStr == NULL || splitStr->slen < 0) return _AL_BSTR_ERR;

if (splitStr->slen == 1) return _al_bssplitscb (s, splitStr, cb, parm);

if (NULL == (buff = _al_bfromcstr (""))) return _AL_BSTR_ERR;

if (splitStr->slen == 0) {
for (i=0; _al_bsreada (buff, s, BSSSC_BUFF_LEN) >= 0; i++) {
if ((ret = cb (parm, 0, buff)) < 0) {
_al_bdestroy (buff);
return ret;
}
buff->slen = 0;
}
return _AL_BSTR_OK;
} else {
ret = p = i = 0;
for (i=p=0;;) {
if ((ret = _al_binstr (buff, 0, splitStr)) >= 0) {
struct _al_tagbstring t;
_al_blk2tbstr (t, buff->data, ret);
i = ret + splitStr->slen;
if ((ret = cb (parm, p, &t)) < 0) break;
p += i;
_al_bdelete (buff, 0, i);
} else {
_al_bsreada (buff, s, BSSSC_BUFF_LEN);
if (_al_bseof (s)) {
if ((ret = cb (parm, p, buff)) > 0) ret = 0;
break;
}
}
}
}

_al_bdestroy (buff);
return ret;
}





struct _al_bstrList * _al_bstrListCreate (void) {
struct _al_bstrList * sl = (struct _al_bstrList *) bstr__alloc (sizeof (struct _al_bstrList));
if (sl) {
sl->entry = (_al_bstring *) bstr__alloc (1*sizeof (_al_bstring));
if (!sl->entry) {
bstr__free (sl);
sl = NULL;
} else {
sl->qty = 0;
sl->mlen = 1;
}
}
return sl;
}





int _al_bstrListDestroy (struct _al_bstrList * sl) {
int i;
if (sl == NULL || sl->qty < 0) return _AL_BSTR_ERR;
for (i=0; i < sl->qty; i++) {
if (sl->entry[i]) {
_al_bdestroy (sl->entry[i]);
sl->entry[i] = NULL;
}
}
sl->qty = -1;
sl->mlen = -1;
bstr__free (sl->entry);
sl->entry = NULL;
bstr__free (sl);
return _AL_BSTR_OK;
}






int _al_bstrListAlloc (struct _al_bstrList * sl, int msz) {
_al_bstring * l;
int smsz;
size_t nsz;
if (!sl || msz <= 0 || !sl->entry || sl->qty < 0 || sl->mlen <= 0 || sl->qty > sl->mlen) return _AL_BSTR_ERR;
if (sl->mlen >= msz) return _AL_BSTR_OK;
smsz = snapUpSize (msz);
nsz = ((size_t) smsz) * sizeof (_al_bstring);
if (nsz < (size_t) smsz) return _AL_BSTR_ERR;
l = (_al_bstring *) bstr__realloc (sl->entry, nsz);
if (!l) {
smsz = msz;
nsz = ((size_t) smsz) * sizeof (_al_bstring);
l = (_al_bstring *) bstr__realloc (sl->entry, nsz);
if (!l) return _AL_BSTR_ERR;
}
sl->mlen = smsz;
sl->entry = l;
return _AL_BSTR_OK;
}






int _al_bstrListAllocMin (struct _al_bstrList * sl, int msz) {
_al_bstring * l;
size_t nsz;
if (!sl || msz <= 0 || !sl->entry || sl->qty < 0 || sl->mlen <= 0 || sl->qty > sl->mlen) return _AL_BSTR_ERR;
if (msz < sl->qty) msz = sl->qty;
if (sl->mlen == msz) return _AL_BSTR_OK;
nsz = ((size_t) msz) * sizeof (_al_bstring);
if (nsz < (size_t) msz) return _AL_BSTR_ERR;
l = (_al_bstring *) bstr__realloc (sl->entry, nsz);
if (!l) return _AL_BSTR_ERR;
sl->mlen = msz;
sl->entry = l;
return _AL_BSTR_OK;
}
















int _al_bsplitcb (_al_const_bstring str, unsigned char splitChar, int pos,
int (* cb) (void * parm, int ofs, int len), void * parm) {
int i, p, ret;

if (cb == NULL || str == NULL || pos < 0 || pos > str->slen) 
return _AL_BSTR_ERR;

p = pos;
do {
for (i=p; i < str->slen; i++) {
if (str->data[i] == splitChar) break;
}
if ((ret = cb (parm, p, i - p)) < 0) return ret;
p = i + 1;
} while (p <= str->slen);
return _AL_BSTR_OK;
}

















int _al_bsplitscb (_al_const_bstring str, _al_const_bstring splitStr, int pos,
int (* cb) (void * parm, int ofs, int len), void * parm) {
struct charField chrs;
int i, p, ret;

if (cb == NULL || str == NULL || pos < 0 || pos > str->slen 
|| splitStr == NULL || splitStr->slen < 0) return _AL_BSTR_ERR;
if (splitStr->slen == 0) {
if ((ret = cb (parm, 0, str->slen)) > 0) ret = 0;
return ret;
}

if (splitStr->slen == 1) 
return _al_bsplitcb (str, splitStr->data[0], pos, cb, parm);

buildCharField (&chrs, splitStr);

p = pos;
do {
for (i=p; i < str->slen; i++) {
if (testInCharField (&chrs, str->data[i])) break;
}
if ((ret = cb (parm, p, i - p)) < 0) return ret;
p = i + 1;
} while (p <= str->slen);
return _AL_BSTR_OK;
}

















int _al_bsplitstrcb (_al_const_bstring str, _al_const_bstring splitStr, int pos,
int (* cb) (void * parm, int ofs, int len), void * parm) {
int i, p, ret;

if (cb == NULL || str == NULL || pos < 0 || pos > str->slen 
|| splitStr == NULL || splitStr->slen < 0) return _AL_BSTR_ERR;

if (0 == splitStr->slen) {
for (i=pos; i < str->slen; i++) {
if ((ret = cb (parm, i, 1)) < 0) return ret;
}
return _AL_BSTR_OK;
}

if (splitStr->slen == 1) 
return _al_bsplitcb (str, splitStr->data[0], pos, cb, parm);

for (i=p=pos; i <= str->slen - splitStr->slen; i++) {
if (0 == bstr__memcmp (splitStr->data, str->data + i, splitStr->slen)) {
if ((ret = cb (parm, p, i - p)) < 0) return ret;
i += splitStr->slen;
p = i;
}
}
if ((ret = cb (parm, p, str->slen - p)) < 0) return ret;
return _AL_BSTR_OK;
}

struct genBstrList {
_al_bstring b;
struct _al_bstrList * bl;
};

static int bscb (void * parm, int ofs, int len) {
struct genBstrList * g = (struct genBstrList *) parm;
if (g->bl->qty >= g->bl->mlen) {
int mlen = g->bl->mlen * 2;
_al_bstring * tbl;

while (g->bl->qty >= mlen) {
if (mlen < g->bl->mlen) return _AL_BSTR_ERR;
mlen += mlen;
}

tbl = (_al_bstring *) bstr__realloc (g->bl->entry, sizeof (_al_bstring) * mlen);
if (tbl == NULL) return _AL_BSTR_ERR;

g->bl->entry = tbl;
g->bl->mlen = mlen;
}

g->bl->entry[g->bl->qty] = _al_bmidstr (g->b, ofs, len);
g->bl->qty++;
return _AL_BSTR_OK;
}






struct _al_bstrList * _al_bsplit (_al_const_bstring str, unsigned char splitChar) {
struct genBstrList g;

if (str == NULL || str->data == NULL || str->slen < 0) return NULL;

g.bl = (struct _al_bstrList *) bstr__alloc (sizeof (struct _al_bstrList));
if (g.bl == NULL) return NULL;
g.bl->mlen = 4;
g.bl->entry = (_al_bstring *) bstr__alloc (g.bl->mlen * sizeof (_al_bstring));
if (NULL == g.bl->entry) {
bstr__free (g.bl);
return NULL;
}

g.b = (_al_bstring) str;
g.bl->qty = 0;
if (_al_bsplitcb (str, splitChar, 0, bscb, &g) < 0) {
_al_bstrListDestroy (g.bl);
return NULL;
}
return g.bl;
}






struct _al_bstrList * _al_bsplitstr (_al_const_bstring str, _al_const_bstring splitStr) {
struct genBstrList g;

if (str == NULL || str->data == NULL || str->slen < 0) return NULL;

g.bl = (struct _al_bstrList *) bstr__alloc (sizeof (struct _al_bstrList));
if (g.bl == NULL) return NULL;
g.bl->mlen = 4;
g.bl->entry = (_al_bstring *) bstr__alloc (g.bl->mlen * sizeof (_al_bstring));
if (NULL == g.bl->entry) {
bstr__free (g.bl);
return NULL;
}

g.b = (_al_bstring) str;
g.bl->qty = 0;
if (_al_bsplitstrcb (str, splitStr, 0, bscb, &g) < 0) {
_al_bstrListDestroy (g.bl);
return NULL;
}
return g.bl;
}







struct _al_bstrList * _al_bsplits (_al_const_bstring str, _al_const_bstring splitStr) {
struct genBstrList g;

if ( str == NULL || str->slen < 0 || str->data == NULL ||
splitStr == NULL || splitStr->slen < 0 || splitStr->data == NULL)
return NULL;

g.bl = (struct _al_bstrList *) bstr__alloc (sizeof (struct _al_bstrList));
if (g.bl == NULL) return NULL;
g.bl->mlen = 4;
g.bl->entry = (_al_bstring *) bstr__alloc (g.bl->mlen * sizeof (_al_bstring));
if (NULL == g.bl->entry) {
bstr__free (g.bl);
return NULL;
}
g.b = (_al_bstring) str;
g.bl->qty = 0;

if (_al_bsplitscb (str, splitStr, 0, bscb, &g) < 0) {
_al_bstrListDestroy (g.bl);
return NULL;
}
return g.bl;
}

#if defined (__TURBOC__) && !defined (__BORLANDC__)
#if !defined(BSTRLIB_NOVSNP)
#define BSTRLIB_NOVSNP
#endif
#endif


#if defined(__WATCOMC__) || defined(_MSC_VER)
#define exvsnprintf(r,b,n,f,a) {r = _vsnprintf (b,n,f,a);}
#else
#if defined(BSTRLIB_NOVSNP)


#define exvsnprintf(r,b,n,f,a) {vsprintf (b,f,a); r = -1;}
#define START_VSNBUFF (256)
#else

#if defined(__GNUC__)








#endif

#define exvsnprintf(r,b,n,f,a) {r = vsnprintf (b,n,f,a);}
#endif
#endif

#if !defined (BSTRLIB_NOVSNP)

#if !defined(START_VSNBUFF)
#define START_VSNBUFF (16)
#endif
















int _al_bformata (_al_bstring b, const char * fmt, ...) {
va_list arglist;
_al_bstring buff;
int n, r;

if (b == NULL || fmt == NULL || b->data == NULL || b->mlen <= 0 
|| b->slen < 0 || b->slen > b->mlen) return _AL_BSTR_ERR;





if ((n = (int) (2*strlen (fmt))) < START_VSNBUFF) n = START_VSNBUFF;
if (NULL == (buff = _al_bfromcstralloc (n + 2, ""))) {
n = 1;
if (NULL == (buff = _al_bfromcstralloc (n + 2, ""))) return _AL_BSTR_ERR;
}

for (;;) {
va_start (arglist, fmt);
exvsnprintf (r, (char *) buff->data, n + 1, fmt, arglist);
va_end (arglist);

buff->data[n] = (unsigned char) '\0';
buff->slen = (int) (strlen) ((char *) buff->data);

if (buff->slen < n) break;

if (r > n) n = r; else n += n;

if (_AL_BSTR_OK != _al_balloc (buff, n + 2)) {
_al_bdestroy (buff);
return _AL_BSTR_ERR;
}
}

r = _al_bconcat (b, buff);
_al_bdestroy (buff);
return r;
}








int _al_bassignformat (_al_bstring b, const char * fmt, ...) {
va_list arglist;
_al_bstring buff;
int n, r;

if (b == NULL || fmt == NULL || b->data == NULL || b->mlen <= 0 
|| b->slen < 0 || b->slen > b->mlen) return _AL_BSTR_ERR;





if ((n = (int) (2*strlen (fmt))) < START_VSNBUFF) n = START_VSNBUFF;
if (NULL == (buff = _al_bfromcstralloc (n + 2, ""))) {
n = 1;
if (NULL == (buff = _al_bfromcstralloc (n + 2, ""))) return _AL_BSTR_ERR;
}

for (;;) {
va_start (arglist, fmt);
exvsnprintf (r, (char *) buff->data, n + 1, fmt, arglist);
va_end (arglist);

buff->data[n] = (unsigned char) '\0';
buff->slen = (int) (strlen) ((char *) buff->data);

if (buff->slen < n) break;

if (r > n) n = r; else n += n;

if (_AL_BSTR_OK != _al_balloc (buff, n + 2)) {
_al_bdestroy (buff);
return _AL_BSTR_ERR;
}
}

r = _al_bassign (b, buff);
_al_bdestroy (buff);
return r;
}








_al_bstring _al_bformat (const char * fmt, ...) {
va_list arglist;
_al_bstring buff;
int n, r;

if (fmt == NULL) return NULL;





if ((n = (int) (2*strlen (fmt))) < START_VSNBUFF) n = START_VSNBUFF;
if (NULL == (buff = _al_bfromcstralloc (n + 2, ""))) {
n = 1;
if (NULL == (buff = _al_bfromcstralloc (n + 2, ""))) return NULL;
}

for (;;) {
va_start (arglist, fmt);
exvsnprintf (r, (char *) buff->data, n + 1, fmt, arglist);
va_end (arglist);

buff->data[n] = (unsigned char) '\0';
buff->slen = (int) (strlen) ((char *) buff->data);

if (buff->slen < n) break;

if (r > n) n = r; else n += n;

if (_AL_BSTR_OK != _al_balloc (buff, n + 2)) {
_al_bdestroy (buff);
return NULL;
}
}

return buff;
}





















int _al_bvcformata (_al_bstring b, int count, const char * fmt, va_list arg) {
int n, r, l;

if (b == NULL || fmt == NULL || count <= 0 || b->data == NULL
|| b->mlen <= 0 || b->slen < 0 || b->slen > b->mlen) return _AL_BSTR_ERR;

if (count > (n = b->slen + count) + 2) return _AL_BSTR_ERR;
if (_AL_BSTR_OK != _al_balloc (b, n + 2)) return _AL_BSTR_ERR;

exvsnprintf (r, (char *) b->data + b->slen, count + 2, fmt, arg);



if (n >= (l = b->slen + (int) (strlen) ((const char *) b->data + b->slen))) {
b->slen = l;
return _AL_BSTR_OK;
}




b->data[b->slen] = '\0';
if (r > count+1) l = r; else {
l = count+count;
if (count > l) l = INT_MAX;
}
n = -l;
if (n > _AL_BSTR_ERR-1) n = _AL_BSTR_ERR-1;
return n;
}

#endif
