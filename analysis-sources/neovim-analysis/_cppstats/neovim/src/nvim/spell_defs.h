#include <stdbool.h>
#include <stdint.h>
#include "nvim/buffer_defs.h"
#include "nvim/garray.h"
#include "nvim/regexp_defs.h"
#include "nvim/types.h"
#define MAXWLEN 254 
#define MAXREGIONS 8
typedef int idx_T;
#define SPL_FNAME_TMPL "%s.%s.spl"
#define SPL_FNAME_ADD ".add."
#define SPL_FNAME_ASCII ".ascii."
#define WF_REGION 0x01 
#define WF_ONECAP 0x02 
#define WF_ALLCAP 0x04 
#define WF_RARE 0x08 
#define WF_BANNED 0x10 
#define WF_AFX 0x20 
#define WF_FIXCAP 0x40 
#define WF_KEEPCAP 0x80 
#define WF_HAS_AFF 0x0100 
#define WF_NEEDCOMP 0x0200 
#define WF_NOSUGGEST 0x0400 
#define WF_COMPROOT 0x0800 
#define WF_NOCOMPBEF 0x1000 
#define WF_NOCOMPAFT 0x2000 
#define WFP_RARE 0x01 
#define WFP_NC 0x02 
#define WFP_UP 0x04 
#define WFP_COMPPERMIT 0x08 
#define WFP_COMPFORBID 0x10 
#define WF_RAREPFX (WFP_RARE << 24) 
#define WF_PFX_NC (WFP_NC << 24) 
#define WF_PFX_UP (WFP_UP << 24) 
#define WF_PFX_COMPPERMIT (WFP_COMPPERMIT << 24) 
#define WF_PFX_COMPFORBID (WFP_COMPFORBID << 24) 
#define COMP_CHECKDUP 1 
#define COMP_CHECKREP 2 
#define COMP_CHECKCASE 4 
#define COMP_CHECKTRIPLE 8 
typedef struct fromto_S {
char_u *ft_from;
char_u *ft_to;
} fromto_T;
typedef struct salitem_S {
char_u *sm_lead; 
int sm_leadlen; 
char_u *sm_oneof; 
char_u *sm_rules; 
char_u *sm_to; 
int *sm_lead_w; 
int *sm_oneof_w; 
int *sm_to_w; 
} salitem_T;
typedef int salfirst_T;
#define SP_TRUNCERROR -1 
#define SP_FORMERROR -2 
#define SP_OTHERERROR -3 
typedef struct slang_S slang_T;
struct slang_S {
slang_T *sl_next; 
char_u *sl_name; 
char_u *sl_fname; 
bool sl_add; 
char_u *sl_fbyts; 
idx_T *sl_fidxs; 
char_u *sl_kbyts; 
idx_T *sl_kidxs; 
char_u *sl_pbyts; 
idx_T *sl_pidxs; 
char_u *sl_info; 
char_u sl_regions[MAXREGIONS * 2 + 1];
char_u *sl_midword; 
hashtab_T sl_wordcount; 
int sl_compmax; 
int sl_compminlen; 
int sl_compsylmax; 
int sl_compoptions; 
garray_T sl_comppat; 
regprog_T *sl_compprog; 
char_u *sl_comprules; 
char_u *sl_compstartflags; 
char_u *sl_compallflags; 
bool sl_nobreak; 
char_u *sl_syllable; 
garray_T sl_syl_items; 
int sl_prefixcnt; 
regprog_T **sl_prefprog; 
garray_T sl_rep; 
int16_t sl_rep_first[256]; 
garray_T sl_sal; 
salfirst_T sl_sal_first[256]; 
bool sl_followup; 
bool sl_collapse; 
bool sl_rem_accents; 
bool sl_sofo; 
garray_T sl_repsal; 
int16_t sl_repsal_first[256]; 
bool sl_nosplitsugs; 
bool sl_nocompoundsugs; 
time_t sl_sugtime; 
char_u *sl_sbyts; 
idx_T *sl_sidxs; 
buf_T *sl_sugbuf; 
bool sl_sugloaded; 
bool sl_has_map; 
hashtab_T sl_map_hash; 
int sl_map_array[256]; 
hashtab_T sl_sounddone; 
};
typedef struct langp_S {
slang_T *lp_slang; 
slang_T *lp_sallang; 
slang_T *lp_replang; 
int lp_region; 
} langp_T;
#define LANGP_ENTRY(ga, i) (((langp_T *)(ga).ga_data) + (i))
#define VIMSUGMAGIC "VIMsug" 
#define VIMSUGMAGICL 6
#define VIMSUGVERSION 1
#define REGION_ALL 0xff 
typedef struct {
bool st_isw[256]; 
bool st_isu[256]; 
char_u st_fold[256]; 
char_u st_upper[256]; 
} spelltab_T;
typedef enum {
STATE_START = 0, 
STATE_NOPREFIX, 
STATE_SPLITUNDO, 
STATE_ENDNUL, 
STATE_PLAIN, 
STATE_DEL, 
STATE_INS_PREP, 
STATE_INS, 
STATE_SWAP, 
STATE_UNSWAP, 
STATE_SWAP3, 
STATE_UNSWAP3, 
STATE_UNROT3L, 
STATE_UNROT3R, 
STATE_REP_INI, 
STATE_REP, 
STATE_REP_UNDO, 
STATE_FINAL 
} state_T;
typedef struct trystate_S {
state_T ts_state; 
int ts_score; 
idx_T ts_arridx; 
short ts_curi; 
char_u ts_fidx; 
char_u ts_fidxtry; 
char_u ts_twordlen; 
char_u ts_prefixdepth; 
char_u ts_flags; 
char_u ts_tcharlen; 
char_u ts_tcharidx; 
char_u ts_isdiff; 
char_u ts_fcharstart; 
char_u ts_prewordlen; 
char_u ts_splitoff; 
char_u ts_splitfidx; 
char_u ts_complen; 
char_u ts_compsplit; 
char_u ts_save_badflags; 
char_u ts_delidx; 
} trystate_T;
#define SPELL_TOFOLD(c) ((c) >= 128 ? utf_fold(c) : (int)spelltab.st_fold[c])
#define SPELL_TOUPPER(c) ((c) >= 128 ? mb_toupper(c) : (int)spelltab.st_upper[c])
#define SPELL_ISUPPER(c) ((c) >= 128 ? mb_isupper(c) : spelltab.st_isu[c])
extern slang_T *first_lang;
extern char_u *int_wordlist;
extern spelltab_T spelltab;
extern int did_set_spelltab;
extern char *e_format;
