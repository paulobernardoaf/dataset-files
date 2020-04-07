#include <stdio.h>
#include <stdint.h>
#include <wctype.h>
#include "nvim/vim.h"
#include "nvim/spell_defs.h"
#include "nvim/ascii.h"
#include "nvim/buffer.h"
#include "nvim/charset.h"
#include "nvim/ex_cmds2.h"
#include "nvim/fileio.h"
#include "nvim/memory.h"
#include "nvim/memline.h"
#include "nvim/misc1.h"
#include "nvim/option.h"
#include "nvim/os/os.h"
#include "nvim/path.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/spell.h"
#include "nvim/spellfile.h"
#include "nvim/ui.h"
#include "nvim/undo.h"
#if !defined(UNIX)
#include <time.h> 
#endif
#define BY_NOFLAGS 0 
#define BY_INDEX 1 
#define BY_FLAGS 2 
#define BY_FLAGS2 3 
#define BY_SPECIAL BY_FLAGS2 
#define ZERO_FLAG 65009 
#define SAL_F0LLOWUP 1
#define SAL_COLLAPSE 2
#define SAL_REM_ACCENTS 4
#define VIMSPELLMAGIC "VIMspell" 
#define VIMSPELLMAGICL (sizeof(VIMSPELLMAGIC) - 1)
#define VIMSPELLVERSION 50
#define SN_REGION 0 
#define SN_CHARFLAGS 1 
#define SN_MIDWORD 2 
#define SN_PREFCOND 3 
#define SN_REP 4 
#define SN_SAL 5 
#define SN_SOFO 6 
#define SN_MAP 7 
#define SN_COMPOUND 8 
#define SN_SYLLABLE 9 
#define SN_NOBREAK 10 
#define SN_SUGFILE 11 
#define SN_REPSAL 12 
#define SN_WORDS 13 
#define SN_NOSPLITSUGS 14 
#define SN_INFO 15 
#define SN_NOCOMPOUNDSUGS 16 
#define SN_END 255 
#define SNF_REQUIRED 1 
#define CF_WORD 0x01
#define CF_UPPER 0x02
static char *e_spell_trunc = N_("E758: Truncated spell file");
static char *e_afftrailing = N_("Trailing text in %s line %d: %s");
static char *e_affname = N_("Affix name too long in %s line %d: %s");
static char *msg_compressing = N_("Compressing word tree...");
#define MAXLINELEN 500 
typedef struct afffile_S {
char_u *af_enc; 
int af_flagtype; 
unsigned af_rare; 
unsigned af_keepcase; 
unsigned af_bad; 
unsigned af_needaffix; 
unsigned af_circumfix; 
unsigned af_needcomp; 
unsigned af_comproot; 
unsigned af_compforbid; 
unsigned af_comppermit; 
unsigned af_nosuggest; 
int af_pfxpostpone; 
bool af_ignoreextra; 
hashtab_T af_pref; 
hashtab_T af_suff; 
hashtab_T af_comp; 
} afffile_T;
#define AFT_CHAR 0 
#define AFT_LONG 1 
#define AFT_CAPLONG 2 
#define AFT_NUM 3 
typedef struct affentry_S affentry_T;
struct affentry_S {
affentry_T *ae_next; 
char_u *ae_chop; 
char_u *ae_add; 
char_u *ae_flags; 
char_u *ae_cond; 
regprog_T *ae_prog; 
char ae_compforbid; 
char ae_comppermit; 
};
#define AH_KEY_LEN 17 
typedef struct affheader_S {
char_u ah_key[AH_KEY_LEN]; 
unsigned ah_flag; 
int ah_newID; 
int ah_combine; 
int ah_follows; 
affentry_T *ah_first; 
} affheader_T;
#define HI2AH(hi) ((affheader_T *)(hi)->hi_key)
typedef struct compitem_S {
char_u ci_key[AH_KEY_LEN]; 
unsigned ci_flag; 
int ci_newID; 
} compitem_T;
#define HI2CI(hi) ((compitem_T *)(hi)->hi_key)
#define SBLOCKSIZE 16000 
typedef struct sblock_S sblock_T;
struct sblock_S {
int sb_used; 
sblock_T *sb_next; 
char_u sb_data[1]; 
};
typedef struct wordnode_S wordnode_T;
struct wordnode_S {
union { 
char_u hashkey[6]; 
int index; 
} wn_u1;
union { 
wordnode_T *next; 
wordnode_T *wnode; 
} wn_u2;
wordnode_T *wn_child; 
wordnode_T *wn_sibling; 
int wn_refs; 
char_u wn_byte; 
char_u wn_affixID; 
uint16_t wn_flags; 
short wn_region; 
#if defined(SPELL_PRINTTREE)
int wn_nr; 
#endif
};
#define WN_MASK 0xffff 
#define HI2WN(hi) (wordnode_T *)((hi)->hi_key)
typedef struct spellinfo_S {
wordnode_T *si_foldroot; 
long si_foldwcount; 
wordnode_T *si_keeproot; 
long si_keepwcount; 
wordnode_T *si_prefroot; 
long si_sugtree; 
sblock_T *si_blocks; 
long si_blocks_cnt; 
int si_did_emsg; 
long si_compress_cnt; 
wordnode_T *si_first_free; 
long si_free_count; 
#if defined(SPELL_PRINTTREE)
int si_wordnode_nr; 
#endif
buf_T *si_spellbuf; 
int si_ascii; 
int si_add; 
int si_clear_chartab; 
int si_region; 
vimconv_T si_conv; 
int si_memtot; 
int si_verbose; 
int si_msg_count; 
char_u *si_info; 
int si_region_count; 
char_u si_region_name[MAXREGIONS * 2 + 1];
garray_T si_rep; 
garray_T si_repsal; 
garray_T si_sal; 
char_u *si_sofofr; 
char_u *si_sofoto; 
int si_nosugfile; 
int si_nosplitsugs; 
int si_nocompoundsugs; 
int si_followup; 
int si_collapse; 
hashtab_T si_commonwords; 
time_t si_sugtime; 
int si_rem_accents; 
garray_T si_map; 
char_u *si_midword; 
int si_compmax; 
int si_compminlen; 
int si_compsylmax; 
int si_compoptions; 
garray_T si_comppat; 
char_u *si_compflags; 
char_u si_nobreak; 
char_u *si_syllable; 
garray_T si_prefcond; 
int si_newprefID; 
int si_newcompID; 
} spellinfo_T;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "spellfile.c.generated.h"
#endif
#define SPELL_READ_BYTES(buf, n, fd, exit_code) do { const size_t n__SPRB = (n); FILE *const fd__SPRB = (fd); char *const buf__SPRB = (buf); const size_t read_bytes__SPRB = fread(buf__SPRB, 1, n__SPRB, fd__SPRB); if (read_bytes__SPRB != n__SPRB) { exit_code; return feof(fd__SPRB) ? SP_TRUNCERROR : SP_OTHERERROR; } } while (0)
#define SPELL_READ_NONNUL_BYTES(buf, n, fd, exit_code) do { const size_t n__SPRNB = (n); FILE *const fd__SPRNB = (fd); char *const buf__SPRNB = (buf); SPELL_READ_BYTES(buf__SPRNB, n__SPRNB, fd__SPRNB, exit_code); if (memchr(buf__SPRNB, NUL, (size_t)n__SPRNB)) { exit_code; return SP_FORMERROR; } } while (0)
static inline int spell_check_magic_string(FILE *const fd)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_ALWAYS_INLINE
{
char buf[VIMSPELLMAGICL];
SPELL_READ_BYTES(buf, VIMSPELLMAGICL, fd, ;);
if (memcmp(buf, VIMSPELLMAGIC, VIMSPELLMAGICL) != 0) {
return SP_FORMERROR;
}
return 0;
}
slang_T *
spell_load_file (
char_u *fname,
char_u *lang,
slang_T *old_lp,
bool silent 
)
{
FILE *fd;
char_u *p;
int n;
int len;
char_u *save_sourcing_name = sourcing_name;
linenr_T save_sourcing_lnum = sourcing_lnum;
slang_T *lp = NULL;
int c = 0;
int res;
fd = os_fopen((char *)fname, "r");
if (fd == NULL) {
if (!silent)
EMSG2(_(e_notopen), fname);
else if (p_verbose > 2) {
verbose_enter();
smsg((char *)e_notopen, fname);
verbose_leave();
}
goto endFAIL;
}
if (p_verbose > 2) {
verbose_enter();
smsg(_("Reading spell file \"%s\""), fname);
verbose_leave();
}
if (old_lp == NULL) {
lp = slang_alloc(lang);
lp->sl_fname = vim_strsave(fname);
lp->sl_add = strstr((char *)path_tail(fname), SPL_FNAME_ADD) != NULL;
} else
lp = old_lp;
sourcing_name = fname;
sourcing_lnum = 0;
const int scms_ret = spell_check_magic_string(fd);
switch (scms_ret) {
case SP_FORMERROR:
case SP_TRUNCERROR: {
emsgf("%s", _("E757: This does not look like a spell file"));
goto endFAIL;
}
case SP_OTHERERROR: {
emsgf(_("E5042: Failed to read spell file %s: %s"),
fname, strerror(ferror(fd)));
}
case 0: {
break;
}
}
c = getc(fd); 
if (c < VIMSPELLVERSION) {
EMSG(_("E771: Old spell file, needs to be updated"));
goto endFAIL;
} else if (c > VIMSPELLVERSION) {
EMSG(_("E772: Spell file is for newer version of Vim"));
goto endFAIL;
}
for (;; ) {
n = getc(fd); 
if (n == SN_END)
break;
c = getc(fd); 
len = get4c(fd); 
if (len < 0)
goto truncerr;
res = 0;
switch (n) {
case SN_INFO:
lp->sl_info = READ_STRING(fd, len); 
if (lp->sl_info == NULL)
goto endFAIL;
break;
case SN_REGION:
res = read_region_section(fd, lp, len);
break;
case SN_CHARFLAGS:
res = read_charflags_section(fd);
break;
case SN_MIDWORD:
lp->sl_midword = READ_STRING(fd, len); 
if (lp->sl_midword == NULL)
goto endFAIL;
break;
case SN_PREFCOND:
res = read_prefcond_section(fd, lp);
break;
case SN_REP:
res = read_rep_section(fd, &lp->sl_rep, lp->sl_rep_first);
break;
case SN_REPSAL:
res = read_rep_section(fd, &lp->sl_repsal, lp->sl_repsal_first);
break;
case SN_SAL:
res = read_sal_section(fd, lp);
break;
case SN_SOFO:
res = read_sofo_section(fd, lp);
break;
case SN_MAP:
p = READ_STRING(fd, len); 
if (p == NULL)
goto endFAIL;
set_map_str(lp, p);
xfree(p);
break;
case SN_WORDS:
res = read_words_section(fd, lp, len);
break;
case SN_SUGFILE:
lp->sl_sugtime = get8ctime(fd); 
break;
case SN_NOSPLITSUGS:
lp->sl_nosplitsugs = true;
break;
case SN_NOCOMPOUNDSUGS:
lp->sl_nocompoundsugs = true;
break;
case SN_COMPOUND:
res = read_compound(fd, lp, len);
break;
case SN_NOBREAK:
lp->sl_nobreak = true;
break;
case SN_SYLLABLE:
lp->sl_syllable = READ_STRING(fd, len); 
if (lp->sl_syllable == NULL)
goto endFAIL;
if (init_syl_tab(lp) == FAIL)
goto endFAIL;
break;
default:
if (c & SNF_REQUIRED) {
EMSG(_("E770: Unsupported section in spell file"));
goto endFAIL;
}
while (--len >= 0)
if (getc(fd) < 0)
goto truncerr;
break;
}
someerror:
if (res == SP_FORMERROR) {
EMSG(_(e_format));
goto endFAIL;
}
if (res == SP_TRUNCERROR) {
truncerr:
EMSG(_(e_spell_trunc));
goto endFAIL;
}
if (res == SP_OTHERERROR)
goto endFAIL;
}
res = spell_read_tree(fd, &lp->sl_fbyts, &lp->sl_fidxs, false, 0);
if (res != 0)
goto someerror;
res = spell_read_tree(fd, &lp->sl_kbyts, &lp->sl_kidxs, false, 0);
if (res != 0)
goto someerror;
res = spell_read_tree(fd, &lp->sl_pbyts, &lp->sl_pidxs, true,
lp->sl_prefixcnt);
if (res != 0)
goto someerror;
if (old_lp == NULL && lang != NULL) {
lp->sl_next = first_lang;
first_lang = lp;
}
goto endOK;
endFAIL:
if (lang != NULL)
*lang = NUL;
if (lp != NULL && old_lp == NULL)
slang_free(lp);
lp = NULL;
endOK:
if (fd != NULL)
fclose(fd);
sourcing_name = save_sourcing_name;
sourcing_lnum = save_sourcing_lnum;
return lp;
}
static void tree_count_words(char_u *byts, idx_T *idxs)
{
int depth;
idx_T arridx[MAXWLEN];
int curi[MAXWLEN];
int c;
idx_T n;
int wordcount[MAXWLEN];
arridx[0] = 0;
curi[0] = 1;
wordcount[0] = 0;
depth = 0;
while (depth >= 0 && !got_int) {
if (curi[depth] > byts[arridx[depth]]) {
idxs[arridx[depth]] = wordcount[depth];
if (depth > 0)
wordcount[depth - 1] += wordcount[depth];
--depth;
fast_breakcheck();
} else {
n = arridx[depth] + curi[depth];
++curi[depth];
c = byts[n];
if (c == 0) {
++wordcount[depth];
while (byts[n + 1] == 0) {
++n;
++curi[depth];
}
} else {
++depth;
arridx[depth] = idxs[n];
curi[depth] = 1;
wordcount[depth] = 0;
}
}
}
}
void suggest_load_files(void)
{
langp_T *lp;
slang_T *slang;
char_u *dotp;
FILE *fd;
char_u buf[MAXWLEN];
int i;
time_t timestamp;
int wcount;
int wordnr;
garray_T ga;
int c;
for (int lpi = 0; lpi < curwin->w_s->b_langp.ga_len; ++lpi) {
lp = LANGP_ENTRY(curwin->w_s->b_langp, lpi);
slang = lp->lp_slang;
if (slang->sl_sugtime != 0 && !slang->sl_sugloaded) {
slang->sl_sugloaded = true;
dotp = STRRCHR(slang->sl_fname, '.');
if (dotp == NULL || fnamecmp(dotp, ".spl") != 0) {
continue;
}
STRCPY(dotp, ".sug");
fd = os_fopen((char *)slang->sl_fname, "r");
if (fd == NULL) {
goto nextone;
}
for (i = 0; i < VIMSUGMAGICL; ++i)
buf[i] = getc(fd); 
if (STRNCMP(buf, VIMSUGMAGIC, VIMSUGMAGICL) != 0) {
EMSG2(_("E778: This does not look like a .sug file: %s"),
slang->sl_fname);
goto nextone;
}
c = getc(fd); 
if (c < VIMSUGVERSION) {
EMSG2(_("E779: Old .sug file, needs to be updated: %s"),
slang->sl_fname);
goto nextone;
} else if (c > VIMSUGVERSION) {
EMSG2(_("E780: .sug file is for newer version of Vim: %s"),
slang->sl_fname);
goto nextone;
}
timestamp = get8ctime(fd); 
if (timestamp != slang->sl_sugtime) {
EMSG2(_("E781: .sug file doesn't match .spl file: %s"),
slang->sl_fname);
goto nextone;
}
if (spell_read_tree(fd, &slang->sl_sbyts, &slang->sl_sidxs,
false, 0) != 0) {
someerror:
EMSG2(_("E782: error while reading .sug file: %s"),
slang->sl_fname);
slang_clear_sug(slang);
goto nextone;
}
slang->sl_sugbuf = open_spellbuf();
wcount = get4c(fd);
if (wcount < 0)
goto someerror;
ga_init(&ga, 1, 100);
for (wordnr = 0; wordnr < wcount; ++wordnr) {
ga.ga_len = 0;
for (;; ) {
c = getc(fd); 
if (c < 0) {
goto someerror;
}
GA_APPEND(char_u, &ga, c);
if (c == NUL)
break;
}
if (ml_append_buf(slang->sl_sugbuf, (linenr_T)wordnr,
ga.ga_data, ga.ga_len, true) == FAIL) {
goto someerror;
}
}
ga_clear(&ga);
tree_count_words(slang->sl_fbyts, slang->sl_fidxs);
tree_count_words(slang->sl_sbyts, slang->sl_sidxs);
nextone:
if (fd != NULL)
fclose(fd);
STRCPY(dotp, ".spl");
}
}
}
static char_u *read_cnt_string(FILE *fd, int cnt_bytes, int *cntp)
{
int cnt = 0;
int i;
char_u *str;
for (i = 0; i < cnt_bytes; ++i)
cnt = (cnt << 8) + getc(fd);
if (cnt < 0) {
*cntp = SP_TRUNCERROR;
return NULL;
}
*cntp = cnt;
if (cnt == 0)
return NULL; 
str = READ_STRING(fd, cnt);
if (str == NULL)
*cntp = SP_OTHERERROR;
return str;
}
static int read_region_section(FILE *fd, slang_T *lp, int len)
{
if (len > MAXREGIONS * 2) {
return SP_FORMERROR;
}
SPELL_READ_NONNUL_BYTES((char *)lp->sl_regions, (size_t)len, fd, ;);
lp->sl_regions[len] = NUL;
return 0;
}
static int read_charflags_section(FILE *fd)
{
char_u *flags;
char_u *fol;
int flagslen, follen;
flags = read_cnt_string(fd, 1, &flagslen);
if (flagslen < 0)
return flagslen;
fol = read_cnt_string(fd, 2, &follen);
if (follen < 0) {
xfree(flags);
return follen;
}
if (flags != NULL && fol != NULL)
set_spell_charflags(flags, flagslen, fol);
xfree(flags);
xfree(fol);
if ((flags == NULL) != (fol == NULL))
return SP_FORMERROR;
return 0;
}
static int read_prefcond_section(FILE *fd, slang_T *lp)
{
const int cnt = get2c(fd); 
if (cnt <= 0) {
return SP_FORMERROR;
}
lp->sl_prefprog = xcalloc(cnt, sizeof(regprog_T *));
lp->sl_prefixcnt = cnt;
for (int i = 0; i < cnt; i++) {
const int n = getc(fd); 
if (n < 0 || n >= MAXWLEN) {
return SP_FORMERROR;
}
if (n > 0) {
char buf[MAXWLEN + 1];
buf[0] = '^'; 
SPELL_READ_NONNUL_BYTES(buf + 1, (size_t)n, fd, ;);
buf[n + 1] = NUL;
lp->sl_prefprog[i] = vim_regcomp((char_u *)buf, RE_MAGIC | RE_STRING);
}
}
return 0;
}
static int read_rep_section(FILE *fd, garray_T *gap, int16_t *first)
{
int cnt;
fromto_T *ftp;
cnt = get2c(fd); 
if (cnt < 0)
return SP_TRUNCERROR;
ga_grow(gap, cnt);
for (; gap->ga_len < cnt; ++gap->ga_len) {
int c;
ftp = &((fromto_T *)gap->ga_data)[gap->ga_len];
ftp->ft_from = read_cnt_string(fd, 1, &c);
if (c < 0)
return c;
if (c == 0)
return SP_FORMERROR;
ftp->ft_to = read_cnt_string(fd, 1, &c);
if (c <= 0) {
xfree(ftp->ft_from);
if (c < 0)
return c;
return SP_FORMERROR;
}
}
for (int i = 0; i < 256; ++i) {
first[i] = -1;
}
for (int i = 0; i < gap->ga_len; ++i) {
ftp = &((fromto_T *)gap->ga_data)[i];
if (first[*ftp->ft_from] == -1)
first[*ftp->ft_from] = i;
}
return 0;
}
static int read_sal_section(FILE *fd, slang_T *slang)
{
int cnt;
garray_T *gap;
salitem_T *smp;
int ccnt;
char_u *p;
int c = NUL;
slang->sl_sofo = false;
const int flags = getc(fd); 
if (flags & SAL_F0LLOWUP) {
slang->sl_followup = true;
}
if (flags & SAL_COLLAPSE) {
slang->sl_collapse = true;
}
if (flags & SAL_REM_ACCENTS) {
slang->sl_rem_accents = true;
}
cnt = get2c(fd); 
if (cnt < 0)
return SP_TRUNCERROR;
gap = &slang->sl_sal;
ga_init(gap, sizeof(salitem_T), 10);
ga_grow(gap, cnt + 1);
for (; gap->ga_len < cnt; ++gap->ga_len) {
smp = &((salitem_T *)gap->ga_data)[gap->ga_len];
ccnt = getc(fd); 
if (ccnt < 0)
return SP_TRUNCERROR;
p = xmalloc(ccnt + 2);
smp->sm_lead = p;
int i = 0;
for (; i < ccnt; ++i) {
c = getc(fd); 
if (vim_strchr((char_u *)"0123456789(-<^$", c) != NULL)
break;
*p++ = c;
}
smp->sm_leadlen = (int)(p - smp->sm_lead);
*p++ = NUL;
if (c == '(') {
smp->sm_oneof = p;
for (++i; i < ccnt; ++i) {
c = getc(fd); 
if (c == ')')
break;
*p++ = c;
}
*p++ = NUL;
if (++i < ccnt)
c = getc(fd);
} else
smp->sm_oneof = NULL;
smp->sm_rules = p;
if (i < ccnt) {
*p++ = c;
}
i++;
if (i < ccnt) {
SPELL_READ_NONNUL_BYTES( 
(char *)p, (size_t)(ccnt - i), fd, xfree(smp->sm_lead));
p += (ccnt - i);
}
*p++ = NUL;
smp->sm_to = read_cnt_string(fd, 1, &ccnt);
if (ccnt < 0) {
xfree(smp->sm_lead);
return ccnt;
}
if (has_mbyte) {
smp->sm_lead_w = mb_str2wide(smp->sm_lead);
smp->sm_leadlen = mb_charlen(smp->sm_lead);
if (smp->sm_oneof == NULL)
smp->sm_oneof_w = NULL;
else
smp->sm_oneof_w = mb_str2wide(smp->sm_oneof);
if (smp->sm_to == NULL)
smp->sm_to_w = NULL;
else
smp->sm_to_w = mb_str2wide(smp->sm_to);
}
}
if (!GA_EMPTY(gap)) {
smp = &((salitem_T *)gap->ga_data)[gap->ga_len];
p = xmalloc(1);
p[0] = NUL;
smp->sm_lead = p;
smp->sm_lead_w = mb_str2wide(smp->sm_lead);
smp->sm_leadlen = 0;
smp->sm_oneof = NULL;
smp->sm_oneof_w = NULL;
smp->sm_rules = p;
smp->sm_to = NULL;
smp->sm_to_w = NULL;
gap->ga_len++;
}
set_sal_first(slang);
return 0;
}
static int read_words_section(FILE *fd, slang_T *lp, int len)
{
int done = 0;
int i;
int c;
char_u word[MAXWLEN];
while (done < len) {
for (i = 0;; ++i) {
c = getc(fd);
if (c == EOF)
return SP_TRUNCERROR;
word[i] = c;
if (word[i] == NUL)
break;
if (i == MAXWLEN - 1)
return SP_FORMERROR;
}
count_common_word(lp, word, -1, 10);
done += i + 1;
}
return 0;
}
static int read_sofo_section(FILE *fd, slang_T *slang)
{
int cnt;
char_u *from, *to;
int res;
slang->sl_sofo = true;
from = read_cnt_string(fd, 2, &cnt);
if (cnt < 0)
return cnt;
to = read_cnt_string(fd, 2, &cnt);
if (cnt < 0) {
xfree(from);
return cnt;
}
if (from != NULL && to != NULL)
res = set_sofo(slang, from, to);
else if (from != NULL || to != NULL)
res = SP_FORMERROR; 
else
res = 0;
xfree(from);
xfree(to);
return res;
}
static int read_compound(FILE *fd, slang_T *slang, int len)
{
int todo = len;
int c;
int atstart;
char_u *pat;
char_u *pp;
char_u *cp;
char_u *ap;
char_u *crp;
int cnt;
garray_T *gap;
if (todo < 2)
return SP_FORMERROR; 
--todo;
c = getc(fd); 
if (c < 2)
c = MAXWLEN;
slang->sl_compmax = c;
--todo;
c = getc(fd); 
if (c < 1)
c = 0;
slang->sl_compminlen = c;
--todo;
c = getc(fd); 
if (c < 1)
c = MAXWLEN;
slang->sl_compsylmax = c;
c = getc(fd); 
if (c != 0)
ungetc(c, fd); 
else {
--todo;
c = getc(fd); 
--todo;
slang->sl_compoptions = c;
gap = &slang->sl_comppat;
c = get2c(fd); 
todo -= 2;
ga_init(gap, sizeof(char_u *), c);
ga_grow(gap, c);
while (--c >= 0) {
((char_u **)(gap->ga_data))[gap->ga_len++] =
read_cnt_string(fd, 1, &cnt);
if (cnt < 0)
return cnt;
todo -= cnt + 1;
}
}
if (todo < 0)
return SP_FORMERROR;
c = todo * 2 + 7;
c += todo * 2;
pat = xmalloc(c);
cp = xmalloc(todo + 1);
slang->sl_compstartflags = cp;
*cp = NUL;
ap = xmalloc(todo + 1);
slang->sl_compallflags = ap;
*ap = NUL;
crp = xmalloc(todo + 1);
slang->sl_comprules = crp;
pp = pat;
*pp++ = '^';
*pp++ = '\\';
*pp++ = '(';
atstart = 1;
while (todo-- > 0) {
c = getc(fd); 
if (c == EOF) {
xfree(pat);
return SP_TRUNCERROR;
}
if (vim_strchr((char_u *)"?*+[]/", c) == NULL
&& !byte_in_str(slang->sl_compallflags, c)) {
*ap++ = c;
*ap = NUL;
}
if (atstart != 0) {
if (c == '[')
atstart = 2;
else if (c == ']')
atstart = 0;
else {
if (!byte_in_str(slang->sl_compstartflags, c)) {
*cp++ = c;
*cp = NUL;
}
if (atstart == 1)
atstart = 0;
}
}
if (crp != NULL) {
if (c == '?' || c == '+' || c == '*') {
XFREE_CLEAR(slang->sl_comprules);
crp = NULL;
} else
*crp++ = c;
}
if (c == '/') { 
*pp++ = '\\';
*pp++ = '|';
atstart = 1;
} else { 
if (c == '?' || c == '+' || c == '~') {
*pp++ = '\\'; 
}
pp += utf_char2bytes(c, pp);
}
}
*pp++ = '\\';
*pp++ = ')';
*pp++ = '$';
*pp = NUL;
if (crp != NULL)
*crp = NUL;
slang->sl_compprog = vim_regcomp(pat, RE_MAGIC + RE_STRING + RE_STRICT);
xfree(pat);
if (slang->sl_compprog == NULL)
return SP_FORMERROR;
return 0;
}
static int set_sofo(slang_T *lp, char_u *from, char_u *to)
{
int i;
garray_T *gap;
char_u *s;
char_u *p;
int c;
int *inp;
if (has_mbyte) {
gap = &lp->sl_sal;
ga_init(gap, sizeof(int *), 1);
ga_grow(gap, 256);
memset(gap->ga_data, 0, sizeof(int *) * 256);
gap->ga_len = 256;
for (p = from, s = to; *p != NUL && *s != NUL; ) {
c = mb_cptr2char_adv((const char_u **)&p);
MB_CPTR_ADV(s);
if (c >= 256) {
lp->sl_sal_first[c & 0xff]++;
}
}
if (*p != NUL || *s != NUL) 
return SP_FORMERROR;
for (i = 0; i < 256; ++i)
if (lp->sl_sal_first[i] > 0) {
p = xmalloc(sizeof(int) * (lp->sl_sal_first[i] * 2 + 1));
((int **)gap->ga_data)[i] = (int *)p;
*(int *)p = 0;
}
memset(lp->sl_sal_first, 0, sizeof(salfirst_T) * 256);
for (p = from, s = to; *p != NUL && *s != NUL; ) {
c = mb_cptr2char_adv((const char_u **)&p);
i = mb_cptr2char_adv((const char_u **)&s);
if (c >= 256) {
inp = ((int **)gap->ga_data)[c & 0xff];
while (*inp != 0)
++inp;
*inp++ = c; 
*inp++ = i; 
*inp++ = NUL; 
} else
lp->sl_sal_first[c] = i;
}
} else {
if (STRLEN(from) != STRLEN(to))
return SP_FORMERROR;
for (i = 0; to[i] != NUL; ++i)
lp->sl_sal_first[from[i]] = to[i];
lp->sl_sal.ga_len = 1; 
}
return 0;
}
static void set_sal_first(slang_T *lp)
{
salfirst_T *sfirst;
salitem_T *smp;
int c;
garray_T *gap = &lp->sl_sal;
sfirst = lp->sl_sal_first;
for (int i = 0; i < 256; ++i) {
sfirst[i] = -1;
}
smp = (salitem_T *)gap->ga_data;
for (int i = 0; i < gap->ga_len; ++i) {
if (has_mbyte)
c = *smp[i].sm_lead_w & 0xff;
else
c = *smp[i].sm_lead;
if (sfirst[c] == -1) {
sfirst[c] = i;
if (has_mbyte) {
int n;
while (i + 1 < gap->ga_len
&& (*smp[i + 1].sm_lead_w & 0xff) == c)
++i;
for (n = 1; i + n < gap->ga_len; ++n)
if ((*smp[i + n].sm_lead_w & 0xff) == c) {
salitem_T tsal;
++i;
--n;
tsal = smp[i + n];
memmove(smp + i + 1, smp + i,
sizeof(salitem_T) * n);
smp[i] = tsal;
}
}
}
}
}
static int *mb_str2wide(char_u *s)
{
int i = 0;
int *res = xmalloc((mb_charlen(s) + 1) * sizeof(int));
for (char_u *p = s; *p != NUL; ) {
res[i++] = mb_ptr2char_adv((const char_u **)&p);
}
res[i] = NUL;
return res;
}
static int
spell_read_tree (
FILE *fd,
char_u **bytsp,
idx_T **idxsp,
bool prefixtree, 
int prefixcnt 
)
{
int idx;
char_u *bp;
idx_T *ip;
long len = get4c(fd);
if (len < 0) {
return SP_TRUNCERROR;
}
if ((size_t)len >= SIZE_MAX / sizeof(int)) { 
return SP_FORMERROR;
}
if (len > 0) {
bp = xmalloc(len);
*bytsp = bp;
ip = xcalloc(len, sizeof(*ip));
*idxsp = ip;
idx = read_tree_node(fd, bp, ip, len, 0, prefixtree, prefixcnt);
if (idx < 0)
return idx;
}
return 0;
}
static idx_T
read_tree_node (
FILE *fd,
char_u *byts,
idx_T *idxs,
int maxidx, 
idx_T startidx, 
bool prefixtree, 
int maxprefcondnr 
)
{
int len;
int i;
int n;
idx_T idx = startidx;
int c;
int c2;
#define SHARED_MASK 0x8000000
len = getc(fd); 
if (len <= 0)
return SP_TRUNCERROR;
if (startidx + len >= maxidx)
return SP_FORMERROR;
byts[idx++] = len;
for (i = 1; i <= len; ++i) {
c = getc(fd); 
if (c < 0)
return SP_TRUNCERROR;
if (c <= BY_SPECIAL) {
if (c == BY_NOFLAGS && !prefixtree) {
idxs[idx] = 0;
} else if (c != BY_INDEX) {
if (prefixtree) {
if (c == BY_FLAGS)
c = getc(fd) << 24; 
else
c = 0;
c |= getc(fd); 
n = get2c(fd); 
if (n >= maxprefcondnr)
return SP_FORMERROR;
c |= (n << 8);
} else { 
c2 = c;
c = getc(fd); 
if (c2 == BY_FLAGS2)
c = (getc(fd) << 8) + c; 
if (c & WF_REGION)
c = (getc(fd) << 16) + c; 
if (c & WF_AFX)
c = (getc(fd) << 24) + c; 
}
idxs[idx] = c;
c = 0;
} else { 
n = get3c(fd);
if (n < 0 || n >= maxidx)
return SP_FORMERROR;
idxs[idx] = n + SHARED_MASK;
c = getc(fd); 
}
}
byts[idx++] = c;
}
for (i = 1; i <= len; ++i)
if (byts[startidx + i] != 0) {
if (idxs[startidx + i] & SHARED_MASK)
idxs[startidx + i] &= ~SHARED_MASK;
else {
idxs[startidx + i] = idx;
idx = read_tree_node(fd, byts, idxs, maxidx, idx,
prefixtree, maxprefcondnr);
if (idx < 0)
break;
}
}
return idx;
}
static void
spell_reload_one (
char_u *fname,
bool added_word 
)
{
slang_T *slang;
bool didit = false;
for (slang = first_lang; slang != NULL; slang = slang->sl_next) {
if (path_full_compare(fname, slang->sl_fname, false, true) == kEqualFiles) {
slang_clear(slang);
if (spell_load_file(fname, NULL, slang, false) == NULL)
slang_clear(slang);
redraw_all_later(SOME_VALID);
didit = true;
}
}
if (added_word && !didit)
did_set_spelllang(curwin);
}
#define PFX_FLAGS -256
#define CONDIT_COMB 1 
#define CONDIT_CFIX 2 
#define CONDIT_SUF 4 
#define CONDIT_AFF 8 
static long compress_start = 30000; 
static long compress_inc = 100; 
static long compress_added = 500000; 
int spell_check_msm(void)
{
char_u *p = p_msm;
long start = 0;
long incr = 0;
long added = 0;
if (!ascii_isdigit(*p))
return FAIL;
start = (getdigits_long(&p, true, 0) * 10) / (SBLOCKSIZE / 102);
if (*p != ',') {
return FAIL;
}
p++;
if (!ascii_isdigit(*p)) {
return FAIL;
}
incr = (getdigits_long(&p, true, 0) * 102) / (SBLOCKSIZE / 10);
if (*p != ',') {
return FAIL;
}
p++;
if (!ascii_isdigit(*p)) {
return FAIL;
}
added = getdigits_long(&p, true, 0) * 1024;
if (*p != NUL) {
return FAIL;
}
if (start == 0 || incr == 0 || added == 0 || incr > start) {
return FAIL;
}
compress_start = start;
compress_inc = incr;
compress_added = added;
return OK;
}
#if defined(SPELL_PRINTTREE)
#define PRINTLINESIZE 1000
#define PRINTWIDTH 6
#define PRINTSOME(l, depth, fmt, a1, a2) vim_snprintf(l + depth * PRINTWIDTH, PRINTLINESIZE - PRINTWIDTH * depth, fmt, a1, a2)
static char line1[PRINTLINESIZE];
static char line2[PRINTLINESIZE];
static char line3[PRINTLINESIZE];
static void spell_clear_flags(wordnode_T *node)
{
wordnode_T *np;
for (np = node; np != NULL; np = np->wn_sibling) {
np->wn_u1.index = FALSE;
spell_clear_flags(np->wn_child);
}
}
static void spell_print_node(wordnode_T *node, int depth)
{
if (node->wn_u1.index) {
PRINTSOME(line1, depth, "(%d)", node->wn_nr, 0);
PRINTSOME(line2, depth, " ", 0, 0);
PRINTSOME(line3, depth, " ", 0, 0);
msg((char_u *)line1);
msg((char_u *)line2);
msg((char_u *)line3);
} else {
node->wn_u1.index = TRUE;
if (node->wn_byte != NUL) {
if (node->wn_child != NULL)
PRINTSOME(line1, depth, " %c -> ", node->wn_byte, 0);
else
PRINTSOME(line1, depth, " %c ???", node->wn_byte, 0);
} else
PRINTSOME(line1, depth, " $ ", 0, 0);
PRINTSOME(line2, depth, "%d/%d ", node->wn_nr, node->wn_refs);
if (node->wn_sibling != NULL)
PRINTSOME(line3, depth, " | ", 0, 0);
else
PRINTSOME(line3, depth, " ", 0, 0);
if (node->wn_byte == NUL) {
msg((char_u *)line1);
msg((char_u *)line2);
msg((char_u *)line3);
}
if (node->wn_byte != NUL && node->wn_child != NULL)
spell_print_node(node->wn_child, depth + 1);
if (node->wn_sibling != NULL) {
STRCPY(line1, line3);
STRCPY(line2, line3);
spell_print_node(node->wn_sibling, depth);
}
}
}
static void spell_print_tree(wordnode_T *root)
{
if (root != NULL) {
spell_clear_flags(root);
spell_print_node(root, 0);
}
}
#endif 
static afffile_T *spell_read_aff(spellinfo_T *spin, char_u *fname)
{
FILE *fd;
char_u rline[MAXLINELEN];
char_u *line;
char_u *pc = NULL;
#define MAXITEMCNT 30
char_u *(items[MAXITEMCNT]);
int itemcnt;
char_u *p;
int lnum = 0;
affheader_T *cur_aff = NULL;
bool did_postpone_prefix = false;
int aff_todo = 0;
hashtab_T *tp;
char_u *low = NULL;
char_u *fol = NULL;
char_u *upp = NULL;
int do_rep;
int do_repsal;
int do_sal;
int do_mapline;
bool found_map = false;
hashitem_T *hi;
int l;
int compminlen = 0; 
int compsylmax = 0; 
int compoptions = 0; 
int compmax = 0; 
char_u *compflags = NULL; 
char_u *midword = NULL; 
char_u *syllable = NULL; 
char_u *sofofrom = NULL; 
char_u *sofoto = NULL; 
fd = os_fopen((char *)fname, "r");
if (fd == NULL) {
EMSG2(_(e_notopen), fname);
return NULL;
}
vim_snprintf((char *)IObuff, IOSIZE, _("Reading affix file %s..."), fname);
spell_message(spin, IObuff);
do_rep = GA_EMPTY(&spin->si_rep);
do_repsal = GA_EMPTY(&spin->si_repsal);
do_sal = GA_EMPTY(&spin->si_sal);
do_mapline = GA_EMPTY(&spin->si_map);
afffile_T *aff = getroom(spin, sizeof(*aff), true);
hash_init(&aff->af_pref);
hash_init(&aff->af_suff);
hash_init(&aff->af_comp);
while (!vim_fgets(rline, MAXLINELEN, fd) && !got_int) {
line_breakcheck();
++lnum;
if (*rline == '#')
continue;
xfree(pc);
if (spin->si_conv.vc_type != CONV_NONE) {
pc = string_convert(&spin->si_conv, rline, NULL);
if (pc == NULL) {
smsg(_("Conversion failure for word in %s line %d: %s"),
fname, lnum, rline);
continue;
}
line = pc;
} else {
pc = NULL;
line = rline;
}
itemcnt = 0;
for (p = line;; ) {
while (*p != NUL && *p <= ' ') 
++p;
if (*p == NUL)
break;
if (itemcnt == MAXITEMCNT) 
break;
items[itemcnt++] = p;
if (itemcnt == 2 && spell_info_item(items[0]))
while (*p >= ' ' || *p == TAB) 
++p;
else
while (*p > ' ') 
++p;
if (*p == NUL)
break;
*p++ = NUL;
}
if (itemcnt > 0) {
if (is_aff_rule(items, itemcnt, "SET", 2) && aff->af_enc == NULL) {
aff->af_enc = enc_canonize(items[1]);
if (!spin->si_ascii
&& convert_setup(&spin->si_conv, aff->af_enc,
p_enc) == FAIL)
smsg(_("Conversion in %s not supported: from %s to %s"),
fname, aff->af_enc, p_enc);
spin->si_conv.vc_fail = true;
} else if (is_aff_rule(items, itemcnt, "FLAG", 2)
&& aff->af_flagtype == AFT_CHAR) {
if (STRCMP(items[1], "long") == 0)
aff->af_flagtype = AFT_LONG;
else if (STRCMP(items[1], "num") == 0)
aff->af_flagtype = AFT_NUM;
else if (STRCMP(items[1], "caplong") == 0)
aff->af_flagtype = AFT_CAPLONG;
else
smsg(_("Invalid value for FLAG in %s line %d: %s"),
fname, lnum, items[1]);
if (aff->af_rare != 0
|| aff->af_keepcase != 0
|| aff->af_bad != 0
|| aff->af_needaffix != 0
|| aff->af_circumfix != 0
|| aff->af_needcomp != 0
|| aff->af_comproot != 0
|| aff->af_nosuggest != 0
|| compflags != NULL
|| aff->af_suff.ht_used > 0
|| aff->af_pref.ht_used > 0)
smsg(_("FLAG after using flags in %s line %d: %s"),
fname, lnum, items[1]);
} else if (spell_info_item(items[0]) && itemcnt > 1) {
p = getroom(spin,
(spin->si_info == NULL ? 0 : STRLEN(spin->si_info))
+ STRLEN(items[0])
+ STRLEN(items[1]) + 3, false);
if (spin->si_info != NULL) {
STRCPY(p, spin->si_info);
STRCAT(p, "\n");
}
STRCAT(p, items[0]);
STRCAT(p, " ");
STRCAT(p, items[1]);
spin->si_info = p;
} else if (is_aff_rule(items, itemcnt, "MIDWORD", 2)
&& midword == NULL) {
midword = getroom_save(spin, items[1]);
} else if (is_aff_rule(items, itemcnt, "TRY", 2)) {
}
else if ((is_aff_rule(items, itemcnt, "RAR", 2)
|| is_aff_rule(items, itemcnt, "RARE", 2))
&& aff->af_rare == 0) {
aff->af_rare = affitem2flag(aff->af_flagtype, items[1],
fname, lnum);
}
else if ((is_aff_rule(items, itemcnt, "KEP", 2)
|| is_aff_rule(items, itemcnt, "KEEPCASE", 2))
&& aff->af_keepcase == 0) {
aff->af_keepcase = affitem2flag(aff->af_flagtype, items[1],
fname, lnum);
} else if ((is_aff_rule(items, itemcnt, "BAD", 2)
|| is_aff_rule(items, itemcnt, "FORBIDDENWORD", 2))
&& aff->af_bad == 0) {
aff->af_bad = affitem2flag(aff->af_flagtype, items[1],
fname, lnum);
} else if (is_aff_rule(items, itemcnt, "NEEDAFFIX", 2)
&& aff->af_needaffix == 0) {
aff->af_needaffix = affitem2flag(aff->af_flagtype, items[1],
fname, lnum);
} else if (is_aff_rule(items, itemcnt, "CIRCUMFIX", 2)
&& aff->af_circumfix == 0) {
aff->af_circumfix = affitem2flag(aff->af_flagtype, items[1],
fname, lnum);
} else if (is_aff_rule(items, itemcnt, "NOSUGGEST", 2)
&& aff->af_nosuggest == 0) {
aff->af_nosuggest = affitem2flag(aff->af_flagtype, items[1],
fname, lnum);
} else if ((is_aff_rule(items, itemcnt, "NEEDCOMPOUND", 2)
|| is_aff_rule(items, itemcnt, "ONLYINCOMPOUND", 2))
&& aff->af_needcomp == 0) {
aff->af_needcomp = affitem2flag(aff->af_flagtype, items[1],
fname, lnum);
} else if (is_aff_rule(items, itemcnt, "COMPOUNDROOT", 2)
&& aff->af_comproot == 0) {
aff->af_comproot = affitem2flag(aff->af_flagtype, items[1],
fname, lnum);
} else if (is_aff_rule(items, itemcnt, "COMPOUNDFORBIDFLAG", 2)
&& aff->af_compforbid == 0) {
aff->af_compforbid = affitem2flag(aff->af_flagtype, items[1],
fname, lnum);
if (aff->af_pref.ht_used > 0)
smsg(_("Defining COMPOUNDFORBIDFLAG after PFX item may give wrong results in %s line %d"),
fname, lnum);
} else if (is_aff_rule(items, itemcnt, "COMPOUNDPERMITFLAG", 2)
&& aff->af_comppermit == 0) {
aff->af_comppermit = affitem2flag(aff->af_flagtype, items[1],
fname, lnum);
if (aff->af_pref.ht_used > 0)
smsg(_("Defining COMPOUNDPERMITFLAG after PFX item may give wrong results in %s line %d"),
fname, lnum);
} else if (is_aff_rule(items, itemcnt, "COMPOUNDFLAG", 2)
&& compflags == NULL) {
p = getroom(spin, STRLEN(items[1]) + 2, false);
STRCPY(p, items[1]);
STRCAT(p, "+");
compflags = p;
} else if (is_aff_rule(items, itemcnt, "COMPOUNDRULES", 2)) {
if (atoi((char *)items[1]) == 0)
smsg(_("Wrong COMPOUNDRULES value in %s line %d: %s"),
fname, lnum, items[1]);
} else if (is_aff_rule(items, itemcnt, "COMPOUNDRULE", 2)) {
if (compflags != NULL || *skipdigits(items[1]) != NUL) {
l = (int)STRLEN(items[1]) + 1;
if (compflags != NULL)
l += (int)STRLEN(compflags) + 1;
p = getroom(spin, l, false);
if (compflags != NULL) {
STRCPY(p, compflags);
STRCAT(p, "/");
}
STRCAT(p, items[1]);
compflags = p;
}
} else if (is_aff_rule(items, itemcnt, "COMPOUNDWORDMAX", 2)
&& compmax == 0) {
compmax = atoi((char *)items[1]);
if (compmax == 0)
smsg(_("Wrong COMPOUNDWORDMAX value in %s line %d: %s"),
fname, lnum, items[1]);
} else if (is_aff_rule(items, itemcnt, "COMPOUNDMIN", 2)
&& compminlen == 0) {
compminlen = atoi((char *)items[1]);
if (compminlen == 0)
smsg(_("Wrong COMPOUNDMIN value in %s line %d: %s"),
fname, lnum, items[1]);
} else if (is_aff_rule(items, itemcnt, "COMPOUNDSYLMAX", 2)
&& compsylmax == 0) {
compsylmax = atoi((char *)items[1]);
if (compsylmax == 0)
smsg(_("Wrong COMPOUNDSYLMAX value in %s line %d: %s"),
fname, lnum, items[1]);
} else if (is_aff_rule(items, itemcnt, "CHECKCOMPOUNDDUP", 1)) {
compoptions |= COMP_CHECKDUP;
} else if (is_aff_rule(items, itemcnt, "CHECKCOMPOUNDREP", 1)) {
compoptions |= COMP_CHECKREP;
} else if (is_aff_rule(items, itemcnt, "CHECKCOMPOUNDCASE", 1)) {
compoptions |= COMP_CHECKCASE;
} else if (is_aff_rule(items, itemcnt, "CHECKCOMPOUNDTRIPLE", 1)) {
compoptions |= COMP_CHECKTRIPLE;
} else if (is_aff_rule(items, itemcnt, "CHECKCOMPOUNDPATTERN", 2)) {
if (atoi((char *)items[1]) == 0)
smsg(_("Wrong CHECKCOMPOUNDPATTERN value in %s line %d: %s"),
fname, lnum, items[1]);
} else if (is_aff_rule(items, itemcnt, "CHECKCOMPOUNDPATTERN", 3)) {
garray_T *gap = &spin->si_comppat;
int i;
for (i = 0; i < gap->ga_len - 1; i += 2)
if (STRCMP(((char_u **)(gap->ga_data))[i], items[1]) == 0
&& STRCMP(((char_u **)(gap->ga_data))[i + 1],
items[2]) == 0)
break;
if (i >= gap->ga_len) {
ga_grow(gap, 2);
((char_u **)(gap->ga_data))[gap->ga_len++]
= getroom_save(spin, items[1]);
((char_u **)(gap->ga_data))[gap->ga_len++]
= getroom_save(spin, items[2]);
}
} else if (is_aff_rule(items, itemcnt, "SYLLABLE", 2)
&& syllable == NULL) {
syllable = getroom_save(spin, items[1]);
} else if (is_aff_rule(items, itemcnt, "NOBREAK", 1)) {
spin->si_nobreak = true;
} else if (is_aff_rule(items, itemcnt, "NOSPLITSUGS", 1)) {
spin->si_nosplitsugs = true;
} else if (is_aff_rule(items, itemcnt, "NOCOMPOUNDSUGS", 1)) {
spin->si_nocompoundsugs = true;
} else if (is_aff_rule(items, itemcnt, "NOSUGFILE", 1)) {
spin->si_nosugfile = true;
} else if (is_aff_rule(items, itemcnt, "PFXPOSTPONE", 1)) {
aff->af_pfxpostpone = true;
} else if (is_aff_rule(items, itemcnt, "IGNOREEXTRA", 1)) {
aff->af_ignoreextra = true;
} else if ((STRCMP(items[0], "PFX") == 0
|| STRCMP(items[0], "SFX") == 0)
&& aff_todo == 0
&& itemcnt >= 4) {
int lasti = 4;
char_u key[AH_KEY_LEN];
if (*items[0] == 'P')
tp = &aff->af_pref;
else
tp = &aff->af_suff;
STRLCPY(key, items[1], AH_KEY_LEN);
hi = hash_find(tp, key);
if (!HASHITEM_EMPTY(hi)) {
cur_aff = HI2AH(hi);
if (cur_aff->ah_combine != (*items[2] == 'Y'))
smsg(_("Different combining flag in continued affix block in %s line %d: %s"),
fname, lnum, items[1]);
if (!cur_aff->ah_follows)
smsg(_("Duplicate affix in %s line %d: %s"),
fname, lnum, items[1]);
} else {
cur_aff = getroom(spin, sizeof(*cur_aff), true);
cur_aff->ah_flag = affitem2flag(aff->af_flagtype, items[1],
fname, lnum);
if (cur_aff->ah_flag == 0 || STRLEN(items[1]) >= AH_KEY_LEN) {
break;
}
if (cur_aff->ah_flag == aff->af_bad
|| cur_aff->ah_flag == aff->af_rare
|| cur_aff->ah_flag == aff->af_keepcase
|| cur_aff->ah_flag == aff->af_needaffix
|| cur_aff->ah_flag == aff->af_circumfix
|| cur_aff->ah_flag == aff->af_nosuggest
|| cur_aff->ah_flag == aff->af_needcomp
|| cur_aff->ah_flag == aff->af_comproot) {
smsg(_("Affix also used for "
"BAD/RARE/KEEPCASE/NEEDAFFIX/NEEDCOMPOUND/NOSUGGEST"
"in %s line %d: %s"),
fname, lnum, items[1]);
}
STRCPY(cur_aff->ah_key, items[1]);
hash_add(tp, cur_aff->ah_key);
cur_aff->ah_combine = (*items[2] == 'Y');
}
if (itemcnt > lasti && STRCMP(items[lasti], "S") == 0) {
++lasti;
cur_aff->ah_follows = true;
} else
cur_aff->ah_follows = false;
if (itemcnt > lasti
&& !aff->af_ignoreextra
&& *items[lasti] != '#')
smsg(_(e_afftrailing), fname, lnum, items[lasti]);
if (STRCMP(items[2], "Y") != 0 && STRCMP(items[2], "N") != 0)
smsg(_("Expected Y or N in %s line %d: %s"),
fname, lnum, items[2]);
if (*items[0] == 'P' && aff->af_pfxpostpone) {
if (cur_aff->ah_newID == 0) {
check_renumber(spin);
cur_aff->ah_newID = ++spin->si_newprefID;
did_postpone_prefix = false;
} else
did_postpone_prefix = true;
}
aff_todo = atoi((char *)items[3]);
} else if ((STRCMP(items[0], "PFX") == 0
|| STRCMP(items[0], "SFX") == 0)
&& aff_todo > 0
&& STRCMP(cur_aff->ah_key, items[1]) == 0
&& itemcnt >= 5) {
affentry_T *aff_entry;
bool upper = false;
int lasti = 5;
if (itemcnt > lasti && *items[lasti] != '#'
&& (STRCMP(items[lasti], "-") != 0
|| itemcnt != lasti + 1))
smsg(_(e_afftrailing), fname, lnum, items[lasti]);
aff_todo--;
aff_entry = getroom(spin, sizeof(*aff_entry), true);
if (STRCMP(items[2], "0") != 0)
aff_entry->ae_chop = getroom_save(spin, items[2]);
if (STRCMP(items[3], "0") != 0) {
aff_entry->ae_add = getroom_save(spin, items[3]);
aff_entry->ae_flags = vim_strchr(aff_entry->ae_add, '/');
if (aff_entry->ae_flags != NULL) {
*aff_entry->ae_flags++ = NUL;
aff_process_flags(aff, aff_entry);
}
}
if (!spin->si_ascii || !(has_non_ascii(aff_entry->ae_chop)
|| has_non_ascii(aff_entry->ae_add))) {
aff_entry->ae_next = cur_aff->ah_first;
cur_aff->ah_first = aff_entry;
if (STRCMP(items[4], ".") != 0) {
char_u buf[MAXLINELEN];
aff_entry->ae_cond = getroom_save(spin, items[4]);
if (*items[0] == 'P')
sprintf((char *)buf, "^%s", items[4]);
else
sprintf((char *)buf, "%s$", items[4]);
aff_entry->ae_prog = vim_regcomp(buf,
RE_MAGIC + RE_STRING + RE_STRICT);
if (aff_entry->ae_prog == NULL)
smsg(_("Broken condition in %s line %d: %s"),
fname, lnum, items[4]);
}
if (*items[0] == 'P' && aff->af_pfxpostpone
&& aff_entry->ae_flags == NULL) {
if (aff_entry->ae_chop != NULL
&& aff_entry->ae_add != NULL
&& aff_entry->ae_chop[(*mb_ptr2len)(
aff_entry->ae_chop)] == NUL
) {
int c, c_up;
c = PTR2CHAR(aff_entry->ae_chop);
c_up = SPELL_TOUPPER(c);
if (c_up != c
&& (aff_entry->ae_cond == NULL
|| PTR2CHAR(aff_entry->ae_cond) == c)) {
p = aff_entry->ae_add
+ STRLEN(aff_entry->ae_add);
MB_PTR_BACK(aff_entry->ae_add, p);
if (PTR2CHAR(p) == c_up) {
upper = true;
aff_entry->ae_chop = NULL;
*p = NUL;
if (aff_entry->ae_cond != NULL) {
char_u buf[MAXLINELEN];
if (has_mbyte) {
onecap_copy(items[4], buf, true);
aff_entry->ae_cond = getroom_save(
spin, buf);
} else
*aff_entry->ae_cond = c_up;
if (aff_entry->ae_cond != NULL) {
sprintf((char *)buf, "^%s",
aff_entry->ae_cond);
vim_regfree(aff_entry->ae_prog);
aff_entry->ae_prog = vim_regcomp(
buf, RE_MAGIC + RE_STRING);
}
}
}
}
}
if (aff_entry->ae_chop == NULL) {
int idx;
char_u **pp;
int n;
for (idx = spin->si_prefcond.ga_len - 1; idx >= 0;
--idx) {
p = ((char_u **)spin->si_prefcond.ga_data)[idx];
if (str_equal(p, aff_entry->ae_cond))
break;
}
if (idx < 0) {
idx = spin->si_prefcond.ga_len;
pp = GA_APPEND_VIA_PTR(char_u *, &spin->si_prefcond);
*pp = (aff_entry->ae_cond == NULL) ?
NULL : getroom_save(spin, aff_entry->ae_cond);
}
if (aff_entry->ae_add == NULL)
p = (char_u *)"";
else
p = aff_entry->ae_add;
n = PFX_FLAGS;
if (!cur_aff->ah_combine)
n |= WFP_NC;
if (upper)
n |= WFP_UP;
if (aff_entry->ae_comppermit)
n |= WFP_COMPPERMIT;
if (aff_entry->ae_compforbid)
n |= WFP_COMPFORBID;
tree_add_word(spin, p, spin->si_prefroot, n,
idx, cur_aff->ah_newID);
did_postpone_prefix = true;
}
if (aff_todo == 0 && !did_postpone_prefix) {
--spin->si_newprefID;
cur_aff->ah_newID = 0;
}
}
}
} else if (is_aff_rule(items, itemcnt, "FOL", 2) && fol == NULL) {
fol = vim_strsave(items[1]);
} else if (is_aff_rule(items, itemcnt, "LOW", 2) && low == NULL) {
low = vim_strsave(items[1]);
} else if (is_aff_rule(items, itemcnt, "UPP", 2) && upp == NULL) {
upp = vim_strsave(items[1]);
} else if (is_aff_rule(items, itemcnt, "REP", 2)
|| is_aff_rule(items, itemcnt, "REPSAL", 2)) {
;
if (!isdigit(*items[1]))
smsg(_("Expected REP(SAL) count in %s line %d"),
fname, lnum);
} else if ((STRCMP(items[0], "REP") == 0
|| STRCMP(items[0], "REPSAL") == 0)
&& itemcnt >= 3) {
if (itemcnt > 3 && items[3][0] != '#')
smsg(_(e_afftrailing), fname, lnum, items[3]);
if (items[0][3] == 'S' ? do_repsal : do_rep) {
for (p = items[1]; *p != NUL; MB_PTR_ADV(p)) {
if (*p == '_') {
*p = ' ';
}
}
for (p = items[2]; *p != NUL; MB_PTR_ADV(p)) {
if (*p == '_') {
*p = ' ';
}
}
add_fromto(spin, items[0][3] == 'S'
? &spin->si_repsal
: &spin->si_rep, items[1], items[2]);
}
} else if (is_aff_rule(items, itemcnt, "MAP", 2)) {
if (!found_map) {
found_map = true;
if (!isdigit(*items[1]))
smsg(_("Expected MAP count in %s line %d"),
fname, lnum);
} else if (do_mapline) {
int c;
for (p = items[1]; *p != NUL; ) {
c = mb_ptr2char_adv((const char_u **)&p);
if ((!GA_EMPTY(&spin->si_map)
&& vim_strchr(spin->si_map.ga_data, c)
!= NULL)
|| vim_strchr(p, c) != NULL) {
smsg(_("Duplicate character in MAP in %s line %d"),
fname, lnum);
}
}
ga_concat(&spin->si_map, items[1]);
ga_append(&spin->si_map, '/');
}
}
else if (is_aff_rule(items, itemcnt, "SAL", 3)) {
if (do_sal) {
if (STRCMP(items[1], "followup") == 0)
spin->si_followup = sal_to_bool(items[2]);
else if (STRCMP(items[1], "collapse_result") == 0)
spin->si_collapse = sal_to_bool(items[2]);
else if (STRCMP(items[1], "remove_accents") == 0)
spin->si_rem_accents = sal_to_bool(items[2]);
else
add_fromto(spin, &spin->si_sal, items[1],
STRCMP(items[2], "_") == 0 ? (char_u *)""
: items[2]);
}
} else if (is_aff_rule(items, itemcnt, "SOFOFROM", 2)
&& sofofrom == NULL) {
sofofrom = getroom_save(spin, items[1]);
} else if (is_aff_rule(items, itemcnt, "SOFOTO", 2)
&& sofoto == NULL) {
sofoto = getroom_save(spin, items[1]);
} else if (STRCMP(items[0], "COMMON") == 0) {
int i;
for (i = 1; i < itemcnt; ++i) {
if (HASHITEM_EMPTY(hash_find(&spin->si_commonwords,
items[i]))) {
p = vim_strsave(items[i]);
hash_add(&spin->si_commonwords, p);
}
}
} else
smsg(_("Unrecognized or duplicate item in %s line %d: %s"),
fname, lnum, items[0]);
}
}
if (fol != NULL || low != NULL || upp != NULL) {
if (spin->si_clear_chartab) {
init_spell_chartab();
spin->si_clear_chartab = false;
}
xfree(fol);
xfree(low);
xfree(upp);
}
if (compmax != 0) {
aff_check_number(spin->si_compmax, compmax, "COMPOUNDWORDMAX");
spin->si_compmax = compmax;
}
if (compminlen != 0) {
aff_check_number(spin->si_compminlen, compminlen, "COMPOUNDMIN");
spin->si_compminlen = compminlen;
}
if (compsylmax != 0) {
if (syllable == NULL) {
smsg("%s", _("COMPOUNDSYLMAX used without SYLLABLE"));
}
aff_check_number(spin->si_compsylmax, compsylmax, "COMPOUNDSYLMAX");
spin->si_compsylmax = compsylmax;
}
if (compoptions != 0) {
aff_check_number(spin->si_compoptions, compoptions, "COMPOUND options");
spin->si_compoptions |= compoptions;
}
if (compflags != NULL)
process_compflags(spin, aff, compflags);
if (spin->si_newcompID < spin->si_newprefID) {
if (spin->si_newcompID == 127 || spin->si_newcompID == 255)
MSG(_("Too many postponed prefixes"));
else if (spin->si_newprefID == 0 || spin->si_newprefID == 127)
MSG(_("Too many compound flags"));
else
MSG(_("Too many postponed prefixes and/or compound flags"));
}
if (syllable != NULL) {
aff_check_string(spin->si_syllable, syllable, "SYLLABLE");
spin->si_syllable = syllable;
}
if (sofofrom != NULL || sofoto != NULL) {
if (sofofrom == NULL || sofoto == NULL)
smsg(_("Missing SOFO%s line in %s"),
sofofrom == NULL ? "FROM" : "TO", fname);
else if (!GA_EMPTY(&spin->si_sal))
smsg(_("Both SAL and SOFO lines in %s"), fname);
else {
aff_check_string(spin->si_sofofr, sofofrom, "SOFOFROM");
aff_check_string(spin->si_sofoto, sofoto, "SOFOTO");
spin->si_sofofr = sofofrom;
spin->si_sofoto = sofoto;
}
}
if (midword != NULL) {
aff_check_string(spin->si_midword, midword, "MIDWORD");
spin->si_midword = midword;
}
xfree(pc);
fclose(fd);
return aff;
}
static bool is_aff_rule(char_u **items, int itemcnt, char *rulename, int mincount)
{
return STRCMP(items[0], rulename) == 0
&& (itemcnt == mincount
|| (itemcnt > mincount && items[mincount][0] == '#'));
}
static void aff_process_flags(afffile_T *affile, affentry_T *entry)
{
char_u *p;
char_u *prevp;
unsigned flag;
if (entry->ae_flags != NULL
&& (affile->af_compforbid != 0 || affile->af_comppermit != 0)) {
for (p = entry->ae_flags; *p != NUL; ) {
prevp = p;
flag = get_affitem(affile->af_flagtype, &p);
if (flag == affile->af_comppermit || flag == affile->af_compforbid) {
STRMOVE(prevp, p);
p = prevp;
if (flag == affile->af_comppermit)
entry->ae_comppermit = true;
else
entry->ae_compforbid = true;
}
if (affile->af_flagtype == AFT_NUM && *p == ',')
++p;
}
if (*entry->ae_flags == NUL)
entry->ae_flags = NULL; 
}
}
static bool spell_info_item(char_u *s)
{
return STRCMP(s, "NAME") == 0
|| STRCMP(s, "HOME") == 0
|| STRCMP(s, "VERSION") == 0
|| STRCMP(s, "AUTHOR") == 0
|| STRCMP(s, "EMAIL") == 0
|| STRCMP(s, "COPYRIGHT") == 0;
}
static unsigned affitem2flag(int flagtype, char_u *item, char_u *fname, int lnum)
{
unsigned res;
char_u *p = item;
res = get_affitem(flagtype, &p);
if (res == 0) {
if (flagtype == AFT_NUM)
smsg(_("Flag is not a number in %s line %d: %s"),
fname, lnum, item);
else
smsg(_("Illegal flag in %s line %d: %s"),
fname, lnum, item);
}
if (*p != NUL) {
smsg(_(e_affname), fname, lnum, item);
return 0;
}
return res;
}
static unsigned get_affitem(int flagtype, char_u **pp)
{
int res;
if (flagtype == AFT_NUM) {
if (!ascii_isdigit(**pp)) {
++*pp; 
return 0;
}
res = getdigits_int(pp, true, 0);
if (res == 0) {
res = ZERO_FLAG;
}
} else {
res = mb_ptr2char_adv((const char_u **)pp);
if (flagtype == AFT_LONG || (flagtype == AFT_CAPLONG
&& res >= 'A' && res <= 'Z')) {
if (**pp == NUL)
return 0;
res = mb_ptr2char_adv((const char_u **)pp) + (res << 16);
}
}
return res;
}
static void process_compflags(spellinfo_T *spin, afffile_T *aff, char_u *compflags)
{
char_u *p;
char_u *prevp;
unsigned flag;
compitem_T *ci;
int id;
int len;
char_u *tp;
char_u key[AH_KEY_LEN];
hashitem_T *hi;
len = (int)STRLEN(compflags) + 1;
if (spin->si_compflags != NULL)
len += (int)STRLEN(spin->si_compflags) + 1;
p = getroom(spin, len, false);
if (spin->si_compflags != NULL) {
STRCPY(p, spin->si_compflags);
STRCAT(p, "/");
}
spin->si_compflags = p;
tp = p + STRLEN(p);
for (p = compflags; *p != NUL; ) {
if (vim_strchr((char_u *)"/?*+[]", *p) != NULL)
*tp++ = *p++;
else {
prevp = p;
flag = get_affitem(aff->af_flagtype, &p);
if (flag != 0) {
STRLCPY(key, prevp, p - prevp + 1);
hi = hash_find(&aff->af_comp, key);
if (!HASHITEM_EMPTY(hi)) {
id = HI2CI(hi)->ci_newID;
} else {
ci = getroom(spin, sizeof(compitem_T), true);
STRCPY(ci->ci_key, key);
ci->ci_flag = flag;
do {
check_renumber(spin);
id = spin->si_newcompID--;
} while (vim_strchr((char_u *)"/?*+[]\\-^", id) != NULL);
ci->ci_newID = id;
hash_add(&aff->af_comp, ci->ci_key);
}
*tp++ = id;
}
if (aff->af_flagtype == AFT_NUM && *p == ',')
++p;
}
}
*tp = NUL;
}
static void check_renumber(spellinfo_T *spin)
{
if (spin->si_newprefID == spin->si_newcompID && spin->si_newcompID < 128) {
spin->si_newprefID = 127;
spin->si_newcompID = 255;
}
}
static bool flag_in_afflist(int flagtype, char_u *afflist, unsigned flag)
{
char_u *p;
unsigned n;
switch (flagtype) {
case AFT_CHAR:
return vim_strchr(afflist, flag) != NULL;
case AFT_CAPLONG:
case AFT_LONG:
for (p = afflist; *p != NUL; ) {
n = mb_ptr2char_adv((const char_u **)&p);
if ((flagtype == AFT_LONG || (n >= 'A' && n <= 'Z'))
&& *p != NUL) {
n = mb_ptr2char_adv((const char_u **)&p) + (n << 16);
}
if (n == flag) {
return true;
}
}
break;
case AFT_NUM:
for (p = afflist; *p != NUL; ) {
int digits = getdigits_int(&p, true, 0);
assert(digits >= 0);
n = (unsigned int)digits;
if (n == 0) {
n = ZERO_FLAG;
}
if (n == flag) {
return true;
}
if (*p != NUL) { 
p++;
}
}
break;
}
return false;
}
static void aff_check_number(int spinval, int affval, char *name)
{
if (spinval != 0 && spinval != affval)
smsg(_("%s value differs from what is used in another .aff file"),
name);
}
static void aff_check_string(char_u *spinval, char_u *affval, char *name)
{
if (spinval != NULL && STRCMP(spinval, affval) != 0)
smsg(_("%s value differs from what is used in another .aff file"),
name);
}
static bool str_equal(char_u *s1, char_u *s2)
{
if (s1 == NULL || s2 == NULL)
return s1 == s2;
return STRCMP(s1, s2) == 0;
}
static void add_fromto(spellinfo_T *spin, garray_T *gap, char_u *from, char_u *to)
{
char_u word[MAXWLEN];
fromto_T *ftp = GA_APPEND_VIA_PTR(fromto_T, gap);
(void)spell_casefold(from, (int)STRLEN(from), word, MAXWLEN);
ftp->ft_from = getroom_save(spin, word);
(void)spell_casefold(to, (int)STRLEN(to), word, MAXWLEN);
ftp->ft_to = getroom_save(spin, word);
}
static bool sal_to_bool(char_u *s)
{
return STRCMP(s, "1") == 0 || STRCMP(s, "true") == 0;
}
static void spell_free_aff(afffile_T *aff)
{
hashtab_T *ht;
hashitem_T *hi;
int todo;
affheader_T *ah;
affentry_T *ae;
xfree(aff->af_enc);
for (ht = &aff->af_pref;; ht = &aff->af_suff) {
todo = (int)ht->ht_used;
for (hi = ht->ht_array; todo > 0; ++hi) {
if (!HASHITEM_EMPTY(hi)) {
--todo;
ah = HI2AH(hi);
for (ae = ah->ah_first; ae != NULL; ae = ae->ae_next)
vim_regfree(ae->ae_prog);
}
}
if (ht == &aff->af_suff)
break;
}
hash_clear(&aff->af_pref);
hash_clear(&aff->af_suff);
hash_clear(&aff->af_comp);
}
static int spell_read_dic(spellinfo_T *spin, char_u *fname, afffile_T *affile)
{
hashtab_T ht;
char_u line[MAXLINELEN];
char_u *p;
char_u *afflist;
char_u store_afflist[MAXWLEN];
int pfxlen;
bool need_affix;
char_u *dw;
char_u *pc;
char_u *w;
int l;
hash_T hash;
hashitem_T *hi;
FILE *fd;
int lnum = 1;
int non_ascii = 0;
int retval = OK;
char_u message[MAXLINELEN + MAXWLEN];
int flags;
int duplicate = 0;
fd = os_fopen((char *)fname, "r");
if (fd == NULL) {
EMSG2(_(e_notopen), fname);
return FAIL;
}
hash_init(&ht);
vim_snprintf((char *)IObuff, IOSIZE,
_("Reading dictionary file %s..."), fname);
spell_message(spin, IObuff);
spin->si_msg_count = 999999;
(void)vim_fgets(line, MAXLINELEN, fd);
if (!ascii_isdigit(*skipwhite(line)))
EMSG2(_("E760: No word count in %s"), fname);
while (!vim_fgets(line, MAXLINELEN, fd) && !got_int) {
line_breakcheck();
++lnum;
if (line[0] == '#' || line[0] == '/')
continue; 
l = (int)STRLEN(line);
while (l > 0 && line[l - 1] <= ' ')
--l;
if (l == 0)
continue; 
line[l] = NUL;
if (spin->si_conv.vc_type != CONV_NONE) {
pc = string_convert(&spin->si_conv, line, NULL);
if (pc == NULL) {
smsg(_("Conversion failure for word in %s line %d: %s"),
fname, lnum, line);
continue;
}
w = pc;
} else {
pc = NULL;
w = line;
}
afflist = NULL;
for (p = w; *p != NUL; MB_PTR_ADV(p)) {
if (*p == '\\' && (p[1] == '\\' || p[1] == '/')) {
STRMOVE(p, p + 1);
} else if (*p == '/') {
*p = NUL;
afflist = p + 1;
break;
}
}
if (spin->si_ascii && has_non_ascii(w)) {
++non_ascii;
xfree(pc);
continue;
}
if (spin->si_verbose && spin->si_msg_count > 10000) {
spin->si_msg_count = 0;
vim_snprintf((char *)message, sizeof(message),
_("line %6d, word %6ld - %s"),
lnum, spin->si_foldwcount + spin->si_keepwcount, w);
msg_start();
msg_puts_long_attr(message, 0);
msg_clr_eos();
msg_didout = FALSE;
msg_col = 0;
ui_flush();
}
dw = getroom_save(spin, w);
if (dw == NULL) {
retval = FAIL;
xfree(pc);
break;
}
hash = hash_hash(dw);
hi = hash_lookup(&ht, (const char *)dw, STRLEN(dw), hash);
if (!HASHITEM_EMPTY(hi)) {
if (p_verbose > 0)
smsg(_("Duplicate word in %s line %d: %s"),
fname, lnum, dw);
else if (duplicate == 0)
smsg(_("First duplicate word in %s line %d: %s"),
fname, lnum, dw);
++duplicate;
} else
hash_add_item(&ht, hi, dw, hash);
flags = 0;
store_afflist[0] = NUL;
pfxlen = 0;
need_affix = false;
if (afflist != NULL) {
flags |= get_affix_flags(affile, afflist);
if (affile->af_needaffix != 0 && flag_in_afflist(
affile->af_flagtype, afflist, affile->af_needaffix))
need_affix = true;
if (affile->af_pfxpostpone)
pfxlen = get_pfxlist(affile, afflist, store_afflist);
if (spin->si_compflags != NULL)
get_compflags(affile, afflist, store_afflist + pfxlen);
}
if (store_word(spin, dw, flags, spin->si_region,
store_afflist, need_affix) == FAIL)
retval = FAIL;
if (afflist != NULL) {
if (store_aff_word(spin, dw, afflist, affile,
&affile->af_suff, &affile->af_pref,
CONDIT_SUF, flags, store_afflist, pfxlen) == FAIL)
retval = FAIL;
if (store_aff_word(spin, dw, afflist, affile,
&affile->af_pref, NULL,
CONDIT_SUF, flags, store_afflist, pfxlen) == FAIL)
retval = FAIL;
}
xfree(pc);
}
if (duplicate > 0)
smsg(_("%d duplicate word(s) in %s"), duplicate, fname);
if (spin->si_ascii && non_ascii > 0)
smsg(_("Ignored %d word(s) with non-ASCII characters in %s"),
non_ascii, fname);
hash_clear(&ht);
fclose(fd);
return retval;
}
static int get_affix_flags(afffile_T *affile, char_u *afflist)
{
int flags = 0;
if (affile->af_keepcase != 0 && flag_in_afflist(
affile->af_flagtype, afflist, affile->af_keepcase))
flags |= WF_KEEPCAP | WF_FIXCAP;
if (affile->af_rare != 0 && flag_in_afflist(
affile->af_flagtype, afflist, affile->af_rare))
flags |= WF_RARE;
if (affile->af_bad != 0 && flag_in_afflist(
affile->af_flagtype, afflist, affile->af_bad))
flags |= WF_BANNED;
if (affile->af_needcomp != 0 && flag_in_afflist(
affile->af_flagtype, afflist, affile->af_needcomp))
flags |= WF_NEEDCOMP;
if (affile->af_comproot != 0 && flag_in_afflist(
affile->af_flagtype, afflist, affile->af_comproot))
flags |= WF_COMPROOT;
if (affile->af_nosuggest != 0 && flag_in_afflist(
affile->af_flagtype, afflist, affile->af_nosuggest))
flags |= WF_NOSUGGEST;
return flags;
}
static int get_pfxlist(afffile_T *affile, char_u *afflist, char_u *store_afflist)
{
char_u *p;
char_u *prevp;
int cnt = 0;
int id;
char_u key[AH_KEY_LEN];
hashitem_T *hi;
for (p = afflist; *p != NUL; ) {
prevp = p;
if (get_affitem(affile->af_flagtype, &p) != 0) {
STRLCPY(key, prevp, p - prevp + 1);
hi = hash_find(&affile->af_pref, key);
if (!HASHITEM_EMPTY(hi)) {
id = HI2AH(hi)->ah_newID;
if (id != 0)
store_afflist[cnt++] = id;
}
}
if (affile->af_flagtype == AFT_NUM && *p == ',')
++p;
}
store_afflist[cnt] = NUL;
return cnt;
}
static void get_compflags(afffile_T *affile, char_u *afflist, char_u *store_afflist)
{
char_u *p;
char_u *prevp;
int cnt = 0;
char_u key[AH_KEY_LEN];
hashitem_T *hi;
for (p = afflist; *p != NUL; ) {
prevp = p;
if (get_affitem(affile->af_flagtype, &p) != 0) {
STRLCPY(key, prevp, p - prevp + 1);
hi = hash_find(&affile->af_comp, key);
if (!HASHITEM_EMPTY(hi))
store_afflist[cnt++] = HI2CI(hi)->ci_newID;
}
if (affile->af_flagtype == AFT_NUM && *p == ',')
++p;
}
store_afflist[cnt] = NUL;
}
static int
store_aff_word (
spellinfo_T *spin, 
char_u *word, 
char_u *afflist, 
afffile_T *affile,
hashtab_T *ht,
hashtab_T *xht,
int condit, 
int flags, 
char_u *pfxlist, 
int pfxlen 
)
{
int todo;
hashitem_T *hi;
affheader_T *ah;
affentry_T *ae;
char_u newword[MAXWLEN];
int retval = OK;
int i, j;
char_u *p;
int use_flags;
char_u *use_pfxlist;
int use_pfxlen;
bool need_affix;
char_u store_afflist[MAXWLEN];
char_u pfx_pfxlist[MAXWLEN];
size_t wordlen = STRLEN(word);
int use_condit;
todo = (int)ht->ht_used;
for (hi = ht->ht_array; todo > 0 && retval == OK; ++hi) {
if (!HASHITEM_EMPTY(hi)) {
--todo;
ah = HI2AH(hi);
if (((condit & CONDIT_COMB) == 0 || ah->ah_combine)
&& flag_in_afflist(affile->af_flagtype, afflist,
ah->ah_flag)) {
for (ae = ah->ah_first; ae != NULL; ae = ae->ae_next) {
if ((xht != NULL || !affile->af_pfxpostpone
|| ae->ae_chop != NULL
|| ae->ae_flags != NULL)
&& (ae->ae_chop == NULL
|| STRLEN(ae->ae_chop) < wordlen)
&& (ae->ae_prog == NULL
|| vim_regexec_prog(&ae->ae_prog, false, word, (colnr_T)0))
&& (((condit & CONDIT_CFIX) == 0)
== ((condit & CONDIT_AFF) == 0
|| ae->ae_flags == NULL
|| !flag_in_afflist(affile->af_flagtype,
ae->ae_flags, affile->af_circumfix)))) {
if (xht == NULL) {
if (ae->ae_add == NULL) {
*newword = NUL;
} else {
STRLCPY(newword, ae->ae_add, MAXWLEN);
}
p = word;
if (ae->ae_chop != NULL) {
if (has_mbyte) {
i = mb_charlen(ae->ae_chop);
for (; i > 0; i--) {
MB_PTR_ADV(p);
}
} else {
p += STRLEN(ae->ae_chop);
}
}
STRCAT(newword, p);
} else {
STRLCPY(newword, word, MAXWLEN);
if (ae->ae_chop != NULL) {
p = newword + STRLEN(newword);
i = (int)MB_CHARLEN(ae->ae_chop);
for (; i > 0; i--) {
MB_PTR_BACK(newword, p);
}
*p = NUL;
}
if (ae->ae_add != NULL)
STRCAT(newword, ae->ae_add);
}
use_flags = flags;
use_pfxlist = pfxlist;
use_pfxlen = pfxlen;
need_affix = false;
use_condit = condit | CONDIT_COMB | CONDIT_AFF;
if (ae->ae_flags != NULL) {
use_flags |= get_affix_flags(affile, ae->ae_flags);
if (affile->af_needaffix != 0 && flag_in_afflist(
affile->af_flagtype, ae->ae_flags,
affile->af_needaffix))
need_affix = true;
if (affile->af_circumfix != 0 && flag_in_afflist(
affile->af_flagtype, ae->ae_flags,
affile->af_circumfix)) {
use_condit |= CONDIT_CFIX;
if ((condit & CONDIT_CFIX) == 0)
need_affix = true;
}
if (affile->af_pfxpostpone
|| spin->si_compflags != NULL) {
if (affile->af_pfxpostpone)
use_pfxlen = get_pfxlist(affile,
ae->ae_flags, store_afflist);
else
use_pfxlen = 0;
use_pfxlist = store_afflist;
for (i = 0; i < pfxlen; ++i) {
for (j = 0; j < use_pfxlen; ++j)
if (pfxlist[i] == use_pfxlist[j])
break;
if (j == use_pfxlen)
use_pfxlist[use_pfxlen++] = pfxlist[i];
}
if (spin->si_compflags != NULL)
get_compflags(affile, ae->ae_flags,
use_pfxlist + use_pfxlen);
else
use_pfxlist[use_pfxlen] = NUL;
for (i = pfxlen; pfxlist[i] != NUL; ++i) {
for (j = use_pfxlen;
use_pfxlist[j] != NUL; ++j)
if (pfxlist[i] == use_pfxlist[j])
break;
if (use_pfxlist[j] == NUL) {
use_pfxlist[j++] = pfxlist[i];
use_pfxlist[j] = NUL;
}
}
}
}
if (use_pfxlist != NULL && ae->ae_compforbid) {
STRLCPY(pfx_pfxlist, use_pfxlist, use_pfxlen + 1);
use_pfxlist = pfx_pfxlist;
}
if (spin->si_prefroot != NULL
&& spin->si_prefroot->wn_sibling != NULL) {
use_flags |= WF_HAS_AFF;
if (!ah->ah_combine && use_pfxlist != NULL)
use_pfxlist += use_pfxlen;
}
if (spin->si_compflags != NULL && !ae->ae_comppermit) {
if (xht != NULL)
use_flags |= WF_NOCOMPAFT;
else
use_flags |= WF_NOCOMPBEF;
}
if (store_word(spin, newword, use_flags,
spin->si_region, use_pfxlist,
need_affix) == FAIL)
retval = FAIL;
if ((condit & CONDIT_SUF) && ae->ae_flags != NULL)
if (store_aff_word(spin, newword, ae->ae_flags,
affile, &affile->af_suff, xht,
use_condit & (xht == NULL
? ~0 : ~CONDIT_SUF),
use_flags, use_pfxlist, pfxlen) == FAIL)
retval = FAIL;
if (xht != NULL && ah->ah_combine) {
if (store_aff_word(spin, newword,
afflist, affile,
xht, NULL, use_condit,
use_flags, use_pfxlist,
pfxlen) == FAIL
|| (ae->ae_flags != NULL
&& store_aff_word(spin, newword,
ae->ae_flags, affile,
xht, NULL, use_condit,
use_flags, use_pfxlist,
pfxlen) == FAIL))
retval = FAIL;
}
}
}
}
}
}
return retval;
}
static int spell_read_wordfile(spellinfo_T *spin, char_u *fname)
{
FILE *fd;
long lnum = 0;
char_u rline[MAXLINELEN];
char_u *line;
char_u *pc = NULL;
char_u *p;
int l;
int retval = OK;
bool did_word = false;
int non_ascii = 0;
int flags;
int regionmask;
fd = os_fopen((char *)fname, "r");
if (fd == NULL) {
EMSG2(_(e_notopen), fname);
return FAIL;
}
vim_snprintf((char *)IObuff, IOSIZE, _("Reading word file %s..."), fname);
spell_message(spin, IObuff);
while (!vim_fgets(rline, MAXLINELEN, fd) && !got_int) {
line_breakcheck();
++lnum;
if (*rline == '#')
continue;
l = (int)STRLEN(rline);
while (l > 0 && rline[l - 1] <= ' ')
--l;
if (l == 0)
continue; 
rline[l] = NUL;
xfree(pc);
if (spin->si_conv.vc_type != CONV_NONE) {
pc = string_convert(&spin->si_conv, rline, NULL);
if (pc == NULL) {
smsg(_("Conversion failure for word in %s line %ld: %s"),
fname, lnum, rline);
continue;
}
line = pc;
} else {
pc = NULL;
line = rline;
}
if (*line == '/') {
++line;
if (STRNCMP(line, "encoding=", 9) == 0) {
if (spin->si_conv.vc_type != CONV_NONE) {
smsg(_("Duplicate /encoding= line ignored in %s line %ld: %s"),
fname, lnum, line - 1);
} else if (did_word) {
smsg(_("/encoding= line after word ignored in %s line %ld: %s"),
fname, lnum, line - 1);
} else {
char_u *enc;
line += 9;
enc = enc_canonize(line);
if (!spin->si_ascii
&& convert_setup(&spin->si_conv, enc,
p_enc) == FAIL)
smsg(_("Conversion in %s not supported: from %s to %s"),
fname, line, p_enc);
xfree(enc);
spin->si_conv.vc_fail = true;
}
continue;
}
if (STRNCMP(line, "regions=", 8) == 0) {
if (spin->si_region_count > 1) {
smsg(_("Duplicate /regions= line ignored in %s line %ld: %s"),
fname, lnum, line);
} else {
line += 8;
if (STRLEN(line) > MAXREGIONS * 2) {
smsg(_("Too many regions in %s line %ld: %s"),
fname, lnum, line);
} else {
spin->si_region_count = (int)STRLEN(line) / 2;
STRCPY(spin->si_region_name, line);
spin->si_region = (1 << spin->si_region_count) - 1;
}
}
continue;
}
smsg(_("/ line ignored in %s line %ld: %s"),
fname, lnum, line - 1);
continue;
}
flags = 0;
regionmask = spin->si_region;
p = vim_strchr(line, '/');
if (p != NULL) {
*p++ = NUL;
while (*p != NUL) {
if (*p == '=') 
flags |= WF_KEEPCAP | WF_FIXCAP;
else if (*p == '!') 
flags |= WF_BANNED;
else if (*p == '?') 
flags |= WF_RARE;
else if (ascii_isdigit(*p)) { 
if ((flags & WF_REGION) == 0) 
regionmask = 0;
flags |= WF_REGION;
l = *p - '0';
if (l == 0 || l > spin->si_region_count) {
smsg(_("Invalid region nr in %s line %ld: %s"),
fname, lnum, p);
break;
}
regionmask |= 1 << (l - 1);
} else {
smsg(_("Unrecognized flags in %s line %ld: %s"),
fname, lnum, p);
break;
}
++p;
}
}
if (spin->si_ascii && has_non_ascii(line)) {
++non_ascii;
continue;
}
if (store_word(spin, line, flags, regionmask, NULL, false) == FAIL) {
retval = FAIL;
break;
}
did_word = true;
}
xfree(pc);
fclose(fd);
if (spin->si_ascii && non_ascii > 0) {
vim_snprintf((char *)IObuff, IOSIZE,
_("Ignored %d words with non-ASCII characters"), non_ascii);
spell_message(spin, IObuff);
}
return retval;
}
static void *getroom(spellinfo_T *spin, size_t len, bool align)
FUNC_ATTR_NONNULL_RET
{
char_u *p;
sblock_T *bl = spin->si_blocks;
assert(len <= SBLOCKSIZE);
if (align && bl != NULL)
bl->sb_used = (bl->sb_used + sizeof(char *) - 1)
& ~(sizeof(char *) - 1);
if (bl == NULL || bl->sb_used + len > SBLOCKSIZE) {
bl = xcalloc(1, (sizeof(sblock_T) + SBLOCKSIZE));
bl->sb_next = spin->si_blocks;
spin->si_blocks = bl;
bl->sb_used = 0;
++spin->si_blocks_cnt;
}
p = bl->sb_data + bl->sb_used;
bl->sb_used += (int)len;
return p;
}
static char_u *getroom_save(spellinfo_T *spin, char_u *s)
{
const size_t s_size = STRLEN(s) + 1;
return memcpy(getroom(spin, s_size, false), s, s_size);
}
static void free_blocks(sblock_T *bl)
{
sblock_T *next;
while (bl != NULL) {
next = bl->sb_next;
xfree(bl);
bl = next;
}
}
static wordnode_T *wordtree_alloc(spellinfo_T *spin)
FUNC_ATTR_NONNULL_RET
{
return (wordnode_T *)getroom(spin, sizeof(wordnode_T), true);
}
static int
store_word (
spellinfo_T *spin,
char_u *word,
int flags, 
int region, 
char_u *pfxlist, 
bool need_affix 
)
{
int len = (int)STRLEN(word);
int ct = captype(word, word + len);
char_u foldword[MAXWLEN];
int res = OK;
char_u *p;
(void)spell_casefold(word, len, foldword, MAXWLEN);
for (p = pfxlist; res == OK; ++p) {
if (!need_affix || (p != NULL && *p != NUL))
res = tree_add_word(spin, foldword, spin->si_foldroot, ct | flags,
region, p == NULL ? 0 : *p);
if (p == NULL || *p == NUL)
break;
}
++spin->si_foldwcount;
if (res == OK && (ct == WF_KEEPCAP || (flags & WF_KEEPCAP))) {
for (p = pfxlist; res == OK; ++p) {
if (!need_affix || (p != NULL && *p != NUL))
res = tree_add_word(spin, word, spin->si_keeproot, flags,
region, p == NULL ? 0 : *p);
if (p == NULL || *p == NUL)
break;
}
++spin->si_keepwcount;
}
return res;
}
static int tree_add_word(spellinfo_T *spin, char_u *word, wordnode_T *root, int flags, int region, int affixID)
{
wordnode_T *node = root;
wordnode_T *np;
wordnode_T *copyp, **copyprev;
wordnode_T **prev = NULL;
int i;
for (i = 0;; ++i) {
if (node != NULL && node->wn_refs > 1) {
--node->wn_refs;
copyprev = prev;
for (copyp = node; copyp != NULL; copyp = copyp->wn_sibling) {
np = get_wordnode(spin);
if (np == NULL)
return FAIL;
np->wn_child = copyp->wn_child;
if (np->wn_child != NULL)
++np->wn_child->wn_refs; 
np->wn_byte = copyp->wn_byte;
if (np->wn_byte == NUL) {
np->wn_flags = copyp->wn_flags;
np->wn_region = copyp->wn_region;
np->wn_affixID = copyp->wn_affixID;
}
np->wn_refs = 1;
if (copyprev != NULL)
*copyprev = np;
copyprev = &np->wn_sibling;
if (copyp == node)
node = np;
}
}
while (node != NULL
&& (node->wn_byte < word[i]
|| (node->wn_byte == NUL
&& (flags < 0
? node->wn_affixID < (unsigned)affixID
: (node->wn_flags < (unsigned)(flags & WN_MASK)
|| (node->wn_flags == (flags & WN_MASK)
&& (spin->si_sugtree
? (node->wn_region & 0xffff) < region
: node->wn_affixID
< (unsigned)affixID))))))) {
prev = &node->wn_sibling;
node = *prev;
}
if (node == NULL
|| node->wn_byte != word[i]
|| (word[i] == NUL
&& (flags < 0
|| spin->si_sugtree
|| node->wn_flags != (flags & WN_MASK)
|| node->wn_affixID != affixID))) {
np = get_wordnode(spin);
if (np == NULL)
return FAIL;
np->wn_byte = word[i];
if (node == NULL)
np->wn_refs = 1;
else {
np->wn_refs = node->wn_refs;
node->wn_refs = 1;
}
if (prev != NULL)
*prev = np;
np->wn_sibling = node;
node = np;
}
if (word[i] == NUL) {
node->wn_flags = flags;
node->wn_region |= region;
node->wn_affixID = affixID;
break;
}
prev = &node->wn_child;
node = *prev;
}
#if defined(SPELL_PRINTTREE)
smsg((char_u *)"Added \"%s\"", word);
spell_print_tree(root->wn_sibling);
#endif
++spin->si_msg_count;
if (spin->si_compress_cnt > 1) {
if (--spin->si_compress_cnt == 1)
spin->si_blocks_cnt += compress_inc;
}
#if !defined(SPELL_COMPRESS_ALLWAYS)
if (spin->si_compress_cnt == 1 
? spin->si_free_count < MAXWLEN
: spin->si_blocks_cnt >= compress_start)
#endif
{
spin->si_blocks_cnt -= compress_inc;
spin->si_compress_cnt = compress_added;
if (spin->si_verbose) {
msg_start();
msg_puts(_(msg_compressing));
msg_clr_eos();
msg_didout = FALSE;
msg_col = 0;
ui_flush();
}
wordtree_compress(spin, spin->si_foldroot);
if (affixID >= 0)
wordtree_compress(spin, spin->si_keeproot);
}
return OK;
}
static wordnode_T *get_wordnode(spellinfo_T *spin)
{
wordnode_T *n;
if (spin->si_first_free == NULL)
n = (wordnode_T *)getroom(spin, sizeof(wordnode_T), true);
else {
n = spin->si_first_free;
spin->si_first_free = n->wn_child;
memset(n, 0, sizeof(wordnode_T));
--spin->si_free_count;
}
#if defined(SPELL_PRINTTREE)
if (n != NULL)
n->wn_nr = ++spin->si_wordnode_nr;
#endif
return n;
}
static int deref_wordnode(spellinfo_T *spin, wordnode_T *node)
{
wordnode_T *np;
int cnt = 0;
if (--node->wn_refs == 0) {
for (np = node; np != NULL; np = np->wn_sibling) {
if (np->wn_child != NULL)
cnt += deref_wordnode(spin, np->wn_child);
free_wordnode(spin, np);
++cnt;
}
++cnt; 
}
return cnt;
}
static void free_wordnode(spellinfo_T *spin, wordnode_T *n)
{
n->wn_child = spin->si_first_free;
spin->si_first_free = n;
++spin->si_free_count;
}
static void wordtree_compress(spellinfo_T *spin, wordnode_T *root)
{
hashtab_T ht;
int n;
int tot = 0;
int perc;
if (root->wn_sibling != NULL) {
hash_init(&ht);
n = node_compress(spin, root->wn_sibling, &ht, &tot);
#if !defined(SPELL_PRINTTREE)
if (spin->si_verbose || p_verbose > 2)
#endif
{
if (tot > 1000000)
perc = (tot - n) / (tot / 100);
else if (tot == 0)
perc = 0;
else
perc = (tot - n) * 100 / tot;
vim_snprintf((char *)IObuff, IOSIZE,
_("Compressed %d of %d nodes; %d (%d%%) remaining"),
n, tot, tot - n, perc);
spell_message(spin, IObuff);
}
#if defined(SPELL_PRINTTREE)
spell_print_tree(root->wn_sibling);
#endif
hash_clear(&ht);
}
}
static int
node_compress (
spellinfo_T *spin,
wordnode_T *node,
hashtab_T *ht,
int *tot 
)
{
wordnode_T *np;
wordnode_T *tp;
wordnode_T *child;
hash_T hash;
hashitem_T *hi;
int len = 0;
unsigned nr, n;
int compressed = 0;
for (np = node; np != NULL && !got_int; np = np->wn_sibling) {
++len;
if ((child = np->wn_child) != NULL) {
compressed += node_compress(spin, child, ht, tot);
hash = hash_hash(child->wn_u1.hashkey);
hi = hash_lookup(ht, (const char *)child->wn_u1.hashkey,
STRLEN(child->wn_u1.hashkey), hash);
if (!HASHITEM_EMPTY(hi)) {
for (tp = HI2WN(hi); tp != NULL; tp = tp->wn_u2.next)
if (node_equal(child, tp)) {
++tp->wn_refs;
compressed += deref_wordnode(spin, child);
np->wn_child = tp;
break;
}
if (tp == NULL) {
tp = HI2WN(hi);
child->wn_u2.next = tp->wn_u2.next;
tp->wn_u2.next = child;
}
} else
hash_add_item(ht, hi, child->wn_u1.hashkey, hash);
}
}
*tot += len + 1; 
node->wn_u1.hashkey[0] = len;
nr = 0;
for (np = node; np != NULL; np = np->wn_sibling) {
if (np->wn_byte == NUL)
n = np->wn_flags + (np->wn_region << 8) + (np->wn_affixID << 16);
else
n = (unsigned)(np->wn_byte + ((uintptr_t)np->wn_child << 8));
nr = nr * 101 + n;
}
n = nr & 0xff;
node->wn_u1.hashkey[1] = n == 0 ? 1 : n;
n = (nr >> 8) & 0xff;
node->wn_u1.hashkey[2] = n == 0 ? 1 : n;
n = (nr >> 16) & 0xff;
node->wn_u1.hashkey[3] = n == 0 ? 1 : n;
n = (nr >> 24) & 0xff;
node->wn_u1.hashkey[4] = n == 0 ? 1 : n;
node->wn_u1.hashkey[5] = NUL;
fast_breakcheck();
return compressed;
}
static bool node_equal(wordnode_T *n1, wordnode_T *n2)
{
wordnode_T *p1;
wordnode_T *p2;
for (p1 = n1, p2 = n2; p1 != NULL && p2 != NULL;
p1 = p1->wn_sibling, p2 = p2->wn_sibling)
if (p1->wn_byte != p2->wn_byte
|| (p1->wn_byte == NUL
? (p1->wn_flags != p2->wn_flags
|| p1->wn_region != p2->wn_region
|| p1->wn_affixID != p2->wn_affixID)
: (p1->wn_child != p2->wn_child)))
break;
return p1 == NULL && p2 == NULL;
}
static int rep_compare(const void *s1, const void *s2)
{
fromto_T *p1 = (fromto_T *)s1;
fromto_T *p2 = (fromto_T *)s2;
return STRCMP(p1->ft_from, p2->ft_from);
}
static int write_vim_spell(spellinfo_T *spin, char_u *fname)
{
int retval = OK;
int regionmask;
FILE *fd = os_fopen((char *)fname, "w");
if (fd == NULL) {
EMSG2(_(e_notopen), fname);
return FAIL;
}
size_t fwv = fwrite(VIMSPELLMAGIC, VIMSPELLMAGICL, 1, fd);
if (fwv != (size_t)1)
goto theend;
putc(VIMSPELLVERSION, fd); 
if (spin->si_info != NULL) {
putc(SN_INFO, fd); 
putc(0, fd); 
size_t i = STRLEN(spin->si_info);
put_bytes(fd, i, 4); 
fwv &= fwrite(spin->si_info, i, 1, fd); 
}
if (spin->si_region_count > 1) {
putc(SN_REGION, fd); 
putc(SNF_REQUIRED, fd); 
size_t l = (size_t)spin->si_region_count * 2;
put_bytes(fd, l, 4); 
fwv &= fwrite(spin->si_region_name, l, 1, fd);
regionmask = (1 << spin->si_region_count) - 1;
} else
regionmask = 0;
if (!spin->si_ascii && !spin->si_add) {
char_u folchars[128 * 8];
int flags;
putc(SN_CHARFLAGS, fd); 
putc(SNF_REQUIRED, fd); 
size_t l = 0;
for (size_t i = 128; i < 256; i++) {
l += (size_t)utf_char2bytes(spelltab.st_fold[i], folchars + l);
}
put_bytes(fd, 1 + 128 + 2 + l, 4); 
fputc(128, fd); 
for (size_t i = 128; i < 256; ++i) {
flags = 0;
if (spelltab.st_isw[i])
flags |= CF_WORD;
if (spelltab.st_isu[i])
flags |= CF_UPPER;
fputc(flags, fd); 
}
put_bytes(fd, l, 2); 
fwv &= fwrite(folchars, l, 1, fd); 
}
if (spin->si_midword != NULL) {
putc(SN_MIDWORD, fd); 
putc(SNF_REQUIRED, fd); 
size_t i = STRLEN(spin->si_midword);
put_bytes(fd, i, 4); 
fwv &= fwrite(spin->si_midword, i, 1, fd);
}
if (!GA_EMPTY(&spin->si_prefcond)) {
putc(SN_PREFCOND, fd); 
putc(SNF_REQUIRED, fd); 
size_t l = (size_t)write_spell_prefcond(NULL, &spin->si_prefcond);
put_bytes(fd, l, 4); 
write_spell_prefcond(fd, &spin->si_prefcond);
}
for (unsigned int round = 1; round <= 3; ++round) {
garray_T *gap;
if (round == 1)
gap = &spin->si_rep;
else if (round == 2) {
if (spin->si_sofofr != NULL && spin->si_sofoto != NULL)
continue;
gap = &spin->si_sal;
} else
gap = &spin->si_repsal;
if (GA_EMPTY(gap))
continue;
if (round != 2)
qsort(gap->ga_data, (size_t)gap->ga_len,
sizeof(fromto_T), rep_compare);
int sect_id = round == 1 ? SN_REP : (round == 2 ? SN_SAL : SN_REPSAL);
putc(sect_id, fd); 
putc(0, fd); 
size_t l = 2; 
assert(gap->ga_len >= 0);
for (size_t i = 0; i < (size_t)gap->ga_len; ++i) {
fromto_T *ftp = &((fromto_T *)gap->ga_data)[i];
l += 1 + STRLEN(ftp->ft_from); 
l += 1 + STRLEN(ftp->ft_to); 
}
if (round == 2)
++l; 
put_bytes(fd, l, 4); 
if (round == 2) {
int i = 0;
if (spin->si_followup)
i |= SAL_F0LLOWUP;
if (spin->si_collapse)
i |= SAL_COLLAPSE;
if (spin->si_rem_accents)
i |= SAL_REM_ACCENTS;
putc(i, fd); 
}
put_bytes(fd, (uintmax_t)gap->ga_len, 2); 
for (size_t i = 0; i < (size_t)gap->ga_len; ++i) {
fromto_T *ftp = &((fromto_T *)gap->ga_data)[i];
for (unsigned int rr = 1; rr <= 2; ++rr) {
char_u *p = rr == 1 ? ftp->ft_from : ftp->ft_to;
l = STRLEN(p);
assert(l < INT_MAX);
putc((int)l, fd);
if (l > 0)
fwv &= fwrite(p, l, 1, fd);
}
}
}
if (spin->si_sofofr != NULL && spin->si_sofoto != NULL) {
putc(SN_SOFO, fd); 
putc(0, fd); 
size_t l = STRLEN(spin->si_sofofr);
put_bytes(fd, l + STRLEN(spin->si_sofoto) + 4, 4); 
put_bytes(fd, l, 2); 
fwv &= fwrite(spin->si_sofofr, l, 1, fd); 
l = STRLEN(spin->si_sofoto);
put_bytes(fd, l, 2); 
fwv &= fwrite(spin->si_sofoto, l, 1, fd); 
}
if (spin->si_commonwords.ht_used > 0) {
putc(SN_WORDS, fd); 
putc(0, fd); 
for (unsigned int round = 1; round <= 2; ++round) {
size_t todo;
size_t len = 0;
hashitem_T *hi;
todo = spin->si_commonwords.ht_used;
for (hi = spin->si_commonwords.ht_array; todo > 0; ++hi)
if (!HASHITEM_EMPTY(hi)) {
size_t l = STRLEN(hi->hi_key) + 1;
len += l;
if (round == 2) 
fwv &= fwrite(hi->hi_key, l, 1, fd);
--todo;
}
if (round == 1)
put_bytes(fd, len, 4); 
}
}
if (!GA_EMPTY(&spin->si_map)) {
putc(SN_MAP, fd); 
putc(0, fd); 
size_t l = (size_t)spin->si_map.ga_len;
put_bytes(fd, l, 4); 
fwv &= fwrite(spin->si_map.ga_data, l, 1, fd); 
}
if (!spin->si_nosugfile
&& (!GA_EMPTY(&spin->si_sal)
|| (spin->si_sofofr != NULL && spin->si_sofoto != NULL))) {
putc(SN_SUGFILE, fd); 
putc(0, fd); 
put_bytes(fd, 8, 4); 
spin->si_sugtime = time(NULL);
put_time(fd, spin->si_sugtime); 
}
if (spin->si_nosplitsugs) {
putc(SN_NOSPLITSUGS, fd); 
putc(0, fd); 
put_bytes(fd, 0, 4); 
}
if (spin->si_nocompoundsugs) {
putc(SN_NOCOMPOUNDSUGS, fd); 
putc(0, fd); 
put_bytes(fd, 0, 4); 
}
if (spin->si_compflags != NULL) {
putc(SN_COMPOUND, fd); 
putc(0, fd); 
size_t l = STRLEN(spin->si_compflags);
assert(spin->si_comppat.ga_len >= 0);
for (size_t i = 0; i < (size_t)spin->si_comppat.ga_len; ++i) {
l += STRLEN(((char_u **)(spin->si_comppat.ga_data))[i]) + 1;
}
put_bytes(fd, l + 7, 4); 
putc(spin->si_compmax, fd); 
putc(spin->si_compminlen, fd); 
putc(spin->si_compsylmax, fd); 
putc(0, fd); 
putc(spin->si_compoptions, fd); 
put_bytes(fd, (uintmax_t)spin->si_comppat.ga_len, 2); 
for (size_t i = 0; i < (size_t)spin->si_comppat.ga_len; ++i) {
char_u *p = ((char_u **)(spin->si_comppat.ga_data))[i];
assert(STRLEN(p) < INT_MAX);
putc((int)STRLEN(p), fd); 
fwv &= fwrite(p, STRLEN(p), 1, fd); 
}
fwv &= fwrite(spin->si_compflags, STRLEN(spin->si_compflags), 1, fd);
}
if (spin->si_nobreak) {
putc(SN_NOBREAK, fd); 
putc(0, fd); 
put_bytes(fd, 0, 4); 
}
if (spin->si_syllable != NULL) {
putc(SN_SYLLABLE, fd); 
putc(0, fd); 
size_t l = STRLEN(spin->si_syllable);
put_bytes(fd, l, 4); 
fwv &= fwrite(spin->si_syllable, l, 1, fd); 
}
putc(SN_END, fd); 
spin->si_memtot = 0;
for (unsigned int round = 1; round <= 3; ++round) {
wordnode_T *tree;
if (round == 1)
tree = spin->si_foldroot->wn_sibling;
else if (round == 2)
tree = spin->si_keeproot->wn_sibling;
else
tree = spin->si_prefroot->wn_sibling;
clear_node(tree);
size_t nodecount = (size_t)put_node(NULL, tree, 0, regionmask, round == 3);
put_bytes(fd, nodecount, 4); 
assert(nodecount + nodecount * sizeof(int) < INT_MAX);
spin->si_memtot += (int)(nodecount + nodecount * sizeof(int));
(void)put_node(fd, tree, 0, regionmask, round == 3);
}
if (putc(0, fd) == EOF)
retval = FAIL;
theend:
if (fclose(fd) == EOF)
retval = FAIL;
if (fwv != (size_t)1)
retval = FAIL;
if (retval == FAIL)
EMSG(_(e_write));
return retval;
}
static void clear_node(wordnode_T *node)
{
wordnode_T *np;
if (node != NULL)
for (np = node; np != NULL; np = np->wn_sibling) {
np->wn_u1.index = 0;
np->wn_u2.wnode = NULL;
if (np->wn_byte != NUL)
clear_node(np->wn_child);
}
}
static int
put_node (
FILE *fd, 
wordnode_T *node,
int idx,
int regionmask,
bool prefixtree 
)
{
if (node == NULL)
return 0;
node->wn_u1.index = idx;
int siblingcount = 0;
for (wordnode_T *np = node; np != NULL; np = np->wn_sibling)
++siblingcount;
if (fd != NULL)
putc(siblingcount, fd); 
for (wordnode_T *np = node; np != NULL; np = np->wn_sibling) {
if (np->wn_byte == 0) {
if (fd != NULL) {
if (prefixtree) {
if (np->wn_flags == (uint16_t)PFX_FLAGS)
putc(BY_NOFLAGS, fd); 
else {
putc(BY_FLAGS, fd); 
putc(np->wn_flags, fd); 
}
putc(np->wn_affixID, fd); 
put_bytes(fd, (uintmax_t)np->wn_region, 2); 
} else {
int flags = np->wn_flags;
if (regionmask != 0 && np->wn_region != regionmask)
flags |= WF_REGION;
if (np->wn_affixID != 0)
flags |= WF_AFX;
if (flags == 0) {
putc(BY_NOFLAGS, fd); 
} else {
if (np->wn_flags >= 0x100) {
putc(BY_FLAGS2, fd); 
putc(flags, fd); 
putc((int)((unsigned)flags >> 8), fd); 
} else {
putc(BY_FLAGS, fd); 
putc(flags, fd); 
}
if (flags & WF_REGION)
putc(np->wn_region, fd); 
if (flags & WF_AFX)
putc(np->wn_affixID, fd); 
}
}
}
} else {
if (np->wn_child->wn_u1.index != 0
&& np->wn_child->wn_u2.wnode != node) {
if (fd != NULL) {
putc(BY_INDEX, fd); 
put_bytes(fd, (uintmax_t)np->wn_child->wn_u1.index, 3); 
}
} else if (np->wn_child->wn_u2.wnode == NULL)
np->wn_child->wn_u2.wnode = node;
if (fd != NULL)
if (putc(np->wn_byte, fd) == EOF) { 
EMSG(_(e_write));
return 0;
}
}
}
int newindex = idx + siblingcount + 1;
for (wordnode_T *np = node; np != NULL; np = np->wn_sibling)
if (np->wn_byte != 0 && np->wn_child->wn_u2.wnode == node)
newindex = put_node(fd, np->wn_child, newindex, regionmask,
prefixtree);
return newindex;
}
void ex_mkspell(exarg_T *eap)
{
int fcount;
char_u **fnames;
char_u *arg = eap->arg;
bool ascii = false;
if (STRNCMP(arg, "-ascii", 6) == 0) {
ascii = true;
arg = skipwhite(arg + 6);
}
if (get_arglist_exp(arg, &fcount, &fnames, false) == OK) {
mkspell(fcount, fnames, ascii, eap->forceit, false);
FreeWild(fcount, fnames);
}
}
static void spell_make_sugfile(spellinfo_T *spin, char_u *wfname)
{
char_u *fname = NULL;
int len;
slang_T *slang;
bool free_slang = false;
for (slang = first_lang; slang != NULL; slang = slang->sl_next) {
if (path_full_compare(wfname, slang->sl_fname, false, true)
== kEqualFiles) {
break;
}
}
if (slang == NULL) {
spell_message(spin, (char_u *)_("Reading back spell file..."));
slang = spell_load_file(wfname, NULL, NULL, false);
if (slang == NULL)
return;
free_slang = true;
}
spin->si_blocks = NULL;
spin->si_blocks_cnt = 0;
spin->si_compress_cnt = 0; 
spin->si_free_count = 0;
spin->si_first_free = NULL;
spin->si_foldwcount = 0;
spell_message(spin, (char_u *)_("Performing soundfolding..."));
if (sug_filltree(spin, slang) == FAIL)
goto theend;
if (sug_maketable(spin) == FAIL)
goto theend;
smsg(_("Number of words after soundfolding: %" PRId64),
(int64_t)spin->si_spellbuf->b_ml.ml_line_count);
spell_message(spin, (char_u *)_(msg_compressing));
wordtree_compress(spin, spin->si_foldroot);
fname = xmalloc(MAXPATHL);
STRLCPY(fname, wfname, MAXPATHL);
len = (int)STRLEN(fname);
fname[len - 2] = 'u';
fname[len - 1] = 'g';
sug_write(spin, fname);
theend:
xfree(fname);
if (free_slang)
slang_free(slang);
free_blocks(spin->si_blocks);
close_spellbuf(spin->si_spellbuf);
}
static int sug_filltree(spellinfo_T *spin, slang_T *slang)
{
char_u *byts;
idx_T *idxs;
int depth;
idx_T arridx[MAXWLEN];
int curi[MAXWLEN];
char_u tword[MAXWLEN];
char_u tsalword[MAXWLEN];
int c;
idx_T n;
unsigned words_done = 0;
int wordcount[MAXWLEN];
spin->si_foldroot = wordtree_alloc(spin);
spin->si_sugtree = true;
byts = slang->sl_fbyts;
idxs = slang->sl_fidxs;
arridx[0] = 0;
curi[0] = 1;
wordcount[0] = 0;
depth = 0;
while (depth >= 0 && !got_int) {
if (curi[depth] > byts[arridx[depth]]) {
idxs[arridx[depth]] = wordcount[depth];
if (depth > 0)
wordcount[depth - 1] += wordcount[depth];
--depth;
line_breakcheck();
} else {
n = arridx[depth] + curi[depth];
++curi[depth];
c = byts[n];
if (c == 0) {
tword[depth] = NUL;
spell_soundfold(slang, tword, true, tsalword);
if (tree_add_word(spin, tsalword, spin->si_foldroot,
words_done >> 16, words_done & 0xffff,
0) == FAIL)
return FAIL;
++words_done;
++wordcount[depth];
spin->si_blocks_cnt = 0;
while (byts[n + 1] == 0) {
++n;
++curi[depth];
}
} else {
tword[depth++] = c;
arridx[depth] = idxs[n];
curi[depth] = 1;
wordcount[depth] = 0;
}
}
}
smsg(_("Total number of words: %d"), words_done);
return OK;
}
static int sug_maketable(spellinfo_T *spin)
{
garray_T ga;
int res = OK;
spin->si_spellbuf = open_spellbuf();
ga_init(&ga, 1, 100);
if (sug_filltable(spin, spin->si_foldroot->wn_sibling, 0, &ga) == -1)
res = FAIL;
ga_clear(&ga);
return res;
}
static int
sug_filltable (
spellinfo_T *spin,
wordnode_T *node,
int startwordnr,
garray_T *gap 
)
{
wordnode_T *p, *np;
int wordnr = startwordnr;
int nr;
int prev_nr;
for (p = node; p != NULL; p = p->wn_sibling) {
if (p->wn_byte == NUL) {
gap->ga_len = 0;
prev_nr = 0;
for (np = p; np != NULL && np->wn_byte == NUL; np = np->wn_sibling) {
ga_grow(gap, 10);
nr = (np->wn_flags << 16) + (np->wn_region & 0xffff);
nr -= prev_nr;
prev_nr += nr;
gap->ga_len += offset2bytes(nr,
(char_u *)gap->ga_data + gap->ga_len);
}
((char_u *)gap->ga_data)[gap->ga_len++] = NUL;
if (ml_append_buf(spin->si_spellbuf, (linenr_T)wordnr,
gap->ga_data, gap->ga_len, true) == FAIL) {
return -1;
}
wordnr++;
while (p->wn_sibling != NULL && p->wn_sibling->wn_byte == NUL)
p->wn_sibling = p->wn_sibling->wn_sibling;
p->wn_flags = 0;
p->wn_region = 0;
} else {
wordnr = sug_filltable(spin, p->wn_child, wordnr, gap);
if (wordnr == -1)
return -1;
}
}
return wordnr;
}
static int offset2bytes(int nr, char_u *buf)
{
int rem;
int b1, b2, b3, b4;
b1 = nr % 255 + 1;
rem = nr / 255;
b2 = rem % 255 + 1;
rem = rem / 255;
b3 = rem % 255 + 1;
b4 = rem / 255 + 1;
if (b4 > 1 || b3 > 0x1f) { 
buf[0] = 0xe0 + b4;
buf[1] = b3;
buf[2] = b2;
buf[3] = b1;
return 4;
}
if (b3 > 1 || b2 > 0x3f ) { 
buf[0] = 0xc0 + b3;
buf[1] = b2;
buf[2] = b1;
return 3;
}
if (b2 > 1 || b1 > 0x7f ) { 
buf[0] = 0x80 + b2;
buf[1] = b1;
return 2;
}
buf[0] = b1;
return 1;
}
static void sug_write(spellinfo_T *spin, char_u *fname)
{
FILE *fd = os_fopen((char *)fname, "w");
if (fd == NULL) {
EMSG2(_(e_notopen), fname);
return;
}
vim_snprintf((char *)IObuff, IOSIZE,
_("Writing suggestion file %s..."), fname);
spell_message(spin, IObuff);
if (fwrite(VIMSUGMAGIC, VIMSUGMAGICL, (size_t)1, fd) != 1) { 
EMSG(_(e_write));
goto theend;
}
putc(VIMSUGVERSION, fd); 
put_time(fd, spin->si_sugtime); 
spin->si_memtot = 0;
wordnode_T *tree = spin->si_foldroot->wn_sibling;
clear_node(tree);
size_t nodecount = (size_t)put_node(NULL, tree, 0, 0, false);
put_bytes(fd, nodecount, 4); 
assert(nodecount + nodecount * sizeof(int) < INT_MAX);
spin->si_memtot += (int)(nodecount + nodecount * sizeof(int));
(void)put_node(fd, tree, 0, 0, false);
linenr_T wcount = spin->si_spellbuf->b_ml.ml_line_count;
assert(wcount >= 0);
put_bytes(fd, (uintmax_t)wcount, 4); 
for (linenr_T lnum = 1; lnum <= wcount; ++lnum) {
char_u *line = ml_get_buf(spin->si_spellbuf, lnum, FALSE);
size_t len = STRLEN(line) + 1;
if (fwrite(line, len, 1, fd) == 0) {
EMSG(_(e_write));
goto theend;
}
assert((size_t)spin->si_memtot + len <= INT_MAX);
spin->si_memtot += (int)len;
}
if (putc(0, fd) == EOF)
EMSG(_(e_write));
vim_snprintf((char *)IObuff, IOSIZE,
_("Estimated runtime memory use: %d bytes"), spin->si_memtot);
spell_message(spin, IObuff);
theend:
fclose(fd);
}
static void
mkspell (
int fcount,
char_u **fnames,
bool ascii, 
bool over_write, 
bool added_word 
)
{
char_u *fname = NULL;
char_u *wfname;
char_u **innames;
int incount;
afffile_T *(afile[MAXREGIONS]);
int i;
int len;
bool error = false;
spellinfo_T spin;
memset(&spin, 0, sizeof(spin));
spin.si_verbose = !added_word;
spin.si_ascii = ascii;
spin.si_followup = true;
spin.si_rem_accents = true;
ga_init(&spin.si_rep, (int)sizeof(fromto_T), 20);
ga_init(&spin.si_repsal, (int)sizeof(fromto_T), 20);
ga_init(&spin.si_sal, (int)sizeof(fromto_T), 20);
ga_init(&spin.si_map, (int)sizeof(char_u), 100);
ga_init(&spin.si_comppat, (int)sizeof(char_u *), 20);
ga_init(&spin.si_prefcond, (int)sizeof(char_u *), 50);
hash_init(&spin.si_commonwords);
spin.si_newcompID = 127; 
innames = &fnames[1];
incount = fcount - 1;
wfname = xmalloc(MAXPATHL);
if (fcount >= 1) {
len = (int)STRLEN(fnames[0]);
if (fcount == 1 && len > 4 && STRCMP(fnames[0] + len - 4, ".add") == 0) {
innames = &fnames[0];
incount = 1;
vim_snprintf((char *)wfname, MAXPATHL, "%s.spl", fnames[0]);
} else if (fcount == 1) {
innames = &fnames[0];
incount = 1;
vim_snprintf((char *)wfname, MAXPATHL, SPL_FNAME_TMPL,
fnames[0], spin.si_ascii ? (char_u *)"ascii" : spell_enc());
} else if (len > 4 && STRCMP(fnames[0] + len - 4, ".spl") == 0) {
STRLCPY(wfname, fnames[0], MAXPATHL);
} else
vim_snprintf((char *)wfname, MAXPATHL, SPL_FNAME_TMPL,
fnames[0], spin.si_ascii ? (char_u *)"ascii" : spell_enc());
if (strstr((char *)path_tail(wfname), SPL_FNAME_ASCII) != NULL)
spin.si_ascii = true;
if (strstr((char *)path_tail(wfname), SPL_FNAME_ADD) != NULL)
spin.si_add = true;
}
if (incount <= 0) {
EMSG(_(e_invarg)); 
} else if (vim_strchr(path_tail(wfname), '_') != NULL) {
EMSG(_("E751: Output file name must not have region name"));
} else if (incount > MAXREGIONS) {
emsgf(_("E754: Only up to %d regions supported"), MAXREGIONS);
} else {
if (!over_write && os_path_exists(wfname)) {
EMSG(_(e_exists));
goto theend;
}
if (os_isdir(wfname)) {
EMSG2(_(e_isadir2), wfname);
goto theend;
}
fname = xmalloc(MAXPATHL);
for (i = 0; i < incount; ++i) {
afile[i] = NULL;
if (incount > 1) {
len = (int)STRLEN(innames[i]);
if (STRLEN(path_tail(innames[i])) < 5
|| innames[i][len - 3] != '_') {
EMSG2(_("E755: Invalid region in %s"), innames[i]);
goto theend;
}
spin.si_region_name[i * 2] = TOLOWER_ASC(innames[i][len - 2]);
spin.si_region_name[i * 2 + 1] =
TOLOWER_ASC(innames[i][len - 1]);
}
}
spin.si_region_count = incount;
spin.si_foldroot = wordtree_alloc(&spin);
spin.si_keeproot = wordtree_alloc(&spin);
spin.si_prefroot = wordtree_alloc(&spin);
if (!spin.si_add)
spin.si_clear_chartab = true;
for (i = 0; i < incount && !error; ++i) {
spin.si_conv.vc_type = CONV_NONE;
spin.si_region = 1 << i;
vim_snprintf((char *)fname, MAXPATHL, "%s.aff", innames[i]);
if (os_path_exists(fname)) {
afile[i] = spell_read_aff(&spin, fname);
if (afile[i] == NULL)
error = true;
else {
vim_snprintf((char *)fname, MAXPATHL, "%s.dic",
innames[i]);
if (spell_read_dic(&spin, fname, afile[i]) == FAIL)
error = true;
}
} else {
if (spell_read_wordfile(&spin, innames[i]) == FAIL)
error = true;
}
convert_setup(&spin.si_conv, NULL, NULL);
}
if (spin.si_compflags != NULL && spin.si_nobreak)
MSG(_("Warning: both compounding and NOBREAK specified"));
if (!error && !got_int) {
spell_message(&spin, (char_u *)_(msg_compressing));
wordtree_compress(&spin, spin.si_foldroot);
wordtree_compress(&spin, spin.si_keeproot);
wordtree_compress(&spin, spin.si_prefroot);
}
if (!error && !got_int) {
vim_snprintf((char *)IObuff, IOSIZE,
_("Writing spell file %s..."), wfname);
spell_message(&spin, IObuff);
error = write_vim_spell(&spin, wfname) == FAIL;
spell_message(&spin, (char_u *)_("Done!"));
vim_snprintf((char *)IObuff, IOSIZE,
_("Estimated runtime memory use: %d bytes"), spin.si_memtot);
spell_message(&spin, IObuff);
if (!error)
spell_reload_one(wfname, added_word);
}
ga_clear(&spin.si_rep);
ga_clear(&spin.si_repsal);
ga_clear(&spin.si_sal);
ga_clear(&spin.si_map);
ga_clear(&spin.si_comppat);
ga_clear(&spin.si_prefcond);
hash_clear_all(&spin.si_commonwords, 0);
for (i = 0; i < incount; ++i)
if (afile[i] != NULL)
spell_free_aff(afile[i]);
free_blocks(spin.si_blocks);
if (spin.si_sugtime != 0 && !error && !got_int)
spell_make_sugfile(&spin, wfname);
}
theend:
xfree(fname);
xfree(wfname);
}
static void spell_message(spellinfo_T *spin, char_u *str)
{
if (spin->si_verbose || p_verbose > 2) {
if (!spin->si_verbose)
verbose_enter();
MSG(str);
ui_flush();
if (!spin->si_verbose)
verbose_leave();
}
}
void ex_spell(exarg_T *eap)
{
spell_add_word(eap->arg, (int)STRLEN(eap->arg), eap->cmdidx == CMD_spellwrong,
eap->forceit ? 0 : (int)eap->line2,
eap->cmdidx == CMD_spellundo);
}
void
spell_add_word (
char_u *word,
int len,
int bad,
int idx, 
bool undo 
)
{
FILE *fd = NULL;
buf_T *buf = NULL;
bool new_spf = false;
char_u *fname;
char_u *fnamebuf = NULL;
char_u line[MAXWLEN * 2];
long fpos, fpos_next = 0;
int i;
char_u *spf;
if (idx == 0) { 
if (int_wordlist == NULL) {
int_wordlist = vim_tempname();
if (int_wordlist == NULL)
return;
}
fname = int_wordlist;
} else {
if (*curwin->w_s->b_p_spf == NUL) {
init_spellfile();
new_spf = true;
}
if (*curwin->w_s->b_p_spf == NUL) {
EMSG2(_(e_notset), "spellfile");
return;
}
fnamebuf = xmalloc(MAXPATHL);
for (spf = curwin->w_s->b_p_spf, i = 1; *spf != NUL; ++i) {
copy_option_part(&spf, fnamebuf, MAXPATHL, ",");
if (i == idx)
break;
if (*spf == NUL) {
EMSGN(_("E765: 'spellfile' does not have %" PRId64 " entries"), idx);
xfree(fnamebuf);
return;
}
}
buf = buflist_findname_exp(fnamebuf);
if (buf != NULL && buf->b_ml.ml_mfp == NULL)
buf = NULL;
if (buf != NULL && bufIsChanged(buf)) {
EMSG(_(e_bufloaded));
xfree(fnamebuf);
return;
}
fname = fnamebuf;
}
if (bad || undo) {
fd = os_fopen((char *)fname, "r");
if (fd != NULL) {
while (!vim_fgets(line, MAXWLEN * 2, fd)) {
fpos = fpos_next;
fpos_next = ftell(fd);
if (STRNCMP(word, line, len) == 0
&& (line[len] == '/' || line[len] < ' ')) {
fclose(fd);
fd = os_fopen((char *)fname, "r+");
if (fd == NULL) {
break;
}
if (fseek(fd, fpos, SEEK_SET) == 0) {
fputc('#', fd);
if (undo) {
home_replace(NULL, fname, NameBuff, MAXPATHL, TRUE);
smsg(_("Word '%.*s' removed from %s"),
len, word, NameBuff);
}
}
if (fseek(fd, fpos_next, SEEK_SET) <= 0) {
break;
}
}
}
if (fd != NULL)
fclose(fd);
}
}
if (!undo) {
fd = os_fopen((char *)fname, "a");
if (fd == NULL && new_spf) {
char_u *p;
if (!dir_of_file_exists(fname) && (p = path_tail_with_sep(fname)) != fname) {
int c = *p;
*p = NUL;
os_mkdir((char *)fname, 0755);
*p = c;
fd = os_fopen((char *)fname, "a");
}
}
if (fd == NULL)
EMSG2(_(e_notopen), fname);
else {
if (bad)
fprintf(fd, "%.*s/!\n", len, word);
else
fprintf(fd, "%.*s\n", len, word);
fclose(fd);
home_replace(NULL, fname, NameBuff, MAXPATHL, TRUE);
smsg(_("Word '%.*s' added to %s"), len, word, NameBuff);
}
}
if (fd != NULL) {
mkspell(1, &fname, false, true, true);
if (buf != NULL)
buf_reload(buf, buf->b_orig_mode);
redraw_all_later(SOME_VALID);
}
xfree(fnamebuf);
}
static void init_spellfile(void)
{
char_u *buf;
int l;
char_u *fname;
char_u *rtp;
char_u *lend;
bool aspath = false;
char_u *lstart = curbuf->b_s.b_p_spl;
if (*curwin->w_s->b_p_spl != NUL && !GA_EMPTY(&curwin->w_s->b_langp)) {
buf = xmalloc(MAXPATHL);
for (lend = curwin->w_s->b_p_spl; *lend != NUL
&& vim_strchr((char_u *)",._", *lend) == NULL; ++lend)
if (vim_ispathsep(*lend)) {
aspath = true;
lstart = lend + 1;
}
rtp = p_rtp;
while (*rtp != NUL) {
if (aspath)
STRLCPY(buf, curbuf->b_s.b_p_spl,
lstart - curbuf->b_s.b_p_spl);
else
copy_option_part(&rtp, buf, MAXPATHL, ",");
if (os_file_is_writable((char *)buf) == 2) {
if (aspath)
STRLCPY(buf, curbuf->b_s.b_p_spl,
lend - curbuf->b_s.b_p_spl + 1);
else {
l = (int)STRLEN(buf);
vim_snprintf((char *)buf + l, MAXPATHL - l, "/spell");
if (os_file_is_writable((char *)buf) != 2) {
os_mkdir((char *)buf, 0755);
}
l = (int)STRLEN(buf);
vim_snprintf((char *)buf + l, MAXPATHL - l,
"/%.*s", (int)(lend - lstart), lstart);
}
l = (int)STRLEN(buf);
fname = LANGP_ENTRY(curwin->w_s->b_langp, 0)
->lp_slang->sl_fname;
vim_snprintf((char *)buf + l, MAXPATHL - l, ".%s.add",
((fname != NULL
&& strstr((char *)path_tail(fname), ".ascii.") != NULL)
? "ascii"
: (const char *)spell_enc()));
set_option_value("spellfile", 0L, (const char *)buf, OPT_LOCAL);
break;
}
aspath = false;
}
xfree(buf);
}
}
static void
set_spell_charflags (
char_u *flags,
int cnt, 
char_u *fol
)
{
spelltab_T new_st;
int i;
char_u *p = fol;
int c;
clear_spell_chartab(&new_st);
for (i = 0; i < 128; ++i) {
if (i < cnt) {
new_st.st_isw[i + 128] = (flags[i] & CF_WORD) != 0;
new_st.st_isu[i + 128] = (flags[i] & CF_UPPER) != 0;
}
if (*p != NUL) {
c = mb_ptr2char_adv((const char_u **)&p);
new_st.st_fold[i + 128] = c;
if (i + 128 != c && new_st.st_isu[i + 128] && c < 256)
new_st.st_upper[c] = i + 128;
}
}
(void)set_spell_finish(&new_st);
}
static int set_spell_finish(spelltab_T *new_st)
{
int i;
if (did_set_spelltab) {
for (i = 0; i < 256; ++i) {
if (spelltab.st_isw[i] != new_st->st_isw[i]
|| spelltab.st_isu[i] != new_st->st_isu[i]
|| spelltab.st_fold[i] != new_st->st_fold[i]
|| spelltab.st_upper[i] != new_st->st_upper[i]) {
EMSG(_("E763: Word characters differ between spell files"));
return FAIL;
}
}
} else {
spelltab = *new_st;
did_set_spelltab = true;
}
return OK;
}
static int write_spell_prefcond(FILE *fd, garray_T *gap)
{
assert(gap->ga_len >= 0);
if (fd != NULL)
put_bytes(fd, (uintmax_t)gap->ga_len, 2); 
size_t totlen = 2 + (size_t)gap->ga_len; 
size_t x = 1; 
for (int i = 0; i < gap->ga_len; ++i) {
char_u *p = ((char_u **)gap->ga_data)[i];
if (p != NULL) {
size_t len = STRLEN(p);
if (fd != NULL) {
assert(len <= INT_MAX);
fputc((int)len, fd);
x &= fwrite(p, len, 1, fd);
}
totlen += len;
} else if (fd != NULL)
fputc(0, fd);
}
assert(totlen <= INT_MAX);
return (int)totlen;
}
static void set_map_str(slang_T *lp, char_u *map)
{
char_u *p;
int headc = 0;
int c;
int i;
if (*map == NUL) {
lp->sl_has_map = false;
return;
}
lp->sl_has_map = true;
for (i = 0; i < 256; ++i)
lp->sl_map_array[i] = 0;
hash_init(&lp->sl_map_hash);
for (p = map; *p != NUL; ) {
c = mb_cptr2char_adv((const char_u **)&p);
if (c == '/') {
headc = 0;
} else {
if (headc == 0) {
headc = c;
}
if (c >= 256) {
int cl = mb_char2len(c);
int headcl = mb_char2len(headc);
char_u *b;
hash_T hash;
hashitem_T *hi;
b = xmalloc(cl + headcl + 2);
utf_char2bytes(c, b);
b[cl] = NUL;
utf_char2bytes(headc, b + cl + 1);
b[cl + 1 + headcl] = NUL;
hash = hash_hash(b);
hi = hash_lookup(&lp->sl_map_hash, (const char *)b, STRLEN(b), hash);
if (HASHITEM_EMPTY(hi)) {
hash_add_item(&lp->sl_map_hash, hi, b, hash);
} else {
EMSG(_("E783: duplicate char in MAP entry"));
xfree(b);
}
} else
lp->sl_map_array[c] = headc;
}
}
}
