#include <stdbool.h>
#include <inttypes.h>
#include "nvim/macros.h"
#include "nvim/ex_eval.h"
#include "nvim/iconv.h"
#include "nvim/mbyte.h"
#include "nvim/menu.h"
#include "nvim/syntax_defs.h"
#include "nvim/types.h"
#include "nvim/event/loop.h"
#include "nvim/os/os_defs.h"
#define IOSIZE (1024+1) 
#define MSG_BUF_LEN 480 
#define MSG_BUF_CLEN (MSG_BUF_LEN / 6) 
#if defined(WIN32)
#define _PATHSEPSTR "\\"
#else
#define _PATHSEPSTR "/"
#endif
#if !defined(FILETYPE_FILE)
#define FILETYPE_FILE "filetype.vim"
#endif
#if !defined(FTPLUGIN_FILE)
#define FTPLUGIN_FILE "ftplugin.vim"
#endif
#if !defined(INDENT_FILE)
#define INDENT_FILE "indent.vim"
#endif
#if !defined(FTOFF_FILE)
#define FTOFF_FILE "ftoff.vim"
#endif
#if !defined(FTPLUGOF_FILE)
#define FTPLUGOF_FILE "ftplugof.vim"
#endif
#if !defined(INDOFF_FILE)
#define INDOFF_FILE "indoff.vim"
#endif
#define DFLT_ERRORFILE "errors.err"
#if !defined(SYS_VIMRC_FILE)
#define SYS_VIMRC_FILE "$VIM" _PATHSEPSTR "sysinit.vim"
#endif
#if !defined(DFLT_HELPFILE)
#define DFLT_HELPFILE "$VIMRUNTIME" _PATHSEPSTR "doc" _PATHSEPSTR "help.txt"
#endif
#if !defined(SYNTAX_FNAME)
#define SYNTAX_FNAME "$VIMRUNTIME" _PATHSEPSTR "syntax" _PATHSEPSTR "%s.vim"
#endif
#if !defined(EXRC_FILE)
#define EXRC_FILE ".exrc"
#endif
#if !defined(VIMRC_FILE)
#define VIMRC_FILE ".nvimrc"
#endif
EXTERN struct nvim_stats_s {
int64_t fsync;
int64_t redraw;
} g_stats INIT(= { 0, 0 });
#define NO_SCREEN 2 
#define NO_BUFFERS 1 
#define DFLT_COLS 80 
#define DFLT_ROWS 24 
EXTERN int Rows INIT(= DFLT_ROWS); 
EXTERN int Columns INIT(= DFLT_COLS); 
#if (defined(_MSC_VER) && (_MSC_VER >= 1300)) || defined(__MINGW32__)
typedef __int64 off_T;
#if defined(__MINGW32__)
#define vim_lseek lseek64
#define vim_fseek fseeko64
#define vim_ftell ftello64
#else
#define vim_lseek _lseeki64
#define vim_fseek _fseeki64
#define vim_ftell _ftelli64
#endif
#else
typedef off_t off_T;
#if defined(HAVE_FSEEKO)
#define vim_lseek lseek
#define vim_ftell ftello
#define vim_fseek fseeko
#else
#define vim_lseek lseek
#define vim_ftell ftell
#define vim_fseek(a, b, c) fseek(a, (long)b, c)
#endif
#endif
EXTERN int mod_mask INIT(= 0x0); 
EXTERN bool lua_attr_active INIT(= false);
EXTERN int cmdline_row;
EXTERN int redraw_cmdline INIT(= false); 
EXTERN int clear_cmdline INIT(= false); 
EXTERN int mode_displayed INIT(= false); 
EXTERN int cmdline_star INIT(= false); 
EXTERN int redrawing_cmdline INIT(= false); 
EXTERN int cmdline_was_last_drawn INIT(= false); 
EXTERN int exec_from_reg INIT(= false); 
EXTERN colnr_T dollar_vcol INIT(= -1);
EXTERN int compl_length INIT(= 0);
EXTERN int compl_interrupted INIT(= false);
EXTERN int compl_busy INIT(= false);
EXTERN int compl_cont_status INIT(= 0);
#define CONT_ADDING 1 
#define CONT_INTRPT (2 + 4) 
#define CONT_N_ADDS 4 
#define CONT_S_IPOS 8 
#define CONT_SOL 16 
#define CONT_LOCAL 32 
EXTERN int cmdmsg_rl INIT(= false); 
EXTERN int msg_col;
EXTERN int msg_row;
EXTERN int msg_scrolled; 
EXTERN bool msg_scrolled_ign INIT(= false);
EXTERN bool msg_did_scroll INIT(= false);
EXTERN char_u *keep_msg INIT(= NULL); 
EXTERN int keep_msg_attr INIT(= 0); 
EXTERN int keep_msg_more INIT(= false); 
EXTERN int need_fileinfo INIT(= false); 
EXTERN int msg_scroll INIT(= false); 
EXTERN int msg_didout INIT(= false); 
EXTERN int msg_didany INIT(= false); 
EXTERN int msg_nowait INIT(= false); 
EXTERN int emsg_off INIT(= 0); 
EXTERN int info_message INIT(= false); 
EXTERN int msg_hist_off INIT(= false); 
EXTERN int need_clr_eos INIT(= false); 
EXTERN int emsg_skip INIT(= 0); 
EXTERN int emsg_severe INIT(= false); 
EXTERN int did_endif INIT(= false); 
EXTERN dict_T vimvardict; 
EXTERN dict_T globvardict; 
EXTERN int did_emsg; 
EXTERN bool called_vim_beep; 
EXTERN int did_emsg_syntax; 
EXTERN int called_emsg; 
EXTERN int ex_exitval INIT(= 0); 
EXTERN int emsg_on_display INIT(= false); 
EXTERN int rc_did_emsg INIT(= false); 
EXTERN int no_wait_return INIT(= 0); 
EXTERN int need_wait_return INIT(= 0); 
EXTERN int did_wait_return INIT(= false); 
EXTERN int need_maketitle INIT(= true); 
EXTERN int quit_more INIT(= false); 
EXTERN int ex_keep_indent INIT(= false); 
EXTERN int vgetc_busy INIT(= 0); 
EXTERN int didset_vim INIT(= false); 
EXTERN int didset_vimruntime INIT(= false); 
EXTERN int lines_left INIT(= -1); 
EXTERN int msg_no_more INIT(= false); 
EXTERN char_u *sourcing_name INIT(= NULL); 
EXTERN linenr_T sourcing_lnum INIT(= 0); 
EXTERN int ex_nesting_level INIT(= 0); 
EXTERN int debug_break_level INIT(= -1); 
EXTERN int debug_did_msg INIT(= false); 
EXTERN int debug_tick INIT(= 0); 
EXTERN int debug_backtrace_level INIT(= 0); 
#define PROF_NONE 0 
#define PROF_YES 1 
#define PROF_PAUSED 2 
EXTERN int do_profiling INIT(= PROF_NONE); 
EXTERN except_T *current_exception;
EXTERN int need_rethrow INIT(= false);
EXTERN int check_cstack INIT(= false);
EXTERN int trylevel INIT(= 0);
EXTERN int force_abort INIT(= false);
EXTERN struct msglist **msg_list INIT(= NULL);
EXTERN int suppress_errthrow INIT(= false);
EXTERN except_T *caught_stack INIT(= NULL);
EXTERN bool may_garbage_collect INIT(= false);
EXTERN int want_garbage_collect INIT(= false);
EXTERN int garbage_collect_at_exit INIT(= false);
#define SID_MODELINE -1 
#define SID_CMDARG -2 
#define SID_CARG -3 
#define SID_ENV -4 
#define SID_ERROR -5 
#define SID_NONE -6 
#define SID_LUA -7 
#define SID_API_CLIENT -8 
#define SID_STR -9 
EXTERN sctx_T current_sctx INIT(= { 0 COMMA 0 COMMA 0 });
EXTERN uint64_t current_channel_id INIT(= 0);
EXTERN bool did_source_packages INIT(= false);
EXTERN struct caller_scope {
sctx_T script_ctx;
uint8_t *sourcing_name, *autocmd_fname, *autocmd_match;
linenr_T sourcing_lnum;
int autocmd_bufnr;
void *funccalp;
} provider_caller_scope;
EXTERN int provider_call_nesting INIT(= 0);
EXTERN int t_colors INIT(= 256); 
EXTERN int highlight_match INIT(= false); 
EXTERN linenr_T search_match_lines; 
EXTERN colnr_T search_match_endcol; 
EXTERN int no_smartcase INIT(= false); 
EXTERN int need_check_timestamps INIT(= false); 
EXTERN int did_check_timestamps INIT(= false); 
EXTERN int no_check_timestamps INIT(= 0); 
EXTERN int autocmd_busy INIT(= false); 
EXTERN int autocmd_no_enter INIT(= false); 
EXTERN int autocmd_no_leave INIT(= false); 
EXTERN int modified_was_set; 
EXTERN int did_filetype INIT(= false); 
EXTERN int keep_filetype INIT(= false);
EXTERN bufref_T au_new_curbuf INIT(= { NULL, 0, 0 });
EXTERN buf_T *au_pending_free_buf INIT(= NULL);
EXTERN win_T *au_pending_free_win INIT(= NULL);
EXTERN int mouse_grid;
EXTERN int mouse_row;
EXTERN int mouse_col;
EXTERN bool mouse_past_bottom INIT(= false); 
EXTERN bool mouse_past_eol INIT(= false); 
EXTERN int mouse_dragging INIT(= 0); 
EXTERN vimmenu_T *root_menu INIT(= NULL);
EXTERN int sys_menu INIT(= false);
EXTERN int updating_screen INIT(= 0);
EXTERN win_T *firstwin; 
EXTERN win_T *lastwin; 
EXTERN win_T *prevwin INIT(= NULL); 
#define ONE_WINDOW (firstwin == lastwin)
#define FOR_ALL_FRAMES(frp, first_frame) for (frp = first_frame; frp != NULL; frp = frp->fr_next) 
#define FOR_ALL_TAB_WINDOWS(tp, wp) FOR_ALL_TABS(tp) FOR_ALL_WINDOWS_IN_TAB(wp, tp)
#define FOR_ALL_WINDOWS_IN_TAB(wp, tp) for (win_T *wp = ((tp) == curtab) ? firstwin : (tp)->tp_firstwin; wp != NULL; wp = wp->w_next)
EXTERN win_T *curwin; 
EXTERN win_T *aucmd_win; 
EXTERN int aucmd_win_used INIT(= false); 
EXTERN frame_T *topframe; 
EXTERN tabpage_T *first_tabpage;
EXTERN tabpage_T *lastused_tabpage;
EXTERN tabpage_T *curtab;
EXTERN int redraw_tabline INIT(= false); 
#define FOR_ALL_TABS(tp) for (tabpage_T *tp = first_tabpage; tp != NULL; tp = tp->tp_next)
EXTERN buf_T *firstbuf INIT(= NULL); 
EXTERN buf_T *lastbuf INIT(= NULL); 
EXTERN buf_T *curbuf INIT(= NULL); 
#define FOR_ALL_BUFFERS(buf) for (buf_T *buf = firstbuf; buf != NULL; buf = buf->b_next)
#define FOR_ALL_BUFFERS_BACKWARDS(buf) for (buf_T *buf = lastbuf; buf != NULL; buf = buf->b_prev)
#define FOR_ALL_SIGNS_IN_BUF(buf, sign) for (sign = buf->b_signlist; sign != NULL; sign = sign->next) 
EXTERN alist_T global_alist; 
EXTERN int max_alist_id INIT(= 0); 
EXTERN bool arg_had_last INIT(= false); 
EXTERN int ru_col; 
EXTERN int ru_wid; 
EXTERN int sc_col; 
EXTERN int starting INIT(= NO_SCREEN);
EXTERN bool exiting INIT(= false);
EXTERN int stdin_isatty INIT(= true);
EXTERN int stdout_isatty INIT(= true);
EXTERN volatile int full_screen INIT(= false);
EXTERN int restricted INIT(= false);
EXTERN int secure INIT(= false);
EXTERN int textlock INIT(= 0);
EXTERN int curbuf_lock INIT(= 0);
EXTERN int allbuf_lock INIT(= 0);
EXTERN int sandbox INIT(= 0);
EXTERN int silent_mode INIT(= false);
EXTERN pos_T VIsual;
EXTERN int VIsual_active INIT(= false);
EXTERN int VIsual_select INIT(= false);
EXTERN int VIsual_reselect;
EXTERN int VIsual_mode INIT(= 'v');
EXTERN int redo_VIsual_busy INIT(= false);
EXTERN pos_T where_paste_started;
EXTERN bool did_ai INIT(= false);
EXTERN colnr_T ai_col INIT(= 0);
EXTERN int end_comment_pending INIT(= NUL);
EXTERN int did_syncbind INIT(= false);
EXTERN bool did_si INIT(= false);
EXTERN bool can_si INIT(= false);
EXTERN bool can_si_back INIT(= false);
EXTERN pos_T saved_cursor INIT(= { 0, 0, 0 });
EXTERN pos_T Insstart; 
EXTERN pos_T Insstart_orig;
EXTERN int orig_line_count INIT(= 0); 
EXTERN int vr_lines_changed INIT(= 0); 
EXTERN int inhibit_delete_count INIT(= 0);
#define DBCS_JPN 932 
#define DBCS_JPNU 9932 
#define DBCS_KOR 949 
#define DBCS_KORU 9949 
#define DBCS_CHS 936 
#define DBCS_CHSU 9936 
#define DBCS_CHT 950 
#define DBCS_CHTU 9950 
#define DBCS_2BYTE 1 
#define DBCS_DEBUG -1
#define enc_utf8 true
#define has_mbyte true
EXTERN char_u *fenc_default INIT(= NULL);
EXTERN int State INIT(= NORMAL); 
EXTERN bool debug_mode INIT(= false);
EXTERN bool finish_op INIT(= false); 
EXTERN long opcount INIT(= 0); 
EXTERN int motion_force INIT(=0); 
EXTERN int exmode_active INIT(= 0); 
EXTERN int ex_no_reprint INIT(=false); 
EXTERN int reg_recording INIT(= 0); 
EXTERN int reg_executing INIT(= 0); 
EXTERN int no_mapping INIT(= false); 
EXTERN int no_zero_mapping INIT(= 0); 
EXTERN int no_u_sync INIT(= 0); 
EXTERN int u_sync_once INIT(= 0); 
EXTERN bool force_restart_edit INIT(= false); 
EXTERN int restart_edit INIT(= 0); 
EXTERN int arrow_used; 
EXTERN int ins_at_eol INIT(= false); 
EXTERN char_u *edit_submode INIT(= NULL); 
EXTERN char_u *edit_submode_pre INIT(= NULL); 
EXTERN char_u *edit_submode_extra INIT(= NULL); 
EXTERN hlf_T edit_submode_highl; 
EXTERN int no_abbr INIT(= true); 
EXTERN int mapped_ctrl_c INIT(= 0); 
EXTERN cmdmod_T cmdmod; 
EXTERN int msg_silent INIT(= 0); 
EXTERN int emsg_silent INIT(= 0); 
EXTERN bool emsg_noredir INIT(= false); 
EXTERN bool cmd_silent INIT(= false); 
#define SEA_NONE 0 
#define SEA_DIALOG 1 
#define SEA_QUIT 2 
#define SEA_RECOVER 3 
EXTERN int swap_exists_action INIT(= SEA_NONE);
EXTERN int swap_exists_did_quit INIT(= false);
EXTERN char_u IObuff[IOSIZE]; 
EXTERN char_u NameBuff[MAXPATHL]; 
EXTERN char_u msg_buf[MSG_BUF_LEN]; 
EXTERN char os_buf[ 
#if MAXPATHL > IOSIZE
MAXPATHL
#else
IOSIZE
#endif
];
EXTERN int RedrawingDisabled INIT(= 0);
EXTERN int readonlymode INIT(= false); 
EXTERN int recoverymode INIT(= false); 
EXTERN typebuf_T typebuf INIT(= { NULL, NULL, 0, 0, 0, 0, 0, 0, 0 });
EXTERN int ex_normal_busy INIT(= 0); 
EXTERN int ex_normal_lock INIT(= 0); 
EXTERN int ignore_script INIT(= false); 
EXTERN int stop_insert_mode; 
EXTERN bool KeyTyped; 
EXTERN int KeyStuffed; 
EXTERN int maptick INIT(= 0); 
EXTERN int must_redraw INIT(= 0); 
EXTERN bool skip_redraw INIT(= false); 
EXTERN bool do_redraw INIT(= false); 
EXTERN bool must_redraw_pum INIT(= false); 
EXTERN int need_highlight_changed INIT(= true);
EXTERN FILE *scriptout INIT(= NULL); 
EXTERN volatile int got_int INIT(= false); 
EXTERN int bangredo INIT(= false); 
EXTERN int searchcmdlen; 
EXTERN int reg_do_extmatch INIT(= 0); 
EXTERN reg_extmatch_T *re_extmatch_in INIT(= NULL);
EXTERN reg_extmatch_T *re_extmatch_out INIT(= NULL);
EXTERN int did_outofmem_msg INIT(= false);
EXTERN int did_swapwrite_msg INIT(= false);
EXTERN int global_busy INIT(= 0); 
EXTERN int listcmd_busy INIT(= false); 
EXTERN int need_start_insertmode INIT(= false);
EXTERN char_u *last_cmdline INIT(= NULL); 
EXTERN char_u *repeat_cmdline INIT(= NULL); 
EXTERN char_u *new_last_cmdline INIT(= NULL); 
EXTERN char_u *autocmd_fname INIT(= NULL); 
EXTERN int autocmd_bufnr INIT(= 0); 
EXTERN char_u *autocmd_match INIT(= NULL); 
EXTERN int did_cursorhold INIT(= false); 
EXTERN int postponed_split INIT(= 0); 
EXTERN int postponed_split_flags INIT(= 0); 
EXTERN int postponed_split_tab INIT(= 0); 
EXTERN int g_do_tagpreview INIT(= 0); 
EXTERN int g_tag_at_cursor INIT(= false); 
EXTERN int replace_offset INIT(= 0); 
EXTERN char_u *escape_chars INIT(= (char_u *)" \t\\\"|");
EXTERN int keep_help_flag INIT(= false); 
EXTERN char_u *empty_option INIT(= (char_u *)"");
EXTERN int redir_off INIT(= false); 
EXTERN FILE *redir_fd INIT(= NULL); 
EXTERN int redir_reg INIT(= 0); 
EXTERN int redir_vname INIT(= 0); 
EXTERN garray_T *capture_ga INIT(= NULL); 
EXTERN char_u langmap_mapchar[256]; 
EXTERN int save_p_ls INIT(= -1); 
EXTERN int save_p_wmh INIT(= -1); 
EXTERN int wild_menu_showing INIT(= 0);
enum {
WM_SHOWN = 1, 
WM_SCROLLED = 2, 
WM_LIST = 3, 
};
#if defined(HAVE_PATHDEF)
extern char *default_vim_dir;
extern char *default_vimruntime_dir;
extern char *default_lib_dir;
extern char_u *compiled_user;
extern char_u *compiled_sys;
#endif
EXTERN char_u *globaldir INIT(= NULL);
EXTERN int km_stopsel INIT(= false);
EXTERN int km_startsel INIT(= false);
EXTERN int cedit_key INIT(= -1); 
EXTERN int cmdwin_type INIT(= 0); 
EXTERN int cmdwin_result INIT(= 0); 
EXTERN int cmdwin_level INIT(= 0); 
EXTERN char_u no_lines_msg[] INIT(= N_("--No lines in buffer--"));
EXTERN long sub_nsubs; 
EXTERN linenr_T sub_nlines; 
EXTERN char_u wim_flags[4];
#define STL_IN_ICON 1
#define STL_IN_TITLE 2
EXTERN int stl_syntax INIT(= 0);
EXTERN bool no_hlsearch INIT(= false);
EXTERN linenr_T printer_page_num;
EXTERN bool typebuf_was_filled INIT(= false); 
#if defined(BACKSLASH_IN_FILENAME)
EXTERN char psepc INIT(= '\\'); 
EXTERN char psepcN INIT(= '/'); 
EXTERN char pseps[2] INIT(= { '\\', 0 }); 
#endif
EXTERN TriState virtual_op INIT(= kNone);
EXTERN disptick_T display_tick INIT(= 0);
EXTERN linenr_T spell_redraw_lnum INIT(= 0);
EXTERN char_u e_abort[] INIT(= N_("E470: Command aborted"));
EXTERN char_u e_afterinit[] INIT(= N_(
"E905: Cannot set this option after startup"));
EXTERN char_u e_api_spawn_failed[] INIT(= N_("E903: Could not spawn API job"));
EXTERN char_u e_argreq[] INIT(= N_("E471: Argument required"));
EXTERN char_u e_backslash[] INIT(= N_("E10: \\ should be followed by /, ? or &"));
EXTERN char_u e_cmdwin[] INIT(= N_(
"E11: Invalid in command-line window; <CR> executes, CTRL-C quits"));
EXTERN char_u e_curdir[] INIT(= N_(
"E12: Command not allowed from exrc/vimrc in current dir or tag search"));
EXTERN char_u e_endif[] INIT(= N_("E171: Missing :endif"));
EXTERN char_u e_endtry[] INIT(= N_("E600: Missing :endtry"));
EXTERN char_u e_endwhile[] INIT(= N_("E170: Missing :endwhile"));
EXTERN char_u e_endfor[] INIT(= N_("E170: Missing :endfor"));
EXTERN char_u e_while[] INIT(= N_("E588: :endwhile without :while"));
EXTERN char_u e_for[] INIT(= N_("E588: :endfor without :for"));
EXTERN char_u e_exists[] INIT(= N_("E13: File exists (add ! to override)"));
EXTERN char_u e_failed[] INIT(= N_("E472: Command failed"));
EXTERN char_u e_internal[] INIT(= N_("E473: Internal error"));
EXTERN char_u e_intern2[] INIT(= N_("E685: Internal error: %s"));
EXTERN char_u e_interr[] INIT(= N_("Interrupted"));
EXTERN char_u e_invaddr[] INIT(= N_("E14: Invalid address"));
EXTERN char_u e_invarg[] INIT(= N_("E474: Invalid argument"));
EXTERN char_u e_invarg2[] INIT(= N_("E475: Invalid argument: %s"));
EXTERN char_u e_invargval[] INIT(= N_("E475: Invalid value for argument %s"));
EXTERN char_u e_invargNval[] INIT(= N_(
"E475: Invalid value for argument %s: %s"));
EXTERN char_u e_duparg2[] INIT(= N_("E983: Duplicate argument: %s"));
EXTERN char_u e_invexpr2[] INIT(= N_("E15: Invalid expression: %s"));
EXTERN char_u e_invrange[] INIT(= N_("E16: Invalid range"));
EXTERN char_u e_invcmd[] INIT(= N_("E476: Invalid command"));
EXTERN char_u e_isadir2[] INIT(= N_("E17: \"%s\" is a directory"));
EXTERN char_u e_invchan[] INIT(= N_("E900: Invalid channel id"));
EXTERN char_u e_invchanjob[] INIT(= N_("E900: Invalid channel id: not a job"));
EXTERN char_u e_jobtblfull[] INIT(= N_("E901: Job table is full"));
EXTERN char_u e_jobspawn[] INIT(= N_(
"E903: Process failed to start: %s: \"%s\""));
EXTERN char_u e_channotpty[] INIT(= N_("E904: channel is not a pty"));
EXTERN char_u e_stdiochan2[] INIT(= N_(
"E905: Couldn't open stdio channel: %s"));
EXTERN char_u e_invstream[] INIT(= N_("E906: invalid stream for channel"));
EXTERN char_u e_invstreamrpc[] INIT(= N_(
"E906: invalid stream for rpc channel, use 'rpc'"));
EXTERN char_u e_streamkey[] INIT(= N_(
"E5210: dict key '%s' already set for buffered stream in channel %"
PRIu64));
EXTERN char_u e_libcall[] INIT(= N_("E364: Library call failed for \"%s()\""));
EXTERN char e_fsync[] INIT(= N_("E667: Fsync failed: %s"));
EXTERN char_u e_mkdir[] INIT(= N_("E739: Cannot create directory %s: %s"));
EXTERN char_u e_markinval[] INIT(= N_("E19: Mark has invalid line number"));
EXTERN char_u e_marknotset[] INIT(= N_("E20: Mark not set"));
EXTERN char_u e_modifiable[] INIT(= N_(
"E21: Cannot make changes, 'modifiable' is off"));
EXTERN char_u e_nesting[] INIT(= N_("E22: Scripts nested too deep"));
EXTERN char_u e_noalt[] INIT(= N_("E23: No alternate file"));
EXTERN char_u e_noabbr[] INIT(= N_("E24: No such abbreviation"));
EXTERN char_u e_nobang[] INIT(= N_("E477: No ! allowed"));
EXTERN char_u e_nogroup[] INIT(= N_("E28: No such highlight group name: %s"));
EXTERN char_u e_noinstext[] INIT(= N_("E29: No inserted text yet"));
EXTERN char_u e_nolastcmd[] INIT(= N_("E30: No previous command line"));
EXTERN char_u e_nomap[] INIT(= N_("E31: No such mapping"));
EXTERN char_u e_nomatch[] INIT(= N_("E479: No match"));
EXTERN char_u e_nomatch2[] INIT(= N_("E480: No match: %s"));
EXTERN char_u e_noname[] INIT(= N_("E32: No file name"));
EXTERN char_u e_nopresub[] INIT(= N_(
"E33: No previous substitute regular expression"));
EXTERN char_u e_noprev[] INIT(= N_("E34: No previous command"));
EXTERN char_u e_noprevre[] INIT(= N_("E35: No previous regular expression"));
EXTERN char_u e_norange[] INIT(= N_("E481: No range allowed"));
EXTERN char_u e_noroom[] INIT(= N_("E36: Not enough room"));
EXTERN char_u e_notmp[] INIT(= N_("E483: Can't get temp file name"));
EXTERN char_u e_notopen[] INIT(= N_("E484: Can't open file %s"));
EXTERN char_u e_notopen_2[] INIT(= N_("E484: Can't open file %s: %s"));
EXTERN char_u e_notread[] INIT(= N_("E485: Can't read file %s"));
EXTERN char_u e_null[] INIT(= N_("E38: Null argument"));
EXTERN char_u e_number_exp[] INIT(= N_("E39: Number expected"));
EXTERN char_u e_openerrf[] INIT(= N_("E40: Can't open errorfile %s"));
EXTERN char_u e_outofmem[] INIT(= N_("E41: Out of memory!"));
EXTERN char_u e_patnotf[] INIT(= N_("Pattern not found"));
EXTERN char_u e_patnotf2[] INIT(= N_("E486: Pattern not found: %s"));
EXTERN char_u e_positive[] INIT(= N_("E487: Argument must be positive"));
EXTERN char_u e_prev_dir[] INIT(= N_(
"E459: Cannot go back to previous directory"));
EXTERN char_u e_quickfix[] INIT(= N_("E42: No Errors"));
EXTERN char_u e_loclist[] INIT(= N_("E776: No location list"));
EXTERN char_u e_re_damg[] INIT(= N_("E43: Damaged match string"));
EXTERN char_u e_re_corr[] INIT(= N_("E44: Corrupted regexp program"));
EXTERN char_u e_readonly[] INIT(= N_(
"E45: 'readonly' option is set (add ! to override)"));
EXTERN char_u e_readonlyvar[] INIT(= N_(
"E46: Cannot change read-only variable \"%.*s\""));
EXTERN char_u e_dictreq[] INIT(= N_("E715: Dictionary required"));
EXTERN char_u e_toomanyarg[] INIT(= N_("E118: Too many arguments for function: %s"));
EXTERN char_u e_dictkey[] INIT(= N_("E716: Key not present in Dictionary: %s"));
EXTERN char_u e_listreq[] INIT(= N_("E714: List required"));
EXTERN char_u e_listdictarg[] INIT(= N_(
"E712: Argument of %s must be a List or Dictionary"));
EXTERN char_u e_readerrf[] INIT(= N_("E47: Error while reading errorfile"));
EXTERN char_u e_sandbox[] INIT(= N_("E48: Not allowed in sandbox"));
EXTERN char_u e_secure[] INIT(= N_("E523: Not allowed here"));
EXTERN char_u e_screenmode[] INIT(= N_(
"E359: Screen mode setting not supported"));
EXTERN char_u e_scroll[] INIT(= N_("E49: Invalid scroll size"));
EXTERN char_u e_shellempty[] INIT(= N_("E91: 'shell' option is empty"));
EXTERN char_u e_signdata[] INIT(= N_("E255: Couldn't read in sign data!"));
EXTERN char_u e_swapclose[] INIT(= N_("E72: Close error on swap file"));
EXTERN char_u e_tagstack[] INIT(= N_("E73: tag stack empty"));
EXTERN char_u e_toocompl[] INIT(= N_("E74: Command too complex"));
EXTERN char_u e_longname[] INIT(= N_("E75: Name too long"));
EXTERN char_u e_toomsbra[] INIT(= N_("E76: Too many ["));
EXTERN char_u e_toomany[] INIT(= N_("E77: Too many file names"));
EXTERN char_u e_trailing[] INIT(= N_("E488: Trailing characters"));
EXTERN char_u e_trailing2[] INIT(= N_("E488: Trailing characters: %s"));
EXTERN char_u e_umark[] INIT(= N_("E78: Unknown mark"));
EXTERN char_u e_wildexpand[] INIT(= N_("E79: Cannot expand wildcards"));
EXTERN char_u e_winheight[] INIT(= N_(
"E591: 'winheight' cannot be smaller than 'winminheight'"));
EXTERN char_u e_winwidth[] INIT(= N_(
"E592: 'winwidth' cannot be smaller than 'winminwidth'"));
EXTERN char_u e_write[] INIT(= N_("E80: Error while writing"));
EXTERN char_u e_zerocount[] INIT(= N_("E939: Positive count required"));
EXTERN char_u e_usingsid[] INIT(= N_(
"E81: Using <SID> not in a script context"));
EXTERN char_u e_maxmempat[] INIT(= N_(
"E363: pattern uses more memory than 'maxmempattern'"));
EXTERN char_u e_emptybuf[] INIT(= N_("E749: empty buffer"));
EXTERN char_u e_nobufnr[] INIT(= N_("E86: Buffer %" PRId64 " does not exist"));
EXTERN char_u e_invalpat[] INIT(= N_(
"E682: Invalid search pattern or delimiter"));
EXTERN char_u e_bufloaded[] INIT(= N_("E139: File is loaded in another buffer"));
EXTERN char_u e_notset[] INIT(= N_("E764: Option '%s' is not set"));
EXTERN char_u e_invalidreg[] INIT(= N_("E850: Invalid register name"));
EXTERN char_u e_dirnotf[] INIT(= N_(
"E919: Directory not found in '%s': \"%s\""));
EXTERN char_u e_au_recursive[] INIT(= N_(
"E952: Autocommand caused recursive behavior"));
EXTERN char_u e_unsupportedoption[] INIT(= N_("E519: Option not supported"));
EXTERN char_u e_fnametoolong[] INIT(= N_("E856: Filename too long"));
EXTERN char_u e_float_as_string[] INIT(= N_("E806: using Float as a String"));
EXTERN char_u e_autocmd_err[] INIT(=N_(
"E5500: autocmd has thrown an exception: %s"));
EXTERN char_u e_cmdmap_err[] INIT(=N_(
"E5520: <Cmd> mapping must end with <CR>"));
EXTERN char_u e_cmdmap_repeated[] INIT(=N_(
"E5521: <Cmd> mapping must end with <CR> before second <Cmd>"));
EXTERN char_u e_cmdmap_key[] INIT(=N_(
"E5522: <Cmd> mapping must not include %s key"));
EXTERN char_u e_api_error[] INIT(=N_(
"E5555: API call: %s"));
EXTERN char e_luv_api_disabled[] INIT(=N_(
"E5560: %s must not be called in a lua loop callback"));
EXTERN char_u e_floatonly[] INIT(=N_(
"E5601: Cannot close window, only floating window would remain"));
EXTERN char_u e_floatexchange[] INIT(=N_(
"E5602: Cannot exchange or rotate float"));
EXTERN char top_bot_msg[] INIT(= N_("search hit TOP, continuing at BOTTOM"));
EXTERN char bot_top_msg[] INIT(= N_("search hit BOTTOM, continuing at TOP"));
EXTERN char line_msg[] INIT(= N_(" line "));
EXTERN time_t starttime;
EXTERN FILE *time_fd INIT(= NULL); 
EXTERN int vim_ignored;
EXTERN bool embedded_mode INIT(= false);
EXTERN bool headless_mode INIT(= false);
typedef enum {
kUnknown,
kWorking,
kBroken
} WorkingStatus;
typedef enum {
kCdScopeInvalid = -1,
kCdScopeWindow, 
kCdScopeTab, 
kCdScopeGlobal, 
} CdScope;
#define MIN_CD_SCOPE kCdScopeWindow
#define MAX_CD_SCOPE kCdScopeGlobal
