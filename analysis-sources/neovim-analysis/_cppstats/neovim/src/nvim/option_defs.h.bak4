#if !defined(NVIM_OPTION_DEFS_H)
#define NVIM_OPTION_DEFS_H

#include "nvim/types.h"
#include "nvim/macros.h" 
#include "eval/typval.h" 




#define SOPT_BOOL 0x01 
#define SOPT_NUM 0x02 
#define SOPT_STRING 0x04 
#define SOPT_GLOBAL 0x08 
#define SOPT_WIN 0x10 
#define SOPT_BUF 0x20 
#define SOPT_UNSET 0x40 


#define SREQ_GLOBAL 0 
#define SREQ_WIN 1 
#define SREQ_BUF 2 




#if defined(WIN32)
#define DFLT_EFM "%f(%l) \\=: %t%*\\D%n: %m,%*[^\"]\"%f\"%*\\D%l: %m,%f(%l) \\=: %m,%*[^ ] %f %l: %m,%f:%l:%c:%m,%f(%l):%m,%f:%l:%m,%f|%l| %m"
#else
#define DFLT_EFM "%*[^\"]\"%f\"%*\\D%l: %m,\"%f\"%*\\D%l: %m,%-G%f:%l: (Each undeclared identifier is reported only once,%-G%f:%l: for each function it appears in.),%-GIn file included from %f:%l:%c:,%-GIn file included from %f:%l:%c\\,,%-GIn file included from %f:%l:%c,%-GIn file included from %f:%l,%-G%*[ ]from %f:%l:%c,%-G%*[ ]from %f:%l:,%-G%*[ ]from %f:%l\\,,%-G%*[ ]from %f:%l,%f:%l:%c:%m,%f(%l):%m,%f:%l:%m,\"%f\"\\, line %l%*\\D%c%*[^ ] %m,%D%*\\a[%*\\d]: Entering directory %*[`']%f',%X%*\\a[%*\\d]: Leaving directory %*[`']%f',%D%*\\a: Entering directory %*[`']%f',%X%*\\a: Leaving directory %*[`']%f',%DMaking %*\\a in %f,%f|%l| %m"
#endif

#define DFLT_GREPFORMAT "%f:%l:%m,%f:%l%m,%f %l%m"


#define FF_DOS "dos"
#define FF_MAC "mac"
#define FF_UNIX "unix"

#if defined(USE_CRNL)
#define DFLT_FF "dos"
#define DFLT_FFS_VIM "dos,unix"
#define DFLT_FFS_VI "dos,unix" 
#else
#define DFLT_FF "unix"
#define DFLT_FFS_VIM "unix,dos"
#define DFLT_FFS_VI ""
#endif



#define ENC_UCSBOM "ucs-bom" 


#define ENC_DFLT "utf-8"


#define EOL_UNKNOWN -1 
#define EOL_UNIX 0 
#define EOL_DOS 1 
#define EOL_MAC 2 


#define FO_WRAP 't'
#define FO_WRAP_COMS 'c'
#define FO_RET_COMS 'r'
#define FO_OPEN_COMS 'o'
#define FO_Q_COMS 'q'
#define FO_Q_NUMBER 'n'
#define FO_Q_SECOND '2'
#define FO_INS_VI 'v'
#define FO_INS_LONG 'l'
#define FO_INS_BLANK 'b'
#define FO_MBYTE_BREAK 'm' 
#define FO_MBYTE_JOIN 'M' 
#define FO_MBYTE_JOIN2 'B' 
#define FO_ONE_LETTER '1'
#define FO_WHITE_PAR 'w' 
#define FO_AUTO 'a' 
#define FO_REMOVE_COMS 'j' 
#define FO_PERIOD_ABBR 'p' 

#define DFLT_FO_VI "vt"
#define DFLT_FO_VIM "tcqj"
#define FO_ALL "tcroq2vlb1mMBn,awjp" 


#define CPO_ALTREAD 'a' 
#define CPO_ALTWRITE 'A' 
#define CPO_BAR 'b' 
#define CPO_BSLASH 'B' 
#define CPO_SEARCH 'c'
#define CPO_CONCAT 'C' 
#define CPO_DOTTAG 'd' 
#define CPO_DIGRAPH 'D' 
#define CPO_EXECBUF 'e'
#define CPO_EMPTYREGION 'E' 
#define CPO_FNAMER 'f' 
#define CPO_FNAMEW 'F' 
#define CPO_INTMOD 'i' 
#define CPO_INDENT 'I' 
#define CPO_ENDOFSENT 'J' 
#define CPO_KOFFSET 'K' 
#define CPO_LITERAL 'l' 
#define CPO_LISTWM 'L' 
#define CPO_SHOWMATCH 'm'
#define CPO_MATCHBSL 'M' 
#define CPO_NUMCOL 'n' 
#define CPO_LINEOFF 'o'
#define CPO_OVERNEW 'O' 
#define CPO_LISP 'p' 
#define CPO_FNAMEAPP 'P' 
#define CPO_JOINCOL 'q' 
#define CPO_REDO 'r'
#define CPO_REMMARK 'R' 
#define CPO_BUFOPT 's'
#define CPO_BUFOPTGLOB 'S'
#define CPO_TAGPAT 't'
#define CPO_UNDO 'u' 
#define CPO_BACKSPACE 'v' 
#define CPO_FWRITE 'W' 
#define CPO_ESC 'x'
#define CPO_REPLCNT 'X' 
#define CPO_YANK 'y'
#define CPO_KEEPRO 'Z' 
#define CPO_DOLLAR '$'
#define CPO_FILTER '!'
#define CPO_MATCH '%'
#define CPO_PLUS '+' 
#define CPO_REGAPPEND '>' 
#define CPO_SCOLON ';' 

#define CPO_CHANGEW '_' 

#define CPO_VIM "aABceFs_"
#define CPO_VI "aAbBcCdDeEfFiIJKlLmMnoOpPqrRsStuvWxXyZ$!%+>;_"


#define WW_ALL "bshl<>[],~"


#define MOUSE_NORMAL 'n' 
#define MOUSE_VISUAL 'v' 
#define MOUSE_INSERT 'i' 
#define MOUSE_COMMAND 'c' 
#define MOUSE_HELP 'h' 
#define MOUSE_RETURN 'r' 
#define MOUSE_A "nvich" 
#define MOUSE_ALL "anvichr" 
#define MOUSE_NONE ' ' 
#define MOUSE_NONEF 'x' 

#define COCU_ALL "nvic" 


enum {
SHM_RO = 'r', 
SHM_MOD = 'm', 
SHM_FILE = 'f', 
SHM_LAST = 'i', 
SHM_TEXT = 'x', 
SHM_LINES = 'l', 
SHM_NEW = 'n', 
SHM_WRI = 'w', 
SHM_ABBREVIATIONS = 'a', 
SHM_WRITE = 'W', 
SHM_TRUNC = 't', 
SHM_TRUNCALL = 'T', 
SHM_OVER = 'o', 
SHM_OVERALL = 'O', 
SHM_SEARCH = 's', 
SHM_ATTENTION = 'A', 
SHM_INTRO = 'I', 
SHM_COMPLETIONMENU = 'c', 
SHM_RECORDING = 'q', 
SHM_FILEINFO = 'F', 
SHM_SEARCHCOUNT = 'S', 
};

#define SHM_ALL_ABBREVIATIONS ((char_u[]) { SHM_RO, SHM_MOD, SHM_FILE, SHM_LAST, SHM_TEXT, SHM_LINES, SHM_NEW, SHM_WRI, 0, })





#define GO_ASEL 'a' 
#define GO_ASELML 'A' 
#define GO_BOT 'b' 
#define GO_CONDIALOG 'c' 
#define GO_TABLINE 'e' 
#define GO_FORG 'f' 
#define GO_GREY 'g' 
#define GO_HORSCROLL 'h' 
#define GO_ICON 'i' 
#define GO_LEFT 'l' 
#define GO_VLEFT 'L' 
#define GO_MENUS 'm' 
#define GO_NOSYSMENU 'M' 
#define GO_POINTER 'p' 
#define GO_ASELPLUS 'P' 
#define GO_RIGHT 'r' 
#define GO_VRIGHT 'R' 
#define GO_TOOLBAR 'T' 
#define GO_FOOTER 'F' 
#define GO_VERTICAL 'v' 
#define GO_KEEPWINSIZE 'k' 
#define GO_ALL "aAbcefFghilmMprTvk" 


#define COM_NEST 'n' 
#define COM_BLANK 'b' 
#define COM_START 's' 
#define COM_MIDDLE 'm' 
#define COM_END 'e' 
#define COM_AUTO_END 'x' 
#define COM_FIRST 'f' 
#define COM_LEFT 'l' 
#define COM_RIGHT 'r' 
#define COM_NOBACK 'O' 
#define COM_ALL "nbsmexflrO" 
#define COM_MAX_LEN 50 


enum {
STL_FILEPATH = 'f', 
STL_FULLPATH = 'F', 
STL_FILENAME = 't', 
STL_COLUMN = 'c', 
STL_VIRTCOL = 'v', 
STL_VIRTCOL_ALT = 'V', 
STL_LINE = 'l', 
STL_NUMLINES = 'L', 
STL_BUFNO = 'n', 
STL_KEYMAP = 'k', 
STL_OFFSET = 'o', 
STL_OFFSET_X = 'O', 
STL_BYTEVAL = 'b', 
STL_BYTEVAL_X = 'B', 
STL_ROFLAG = 'r', 
STL_ROFLAG_ALT = 'R', 
STL_HELPFLAG = 'h', 
STL_HELPFLAG_ALT = 'H', 
STL_FILETYPE = 'y', 
STL_FILETYPE_ALT = 'Y', 
STL_PREVIEWFLAG = 'w', 
STL_PREVIEWFLAG_ALT = 'W', 
STL_MODIFIED = 'm', 
STL_MODIFIED_ALT = 'M', 
STL_QUICKFIX = 'q', 
STL_PERCENTAGE = 'p', 
STL_ALTPERCENT = 'P', 
STL_ARGLISTSTAT = 'a', 
STL_PAGENUM = 'N', 
STL_VIM_EXPR = '{', 
STL_SEPARATE = '=', 
STL_TRUNCMARK = '<', 
STL_USER_HL = '*', 
STL_HIGHLIGHT = '#', 
STL_TABPAGENR = 'T', 
STL_TABCLOSENR = 'X', 
STL_CLICK_FUNC = '@', 
};

#define STL_ALL ((char_u[]) { STL_FILEPATH, STL_FULLPATH, STL_FILENAME, STL_COLUMN, STL_VIRTCOL, STL_VIRTCOL_ALT, STL_LINE, STL_NUMLINES, STL_BUFNO, STL_KEYMAP, STL_OFFSET, STL_OFFSET_X, STL_BYTEVAL, STL_BYTEVAL_X, STL_ROFLAG, STL_ROFLAG_ALT, STL_HELPFLAG, STL_HELPFLAG_ALT, STL_FILETYPE, STL_FILETYPE_ALT, STL_PREVIEWFLAG, STL_PREVIEWFLAG_ALT, STL_MODIFIED, STL_MODIFIED_ALT, STL_QUICKFIX, STL_PERCENTAGE, STL_ALTPERCENT, STL_ARGLISTSTAT, STL_PAGENUM, STL_VIM_EXPR, STL_SEPARATE, STL_TRUNCMARK, STL_USER_HL, STL_HIGHLIGHT, STL_TABPAGENR, STL_TABCLOSENR, STL_CLICK_FUNC, 0, })












#define WIM_FULL 1
#define WIM_LONGEST 2
#define WIM_LIST 4


#define BS_INDENT 'i' 
#define BS_EOL 'o' 
#define BS_START 's' 

#define LISPWORD_VALUE "defun,define,defmacro,set!,lambda,if,case,let,flet,let*,letrec,do,do*,define-syntax,let-syntax,letrec-syntax,destructuring-bind,defpackage,defparameter,defstruct,deftype,defvar,do-all-symbols,do-external-symbols,do-symbols,dolist,dotimes,ecase,etypecase,eval-when,labels,macrolet,multiple-value-bind,multiple-value-call,multiple-value-prog1,multiple-value-setq,prog1,progv,typecase,unless,unwind-protect,when,with-input-from-string,with-open-file,with-open-stream,with-output-to-string,with-package-iterator,define-condition,handler-bind,handler-case,restart-bind,restart-case,with-simple-restart,store-value,use-value,muffle-warning,abort,continue,with-slots,with-slots*,with-accessors,with-accessors*,defclass,defmethod,print-unreadable-object"






EXTERN long p_aleph; 
EXTERN int p_acd; 
EXTERN char_u *p_ambw; 
EXTERN int p_ar; 
EXTERN int p_aw; 
EXTERN int p_awa; 
EXTERN char_u *p_bs; 
EXTERN char_u *p_bg; 
EXTERN int p_bk; 
EXTERN char_u *p_bkc; 
EXTERN unsigned int bkc_flags; 
#if defined(IN_OPTION_C)
static char *(p_bkc_values[]) =
{"yes", "auto", "no", "breaksymlink", "breakhardlink", NULL};
#endif
#define BKC_YES 0x001
#define BKC_AUTO 0x002
#define BKC_NO 0x004
#define BKC_BREAKSYMLINK 0x008
#define BKC_BREAKHARDLINK 0x010
EXTERN char_u *p_bdir; 
EXTERN char_u *p_bex; 
EXTERN char_u *p_bo; 
EXTERN char breakat_flags[256]; 
EXTERN unsigned bo_flags;
#if defined(IN_OPTION_C)
static char *(p_bo_values[]) = {"all", "backspace", "cursor", "complete",
"copy", "ctrlg", "error", "esc", "ex",
"hangul", "insertmode", "lang", "mess",
"showmatch", "operator", "register", "shell",
"spell", "wildmode", NULL};
#endif


#define BO_ALL 0x0001
#define BO_BS 0x0002
#define BO_CRSR 0x0004
#define BO_COMPL 0x0008
#define BO_COPY 0x0010
#define BO_CTRLG 0x0020
#define BO_ERROR 0x0040
#define BO_ESC 0x0080
#define BO_EX 0x0100
#define BO_HANGUL 0x0200
#define BO_IM 0x0400
#define BO_LANG 0x0800
#define BO_MESS 0x1000
#define BO_MATCH 0x2000
#define BO_OPER 0x4000
#define BO_REG 0x8000
#define BO_SH 0x10000
#define BO_SPELL 0x20000
#define BO_WILD 0x40000

EXTERN char_u *p_bsk; 
EXTERN char_u *p_breakat; 
EXTERN char_u *p_cmp; 
EXTERN unsigned cmp_flags;
#if defined(IN_OPTION_C)
static char *(p_cmp_values[]) = {"internal", "keepascii", NULL};
#endif
#define CMP_INTERNAL 0x001
#define CMP_KEEPASCII 0x002
EXTERN char_u *p_enc; 
EXTERN int p_deco; 
EXTERN char_u *p_ccv; 
EXTERN char_u *p_cedit; 
EXTERN char_u *p_cb; 
EXTERN unsigned cb_flags;
#if defined(IN_OPTION_C)
static char *(p_cb_values[]) = {"unnamed", "unnamedplus", NULL};
#endif
#define CB_UNNAMED 0x001
#define CB_UNNAMEDPLUS 0x002
#define CB_UNNAMEDMASK (CB_UNNAMED | CB_UNNAMEDPLUS)
EXTERN long p_cwh; 
EXTERN long p_ch; 
EXTERN long p_columns; 
EXTERN int p_confirm; 
EXTERN int p_cp; 
EXTERN char_u *p_cot; 
EXTERN long p_pb; 
EXTERN long p_ph; 
EXTERN long p_pw; 
EXTERN char_u *p_cpo; 
EXTERN char_u *p_csprg; 
EXTERN int p_csre; 
EXTERN char_u *p_csqf; 
#define CSQF_CMDS "sgdctefia"
#define CSQF_FLAGS "+-0"
EXTERN int p_cst; 
EXTERN long p_csto; 
EXTERN long p_cspc; 
EXTERN int p_csverbose; 
EXTERN char_u *p_debug; 
EXTERN char_u *p_def; 
EXTERN char_u *p_inc;
EXTERN char_u *p_dip; 
EXTERN char_u *p_dex; 
EXTERN char_u *p_dict; 
EXTERN int p_dg; 
EXTERN char_u *p_dir; 
EXTERN char_u *p_dy; 
EXTERN unsigned dy_flags;
#if defined(IN_OPTION_C)
static char *(p_dy_values[]) = { "lastline", "truncate", "uhex", "msgsep",
NULL };
#endif
#define DY_LASTLINE 0x001
#define DY_TRUNCATE 0x002
#define DY_UHEX 0x004

#define DY_MSGSEP 0x008
EXTERN int p_ed; 
EXTERN int p_emoji; 
EXTERN char_u *p_ead; 
EXTERN int p_ea; 
EXTERN char_u *p_ep; 
EXTERN int p_eb; 
EXTERN char_u *p_ef; 
EXTERN char_u *p_efm; 
EXTERN char_u *p_gefm; 
EXTERN char_u *p_gp; 
EXTERN char_u *p_ei; 
EXTERN int p_exrc; 
EXTERN char_u *p_fencs; 
EXTERN char_u *p_ffs; 
EXTERN int p_fic; 
EXTERN char_u *p_fcl; 
EXTERN long p_fdls; 
EXTERN char_u *p_fdo; 
EXTERN unsigned fdo_flags;
#if defined(IN_OPTION_C)
static char *(p_fdo_values[]) = {"all", "block", "hor", "mark", "percent",
"quickfix", "search", "tag", "insert",
"undo", "jump", NULL};
#endif
#define FDO_ALL 0x001
#define FDO_BLOCK 0x002
#define FDO_HOR 0x004
#define FDO_MARK 0x008
#define FDO_PERCENT 0x010
#define FDO_QUICKFIX 0x020
#define FDO_SEARCH 0x040
#define FDO_TAG 0x080
#define FDO_INSERT 0x100
#define FDO_UNDO 0x200
#define FDO_JUMP 0x400
EXTERN char_u *p_fp; 
EXTERN int p_fs; 
EXTERN int p_gd; 
EXTERN char_u *p_pdev; 
EXTERN char_u *p_penc; 
EXTERN char_u *p_pexpr; 
EXTERN char_u *p_pmfn; 
EXTERN char_u *p_pmcs; 
EXTERN char_u *p_pfn; 
EXTERN char_u *p_popt; 
EXTERN char_u *p_header; 
EXTERN int p_prompt; 
EXTERN char_u *p_guicursor; 
EXTERN char_u *p_guifont; 
EXTERN char_u *p_guifontset; 
EXTERN char_u *p_guifontwide; 
EXTERN char_u *p_hf; 
EXTERN long p_hh; 
EXTERN char_u *p_hlg; 
EXTERN int p_hid; 
EXTERN char_u *p_hl; 
EXTERN int p_hls; 
EXTERN long p_hi; 
EXTERN int p_hkmap; 
EXTERN int p_hkmapp; 
EXTERN int p_arshape; 
EXTERN int p_icon; 
EXTERN char_u *p_iconstring; 
EXTERN int p_ic; 
EXTERN int p_is; 
EXTERN char_u *p_icm; 
EXTERN int p_im; 
EXTERN char_u *p_isf; 
EXTERN char_u *p_isi; 
EXTERN char_u *p_isp; 
EXTERN int p_js; 
EXTERN char_u *p_jop; 
EXTERN unsigned jop_flags;
#if defined(IN_OPTION_C)
static char *(p_jop_values[]) = { "stack", NULL };
#endif
#define JOP_STACK 0x01
EXTERN char_u *p_kp; 
EXTERN char_u *p_km; 
EXTERN char_u *p_langmap; 
EXTERN int p_lnr; 
EXTERN int p_lrm; 
EXTERN char_u *p_lm; 
EXTERN long p_lines; 
EXTERN long p_linespace; 
EXTERN char_u *p_lispwords; 
EXTERN long p_ls; 
EXTERN long p_stal; 
EXTERN char_u *p_lcs; 

EXTERN int p_lz; 
EXTERN int p_lpl; 
EXTERN int p_magic; 
EXTERN char_u *p_menc; 
EXTERN char_u *p_mef; 
EXTERN char_u *p_mp; 
EXTERN char_u *p_cc; 
EXTERN int p_cc_cols[256]; 
EXTERN long p_mat; 
EXTERN long p_mco; 
EXTERN long p_mfd; 
EXTERN long p_mmd; 
EXTERN long p_mmp; 
EXTERN long p_mis; 
EXTERN char_u *p_msm; 
EXTERN long p_mle; 
EXTERN long p_mls; 
EXTERN char_u *p_mouse; 
EXTERN char_u *p_mousem; 
EXTERN long p_mouset; 
EXTERN int p_more; 
EXTERN char_u *p_opfunc; 
EXTERN char_u *p_para; 
EXTERN int p_paste; 
EXTERN char_u *p_pt; 
EXTERN char_u *p_pex; 
EXTERN char_u *p_pm; 
EXTERN char_u *p_path; 
EXTERN char_u *p_cdpath; 
EXTERN long p_pyx; 
EXTERN char_u *p_rdb; 
EXTERN unsigned rdb_flags;
#if defined(IN_OPTION_C)
static char *(p_rdb_values[]) = {
"compositor",
"nothrottle",
"invalid",
"nodelta",
NULL
};
#endif
#define RDB_COMPOSITOR 0x001
#define RDB_NOTHROTTLE 0x002
#define RDB_INVALID 0x004
#define RDB_NODELTA 0x008

EXTERN long p_rdt; 
EXTERN int p_remap; 
EXTERN long p_re; 
EXTERN long p_report; 
EXTERN long p_pvh; 
EXTERN int p_ari; 
EXTERN int p_ri; 
EXTERN int p_ru; 
EXTERN char_u *p_ruf; 
EXTERN char_u *p_pp; 
EXTERN char_u *p_rtp; 
EXTERN long p_scbk; 
EXTERN long p_sj; 
EXTERN long p_so; 
EXTERN char_u *p_sbo; 
EXTERN char_u *p_sections; 
EXTERN int p_secure; 
EXTERN char_u *p_sel; 
EXTERN char_u *p_slm; 
EXTERN char_u *p_ssop; 
EXTERN unsigned ssop_flags;
#if defined(IN_OPTION_C)

static char *(p_ssop_values[]) = {
"buffers", "winpos", "resize", "winsize",
"localoptions", "options", "help", "blank", "globals", "slash", "unix",
"sesdir", "curdir", "folds", "cursor", "tabpages", NULL
};
#endif
#define SSOP_BUFFERS 0x001
#define SSOP_WINPOS 0x002
#define SSOP_RESIZE 0x004
#define SSOP_WINSIZE 0x008
#define SSOP_LOCALOPTIONS 0x010
#define SSOP_OPTIONS 0x020
#define SSOP_HELP 0x040
#define SSOP_BLANK 0x080
#define SSOP_GLOBALS 0x100
#define SSOP_SLASH 0x200 
#define SSOP_UNIX 0x400 
#define SSOP_SESDIR 0x800
#define SSOP_CURDIR 0x1000
#define SSOP_FOLDS 0x2000
#define SSOP_CURSOR 0x4000
#define SSOP_TABPAGES 0x8000

EXTERN char_u *p_sh; 
EXTERN char_u *p_shcf; 
EXTERN char_u *p_sp; 
EXTERN char_u *p_shq; 
EXTERN char_u *p_sxq; 
EXTERN char_u *p_sxe; 
EXTERN char_u *p_srr; 
EXTERN int p_stmp; 
#if defined(BACKSLASH_IN_FILENAME)
EXTERN int p_ssl; 
#endif
EXTERN char_u *p_stl; 
EXTERN int p_sr; 
EXTERN char_u *p_shm; 
EXTERN char_u *p_sbr; 
EXTERN int p_sc; 
EXTERN int p_sft; 
EXTERN int p_sm; 
EXTERN int p_smd; 
EXTERN long p_ss; 
EXTERN long p_siso; 
EXTERN int p_scs; 
EXTERN int p_sta; 
EXTERN int p_sb; 
EXTERN long p_tpm; 
EXTERN char_u *p_tal; 
EXTERN char_u *p_sps; 
EXTERN int p_spr; 
EXTERN int p_sol; 
EXTERN char_u *p_su; 
EXTERN char_u *p_swb; 
EXTERN unsigned swb_flags;
#if defined(IN_OPTION_C)
static char *(p_swb_values[]) =
{ "useopen", "usetab", "split", "newtab", "vsplit", "uselast", NULL };
#endif
#define SWB_USEOPEN 0x001
#define SWB_USETAB 0x002
#define SWB_SPLIT 0x004
#define SWB_NEWTAB 0x008
#define SWB_VSPLIT 0x010
#define SWB_USELAST 0x020
EXTERN int p_tbs; 
EXTERN char_u *p_tc; 
EXTERN unsigned tc_flags; 
#if defined(IN_OPTION_C)
static char *(p_tc_values[]) =
{ "followic", "ignore", "match", "followscs", "smart", NULL };
#endif
#define TC_FOLLOWIC 0x01
#define TC_IGNORE 0x02
#define TC_MATCH 0x04
#define TC_FOLLOWSCS 0x08
#define TC_SMART 0x10
EXTERN long p_tl; 
EXTERN int p_tr; 
EXTERN char_u *p_tags; 
EXTERN int p_tgst; 
EXTERN int p_tbidi; 
EXTERN int p_terse; 
EXTERN int p_to; 
EXTERN int p_timeout; 
EXTERN long p_tm; 
EXTERN int p_title; 
EXTERN long p_titlelen; 
EXTERN char_u *p_titleold; 
EXTERN char_u *p_titlestring; 
EXTERN char_u *p_tsr; 
EXTERN int p_tgc; 
EXTERN int p_ttimeout; 
EXTERN long p_ttm; 
EXTERN char_u *p_udir; 
EXTERN long p_ul; 
EXTERN long p_ur; 
EXTERN long p_uc; 
EXTERN long p_ut; 
EXTERN char_u *p_fcs; 
EXTERN char_u *p_shada; 
EXTERN char *p_shadafile; 
EXTERN char_u *p_vdir; 
EXTERN char_u *p_vop; 
EXTERN unsigned vop_flags; 
EXTERN int p_vb; 
EXTERN char_u *p_ve; 
EXTERN unsigned ve_flags;
#if defined(IN_OPTION_C)
static char *(p_ve_values[]) = {"block", "insert", "all", "onemore", NULL};
#endif
#define VE_BLOCK 5 
#define VE_INSERT 6 
#define VE_ALL 4
#define VE_ONEMORE 8
EXTERN long p_verbose; 
#if defined(IN_OPTION_C)
char_u *p_vfile = (char_u *)""; 
#else
extern char_u *p_vfile; 
#endif
EXTERN int p_warn; 
EXTERN char_u *p_wop; 
EXTERN unsigned wop_flags;
#if defined(IN_OPTION_C)
static char *(p_wop_values[]) = { "tagfile", "pum", NULL };
#endif
#define WOP_TAGFILE 0x01
#define WOP_PUM 0x02
EXTERN long p_window; 
EXTERN char_u *p_wak; 
EXTERN char_u *p_wig; 
EXTERN char_u *p_ww; 
EXTERN long p_wc; 
EXTERN long p_wcm; 
EXTERN int p_wic; 
EXTERN char_u *p_wim; 
EXTERN int p_wmnu; 
EXTERN long p_wh; 
EXTERN long p_wmh; 
EXTERN long p_wmw; 
EXTERN long p_wiw; 
EXTERN int p_ws; 
EXTERN int p_write; 
EXTERN int p_wa; 
EXTERN int p_wb; 
EXTERN long p_wd; 

EXTERN int p_force_on; 
EXTERN int p_force_off; 






enum {
BV_AI = 0
, BV_AR
, BV_BH
, BV_BKC
, BV_BT
, BV_EFM
, BV_GP
, BV_MP
, BV_BIN
, BV_BL
, BV_BOMB
, BV_CHANNEL
, BV_CI
, BV_CIN
, BV_CINK
, BV_CINO
, BV_CINW
, BV_CM
, BV_CMS
, BV_COM
, BV_CPT
, BV_DICT
, BV_TSR
, BV_CFU
, BV_DEF
, BV_INC
, BV_EOL
, BV_FIXEOL
, BV_EP
, BV_ET
, BV_FENC
, BV_FP
, BV_BEXPR
, BV_FEX
, BV_FF
, BV_FLP
, BV_FO
, BV_FT
, BV_IMI
, BV_IMS
, BV_INDE
, BV_INDK
, BV_INEX
, BV_INF
, BV_ISK
, BV_KMAP
, BV_KP
, BV_LISP
, BV_LW
, BV_MENC
, BV_MA
, BV_ML
, BV_MOD
, BV_MPS
, BV_NF
, BV_OFU
, BV_PATH
, BV_PI
, BV_QE
, BV_RO
, BV_SCBK
, BV_SI
, BV_SMC
, BV_SYN
, BV_SPC
, BV_SPF
, BV_SPL
, BV_STS
, BV_SUA
, BV_SW
, BV_SWF
, BV_TFU
, BV_TAGS
, BV_TC
, BV_TS
, BV_TW
, BV_TX
, BV_UDF
, BV_UL
, BV_WM
, BV_COUNT 
};






enum {
WV_LIST = 0
, WV_ARAB
, WV_COCU
, WV_COLE
, WV_CRBIND
, WV_BRI
, WV_BRIOPT
, WV_DIFF
, WV_FDC
, WV_FEN
, WV_FDI
, WV_FDL
, WV_FDM
, WV_FML
, WV_FDN
, WV_FDE
, WV_FDT
, WV_FMR
, WV_LBR
, WV_NU
, WV_RNU
, WV_NUW
, WV_PVW
, WV_RL
, WV_RLC
, WV_SCBIND
, WV_SCROLL
, WV_SISO
, WV_SO
, WV_SPELL
, WV_CUC
, WV_CUL
, WV_CC
, WV_STL
, WV_WFH
, WV_WFW
, WV_WRAP
, WV_SCL
, WV_WINHL
, WV_FCS
, WV_LCS
, WV_WINBL
, WV_COUNT 
};


#define NO_LOCAL_UNDOLEVEL -123456

#define SB_MAX 100000 


typedef struct {
sctx_T script_ctx; 
uint64_t channel_id; 
} LastSet;

#endif 
