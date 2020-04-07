#if !defined(FEAT_TINY) && !defined(FEAT_SMALL) && !defined(FEAT_NORMAL) && !defined(FEAT_BIG) && !defined(FEAT_HUGE)
#endif
#if !defined(FEAT_TINY) && !defined(FEAT_SMALL) && !defined(FEAT_NORMAL) && !defined(FEAT_BIG) && !defined(FEAT_HUGE)
#if defined(UNIX) || defined(MSWIN) || defined(MACOS_X)
#define FEAT_HUGE
#else
#if defined(MSWIN) || defined(VMS) || defined(AMIGA)
#define FEAT_BIG
#else
#define FEAT_NORMAL
#endif
#endif
#endif
#if defined(FEAT_HUGE)
#define FEAT_BIG
#endif
#if defined(FEAT_BIG)
#define FEAT_NORMAL
#endif
#if defined(FEAT_NORMAL)
#define FEAT_SMALL
#endif
#if defined(FEAT_SMALL)
#define FEAT_TINY
#endif
#if defined(FEAT_SMALL)
#define MAX_MSG_HIST_LEN 200
#else
#define MAX_MSG_HIST_LEN 20
#endif
#if defined(FEAT_SMALL)
#define FEAT_JUMPLIST
#endif
#if defined(FEAT_SMALL)
#define FEAT_CMDWIN
#endif
#if defined(FEAT_NORMAL)
#define FEAT_FOLDING
#endif
#if defined(FEAT_NORMAL)
#define FEAT_DIGRAPHS
#endif
#if defined(FEAT_BIG)
#define FEAT_LANGMAP
#endif
#if defined(FEAT_BIG)
#define FEAT_KEYMAP
#endif
#if defined(FEAT_NORMAL)
#define VIM_BACKTICK 
#endif
#if defined(FEAT_NORMAL)
#define FEAT_CMDL_INFO
#endif
#if defined(FEAT_NORMAL)
#define FEAT_LINEBREAK
#endif
#if defined(FEAT_NORMAL)
#define FEAT_SEARCH_EXTRA
#endif
#if defined(FEAT_NORMAL)
#define FEAT_QUICKFIX
#endif
#if defined(FEAT_NORMAL)
#define FEAT_SEARCHPATH
#endif
#if defined(FEAT_NORMAL)
#if defined(FEAT_SEARCHPATH)
#define FEAT_FIND_ID
#endif
#endif
#if defined(FEAT_NORMAL)
#define FEAT_PATH_EXTRA
#endif
#if defined(FEAT_BIG) && !defined(DISABLE_RIGHTLEFT) && !defined(EBCDIC)
#define FEAT_RIGHTLEFT
#endif
#if defined(FEAT_BIG) && !defined(DISABLE_ARABIC) && !defined(EBCDIC)
#define FEAT_ARABIC
#endif
#if defined(FEAT_ARABIC)
#if !defined(FEAT_RIGHTLEFT)
#define FEAT_RIGHTLEFT
#endif
#endif
#if defined(FEAT_BIG)
#define FEAT_EMACS_TAGS
#endif
#if !defined(EBCDIC)
#define FEAT_TAG_BINS
#endif
#if defined(UNIX) && defined(FEAT_BIG) && !defined(FEAT_CSCOPE) && !defined(MACOS_X)
#define FEAT_CSCOPE
#endif
#if defined(FEAT_NORMAL)
#define FEAT_EVAL
#if defined(HAVE_FLOAT_FUNCS) || defined(MSWIN) || defined(MACOS_X)
#define FEAT_FLOAT
#endif
#endif
#if defined(FEAT_EVAL)
#define HAVE_SANDBOX
#endif
#if defined(FEAT_HUGE) && defined(FEAT_EVAL) && ((defined(HAVE_GETTIMEOFDAY) && defined(HAVE_SYS_TIME_H)) || defined(MSWIN))
#define FEAT_PROFILE
#endif
#if defined(FEAT_NORMAL) && defined(FEAT_EVAL) && ((defined(HAVE_GETTIMEOFDAY) && defined(HAVE_SYS_TIME_H)) || defined(MSWIN))
#define FEAT_RELTIME
#endif
#if defined(FEAT_RELTIME) && (defined(UNIX) || defined(MSWIN) || defined(VMS) )
#define FEAT_TIMERS
#endif
#if defined(FEAT_NORMAL) && defined(FEAT_EVAL)
#define FEAT_TEXTOBJ
#endif
#if defined(FEAT_EVAL)
#define FEAT_COMPL_FUNC
#endif
#if defined(FEAT_NORMAL) && (defined(MSWIN) || defined(FEAT_EVAL)) && !defined(AMIGA)
#define FEAT_PRINTER
#endif
#if defined(FEAT_PRINTER) && ((defined(MSWIN) && defined(MSWINPS)) || (!defined(MSWIN) && defined(FEAT_EVAL)))
#define FEAT_POSTSCRIPT
#endif
#if defined(FEAT_NORMAL)
#define FEAT_DIFF
#endif
#if defined(FEAT_NORMAL)
#define FEAT_TITLE
#endif
#if defined(FEAT_NORMAL)
#define FEAT_STL_OPT
#if !defined(FEAT_CMDL_INFO)
#define FEAT_CMDL_INFO 
#endif
#endif
#if defined(FEAT_NORMAL)
#define FEAT_BYTEOFF
#endif
#if defined(FEAT_NORMAL) || defined(UNIX)
#define FEAT_WILDIGN
#endif
#if defined(FEAT_NORMAL)
#define FEAT_WILDMENU
#endif
#if defined(FEAT_NORMAL)
#define FEAT_VIMINFO
#endif
#if defined(FEAT_NORMAL) || defined(PROTO)
#define FEAT_SYN_HL
#endif
#if defined(FEAT_BIG) && defined(FEAT_SYN_HL)
#define FEAT_CONCEAL
#endif
#if (defined(FEAT_NORMAL) || defined(PROTO)) && !defined(EBCDIC)
#define FEAT_SPELL
#endif
#if defined(HAVE_TGETENT)
#endif
#if !defined(NO_BUILTIN_TCAPS)
#if defined(FEAT_BIG)
#define ALL_BUILTIN_TCAPS
#else
#define SOME_BUILTIN_TCAPS 
#endif
#endif
#if defined(FEAT_NORMAL) || defined(PROTO)
#define FEAT_LISP
#endif
#if defined(FEAT_NORMAL) || defined(PROTO)
#define FEAT_CINDENT
#endif
#if defined(FEAT_NORMAL)
#define FEAT_SMARTINDENT
#endif
#if defined(FEAT_NORMAL) && !defined(FEAT_CRYPT) || defined(PROTO)
#define FEAT_CRYPT
#endif
#if defined(FEAT_EVAL)
#define FEAT_SESSION
#endif
#if defined(FEAT_NORMAL)
#define FEAT_MULTI_LANG
#endif
#if defined(HAVE_GETTEXT) && defined(FEAT_MULTI_LANG) && (defined(HAVE_LOCALE_H) || defined(X_LOCALE))
#define FEAT_GETTEXT
#endif
#if defined(FEAT_GUI_MSWIN) && !defined(FEAT_MBYTE_IME)
#endif
#if defined(FEAT_BIG) && defined(FEAT_GUI_HAIKU) && !defined(FEAT_MBYTE_IME)
#define FEAT_MBYTE_IME
#endif
#if (defined(HAVE_ICONV_H) && defined(HAVE_ICONV)) || defined(DYNAMIC_ICONV)
#define USE_ICONV
#endif
#if !defined(FEAT_XIM)
#endif
#if defined(FEAT_XIM) && defined(FEAT_GUI_GTK)
#define USE_XIM 1 
#endif
#if defined(FEAT_XIM)
#endif
#if !defined(FEAT_XFONTSET)
#if defined(HAVE_X11) && !defined(FEAT_GUI_GTK)
#define FEAT_XFONTSET
#else
#endif
#endif
#if defined(HAVE_DLOPEN) && defined(HAVE_DLSYM)
#define USE_DLOPEN
#endif
#if defined(FEAT_EVAL) && (defined(MSWIN) || ((defined(UNIX) || defined(VMS)) && (defined(USE_DLOPEN) || defined(HAVE_SHL_LOAD))))
#define FEAT_LIBCALL
#endif
#if defined(FEAT_NORMAL)
#define FEAT_MENU
#if defined(FEAT_GUI_MSWIN)
#define FEAT_TEAROFF
#endif
#endif
#if defined(FEAT_MENU) && !defined(ALWAYS_USE_GUI)
#define FEAT_TERM_POPUP_MENU
#endif
#if !defined(FEAT_SOUND) && defined(HAVE_CANBERRA)
#define FEAT_SOUND
#endif
#if defined(FEAT_SOUND) && defined(HAVE_CANBERRA)
#define FEAT_SOUND_CANBERRA
#endif
#if (defined(HAVE_XM_XPMP_H) && defined(FEAT_GUI_MOTIF)) || defined(HAVE_X11_XPM_H)
#define HAVE_XPM 1
#endif
#if defined(FEAT_NORMAL) && defined(FEAT_MENU) && (defined(FEAT_GUI_GTK) || defined(FEAT_GUI_MSWIN) || ((defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA)) && defined(HAVE_XPM)) || defined(FEAT_GUI_PHOTON) || defined(FEAT_GUI_HAIKU))
#define FEAT_TOOLBAR
#endif
#if defined(FEAT_TOOLBAR) && !defined(FEAT_MENU)
#define FEAT_MENU
#endif
#if defined(FEAT_GUI_GTK) && defined(USE_GTK3)
#define FEAT_GUI_DARKTHEME
#endif
#if defined(FEAT_NORMAL) && (defined(FEAT_GUI_GTK) || (defined(FEAT_GUI_MOTIF) && defined(HAVE_XM_NOTEBOOK_H)) || defined(FEAT_GUI_MAC) || defined(FEAT_GUI_HAIKU) || (defined(FEAT_GUI_MSWIN) && (!defined(_MSC_VER) || _MSC_VER > 1020)))
#define FEAT_GUI_TABLINE
#endif
#if defined(FEAT_NORMAL)
#define FEAT_BROWSE_CMD
#if defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_GTK) || defined(FEAT_GUI_HAIKU) || defined(FEAT_GUI_PHOTON) || defined(FEAT_GUI_MAC)
#define FEAT_BROWSE
#endif
#endif
#if (defined(FEAT_GUI_MSWIN) && !defined(VIMDLL)) || (defined(FEAT_GUI_MAC) && !defined(MACOS_X_DARWIN))
#define ALWAYS_USE_GUI
#endif
#if defined(FEAT_NORMAL)
#if ((defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_MOTIF)) && defined(HAVE_X11_XPM_H)) || defined(FEAT_GUI_GTK) || defined(FEAT_GUI_PHOTON) || defined(FEAT_GUI_HAIKU) || defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_MAC)
#define FEAT_CON_DIALOG
#define FEAT_GUI_DIALOG
#else
#define FEAT_CON_DIALOG
#endif
#endif
#if !defined(FEAT_GUI_DIALOG) && (defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_GTK) || defined(FEAT_GUI_MSWIN))
#define FEAT_GUI_DIALOG
#endif
#if defined(FEAT_GUI_DIALOG) && (defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_GTK) || defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_PHOTON) || defined(FEAT_GUI_MAC) || defined(FEAT_GUI_HAIKU))
#define FEAT_GUI_TEXTDIALOG
#if !defined(ALWAYS_USE_GUI)
#define FEAT_CON_DIALOG
#endif
#endif
#if (defined(FEAT_BIG) && defined(FEAT_SYN_HL)) && !defined(ALWAYS_USE_GUI)
#define FEAT_TERMGUICOLORS
#endif
#if defined(FEAT_GUI_MAC)
#define FEAT_CW_EDITOR
#endif
#if defined(FEAT_BIG)
#define FEAT_VARTABS
#endif
#if !defined(VMS)
#define FEAT_WRITEBACKUP
#endif
#if defined(FEAT_NORMAL) && ((defined(HAVE_GETTIMEOFDAY) && defined(HAVE_SYS_TIME_H)) || defined(MSWIN))
#define STARTUPTIME 1
#endif
#define SESSION_FILE "Session.vim"
#if !defined(SYS_OPTWIN_FILE)
#define SYS_OPTWIN_FILE "$VIMRUNTIME/optwin.vim"
#endif
#if !defined(RUNTIME_DIRNAME)
#define RUNTIME_DIRNAME "runtime"
#endif
#if (defined(FEAT_NORMAL) || defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA))
#define WANT_X11
#endif
#if defined(HAVE_X11) && defined(WANT_X11) && defined(HAVE_X11_SM_SMLIB_H)
#define USE_XSMP
#endif
#if defined(USE_XSMP_INTERACT) && !defined(USE_XSMP)
#undef USE_XSMP_INTERACT
#endif
#if defined(UNIX) || defined(VMS)
#define FEAT_MOUSE_XTERM
#if defined(FEAT_BIG)
#define FEAT_MOUSE_NET
#endif
#if defined(FEAT_BIG)
#define FEAT_MOUSE_DEC
#endif
#if defined(FEAT_BIG)
#define FEAT_MOUSE_URXVT
#endif
#endif
#if defined(MSWIN)
#define DOS_MOUSE
#endif
#if defined(__QNX__)
#define FEAT_MOUSE_PTERM
#endif
#if defined(FEAT_NORMAL) && defined(HAVE_GPM)
#define FEAT_MOUSE_GPM
#endif
#if defined(FEAT_NORMAL) && defined(HAVE_SYSMOUSE)
#define FEAT_SYSMOUSE
#endif
#if defined(FEAT_MOUSE_URXVT) && !defined(FEAT_MOUSE_XTERM)
#define FEAT_MOUSE_XTERM
#endif
#if defined(FEAT_CYGWIN_WIN32_CLIPBOARD)
#define FEAT_CLIPBOARD
#endif
#if defined(FEAT_GUI)
#if !defined(FEAT_CLIPBOARD)
#define FEAT_CLIPBOARD
#endif
#endif
#if defined(FEAT_NORMAL) && (defined(UNIX) || defined(VMS)) && defined(WANT_X11) && defined(HAVE_X11)
#define FEAT_XCLIPBOARD
#if !defined(FEAT_CLIPBOARD)
#define FEAT_CLIPBOARD
#endif
#endif
#if defined(FEAT_CLIPBOARD) && defined(FEAT_GUI_GTK)
#define FEAT_DND
#endif
#if defined(FEAT_GUI_MSWIN) && defined(FEAT_SMALL)
#define MSWIN_FIND_REPLACE 
#define MSWIN_FR_BUFSIZE 256
#endif
#if defined(FEAT_GUI_GTK) || defined(FEAT_GUI_MOTIF) || defined(MSWIN_FIND_REPLACE)
#define FIND_REPLACE_DIALOG 1
#endif
#if (defined(MSWIN) || defined(FEAT_XCLIPBOARD)) && defined(FEAT_EVAL)
#define FEAT_CLIENTSERVER
#endif
#if defined(FEAT_CLIENTSERVER) && !defined(FEAT_AUTOSERVERNAME)
#if defined(MSWIN)
#define FEAT_AUTOSERVERNAME
#else
#endif
#endif
#if defined(HAVE_TGETENT)
#define FEAT_TERMRESPONSE
#endif
#if defined(FEAT_NORMAL)
#if defined(MSWIN) && (!defined(FEAT_GUI_MSWIN) || defined(VIMDLL))
#define MCH_CURSOR_SHAPE
#endif
#if defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_GTK) || defined(FEAT_GUI_PHOTON)
#define FEAT_MOUSESHAPE
#endif
#endif
#if defined(FEAT_GUI) || defined(MCH_CURSOR_SHAPE) || defined(FEAT_MOUSESHAPE) || defined(FEAT_CONCEAL) || (defined(UNIX) && defined(FEAT_NORMAL))
#define CURSOR_SHAPE
#endif
#if defined(FEAT_MZSCHEME) && (defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_GTK) || defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_MAC))
#define MZSCHEME_GUI_THREADS
#endif
#if !defined(NO_ARP) && !defined(__amigaos4__)
#define FEAT_ARP
#endif
#if !defined(FEAT_EVAL) && defined(FEAT_NETBEANS_INTG)
#undef FEAT_NETBEANS_INTG
#endif
#if !defined(FEAT_EVAL) && defined(FEAT_JOB_CHANNEL)
#undef FEAT_JOB_CHANNEL
#endif
#if defined(FEAT_TERMINAL) && !defined(FEAT_JOB_CHANNEL)
#undef FEAT_TERMINAL
#endif
#if defined(FEAT_TERMINAL) && !defined(CURSOR_SHAPE)
#define CURSOR_SHAPE
#endif
#if defined(FEAT_TERMINAL) && !defined(FEAT_SYN_HL)
#define FEAT_SYN_HL
#endif
#if defined(FEAT_EVAL) && defined(FEAT_SYN_HL)
#define FEAT_PROP_POPUP
#endif
#if defined(FEAT_SYN_HL) && defined(FEAT_RELTIME)
#define SYN_TIME_LIMIT 1
#endif
#if defined(FEAT_BIG) || defined(FEAT_NETBEANS_INTG) || defined(FEAT_PROP_POPUP)
#define FEAT_SIGNS
#if ((defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA)) && defined(HAVE_X11_XPM_H)) || defined(FEAT_GUI_GTK) || (defined(MSWIN) && defined(FEAT_GUI))
#define FEAT_SIGN_ICONS
#endif
#endif
#if (defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_GTK) || defined(FEAT_GUI_MSWIN)) && ( ((defined(FEAT_TOOLBAR) || defined(FEAT_GUI_TABLINE)) && !defined(FEAT_GUI_GTK) && !defined(FEAT_GUI_MSWIN)) || defined(FEAT_NETBEANS_INTG) || defined(FEAT_EVAL))
#define FEAT_BEVAL_GUI
#if !defined(FEAT_XFONTSET) && !defined(FEAT_GUI_GTK) && !defined(FEAT_GUI_MSWIN)
#define FEAT_XFONTSET
#endif
#endif
#if defined(FEAT_BEVAL_GUI) && (defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA))
#define FEAT_BEVAL_TIP 
#endif
#if defined(FEAT_HUGE) && defined(FEAT_TIMERS) && (defined(UNIX) || defined(VMS) || (defined(MSWIN) && (!defined(FEAT_GUI_MSWIN) || defined(VIMDLL))))
#define FEAT_BEVAL_TERM
#endif
#if defined(FEAT_BEVAL_GUI) || defined(FEAT_BEVAL_TERM)
#define FEAT_BEVAL
#endif
#if defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA)
#define FEAT_GUI_X11
#endif
#if defined(FEAT_NETBEANS_INTG)
#if !defined(FEAT_MENU)
#define FEAT_MENU
#endif
#endif
#if 0
#define FEAT_FOOTER
#endif
#if defined(FEAT_NETBEANS_INTG) || defined(FEAT_BIG)
#define FEAT_AUTOCHDIR
#endif
#if defined(FEAT_NORMAL)
#define FEAT_PERSISTENT_UNDO
#endif
#if (defined(UNIX) && !defined(USE_SYSTEM)) || (defined(MSWIN) && defined(FEAT_GUI_MSWIN))
#define FEAT_FILTERPIPE
#endif
#if (!defined(FEAT_GUI) || defined(VIMDLL)) && defined(MSWIN)
#define FEAT_VTP
#endif
