































































#define RESCORE(word_score, sound_score) ((3 * word_score + sound_score) / 4)



#define MAXSCORE(word_score, sound_score) ((4 * word_score - sound_score) / 3)

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <wctype.h>


#include <stddef.h>

#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/spell.h"
#include "nvim/buffer.h"
#include "nvim/change.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/ex_cmds.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_docmd.h"
#include "nvim/fileio.h"
#include "nvim/func_attr.h"
#include "nvim/getchar.h"
#include "nvim/hashtab.h"
#include "nvim/mark.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/garray.h"
#include "nvim/normal.h"
#include "nvim/option.h"
#include "nvim/os_unix.h"
#include "nvim/path.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/spellfile.h"
#include "nvim/strings.h"
#include "nvim/syntax.h"
#include "nvim/undo.h"
#include "nvim/os/os.h"
#include "nvim/os/input.h"


#define WF_MIXCAP 0x20 

#define WF_CAPMASK (WF_ONECAP | WF_ALLCAP | WF_KEEPCAP | WF_FIXCAP)


#define SP_BANNED -1
#define SP_RARE 0
#define SP_OK 1
#define SP_LOCAL 2
#define SP_BAD 3



slang_T *first_lang = NULL;


char_u *int_wordlist = NULL;

typedef struct wordcount_S {
uint16_t wc_count; 
char_u wc_word[1]; 
} wordcount_T;

#define WC_KEY_OFF offsetof(wordcount_T, wc_word)
#define HI2WC(hi) ((wordcount_T *)((hi)->hi_key - WC_KEY_OFF))
#define MAXWORDCOUNT 0xffff


typedef struct suginfo_S {
garray_T su_ga; 
int su_maxcount; 
int su_maxscore; 
int su_sfmaxscore; 
garray_T su_sga; 
char_u *su_badptr; 
int su_badlen; 
int su_badflags; 
char_u su_badword[MAXWLEN]; 
char_u su_fbadword[MAXWLEN]; 
char_u su_sal_badword[MAXWLEN]; 
hashtab_T su_banned; 
slang_T *su_sallang; 
} suginfo_T;


typedef struct {
char_u *st_word; 
int st_wordlen; 
int st_orglen; 
int st_score; 
int st_altscore; 
bool st_salscore; 
bool st_had_bonus; 
slang_T *st_slang; 
} suggest_T;

#define SUG(ga, i) (((suggest_T *)(ga).ga_data)[i])


#define WAS_BANNED(su, word) (!HASHITEM_EMPTY(hash_find(&su->su_banned, word)))




#define SUG_CLEAN_COUNT(su) ((su)->su_maxcount < 130 ? 150 : (su)->su_maxcount + 20)




#define SUG_MAX_COUNT(su) (SUG_CLEAN_COUNT(su) + 50)


#define SCORE_SPLIT 149 
#define SCORE_SPLIT_NO 249 
#define SCORE_ICASE 52 
#define SCORE_REGION 200 
#define SCORE_RARE 180 
#define SCORE_SWAP 75 
#define SCORE_SWAP3 110 
#define SCORE_REP 65 
#define SCORE_SUBST 93 
#define SCORE_SIMILAR 33 
#define SCORE_SUBCOMP 33 
#define SCORE_DEL 94 
#define SCORE_DELDUP 66 
#define SCORE_DELCOMP 28 
#define SCORE_INS 96 
#define SCORE_INSDUP 67 
#define SCORE_INSCOMP 30 
#define SCORE_NONWORD 103 

#define SCORE_FILE 30 
#define SCORE_MAXINIT 350 


#define SCORE_COMMON1 30 
#define SCORE_COMMON2 40 
#define SCORE_COMMON3 50 
#define SCORE_THRES2 10 
#define SCORE_THRES3 100 




#define SCORE_SFMAX1 200 
#define SCORE_SFMAX2 300 
#define SCORE_SFMAX3 400 

#define SCORE_BIG SCORE_INS * 3 
#define SCORE_MAXMAX 999999 
#define SCORE_LIMITMAX 350 



#define SCORE_EDIT_MIN SCORE_SIMILAR


typedef struct matchinf_S {
langp_T *mi_lp; 


char_u *mi_word; 
char_u *mi_end; 
char_u *mi_fend; 
char_u *mi_cend; 



char_u mi_fword[MAXWLEN + 1]; 
int mi_fwordlen; 


int mi_prefarridx; 

int mi_prefcnt; 
int mi_prefixlen; 
int mi_cprefixlen; 



int mi_compoff; 
char_u mi_compflags[MAXWLEN]; 
int mi_complen; 
int mi_compextra; 


int mi_result; 
int mi_capflags; 
win_T *mi_win; 


int mi_result2; 
char_u *mi_end2; 
} matchinf_T;


typedef struct spelload_S {
char_u sl_lang[MAXWLEN + 1]; 
slang_T *sl_slang; 
int sl_nobreak; 
} spelload_T;

#define SY_MAXLEN 30
typedef struct syl_item_S {
char_u sy_chars[SY_MAXLEN]; 
int sy_len;
} syl_item_T;

spelltab_T spelltab;
int did_set_spelltab;



typedef struct {
short sft_score; 
char_u sft_word[1]; 
} sftword_T;

typedef struct {
int badi;
int goodi;
int score;
} limitscore_T;


#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "spell.c.generated.h"
#endif


#define DIFF_NONE 0 
#define DIFF_YES 1 
#define DIFF_INSERT 2 


#define TSF_PREFIXOK 1 
#define TSF_DIDSPLIT 2 
#define TSF_DIDDEL 4 


#define PFD_NOPREFIX 0xff 
#define PFD_PREFIXTREE 0xfe 
#define PFD_NOTSPECIAL 0xfd 


#define FIND_FOLDWORD 0 
#define FIND_KEEPWORD 1 
#define FIND_PREFIX 2 
#define FIND_COMPOUND 3 
#define FIND_KEEPCOMPOUND 4 

char *e_format = N_("E759: Format error in spell file");


static char_u *repl_from = NULL;
static char_u *repl_to = NULL;














size_t spell_check(
win_T *wp, 
char_u *ptr,
hlf_T *attrp,
int *capcol, 
bool docount 
)
{
matchinf_T mi; 

size_t nrlen = 0; 
int c;
size_t wrongcaplen = 0;
int lpi;
bool count_word = docount;



if (*ptr <= ' ') {
return 1;
}


if (GA_EMPTY(&wp->w_s->b_langp)) {
return 1;
}

memset(&mi, 0, sizeof(matchinf_T));




if (*ptr >= '0' && *ptr <= '9') {
if (*ptr == '0' && (ptr[1] == 'b' || ptr[1] == 'B')) {
mi.mi_end = (char_u*) skipbin((char*) ptr + 2);
} else if (*ptr == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) {
mi.mi_end = skiphex(ptr + 2);
} else {
mi.mi_end = skipdigits(ptr);
}
nrlen = (size_t)(mi.mi_end - ptr);
}


mi.mi_word = ptr;
mi.mi_fend = ptr;
if (spell_iswordp(mi.mi_fend, wp)) {
do {
MB_PTR_ADV(mi.mi_fend);
} while (*mi.mi_fend != NUL && spell_iswordp(mi.mi_fend, wp));

if (capcol != NULL && *capcol == 0 && wp->w_s->b_cap_prog != NULL) {

c = PTR2CHAR(ptr);
if (!SPELL_ISUPPER(c)) {
wrongcaplen = (size_t)(mi.mi_fend - ptr);
}
}
}
if (capcol != NULL) {
*capcol = -1;
}



mi.mi_end = mi.mi_fend;


mi.mi_capflags = 0;
mi.mi_cend = NULL;
mi.mi_win = wp;



if (*mi.mi_fend != NUL) {
MB_PTR_ADV(mi.mi_fend);
}

(void)spell_casefold(ptr, (int)(mi.mi_fend - ptr), mi.mi_fword, MAXWLEN + 1);
mi.mi_fwordlen = (int)STRLEN(mi.mi_fword);


mi.mi_result = SP_BAD;
mi.mi_result2 = SP_BAD;




for (lpi = 0; lpi < wp->w_s->b_langp.ga_len; ++lpi) {
mi.mi_lp = LANGP_ENTRY(wp->w_s->b_langp, lpi);



if (mi.mi_lp->lp_slang->sl_fidxs == NULL) {
continue;
}


find_word(&mi, FIND_FOLDWORD);


find_word(&mi, FIND_KEEPWORD);


find_prefix(&mi, FIND_FOLDWORD);



if (mi.mi_lp->lp_slang->sl_nobreak && mi.mi_result == SP_BAD
&& mi.mi_result2 != SP_BAD) {
mi.mi_result = mi.mi_result2;
mi.mi_end = mi.mi_end2;
}


if (count_word && mi.mi_result == SP_OK) {
count_common_word(mi.mi_lp->lp_slang, ptr,
(int)(mi.mi_end - ptr), 1);
count_word = false;
}
}

if (mi.mi_result != SP_OK) {


if (nrlen > 0) {
if (mi.mi_result == SP_BAD || mi.mi_result == SP_BANNED) {
return nrlen;
}
} else if (!spell_iswordp_nmw(ptr, wp)) {


if (capcol != NULL && wp->w_s->b_cap_prog != NULL) {
regmatch_T regmatch;


regmatch.regprog = wp->w_s->b_cap_prog;
regmatch.rm_ic = false;
int r = vim_regexec(&regmatch, ptr, 0);
wp->w_s->b_cap_prog = regmatch.regprog;
if (r) {
*capcol = (int)(regmatch.endp[0] - ptr);
}
}

if (has_mbyte) {
return (size_t)(*mb_ptr2len)(ptr);
}
return 1;
} else if (mi.mi_end == ptr) {


MB_PTR_ADV(mi.mi_end);
} else if (mi.mi_result == SP_BAD
&& LANGP_ENTRY(wp->w_s->b_langp, 0)->lp_slang->sl_nobreak) {
char_u *p, *fp;
int save_result = mi.mi_result;



mi.mi_lp = LANGP_ENTRY(wp->w_s->b_langp, 0);
if (mi.mi_lp->lp_slang->sl_fidxs != NULL) {
p = mi.mi_word;
fp = mi.mi_fword;
for (;;) {
MB_PTR_ADV(p);
MB_PTR_ADV(fp);
if (p >= mi.mi_end) {
break;
}
mi.mi_compoff = (int)(fp - mi.mi_fword);
find_word(&mi, FIND_COMPOUND);
if (mi.mi_result != SP_BAD) {
mi.mi_end = p;
break;
}
}
mi.mi_result = save_result;
}
}

if (mi.mi_result == SP_BAD || mi.mi_result == SP_BANNED) {
*attrp = HLF_SPB;
} else if (mi.mi_result == SP_RARE) {
*attrp = HLF_SPR;
} else {
*attrp = HLF_SPL;
}
}

if (wrongcaplen > 0 && (mi.mi_result == SP_OK || mi.mi_result == SP_RARE)) {

*attrp = HLF_SPC;
return wrongcaplen;
}

return (size_t)(mi.mi_end - ptr);
}








static void find_word(matchinf_T *mip, int mode)
{
int wlen = 0;
int flen;
char_u *ptr;
slang_T *slang = mip->mi_lp->lp_slang;
char_u *byts;
idx_T *idxs;

if (mode == FIND_KEEPWORD || mode == FIND_KEEPCOMPOUND) {

ptr = mip->mi_word;
flen = 9999; 
byts = slang->sl_kbyts;
idxs = slang->sl_kidxs;

if (mode == FIND_KEEPCOMPOUND)

wlen += mip->mi_compoff;
} else {

ptr = mip->mi_fword;
flen = mip->mi_fwordlen; 
byts = slang->sl_fbyts;
idxs = slang->sl_fidxs;

if (mode == FIND_PREFIX) {

wlen = mip->mi_prefixlen;
flen -= mip->mi_prefixlen;
} else if (mode == FIND_COMPOUND) {

wlen = mip->mi_compoff;
flen -= mip->mi_compoff;
}

}

if (byts == NULL)
return; 

idx_T arridx = 0;
int endlen[MAXWLEN]; 
idx_T endidx[MAXWLEN]; 
int endidxcnt = 0;
int len;
int c;





for (;; ) {
if (flen <= 0 && *mip->mi_fend != NUL)
flen = fold_more(mip);

len = byts[arridx++];



if (byts[arridx] == 0) {
if (endidxcnt == MAXWLEN) {

EMSG(_(e_format));
return;
}
endlen[endidxcnt] = wlen;
endidx[endidxcnt++] = arridx++;
--len;



while (len > 0 && byts[arridx] == 0) {
++arridx;
--len;
}
if (len == 0)
break; 
}


if (ptr[wlen] == NUL)
break;


c = ptr[wlen];
if (c == TAB) 
c = ' ';
idx_T lo = arridx;
idx_T hi = arridx + len - 1;
while (lo < hi) {
idx_T m = (lo + hi) / 2;
if (byts[m] > c)
hi = m - 1;
else if (byts[m] < c)
lo = m + 1;
else {
lo = hi = m;
break;
}
}


if (hi < lo || byts[lo] != c)
break;


arridx = idxs[lo];
++wlen;
--flen;



if (c == ' ') {
for (;; ) {
if (flen <= 0 && *mip->mi_fend != NUL)
flen = fold_more(mip);
if (ptr[wlen] != ' ' && ptr[wlen] != TAB)
break;
++wlen;
--flen;
}
}
}

char_u *p;
bool word_ends;



while (endidxcnt > 0) {
--endidxcnt;
arridx = endidx[endidxcnt];
wlen = endlen[endidxcnt];

if (utf_head_off(ptr, ptr + wlen) > 0) {
continue; 
}
if (spell_iswordp(ptr + wlen, mip->mi_win)) {
if (slang->sl_compprog == NULL && !slang->sl_nobreak)
continue; 
word_ends = false;
} else
word_ends = true;


bool prefix_found = false;

if (mode != FIND_KEEPWORD && has_mbyte) {



p = mip->mi_word;
if (STRNCMP(ptr, p, wlen) != 0) {
for (char_u *s = ptr; s < ptr + wlen; MB_PTR_ADV(s)) {
MB_PTR_ADV(p);
}
wlen = (int)(p - mip->mi_word);
}
}





for (len = byts[arridx - 1]; len > 0 && byts[arridx] == 0;
--len, ++arridx) {
uint32_t flags = idxs[arridx];





if (mode == FIND_FOLDWORD) {
if (mip->mi_cend != mip->mi_word + wlen) {


mip->mi_cend = mip->mi_word + wlen;
mip->mi_capflags = captype(mip->mi_word, mip->mi_cend);
}

if (mip->mi_capflags == WF_KEEPCAP
|| !spell_valid_case(mip->mi_capflags, flags))
continue;
}



else if (mode == FIND_PREFIX && !prefix_found) {
c = valid_word_prefix(mip->mi_prefcnt, mip->mi_prefarridx,
flags,
mip->mi_word + mip->mi_cprefixlen, slang,
false);
if (c == 0)
continue;


if (c & WF_RAREPFX)
flags |= WF_RARE;
prefix_found = true;
}

if (slang->sl_nobreak) {
if ((mode == FIND_COMPOUND || mode == FIND_KEEPCOMPOUND)
&& (flags & WF_BANNED) == 0) {


mip->mi_result = SP_OK;
break;
}
} else if ((mode == FIND_COMPOUND || mode == FIND_KEEPCOMPOUND
|| !word_ends)) {





if (((unsigned)flags >> 24) == 0
|| wlen - mip->mi_compoff < slang->sl_compminlen)
continue;


if (has_mbyte
&& slang->sl_compminlen > 0
&& mb_charlen_len(mip->mi_word + mip->mi_compoff,
wlen - mip->mi_compoff) < slang->sl_compminlen)
continue;



if (!word_ends && mip->mi_complen + mip->mi_compextra + 2
> slang->sl_compmax
&& slang->sl_compsylmax == MAXWLEN)
continue;



if (mip->mi_complen > 0 && (flags & WF_NOCOMPBEF))
continue;
if (!word_ends && (flags & WF_NOCOMPAFT))
continue;


if (!byte_in_str(mip->mi_complen == 0
? slang->sl_compstartflags
: slang->sl_compallflags,
((unsigned)flags >> 24)))
continue;



if (match_checkcompoundpattern(ptr, wlen, &slang->sl_comppat))
continue;

if (mode == FIND_COMPOUND) {
int capflags;



if (has_mbyte && STRNCMP(ptr, mip->mi_word,
mip->mi_compoff) != 0) {

p = mip->mi_word;
for (char_u *s = ptr; s < ptr + mip->mi_compoff; MB_PTR_ADV(s)) {
MB_PTR_ADV(p);
}
} else {
p = mip->mi_word + mip->mi_compoff;
}
capflags = captype(p, mip->mi_word + wlen);
if (capflags == WF_KEEPCAP || (capflags == WF_ALLCAP
&& (flags & WF_FIXCAP) != 0))
continue;

if (capflags != WF_ALLCAP) {




MB_PTR_BACK(mip->mi_word, p);
if (spell_iswordp_nmw(p, mip->mi_win)
? capflags == WF_ONECAP
: (flags & WF_ONECAP) != 0
&& capflags != WF_ONECAP) {
continue;
}
}
}




mip->mi_compflags[mip->mi_complen] = ((unsigned)flags >> 24);
mip->mi_compflags[mip->mi_complen + 1] = NUL;
if (word_ends) {
char_u fword[MAXWLEN] = { 0 };

if (slang->sl_compsylmax < MAXWLEN) {

if (ptr == mip->mi_word)
(void)spell_casefold(ptr, wlen, fword, MAXWLEN);
else
STRLCPY(fword, ptr, endlen[endidxcnt] + 1);
}
if (!can_compound(slang, fword, mip->mi_compflags))
continue;
} else if (slang->sl_comprules != NULL
&& !match_compoundrule(slang, mip->mi_compflags))


continue;
}

else if (flags & WF_NEEDCOMP)
continue;

int nobreak_result = SP_OK;

if (!word_ends) {
int save_result = mip->mi_result;
char_u *save_end = mip->mi_end;
langp_T *save_lp = mip->mi_lp;






if (slang->sl_nobreak)
mip->mi_result = SP_BAD;


mip->mi_compoff = endlen[endidxcnt];
if (has_mbyte && mode == FIND_KEEPWORD) {




p = mip->mi_fword;
if (STRNCMP(ptr, p, wlen) != 0) {
for (char_u *s = ptr; s < ptr + wlen; MB_PTR_ADV(s)) {
MB_PTR_ADV(p);
}
mip->mi_compoff = (int)(p - mip->mi_fword);
}
}
#if 0
c = mip->mi_compoff;
#endif
++mip->mi_complen;
if (flags & WF_COMPROOT)
++mip->mi_compextra;



for (int lpi = 0; lpi < mip->mi_win->w_s->b_langp.ga_len; ++lpi) {
if (slang->sl_nobreak) {
mip->mi_lp = LANGP_ENTRY(mip->mi_win->w_s->b_langp, lpi);
if (mip->mi_lp->lp_slang->sl_fidxs == NULL
|| !mip->mi_lp->lp_slang->sl_nobreak)
continue;
}

find_word(mip, FIND_COMPOUND);




if (!slang->sl_nobreak || mip->mi_result == SP_BAD) {

mip->mi_compoff = wlen;
find_word(mip, FIND_KEEPCOMPOUND);

#if 0 


if (!slang->sl_nobreak || mip->mi_result == SP_BAD) {

mip->mi_compoff = c;
find_prefix(mip, FIND_COMPOUND);
}
#endif
}

if (!slang->sl_nobreak)
break;
}
--mip->mi_complen;
if (flags & WF_COMPROOT)
--mip->mi_compextra;
mip->mi_lp = save_lp;

if (slang->sl_nobreak) {
nobreak_result = mip->mi_result;
mip->mi_result = save_result;
mip->mi_end = save_end;
} else {
if (mip->mi_result == SP_OK)
break;
continue;
}
}

int res = SP_BAD;
if (flags & WF_BANNED)
res = SP_BANNED;
else if (flags & WF_REGION) {

if ((mip->mi_lp->lp_region & (flags >> 16)) != 0)
res = SP_OK;
else
res = SP_LOCAL;
} else if (flags & WF_RARE)
res = SP_RARE;
else
res = SP_OK;




if (nobreak_result == SP_BAD) {
if (mip->mi_result2 > res) {
mip->mi_result2 = res;
mip->mi_end2 = mip->mi_word + wlen;
} else if (mip->mi_result2 == res
&& mip->mi_end2 < mip->mi_word + wlen)
mip->mi_end2 = mip->mi_word + wlen;
} else if (mip->mi_result > res) {
mip->mi_result = res;
mip->mi_end = mip->mi_word + wlen;
} else if (mip->mi_result == res && mip->mi_end < mip->mi_word + wlen)
mip->mi_end = mip->mi_word + wlen;

if (mip->mi_result == SP_OK)
break;
}

if (mip->mi_result == SP_OK)
break;
}
}






static bool
match_checkcompoundpattern (
char_u *ptr,
int wlen,
garray_T *gap 
)
{
char_u *p;
int len;

for (int i = 0; i + 1 < gap->ga_len; i += 2) {
p = ((char_u **)gap->ga_data)[i + 1];
if (STRNCMP(ptr + wlen, p, STRLEN(p)) == 0) {


p = ((char_u **)gap->ga_data)[i];
len = (int)STRLEN(p);
if (len <= wlen && STRNCMP(ptr + wlen - len, p, len) == 0)
return true;
}
}
return false;
}



static bool can_compound(slang_T *slang, const char_u *word,
const char_u *flags)
FUNC_ATTR_NONNULL_ALL
{
char_u uflags[MAXWLEN * 2] = { 0 };

if (slang->sl_compprog == NULL) {
return false;
}

char_u *p = uflags;
for (int i = 0; flags[i] != NUL; i++) {
p += utf_char2bytes(flags[i], p);
}
*p = NUL;
p = uflags;
if (!vim_regexec_prog(&slang->sl_compprog, false, p, 0)) {
return false;
}




if (slang->sl_compsylmax < MAXWLEN
&& count_syllables(slang, word) > slang->sl_compsylmax)
return (int)STRLEN(flags) < slang->sl_compmax;
return true;
}




static bool can_be_compound(trystate_T *sp, slang_T *slang, char_u *compflags, int flag)
{


if (!byte_in_str(sp->ts_complen == sp->ts_compsplit
? slang->sl_compstartflags : slang->sl_compallflags, flag))
return false;




if (slang->sl_comprules != NULL && sp->ts_complen > sp->ts_compsplit) {
compflags[sp->ts_complen] = flag;
compflags[sp->ts_complen + 1] = NUL;
bool v = match_compoundrule(slang, compflags + sp->ts_compsplit);
compflags[sp->ts_complen] = NUL;
return v;
}

return true;
}





static bool match_compoundrule(slang_T *slang, char_u *compflags)
{
char_u *p;
int i;
int c;


for (p = slang->sl_comprules; *p != NUL; ++p) {


for (i = 0;; ++i) {
c = compflags[i];
if (c == NUL)

return true;
if (*p == '/' || *p == NUL)
break; 
if (*p == '[') {
bool match = false;


++p;
while (*p != ']' && *p != NUL)
if (*p++ == c)
match = true;
if (!match)
break; 
} else if (*p != c)
break; 
++p;
}


p = vim_strchr(p, '/');
if (p == NULL)
break;
}



return false;
}




static int
valid_word_prefix (
int totprefcnt, 
int arridx, 
int flags,
char_u *word,
slang_T *slang,
bool cond_req 
)
{
int prefcnt;
int pidx;
int prefid;

prefid = (unsigned)flags >> 24;
for (prefcnt = totprefcnt - 1; prefcnt >= 0; --prefcnt) {
pidx = slang->sl_pidxs[arridx + prefcnt];


if (prefid != (pidx & 0xff))
continue;



if ((flags & WF_HAS_AFF) && (pidx & WF_PFX_NC))
continue;



regprog_T **rp = &slang->sl_prefprog[((unsigned)pidx >> 8) & 0xffff];
if (*rp != NULL) {
if (!vim_regexec_prog(rp, false, word, 0)) {
continue;
}
} else if (cond_req)
continue;


return pidx;
}
return 0;
}








static void find_prefix(matchinf_T *mip, int mode)
{
idx_T arridx = 0;
int len;
int wlen = 0;
int flen;
int c;
char_u *ptr;
idx_T lo, hi, m;
slang_T *slang = mip->mi_lp->lp_slang;
char_u *byts;
idx_T *idxs;

byts = slang->sl_pbyts;
if (byts == NULL)
return; 



ptr = mip->mi_fword;
flen = mip->mi_fwordlen; 
if (mode == FIND_COMPOUND) {

ptr += mip->mi_compoff;
flen -= mip->mi_compoff;
}
idxs = slang->sl_pidxs;





for (;; ) {
if (flen == 0 && *mip->mi_fend != NUL)
flen = fold_more(mip);

len = byts[arridx++];



if (byts[arridx] == 0) {




mip->mi_prefarridx = arridx;
mip->mi_prefcnt = len;
while (len > 0 && byts[arridx] == 0) {
++arridx;
--len;
}
mip->mi_prefcnt -= len;


mip->mi_prefixlen = wlen;
if (mode == FIND_COMPOUND)

mip->mi_prefixlen += mip->mi_compoff;

if (has_mbyte) {

mip->mi_cprefixlen = nofold_len(mip->mi_fword,
mip->mi_prefixlen, mip->mi_word);
} else
mip->mi_cprefixlen = mip->mi_prefixlen;
find_word(mip, FIND_PREFIX);


if (len == 0)
break; 
}


if (ptr[wlen] == NUL)
break;


c = ptr[wlen];
lo = arridx;
hi = arridx + len - 1;
while (lo < hi) {
m = (lo + hi) / 2;
if (byts[m] > c)
hi = m - 1;
else if (byts[m] < c)
lo = m + 1;
else {
lo = hi = m;
break;
}
}


if (hi < lo || byts[lo] != c)
break;


arridx = idxs[lo];
++wlen;
--flen;
}
}




static int fold_more(matchinf_T *mip)
{
int flen;
char_u *p;

p = mip->mi_fend;
do {
MB_PTR_ADV(mip->mi_fend);
} while (*mip->mi_fend != NUL && spell_iswordp(mip->mi_fend, mip->mi_win));


if (*mip->mi_fend != NUL) {
MB_PTR_ADV(mip->mi_fend);
}

(void)spell_casefold(p, (int)(mip->mi_fend - p),
mip->mi_fword + mip->mi_fwordlen,
MAXWLEN - mip->mi_fwordlen);
flen = (int)STRLEN(mip->mi_fword + mip->mi_fwordlen);
mip->mi_fwordlen += flen;
return flen;
}






static bool spell_valid_case(int wordflags, int treeflags)
{
return (wordflags == WF_ALLCAP && (treeflags & WF_FIXCAP) == 0)
|| ((treeflags & (WF_ALLCAP | WF_KEEPCAP)) == 0
&& ((treeflags & WF_ONECAP) == 0
|| (wordflags & WF_ONECAP) != 0));
}


static bool no_spell_checking(win_T *wp)
{
if (!wp->w_p_spell || *wp->w_s->b_p_spl == NUL
|| GA_EMPTY(&wp->w_s->b_langp)) {
EMSG(_("E756: Spell checking is not enabled"));
return true;
}
return false;
}







size_t
spell_move_to (
win_T *wp,
int dir, 
bool allwords, 
bool curline,
hlf_T *attrp 

)
{
linenr_T lnum;
pos_T found_pos;
size_t found_len = 0;
char_u *line;
char_u *p;
char_u *endp;
hlf_T attr = HLF_COUNT;
size_t len;
int has_syntax = syntax_present(wp);
int col;
bool can_spell;
char_u *buf = NULL;
size_t buflen = 0;
int skip = 0;
int capcol = -1;
bool found_one = false;
bool wrapped = false;

if (no_spell_checking(wp))
return 0;










lnum = wp->w_cursor.lnum;
clearpos(&found_pos);

while (!got_int) {
line = ml_get_buf(wp->w_buffer, lnum, FALSE);

len = STRLEN(line);
if (buflen < len + MAXWLEN + 2) {
xfree(buf);
buflen = len + MAXWLEN + 2;
buf = xmalloc(buflen);
}
assert(buf && buflen >= len + MAXWLEN + 2);


if (lnum == 1)
capcol = 0;


if (capcol == 0) {
capcol = (int)getwhitecols(line);
} else if (curline && wp == curwin) {

col = (int)getwhitecols(line);
if (check_need_cap(lnum, col)) {
capcol = col;
}



line = ml_get_buf(wp->w_buffer, lnum, FALSE);
}



STRCPY(buf, line);
if (lnum < wp->w_buffer->b_ml.ml_line_count)
spell_cat_line(buf + STRLEN(buf),
ml_get_buf(wp->w_buffer, lnum + 1, FALSE),
MAXWLEN);
p = buf + skip;
endp = buf + len;
while (p < endp) {


if (dir == BACKWARD
&& lnum == wp->w_cursor.lnum
&& !wrapped
&& (colnr_T)(p - buf) >= wp->w_cursor.col)
break;


attr = HLF_COUNT;
len = spell_check(wp, p, &attr, &capcol, false);

if (attr != HLF_COUNT) {

if (allwords || attr == HLF_SPB) {


if (dir == BACKWARD
|| lnum != wp->w_cursor.lnum
|| wrapped
|| ((colnr_T)(curline
? p - buf + (ptrdiff_t)len
: p - buf) > wp->w_cursor.col)) {
if (has_syntax) {
col = (int)(p - buf);
(void)syn_get_id(wp, lnum, (colnr_T)col,
FALSE, &can_spell, FALSE);
if (!can_spell)
attr = HLF_COUNT;
} else
can_spell = true;

if (can_spell) {
found_one = true;
found_pos.lnum = lnum;
found_pos.col = (int)(p - buf);
found_pos.coladd = 0;
if (dir == FORWARD) {

wp->w_cursor = found_pos;
xfree(buf);
if (attrp != NULL)
*attrp = attr;
return len;
} else if (curline) {


assert(len <= INT_MAX);
found_pos.col += (int)len;
}
found_len = len;
}
} else
found_one = true;
}
}


p += len;
assert(len <= INT_MAX);
capcol -= (int)len;
}

if (dir == BACKWARD && found_pos.lnum != 0) {

wp->w_cursor = found_pos;
xfree(buf);
return found_len;
}

if (curline) {
break; 
}



if (lnum == wp->w_cursor.lnum && wrapped) {
break;
}


if (dir == BACKWARD) {
if (lnum > 1) {
lnum--;
} else if (!p_ws) {
break; 
} else {


lnum = wp->w_buffer->b_ml.ml_line_count;
wrapped = true;
if (!shortmess(SHM_SEARCH))
give_warning((char_u *)_(top_bot_msg), true);
}
capcol = -1;
} else {
if (lnum < wp->w_buffer->b_ml.ml_line_count)
++lnum;
else if (!p_ws)
break; 
else {


lnum = 1;
wrapped = true;
if (!shortmess(SHM_SEARCH))
give_warning((char_u *)_(bot_top_msg), true);
}



if (lnum == wp->w_cursor.lnum && !found_one) {
break;
}



if (attr == HLF_COUNT)
skip = (int)(p - endp);
else
skip = 0;


--capcol;


if (*skipwhite(line) == NUL)
capcol = 0;
}

line_breakcheck();
}

xfree(buf);
return 0;
}





void spell_cat_line(char_u *buf, char_u *line, int maxlen)
{
char_u *p;
int n;

p = skipwhite(line);
while (vim_strchr((char_u *)"*#/\"\t", *p) != NULL)
p = skipwhite(p + 1);

if (*p != NUL) {


n = (int)(p - line) + 1;
if (n < maxlen - 1) {
memset(buf, ' ', n);
STRLCPY(buf + n, p, maxlen - n);
}
}
}



static void spell_load_lang(char_u *lang)
{
char_u fname_enc[85];
int r;
spelload_T sl;
int round;



STRCPY(sl.sl_lang, lang);
sl.sl_slang = NULL;
sl.sl_nobreak = false;



for (round = 1; round <= 2; ++round) {

vim_snprintf((char *)fname_enc, sizeof(fname_enc) - 5,
"spell/%s.%s.spl", lang, spell_enc());
r = do_in_runtimepath(fname_enc, 0, spell_load_cb, &sl);

if (r == FAIL && *sl.sl_lang != NUL) {

vim_snprintf((char *)fname_enc, sizeof(fname_enc) - 5,
"spell/%s.ascii.spl", lang);
r = do_in_runtimepath(fname_enc, 0, spell_load_cb, &sl);

if (r == FAIL && *sl.sl_lang != NUL && round == 1
&& apply_autocmds(EVENT_SPELLFILEMISSING, lang,
curbuf->b_fname, FALSE, curbuf))
continue;
break;
}
break;
}

if (r == FAIL) {
if (starting) {


char autocmd_buf[512] = { 0 };
snprintf(autocmd_buf, sizeof(autocmd_buf),
"autocmd VimEnter * call spellfile#LoadFile('%s')|set spell",
lang);
do_cmdline_cmd(autocmd_buf);
} else {
smsg(
_("Warning: Cannot find word list \"%s.%s.spl\" or \"%s.ascii.spl\""),
lang, spell_enc(), lang);
}
} else if (sl.sl_slang != NULL) {

STRCPY(fname_enc + STRLEN(fname_enc) - 3, "add.spl");
do_in_runtimepath(fname_enc, DIP_ALL, spell_load_cb, &sl);
}
}



char_u *spell_enc(void)
{

if (STRLEN(p_enc) < 60 && STRCMP(p_enc, "iso-8859-15") != 0)
return p_enc;
return (char_u *)"latin1";
}



static void int_wordlist_spl(char_u *fname)
{
vim_snprintf((char *)fname, MAXPATHL, SPL_FNAME_TMPL,
int_wordlist, spell_enc());
}



slang_T *slang_alloc(char_u *lang)
{
slang_T *lp = xcalloc(1, sizeof(slang_T));

if (lang != NULL)
lp->sl_name = vim_strsave(lang);
ga_init(&lp->sl_rep, sizeof(fromto_T), 10);
ga_init(&lp->sl_repsal, sizeof(fromto_T), 10);
lp->sl_compmax = MAXWLEN;
lp->sl_compsylmax = MAXWLEN;
hash_init(&lp->sl_wordcount);

return lp;
}


void slang_free(slang_T *lp)
{
xfree(lp->sl_name);
xfree(lp->sl_fname);
slang_clear(lp);
xfree(lp);
}


static void free_salitem(salitem_T *smp) {
xfree(smp->sm_lead);

xfree(smp->sm_to);
xfree(smp->sm_lead_w);
xfree(smp->sm_oneof_w);
xfree(smp->sm_to_w);
}


static void free_fromto(fromto_T *ftp) {
xfree(ftp->ft_from);
xfree(ftp->ft_to);
}


void slang_clear(slang_T *lp)
{
garray_T *gap;

XFREE_CLEAR(lp->sl_fbyts);
XFREE_CLEAR(lp->sl_kbyts);
XFREE_CLEAR(lp->sl_pbyts);

XFREE_CLEAR(lp->sl_fidxs);
XFREE_CLEAR(lp->sl_kidxs);
XFREE_CLEAR(lp->sl_pidxs);

GA_DEEP_CLEAR(&lp->sl_rep, fromto_T, free_fromto);
GA_DEEP_CLEAR(&lp->sl_repsal, fromto_T, free_fromto);

gap = &lp->sl_sal;
if (lp->sl_sofo) {

GA_DEEP_CLEAR_PTR(gap);
} else {

GA_DEEP_CLEAR(gap, salitem_T, free_salitem);
}

for (int i = 0; i < lp->sl_prefixcnt; ++i) {
vim_regfree(lp->sl_prefprog[i]);
}
lp->sl_prefixcnt = 0;
XFREE_CLEAR(lp->sl_prefprog);
XFREE_CLEAR(lp->sl_info);
XFREE_CLEAR(lp->sl_midword);

vim_regfree(lp->sl_compprog);
lp->sl_compprog = NULL;
XFREE_CLEAR(lp->sl_comprules);
XFREE_CLEAR(lp->sl_compstartflags);
XFREE_CLEAR(lp->sl_compallflags);

XFREE_CLEAR(lp->sl_syllable);
ga_clear(&lp->sl_syl_items);

ga_clear_strings(&lp->sl_comppat);

hash_clear_all(&lp->sl_wordcount, WC_KEY_OFF);
hash_init(&lp->sl_wordcount);

hash_clear_all(&lp->sl_map_hash, 0);


slang_clear_sug(lp);

lp->sl_compmax = MAXWLEN;
lp->sl_compminlen = 0;
lp->sl_compsylmax = MAXWLEN;
lp->sl_regions[0] = NUL;
}


void slang_clear_sug(slang_T *lp)
{
XFREE_CLEAR(lp->sl_sbyts);
XFREE_CLEAR(lp->sl_sidxs);
close_spellbuf(lp->sl_sugbuf);
lp->sl_sugbuf = NULL;
lp->sl_sugloaded = false;
lp->sl_sugtime = 0;
}



static void spell_load_cb(char_u *fname, void *cookie)
{
spelload_T *slp = (spelload_T *)cookie;
slang_T *slang;

slang = spell_load_file(fname, slp->sl_lang, NULL, false);
if (slang != NULL) {


if (slp->sl_nobreak && slang->sl_add)
slang->sl_nobreak = true;
else if (slang->sl_nobreak)
slp->sl_nobreak = true;

slp->sl_slang = slang;
}
}








void count_common_word(slang_T *lp, char_u *word, int len, int count)
{
hash_T hash;
hashitem_T *hi;
wordcount_T *wc;
char_u buf[MAXWLEN];
char_u *p;

if (len == -1) {
p = word;
} else if (len >= MAXWLEN) {
return;
} else {
STRLCPY(buf, word, len + 1);
p = buf;
}

hash = hash_hash(p);
const size_t p_len = STRLEN(p);
hi = hash_lookup(&lp->sl_wordcount, (const char *)p, p_len, hash);
if (HASHITEM_EMPTY(hi)) {
wc = xmalloc(sizeof(wordcount_T) + p_len);
memcpy(wc->wc_word, p, p_len + 1);
wc->wc_count = count;
hash_add_item(&lp->sl_wordcount, hi, wc->wc_word, hash);
} else {
wc = HI2WC(hi);
if ((wc->wc_count += count) < (unsigned)count) 
wc->wc_count = MAXWORDCOUNT;
}
}


static int
score_wordcount_adj (
slang_T *slang,
int score,
char_u *word,
bool split 
)
{
hashitem_T *hi;
wordcount_T *wc;
int bonus;
int newscore;

hi = hash_find(&slang->sl_wordcount, word);
if (!HASHITEM_EMPTY(hi)) {
wc = HI2WC(hi);
if (wc->wc_count < SCORE_THRES2)
bonus = SCORE_COMMON1;
else if (wc->wc_count < SCORE_THRES3)
bonus = SCORE_COMMON2;
else
bonus = SCORE_COMMON3;
if (split)
newscore = score - bonus / 2;
else
newscore = score - bonus;
if (newscore < 0)
return 0;
return newscore;
}
return score;
}



bool byte_in_str(char_u *str, int n)
{
char_u *p;

for (p = str; *p != NUL; ++p)
if (*p == n)
return true;
return false;
}



int init_syl_tab(slang_T *slang)
{
char_u *p;
char_u *s;
int l;

ga_init(&slang->sl_syl_items, sizeof(syl_item_T), 4);
p = vim_strchr(slang->sl_syllable, '/');
while (p != NULL) {
*p++ = NUL;
if (*p == NUL) 
break;
s = p;
p = vim_strchr(p, '/');
if (p == NULL)
l = (int)STRLEN(s);
else
l = (int)(p - s);
if (l >= SY_MAXLEN)
return SP_FORMERROR;

syl_item_T *syl = GA_APPEND_VIA_PTR(syl_item_T, &slang->sl_syl_items);
STRLCPY(syl->sy_chars, s, l + 1);
syl->sy_len = l;
}
return OK;
}




static int count_syllables(slang_T *slang, const char_u *word)
FUNC_ATTR_NONNULL_ALL
{
int cnt = 0;
bool skip = false;
int len;
syl_item_T *syl;
int c;

if (slang->sl_syllable == NULL)
return 0;

for (const char_u *p = word; *p != NUL; p += len) {

if (*p == ' ') {
len = 1;
cnt = 0;
continue;
}


len = 0;
for (int i = 0; i < slang->sl_syl_items.ga_len; ++i) {
syl = ((syl_item_T *)slang->sl_syl_items.ga_data) + i;
if (syl->sy_len > len
&& STRNCMP(p, syl->sy_chars, syl->sy_len) == 0)
len = syl->sy_len;
}
if (len != 0) { 
++cnt;
skip = false;
} else {

c = utf_ptr2char(p);
len = (*mb_ptr2len)(p);
if (vim_strchr(slang->sl_syllable, c) == NULL)
skip = false; 
else if (!skip) {
++cnt; 
skip = true; 
}
}
}
return cnt;
}



char_u *did_set_spelllang(win_T *wp)
{
garray_T ga;
char_u *splp;
char_u *region;
char_u region_cp[3];
bool filename;
int region_mask;
slang_T *slang;
int c;
char_u lang[MAXWLEN + 1];
char_u spf_name[MAXPATHL];
int len;
char_u *p;
int round;
char_u *spf;
char_u *use_region = NULL;
bool dont_use_region = false;
bool nobreak = false;
langp_T *lp, *lp2;
static bool recursive = false;
char_u *ret_msg = NULL;
char_u *spl_copy;

bufref_T bufref;
set_bufref(&bufref, wp->w_buffer);




if (recursive)
return NULL;
recursive = true;

ga_init(&ga, sizeof(langp_T), 2);
clear_midword(wp);



spl_copy = vim_strsave(wp->w_s->b_p_spl);

wp->w_s->b_cjk = 0;


for (splp = spl_copy; *splp != NUL; ) {

copy_option_part(&splp, lang, MAXWLEN, ",");
region = NULL;
len = (int)STRLEN(lang);

if (!valid_spellang(lang)) {
continue;
}

if (STRCMP(lang, "cjk") == 0) {
wp->w_s->b_cjk = 1;
continue;
}




if (len > 4 && fnamecmp(lang + len - 4, ".spl") == 0) {
filename = true;


p = vim_strchr(path_tail(lang), '_');
if (p != NULL && ASCII_ISALPHA(p[1]) && ASCII_ISALPHA(p[2])
&& !ASCII_ISALPHA(p[3])) {
STRLCPY(region_cp, p + 1, 3);
memmove(p, p + 3, len - (p - lang) - 2);
region = region_cp;
} else
dont_use_region = true;


for (slang = first_lang; slang != NULL; slang = slang->sl_next) {
if (path_full_compare(lang, slang->sl_fname, false, true)
== kEqualFiles) {
break;
}
}
} else {
filename = false;
if (len > 3 && lang[len - 3] == '_') {
region = lang + len - 2;
lang[len - 3] = NUL;
} else
dont_use_region = true;


for (slang = first_lang; slang != NULL; slang = slang->sl_next)
if (STRICMP(lang, slang->sl_name) == 0)
break;
}

if (region != NULL) {


if (use_region != NULL && STRCMP(region, use_region) != 0)
dont_use_region = true;
use_region = region;
}


if (slang == NULL) {
if (filename)
(void)spell_load_file(lang, lang, NULL, false);
else {
spell_load_lang(lang);


if (!bufref_valid(&bufref)) {
ret_msg =
(char_u *)N_("E797: SpellFileMissing autocommand deleted buffer");
goto theend;
}
}
}


for (slang = first_lang; slang != NULL; slang = slang->sl_next) {
if (filename
? path_full_compare(lang, slang->sl_fname, false, true) == kEqualFiles
: STRICMP(lang, slang->sl_name) == 0) {
region_mask = REGION_ALL;
if (!filename && region != NULL) {

c = find_region(slang->sl_regions, region);
if (c == REGION_ALL) {
if (slang->sl_add) {
if (*slang->sl_regions != NUL)

region_mask = 0;
} else


smsg(_("Warning: region %s not supported"),
region);
} else
region_mask = 1 << c;
}

if (region_mask != 0) {
langp_T *p_ = GA_APPEND_VIA_PTR(langp_T, &ga);
p_->lp_slang = slang;
p_->lp_region = region_mask;

use_midword(slang, wp);
if (slang->sl_nobreak)
nobreak = true;
}
}
}
}





spf = curwin->w_s->b_p_spf;
for (round = 0; round == 0 || *spf != NUL; ++round) {
if (round == 0) {

if (int_wordlist == NULL)
continue;
int_wordlist_spl(spf_name);
} else {

copy_option_part(&spf, spf_name, MAXPATHL - 5, ",");
STRCAT(spf_name, ".spl");


for (c = 0; c < ga.ga_len; ++c) {
p = LANGP_ENTRY(ga, c)->lp_slang->sl_fname;
if (p != NULL
&& path_full_compare(spf_name, p, false, true) == kEqualFiles) {
break;
}
}
if (c < ga.ga_len)
continue;
}


for (slang = first_lang; slang != NULL; slang = slang->sl_next) {
if (path_full_compare(spf_name, slang->sl_fname, false, true)
== kEqualFiles) {
break;
}
}
if (slang == NULL) {



if (round == 0)
STRCPY(lang, "internal wordlist");
else {
STRLCPY(lang, path_tail(spf_name), MAXWLEN + 1);
p = vim_strchr(lang, '.');
if (p != NULL)
*p = NUL; 
}
slang = spell_load_file(spf_name, lang, NULL, true);



if (slang != NULL && nobreak)
slang->sl_nobreak = true;
}
if (slang != NULL) {
region_mask = REGION_ALL;
if (use_region != NULL && !dont_use_region) {

c = find_region(slang->sl_regions, use_region);
if (c != REGION_ALL)
region_mask = 1 << c;
else if (*slang->sl_regions != NUL)

region_mask = 0;
}

if (region_mask != 0) {
langp_T *p_ = GA_APPEND_VIA_PTR(langp_T, &ga);
p_->lp_slang = slang;
p_->lp_sallang = NULL;
p_->lp_replang = NULL;
p_->lp_region = region_mask;

use_midword(slang, wp);
}
}
}


ga_clear(&wp->w_s->b_langp);
wp->w_s->b_langp = ga;




for (int i = 0; i < ga.ga_len; ++i) {
lp = LANGP_ENTRY(ga, i);


if (!GA_EMPTY(&lp->lp_slang->sl_sal))

lp->lp_sallang = lp->lp_slang;
else

for (int j = 0; j < ga.ga_len; ++j) {
lp2 = LANGP_ENTRY(ga, j);
if (!GA_EMPTY(&lp2->lp_slang->sl_sal)
&& STRNCMP(lp->lp_slang->sl_name,
lp2->lp_slang->sl_name, 2) == 0) {
lp->lp_sallang = lp2->lp_slang;
break;
}
}


if (!GA_EMPTY(&lp->lp_slang->sl_rep))

lp->lp_replang = lp->lp_slang;
else

for (int j = 0; j < ga.ga_len; ++j) {
lp2 = LANGP_ENTRY(ga, j);
if (!GA_EMPTY(&lp2->lp_slang->sl_rep)
&& STRNCMP(lp->lp_slang->sl_name,
lp2->lp_slang->sl_name, 2) == 0) {
lp->lp_replang = lp2->lp_slang;
break;
}
}
}

theend:
xfree(spl_copy);
recursive = false;
redraw_win_later(wp, NOT_VALID);
return ret_msg;
}


static void clear_midword(win_T *wp)
{
memset(wp->w_s->b_spell_ismw, 0, 256);
XFREE_CLEAR(wp->w_s->b_spell_ismw_mb);
}



static void use_midword(slang_T *lp, win_T *wp)
{
char_u *p;

if (lp->sl_midword == NULL) 
return;

for (p = lp->sl_midword; *p != NUL; )
if (has_mbyte) {
int c, l, n;
char_u *bp;

c = utf_ptr2char(p);
l = (*mb_ptr2len)(p);
if (c < 256 && l <= 2)
wp->w_s->b_spell_ismw[c] = true;
else if (wp->w_s->b_spell_ismw_mb == NULL)

wp->w_s->b_spell_ismw_mb = vim_strnsave(p, l);
else {

n = (int)STRLEN(wp->w_s->b_spell_ismw_mb);
bp = vim_strnsave(wp->w_s->b_spell_ismw_mb, n + l);
xfree(wp->w_s->b_spell_ismw_mb);
wp->w_s->b_spell_ismw_mb = bp;
STRLCPY(bp + n, p, l + 1);
}
p += l;
} else
wp->w_s->b_spell_ismw[*p++] = true;
}




static int find_region(char_u *rp, char_u *region)
{
int i;

for (i = 0;; i += 2) {
if (rp[i] == NUL)
return REGION_ALL;
if (rp[i] == region[0] && rp[i + 1] == region[1])
break;
}
return i / 2;
}











int captype(char_u *word, char_u *end)
FUNC_ATTR_NONNULL_ARG(1)
{
char_u *p;
int c;
int firstcap;
bool allcap;
bool past_second = false; 


for (p = word; !spell_iswordp_nmw(p, curwin); MB_PTR_ADV(p)) {
if (end == NULL ? *p == NUL : p >= end) {
return 0; 
}
}
if (has_mbyte) {
c = mb_ptr2char_adv((const char_u **)&p);
} else {
c = *p++;
}
firstcap = allcap = SPELL_ISUPPER(c);



for (; end == NULL ? *p != NUL : p < end; MB_PTR_ADV(p)) {
if (spell_iswordp_nmw(p, curwin)) {
c = PTR2CHAR(p);
if (!SPELL_ISUPPER(c)) {

if (past_second && allcap) {
return WF_KEEPCAP;
}
allcap = false;
} else if (!allcap) {

return WF_KEEPCAP;
}
past_second = true;
}
}

if (allcap)
return WF_ALLCAP;
if (firstcap)
return WF_ONECAP;
return 0;
}




static int badword_captype(char_u *word, char_u *end)
FUNC_ATTR_NONNULL_ALL
{
int flags = captype(word, end);
int c;
int l, u;
bool first;
char_u *p;

if (flags & WF_KEEPCAP) {

l = u = 0;
first = false;
for (p = word; p < end; MB_PTR_ADV(p)) {
c = PTR2CHAR(p);
if (SPELL_ISUPPER(c)) {
++u;
if (p == word)
first = true;
} else
++l;
}





if (u > l && u > 2)
flags |= WF_ALLCAP;
else if (first)
flags |= WF_ONECAP;

if (u >= 2 && l >= 2) 
flags |= WF_MIXCAP;
}
return flags;
}


void spell_delete_wordlist(void)
{
char_u fname[MAXPATHL] = {0};

if (int_wordlist != NULL) {
os_remove((char *)int_wordlist);
int_wordlist_spl(fname);
os_remove((char *)fname);
XFREE_CLEAR(int_wordlist);
}
}


void spell_free_all(void)
{
slang_T *slang;


FOR_ALL_BUFFERS(buf) {
ga_clear(&buf->b_s.b_langp);
}

while (first_lang != NULL) {
slang = first_lang;
first_lang = slang->sl_next;
slang_free(slang);
}

spell_delete_wordlist();

XFREE_CLEAR(repl_to);
XFREE_CLEAR(repl_from);
}



void spell_reload(void)
{

init_spell_chartab();


spell_free_all();


FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {


if (*wp->w_s->b_p_spl != NUL) {
if (wp->w_p_spell) {
(void)did_set_spelllang(wp);
break;
}
}
}
}





static int bytes2offset(char_u **pp)
{
char_u *p = *pp;
int nr;
int c;

c = *p++;
if ((c & 0x80) == 0x00) { 
nr = c - 1;
} else if ((c & 0xc0) == 0x80) { 
nr = (c & 0x3f) - 1;
nr = nr * 255 + (*p++ - 1);
} else if ((c & 0xe0) == 0xc0) { 
nr = (c & 0x1f) - 1;
nr = nr * 255 + (*p++ - 1);
nr = nr * 255 + (*p++ - 1);
} else { 
nr = (c & 0x0f) - 1;
nr = nr * 255 + (*p++ - 1);
nr = nr * 255 + (*p++ - 1);
nr = nr * 255 + (*p++ - 1);
}

*pp = p;
return nr;
}






buf_T *open_spellbuf(void)
{
buf_T *buf = xcalloc(1, sizeof(buf_T));

buf->b_spell = true;
buf->b_p_swf = true; 
if (ml_open(buf) == FAIL) {
ELOG("Error opening a new memline");
}
ml_open_file(buf); 

return buf;
}


void close_spellbuf(buf_T *buf)
{
if (buf != NULL) {
ml_close(buf, TRUE);
xfree(buf);
}
}


void clear_spell_chartab(spelltab_T *sp)
{
int i;


memset(sp->st_isw, false, sizeof(sp->st_isw));
memset(sp->st_isu, false, sizeof(sp->st_isu));

for (i = 0; i < 256; ++i) {
sp->st_fold[i] = i;
sp->st_upper[i] = i;
}



for (i = '0'; i <= '9'; ++i)
sp->st_isw[i] = true;
for (i = 'A'; i <= 'Z'; ++i) {
sp->st_isw[i] = true;
sp->st_isu[i] = true;
sp->st_fold[i] = i + 0x20;
}
for (i = 'a'; i <= 'z'; ++i) {
sp->st_isw[i] = true;
sp->st_upper[i] = i - 0x20;
}
}





void init_spell_chartab(void)
{
int i;

did_set_spelltab = false;
clear_spell_chartab(&spelltab);
for (i = 128; i < 256; i++) {
int f = utf_fold(i);
int u = mb_toupper(i);

spelltab.st_isu[i] = mb_isupper(i);
spelltab.st_isw[i] = spelltab.st_isu[i] || mb_islower(i);



spelltab.st_fold[i] = (f < 256) ? f : i;
spelltab.st_upper[i] = (u < 256) ? u : i;
}
}







static bool spell_iswordp(const char_u *p, const win_T *wp)
FUNC_ATTR_NONNULL_ALL
{
int c;

const int l = utfc_ptr2len(p);
const char_u *s = p;
if (l == 1) {

if (wp->w_s->b_spell_ismw[*p]) {
s = p + 1; 
}
} else {
c = utf_ptr2char(p);
if (c < 256
? wp->w_s->b_spell_ismw[c]
: (wp->w_s->b_spell_ismw_mb != NULL
&& vim_strchr(wp->w_s->b_spell_ismw_mb, c) != NULL)) {
s = p + l;
}
}

c = utf_ptr2char(s);
if (c > 255) {
return spell_mb_isword_class(mb_get_class(s), wp);
}
return spelltab.st_isw[c];
}



bool spell_iswordp_nmw(const char_u *p, win_T *wp)
{
int c = utf_ptr2char(p);
if (c > 255) {
return spell_mb_isword_class(mb_get_class(p), wp);
}
return spelltab.st_isw[c];
}





static bool spell_mb_isword_class(int cl, const win_T *wp)
FUNC_ATTR_PURE FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
if (wp->w_s->b_cjk)

return cl == 2 || cl == 0x2800;
return cl >= 2 && cl != 0x2070 && cl != 0x2080 && cl != 3;
}



static bool spell_iswordp_w(const int *p, const win_T *wp)
FUNC_ATTR_NONNULL_ALL
{
const int *s;

if (*p < 256 ? wp->w_s->b_spell_ismw[*p]
: (wp->w_s->b_spell_ismw_mb != NULL
&& vim_strchr(wp->w_s->b_spell_ismw_mb, *p) != NULL))
s = p + 1;
else
s = p;

if (*s > 255) {
return spell_mb_isword_class(utf_class(*s), wp);
}
return spelltab.st_isw[*s];
}





int spell_casefold(char_u *str, int len, char_u *buf, int buflen)
{
int i;

if (len >= buflen) {
buf[0] = NUL;
return FAIL; 
}

if (has_mbyte) {
int outi = 0;
char_u *p;
int c;


for (p = str; p < str + len; ) {
if (outi + MB_MAXBYTES > buflen) {
buf[outi] = NUL;
return FAIL;
}
c = mb_cptr2char_adv((const char_u **)&p);
outi += utf_char2bytes(SPELL_TOFOLD(c), buf + outi);
}
buf[outi] = NUL;
} else {

for (i = 0; i < len; ++i)
buf[i] = spelltab.st_fold[str[i]];
buf[i] = NUL;
}

return OK;
}


#define SPS_BEST 1
#define SPS_FAST 2
#define SPS_DOUBLE 4

static int sps_flags = SPS_BEST; 
static int sps_limit = 9999; 



int spell_check_sps(void)
{
char_u *p;
char_u *s;
char_u buf[MAXPATHL];
int f;

sps_flags = 0;
sps_limit = 9999;

for (p = p_sps; *p != NUL; ) {
copy_option_part(&p, buf, MAXPATHL, ",");

f = 0;
if (ascii_isdigit(*buf)) {
s = buf;
sps_limit = getdigits_int(&s, true, 0);
if (*s != NUL && !ascii_isdigit(*s)) {
f = -1;
}
} else if (STRCMP(buf, "best") == 0) {
f = SPS_BEST;
} else if (STRCMP(buf, "fast") == 0) {
f = SPS_FAST;
} else if (STRCMP(buf, "double") == 0) {
f = SPS_DOUBLE;
} else if (STRNCMP(buf, "expr:", 5) != 0
&& STRNCMP(buf, "file:", 5) != 0) {
f = -1;
}

if (f == -1 || (sps_flags != 0 && f != 0)) {
sps_flags = SPS_BEST;
sps_limit = 9999;
return FAIL;
}
if (f != 0)
sps_flags = f;
}

if (sps_flags == 0)
sps_flags = SPS_BEST;

return OK;
}





void spell_suggest(int count)
{
char_u *line;
pos_T prev_cursor = curwin->w_cursor;
char_u wcopy[MAXWLEN + 2];
char_u *p;
int c;
suginfo_T sug;
suggest_T *stp;
int mouse_used;
int need_cap;
int limit;
int selected = count;
int badlen = 0;
int msg_scroll_save = msg_scroll;

if (no_spell_checking(curwin))
return;

if (VIsual_active) {


if (curwin->w_cursor.lnum != VIsual.lnum) {
vim_beep(BO_SPELL);
return;
}
badlen = (int)curwin->w_cursor.col - (int)VIsual.col;
if (badlen < 0) {
badlen = -badlen;
} else {
curwin->w_cursor.col = VIsual.col;
}
badlen++;
end_visual_mode();
} else

if (spell_move_to(curwin, FORWARD, true, true, NULL) == 0
|| curwin->w_cursor.col > prev_cursor.col) {


curwin->w_cursor = prev_cursor;
line = get_cursor_line_ptr();
p = line + curwin->w_cursor.col;

while (p > line && spell_iswordp_nmw(p, curwin)) {
MB_PTR_BACK(line, p);
}

while (*p != NUL && !spell_iswordp_nmw(p, curwin)) {
MB_PTR_ADV(p);
}

if (!spell_iswordp_nmw(p, curwin)) { 
beep_flush();
return;
}
curwin->w_cursor.col = (colnr_T)(p - line);
}




need_cap = check_need_cap(curwin->w_cursor.lnum, curwin->w_cursor.col);


line = vim_strsave(get_cursor_line_ptr());



if (sps_limit > (int)Rows - 2)
limit = (int)Rows - 2;
else
limit = sps_limit;
spell_find_suggest(line + curwin->w_cursor.col, badlen, &sug, limit,
true, need_cap, true);

if (GA_EMPTY(&sug.su_ga))
MSG(_("Sorry, no suggestions"));
else if (count > 0) {
if (count > sug.su_ga.ga_len)
smsg(_("Sorry, only %" PRId64 " suggestions"),
(int64_t)sug.su_ga.ga_len);
} else {
XFREE_CLEAR(repl_from);
XFREE_CLEAR(repl_to);


cmdmsg_rl = curwin->w_p_rl;
if (cmdmsg_rl)
msg_col = Columns - 1;


msg_start();
msg_row = Rows - 1; 
lines_left = Rows; 
vim_snprintf((char *)IObuff, IOSIZE, _("Change \"%.*s\" to:"),
sug.su_badlen, sug.su_badptr);
if (cmdmsg_rl && STRNCMP(IObuff, "Change", 6) == 0) {


vim_snprintf((char *)IObuff, IOSIZE, ":ot \"%.*s\" egnahC",
sug.su_badlen, sug.su_badptr);
}
msg_puts((const char *)IObuff);
msg_clr_eos();
msg_putchar('\n');

msg_scroll = TRUE;
for (int i = 0; i < sug.su_ga.ga_len; ++i) {
stp = &SUG(sug.su_ga, i);



STRLCPY(wcopy, stp->st_word, MAXWLEN + 1);
if (sug.su_badlen > stp->st_orglen)
STRLCPY(wcopy + stp->st_wordlen,
sug.su_badptr + stp->st_orglen,
sug.su_badlen - stp->st_orglen + 1);
vim_snprintf((char *)IObuff, IOSIZE, "%2d", i + 1);
if (cmdmsg_rl) {
rl_mirror(IObuff);
}
msg_puts((const char *)IObuff);

vim_snprintf((char *)IObuff, IOSIZE, " \"%s\"", wcopy);
msg_puts((const char *)IObuff);


if (sug.su_badlen < stp->st_orglen) {
vim_snprintf((char *)IObuff, IOSIZE, _(" < \"%.*s\""),
stp->st_orglen, sug.su_badptr);
msg_puts((const char *)IObuff);
}

if (p_verbose > 0) {

if (sps_flags & (SPS_DOUBLE | SPS_BEST))
vim_snprintf((char *)IObuff, IOSIZE, " (%s%d - %d)",
stp->st_salscore ? "s " : "",
stp->st_score, stp->st_altscore);
else
vim_snprintf((char *)IObuff, IOSIZE, " (%d)",
stp->st_score);
if (cmdmsg_rl)

rl_mirror(IObuff + 1);
msg_advance(30);
msg_puts((const char *)IObuff);
}
msg_putchar('\n');
}

cmdmsg_rl = FALSE;
msg_col = 0;

selected = prompt_for_number(&mouse_used);
if (mouse_used)
selected -= lines_left;
lines_left = Rows; 

msg_scroll = msg_scroll_save;
}

if (selected > 0 && selected <= sug.su_ga.ga_len && u_save_cursor() == OK) {

stp = &SUG(sug.su_ga, selected - 1);
if (sug.su_badlen > stp->st_orglen) {


repl_from = vim_strnsave(sug.su_badptr, sug.su_badlen);
vim_snprintf((char *)IObuff, IOSIZE, "%s%.*s", stp->st_word,
sug.su_badlen - stp->st_orglen,
sug.su_badptr + stp->st_orglen);
repl_to = vim_strsave(IObuff);
} else {

repl_from = vim_strnsave(sug.su_badptr, stp->st_orglen);
repl_to = vim_strsave(stp->st_word);
}


p = xmalloc(STRLEN(line) - stp->st_orglen + stp->st_wordlen + 1);
c = (int)(sug.su_badptr - line);
memmove(p, line, c);
STRCPY(p + c, stp->st_word);
STRCAT(p, sug.su_badptr + stp->st_orglen);
ml_replace(curwin->w_cursor.lnum, p, false);
curwin->w_cursor.col = c;


ResetRedobuff();
AppendToRedobuff("ciw");
AppendToRedobuffLit(p + c,
stp->st_wordlen + sug.su_badlen - stp->st_orglen);
AppendCharToRedobuff(ESC);


changed_bytes(curwin->w_cursor.lnum, c);
} else
curwin->w_cursor = prev_cursor;

spell_find_cleanup(&sug);
xfree(line);
}



static bool check_need_cap(linenr_T lnum, colnr_T col)
{
bool need_cap = false;
char_u *line;
char_u *line_copy = NULL;
char_u *p;
colnr_T endcol;
regmatch_T regmatch;

if (curwin->w_s->b_cap_prog == NULL)
return false;

line = get_cursor_line_ptr();
endcol = 0;
if (getwhitecols(line) >= (int)col) {


if (lnum == 1)
need_cap = true;
else {
line = ml_get(lnum - 1);
if (*skipwhite(line) == NUL)
need_cap = true;
else {

line_copy = concat_str(line, (char_u *)" ");
line = line_copy;
endcol = (colnr_T)STRLEN(line);
}
}
} else {
endcol = col;
}

if (endcol > 0) {

regmatch.regprog = curwin->w_s->b_cap_prog;
regmatch.rm_ic = FALSE;
p = line + endcol;
for (;; ) {
MB_PTR_BACK(line, p);
if (p == line || spell_iswordp_nmw(p, curwin)) {
break;
}
if (vim_regexec(&regmatch, p, 0)
&& regmatch.endp[0] == line + endcol) {
need_cap = true;
break;
}
}
curwin->w_s->b_cap_prog = regmatch.regprog;
}

xfree(line_copy);

return need_cap;
}



void ex_spellrepall(exarg_T *eap)
{
pos_T pos = curwin->w_cursor;
char_u *frompat;
int addlen;
char_u *line;
char_u *p;
bool save_ws = p_ws;
linenr_T prev_lnum = 0;

if (repl_from == NULL || repl_to == NULL) {
EMSG(_("E752: No previous spell replacement"));
return;
}
addlen = (int)(STRLEN(repl_to) - STRLEN(repl_from));

frompat = xmalloc(STRLEN(repl_from) + 7);
sprintf((char *)frompat, "\\V\\<%s\\>", repl_from);
p_ws = false;

sub_nsubs = 0;
sub_nlines = 0;
curwin->w_cursor.lnum = 0;
while (!got_int) {
if (do_search(NULL, '/', frompat, 1L, SEARCH_KEEP, NULL) == 0
|| u_save_cursor() == FAIL) {
break;
}



line = get_cursor_line_ptr();
if (addlen <= 0 || STRNCMP(line + curwin->w_cursor.col,
repl_to, STRLEN(repl_to)) != 0) {
p = xmalloc(STRLEN(line) + addlen + 1);
memmove(p, line, curwin->w_cursor.col);
STRCPY(p + curwin->w_cursor.col, repl_to);
STRCAT(p, line + curwin->w_cursor.col + STRLEN(repl_from));
ml_replace(curwin->w_cursor.lnum, p, false);
changed_bytes(curwin->w_cursor.lnum, curwin->w_cursor.col);

if (curwin->w_cursor.lnum != prev_lnum) {
++sub_nlines;
prev_lnum = curwin->w_cursor.lnum;
}
++sub_nsubs;
}
curwin->w_cursor.col += (colnr_T)STRLEN(repl_to);
}

p_ws = save_ws;
curwin->w_cursor = pos;
xfree(frompat);

if (sub_nsubs == 0)
EMSG2(_("E753: Not found: %s"), repl_from);
else
do_sub_msg(false);
}



void
spell_suggest_list (
garray_T *gap,
char_u *word,
int maxcount, 
bool need_cap, 
bool interactive
)
{
suginfo_T sug;
suggest_T *stp;
char_u *wcopy;

spell_find_suggest(word, 0, &sug, maxcount, false, need_cap, interactive);


ga_init(gap, sizeof(char_u *), sug.su_ga.ga_len + 1);
ga_grow(gap, sug.su_ga.ga_len);
for (int i = 0; i < sug.su_ga.ga_len; ++i) {
stp = &SUG(sug.su_ga, i);



wcopy = xmalloc(stp->st_wordlen
+ STRLEN(sug.su_badptr + stp->st_orglen) + 1);
STRCPY(wcopy, stp->st_word);
STRCPY(wcopy + stp->st_wordlen, sug.su_badptr + stp->st_orglen);
((char_u **)gap->ga_data)[gap->ga_len++] = wcopy;
}

spell_find_cleanup(&sug);
}






static void
spell_find_suggest (
char_u *badptr,
int badlen, 
suginfo_T *su,
int maxcount,
bool banbadword, 
bool need_cap, 
bool interactive
)
{
hlf_T attr = HLF_COUNT;
char_u buf[MAXPATHL];
char_u *p;
bool do_combine = false;
char_u *sps_copy;
static bool expr_busy = false;
int c;
langp_T *lp;


memset(su, 0, sizeof(suginfo_T));
ga_init(&su->su_ga, (int)sizeof(suggest_T), 10);
ga_init(&su->su_sga, (int)sizeof(suggest_T), 10);
if (*badptr == NUL)
return;
hash_init(&su->su_banned);

su->su_badptr = badptr;
if (badlen != 0)
su->su_badlen = badlen;
else {
size_t tmplen = spell_check(curwin, su->su_badptr, &attr, NULL, false);
assert(tmplen <= INT_MAX);
su->su_badlen = (int)tmplen;
}
su->su_maxcount = maxcount;
su->su_maxscore = SCORE_MAXINIT;

if (su->su_badlen >= MAXWLEN)
su->su_badlen = MAXWLEN - 1; 
STRLCPY(su->su_badword, su->su_badptr, su->su_badlen + 1);
(void)spell_casefold(su->su_badptr, su->su_badlen, su->su_fbadword, MAXWLEN);




su->su_fbadword[su->su_badlen] = NUL;


su->su_badflags = badword_captype(su->su_badptr,
su->su_badptr + su->su_badlen);
if (need_cap)
su->su_badflags |= WF_ONECAP;





for (int i = 0; i < curbuf->b_s.b_langp.ga_len; ++i) {
lp = LANGP_ENTRY(curbuf->b_s.b_langp, i);
if (lp->lp_sallang != NULL) {
su->su_sallang = lp->lp_sallang;
break;
}
}



if (su->su_sallang != NULL)
spell_soundfold(su->su_sallang, su->su_fbadword, true,
su->su_sal_badword);




c = PTR2CHAR(su->su_badptr);
if (!SPELL_ISUPPER(c) && attr == HLF_COUNT) {
make_case_word(su->su_badword, buf, WF_ONECAP);
add_suggestion(su, &su->su_ga, buf, su->su_badlen, SCORE_ICASE,
0, true, su->su_sallang, false);
}


if (banbadword)
add_banned(su, su->su_badword);


sps_copy = vim_strsave(p_sps);


for (p = sps_copy; *p != NUL; ) {
copy_option_part(&p, buf, MAXPATHL, ",");

if (STRNCMP(buf, "expr:", 5) == 0) {


if (!expr_busy) {
expr_busy = true;
spell_suggest_expr(su, buf + 5);
expr_busy = false;
}
} else if (STRNCMP(buf, "file:", 5) == 0)

spell_suggest_file(su, buf + 5);
else {

spell_suggest_intern(su, interactive);
if (sps_flags & SPS_DOUBLE)
do_combine = true;
}
}

xfree(sps_copy);

if (do_combine)


score_combine(su);
}


static void spell_suggest_expr(suginfo_T *su, char_u *expr)
{
int score;
const char *p;




list_T *const list = eval_spell_expr(su->su_badword, expr);
if (list != NULL) {

TV_LIST_ITER(list, li, {
if (TV_LIST_ITEM_TV(li)->v_type == VAR_LIST) {

score = get_spellword(TV_LIST_ITEM_TV(li)->vval.v_list, &p);
if (score >= 0 && score <= su->su_maxscore) {
add_suggestion(su, &su->su_ga, (const char_u *)p, su->su_badlen,
score, 0, true, su->su_sallang, false);
}
}
});
tv_list_unref(list);
}


check_suggestions(su, &su->su_ga);
(void)cleanup_suggestions(&su->su_ga, su->su_maxscore, su->su_maxcount);
}


static void spell_suggest_file(suginfo_T *su, char_u *fname)
{
FILE *fd;
char_u line[MAXWLEN * 2];
char_u *p;
int len;
char_u cword[MAXWLEN];


fd = os_fopen((char *)fname, "r");
if (fd == NULL) {
EMSG2(_(e_notopen), fname);
return;
}


while (!vim_fgets(line, MAXWLEN * 2, fd) && !got_int) {
line_breakcheck();

p = vim_strchr(line, '/');
if (p == NULL)
continue; 
*p++ = NUL;
if (STRICMP(su->su_badword, line) == 0) {

for (len = 0; p[len] >= ' '; ++len)
;
p[len] = NUL;



if (captype(p, NULL) == 0) {
make_case_word(p, cword, su->su_badflags);
p = cword;
}

add_suggestion(su, &su->su_ga, p, su->su_badlen,
SCORE_FILE, 0, true, su->su_sallang, false);
}
}

fclose(fd);


check_suggestions(su, &su->su_ga);
(void)cleanup_suggestions(&su->su_ga, su->su_maxscore, su->su_maxcount);
}


static void spell_suggest_intern(suginfo_T *su, bool interactive)
{

suggest_load_files();





suggest_try_special(su);



suggest_try_change(su);


if (sps_flags & SPS_DOUBLE)
score_comp_sal(su);


if ((sps_flags & SPS_FAST) == 0) {
if (sps_flags & SPS_BEST)


rescore_suggestions(su);









suggest_try_soundalike_prep();
su->su_maxscore = SCORE_SFMAX1;
su->su_sfmaxscore = SCORE_MAXINIT * 3;
suggest_try_soundalike(su);
if (su->su_ga.ga_len < SUG_CLEAN_COUNT(su)) {


su->su_maxscore = SCORE_SFMAX2;
suggest_try_soundalike(su);
if (su->su_ga.ga_len < SUG_CLEAN_COUNT(su)) {


su->su_maxscore = SCORE_SFMAX3;
suggest_try_soundalike(su);
}
}
su->su_maxscore = su->su_sfmaxscore;
suggest_try_soundalike_finish();
}



os_breakcheck();
if (interactive && got_int) {
(void)vgetc();
got_int = FALSE;
}

if ((sps_flags & SPS_DOUBLE) == 0 && su->su_ga.ga_len != 0) {
if (sps_flags & SPS_BEST)

rescore_suggestions(su);


check_suggestions(su, &su->su_ga);
(void)cleanup_suggestions(&su->su_ga, su->su_maxscore, su->su_maxcount);
}
}


static void spell_find_cleanup(suginfo_T *su)
{
#define FREE_SUG_WORD(sug) xfree(sug->st_word)

GA_DEEP_CLEAR(&su->su_ga, suggest_T, FREE_SUG_WORD);
GA_DEEP_CLEAR(&su->su_sga, suggest_T, FREE_SUG_WORD);


hash_clear_all(&su->su_banned, 0);
}








void onecap_copy(char_u *word, char_u *wcopy, bool upper)
{
char_u *p;
int c;
int l;

p = word;
if (has_mbyte) {
c = mb_cptr2char_adv((const char_u **)&p);
} else {
c = *p++;
}
if (upper) {
c = SPELL_TOUPPER(c);
} else {
c = SPELL_TOFOLD(c);
}
l = utf_char2bytes(c, wcopy);
STRLCPY(wcopy + l, p, MAXWLEN - l);
}



static void allcap_copy(char_u *word, char_u *wcopy)
{
char_u *s;
char_u *d;
int c;

d = wcopy;
for (s = word; *s != NUL; ) {
if (has_mbyte) {
c = mb_cptr2char_adv((const char_u **)&s);
} else {
c = *s++;
}

if (c == 0xdf) {
c = 'S';
if (d - wcopy >= MAXWLEN - 1)
break;
*d++ = c;
} else
c = SPELL_TOUPPER(c);

if (d - wcopy >= MAXWLEN - MB_MAXBYTES) {
break;
}
d += utf_char2bytes(c, d);
}
*d = NUL;
}


static void suggest_try_special(suginfo_T *su)
{
char_u *p;
size_t len;
int c;
char_u word[MAXWLEN];


p = skiptowhite(su->su_fbadword);
len = p - su->su_fbadword;
p = skipwhite(p);
if (STRLEN(p) == len && STRNCMP(su->su_fbadword, p, len) == 0) {


c = su->su_fbadword[len];
su->su_fbadword[len] = NUL;
make_case_word(su->su_fbadword, word, su->su_badflags);
su->su_fbadword[len] = c;



add_suggestion(su, &su->su_ga, word, su->su_badlen,
RESCORE(SCORE_REP, 0), 0, true, su->su_sallang, false);
}
}




#if defined(SUGGEST_PROFILE)
proftime_T current;
proftime_T total;
proftime_T times[STATE_FINAL + 1];
long counts[STATE_FINAL + 1];

static void
prof_init(void)
{
for (int i = 0; i <= STATE_FINAL; i++) {
profile_zero(&times[i]);
counts[i] = 0;
}
profile_start(&current);
profile_start(&total);
}


static void
prof_store(state_T state)
{
profile_end(&current);
profile_add(&times[state], &current);
counts[state]++;
profile_start(&current);
}
#define PROF_STORE(state) prof_store(state);

static void
prof_report(char *name)
{
FILE *fd = fopen("suggestprof", "a");

profile_end(&total);
fprintf(fd, "-----------------------\n");
fprintf(fd, "%s: %s\n", name, profile_msg(&total));
for (int i = 0; i <= STATE_FINAL; i++) {
fprintf(fd, "%d: %s ("%" PRId64)\n", i, profile_msg(&times[i]), counts[i]);
}
fclose(fd);
}
#else
#define PROF_STORE(state)
#endif



static void suggest_try_change(suginfo_T *su)
{
char_u fword[MAXWLEN]; 
int n;
char_u *p;
langp_T *lp;




STRCPY(fword, su->su_fbadword);
n = (int)STRLEN(fword);
p = su->su_badptr + su->su_badlen;
(void)spell_casefold(p, (int)STRLEN(p), fword + n, MAXWLEN - n);

for (int lpi = 0; lpi < curwin->w_s->b_langp.ga_len; ++lpi) {
lp = LANGP_ENTRY(curwin->w_s->b_langp, lpi);



if (lp->lp_slang->sl_fbyts == NULL)
continue;



#if defined(SUGGEST_PROFILE)
prof_init();
#endif
suggest_trie_walk(su, lp, fword, false);
#if defined(SUGGEST_PROFILE)
prof_report("try_change");
#endif
}
}


#define TRY_DEEPER(su, stack, depth, add) (stack[depth].ts_score + (add) < su->su_maxscore)































static void suggest_trie_walk(suginfo_T *su, langp_T *lp, char_u *fword, bool soundfold)
{
char_u tword[MAXWLEN]; 
trystate_T stack[MAXWLEN];
char_u preword[MAXWLEN * 3] = { 0 }; 




char_u compflags[MAXWLEN]; 
trystate_T *sp;
int newscore;
int score;
char_u *byts, *fbyts, *pbyts;
idx_T *idxs, *fidxs, *pidxs;
int depth;
int c, c2, c3;
int n = 0;
int flags;
garray_T *gap;
idx_T arridx;
int len;
char_u *p;
fromto_T *ftp;
int fl = 0, tl;
int repextra = 0; 
slang_T *slang = lp->lp_slang;
int fword_ends;
bool goodword_ends;
#if defined(DEBUG_TRIEWALK)

char_u changename[MAXWLEN][80];
#endif
int breakcheckcount = 1000;
bool compound_ok;





depth = 0;
sp = &stack[0];
memset(sp, 0, sizeof(trystate_T)); 
sp->ts_curi = 1;

if (soundfold) {

byts = fbyts = slang->sl_sbyts;
idxs = fidxs = slang->sl_sidxs;
pbyts = NULL;
pidxs = NULL;
sp->ts_prefixdepth = PFD_NOPREFIX;
sp->ts_state = STATE_START;
} else {


fbyts = slang->sl_fbyts;
fidxs = slang->sl_fidxs;
pbyts = slang->sl_pbyts;
pidxs = slang->sl_pidxs;
if (pbyts != NULL) {
byts = pbyts;
idxs = pidxs;
sp->ts_prefixdepth = PFD_PREFIXTREE;
sp->ts_state = STATE_NOPREFIX; 
} else {
byts = fbyts;
idxs = fidxs;
sp->ts_prefixdepth = PFD_NOPREFIX;
sp->ts_state = STATE_START;
}
}






while (depth >= 0 && !got_int) {
sp = &stack[depth];
switch (sp->ts_state) {
case STATE_START:
case STATE_NOPREFIX:


arridx = sp->ts_arridx; 
len = byts[arridx]; 
arridx += sp->ts_curi; 

if (sp->ts_prefixdepth == PFD_PREFIXTREE) {

for (n = 0; n < len && byts[arridx + n] == 0; ++n)
;
sp->ts_curi += n;


n = (int)sp->ts_state;
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_ENDNUL;
sp->ts_save_badflags = su->su_badflags;



if (byts[arridx] == 0 || n == (int)STATE_NOPREFIX) {


if (has_mbyte)
n = nofold_len(fword, sp->ts_fidx, su->su_badptr);
else
n = sp->ts_fidx;
flags = badword_captype(su->su_badptr, su->su_badptr + n);
su->su_badflags = badword_captype(su->su_badptr + n,
su->su_badptr + su->su_badlen);
#if defined(DEBUG_TRIEWALK)
sprintf(changename[depth], "prefix");
#endif
go_deeper(stack, depth, 0);
++depth;
sp = &stack[depth];
sp->ts_prefixdepth = depth - 1;
byts = fbyts;
idxs = fidxs;
sp->ts_arridx = 0;



tword[sp->ts_twordlen] = NUL;
make_case_word(tword + sp->ts_splitoff,
preword + sp->ts_prewordlen, flags);
sp->ts_prewordlen = (char_u)STRLEN(preword);
sp->ts_splitoff = sp->ts_twordlen;
}
break;
}

if (sp->ts_curi > len || byts[arridx] != 0) {

PROF_STORE(sp->ts_state)
sp->ts_state = STATE_ENDNUL;
sp->ts_save_badflags = su->su_badflags;
break;
}


++sp->ts_curi; 

flags = (int)idxs[arridx];


if (flags & WF_NOSUGGEST)
break;

fword_ends = (fword[sp->ts_fidx] == NUL
|| (soundfold
? ascii_iswhite(fword[sp->ts_fidx])
: !spell_iswordp(fword + sp->ts_fidx, curwin)));
tword[sp->ts_twordlen] = NUL;

if (sp->ts_prefixdepth <= PFD_NOTSPECIAL
&& (sp->ts_flags & TSF_PREFIXOK) == 0) {




n = stack[sp->ts_prefixdepth].ts_arridx;
len = pbyts[n++];
for (c = 0; c < len && pbyts[n + c] == 0; ++c)
;
if (c > 0) {
c = valid_word_prefix(c, n, flags,
tword + sp->ts_splitoff, slang, false);
if (c == 0)
break;


if (c & WF_RAREPFX)
flags |= WF_RARE;





sp->ts_flags |= TSF_PREFIXOK;
}
}



if (sp->ts_complen == sp->ts_compsplit && fword_ends
&& (flags & WF_NEEDCOMP))
goodword_ends = false;
else
goodword_ends = true;

p = NULL;
compound_ok = true;
if (sp->ts_complen > sp->ts_compsplit) {
if (slang->sl_nobreak) {




if (sp->ts_fidx - sp->ts_splitfidx
== sp->ts_twordlen - sp->ts_splitoff
&& STRNCMP(fword + sp->ts_splitfidx,
tword + sp->ts_splitoff,
sp->ts_fidx - sp->ts_splitfidx) == 0) {
preword[sp->ts_prewordlen] = NUL;
newscore = score_wordcount_adj(slang, sp->ts_score,
preword + sp->ts_prewordlen,
sp->ts_prewordlen > 0);

if (newscore <= su->su_maxscore)
add_suggestion(su, &su->su_ga, preword,
sp->ts_splitfidx - repextra,
newscore, 0, false,
lp->lp_sallang, false);
break;
}
} else {




if (((unsigned)flags >> 24) == 0
|| sp->ts_twordlen - sp->ts_splitoff
< slang->sl_compminlen)
break;


if (has_mbyte
&& slang->sl_compminlen > 0
&& mb_charlen(tword + sp->ts_splitoff)
< slang->sl_compminlen)
break;

compflags[sp->ts_complen] = ((unsigned)flags >> 24);
compflags[sp->ts_complen + 1] = NUL;
STRLCPY(preword + sp->ts_prewordlen,
tword + sp->ts_splitoff,
sp->ts_twordlen - sp->ts_splitoff + 1);


if (match_checkcompoundpattern(preword, sp->ts_prewordlen,
&slang->sl_comppat))
compound_ok = false;

if (compound_ok) {
p = preword;
while (*skiptowhite(p) != NUL)
p = skipwhite(skiptowhite(p));
if (fword_ends && !can_compound(slang, p,
compflags + sp->ts_compsplit))


compound_ok = false;
}


p = preword + sp->ts_prewordlen;
MB_PTR_BACK(preword, p);
}
}




if (soundfold)
STRCPY(preword + sp->ts_prewordlen, tword + sp->ts_splitoff);
else if (flags & WF_KEEPCAP)

find_keepcap_word(slang, tword + sp->ts_splitoff,
preword + sp->ts_prewordlen);
else {



c = su->su_badflags;
if ((c & WF_ALLCAP)
&& su->su_badlen == (*mb_ptr2len)(su->su_badptr)
)
c = WF_ONECAP;
c |= flags;



if (p != NULL && spell_iswordp_nmw(p, curwin))
c &= ~WF_ONECAP;
make_case_word(tword + sp->ts_splitoff,
preword + sp->ts_prewordlen, c);
}

if (!soundfold) {


if (flags & WF_BANNED) {
add_banned(su, preword + sp->ts_prewordlen);
break;
}
if ((sp->ts_complen == sp->ts_compsplit
&& WAS_BANNED(su, preword + sp->ts_prewordlen))
|| WAS_BANNED(su, preword)) {
if (slang->sl_compprog == NULL)
break;

goodword_ends = false;
}
}

newscore = 0;
if (!soundfold) { 
if ((flags & WF_REGION)
&& (((unsigned)flags >> 16) & lp->lp_region) == 0)
newscore += SCORE_REGION;
if (flags & WF_RARE)
newscore += SCORE_RARE;

if (!spell_valid_case(su->su_badflags,
captype(preword + sp->ts_prewordlen, NULL)))
newscore += SCORE_ICASE;
}


if (fword_ends
&& goodword_ends
&& sp->ts_fidx >= sp->ts_fidxtry
&& compound_ok) {

#if defined(DEBUG_TRIEWALK)
if (soundfold && STRCMP(preword, "smwrd") == 0) {
int j;


smsg("------ %s -------", fword);
for (j = 0; j < depth; ++j)
smsg("%s", changename[j]);
}
#endif
if (soundfold) {


add_sound_suggest(su, preword, sp->ts_score, lp);
} else if (sp->ts_fidx > 0) {


p = fword + sp->ts_fidx;
MB_PTR_BACK(fword, p);
if (!spell_iswordp(p, curwin)) {
p = preword + STRLEN(preword);
MB_PTR_BACK(preword, p);
if (spell_iswordp(p, curwin)) {
newscore += SCORE_NONWORD;
}
}


score = score_wordcount_adj(slang,
sp->ts_score + newscore,
preword + sp->ts_prewordlen,
sp->ts_prewordlen > 0);


if (score <= su->su_maxscore) {
add_suggestion(su, &su->su_ga, preword,
sp->ts_fidx - repextra,
score, 0, false, lp->lp_sallang, false);

if (su->su_badflags & WF_MIXCAP) {


c = captype(preword, NULL);
if (c == 0 || c == WF_ALLCAP) {
make_case_word(tword + sp->ts_splitoff,
preword + sp->ts_prewordlen,
c == 0 ? WF_ALLCAP : 0);

add_suggestion(su, &su->su_ga, preword,
sp->ts_fidx - repextra,
score + SCORE_ICASE, 0, false,
lp->lp_sallang, false);
}
}
}
}
}


if ((sp->ts_fidx >= sp->ts_fidxtry || fword_ends)

&& (!has_mbyte || sp->ts_tcharlen == 0)
) {
bool try_compound;
int try_split;






try_split = (sp->ts_fidx - repextra < su->su_badlen)
&& !soundfold;













try_compound = false;
if (!soundfold
&& !slang->sl_nocompoundsugs
&& slang->sl_compprog != NULL
&& ((unsigned)flags >> 24) != 0
&& sp->ts_twordlen - sp->ts_splitoff
>= slang->sl_compminlen
&& (!has_mbyte
|| slang->sl_compminlen == 0
|| mb_charlen(tword + sp->ts_splitoff)
>= slang->sl_compminlen)
&& (slang->sl_compsylmax < MAXWLEN
|| sp->ts_complen + 1 - sp->ts_compsplit
< slang->sl_compmax)
&& (can_be_compound(sp, slang,
compflags, ((unsigned)flags >> 24)))) {
try_compound = true;
compflags[sp->ts_complen] = ((unsigned)flags >> 24);
compflags[sp->ts_complen + 1] = NUL;
}



if (slang->sl_nobreak && !slang->sl_nocompoundsugs) {
try_compound = true;
} else if (!fword_ends
&& try_compound
&& (sp->ts_flags & TSF_DIDSPLIT) == 0) {



try_compound = false;
sp->ts_flags |= TSF_DIDSPLIT;
--sp->ts_curi; 
compflags[sp->ts_complen] = NUL;
} else {
sp->ts_flags &= ~TSF_DIDSPLIT;
}

if (try_split || try_compound) {
if (!try_compound && (!fword_ends || !goodword_ends)) {




if (sp->ts_complen == sp->ts_compsplit
&& (flags & WF_NEEDCOMP))
break;
p = preword;
while (*skiptowhite(p) != NUL)
p = skipwhite(skiptowhite(p));
if (sp->ts_complen > sp->ts_compsplit
&& !can_compound(slang, p,
compflags + sp->ts_compsplit))
break;

if (slang->sl_nosplitsugs)
newscore += SCORE_SPLIT_NO;
else
newscore += SCORE_SPLIT;


newscore = score_wordcount_adj(slang, newscore,
preword + sp->ts_prewordlen, true);
}

if (TRY_DEEPER(su, stack, depth, newscore)) {
go_deeper(stack, depth, newscore);
#if defined(DEBUG_TRIEWALK)
if (!try_compound && !fword_ends)
sprintf(changename[depth], "%.*s-%s: split",
sp->ts_twordlen, tword, fword + sp->ts_fidx);
else
sprintf(changename[depth], "%.*s-%s: compound",
sp->ts_twordlen, tword, fword + sp->ts_fidx);
#endif

sp->ts_save_badflags = su->su_badflags;
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_SPLITUNDO;

++depth;
sp = &stack[depth];


if (!try_compound && !fword_ends)
STRCAT(preword, " ");
sp->ts_prewordlen = (char_u)STRLEN(preword);
sp->ts_splitoff = sp->ts_twordlen;
sp->ts_splitfidx = sp->ts_fidx;






if (((!try_compound && !spell_iswordp_nmw(fword
+ sp->ts_fidx,
curwin))
|| fword_ends)
&& fword[sp->ts_fidx] != NUL
&& goodword_ends) {
int l;

l = utfc_ptr2len(fword + sp->ts_fidx);
if (fword_ends) {

memmove(preword + sp->ts_prewordlen,
fword + sp->ts_fidx, l);
sp->ts_prewordlen += l;
preword[sp->ts_prewordlen] = NUL;
} else
sp->ts_score -= SCORE_SPLIT - SCORE_SUBST;
sp->ts_fidx += l;
}




if (try_compound)
++sp->ts_complen;
else
sp->ts_compsplit = sp->ts_complen;
sp->ts_prefixdepth = PFD_NOPREFIX;



if (has_mbyte)
n = nofold_len(fword, sp->ts_fidx, su->su_badptr);
else
n = sp->ts_fidx;
su->su_badflags = badword_captype(su->su_badptr + n,
su->su_badptr + su->su_badlen);


sp->ts_arridx = 0;


if (pbyts != NULL) {
byts = pbyts;
idxs = pidxs;
sp->ts_prefixdepth = PFD_PREFIXTREE;
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_NOPREFIX;
}
}
}
}
break;

case STATE_SPLITUNDO:

su->su_badflags = sp->ts_save_badflags;


PROF_STORE(sp->ts_state)
sp->ts_state = STATE_START;


byts = fbyts;
idxs = fidxs;
break;

case STATE_ENDNUL:

su->su_badflags = sp->ts_save_badflags;
if (fword[sp->ts_fidx] == NUL
&& sp->ts_tcharlen == 0
) {

PROF_STORE(sp->ts_state)
sp->ts_state = STATE_DEL;
break;
}
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_PLAIN;
FALLTHROUGH;

case STATE_PLAIN:


arridx = sp->ts_arridx;
if (sp->ts_curi > byts[arridx]) {


PROF_STORE(sp->ts_state)
if (sp->ts_fidx >= sp->ts_fidxtry) {
sp->ts_state = STATE_DEL;
} else {
sp->ts_state = STATE_FINAL;
}
} else {
arridx += sp->ts_curi++;
c = byts[arridx];






if (c == fword[sp->ts_fidx]
|| (sp->ts_tcharlen > 0 && sp->ts_isdiff != DIFF_NONE)
)
newscore = 0;
else
newscore = SCORE_SUBST;
if ((newscore == 0
|| (sp->ts_fidx >= sp->ts_fidxtry
&& ((sp->ts_flags & TSF_DIDDEL) == 0
|| c != fword[sp->ts_delidx])))
&& TRY_DEEPER(su, stack, depth, newscore)) {
go_deeper(stack, depth, newscore);
#if defined(DEBUG_TRIEWALK)
if (newscore > 0)
sprintf(changename[depth], "%.*s-%s: subst %c to %c",
sp->ts_twordlen, tword, fword + sp->ts_fidx,
fword[sp->ts_fidx], c);
else
sprintf(changename[depth], "%.*s-%s: accept %c",
sp->ts_twordlen, tword, fword + sp->ts_fidx,
fword[sp->ts_fidx]);
#endif
++depth;
sp = &stack[depth];
++sp->ts_fidx;
tword[sp->ts_twordlen++] = c;
sp->ts_arridx = idxs[arridx];
if (newscore == SCORE_SUBST)
sp->ts_isdiff = DIFF_YES;
if (has_mbyte) {



if (sp->ts_tcharlen == 0) {

sp->ts_tcharidx = 0;
sp->ts_tcharlen = MB_BYTE2LEN(c);
sp->ts_fcharstart = sp->ts_fidx - 1;
sp->ts_isdiff = (newscore != 0)
? DIFF_YES : DIFF_NONE;
} else if (sp->ts_isdiff == DIFF_INSERT)


--sp->ts_fidx;
if (++sp->ts_tcharidx == sp->ts_tcharlen) {

if (sp->ts_isdiff == DIFF_YES) {


sp->ts_fidx = sp->ts_fcharstart
+ utfc_ptr2len(fword + sp->ts_fcharstart);




if (utf_iscomposing(utf_ptr2char(tword + sp->ts_twordlen
- sp->ts_tcharlen))
&& utf_iscomposing(utf_ptr2char(fword
+ sp->ts_fcharstart))) {
sp->ts_score -= SCORE_SUBST - SCORE_SUBCOMP;
} else if (
!soundfold
&& slang->sl_has_map
&& similar_chars(
slang,
utf_ptr2char(tword + sp->ts_twordlen - sp->ts_tcharlen),
utf_ptr2char(fword + sp->ts_fcharstart))) {


sp->ts_score -= SCORE_SUBST - SCORE_SIMILAR;
}
} else if (sp->ts_isdiff == DIFF_INSERT
&& sp->ts_twordlen > sp->ts_tcharlen) {
p = tword + sp->ts_twordlen - sp->ts_tcharlen;
c = utf_ptr2char(p);
if (utf_iscomposing(c)) {


sp->ts_score -= SCORE_INS - SCORE_INSCOMP;
} else {





MB_PTR_BACK(tword, p);
if (c == utf_ptr2char(p)) {
sp->ts_score -= SCORE_INS - SCORE_INSDUP;
}
}
}


sp->ts_tcharlen = 0;
}
} else {



if (newscore != 0
&& !soundfold
&& slang->sl_has_map
&& similar_chars(slang,
c, fword[sp->ts_fidx - 1]))
sp->ts_score -= SCORE_SUBST - SCORE_SIMILAR;
}
}
}
break;

case STATE_DEL:


if (has_mbyte && sp->ts_tcharlen > 0) {
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_FINAL;
break;
}

PROF_STORE(sp->ts_state)
sp->ts_state = STATE_INS_PREP;
sp->ts_curi = 1;
if (soundfold && sp->ts_fidx == 0 && fword[sp->ts_fidx] == '*')


newscore = 2 * SCORE_DEL / 3;
else
newscore = SCORE_DEL;
if (fword[sp->ts_fidx] != NUL
&& TRY_DEEPER(su, stack, depth, newscore)) {
go_deeper(stack, depth, newscore);
#if defined(DEBUG_TRIEWALK)
sprintf(changename[depth], "%.*s-%s: delete %c",
sp->ts_twordlen, tword, fword + sp->ts_fidx,
fword[sp->ts_fidx]);
#endif
++depth;



stack[depth].ts_flags |= TSF_DIDDEL;
stack[depth].ts_delidx = sp->ts_fidx;





c = utf_ptr2char(fword + sp->ts_fidx);
stack[depth].ts_fidx += utfc_ptr2len(fword + sp->ts_fidx);
if (utf_iscomposing(c)) {
stack[depth].ts_score -= SCORE_DEL - SCORE_DELCOMP;
} else if (c == utf_ptr2char(fword + stack[depth].ts_fidx)) {
stack[depth].ts_score -= SCORE_DEL - SCORE_DELDUP;
}

break;
}
FALLTHROUGH;

case STATE_INS_PREP:
if (sp->ts_flags & TSF_DIDDEL) {


PROF_STORE(sp->ts_state)
sp->ts_state = STATE_SWAP;
break;
}


n = sp->ts_arridx;
for (;; ) {
if (sp->ts_curi > byts[n]) {

PROF_STORE(sp->ts_state)
sp->ts_state = STATE_SWAP;
break;
}
if (byts[n + sp->ts_curi] != NUL) {

PROF_STORE(sp->ts_state)
sp->ts_state = STATE_INS;
break;
}
++sp->ts_curi;
}
break;

FALLTHROUGH;

case STATE_INS:


n = sp->ts_arridx;
if (sp->ts_curi > byts[n]) {

PROF_STORE(sp->ts_state)
sp->ts_state = STATE_SWAP;
break;
}





n += sp->ts_curi++;
c = byts[n];
if (soundfold && sp->ts_twordlen == 0 && c == '*')


newscore = 2 * SCORE_INS / 3;
else
newscore = SCORE_INS;
if (c != fword[sp->ts_fidx]
&& TRY_DEEPER(su, stack, depth, newscore)) {
go_deeper(stack, depth, newscore);
#if defined(DEBUG_TRIEWALK)
sprintf(changename[depth], "%.*s-%s: insert %c",
sp->ts_twordlen, tword, fword + sp->ts_fidx,
c);
#endif
++depth;
sp = &stack[depth];
tword[sp->ts_twordlen++] = c;
sp->ts_arridx = idxs[n];
if (has_mbyte) {
fl = MB_BYTE2LEN(c);
if (fl > 1) {



sp->ts_tcharlen = fl;
sp->ts_tcharidx = 1;
sp->ts_isdiff = DIFF_INSERT;
}
} else
fl = 1;
if (fl == 1) {




if (sp->ts_twordlen >= 2
&& tword[sp->ts_twordlen - 2] == c)
sp->ts_score -= SCORE_INS - SCORE_INSDUP;
}
}
break;

case STATE_SWAP:



p = fword + sp->ts_fidx;
c = *p;
if (c == NUL) {

PROF_STORE(sp->ts_state)
sp->ts_state = STATE_FINAL;
break;
}



if (!soundfold && !spell_iswordp(p, curwin)) {
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_REP_INI;
break;
}

n = MB_CPTR2LEN(p);
c = utf_ptr2char(p);
if (p[n] == NUL) {
c2 = NUL;
} else if (!soundfold && !spell_iswordp(p + n, curwin)) {
c2 = c; 
} else {
c2 = utf_ptr2char(p + n);
}


if (c2 == NUL) {
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_REP_INI;
break;
}



if (c == c2) {
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_SWAP3;
break;
}
if (TRY_DEEPER(su, stack, depth, SCORE_SWAP)) {
go_deeper(stack, depth, SCORE_SWAP);
#if defined(DEBUG_TRIEWALK)
snprintf(changename[depth], sizeof(changename[0]),
"%.*s-%s: swap %c and %c",
sp->ts_twordlen, tword, fword + sp->ts_fidx,
c, c2);
#endif
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_UNSWAP;
depth++;
fl = mb_char2len(c2);
memmove(p, p + n, fl);
utf_char2bytes(c, p + fl);
stack[depth].ts_fidxtry = sp->ts_fidx + n + fl;
} else {

PROF_STORE(sp->ts_state)
sp->ts_state = STATE_REP_INI;
}
break;

case STATE_UNSWAP:

p = fword + sp->ts_fidx;
n = utfc_ptr2len(p);
c = utf_ptr2char(p + n);
memmove(p + utfc_ptr2len(p + n), p, n);
utf_char2bytes(c, p);

FALLTHROUGH;

case STATE_SWAP3:


p = fword + sp->ts_fidx;
n = MB_CPTR2LEN(p);
c = utf_ptr2char(p);
fl = MB_CPTR2LEN(p + n);
c2 = utf_ptr2char(p + n);
if (!soundfold && !spell_iswordp(p + n + fl, curwin)) {
c3 = c; 
} else {
c3 = utf_ptr2char(p + n + fl);
}







if (c == c3 || c3 == NUL) {
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_REP_INI;
break;
}
if (TRY_DEEPER(su, stack, depth, SCORE_SWAP3)) {
go_deeper(stack, depth, SCORE_SWAP3);
#if defined(DEBUG_TRIEWALK)
sprintf(changename[depth], "%.*s-%s: swap3 %c and %c",
sp->ts_twordlen, tword, fword + sp->ts_fidx,
c, c3);
#endif
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_UNSWAP3;
depth++;
tl = mb_char2len(c3);
memmove(p, p + n + fl, tl);
utf_char2bytes(c2, p + tl);
utf_char2bytes(c, p + fl + tl);
stack[depth].ts_fidxtry = sp->ts_fidx + n + fl + tl;
} else {
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_REP_INI;
}
break;

case STATE_UNSWAP3:

p = fword + sp->ts_fidx;
n = utfc_ptr2len(p);
c2 = utf_ptr2char(p + n);
fl = utfc_ptr2len(p + n);
c = utf_ptr2char(p + n + fl);
tl = utfc_ptr2len(p + n + fl);
memmove(p + fl + tl, p, n);
utf_char2bytes(c, p);
utf_char2bytes(c2, p + tl);
p = p + tl;

if (!soundfold && !spell_iswordp(p, curwin)) {


PROF_STORE(sp->ts_state)
sp->ts_state = STATE_REP_INI;
break;
}



if (TRY_DEEPER(su, stack, depth, SCORE_SWAP3)) {
go_deeper(stack, depth, SCORE_SWAP3);
#if defined(DEBUG_TRIEWALK)
p = fword + sp->ts_fidx;
sprintf(changename[depth], "%.*s-%s: rotate left %c%c%c",
sp->ts_twordlen, tword, fword + sp->ts_fidx,
p[0], p[1], p[2]);
#endif
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_UNROT3L;
++depth;
p = fword + sp->ts_fidx;
n = MB_CPTR2LEN(p);
c = utf_ptr2char(p);
fl = MB_CPTR2LEN(p + n);
fl += MB_CPTR2LEN(p + n + fl);
memmove(p, p + n, fl);
utf_char2bytes(c, p + fl);
stack[depth].ts_fidxtry = sp->ts_fidx + n + fl;
} else {
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_REP_INI;
}
break;

case STATE_UNROT3L:

p = fword + sp->ts_fidx;
n = utfc_ptr2len(p);
n += utfc_ptr2len(p + n);
c = utf_ptr2char(p + n);
tl = utfc_ptr2len(p + n);
memmove(p + tl, p, n);
utf_char2bytes(c, p);



if (TRY_DEEPER(su, stack, depth, SCORE_SWAP3)) {
go_deeper(stack, depth, SCORE_SWAP3);
#if defined(DEBUG_TRIEWALK)
p = fword + sp->ts_fidx;
sprintf(changename[depth], "%.*s-%s: rotate right %c%c%c",
sp->ts_twordlen, tword, fword + sp->ts_fidx,
p[0], p[1], p[2]);
#endif
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_UNROT3R;
++depth;
p = fword + sp->ts_fidx;
n = MB_CPTR2LEN(p);
n += MB_CPTR2LEN(p + n);
c = utf_ptr2char(p + n);
tl = MB_CPTR2LEN(p + n);
memmove(p + tl, p, n);
utf_char2bytes(c, p);
stack[depth].ts_fidxtry = sp->ts_fidx + n + tl;
} else {
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_REP_INI;
}
break;

case STATE_UNROT3R:

p = fword + sp->ts_fidx;
c = utf_ptr2char(p);
tl = utfc_ptr2len(p);
n = utfc_ptr2len(p + tl);
n += utfc_ptr2len(p + tl + n);
memmove(p, p + tl, n);
utf_char2bytes(c, p + n);

FALLTHROUGH;

case STATE_REP_INI:





if ((lp->lp_replang == NULL && !soundfold)
|| sp->ts_score + SCORE_REP >= su->su_maxscore
|| sp->ts_fidx < sp->ts_fidxtry) {
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_FINAL;
break;
}



if (soundfold)
sp->ts_curi = slang->sl_repsal_first[fword[sp->ts_fidx]];
else
sp->ts_curi = lp->lp_replang->sl_rep_first[fword[sp->ts_fidx]];

if (sp->ts_curi < 0) {
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_FINAL;
break;
}

PROF_STORE(sp->ts_state)
sp->ts_state = STATE_REP;
FALLTHROUGH;

case STATE_REP:



p = fword + sp->ts_fidx;

if (soundfold)
gap = &slang->sl_repsal;
else
gap = &lp->lp_replang->sl_rep;
while (sp->ts_curi < gap->ga_len) {
ftp = (fromto_T *)gap->ga_data + sp->ts_curi++;
if (*ftp->ft_from != *p) {

sp->ts_curi = gap->ga_len;
break;
}
if (STRNCMP(ftp->ft_from, p, STRLEN(ftp->ft_from)) == 0
&& TRY_DEEPER(su, stack, depth, SCORE_REP)) {
go_deeper(stack, depth, SCORE_REP);
#if defined(DEBUG_TRIEWALK)
sprintf(changename[depth], "%.*s-%s: replace %s with %s",
sp->ts_twordlen, tword, fword + sp->ts_fidx,
ftp->ft_from, ftp->ft_to);
#endif

PROF_STORE(sp->ts_state)
sp->ts_state = STATE_REP_UNDO;


++depth;
fl = (int)STRLEN(ftp->ft_from);
tl = (int)STRLEN(ftp->ft_to);
if (fl != tl) {
STRMOVE(p + tl, p + fl);
repextra += tl - fl;
}
memmove(p, ftp->ft_to, tl);
stack[depth].ts_fidxtry = sp->ts_fidx + tl;
stack[depth].ts_tcharlen = 0;
break;
}
}

if (sp->ts_curi >= gap->ga_len && sp->ts_state == STATE_REP)

PROF_STORE(sp->ts_state)
sp->ts_state = STATE_FINAL;

break;

case STATE_REP_UNDO:

if (soundfold)
gap = &slang->sl_repsal;
else
gap = &lp->lp_replang->sl_rep;
ftp = (fromto_T *)gap->ga_data + sp->ts_curi - 1;
fl = (int)STRLEN(ftp->ft_from);
tl = (int)STRLEN(ftp->ft_to);
p = fword + sp->ts_fidx;
if (fl != tl) {
STRMOVE(p + fl, p + tl);
repextra -= tl - fl;
}
memmove(p, ftp->ft_from, fl);
PROF_STORE(sp->ts_state)
sp->ts_state = STATE_REP;
break;

default:

--depth;

if (depth >= 0 && stack[depth].ts_prefixdepth == PFD_PREFIXTREE) {

byts = pbyts;
idxs = pidxs;
}


if (--breakcheckcount == 0) {
os_breakcheck();
breakcheckcount = 1000;
}
}
}
}



static void go_deeper(trystate_T *stack, int depth, int score_add)
{
stack[depth + 1] = stack[depth];
stack[depth + 1].ts_state = STATE_START;
stack[depth + 1].ts_score = stack[depth].ts_score + score_add;
stack[depth + 1].ts_curi = 1; 
stack[depth + 1].ts_flags = 0;
}



static int nofold_len(char_u *fword, int flen, char_u *word)
{
char_u *p;
int i = 0;

for (p = fword; p < fword + flen; MB_PTR_ADV(p)) {
i++;
}
for (p = word; i > 0; MB_PTR_ADV(p)) {
i--;
}
return (int)(p - word);
}





static void find_keepcap_word(slang_T *slang, char_u *fword, char_u *kword)
{
char_u uword[MAXWLEN]; 
int depth;
idx_T tryidx;


idx_T arridx[MAXWLEN];
int round[MAXWLEN];
int fwordidx[MAXWLEN];
int uwordidx[MAXWLEN];
int kwordlen[MAXWLEN];

int flen, ulen;
int l;
int len;
int c;
idx_T lo, hi, m;
char_u *p;
char_u *byts = slang->sl_kbyts; 
idx_T *idxs = slang->sl_kidxs; 

if (byts == NULL) {

*kword = NUL;
return;
}


allcap_copy(fword, uword);




depth = 0;
arridx[0] = 0;
round[0] = 0;
fwordidx[0] = 0;
uwordidx[0] = 0;
kwordlen[0] = 0;
while (depth >= 0) {
if (fword[fwordidx[depth]] == NUL) {


if (byts[arridx[depth] + 1] == 0) {
kword[kwordlen[depth]] = NUL;
return;
}


--depth;
} else if (++round[depth] > 2) {


--depth;
} else {


if (has_mbyte) {
flen = MB_CPTR2LEN(fword + fwordidx[depth]);
ulen = MB_CPTR2LEN(uword + uwordidx[depth]);
} else {
ulen = flen = 1;
}
if (round[depth] == 1) {
p = fword + fwordidx[depth];
l = flen;
} else {
p = uword + uwordidx[depth];
l = ulen;
}

for (tryidx = arridx[depth]; l > 0; --l) {

len = byts[tryidx++];
c = *p++;
lo = tryidx;
hi = tryidx + len - 1;
while (lo < hi) {
m = (lo + hi) / 2;
if (byts[m] > c)
hi = m - 1;
else if (byts[m] < c)
lo = m + 1;
else {
lo = hi = m;
break;
}
}


if (hi < lo || byts[lo] != c)
break;


tryidx = idxs[lo];
}

if (l == 0) {


if (round[depth] == 1) {
STRNCPY(kword + kwordlen[depth], fword + fwordidx[depth],
flen);
kwordlen[depth + 1] = kwordlen[depth] + flen;
} else {
STRNCPY(kword + kwordlen[depth], uword + uwordidx[depth],
ulen);
kwordlen[depth + 1] = kwordlen[depth] + ulen;
}
fwordidx[depth + 1] = fwordidx[depth] + flen;
uwordidx[depth + 1] = uwordidx[depth] + ulen;

++depth;
arridx[depth] = tryidx;
round[depth] = 0;
}
}
}


*kword = NUL;
}



static void score_comp_sal(suginfo_T *su)
{
langp_T *lp;
char_u badsound[MAXWLEN];
int i;
suggest_T *stp;
suggest_T *sstp;
int score;

ga_grow(&su->su_sga, su->su_ga.ga_len);


for (int lpi = 0; lpi < curwin->w_s->b_langp.ga_len; ++lpi) {
lp = LANGP_ENTRY(curwin->w_s->b_langp, lpi);
if (!GA_EMPTY(&lp->lp_slang->sl_sal)) {

spell_soundfold(lp->lp_slang, su->su_fbadword, true, badsound);

for (i = 0; i < su->su_ga.ga_len; ++i) {
stp = &SUG(su->su_ga, i);



score = stp_sal_score(stp, su, lp->lp_slang, badsound);
if (score < SCORE_MAXMAX) {

sstp = &SUG(su->su_sga, su->su_sga.ga_len);
sstp->st_word = vim_strsave(stp->st_word);
sstp->st_wordlen = stp->st_wordlen;
sstp->st_score = score;
sstp->st_altscore = 0;
sstp->st_orglen = stp->st_orglen;
++su->su_sga.ga_len;
}
}
break;
}
}
}



static void score_combine(suginfo_T *su)
{
garray_T ga;
garray_T *gap;
langp_T *lp;
suggest_T *stp;
char_u *p;
char_u badsound[MAXWLEN];
int round;
slang_T *slang = NULL;


for (int lpi = 0; lpi < curwin->w_s->b_langp.ga_len; ++lpi) {
lp = LANGP_ENTRY(curwin->w_s->b_langp, lpi);
if (!GA_EMPTY(&lp->lp_slang->sl_sal)) {

slang = lp->lp_slang;
spell_soundfold(slang, su->su_fbadword, true, badsound);

for (int i = 0; i < su->su_ga.ga_len; ++i) {
stp = &SUG(su->su_ga, i);
stp->st_altscore = stp_sal_score(stp, su, slang, badsound);
if (stp->st_altscore == SCORE_MAXMAX)
stp->st_score = (stp->st_score * 3 + SCORE_BIG) / 4;
else
stp->st_score = (stp->st_score * 3
+ stp->st_altscore) / 4;
stp->st_salscore = false;
}
break;
}
}

if (slang == NULL) { 
(void)cleanup_suggestions(&su->su_ga, su->su_maxscore,
su->su_maxcount);
return;
}


for (int i = 0; i < su->su_sga.ga_len; ++i) {
stp = &SUG(su->su_sga, i);
stp->st_altscore = spell_edit_score(slang,
su->su_badword, stp->st_word);
if (stp->st_score == SCORE_MAXMAX)
stp->st_score = (SCORE_BIG * 7 + stp->st_altscore) / 8;
else
stp->st_score = (stp->st_score * 7 + stp->st_altscore) / 8;
stp->st_salscore = true;
}



check_suggestions(su, &su->su_ga);
(void)cleanup_suggestions(&su->su_ga, su->su_maxscore, su->su_maxcount);
check_suggestions(su, &su->su_sga);
(void)cleanup_suggestions(&su->su_sga, su->su_maxscore, su->su_maxcount);

ga_init(&ga, (int)sizeof(suginfo_T), 1);
ga_grow(&ga, su->su_ga.ga_len + su->su_sga.ga_len);

stp = &SUG(ga, 0);
for (int i = 0; i < su->su_ga.ga_len || i < su->su_sga.ga_len; ++i) {


for (round = 1; round <= 2; ++round) {
gap = round == 1 ? &su->su_ga : &su->su_sga;
if (i < gap->ga_len) {

p = SUG(*gap, i).st_word;
int j;
for (j = 0; j < ga.ga_len; ++j)
if (STRCMP(stp[j].st_word, p) == 0)
break;
if (j == ga.ga_len)
stp[ga.ga_len++] = SUG(*gap, i);
else
xfree(p);
}
}
}

ga_clear(&su->su_ga);
ga_clear(&su->su_sga);


if (ga.ga_len > su->su_maxcount) {
for (int i = su->su_maxcount; i < ga.ga_len; ++i) {
xfree(stp[i].st_word);
}
ga.ga_len = su->su_maxcount;
}

su->su_ga = ga;
}



static int
stp_sal_score (
suggest_T *stp,
suginfo_T *su,
slang_T *slang,
char_u *badsound 
)
{
char_u *p;
char_u *pbad;
char_u *pgood;
char_u badsound2[MAXWLEN];
char_u fword[MAXWLEN];
char_u goodsound[MAXWLEN];
char_u goodword[MAXWLEN];
int lendiff;

lendiff = su->su_badlen - stp->st_orglen;
if (lendiff >= 0)
pbad = badsound;
else {

(void)spell_casefold(su->su_badptr, stp->st_orglen, fword, MAXWLEN);





if (ascii_iswhite(su->su_badptr[su->su_badlen])
&& *skiptowhite(stp->st_word) == NUL)
for (p = fword; *(p = skiptowhite(p)) != NUL; )
STRMOVE(p, p + 1);

spell_soundfold(slang, fword, true, badsound2);
pbad = badsound2;
}

if (lendiff > 0 && stp->st_wordlen + lendiff < MAXWLEN) {


STRCPY(goodword, stp->st_word);
STRLCPY(goodword + stp->st_wordlen,
su->su_badptr + su->su_badlen - lendiff, lendiff + 1);
pgood = goodword;
} else
pgood = stp->st_word;


spell_soundfold(slang, pgood, false, goodsound);

return soundalike_score(goodsound, pbad);
}

static sftword_T dumsft;
#define HIKEY2SFT(p) ((sftword_T *)(p - (dumsft.sft_word - (char_u *)&dumsft)))
#define HI2SFT(hi) HIKEY2SFT((hi)->hi_key)


static void suggest_try_soundalike_prep(void)
{
langp_T *lp;
slang_T *slang;



for (int lpi = 0; lpi < curwin->w_s->b_langp.ga_len; ++lpi) {
lp = LANGP_ENTRY(curwin->w_s->b_langp, lpi);
slang = lp->lp_slang;
if (!GA_EMPTY(&slang->sl_sal) && slang->sl_sbyts != NULL)

hash_init(&slang->sl_sounddone);
}
}



static void suggest_try_soundalike(suginfo_T *su)
{
char_u salword[MAXWLEN];
langp_T *lp;
slang_T *slang;



for (int lpi = 0; lpi < curwin->w_s->b_langp.ga_len; ++lpi) {
lp = LANGP_ENTRY(curwin->w_s->b_langp, lpi);
slang = lp->lp_slang;
if (!GA_EMPTY(&slang->sl_sal) && slang->sl_sbyts != NULL) {

spell_soundfold(slang, su->su_fbadword, true, salword);




#if defined(SUGGEST_PROFILE)
prof_init();
#endif
suggest_trie_walk(su, lp, salword, true);
#if defined(SUGGEST_PROFILE)
prof_report("soundalike");
#endif
}
}
}


static void suggest_try_soundalike_finish(void)
{
langp_T *lp;
slang_T *slang;
int todo;
hashitem_T *hi;



for (int lpi = 0; lpi < curwin->w_s->b_langp.ga_len; ++lpi) {
lp = LANGP_ENTRY(curwin->w_s->b_langp, lpi);
slang = lp->lp_slang;
if (!GA_EMPTY(&slang->sl_sal) && slang->sl_sbyts != NULL) {

todo = (int)slang->sl_sounddone.ht_used;
for (hi = slang->sl_sounddone.ht_array; todo > 0; ++hi)
if (!HASHITEM_EMPTY(hi)) {
xfree(HI2SFT(hi));
--todo;
}


hash_clear(&slang->sl_sounddone);
hash_init(&slang->sl_sounddone);
}
}
}



static void
add_sound_suggest (
suginfo_T *su,
char_u *goodword,
int score, 
langp_T *lp
)
{
slang_T *slang = lp->lp_slang; 
int sfwordnr;
char_u *nrline;
int orgnr;
char_u theword[MAXWLEN];
int i;
int wlen;
char_u *byts;
idx_T *idxs;
int n;
int wordcount;
int wc;
int goodscore;
hash_T hash;
hashitem_T *hi;
sftword_T *sft;
int bc, gc;
int limit;





hash = hash_hash(goodword);
const size_t goodword_len = STRLEN(goodword);
hi = hash_lookup(&slang->sl_sounddone, (const char *)goodword, goodword_len,
hash);
if (HASHITEM_EMPTY(hi)) {
sft = xmalloc(sizeof(sftword_T) + goodword_len);
sft->sft_score = score;
memcpy(sft->sft_word, goodword, goodword_len + 1);
hash_add_item(&slang->sl_sounddone, hi, sft->sft_word, hash);
} else {
sft = HI2SFT(hi);
if (score >= sft->sft_score)
return;
sft->sft_score = score;
}


sfwordnr = soundfold_find(slang, goodword);
if (sfwordnr < 0) {
internal_error("add_sound_suggest()");
return;
}


nrline = ml_get_buf(slang->sl_sugbuf, (linenr_T)sfwordnr + 1, false);
orgnr = 0;
while (*nrline != NUL) {


orgnr += bytes2offset(&nrline);

byts = slang->sl_fbyts;
idxs = slang->sl_fidxs;


n = 0;
wordcount = 0;
for (wlen = 0; wlen < MAXWLEN - 3; ++wlen) {
i = 1;
if (wordcount == orgnr && byts[n + 1] == NUL)
break; 

if (byts[n + 1] == NUL)
++wordcount;


for (; byts[n + i] == NUL; ++i)
if (i > byts[n]) { 
STRCPY(theword + wlen, "BAD");
wlen += 3;
goto badword;
}


for (; i < byts[n]; ++i) {
wc = idxs[idxs[n + i]]; 
if (wordcount + wc > orgnr)
break;
wordcount += wc;
}

theword[wlen] = byts[n + i];
n = idxs[n + i];
}
badword:
theword[wlen] = NUL;


for (; i <= byts[n] && byts[n + i] == NUL; ++i) {
char_u cword[MAXWLEN];
char_u *p;
int flags = (int)idxs[n + i];


if (flags & WF_NOSUGGEST)
continue;

if (flags & WF_KEEPCAP) {

find_keepcap_word(slang, theword, cword);
p = cword;
} else {
flags |= su->su_badflags;
if ((flags & WF_CAPMASK) != 0) {

make_case_word(theword, cword, flags);
p = cword;
} else
p = theword;
}


if (sps_flags & SPS_DOUBLE) {

if (score <= su->su_maxscore)
add_suggestion(su, &su->su_sga, p, su->su_badlen,
score, 0, false, slang, false);
} else {

if ((flags & WF_REGION)
&& (((unsigned)flags >> 16) & lp->lp_region) == 0)
goodscore = SCORE_REGION;
else
goodscore = 0;





gc = PTR2CHAR(p);
if (SPELL_ISUPPER(gc)) {
bc = PTR2CHAR(su->su_badword);
if (!SPELL_ISUPPER(bc)
&& SPELL_TOFOLD(bc) != SPELL_TOFOLD(gc))
goodscore += SCORE_ICASE / 2;
}








limit = MAXSCORE(su->su_sfmaxscore - goodscore, score);
if (limit > SCORE_LIMITMAX)
goodscore += spell_edit_score(slang, su->su_badword, p);
else
goodscore += spell_edit_score_limit(slang, su->su_badword,
p, limit);


if (goodscore < SCORE_MAXMAX) {

goodscore = score_wordcount_adj(slang, goodscore, p, false);


goodscore = RESCORE(goodscore, score);
if (goodscore <= su->su_sfmaxscore)
add_suggestion(su, &su->su_ga, p, su->su_badlen,
goodscore, score, true, slang, true);
}
}
}
}
}


static int soundfold_find(slang_T *slang, char_u *word)
{
idx_T arridx = 0;
int len;
int wlen = 0;
int c;
char_u *ptr = word;
char_u *byts;
idx_T *idxs;
int wordnr = 0;

byts = slang->sl_sbyts;
idxs = slang->sl_sidxs;

for (;; ) {

len = byts[arridx++];



c = ptr[wlen];
if (byts[arridx] == NUL) {
if (c == NUL)
break;


while (len > 0 && byts[arridx] == NUL) {
++arridx;
--len;
}
if (len == 0)
return -1; 
++wordnr;
}


if (c == NUL)
return -1;


if (c == TAB) 
c = ' ';
while (byts[arridx] < c) {

wordnr += idxs[idxs[arridx]];
++arridx;
if (--len == 0) 
return -1;
}
if (byts[arridx] != c) 
return -1;


arridx = idxs[arridx];
++wlen;



if (c == ' ')
while (ptr[wlen] == ' ' || ptr[wlen] == TAB)
++wlen;
}

return wordnr;
}


static void make_case_word(char_u *fword, char_u *cword, int flags)
{
if (flags & WF_ALLCAP)

allcap_copy(fword, cword);
else if (flags & WF_ONECAP)

onecap_copy(fword, cword, true);
else

STRCPY(cword, fword);
}



static bool similar_chars(slang_T *slang, int c1, int c2)
{
int m1, m2;
char_u buf[MB_MAXBYTES + 1];
hashitem_T *hi;

if (c1 >= 256) {
buf[utf_char2bytes(c1, buf)] = 0;
hi = hash_find(&slang->sl_map_hash, buf);
if (HASHITEM_EMPTY(hi)) {
m1 = 0;
} else {
m1 = utf_ptr2char(hi->hi_key + STRLEN(hi->hi_key) + 1);
}
} else {
m1 = slang->sl_map_array[c1];
}
if (m1 == 0) {
return false;
}

if (c2 >= 256) {
buf[utf_char2bytes(c2, buf)] = 0;
hi = hash_find(&slang->sl_map_hash, buf);
if (HASHITEM_EMPTY(hi)) {
m2 = 0;
} else {
m2 = utf_ptr2char(hi->hi_key + STRLEN(hi->hi_key) + 1);
}
} else {
m2 = slang->sl_map_array[c2];
}

return m1 == m2;
}



static void
add_suggestion (
suginfo_T *su,
garray_T *gap, 
const char_u *goodword,
int badlenarg, 
int score,
int altscore,
bool had_bonus, 
slang_T *slang, 
bool maxsf 

)
{
int goodlen; 
int badlen; 
suggest_T *stp;
suggest_T new_sug;



const char_u *pgood = goodword + STRLEN(goodword);
char_u *pbad = su->su_badptr + badlenarg;
for (;; ) {
goodlen = (int)(pgood - goodword);
badlen = (int)(pbad - su->su_badptr);
if (goodlen <= 0 || badlen <= 0)
break;
MB_PTR_BACK(goodword, pgood);
MB_PTR_BACK(su->su_badptr, pbad);
if (utf_ptr2char(pgood) != utf_ptr2char(pbad)) {
break;
}
}

if (badlen == 0 && goodlen == 0)


return;

int i;
if (GA_EMPTY(gap)) {
i = -1;
} else {



stp = &SUG(*gap, 0);
for (i = gap->ga_len; --i >= 0; ++stp) {
if (stp->st_wordlen == goodlen
&& stp->st_orglen == badlen
&& STRNCMP(stp->st_word, goodword, goodlen) == 0) {

if (stp->st_slang == NULL)
stp->st_slang = slang;

new_sug.st_score = score;
new_sug.st_altscore = altscore;
new_sug.st_had_bonus = had_bonus;

if (stp->st_had_bonus != had_bonus) {






if (had_bonus)
rescore_one(su, stp);
else {
new_sug.st_word = stp->st_word;
new_sug.st_wordlen = stp->st_wordlen;
new_sug.st_slang = stp->st_slang;
new_sug.st_orglen = badlen;
rescore_one(su, &new_sug);
}
}

if (stp->st_score > new_sug.st_score) {
stp->st_score = new_sug.st_score;
stp->st_altscore = new_sug.st_altscore;
stp->st_had_bonus = new_sug.st_had_bonus;
}
break;
}
}
}

if (i < 0) {

stp = GA_APPEND_VIA_PTR(suggest_T, gap);
stp->st_word = vim_strnsave(goodword, goodlen);
stp->st_wordlen = goodlen;
stp->st_score = score;
stp->st_altscore = altscore;
stp->st_had_bonus = had_bonus;
stp->st_orglen = badlen;
stp->st_slang = slang;



if (gap->ga_len > SUG_MAX_COUNT(su)) {
if (maxsf)
su->su_sfmaxscore = cleanup_suggestions(gap,
su->su_sfmaxscore, SUG_CLEAN_COUNT(su));
else
su->su_maxscore = cleanup_suggestions(gap,
su->su_maxscore, SUG_CLEAN_COUNT(su));
}
}
}



static void
check_suggestions (
suginfo_T *su,
garray_T *gap 
)
{
suggest_T *stp;
char_u longword[MAXWLEN + 1];
int len;
hlf_T attr;

stp = &SUG(*gap, 0);
for (int i = gap->ga_len - 1; i >= 0; --i) {

STRLCPY(longword, stp[i].st_word, MAXWLEN + 1);
len = stp[i].st_wordlen;
STRLCPY(longword + len, su->su_badptr + stp[i].st_orglen,
MAXWLEN - len + 1);
attr = HLF_COUNT;
(void)spell_check(curwin, longword, &attr, NULL, false);
if (attr != HLF_COUNT) {

xfree(stp[i].st_word);
--gap->ga_len;
if (i < gap->ga_len)
memmove(stp + i, stp + i + 1,
sizeof(suggest_T) * (gap->ga_len - i));
}
}
}



static void add_banned(suginfo_T *su, char_u *word)
{
char_u *s;
hash_T hash;
hashitem_T *hi;

hash = hash_hash(word);
const size_t word_len = STRLEN(word);
hi = hash_lookup(&su->su_banned, (const char *)word, word_len, hash);
if (HASHITEM_EMPTY(hi)) {
s = xmemdupz(word, word_len);
hash_add_item(&su->su_banned, hi, s, hash);
}
}



static void rescore_suggestions(suginfo_T *su)
{
if (su->su_sallang != NULL) {
for (int i = 0; i < su->su_ga.ga_len; ++i) {
rescore_one(su, &SUG(su->su_ga, i));
}
}
}


static void rescore_one(suginfo_T *su, suggest_T *stp)
{
slang_T *slang = stp->st_slang;
char_u sal_badword[MAXWLEN];
char_u *p;



if (slang != NULL && !GA_EMPTY(&slang->sl_sal) && !stp->st_had_bonus) {
if (slang == su->su_sallang)
p = su->su_sal_badword;
else {
spell_soundfold(slang, su->su_fbadword, true, sal_badword);
p = sal_badword;
}

stp->st_altscore = stp_sal_score(stp, su, slang, p);
if (stp->st_altscore == SCORE_MAXMAX)
stp->st_altscore = SCORE_BIG;
stp->st_score = RESCORE(stp->st_score, stp->st_altscore);
stp->st_had_bonus = true;
}
}




static int sug_compare(const void *s1, const void *s2)
{
suggest_T *p1 = (suggest_T *)s1;
suggest_T *p2 = (suggest_T *)s2;
int n = p1->st_score - p2->st_score;

if (n == 0) {
n = p1->st_altscore - p2->st_altscore;
if (n == 0)
n = STRICMP(p1->st_word, p2->st_word);
}
return n;
}





static int
cleanup_suggestions (
garray_T *gap,
int maxscore,
int keep 
)
{
suggest_T *stp = &SUG(*gap, 0);


qsort(gap->ga_data, (size_t)gap->ga_len, sizeof(suggest_T), sug_compare);


if (gap->ga_len > keep) {
for (int i = keep; i < gap->ga_len; ++i) {
xfree(stp[i].st_word);
}
gap->ga_len = keep;
return stp[keep - 1].st_score;
}
return maxscore;
}








char *eval_soundfold(const char *const word)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_MALLOC FUNC_ATTR_NONNULL_ALL
{
if (curwin->w_p_spell && *curwin->w_s->b_p_spl != NUL) {

for (int lpi = 0; lpi < curwin->w_s->b_langp.ga_len; lpi++) {
langp_T *const lp = LANGP_ENTRY(curwin->w_s->b_langp, lpi);
if (!GA_EMPTY(&lp->lp_slang->sl_sal)) {

char_u sound[MAXWLEN];
spell_soundfold(lp->lp_slang, (char_u *)word, false, sound);
return xstrdup((const char *)sound);
}
}
}


return xstrdup(word);
}
















void spell_soundfold(slang_T *slang, char_u *inword, bool folded, char_u *res)
{
char_u fword[MAXWLEN];
char_u *word;

if (slang->sl_sofo)

spell_soundfold_sofo(slang, inword, res);
else {

if (folded)
word = inword;
else {
(void)spell_casefold(inword, (int)STRLEN(inword), fword, MAXWLEN);
word = fword;
}

spell_soundfold_wsal(slang, word, res);
}
}



static void spell_soundfold_sofo(slang_T *slang, char_u *inword, char_u *res)
{
char_u *s;
int ri = 0;
int c;

if (has_mbyte) {
int prevc = 0;
int *ip;



for (s = inword; *s != NUL; ) {
c = mb_cptr2char_adv((const char_u **)&s);
if (utf_class(c) == 0) {
c = ' ';
} else if (c < 256) {
c = slang->sl_sal_first[c];
} else {
ip = ((int **)slang->sl_sal.ga_data)[c & 0xff];
if (ip == NULL) 
c = NUL;
else
for (;; ) { 
if (*ip == 0) { 
c = NUL;
break;
}
if (*ip == c) { 
c = ip[1];
break;
}
ip += 2;
}
}

if (c != NUL && c != prevc) {
ri += utf_char2bytes(c, res + ri);
if (ri + MB_MAXBYTES > MAXWLEN) {
break;
}
prevc = c;
}
}
} else {

for (s = inword; (c = *s) != NUL; ++s) {
if (ascii_iswhite(c))
c = ' ';
else
c = slang->sl_sal_first[c];
if (c != NUL && (ri == 0 || res[ri - 1] != c))
res[ri++] = c;
}
}

res[ri] = NUL;
}



static void spell_soundfold_wsal(slang_T *slang, char_u *inword, char_u *res)
{
salitem_T *smp = (salitem_T *)slang->sl_sal.ga_data;
int word[MAXWLEN] = { 0 };
int wres[MAXWLEN] = { 0 };
int l;
int *ws;
int *pf;
int i, j, z;
int reslen;
int n, k = 0;
int z0;
int k0;
int n0;
int c;
int pri;
int p0 = -333;
int c0;
bool did_white = false;
int wordlen;





wordlen = 0;
for (const char_u *s = inword; *s != NUL; ) {
const char_u *t = s;
c = mb_cptr2char_adv((const char_u **)&s);
if (slang->sl_rem_accents) {
if (utf_class(c) == 0) {
if (did_white) {
continue;
}
c = ' ';
did_white = true;
} else {
did_white = false;
if (!spell_iswordp_nmw(t, curwin)) {
continue;
}
}
}
word[wordlen++] = c;
}
word[wordlen] = NUL;




i = reslen = z = 0;
while ((c = word[i]) != NUL) {

n = slang->sl_sal_first[c & 0xff];
z0 = 0;

if (n >= 0) {



for (; ((ws = smp[n].sm_lead_w)[0] & 0xff) == (c & 0xff)
&& ws[0] != NUL; ++n) {


if (c != ws[0])
continue;
k = smp[n].sm_leadlen;
if (k > 1) {
if (word[i + 1] != ws[1])
continue;
if (k > 2) {
for (j = 2; j < k; ++j)
if (word[i + j] != ws[j])
break;
if (j < k)
continue;
}
}

if ((pf = smp[n].sm_oneof_w) != NULL) {

while (*pf != NUL && *pf != word[i + k])
++pf;
if (*pf == NUL)
continue;
++k;
}
char_u *s = smp[n].sm_rules;
pri = 5; 

p0 = *s;
k0 = k;
while (*s == '-' && k > 1) {
k--;
s++;
}
if (*s == '<')
s++;
if (ascii_isdigit(*s)) {

pri = *s - '0';
s++;
}
if (*s == '^' && *(s + 1) == '^')
s++;

if (*s == NUL
|| (*s == '^'
&& (i == 0 || !(word[i - 1] == ' '
|| spell_iswordp_w(word + i - 1, curwin)))
&& (*(s + 1) != '$'
|| (!spell_iswordp_w(word + i + k0, curwin))))
|| (*s == '$' && i > 0
&& spell_iswordp_w(word + i - 1, curwin)
&& (!spell_iswordp_w(word + i + k0, curwin)))) {


c0 = word[i + k - 1];
n0 = slang->sl_sal_first[c0 & 0xff];

if (slang->sl_followup && k > 1 && n0 >= 0
&& p0 != '-' && word[i + k] != NUL) {


for (; ((ws = smp[n0].sm_lead_w)[0] & 0xff)
== (c0 & 0xff); ++n0) {

if (c0 != ws[0])
continue;
k0 = smp[n0].sm_leadlen;
if (k0 > 1) {
if (word[i + k] != ws[1])
continue;
if (k0 > 2) {
pf = word + i + k + 1;
for (j = 2; j < k0; ++j)
if (*pf++ != ws[j])
break;
if (j < k0)
continue;
}
}
k0 += k - 1;

if ((pf = smp[n0].sm_oneof_w) != NULL) {


while (*pf != NUL && *pf != word[i + k0])
++pf;
if (*pf == NUL)
continue;
++k0;
}

p0 = 5;
s = smp[n0].sm_rules;
while (*s == '-') {


s++;
}
if (*s == '<')
s++;
if (ascii_isdigit(*s)) {
p0 = *s - '0';
s++;
}

if (*s == NUL

|| (*s == '$'
&& !spell_iswordp_w(word + i + k0,
curwin))) {
if (k0 == k)

continue;

if (p0 < pri)

continue;

break;
}
}

if (p0 >= pri && (smp[n0].sm_lead_w[0] & 0xff)
== (c0 & 0xff))
continue;
}


ws = smp[n].sm_to_w;
s = smp[n].sm_rules;
p0 = (vim_strchr(s, '<') != NULL) ? 1 : 0;
if (p0 == 1 && z == 0) {

if (reslen > 0 && ws != NULL && *ws != NUL
&& (wres[reslen - 1] == c
|| wres[reslen - 1] == *ws))
reslen--;
z0 = 1;
z = 1;
k0 = 0;
if (ws != NULL)
while (*ws != NUL && word[i + k0] != NUL) {
word[i + k0] = *ws;
k0++;
ws++;
}
if (k > k0)
memmove(word + i + k0, word + i + k,
sizeof(int) * (wordlen - (i + k) + 1));


c = word[i];
} else {

i += k - 1;
z = 0;
if (ws != NULL)
while (*ws != NUL && ws[1] != NUL
&& reslen < MAXWLEN) {
if (reslen == 0 || wres[reslen - 1] != *ws)
wres[reslen++] = *ws;
ws++;
}

if (ws == NULL)
c = NUL;
else
c = *ws;
if (strstr((char *)s, "^^") != NULL) {
if (c != NUL)
wres[reslen++] = c;
memmove(word, word + i + 1,
sizeof(int) * (wordlen - (i + 1) + 1));
i = 0;
z0 = 1;
}
}
break;
}
}
} else if (ascii_iswhite(c)) {
c = ' ';
k = 1;
}

if (z0 == 0) {
if (k && !p0 && reslen < MAXWLEN && c != NUL
&& (!slang->sl_collapse || reslen == 0
|| wres[reslen - 1] != c))

wres[reslen++] = c;

i++;
z = 0;
k = 0;
}
}


l = 0;
for (n = 0; n < reslen; n++) {
l += utf_char2bytes(wres[n], res + l);
if (l + MB_MAXBYTES > MAXWLEN) {
break;
}
}
res[l] = NUL;
}





static int
soundalike_score (
char_u *goodstart, 
char_u *badstart 
)
{
char_u *goodsound = goodstart;
char_u *badsound = badstart;
int goodlen;
int badlen;
int n;
char_u *pl, *ps;
char_u *pl2, *ps2;
int score = 0;



if ((*badsound == '*' || *goodsound == '*') && *badsound != *goodsound) {
if ((badsound[0] == NUL && goodsound[1] == NUL)
|| (goodsound[0] == NUL && badsound[1] == NUL))

return SCORE_DEL;
if (badsound[0] == NUL || goodsound[0] == NUL)

return SCORE_MAXMAX;

if (badsound[1] == goodsound[1]
|| (badsound[1] != NUL
&& goodsound[1] != NUL
&& badsound[2] == goodsound[2])) {

} else {
score = 2 * SCORE_DEL / 3;
if (*badsound == '*')
++badsound;
else
++goodsound;
}
}

goodlen = (int)STRLEN(goodsound);
badlen = (int)STRLEN(badsound);



n = goodlen - badlen;
if (n < -2 || n > 2)
return SCORE_MAXMAX;

if (n > 0) {
pl = goodsound; 
ps = badsound;
} else {
pl = badsound; 
ps = goodsound;
}


while (*pl == *ps && *pl != NUL) {
++pl;
++ps;
}

switch (n) {
case -2:
case 2:

++pl; 
while (*pl == *ps) {
++pl;
++ps;
}

if (STRCMP(pl + 1, ps) == 0)
return score + SCORE_DEL * 2;


break;

case -1:
case 1:



pl2 = pl + 1;
ps2 = ps;
while (*pl2 == *ps2) {
if (*pl2 == NUL) 
return score + SCORE_DEL;
++pl2;
++ps2;
}


if (pl2[0] == ps2[1] && pl2[1] == ps2[0]
&& STRCMP(pl2 + 2, ps2 + 2) == 0)
return score + SCORE_DEL + SCORE_SWAP;


if (STRCMP(pl2 + 1, ps2 + 1) == 0)
return score + SCORE_DEL + SCORE_SUBST;


if (pl[0] == ps[1] && pl[1] == ps[0]) {
pl2 = pl + 2; 
ps2 = ps + 2;
while (*pl2 == *ps2) {
++pl2;
++ps2;
}

if (STRCMP(pl2 + 1, ps2) == 0)
return score + SCORE_SWAP + SCORE_DEL;
}


pl2 = pl + 1; 
ps2 = ps + 1;
while (*pl2 == *ps2) {
++pl2;
++ps2;
}

if (STRCMP(pl2 + 1, ps2) == 0)
return score + SCORE_SUBST + SCORE_DEL;


break;

case 0:



if (*pl == NUL)
return score;


if (pl[0] == ps[1] && pl[1] == ps[0]) {
pl2 = pl + 2; 
ps2 = ps + 2;
while (*pl2 == *ps2) {
if (*pl2 == NUL) 
return score + SCORE_SWAP;
++pl2;
++ps2;
}

if (pl2[0] == ps2[1] && pl2[1] == ps2[0]
&& STRCMP(pl2 + 2, ps2 + 2) == 0)
return score + SCORE_SWAP + SCORE_SWAP;


if (STRCMP(pl2 + 1, ps2 + 1) == 0)
return score + SCORE_SWAP + SCORE_SUBST;
}


pl2 = pl + 1;
ps2 = ps + 1;
while (*pl2 == *ps2) {
if (*pl2 == NUL) 
return score + SCORE_SUBST;
++pl2;
++ps2;
}


if (pl2[0] == ps2[1] && pl2[1] == ps2[0]
&& STRCMP(pl2 + 2, ps2 + 2) == 0)
return score + SCORE_SUBST + SCORE_SWAP;


if (STRCMP(pl2 + 1, ps2 + 1) == 0)
return score + SCORE_SUBST + SCORE_SUBST;


pl2 = pl;
ps2 = ps + 1;
while (*pl2 == *ps2) {
++pl2;
++ps2;
}
if (STRCMP(pl2 + 1, ps2) == 0)
return score + SCORE_INS + SCORE_DEL;


pl2 = pl + 1;
ps2 = ps;
while (*pl2 == *ps2) {
++pl2;
++ps2;
}
if (STRCMP(pl2, ps2 + 1) == 0)
return score + SCORE_INS + SCORE_DEL;


break;
}

return SCORE_MAXMAX;
}








static int spell_edit_score(slang_T *slang, char_u *badword, char_u *goodword)
{
int *cnt;
int j, i;
int t;
int bc, gc;
int pbc, pgc;
int wbadword[MAXWLEN];
int wgoodword[MAXWLEN];
const bool l_has_mbyte = has_mbyte;


int badlen;
int goodlen;
if (l_has_mbyte) {


badlen = 0;
for (const char_u *p = badword; *p != NUL; ) {
wbadword[badlen++] = mb_cptr2char_adv(&p);
}
wbadword[badlen++] = 0;
goodlen = 0;
for (const char_u *p = goodword; *p != NUL; ) {
wgoodword[goodlen++] = mb_cptr2char_adv(&p);
}
wgoodword[goodlen++] = 0;
} else {
badlen = (int)STRLEN(badword) + 1;
goodlen = (int)STRLEN(goodword) + 1;
}


#define CNT(a, b) cnt[(a) + (b) * (badlen + 1)]
cnt = xmalloc(sizeof(int) * (badlen + 1) * (goodlen + 1));

CNT(0, 0) = 0;
for (j = 1; j <= goodlen; ++j)
CNT(0, j) = CNT(0, j - 1) + SCORE_INS;

for (i = 1; i <= badlen; ++i) {
CNT(i, 0) = CNT(i - 1, 0) + SCORE_DEL;
for (j = 1; j <= goodlen; ++j) {
if (l_has_mbyte) {
bc = wbadword[i - 1];
gc = wgoodword[j - 1];
} else {
bc = badword[i - 1];
gc = goodword[j - 1];
}
if (bc == gc)
CNT(i, j) = CNT(i - 1, j - 1);
else {

if (SPELL_TOFOLD(bc) == SPELL_TOFOLD(gc))
CNT(i, j) = SCORE_ICASE + CNT(i - 1, j - 1);
else {

if (slang != NULL
&& slang->sl_has_map
&& similar_chars(slang, gc, bc))
CNT(i, j) = SCORE_SIMILAR + CNT(i - 1, j - 1);
else
CNT(i, j) = SCORE_SUBST + CNT(i - 1, j - 1);
}

if (i > 1 && j > 1) {
if (l_has_mbyte) {
pbc = wbadword[i - 2];
pgc = wgoodword[j - 2];
} else {
pbc = badword[i - 2];
pgc = goodword[j - 2];
}
if (bc == pgc && pbc == gc) {
t = SCORE_SWAP + CNT(i - 2, j - 2);
if (t < CNT(i, j))
CNT(i, j) = t;
}
}
t = SCORE_DEL + CNT(i - 1, j);
if (t < CNT(i, j))
CNT(i, j) = t;
t = SCORE_INS + CNT(i, j - 1);
if (t < CNT(i, j))
CNT(i, j) = t;
}
}
}

i = CNT(badlen - 1, goodlen - 1);
xfree(cnt);
return i;
}







static int spell_edit_score_limit(slang_T *slang, char_u *badword, char_u *goodword, int limit)
{
limitscore_T stack[10]; 
int stackidx;
int bi, gi;
int bi2, gi2;
int bc, gc;
int score;
int score_off;
int minscore;
int round;



if (has_mbyte)
return spell_edit_score_limit_w(slang, badword, goodword, limit);








stackidx = 0;
bi = 0;
gi = 0;
score = 0;
minscore = limit + 1;

for (;; ) {

for (;; ) {
bc = badword[bi];
gc = goodword[gi];
if (bc != gc) 
break;
if (bc == NUL) { 
if (score < minscore)
minscore = score;
goto pop; 
}
++bi;
++gi;
}

if (gc == NUL) { 
do {
if ((score += SCORE_DEL) >= minscore)
goto pop; 
} while (badword[++bi] != NUL);
minscore = score;
} else if (bc == NUL) { 
do {
if ((score += SCORE_INS) >= minscore)
goto pop; 
} while (goodword[++gi] != NUL);
minscore = score;
} else { 




for (round = 0; round <= 1; ++round) {
score_off = score + (round == 0 ? SCORE_DEL : SCORE_INS);
if (score_off < minscore) {
if (score_off + SCORE_EDIT_MIN >= minscore) {



bi2 = bi + 1 - round;
gi2 = gi + round;
while (goodword[gi2] == badword[bi2]) {
if (goodword[gi2] == NUL) {
minscore = score_off;
break;
}
++bi2;
++gi2;
}
} else {

stack[stackidx].badi = bi + 1 - round;
stack[stackidx].goodi = gi + round;
stack[stackidx].score = score_off;
++stackidx;
}
}
}

if (score + SCORE_SWAP < minscore) {



if (gc == badword[bi + 1] && bc == goodword[gi + 1]) {

gi += 2;
bi += 2;
score += SCORE_SWAP;
continue;
}
}




if (SPELL_TOFOLD(bc) == SPELL_TOFOLD(gc))
score += SCORE_ICASE;
else {

if (slang != NULL
&& slang->sl_has_map
&& similar_chars(slang, gc, bc))
score += SCORE_SIMILAR;
else
score += SCORE_SUBST;
}

if (score < minscore) {

++gi;
++bi;
continue;
}
}
pop:

if (stackidx == 0) 
break;


--stackidx;
gi = stack[stackidx].goodi;
bi = stack[stackidx].badi;
score = stack[stackidx].score;
}




if (minscore > limit)
return SCORE_MAXMAX;
return minscore;
}



static int spell_edit_score_limit_w(slang_T *slang, char_u *badword, char_u *goodword, int limit)
{
limitscore_T stack[10]; 
int stackidx;
int bi, gi;
int bi2, gi2;
int bc, gc;
int score;
int score_off;
int minscore;
int round;
int wbadword[MAXWLEN];
int wgoodword[MAXWLEN];



bi = 0;
for (const char_u *p = badword; *p != NUL; ) {
wbadword[bi++] = mb_cptr2char_adv(&p);
}
wbadword[bi++] = 0;
gi = 0;
for (const char_u *p = goodword; *p != NUL; ) {
wgoodword[gi++] = mb_cptr2char_adv(&p);
}
wgoodword[gi++] = 0;








stackidx = 0;
bi = 0;
gi = 0;
score = 0;
minscore = limit + 1;

for (;; ) {

for (;; ) {
bc = wbadword[bi];
gc = wgoodword[gi];

if (bc != gc) 
break;
if (bc == NUL) { 
if (score < minscore)
minscore = score;
goto pop; 
}
++bi;
++gi;
}

if (gc == NUL) { 
do {
if ((score += SCORE_DEL) >= minscore)
goto pop; 
} while (wbadword[++bi] != NUL);
minscore = score;
} else if (bc == NUL) { 
do {
if ((score += SCORE_INS) >= minscore)
goto pop; 
} while (wgoodword[++gi] != NUL);
minscore = score;
} else { 




for (round = 0; round <= 1; ++round) {
score_off = score + (round == 0 ? SCORE_DEL : SCORE_INS);
if (score_off < minscore) {
if (score_off + SCORE_EDIT_MIN >= minscore) {



bi2 = bi + 1 - round;
gi2 = gi + round;
while (wgoodword[gi2] == wbadword[bi2]) {
if (wgoodword[gi2] == NUL) {
minscore = score_off;
break;
}
++bi2;
++gi2;
}
} else {

stack[stackidx].badi = bi + 1 - round;
stack[stackidx].goodi = gi + round;
stack[stackidx].score = score_off;
++stackidx;
}
}
}

if (score + SCORE_SWAP < minscore) {



if (gc == wbadword[bi + 1] && bc == wgoodword[gi + 1]) {

gi += 2;
bi += 2;
score += SCORE_SWAP;
continue;
}
}




if (SPELL_TOFOLD(bc) == SPELL_TOFOLD(gc))
score += SCORE_ICASE;
else {

if (slang != NULL
&& slang->sl_has_map
&& similar_chars(slang, gc, bc))
score += SCORE_SIMILAR;
else
score += SCORE_SUBST;
}

if (score < minscore) {

++gi;
++bi;
continue;
}
}
pop:

if (stackidx == 0) 
break;


--stackidx;
gi = stack[stackidx].goodi;
bi = stack[stackidx].badi;
score = stack[stackidx].score;
}




if (minscore > limit)
return SCORE_MAXMAX;
return minscore;
}


void ex_spellinfo(exarg_T *eap)
{
if (no_spell_checking(curwin)) {
return;
}

msg_start();
for (int lpi = 0; lpi < curwin->w_s->b_langp.ga_len && !got_int; lpi++) {
langp_T *const lp = LANGP_ENTRY(curwin->w_s->b_langp, lpi);
msg_puts("file: ");
msg_puts((const char *)lp->lp_slang->sl_fname);
msg_putchar('\n');
const char *const p = (const char *)lp->lp_slang->sl_info;
if (p != NULL) {
msg_puts(p);
msg_putchar('\n');
}
}
msg_end();
}

#define DUMPFLAG_KEEPCASE 1 
#define DUMPFLAG_COUNT 2 
#define DUMPFLAG_ICASE 4 
#define DUMPFLAG_ONECAP 8 
#define DUMPFLAG_ALLCAP 16 


void ex_spelldump(exarg_T *eap)
{
char_u *spl;
long dummy;

if (no_spell_checking(curwin)) {
return;
}
get_option_value((char_u *)"spl", &dummy, &spl, OPT_LOCAL);


do_cmdline_cmd("new");


set_option_value("spell", true, "", OPT_LOCAL);
set_option_value("spl", dummy, (char *)spl, OPT_LOCAL);
xfree(spl);

if (!BUFEMPTY()) {
return;
}

spell_dump_compl(NULL, 0, NULL, eap->forceit ? DUMPFLAG_COUNT : 0);


if (curbuf->b_ml.ml_line_count > 1) {
ml_delete(curbuf->b_ml.ml_line_count, false);
}
redraw_later(NOT_VALID);
}





void
spell_dump_compl (
char_u *pat, 
int ic, 
int *dir, 
int dumpflags_arg 
)
{
langp_T *lp;
slang_T *slang;
idx_T arridx[MAXWLEN];
int curi[MAXWLEN];
char_u word[MAXWLEN];
int c;
char_u *byts;
idx_T *idxs;
linenr_T lnum = 0;
int round;
int depth;
int n;
int flags;
char_u *region_names = NULL; 
bool do_region = true; 
char_u *p;
int dumpflags = dumpflags_arg;
int patlen;



if (pat != NULL) {
if (ic)
dumpflags |= DUMPFLAG_ICASE;
else {
n = captype(pat, NULL);
if (n == WF_ONECAP)
dumpflags |= DUMPFLAG_ONECAP;
else if (n == WF_ALLCAP
&& (int)STRLEN(pat) > mb_ptr2len(pat)
)
dumpflags |= DUMPFLAG_ALLCAP;
}
}



for (int lpi = 0; lpi < curwin->w_s->b_langp.ga_len; ++lpi) {
lp = LANGP_ENTRY(curwin->w_s->b_langp, lpi);
p = lp->lp_slang->sl_regions;
if (p[0] != 0) {
if (region_names == NULL) 
region_names = p;
else if (STRCMP(region_names, p) != 0) {
do_region = false; 
break;
}
}
}

if (do_region && region_names != NULL) {
if (pat == NULL) {
vim_snprintf((char *)IObuff, IOSIZE, "/regions=%s", region_names);
ml_append(lnum++, IObuff, (colnr_T)0, false);
}
} else
do_region = false;


for (int lpi = 0; lpi < curwin->w_s->b_langp.ga_len; ++lpi) {
lp = LANGP_ENTRY(curwin->w_s->b_langp, lpi);
slang = lp->lp_slang;
if (slang->sl_fbyts == NULL) 
continue;

if (pat == NULL) {
vim_snprintf((char *)IObuff, IOSIZE, "#file: %s", slang->sl_fname);
ml_append(lnum++, IObuff, (colnr_T)0, false);
}



if (pat != NULL && slang->sl_pbyts == NULL)
patlen = (int)STRLEN(pat);
else
patlen = -1;



for (round = 1; round <= 2; ++round) {
if (round == 1) {
dumpflags &= ~DUMPFLAG_KEEPCASE;
byts = slang->sl_fbyts;
idxs = slang->sl_fidxs;
} else {
dumpflags |= DUMPFLAG_KEEPCASE;
byts = slang->sl_kbyts;
idxs = slang->sl_kidxs;
}
if (byts == NULL)
continue; 

depth = 0;
arridx[0] = 0;
curi[0] = 1;
while (depth >= 0 && !got_int
&& (pat == NULL || !compl_interrupted)) {
if (curi[depth] > byts[arridx[depth]]) {

--depth;
line_breakcheck();
ins_compl_check_keys(50, false);
} else {

n = arridx[depth] + curi[depth];
++curi[depth];
c = byts[n];
if (c == 0) {




flags = (int)idxs[n];
if ((round == 2 || (flags & WF_KEEPCAP) == 0)
&& (flags & WF_NEEDCOMP) == 0
&& (do_region
|| (flags & WF_REGION) == 0
|| (((unsigned)flags >> 16)
& lp->lp_region) != 0)) {
word[depth] = NUL;
if (!do_region)
flags &= ~WF_REGION;



c = (unsigned)flags >> 24;
if (c == 0 || curi[depth] == 2) {
dump_word(slang, word, pat, dir,
dumpflags, flags, lnum);
if (pat == NULL)
++lnum;
}


if (c != 0)
lnum = dump_prefixes(slang, word, pat, dir,
dumpflags, flags, lnum);
}
} else {

word[depth++] = c;
arridx[depth] = idxs[n];
curi[depth] = 1;







assert(depth >= 0);
if (depth <= patlen
&& mb_strnicmp(word, pat, (size_t)depth) != 0)
--depth;
}
}
}
}
}
}



static void dump_word(slang_T *slang, char_u *word, char_u *pat, int *dir, int dumpflags, int wordflags, linenr_T lnum)
{
bool keepcap = false;
char_u *p;
char_u *tw;
char_u cword[MAXWLEN];
char_u badword[MAXWLEN + 10];
int i;
int flags = wordflags;

if (dumpflags & DUMPFLAG_ONECAP)
flags |= WF_ONECAP;
if (dumpflags & DUMPFLAG_ALLCAP)
flags |= WF_ALLCAP;

if ((dumpflags & DUMPFLAG_KEEPCASE) == 0 && (flags & WF_CAPMASK) != 0) {

make_case_word(word, cword, flags);
p = cword;
} else {
p = word;
if ((dumpflags & DUMPFLAG_KEEPCASE)
&& ((captype(word, NULL) & WF_KEEPCAP) == 0
|| (flags & WF_FIXCAP) != 0))
keepcap = true;
}
tw = p;

if (pat == NULL) {

if ((flags & (WF_BANNED | WF_RARE | WF_REGION)) || keepcap) {
STRCPY(badword, p);
STRCAT(badword, "/");
if (keepcap) {
STRCAT(badword, "=");
}
if (flags & WF_BANNED) {
STRCAT(badword, "!");
} else if (flags & WF_RARE) {
STRCAT(badword, "?");
}
if (flags & WF_REGION) {
for (i = 0; i < 7; i++) {
if (flags & (0x10000 << i)) {
const size_t badword_len = STRLEN(badword);
snprintf((char *)badword + badword_len,
sizeof(badword) - badword_len,
"%d", i + 1);
}
}
}
p = badword;
}

if (dumpflags & DUMPFLAG_COUNT) {
hashitem_T *hi;


hi = hash_find(&slang->sl_wordcount, tw);
if (!HASHITEM_EMPTY(hi)) {
vim_snprintf((char *)IObuff, IOSIZE, "%s\t%d",
tw, HI2WC(hi)->wc_count);
p = IObuff;
}
}

ml_append(lnum, p, (colnr_T)0, false);
} else if (((dumpflags & DUMPFLAG_ICASE)
? mb_strnicmp(p, pat, STRLEN(pat)) == 0
: STRNCMP(p, pat, STRLEN(pat)) == 0)
&& ins_compl_add_infercase(p, (int)STRLEN(p),
p_ic, NULL, *dir, false) == OK) {

*dir = FORWARD;
}
}





static linenr_T
dump_prefixes (
slang_T *slang,
char_u *word, 
char_u *pat,
int *dir,
int dumpflags,
int flags, 
linenr_T startlnum
)
{
idx_T arridx[MAXWLEN];
int curi[MAXWLEN];
char_u prefix[MAXWLEN];
char_u word_up[MAXWLEN];
bool has_word_up = false;
int c;
char_u *byts;
idx_T *idxs;
linenr_T lnum = startlnum;
int depth;
int n;
int len;
int i;



c = PTR2CHAR(word);
if (SPELL_TOUPPER(c) != c) {
onecap_copy(word, word_up, true);
has_word_up = true;
}

byts = slang->sl_pbyts;
idxs = slang->sl_pidxs;
if (byts != NULL) { 


depth = 0;
arridx[0] = 0;
curi[0] = 1;
while (depth >= 0 && !got_int) {
n = arridx[depth];
len = byts[n];
if (curi[depth] > len) {

--depth;
line_breakcheck();
} else {

n += curi[depth];
++curi[depth];
c = byts[n];
if (c == 0) {

for (i = 1; i < len; ++i)
if (byts[n + i] != 0)
break;
curi[depth] += i - 1;

c = valid_word_prefix(i, n, flags, word, slang, false);
if (c != 0) {
STRLCPY(prefix + depth, word, MAXWLEN - depth);
dump_word(slang, prefix, pat, dir, dumpflags,
(c & WF_RAREPFX) ? (flags | WF_RARE)
: flags, lnum);
if (lnum != 0)
++lnum;
}




if (has_word_up) {
c = valid_word_prefix(i, n, flags, word_up, slang,
true);
if (c != 0) {
STRLCPY(prefix + depth, word_up, MAXWLEN - depth);
dump_word(slang, prefix, pat, dir, dumpflags,
(c & WF_RAREPFX) ? (flags | WF_RARE)
: flags, lnum);
if (lnum != 0)
++lnum;
}
}
} else {

prefix[depth++] = c;
arridx[depth] = idxs[n];
curi[depth] = 1;
}
}
}
}

return lnum;
}



char_u *spell_to_word_end(char_u *start, win_T *win)
{
char_u *p = start;

while (*p != NUL && spell_iswordp(p, win)) {
MB_PTR_ADV(p);
}
return p;
}






int spell_word_start(int startcol)
{
char_u *line;
char_u *p;
int col = 0;

if (no_spell_checking(curwin)) {
return startcol;
}


line = get_cursor_line_ptr();
for (p = line + startcol; p > line; ) {
MB_PTR_BACK(line, p);
if (spell_iswordp_nmw(p, curwin)) {
break;
}
}


while (p > line) {
col = (int)(p - line);
MB_PTR_BACK(line, p);
if (!spell_iswordp(p, curwin)) {
break;
}
col = 0;
}

return col;
}



static bool spell_expand_need_cap;

void spell_expand_check_cap(colnr_T col)
{
spell_expand_need_cap = check_need_cap(curwin->w_cursor.lnum, col);
}





int expand_spelling(linenr_T lnum, char_u *pat, char_u ***matchp)
{
garray_T ga;

spell_suggest_list(&ga, pat, 100, spell_expand_need_cap, true);
*matchp = ga.ga_data;
return ga.ga_len;
}
