#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
typedef struct file_buffer buf_T; 
typedef struct {
buf_T *br_buf;
int br_fnum;
int br_buf_free_count;
} bufref_T;
#include "nvim/garray.h"
#include "nvim/grid_defs.h"
#include "nvim/highlight_defs.h"
#include "nvim/pos.h"
#include "nvim/option_defs.h"
#include "nvim/mark_defs.h"
#include "nvim/undo_defs.h"
#include "nvim/hashtab.h"
#include "nvim/eval/typval.h"
#include "nvim/profile.h"
#include "nvim/api/private/defs.h"
#include "nvim/map.h"
#include "nvim/lib/kvec.h"
#include "nvim/marktree.h"
#define GETFILE_SUCCESS(x) ((x) <= 0)
#define MODIFIABLE(buf) (buf->b_p_ma)
#define VALID_WROW 0x01 
#define VALID_WCOL 0x02 
#define VALID_VIRTCOL 0x04 
#define VALID_CHEIGHT 0x08 
#define VALID_CROW 0x10 
#define VALID_BOTLINE 0x20 
#define VALID_BOTLINE_AP 0x40 
#define VALID_TOPLINE 0x80 
#define BF_RECOVERED 0x01 
#define BF_CHECK_RO 0x02 
#define BF_NEVERLOADED 0x04 
#define BF_NOTEDITED 0x08 
#define BF_NEW 0x10 
#define BF_NEW_W 0x20 
#define BF_READERR 0x40 
#define BF_DUMMY 0x80 
#define BF_PRESERVED 0x100 
#define BF_WRITE_MASK (BF_NOTEDITED + BF_NEW + BF_READERR)
typedef struct window_S win_T;
typedef struct wininfo_S wininfo_T;
typedef struct frame_S frame_T;
typedef uint16_t disptick_T; 
#include "nvim/memline_defs.h"
#include "nvim/memfile_defs.h"
#include "nvim/regexp_defs.h"
#include "nvim/syntax_defs.h"
#include "nvim/sign_defs.h"
#include "nvim/os/fs_defs.h" 
#include "nvim/terminal.h" 
typedef struct taggy {
char_u *tagname; 
fmark_T fmark; 
int cur_match; 
int cur_fnum; 
char_u *user_data; 
} taggy_T;
typedef struct buffblock buffblock_T;
typedef struct buffheader buffheader_T;
struct buffblock {
buffblock_T *b_next; 
char_u b_str[1]; 
};
struct buffheader {
buffblock_T bh_first; 
buffblock_T *bh_curr; 
size_t bh_index; 
size_t bh_space; 
};
typedef struct
{
buffheader_T sr_redobuff;
buffheader_T sr_old_redobuff;
} save_redo_T;
typedef struct {
int wo_arab;
#define w_p_arab w_onebuf_opt.wo_arab 
int wo_bri;
#define w_p_bri w_onebuf_opt.wo_bri 
char_u *wo_briopt;
#define w_p_briopt w_onebuf_opt.wo_briopt 
int wo_diff;
#define w_p_diff w_onebuf_opt.wo_diff 
char_u *wo_fdc;
#define w_p_fdc w_onebuf_opt.wo_fdc 
char_u *wo_fdc_save;
#define w_p_fdc_save w_onebuf_opt.wo_fdc_save 
int wo_fen;
#define w_p_fen w_onebuf_opt.wo_fen 
int wo_fen_save;
#define w_p_fen_save w_onebuf_opt.wo_fen_save 
char_u *wo_fdi;
#define w_p_fdi w_onebuf_opt.wo_fdi 
long wo_fdl;
#define w_p_fdl w_onebuf_opt.wo_fdl 
int wo_fdl_save;
#define w_p_fdl_save w_onebuf_opt.wo_fdl_save 
char_u *wo_fdm;
#define w_p_fdm w_onebuf_opt.wo_fdm 
char_u *wo_fdm_save;
#define w_p_fdm_save w_onebuf_opt.wo_fdm_save 
long wo_fml;
#define w_p_fml w_onebuf_opt.wo_fml 
long wo_fdn;
#define w_p_fdn w_onebuf_opt.wo_fdn 
char_u *wo_fde;
#define w_p_fde w_onebuf_opt.wo_fde 
char_u *wo_fdt;
#define w_p_fdt w_onebuf_opt.wo_fdt 
char_u *wo_fmr;
#define w_p_fmr w_onebuf_opt.wo_fmr 
int wo_lbr;
#define w_p_lbr w_onebuf_opt.wo_lbr 
int wo_list;
#define w_p_list w_onebuf_opt.wo_list 
int wo_nu;
#define w_p_nu w_onebuf_opt.wo_nu 
int wo_rnu;
#define w_p_rnu w_onebuf_opt.wo_rnu 
long wo_nuw;
#define w_p_nuw w_onebuf_opt.wo_nuw 
int wo_wfh;
#define w_p_wfh w_onebuf_opt.wo_wfh 
int wo_wfw;
#define w_p_wfw w_onebuf_opt.wo_wfw 
int wo_pvw;
#define w_p_pvw w_onebuf_opt.wo_pvw 
int wo_rl;
#define w_p_rl w_onebuf_opt.wo_rl 
char_u *wo_rlc;
#define w_p_rlc w_onebuf_opt.wo_rlc 
long wo_scr;
#define w_p_scr w_onebuf_opt.wo_scr 
int wo_spell;
#define w_p_spell w_onebuf_opt.wo_spell 
int wo_cuc;
#define w_p_cuc w_onebuf_opt.wo_cuc 
int wo_cul;
#define w_p_cul w_onebuf_opt.wo_cul 
char_u *wo_cc;
#define w_p_cc w_onebuf_opt.wo_cc 
char_u *wo_stl;
#define w_p_stl w_onebuf_opt.wo_stl 
int wo_scb;
#define w_p_scb w_onebuf_opt.wo_scb 
int wo_diff_saved; 
#define w_p_diff_saved w_onebuf_opt.wo_diff_saved
int wo_scb_save; 
#define w_p_scb_save w_onebuf_opt.wo_scb_save
int wo_wrap;
#define w_p_wrap w_onebuf_opt.wo_wrap 
int wo_wrap_save; 
#define w_p_wrap_save w_onebuf_opt.wo_wrap_save
char_u *wo_cocu; 
#define w_p_cocu w_onebuf_opt.wo_cocu
long wo_cole; 
#define w_p_cole w_onebuf_opt.wo_cole
int wo_crb;
#define w_p_crb w_onebuf_opt.wo_crb 
int wo_crb_save; 
#define w_p_crb_save w_onebuf_opt.wo_crb_save
char_u *wo_scl;
#define w_p_scl w_onebuf_opt.wo_scl 
char_u *wo_winhl;
#define w_p_winhl w_onebuf_opt.wo_winhl 
char_u *wo_fcs;
#define w_p_fcs w_onebuf_opt.wo_fcs 
char_u *wo_lcs;
#define w_p_lcs w_onebuf_opt.wo_lcs 
long wo_winbl;
#define w_p_winbl w_onebuf_opt.wo_winbl 
LastSet wo_script_ctx[WV_COUNT]; 
#define w_p_script_ctx w_onebuf_opt.wo_script_ctx
} winopt_T;
struct wininfo_S {
wininfo_T *wi_next; 
wininfo_T *wi_prev; 
win_T *wi_win; 
pos_T wi_fpos; 
bool wi_optset; 
winopt_T wi_opt; 
bool wi_fold_manual; 
garray_T wi_folds; 
};
typedef struct arglist {
garray_T al_ga; 
int al_refcount; 
int id; 
} alist_T;
typedef struct argentry {
char_u *ae_fname; 
int ae_fnum; 
} aentry_T;
#define ALIST(win) (win)->w_alist
#define GARGLIST ((aentry_T *)global_alist.al_ga.ga_data)
#define ARGLIST ((aentry_T *)ALIST(curwin)->al_ga.ga_data)
#define WARGLIST(wp) ((aentry_T *)ALIST(wp)->al_ga.ga_data)
#define AARGLIST(al) ((aentry_T *)((al)->al_ga.ga_data))
#define GARGCOUNT (global_alist.al_ga.ga_len)
#define ARGCOUNT (ALIST(curwin)->al_ga.ga_len)
#define WARGCOUNT(wp) (ALIST(wp)->al_ga.ga_len)
typedef struct {
char_u *tb_buf; 
char_u *tb_noremap; 
int tb_buflen; 
int tb_off; 
int tb_len; 
int tb_maplen; 
int tb_silent; 
int tb_no_abbr_cnt; 
int tb_change_cnt; 
} typebuf_T;
typedef struct {
typebuf_T save_typebuf;
int typebuf_valid; 
int old_char;
int old_mod_mask;
buffheader_T save_readbuf1;
buffheader_T save_readbuf2;
String save_inputbuf;
} tasave_T;
typedef struct mapblock mapblock_T;
struct mapblock {
mapblock_T *m_next; 
char_u *m_keys; 
char_u *m_str; 
char_u *m_orig_str; 
int m_keylen; 
int m_mode; 
int m_noremap; 
char m_silent; 
char m_nowait; 
char m_expr; 
sctx_T m_script_ctx; 
};
struct stl_hlrec {
char_u *start;
int userhl; 
};
#define SYNSPL_DEFAULT 0 
#define SYNSPL_TOP 1 
#define SYNSPL_NOTOP 2 
#define B_SPELL(buf) ((buf)->b_spell)
typedef struct qf_info_S qf_info_T;
typedef struct {
proftime_T total; 
proftime_T slowest; 
long count; 
long match; 
} syn_time_T;
typedef struct {
hashtab_T b_keywtab; 
hashtab_T b_keywtab_ic; 
int b_syn_error; 
bool b_syn_slow; 
int b_syn_ic; 
int b_syn_spell; 
garray_T b_syn_patterns; 
garray_T b_syn_clusters; 
int b_spell_cluster_id; 
int b_nospell_cluster_id; 
int b_syn_containedin; 
int b_syn_sync_flags; 
int16_t b_syn_sync_id; 
long b_syn_sync_minlines; 
long b_syn_sync_maxlines; 
long b_syn_sync_linebreaks; 
char_u *b_syn_linecont_pat; 
regprog_T *b_syn_linecont_prog; 
syn_time_T b_syn_linecont_time;
int b_syn_linecont_ic; 
int b_syn_topgrp; 
int b_syn_conceal; 
int b_syn_folditems; 
synstate_T *b_sst_array;
int b_sst_len;
synstate_T *b_sst_first;
synstate_T *b_sst_firstfree;
int b_sst_freecount;
linenr_T b_sst_check_lnum;
disptick_T b_sst_lasttick; 
garray_T b_langp; 
bool b_spell_ismw[256]; 
char_u *b_spell_ismw_mb; 
char_u *b_p_spc; 
regprog_T *b_cap_prog; 
char_u *b_p_spf; 
char_u *b_p_spl; 
int b_cjk; 
char_u b_syn_chartab[32]; 
char_u *b_syn_isk; 
} synblock_T;
typedef TV_DICTITEM_STRUCT(sizeof("changedtick")) ChangedtickDictItem;
typedef struct {
LuaRef on_lines;
LuaRef on_bytes;
LuaRef on_changedtick;
LuaRef on_detach;
bool utf_sizes;
} BufUpdateCallbacks;
#define BUF_UPDATE_CALLBACKS_INIT { LUA_NOREF, LUA_NOREF, LUA_NOREF, LUA_NOREF, false }
EXTERN int curbuf_splice_pending INIT(= 0);
#define BUF_HAS_QF_ENTRY 1
#define BUF_HAS_LL_ENTRY 2
#define MAX_MAPHASH 256
struct file_buffer {
handle_T handle; 
#define b_fnum handle
memline_T b_ml; 
buf_T *b_next; 
buf_T *b_prev;
int b_nwindows; 
int b_flags; 
int b_locked; 
char_u *b_ffname; 
char_u *b_sfname; 
char_u *b_fname; 
bool file_id_valid;
FileID file_id;
int b_changed; 
ChangedtickDictItem changedtick_di;
varnumber_T b_last_changedtick; 
varnumber_T b_last_changedtick_pum; 
bool b_saving; 
bool b_mod_set; 
linenr_T b_mod_top; 
linenr_T b_mod_bot; 
long b_mod_xlines; 
wininfo_T *b_wininfo; 
long b_mtime; 
long b_mtime_read; 
uint64_t b_orig_size; 
int b_orig_mode; 
fmark_T b_namedm[NMARKS]; 
visualinfo_T b_visual;
int b_visual_mode_eval; 
fmark_T b_last_cursor; 
fmark_T b_last_insert; 
fmark_T b_last_change; 
fmark_T b_changelist[JUMPLISTSIZE];
int b_changelistlen; 
bool b_new_change; 
uint64_t b_chartab[4];
mapblock_T *(b_maphash[MAX_MAPHASH]);
mapblock_T *b_first_abbr;
garray_T b_ucmds;
pos_T b_op_start;
pos_T b_op_start_orig; 
pos_T b_op_end;
bool b_marks_read; 
u_header_T *b_u_oldhead; 
u_header_T *b_u_newhead; 
u_header_T *b_u_curhead; 
int b_u_numhead; 
bool b_u_synced; 
long b_u_seq_last; 
long b_u_save_nr_last; 
long b_u_seq_cur; 
time_t b_u_time_cur; 
long b_u_save_nr_cur; 
char_u *b_u_line_ptr; 
linenr_T b_u_line_lnum; 
colnr_T b_u_line_colnr; 
bool b_scanned; 
long b_p_iminsert; 
long b_p_imsearch; 
#define B_IMODE_USE_INSERT -1 
#define B_IMODE_NONE 0 
#define B_IMODE_LMAP 1 
#define B_IMODE_LAST 1
short b_kmap_state; 
#define KEYMAP_INIT 1 
#define KEYMAP_LOADED 2 
garray_T b_kmap_ga; 
bool b_p_initialized; 
LastSet b_p_script_ctx[BV_COUNT]; 
int b_p_ai; 
int b_p_ai_nopaste; 
char_u *b_p_bkc; 
unsigned int b_bkc_flags; 
int b_p_ci; 
int b_p_bin; 
int b_p_bomb; 
char_u *b_p_bh; 
char_u *b_p_bt; 
int b_has_qf_entry; 
int b_p_bl; 
long b_p_channel; 
int b_p_cin; 
char_u *b_p_cino; 
char_u *b_p_cink; 
char_u *b_p_cinw; 
char_u *b_p_com; 
char_u *b_p_cms; 
char_u *b_p_cpt; 
char_u *b_p_cfu; 
char_u *b_p_ofu; 
char_u *b_p_tfu; 
int b_p_eol; 
int b_p_fixeol; 
int b_p_et; 
int b_p_et_nobin; 
int b_p_et_nopaste; 
char_u *b_p_fenc; 
char_u *b_p_ff; 
char_u *b_p_ft; 
char_u *b_p_fo; 
char_u *b_p_flp; 
int b_p_inf; 
char_u *b_p_isk; 
char_u *b_p_def; 
char_u *b_p_inc; 
char_u *b_p_inex; 
uint32_t b_p_inex_flags; 
char_u *b_p_inde; 
uint32_t b_p_inde_flags; 
char_u *b_p_indk; 
char_u *b_p_fp; 
char_u *b_p_fex; 
uint32_t b_p_fex_flags; 
char_u *b_p_kp; 
int b_p_lisp; 
char_u *b_p_menc; 
char_u *b_p_mps; 
int b_p_ml; 
int b_p_ml_nobin; 
int b_p_ma; 
char_u *b_p_nf; 
int b_p_pi; 
char_u *b_p_qe; 
int b_p_ro; 
long b_p_sw; 
long b_p_scbk; 
int b_p_si; 
long b_p_sts; 
long b_p_sts_nopaste; 
char_u *b_p_sua; 
int b_p_swf; 
long b_p_smc; 
char_u *b_p_syn; 
long b_p_ts; 
long b_p_tw; 
long b_p_tw_nobin; 
long b_p_tw_nopaste; 
long b_p_wm; 
long b_p_wm_nobin; 
long b_p_wm_nopaste; 
char_u *b_p_keymap; 
char_u *b_p_gp; 
char_u *b_p_mp; 
char_u *b_p_efm; 
char_u *b_p_ep; 
char_u *b_p_path; 
int b_p_ar; 
char_u *b_p_tags; 
char_u *b_p_tc; 
unsigned b_tc_flags; 
char_u *b_p_dict; 
char_u *b_p_tsr; 
long b_p_ul; 
int b_p_udf; 
char_u *b_p_lw; 
int b_ind_level;
int b_ind_open_imag;
int b_ind_no_brace;
int b_ind_first_open;
int b_ind_open_extra;
int b_ind_close_extra;
int b_ind_open_left_imag;
int b_ind_jump_label;
int b_ind_case;
int b_ind_case_code;
int b_ind_case_break;
int b_ind_param;
int b_ind_func_type;
int b_ind_comment;
int b_ind_in_comment;
int b_ind_in_comment2;
int b_ind_cpp_baseclass;
int b_ind_continuation;
int b_ind_unclosed;
int b_ind_unclosed2;
int b_ind_unclosed_noignore;
int b_ind_unclosed_wrapped;
int b_ind_unclosed_whiteok;
int b_ind_matching_paren;
int b_ind_paren_prev;
int b_ind_maxparen;
int b_ind_maxcomment;
int b_ind_scopedecl;
int b_ind_scopedecl_code;
int b_ind_java;
int b_ind_js;
int b_ind_keep_case_label;
int b_ind_hash_comment;
int b_ind_cpp_namespace;
int b_ind_if_for_while;
int b_ind_cpp_extern_c;
linenr_T b_no_eol_lnum; 
int b_start_eol; 
int b_start_ffc; 
char_u *b_start_fenc; 
int b_bad_char; 
int b_start_bomb; 
ScopeDictDictItem b_bufvar; 
dict_T *b_vars; 
bool b_may_swap;
bool b_did_warn; 
bool b_help; 
bool b_spell; 
char_u *b_prompt_text; 
Callback b_prompt_callback; 
Callback b_prompt_interrupt; 
int b_prompt_insert; 
synblock_T b_s; 
signlist_T *b_signlist; 
int b_signcols_max; 
int b_signcols; 
Terminal *terminal; 
dict_T *additional_data; 
int b_mapped_ctrl_c; 
MarkTree b_marktree[1];
Map(uint64_t, ExtmarkItem) *b_extmark_index;
Map(uint64_t, ExtmarkNs) *b_extmark_ns; 
kvec_t(uint64_t) update_channels;
kvec_t(BufUpdateCallbacks) update_callbacks;
bool update_need_codepoints;
size_t deleted_bytes;
size_t deleted_codepoints;
size_t deleted_codeunits;
int flush_count;
bool b_luahl;
LuaRef b_luahl_start;
LuaRef b_luahl_window;
LuaRef b_luahl_line;
LuaRef b_luahl_end;
int b_diff_failed; 
};
#define DB_COUNT 8 
typedef struct diffblock_S diff_T;
struct diffblock_S {
diff_T *df_next;
linenr_T df_lnum[DB_COUNT]; 
linenr_T df_count[DB_COUNT]; 
};
#define SNAP_HELP_IDX 0
#define SNAP_AUCMD_IDX 1
#define SNAP_COUNT 2
typedef struct tabpage_S tabpage_T;
struct tabpage_S {
handle_T handle;
tabpage_T *tp_next; 
frame_T *tp_topframe; 
win_T *tp_curwin; 
win_T *tp_prevwin; 
win_T *tp_firstwin; 
win_T *tp_lastwin; 
long tp_old_Rows; 
long tp_old_Columns; 
long tp_ch_used; 
diff_T *tp_first_diff;
buf_T *(tp_diffbuf[DB_COUNT]);
int tp_diff_invalid; 
int tp_diff_update; 
frame_T *(tp_snapshot[SNAP_COUNT]); 
ScopeDictDictItem tp_winvar; 
dict_T *tp_vars; 
char_u *tp_localdir; 
};
typedef struct w_line {
linenr_T wl_lnum; 
uint16_t wl_size; 
char wl_valid; 
char wl_folded; 
linenr_T wl_lastlnum; 
} wline_T;
struct frame_S {
char fr_layout; 
int fr_width;
int fr_newwidth; 
int fr_height;
int fr_newheight; 
frame_T *fr_parent; 
frame_T *fr_next; 
frame_T *fr_prev; 
frame_T *fr_child; 
win_T *fr_win; 
};
#define FR_LEAF 0 
#define FR_ROW 1 
#define FR_COL 2 
typedef struct {
regmmatch_T rm; 
buf_T *buf; 
linenr_T lnum; 
int attr; 
int attr_cur; 
linenr_T first_lnum; 
colnr_T startcol; 
colnr_T endcol; 
bool is_addpos; 
proftime_T tm; 
} match_T;
#define MAXPOSMATCH 8
typedef struct
{
linenr_T lnum; 
colnr_T col; 
int len; 
} llpos_T;
typedef struct posmatch posmatch_T;
struct posmatch
{
llpos_T pos[MAXPOSMATCH]; 
int cur; 
linenr_T toplnum; 
linenr_T botlnum; 
};
typedef struct matchitem matchitem_T;
struct matchitem {
matchitem_T *next;
int id; 
int priority; 
char_u *pattern; 
int hlg_id; 
regmmatch_T match; 
posmatch_T pos; 
match_T hl; 
int conceal_char; 
};
typedef int FloatAnchor;
enum {
kFloatAnchorEast = 1,
kFloatAnchorSouth = 2,
};
EXTERN const char *const float_anchor_str[] INIT(= { "NW", "NE", "SW", "SE" });
typedef enum {
kFloatRelativeEditor = 0,
kFloatRelativeWindow = 1,
kFloatRelativeCursor = 2,
} FloatRelative;
EXTERN const char *const float_relative_str[] INIT(= { "editor", "win",
"cursor" });
typedef enum {
kWinStyleUnused = 0,
kWinStyleMinimal, 
} WinStyle;
typedef struct {
Window window;
lpos_T bufpos;
int height, width;
double row, col;
FloatAnchor anchor;
FloatRelative relative;
bool external;
bool focusable;
WinStyle style;
} FloatConfig;
#define FLOAT_CONFIG_INIT ((FloatConfig){ .height = 0, .width = 0, .bufpos = { -1, 0 }, .row = 0, .col = 0, .anchor = 0, .relative = 0, .external = false, .focusable = true, .style = kWinStyleUnused })
typedef struct
{
int w_topline_save; 
int w_topline_corr; 
pos_T w_cursor_save; 
pos_T w_cursor_corr; 
} pos_save_T;
struct window_S {
handle_T handle; 
buf_T *w_buffer; 
synblock_T *w_s; 
int w_hl_id_normal; 
int w_hl_attr_normal; 
int w_hl_ids[HLF_COUNT]; 
int w_hl_attrs[HLF_COUNT]; 
int w_hl_needs_update; 
win_T *w_prev; 
win_T *w_next; 
bool w_closing; 
frame_T *w_frame; 
pos_T w_cursor; 
colnr_T w_curswant; 
int w_set_curswant; 
linenr_T w_last_cursorline; 
pos_T w_last_cursormoved; 
char w_old_visual_mode; 
linenr_T w_old_cursor_lnum; 
colnr_T w_old_cursor_fcol; 
colnr_T w_old_cursor_lcol; 
linenr_T w_old_visual_lnum; 
colnr_T w_old_visual_col; 
colnr_T w_old_curswant; 
struct {
int eol;
int ext;
int prec;
int nbsp;
int space;
int tab1; 
int tab2; 
int tab3; 
int trail;
int conceal;
} w_p_lcs_chars;
struct {
int stl;
int stlnc;
int vert;
int fold;
int foldopen; 
int foldclosed; 
int foldsep; 
int diff;
int msgsep;
int eob;
} w_p_fcs_chars;
linenr_T w_topline; 
char w_topline_was_set; 
int w_topfill; 
int w_old_topfill; 
bool w_botfill; 
bool w_old_botfill; 
colnr_T w_leftcol; 
colnr_T w_skipcol; 
int w_winrow; 
int w_height; 
int w_status_height; 
int w_wincol; 
int w_width; 
int w_vsep_width; 
pos_save_T w_save_cursor; 
int w_height_inner;
int w_width_inner;
int w_height_request;
int w_width_request;
int w_valid;
pos_T w_valid_cursor; 
colnr_T w_valid_leftcol; 
bool w_viewport_invalid;
int w_cline_height; 
bool w_cline_folded; 
int w_cline_row; 
colnr_T w_virtcol; 
int w_wrow, w_wcol; 
linenr_T w_botline; 
int w_empty_rows; 
int w_filler_rows; 
int w_lines_valid; 
wline_T *w_lines;
garray_T w_folds; 
bool w_fold_manual; 
bool w_foldinvalid; 
int w_nrwidth; 
int w_redr_type; 
int w_upd_rows; 
linenr_T w_redraw_top; 
linenr_T w_redraw_bot; 
int w_redr_status; 
pos_T w_ru_cursor; 
colnr_T w_ru_virtcol; 
linenr_T w_ru_topline; 
linenr_T w_ru_line_count; 
int w_ru_topfill; 
char w_ru_empty; 
int w_alt_fnum; 
alist_T *w_alist; 
int w_arg_idx; 
int w_arg_idx_invalid; 
char_u *w_localdir; 
winopt_T w_onebuf_opt;
winopt_T w_allbuf_opt;
uint32_t w_p_stl_flags; 
uint32_t w_p_fde_flags; 
uint32_t w_p_fdt_flags; 
int *w_p_cc_cols; 
int w_p_brimin; 
int w_p_brishift; 
bool w_p_brisbr; 
long w_p_siso; 
long w_p_so; 
#define GLOBAL_WO(p) ((char *)p + sizeof(winopt_T))
long w_scbind_pos;
ScopeDictDictItem w_winvar; 
dict_T *w_vars; 
pos_T w_pcmark; 
pos_T w_prev_pcmark; 
xfmark_T w_jumplist[JUMPLISTSIZE];
int w_jumplistlen; 
int w_jumplistidx; 
int w_changelistidx; 
matchitem_T *w_match_head; 
int w_next_match_id; 
taggy_T w_tagstack[TAGSTACKSIZE]; 
int w_tagstackidx; 
int w_tagstacklen; 
ScreenGrid w_grid; 
bool w_pos_changed; 
bool w_floating; 
FloatConfig w_float_config;
int w_fraction;
int w_prev_fraction_row;
linenr_T w_nrwidth_line_count; 
int w_nrwidth_width; 
qf_info_T *w_llist; 
qf_info_T *w_llist_ref;
};
static inline int win_hl_attr(win_T *wp, int hlf)
{
return wp->w_hl_attrs[hlf];
}
#define CHANGEDTICK(buf) (=== Include buffer.h & use buf_(get|set|inc)_changedtick ===)
