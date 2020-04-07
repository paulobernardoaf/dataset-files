#define P_BOOL 0x01 
#define P_NUM 0x02 
#define P_STRING 0x04 
#define P_ALLOCED 0x08 
#define P_EXPAND 0x10 
#define P_NODEFAULT 0x40 
#define P_DEF_ALLOCED 0x80 
#define P_WAS_SET 0x100 
#define P_NO_MKRC 0x200 
#define P_VI_DEF 0x400 
#define P_VIM 0x800 
#define P_RSTAT 0x1000 
#define P_RWIN 0x2000 
#define P_RBUF 0x4000 
#define P_RALL 0x6000 
#define P_RCLR 0x7000 
#define P_COMMA 0x8000 
#define P_ONECOMMA 0x18000L 
#define P_NODUP 0x20000L 
#define P_FLAGLIST 0x40000L 
#define P_SECURE 0x80000L 
#define P_GETTEXT 0x100000L 
#define P_NOGLOB 0x200000L 
#define P_NFNAME 0x400000L 
#define P_INSECURE 0x800000L 
#define P_PRI_MKRC 0x1000000L 
#define P_NO_ML 0x2000000L 
#define P_CURSWANT 0x4000000L 
#define P_NDNAME 0x8000000L 
#define P_RWINONLY 0x10000000L 
#define P_MLE 0x20000000L 
#if defined(AMIGA)
#define DFLT_EFM "%f>%l:%c:%t:%n:%m,%f:%l: %t%*\\D%n: %m,%f %l %t%*\\D%n: %m,%*[^\"]\"%f\"%*\\D%l: %m,%f:%l:%m,%f|%l| %m"
#else
#if defined(MSWIN)
#define DFLT_EFM "%f(%l) \\=: %t%*\\D%n: %m,%*[^\"]\"%f\"%*\\D%l: %m,%f(%l) \\=: %m,%*[^ ] %f %l: %m,%f:%l:%c:%m,%f(%l):%m,%f:%l:%m,%f|%l| %m"
#else
#if defined(__QNX__)
#define DFLT_EFM "%f(%l):%*[^WE]%t%*\\D%n:%m,%f|%l| %m"
#else
#if defined(VMS)
#define DFLT_EFM "%A%p^,%C%%CC-%t-%m,%Cat line number %l in file %f,%f|%l| %m"
#else 
#if defined(EBCDIC)
#define DFLT_EFM "%*[^ ] %*[^ ] %f:%l%*[ ]%m,%*[^\"]\"%f\"%*\\D%l: %m,\"%f\"%*\\D%l: %m,%f:%l:%c:%m,%f(%l):%m,%f:%l:%m,\"%f\"\\, line %l%*\\D%c%*[^ ] %m,%D%*\\a[%*\\d]: Entering directory %*[`']%f',%X%*\\a[%*\\d]: Leaving directory %*[`']%f',%DMaking %*\\a in %f,%f|%l| %m"
#else
#define DFLT_EFM "%*[^\"]\"%f\"%*\\D%l: %m,\"%f\"%*\\D%l: %m,%-G%f:%l: (Each undeclared identifier is reported only once,%-G%f:%l: for each function it appears in.),%-GIn file included from %f:%l:%c:,%-GIn file included from %f:%l:%c\\,,%-GIn file included from %f:%l:%c,%-GIn file included from %f:%l,%-G%*[ ]from %f:%l:%c,%-G%*[ ]from %f:%l:,%-G%*[ ]from %f:%l\\,,%-G%*[ ]from %f:%l,%f:%l:%c:%m,%f(%l):%m,%f:%l:%m,\"%f\"\\, line %l%*\\D%c%*[^ ] %m,%D%*\\a[%*\\d]: Entering directory %*[`']%f',%X%*\\a[%*\\d]: Leaving directory %*[`']%f',%D%*\\a: Entering directory %*[`']%f',%X%*\\a: Leaving directory %*[`']%f',%DMaking %*\\a in %f,%f|%l| %m"
#endif
#endif
#endif
#endif
#endif
#define DFLT_GREPFORMAT "%f:%l:%m,%f:%l%m,%f %l%m"
#define FF_DOS "dos"
#define FF_MAC "mac"
#define FF_UNIX "unix"
#if defined(USE_CRNL)
#define DFLT_FF "dos"
#define DFLT_FFS_VIM "dos,unix"
#define DFLT_FFS_VI "dos,unix" 
#define DFLT_TEXTAUTO TRUE
#else
#define DFLT_FF "unix"
#define DFLT_FFS_VIM "unix,dos"
#if defined(__CYGWIN__)
#define DFLT_FFS_VI "unix,dos" 
#define DFLT_TEXTAUTO TRUE
#else
#define DFLT_FFS_VI ""
#define DFLT_TEXTAUTO FALSE
#endif
#endif
#define ENC_UCSBOM "ucs-bom" 
#define ENC_DFLT "latin1"
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
#define DFLT_FO_VIM "tcq"
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
#define CPO_GOTO1 'g' 
#define CPO_INSEND 'H' 
#define CPO_INTMOD 'i' 
#define CPO_INDENT 'I' 
#define CPO_JOINSP 'j' 
#define CPO_ENDOFSENT 'J' 
#define CPO_KEYCODE 'k' 
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
#define CPO_CW 'w' 
#define CPO_FWRITE 'W' 
#define CPO_ESC 'x'
#define CPO_REPLCNT 'X' 
#define CPO_YANK 'y'
#define CPO_KEEPRO 'Z' 
#define CPO_DOLLAR '$'
#define CPO_FILTER '!'
#define CPO_MATCH '%'
#define CPO_STAR '*' 
#define CPO_PLUS '+' 
#define CPO_MINUS '-' 
#define CPO_SPECI '<' 
#define CPO_REGAPPEND '>' 
#define CPO_HASH '#' 
#define CPO_PARA '{' 
#define CPO_TSIZE '|' 
#define CPO_PRESERVE '&' 
#define CPO_SUBPERCENT '/' 
#define CPO_BACKSL '\\' 
#define CPO_CHDIR '.' 
#define CPO_SCOLON ';' 
#define CPO_VIM "aABceFs"
#define CPO_VI "aAbBcCdDeEfFgHiIjJkKlLmMnoOpPqrRsStuvwWxXyZ$!%*-+<>;"
#define CPO_ALL "aAbBcCdDeEfFgHiIjJkKlLmMnoOpPqrRsStuvwWxXyZ$!%*-+<>#{|&/\\.;"
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
#define SHM_RO 'r' 
#define SHM_MOD 'm' 
#define SHM_FILE 'f' 
#define SHM_LAST 'i' 
#define SHM_TEXT 'x' 
#define SHM_LINES 'l' 
#define SHM_NEW 'n' 
#define SHM_WRI 'w' 
#define SHM_A "rmfixlnw" 
#define SHM_WRITE 'W' 
#define SHM_TRUNC 't' 
#define SHM_TRUNCALL 'T' 
#define SHM_OVER 'o' 
#define SHM_OVERALL 'O' 
#define SHM_SEARCH 's' 
#define SHM_ATTENTION 'A' 
#define SHM_INTRO 'I' 
#define SHM_COMPLETIONMENU 'c' 
#define SHM_RECORDING 'q' 
#define SHM_FILEINFO 'F' 
#define SHM_SEARCHCOUNT 'S' 
#define SHM_POSIX "AS" 
#define SHM_ALL "rmfixlnwaWtToOsAIcqFS" 
#define GO_TERMINAL '!' 
#define GO_ASEL 'a' 
#define GO_ASELML 'A' 
#define GO_BOT 'b' 
#define GO_CONDIALOG 'c' 
#define GO_DARKTHEME 'd' 
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
#define GO_TEAROFF 't' 
#define GO_TOOLBAR 'T' 
#define GO_FOOTER 'F' 
#define GO_VERTICAL 'v' 
#define GO_KEEPWINSIZE 'k' 
#define GO_ALL "!aAbcdefFghilmMprtTvk" 
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
#define STL_FILEPATH 'f' 
#define STL_FULLPATH 'F' 
#define STL_FILENAME 't' 
#define STL_COLUMN 'c' 
#define STL_VIRTCOL 'v' 
#define STL_VIRTCOL_ALT 'V' 
#define STL_LINE 'l' 
#define STL_NUMLINES 'L' 
#define STL_BUFNO 'n' 
#define STL_KEYMAP 'k' 
#define STL_OFFSET 'o' 
#define STL_OFFSET_X 'O' 
#define STL_BYTEVAL 'b' 
#define STL_BYTEVAL_X 'B' 
#define STL_ROFLAG 'r' 
#define STL_ROFLAG_ALT 'R' 
#define STL_HELPFLAG 'h' 
#define STL_HELPFLAG_ALT 'H' 
#define STL_FILETYPE 'y' 
#define STL_FILETYPE_ALT 'Y' 
#define STL_PREVIEWFLAG 'w' 
#define STL_PREVIEWFLAG_ALT 'W' 
#define STL_MODIFIED 'm' 
#define STL_MODIFIED_ALT 'M' 
#define STL_QUICKFIX 'q' 
#define STL_PERCENTAGE 'p' 
#define STL_ALTPERCENT 'P' 
#define STL_ARGLISTSTAT 'a' 
#define STL_PAGENUM 'N' 
#define STL_VIM_EXPR '{' 
#define STL_MIDDLEMARK '=' 
#define STL_TRUNCMARK '<' 
#define STL_USER_HL '*' 
#define STL_HIGHLIGHT '#' 
#define STL_TABPAGENR 'T' 
#define STL_TABCLOSENR 'X' 
#define STL_ALL ((char_u *) "fFtcvVlLknoObBrRhHmYyWwMqpPaN{#")
#define WIM_FULL 0x01
#define WIM_LONGEST 0x02
#define WIM_LIST 0x04
#define WIM_BUFLASTUSED 0x08
#define BS_INDENT 'i' 
#define BS_EOL 'o' 
#define BS_START 's' 
#define CULOPT_LINE 0x01 
#define CULOPT_SCRLINE 0x02 
#define CULOPT_NBR 0x04 
#define LISPWORD_VALUE "defun,define,defmacro,set!,lambda,if,case,let,flet,let*,letrec,do,do*,define-syntax,let-syntax,letrec-syntax,destructuring-bind,defpackage,defparameter,defstruct,deftype,defvar,do-all-symbols,do-external-symbols,do-symbols,dolist,dotimes,ecase,etypecase,eval-when,labels,macrolet,multiple-value-bind,multiple-value-call,multiple-value-prog1,multiple-value-setq,prog1,progv,typecase,unless,unwind-protect,when,with-input-from-string,with-open-file,with-open-stream,with-output-to-string,with-package-iterator,define-condition,handler-bind,handler-case,restart-bind,restart-case,with-simple-restart,store-value,use-value,muffle-warning,abort,continue,with-slots,with-slots*,with-accessors,with-accessors*,defclass,defmethod,print-unreadable-object"
#if defined(FEAT_RIGHTLEFT)
EXTERN long p_aleph; 
#endif
EXTERN char_u *p_ambw; 
#if defined(FEAT_AUTOCHDIR)
EXTERN int p_acd; 
#endif
EXTERN int p_ai; 
EXTERN int p_bin; 
EXTERN int p_bomb; 
EXTERN int p_bl; 
#if defined(FEAT_CINDENT)
EXTERN int p_cin; 
EXTERN char_u *p_cink; 
#endif
#if defined(FEAT_SMARTINDENT) || defined(FEAT_CINDENT)
EXTERN char_u *p_cinw; 
#endif
#if defined(FEAT_COMPL_FUNC)
EXTERN char_u *p_cfu; 
EXTERN char_u *p_ofu; 
#endif
EXTERN int p_ci; 
#if defined(FEAT_GUI) && defined(MACOS_X)
EXTERN int *p_antialias; 
#endif
EXTERN int p_ar; 
EXTERN int p_aw; 
EXTERN int p_awa; 
EXTERN char_u *p_bs; 
EXTERN char_u *p_bg; 
EXTERN int p_bk; 
EXTERN char_u *p_bkc; 
EXTERN unsigned bkc_flags; 
#define BKC_YES 0x001
#define BKC_AUTO 0x002
#define BKC_NO 0x004
#define BKC_BREAKSYMLINK 0x008
#define BKC_BREAKHARDLINK 0x010
EXTERN char_u *p_bdir; 
EXTERN char_u *p_bex; 
EXTERN char_u *p_bo; 
EXTERN unsigned bo_flags;
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
#if defined(FEAT_WILDIGN)
EXTERN char_u *p_bsk; 
#endif
#if defined(FEAT_CRYPT)
EXTERN char_u *p_cm; 
#endif
#if defined(FEAT_BEVAL)
#if defined(FEAT_BEVAL_GUI)
EXTERN int p_beval; 
#endif
EXTERN long p_bdlay; 
#if defined(FEAT_EVAL)
EXTERN char_u *p_bexpr;
#endif
#if defined(FEAT_BEVAL_TERM)
EXTERN int p_bevalterm; 
#endif
#endif
#if defined(FEAT_BROWSE)
EXTERN char_u *p_bsdir; 
#endif
#if defined(FEAT_LINEBREAK)
EXTERN char_u *p_breakat; 
#endif
EXTERN char_u *p_bh; 
EXTERN char_u *p_bt; 
EXTERN char_u *p_cmp; 
EXTERN unsigned cmp_flags;
#define CMP_INTERNAL 0x001
#define CMP_KEEPASCII 0x002
EXTERN char_u *p_enc; 
EXTERN int p_deco; 
#if defined(FEAT_EVAL)
EXTERN char_u *p_ccv; 
#endif
EXTERN char_u *p_cino; 
#if defined(FEAT_CMDWIN)
EXTERN char_u *p_cedit; 
EXTERN long p_cwh; 
#endif
#if defined(FEAT_CLIPBOARD)
EXTERN char_u *p_cb; 
#endif
EXTERN long p_ch; 
#if defined(FEAT_FOLDING)
EXTERN char_u *p_cms; 
#endif
EXTERN char_u *p_cpt; 
#if defined(FEAT_GUI_DIALOG) || defined(FEAT_CON_DIALOG)
EXTERN int p_confirm; 
#endif
EXTERN int p_cp; 
EXTERN char_u *p_cot; 
#if defined(BACKSLASH_IN_FILENAME)
EXTERN char_u *p_csl; 
#endif
EXTERN long p_ph; 
EXTERN long p_pw; 
EXTERN char_u *p_com; 
EXTERN char_u *p_cpo; 
#if defined(FEAT_CSCOPE)
EXTERN char_u *p_csprg; 
EXTERN int p_csre; 
#if defined(FEAT_QUICKFIX)
EXTERN char_u *p_csqf; 
#define CSQF_CMDS "sgdctefia"
#define CSQF_FLAGS "+-0"
#endif
EXTERN int p_cst; 
EXTERN long p_csto; 
EXTERN long p_cspc; 
EXTERN int p_csverbose; 
#endif
EXTERN char_u *p_debug; 
#if defined(FEAT_FIND_ID)
EXTERN char_u *p_def; 
EXTERN char_u *p_inc;
#endif
#if defined(FEAT_DIFF)
EXTERN char_u *p_dip; 
#if defined(FEAT_EVAL)
EXTERN char_u *p_dex; 
#endif
#endif
EXTERN char_u *p_dict; 
#if defined(FEAT_DIGRAPHS)
EXTERN int p_dg; 
#endif
EXTERN char_u *p_dir; 
EXTERN char_u *p_dy; 
EXTERN unsigned dy_flags;
#define DY_LASTLINE 0x001
#define DY_TRUNCATE 0x002
#define DY_UHEX 0x004
EXTERN int p_ed; 
EXTERN char_u *p_ead; 
EXTERN char_u *p_emoji; 
EXTERN int p_ea; 
EXTERN char_u *p_ep; 
EXTERN int p_eb; 
#if defined(FEAT_QUICKFIX)
EXTERN char_u *p_ef; 
EXTERN char_u *p_efm; 
EXTERN char_u *p_gefm; 
EXTERN char_u *p_gp; 
#endif
EXTERN int p_eol; 
EXTERN int p_ek; 
EXTERN char_u *p_ei; 
EXTERN int p_et; 
EXTERN int p_exrc; 
EXTERN char_u *p_fenc; 
EXTERN char_u *p_fencs; 
EXTERN char_u *p_ff; 
EXTERN char_u *p_ffs; 
EXTERN long p_fic; 
EXTERN char_u *p_ft; 
EXTERN char_u *p_fcs; 
EXTERN int p_fixeol; 
#if defined(FEAT_FOLDING)
EXTERN char_u *p_fcl; 
EXTERN long p_fdls; 
EXTERN char_u *p_fdo; 
EXTERN unsigned fdo_flags;
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
#endif
#if defined(FEAT_EVAL)
EXTERN char_u *p_fex; 
#endif
EXTERN char_u *p_flp; 
EXTERN char_u *p_fo; 
EXTERN char_u *p_fp; 
#if defined(HAVE_FSYNC)
EXTERN int p_fs; 
#endif
EXTERN int p_gd; 
#if defined(FEAT_PROP_POPUP)
#if defined(FEAT_QUICKFIX)
EXTERN char_u *p_cpp; 
#endif
EXTERN char_u *p_pvp; 
#endif
#if defined(FEAT_PRINTER)
EXTERN char_u *p_pdev; 
#if defined(FEAT_POSTSCRIPT)
EXTERN char_u *p_penc; 
EXTERN char_u *p_pexpr; 
EXTERN char_u *p_pmfn; 
EXTERN char_u *p_pmcs; 
#endif
EXTERN char_u *p_pfn; 
EXTERN char_u *p_popt; 
EXTERN char_u *p_header; 
#endif
EXTERN int p_prompt; 
#if defined(FEAT_GUI)
EXTERN char_u *p_guifont; 
#if defined(FEAT_XFONTSET)
EXTERN char_u *p_guifontset; 
#endif
EXTERN char_u *p_guifontwide; 
EXTERN int p_guipty; 
#endif
#if defined(FEAT_GUI_GTK) || defined(FEAT_GUI_X11)
EXTERN long p_ghr; 
#endif
#if defined(CURSOR_SHAPE)
EXTERN char_u *p_guicursor; 
#endif
#if defined(FEAT_MOUSESHAPE)
EXTERN char_u *p_mouseshape; 
#endif
#if defined(FEAT_GUI)
EXTERN char_u *p_go; 
#endif
#if defined(FEAT_GUI_TABLINE)
EXTERN char_u *p_gtl; 
EXTERN char_u *p_gtt; 
#endif
EXTERN char_u *p_hf; 
EXTERN long p_hh; 
#if defined(FEAT_MULTI_LANG)
EXTERN char_u *p_hlg; 
#endif
EXTERN int p_hid; 
EXTERN char_u *p_hl; 
EXTERN int p_hls; 
EXTERN long p_hi; 
#if defined(FEAT_RIGHTLEFT)
EXTERN int p_hkmap; 
EXTERN int p_hkmapp; 
#if defined(FEAT_ARABIC)
EXTERN int p_arshape; 
#endif
#endif
#if defined(FEAT_TITLE)
EXTERN int p_icon; 
EXTERN char_u *p_iconstring; 
#endif
EXTERN int p_ic; 
#if defined(FEAT_XIM) && defined(FEAT_GUI_GTK)
EXTERN char_u *p_imak; 
#define IM_ON_THE_SPOT 0L
#define IM_OVER_THE_SPOT 1L
EXTERN long p_imst; 
#endif
#if defined(FEAT_EVAL)
EXTERN char_u *p_imaf; 
EXTERN char_u *p_imsf; 
#endif
EXTERN int p_imcmdline; 
EXTERN int p_imdisable; 
EXTERN long p_iminsert; 
EXTERN long p_imsearch; 
EXTERN int p_inf; 
#if defined(FEAT_FIND_ID) && defined(FEAT_EVAL)
EXTERN char_u *p_inex; 
#endif
EXTERN int p_is; 
#if defined(FEAT_CINDENT) && defined(FEAT_EVAL)
EXTERN char_u *p_inde; 
EXTERN char_u *p_indk; 
#endif
EXTERN int p_im; 
EXTERN char_u *p_isf; 
EXTERN char_u *p_isi; 
EXTERN char_u *p_isk; 
EXTERN char_u *p_isp; 
EXTERN int p_js; 
#if defined(FEAT_CRYPT)
EXTERN char_u *p_key; 
#endif
#if defined(FEAT_KEYMAP)
EXTERN char_u *p_keymap; 
#endif
EXTERN char_u *p_kp; 
EXTERN char_u *p_km; 
#if defined(FEAT_LANGMAP)
EXTERN char_u *p_langmap; 
EXTERN int p_lnr; 
EXTERN int p_lrm; 
#endif
#if defined(FEAT_MENU) && defined(FEAT_MULTI_LANG)
EXTERN char_u *p_lm; 
#endif
#if defined(FEAT_GUI)
EXTERN long p_linespace; 
#endif
#if defined(FEAT_LISP)
EXTERN int p_lisp; 
EXTERN char_u *p_lispwords; 
#endif
EXTERN long p_ls; 
EXTERN long p_stal; 
EXTERN char_u *p_lcs; 
EXTERN int p_lz; 
EXTERN int p_lpl; 
#if defined(DYNAMIC_LUA)
EXTERN char_u *p_luadll; 
#endif
#if defined(FEAT_GUI_MAC)
EXTERN int p_macatsui; 
#endif
EXTERN int p_magic; 
EXTERN char_u *p_menc; 
#if defined(FEAT_QUICKFIX)
EXTERN char_u *p_mef; 
EXTERN char_u *p_mp; 
#endif
EXTERN char_u *p_mps; 
#if defined(FEAT_SIGNS)
EXTERN char_u *p_scl; 
#endif
#if defined(FEAT_SYN_HL)
EXTERN char_u *p_cc; 
EXTERN int p_cc_cols[256]; 
#endif
EXTERN long p_mat; 
EXTERN long p_mco; 
#if defined(FEAT_EVAL)
EXTERN long p_mfd; 
#endif
EXTERN long p_mmd; 
EXTERN long p_mm; 
EXTERN long p_mmp; 
EXTERN long p_mmt; 
#if defined(FEAT_MENU)
EXTERN long p_mis; 
#endif
#if defined(FEAT_SPELL)
EXTERN char_u *p_msm; 
#endif
EXTERN int p_ml; 
EXTERN long p_mle; 
EXTERN long p_mls; 
EXTERN int p_ma; 
EXTERN int p_mod; 
EXTERN char_u *p_mouse; 
#if defined(FEAT_GUI)
EXTERN int p_mousef; 
EXTERN int p_mh; 
#endif
EXTERN char_u *p_mousem; 
EXTERN long p_mouset; 
EXTERN int p_more; 
#if defined(FEAT_MZSCHEME)
EXTERN long p_mzq; 
#if defined(DYNAMIC_MZSCHEME)
EXTERN char_u *p_mzschemedll; 
EXTERN char_u *p_mzschemegcdll; 
#endif
#endif
EXTERN char_u *p_nf; 
#if defined(MSWIN)
EXTERN int p_odev; 
#endif
EXTERN char_u *p_opfunc; 
EXTERN char_u *p_para; 
EXTERN int p_paste; 
EXTERN char_u *p_pt; 
#if defined(FEAT_EVAL) && defined(FEAT_DIFF)
EXTERN char_u *p_pex; 
#endif
EXTERN char_u *p_pm; 
EXTERN char_u *p_path; 
#if defined(FEAT_SEARCHPATH)
EXTERN char_u *p_cdpath; 
#endif
#if defined(DYNAMIC_PERL)
EXTERN char_u *p_perldll; 
#endif
EXTERN int p_pi; 
#if defined(DYNAMIC_PYTHON3)
EXTERN char_u *p_py3dll; 
#endif
#if defined(FEAT_PYTHON3)
EXTERN char_u *p_py3home; 
#endif
#if defined(DYNAMIC_PYTHON)
EXTERN char_u *p_pydll; 
#endif
#if defined(FEAT_PYTHON)
EXTERN char_u *p_pyhome; 
#endif
#if defined(FEAT_PYTHON) || defined(FEAT_PYTHON3)
EXTERN long p_pyx; 
#endif
#if defined(FEAT_TEXTOBJ)
EXTERN char_u *p_qe; 
#endif
EXTERN int p_ro; 
#if defined(FEAT_RELTIME)
EXTERN long p_rdt; 
#endif
EXTERN int p_remap; 
EXTERN long p_re; 
#if defined(FEAT_RENDER_OPTIONS)
EXTERN char_u *p_rop; 
#endif
EXTERN long p_report; 
#if defined(FEAT_QUICKFIX)
EXTERN long p_pvh; 
#endif
#if defined(MSWIN)
EXTERN int p_rs; 
#endif
#if defined(FEAT_RIGHTLEFT)
EXTERN int p_ari; 
EXTERN int p_ri; 
#endif
#if defined(DYNAMIC_RUBY)
EXTERN char_u *p_rubydll; 
#endif
#if defined(FEAT_CMDL_INFO)
EXTERN int p_ru; 
#endif
#if defined(FEAT_STL_OPT)
EXTERN char_u *p_ruf; 
#endif
EXTERN char_u *p_pp; 
EXTERN char_u *p_rtp; 
EXTERN long p_sj; 
#if defined(MSWIN) && defined(FEAT_GUI)
EXTERN int p_scf; 
#endif
EXTERN long p_so; 
EXTERN char_u *p_sbo; 
EXTERN char_u *p_sections; 
EXTERN int p_secure; 
EXTERN char_u *p_sel; 
EXTERN char_u *p_slm; 
#if defined(FEAT_SESSION)
EXTERN char_u *p_ssop; 
EXTERN unsigned ssop_flags;
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
#define SSOP_TERMINAL 0x10000
#endif
EXTERN char_u *p_sh; 
EXTERN char_u *p_shcf; 
#if defined(FEAT_QUICKFIX)
EXTERN char_u *p_sp; 
#endif
EXTERN char_u *p_shq; 
EXTERN char_u *p_sxq; 
EXTERN char_u *p_sxe; 
EXTERN char_u *p_srr; 
#if defined(AMIGA)
EXTERN long p_st; 
#endif
EXTERN int p_stmp; 
#if defined(BACKSLASH_IN_FILENAME)
EXTERN int p_ssl; 
#endif
#if defined(FEAT_STL_OPT)
EXTERN char_u *p_stl; 
#endif
EXTERN int p_sr; 
EXTERN long p_sw; 
EXTERN char_u *p_shm; 
EXTERN int p_sn; 
#if defined(FEAT_LINEBREAK)
EXTERN char_u *p_sbr; 
#endif
#if defined(FEAT_CMDL_INFO)
EXTERN int p_sc; 
#endif
EXTERN int p_sft; 
EXTERN int p_sm; 
EXTERN int p_smd; 
EXTERN long p_ss; 
EXTERN long p_siso; 
EXTERN int p_scs; 
#if defined(FEAT_SMARTINDENT)
EXTERN int p_si; 
#endif
EXTERN int p_sta; 
EXTERN long p_sts; 
EXTERN int p_sb; 
#if defined(FEAT_SEARCHPATH)
EXTERN char_u *p_sua; 
#endif
EXTERN int p_swf; 
#if defined(FEAT_SYN_HL)
EXTERN long p_smc; 
#endif
EXTERN long p_tpm; 
#if defined(FEAT_STL_OPT)
EXTERN char_u *p_tal; 
#endif
#if defined(FEAT_EVAL)
EXTERN char_u *p_tfu; 
#endif
#if defined(FEAT_SPELL)
EXTERN char_u *p_spc; 
EXTERN char_u *p_spf; 
EXTERN char_u *p_spl; 
EXTERN char_u *p_sps; 
#endif
EXTERN int p_spr; 
EXTERN int p_sol; 
EXTERN char_u *p_su; 
EXTERN char_u *p_sws; 
EXTERN char_u *p_swb; 
EXTERN unsigned swb_flags;
#define SWB_USEOPEN 0x001
#define SWB_USETAB 0x002
#define SWB_SPLIT 0x004
#define SWB_NEWTAB 0x008
#define SWB_VSPLIT 0x010
#define SWB_USELAST 0x020
EXTERN char_u *p_syn; 
EXTERN long p_ts; 
EXTERN int p_tbs; 
EXTERN char_u *p_tc; 
EXTERN unsigned tc_flags; 
#define TC_FOLLOWIC 0x01
#define TC_IGNORE 0x02
#define TC_MATCH 0x04
#define TC_FOLLOWSCS 0x08
#define TC_SMART 0x10
EXTERN long p_tl; 
EXTERN int p_tr; 
EXTERN char_u *p_tags; 
EXTERN int p_tgst; 
#if defined(DYNAMIC_TCL)
EXTERN char_u *p_tcldll; 
#endif
#if defined(FEAT_ARABIC)
EXTERN int p_tbidi; 
#endif
EXTERN char_u *p_tenc; 
#if defined(FEAT_TERMGUICOLORS)
EXTERN int p_tgc; 
#endif
#if defined(FEAT_TERMINAL)
EXTERN long p_twsl; 
#endif
#if defined(MSWIN) && defined(FEAT_TERMINAL)
EXTERN char_u *p_twt; 
#endif
EXTERN int p_terse; 
EXTERN int p_ta; 
EXTERN int p_tx; 
EXTERN long p_tw; 
EXTERN int p_to; 
EXTERN int p_timeout; 
EXTERN long p_tm; 
#if defined(FEAT_TITLE)
EXTERN int p_title; 
EXTERN long p_titlelen; 
EXTERN char_u *p_titleold; 
EXTERN char_u *p_titlestring; 
#endif
EXTERN char_u *p_tsr; 
EXTERN int p_ttimeout; 
EXTERN long p_ttm; 
EXTERN int p_tbi; 
EXTERN int p_tf; 
#if defined(FEAT_TOOLBAR) && !defined(FEAT_GUI_MSWIN)
EXTERN char_u *p_toolbar; 
EXTERN unsigned toolbar_flags;
#define TOOLBAR_TEXT 0x01
#define TOOLBAR_ICONS 0x02
#define TOOLBAR_TOOLTIPS 0x04
#define TOOLBAR_HORIZ 0x08
#endif
#if defined(FEAT_TOOLBAR) && defined(FEAT_GUI_GTK)
EXTERN char_u *p_tbis; 
EXTERN unsigned tbis_flags;
#define TBIS_TINY 0x01
#define TBIS_SMALL 0x02
#define TBIS_MEDIUM 0x04
#define TBIS_LARGE 0x08
#define TBIS_HUGE 0x10
#define TBIS_GIANT 0x20
#endif
EXTERN long p_ttyscroll; 
#if defined(UNIX) || defined(VMS)
EXTERN char_u *p_ttym; 
EXTERN unsigned ttym_flags;
#define TTYM_XTERM 0x01
#define TTYM_XTERM2 0x02
#define TTYM_DEC 0x04
#define TTYM_NETTERM 0x08
#define TTYM_JSBTERM 0x10
#define TTYM_PTERM 0x20
#define TTYM_URXVT 0x40
#define TTYM_SGR 0x80
#endif
EXTERN char_u *p_udir; 
#if defined(FEAT_PERSISTENT_UNDO)
EXTERN int p_udf; 
#endif
EXTERN long p_ul; 
EXTERN long p_ur; 
EXTERN long p_uc; 
EXTERN long p_ut; 
#if defined(FEAT_VARTABS)
EXTERN char_u *p_vsts; 
EXTERN char_u *p_vts; 
#endif
#if defined(FEAT_VIMINFO)
EXTERN char_u *p_viminfo; 
EXTERN char_u *p_viminfofile; 
#endif
#if defined(FEAT_SESSION)
EXTERN char_u *p_vdir; 
EXTERN char_u *p_vop; 
EXTERN unsigned vop_flags; 
#endif
EXTERN int p_vb; 
EXTERN char_u *p_ve; 
EXTERN unsigned ve_flags;
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
EXTERN long p_window; 
#if defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_MOTIF) || defined(LINT) || defined (FEAT_GUI_GTK) || defined(FEAT_GUI_PHOTON)
#define FEAT_WAK
EXTERN char_u *p_wak; 
#endif
#if defined(FEAT_WILDIGN)
EXTERN char_u *p_wig; 
#endif
EXTERN int p_wiv; 
EXTERN char_u *p_ww; 
EXTERN long p_wc; 
EXTERN long p_wcm; 
EXTERN long p_wic; 
EXTERN char_u *p_wim; 
#if defined(FEAT_WILDMENU)
EXTERN int p_wmnu; 
#endif
EXTERN long p_wh; 
EXTERN long p_wmh; 
EXTERN long p_wmw; 
EXTERN long p_wiw; 
#if defined(MSWIN) && defined(FEAT_TERMINAL)
EXTERN char_u *p_winptydll; 
#endif
EXTERN long p_wm; 
EXTERN int p_ws; 
EXTERN int p_write; 
EXTERN int p_wa; 
EXTERN int p_wb; 
EXTERN long p_wd; 
enum
{
BV_AI = 0
, BV_AR
, BV_BH
, BV_BKC
, BV_BT
#if defined(FEAT_QUICKFIX)
, BV_EFM
, BV_GP
, BV_MP
#endif
, BV_BIN
, BV_BL
, BV_BOMB
, BV_CI
#if defined(FEAT_CINDENT)
, BV_CIN
, BV_CINK
, BV_CINO
#endif
#if defined(FEAT_SMARTINDENT) || defined(FEAT_CINDENT)
, BV_CINW
#endif
, BV_CM
#if defined(FEAT_FOLDING)
, BV_CMS
#endif
, BV_COM
, BV_CPT
, BV_DICT
, BV_TSR
#if defined(BACKSLASH_IN_FILENAME)
, BV_CSL
#endif
#if defined(FEAT_COMPL_FUNC)
, BV_CFU
#endif
#if defined(FEAT_FIND_ID)
, BV_DEF
, BV_INC
#endif
, BV_EOL
, BV_FIXEOL
, BV_EP
, BV_ET
, BV_FENC
, BV_FP
#if defined(FEAT_EVAL)
, BV_BEXPR
, BV_FEX
#endif
, BV_FF
, BV_FLP
, BV_FO
, BV_FT
, BV_IMI
, BV_IMS
#if defined(FEAT_CINDENT) && defined(FEAT_EVAL)
, BV_INDE
, BV_INDK
#endif
#if defined(FEAT_FIND_ID) && defined(FEAT_EVAL)
, BV_INEX
#endif
, BV_INF
, BV_ISK
#if defined(FEAT_CRYPT)
, BV_KEY
#endif
#if defined(FEAT_KEYMAP)
, BV_KMAP
#endif
, BV_KP
#if defined(FEAT_LISP)
, BV_LISP
, BV_LW
#endif
, BV_MENC
, BV_MA
, BV_ML
, BV_MOD
, BV_MPS
, BV_NF
#if defined(FEAT_COMPL_FUNC)
, BV_OFU
#endif
, BV_PATH
, BV_PI
#if defined(FEAT_TEXTOBJ)
, BV_QE
#endif
, BV_RO
#if defined(FEAT_SMARTINDENT)
, BV_SI
#endif
, BV_SN
#if defined(FEAT_SYN_HL)
, BV_SMC
, BV_SYN
#endif
#if defined(FEAT_SPELL)
, BV_SPC
, BV_SPF
, BV_SPL
#endif
, BV_STS
#if defined(FEAT_SEARCHPATH)
, BV_SUA
#endif
, BV_SW
, BV_SWF
#if defined(FEAT_EVAL)
, BV_TFU
#endif
, BV_TAGS
, BV_TC
, BV_TS
, BV_TW
, BV_TX
, BV_UDF
, BV_UL
, BV_WM
#if defined(FEAT_TERMINAL)
, BV_TWSL
#endif
#if defined(FEAT_VARTABS)
, BV_VSTS
, BV_VTS
#endif
, BV_COUNT 
};
enum
{
WV_LIST = 0
#if defined(FEAT_ARABIC)
, WV_ARAB
#endif
#if defined(FEAT_CONCEAL)
, WV_COCU
, WV_COLE
#endif
#if defined(FEAT_TERMINAL)
, WV_TWK
, WV_TWS
#endif
, WV_CRBIND
#if defined(FEAT_LINEBREAK)
, WV_BRI
, WV_BRIOPT
#endif
, WV_WCR
#if defined(FEAT_DIFF)
, WV_DIFF
#endif
#if defined(FEAT_FOLDING)
, WV_FDC
, WV_FEN
, WV_FDI
, WV_FDL
, WV_FDM
, WV_FML
, WV_FDN
#if defined(FEAT_EVAL)
, WV_FDE
, WV_FDT
#endif
, WV_FMR
#endif
#if defined(FEAT_LINEBREAK)
, WV_LBR
#endif
, WV_NU
, WV_RNU
#if defined(FEAT_LINEBREAK)
, WV_NUW
#endif
#if defined(FEAT_QUICKFIX)
, WV_PVW
#endif
#if defined(FEAT_RIGHTLEFT)
, WV_RL
, WV_RLC
#endif
, WV_SCBIND
, WV_SCROLL
, WV_SISO
, WV_SO
#if defined(FEAT_SPELL)
, WV_SPELL
#endif
#if defined(FEAT_SYN_HL)
, WV_CUC
, WV_CUL
, WV_CULOPT
, WV_CC
#endif
#if defined(FEAT_LINEBREAK)
, WV_SBR
#endif
#if defined(FEAT_STL_OPT)
, WV_STL
#endif
, WV_WFH
, WV_WFW
, WV_WRAP
#if defined(FEAT_SIGNS)
, WV_SCL
#endif
, WV_COUNT 
};
#define NO_LOCAL_UNDOLEVEL -123456
