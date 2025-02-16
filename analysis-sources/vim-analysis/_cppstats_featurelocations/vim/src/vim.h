







#if !defined(VIM__H)
#define VIM__H

#include "protodef.h"



#if defined(WIN32) || defined(_WIN32)
#define MSWIN
#endif

#if defined(MSWIN)
#include <io.h>
#endif



#if defined(HAVE_CONFIG_H)
#include "auto/config.h"
#define HAVE_PATHDEF






#if (VIM_SIZEOF_INT == 0)
#error configure did not run properly. Check auto/config.log.
#endif

#if (defined(__linux__) && !defined(__ANDROID__)) || defined(__CYGWIN__)




#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 700
#endif
#endif


#include <limits.h>





#if defined(__CYGWIN32__) && defined(HAVE_FCHDIR)
#undef HAVE_FCHDIR
#endif



#define UINT32_TYPEDEF uint32_t
#endif

#if !defined(UINT32_TYPEDEF)
#if defined(uint32_t) 
#define UINT32_TYPEDEF uint32_t
#else


#define UINT32_TYPEDEF unsigned int
#endif
#endif


#if defined(__TANDEM)
#if !defined(_TANDEM_SOURCE)
#define _TANDEM_SOURCE
#endif
#include <floss.h>
#define ROOT_UID 65535
#define OLDXAW
#if (_TANDEM_ARCH_ == 2 && __H_Series_RVU >= 621)
#define SA_ONSTACK_COMPATIBILITY
#endif
#else
#define ROOT_UID 0
#endif





#if defined(MACOS_X_DARWIN) && !defined(MACOS_X)
#define MACOS_X
#endif

#if defined(MACOS_X) && !defined(HAVE_CONFIG_H)
#define UNIX
#define FEAT_GUI_MAC
#endif

#if defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_GTK) || defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_HAIKU) || defined(FEAT_GUI_MAC) || defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_PHOTON)






#define FEAT_GUI_ENABLED 
#if !defined(FEAT_GUI) && !defined(NO_X11_INCLUDES)
#define FEAT_GUI
#endif
#endif


#if defined(FEAT_GUI)
#if defined(FEAT_DIRECTX)
#define FEAT_RENDER_OPTIONS
#endif
#endif





#if defined(MSWIN)
#define VIM_SIZEOF_INT 4
#endif

#if defined(AMIGA)

#if !defined(FEAT_GUI_GTK)
#if defined(__GNUC__)
#define VIM_SIZEOF_INT 4
#else
#define VIM_SIZEOF_INT 2
#endif
#endif
#endif
#if defined(MACOS_X) && !defined(HAVE_CONFIG_H)
#define VIM_SIZEOF_INT __SIZEOF_INT__
#endif

#if VIM_SIZEOF_INT < 4 && !defined(PROTO)
#error Vim only works with 32 bit int or larger
#endif





#include "feature.h"

#if defined(MACOS_X_DARWIN)
#if defined(FEAT_SMALL) && !defined(FEAT_CLIPBOARD)
#define FEAT_CLIPBOARD
#endif
#endif


#if defined(HAVE_X11) && defined(WANT_X11)
#define FEAT_X11
#endif

#if defined(NO_X11_INCLUDES)


#if defined(FEAT_X11)
#undef FEAT_X11
#endif
#if defined(FEAT_GUI_X11)
#undef FEAT_GUI_X11
#endif
#if defined(FEAT_XCLIPBOARD)
#undef FEAT_XCLIPBOARD
#endif
#if defined(FEAT_GUI_MOTIF)
#undef FEAT_GUI_MOTIF
#endif
#if defined(FEAT_GUI_ATHENA)
#undef FEAT_GUI_ATHENA
#endif
#if defined(FEAT_GUI_GTK)
#undef FEAT_GUI_GTK
#endif
#if defined(FEAT_BEVAL_TIP)
#undef FEAT_BEVAL_TIP
#endif
#if defined(FEAT_XIM)
#undef FEAT_XIM
#endif
#if defined(FEAT_CLIENTSERVER)
#undef FEAT_CLIENTSERVER
#endif
#endif


#if defined(MACOS_X_DARWIN)
#define MACOS_CONVERT
#endif


#if !defined(VIMPACKAGE)
#define VIMPACKAGE "vim"
#endif




#if defined(AZTEC_C)
#include <functions.h>
#endif

#if defined(SASC)
#include <clib/exec_protos.h>
#endif

#if defined(_DCC)
#include <clib/exec_protos.h>
#endif

#if defined(__BEOS__)
#include "os_beos.h"
#endif

#if defined(__HAIKU__)
#include "os_haiku.h"
#define __ARGS(x) x
#endif

#if (defined(UNIX) || defined(VMS)) && (!defined(MACOS_X) || defined(HAVE_CONFIG_H))

#include "os_unix.h" 
#endif



#if defined(HAVE_ATTRIBUTE_UNUSED) || defined(__MINGW32__)
#if !defined(UNUSED)
#define UNUSED __attribute__((unused))
#endif
#else
#define UNUSED
#endif


#if defined(__sun)
#define SUN_SYSTEM
#endif







#if !defined(__cplusplus) && defined(UNIX) && !defined(MACOS_X) 

#include "auto/osdef.h" 
#endif

#if defined(AMIGA)
#include "os_amiga.h"
#endif

#if defined(MSWIN)
#include "os_win32.h"
#endif

#if defined(__MINT__)
#include "os_mint.h"
#endif

#if defined(MACOS_X)
#include "os_mac.h"
#endif

#if defined(__QNX__)
#include "os_qnx.h"
#endif

#if defined(X_LOCALE)
#include <X11/Xlocale.h>
#else
#if defined(HAVE_LOCALE_H)
#include <locale.h>
#endif
#endif





#if !defined(MAXPATHL)
#if defined(MAXPATHLEN)
#define MAXPATHL MAXPATHLEN
#else
#define MAXPATHL 256
#endif
#endif
#if defined(BACKSLASH_IN_FILENAME)
#define PATH_ESC_CHARS ((char_u *)" \t\n*?[{`%#'\"|!<")
#else
#if defined(VMS)

#define PATH_ESC_CHARS ((char_u *)" \t\n*?{`\\%#'\"|!")
#define SHELL_ESC_CHARS ((char_u *)" \t\n*?{`\\%#'|!()&")
#else
#define PATH_ESC_CHARS ((char_u *)" \t\n*?[{`$\\%#'\"|!<")
#define SHELL_ESC_CHARS ((char_u *)" \t\n*?[{`$\\%#'\"|!<>();&")
#endif
#endif


#define NUMBUFLEN 65


#define STR2NR_BIN 0x01
#define STR2NR_OCT 0x02
#define STR2NR_HEX 0x04
#define STR2NR_ALL (STR2NR_BIN + STR2NR_OCT + STR2NR_HEX)
#define STR2NR_NO_OCT (STR2NR_BIN + STR2NR_HEX)

#define STR2NR_FORCE 0x80 

#define STR2NR_QUOTE 0x10 





typedef unsigned char char_u;
typedef unsigned short short_u;
typedef unsigned int int_u;




#if defined(_WIN64)
typedef unsigned __int64 long_u;
typedef __int64 long_i;
#define SCANF_HEX_LONG_U "%Ix"
#define SCANF_DECIMAL_LONG_U "%Iu"
#define PRINTF_HEX_LONG_U "0x%Ix"
#else




#if !defined(_MSC_VER) || (_MSC_VER < 1300)
#define __w64
#endif
typedef unsigned long __w64 long_u;
typedef long __w64 long_i;
#define SCANF_HEX_LONG_U "%lx"
#define SCANF_DECIMAL_LONG_U "%lu"
#define PRINTF_HEX_LONG_U "0x%lx"
#endif
#define PRINTF_DECIMAL_LONG_U SCANF_DECIMAL_LONG_U







#if defined(SIZEOF_OFF_T) && (SIZEOF_OFF_T > VIM_SIZEOF_LONG)
#define LONG_LONG_OFF_T
#endif







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
#if defined(PROTO)
typedef long off_T;
#else
typedef off_t off_T;
#endif
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




typedef char_u schar_T;
typedef unsigned short sattr_T;
#define MAX_TYPENR 65535





typedef unsigned int u8char_T; 

#if !defined(UNIX)
#include <stdio.h>
#include <ctype.h>
#endif

#include "ascii.h"
#include "keymap.h"
#include "term.h"
#include "macros.h"

#if defined(LATTICE)
#include <sys/types.h>
#include <sys/stat.h>
#endif
#if defined(_DCC)
#include <sys/stat.h>
#endif
#if defined(MSWIN)
#include <sys/stat.h>
#endif

#if defined(HAVE_ERRNO_H) || defined(MSWIN)
#include <errno.h>
#endif





#if !defined(UNIX)


#if defined(HAVE_STRING_H)
#include <string.h>
#endif
#if defined(HAVE_STRINGS_H) && !defined(NO_STRINGS_WITH_STRING_H)
#include <strings.h>
#endif
#if defined(HAVE_STAT_H)
#include <stat.h>
#endif
#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif
#endif 

#include <assert.h>

#if defined(HAVE_STDINT_H)
#include <stdint.h>
#endif
#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#endif
#if defined(HAVE_WCTYPE_H)
#include <wctype.h>
#endif
#include <stdarg.h>


#include <stddef.h>

#if defined(HAVE_SYS_SELECT_H) && (!defined(HAVE_SYS_TIME_H) || defined(SYS_SELECT_WITH_SYS_TIME))

#include <sys/select.h>
#endif

#if !defined(HAVE_SELECT)
#if defined(HAVE_SYS_POLL_H)
#include <sys/poll.h>
#elif defined(MSWIN)
#define HAVE_SELECT
#else
#if defined(HAVE_POLL_H)
#include <poll.h>
#endif
#endif
#endif






#if defined(DYNAMIC_IME)
#if !defined(FEAT_MBYTE_IME)
#define FEAT_MBYTE_IME
#endif
#endif




#if defined(DYNAMIC_GETTEXT)
#if !defined(FEAT_GETTEXT)
#define FEAT_GETTEXT
#endif

extern char *(*dyn_libintl_gettext)(const char *msgid);
extern char *(*dyn_libintl_ngettext)(const char *msgid, const char *msgid_plural, unsigned long n);
extern char *(*dyn_libintl_bindtextdomain)(const char *domainname, const char *dirname);
extern char *(*dyn_libintl_bind_textdomain_codeset)(const char *domainname, const char *codeset);
extern char *(*dyn_libintl_textdomain)(const char *domainname);
extern int (*dyn_libintl_wputenv)(const wchar_t *envstring);
#endif










#if defined(FEAT_GETTEXT)
#if defined(DYNAMIC_GETTEXT)
#define _(x) (*dyn_libintl_gettext)((char *)(x))
#define NGETTEXT(x, xs, n) (*dyn_libintl_ngettext)((char *)(x), (char *)(xs), (n))
#define N_(x) x
#define bindtextdomain(domain, dir) (*dyn_libintl_bindtextdomain)((domain), (dir))
#define bind_textdomain_codeset(domain, codeset) (*dyn_libintl_bind_textdomain_codeset)((domain), (codeset))
#if !defined(HAVE_BIND_TEXTDOMAIN_CODESET)
#define HAVE_BIND_TEXTDOMAIN_CODESET 1
#endif
#define textdomain(domain) (*dyn_libintl_textdomain)(domain)
#define libintl_wputenv(envstring) (*dyn_libintl_wputenv)(envstring)
#else
#include <libintl.h>
#define _(x) gettext((char *)(x))
#define NGETTEXT(x, xs, n) ngettext((x), (xs), (n))
#if defined(gettext_noop)
#define N_(x) gettext_noop(x)
#else
#define N_(x) x
#endif
#endif
#else
#define _(x) ((char *)(x))
#define NGETTEXT(x, xs, n) (((n) == 1) ? (char *)(x) : (char *)(xs))
#define N_(x) x
#if defined(bindtextdomain)
#undef bindtextdomain
#endif
#define bindtextdomain(x, y) 
#if defined(bind_textdomain_codeset)
#undef bind_textdomain_codeset
#endif
#define bind_textdomain_codeset(x, y) 
#if defined(textdomain)
#undef textdomain
#endif
#define textdomain(x) 
#endif





#define VALID_NO_UPDATE 5 

#define VALID 10 

#define INVERTED 20 
#define INVERTED_ALL 25 
#define REDRAW_TOP 30 
#define SOME_VALID 35 
#define NOT_VALID 40 
#define CLEAR 50 


#define SLF_RIGHTLEFT 1
#define SLF_POPUP 2

#define MB_FILLER_CHAR '<' 





















#define VALID_WROW 0x01 
#define VALID_WCOL 0x02 
#define VALID_VIRTCOL 0x04 
#define VALID_CHEIGHT 0x08 
#define VALID_CROW 0x10 
#define VALID_BOTLINE 0x20 
#define VALID_BOTLINE_AP 0x40 
#define VALID_TOPLINE 0x80 


#define POPF_IS_POPUP 0x01 
#define POPF_HIDDEN 0x02 
#define POPF_CURSORLINE 0x04 
#define POPF_ON_CMDLINE 0x08 
#define POPF_DRAG 0x10 
#define POPF_RESIZE 0x20 
#define POPF_MAPPING 0x40 
#define POPF_INFO 0x80 
#define POPF_INFO_MENU 0x100 
#define POPF_POSINVERT 0x200 


#define POPUP_HANDLED_1 0x01 
#define POPUP_HANDLED_2 0x02 
#define POPUP_HANDLED_3 0x04 
#define POPUP_HANDLED_4 0x08 
#define POPUP_HANDLED_5 0x10 





#define HL_NORMAL 0x00
#define HL_INVERSE 0x01
#define HL_BOLD 0x02
#define HL_ITALIC 0x04
#define HL_UNDERLINE 0x08
#define HL_UNDERCURL 0x10
#define HL_STANDOUT 0x20
#define HL_NOCOMBINE 0x40
#define HL_STRIKETHROUGH 0x80
#define HL_ALL 0xff


#define MSG_HIST 0x1000









#define NORMAL 0x01 
#define VISUAL 0x02 
#define OP_PENDING 0x04 

#define CMDLINE 0x08 
#define INSERT 0x10 
#define LANGMAP 0x20 


#define REPLACE_FLAG 0x40 
#define REPLACE (REPLACE_FLAG + INSERT)
#define VREPLACE_FLAG 0x80 
#define VREPLACE (REPLACE_FLAG + VREPLACE_FLAG + INSERT)
#define LREPLACE (REPLACE_FLAG + LANGMAP)

#define NORMAL_BUSY (0x100 + NORMAL) 
#define HITRETURN (0x200 + NORMAL) 
#define ASKMORE 0x300 
#define SETWSIZE 0x400 
#define ABBREV 0x500 
#define EXTERNCMD 0x600 
#define SHOWMATCH (0x700 + INSERT) 
#define CONFIRM 0x800 
#define SELECTMODE 0x1000 
#define TERMINAL 0x2000 
#define MODE_ALL 0xffff


#define MAP_ALL_MODES (0x3f | SELECTMODE | TERMINAL)


#define FORWARD 1
#define BACKWARD (-1)
#define FORWARD_FILE 3
#define BACKWARD_FILE (-3)


#if !(defined(OK) && (OK == 1))

#define OK 1
#endif
#define FAIL 0
#define NOTDONE 2 


#define BF_RECOVERED 0x01 
#define BF_CHECK_RO 0x02 


#define BF_NEVERLOADED 0x04 

#define BF_NOTEDITED 0x08 


#define BF_NEW 0x10 
#define BF_NEW_W 0x20 
#define BF_READERR 0x40 
#define BF_DUMMY 0x80 
#define BF_PRESERVED 0x100 
#define BF_SYN_SET 0x200 
#define BF_NO_SEA 0x400 


#define BF_WRITE_MASK (BF_NOTEDITED + BF_NEW + BF_READERR)




#define EXPAND_UNSUCCESSFUL (-2)
#define EXPAND_OK (-1)
#define EXPAND_NOTHING 0
#define EXPAND_COMMANDS 1
#define EXPAND_FILES 2
#define EXPAND_DIRECTORIES 3
#define EXPAND_SETTINGS 4
#define EXPAND_BOOL_SETTINGS 5
#define EXPAND_TAGS 6
#define EXPAND_OLD_SETTING 7
#define EXPAND_HELP 8
#define EXPAND_BUFFERS 9
#define EXPAND_EVENTS 10
#define EXPAND_MENUS 11
#define EXPAND_SYNTAX 12
#define EXPAND_HIGHLIGHT 13
#define EXPAND_AUGROUP 14
#define EXPAND_USER_VARS 15
#define EXPAND_MAPPINGS 16
#define EXPAND_TAGS_LISTFILES 17
#define EXPAND_FUNCTIONS 18
#define EXPAND_USER_FUNC 19
#define EXPAND_EXPRESSION 20
#define EXPAND_MENUNAMES 21
#define EXPAND_USER_COMMANDS 22
#define EXPAND_USER_CMD_FLAGS 23
#define EXPAND_USER_NARGS 24
#define EXPAND_USER_COMPLETE 25
#define EXPAND_ENV_VARS 26
#define EXPAND_LANGUAGE 27
#define EXPAND_COLORS 28
#define EXPAND_COMPILER 29
#define EXPAND_USER_DEFINED 30
#define EXPAND_USER_LIST 31
#define EXPAND_SHELLCMD 32
#define EXPAND_CSCOPE 33
#define EXPAND_SIGN 34
#define EXPAND_PROFILE 35
#define EXPAND_BEHAVE 36
#define EXPAND_FILETYPE 37
#define EXPAND_FILES_IN_PATH 38
#define EXPAND_OWNSYNTAX 39
#define EXPAND_LOCALES 40
#define EXPAND_HISTORY 41
#define EXPAND_USER 42
#define EXPAND_SYNTIME 43
#define EXPAND_USER_ADDR_TYPE 44
#define EXPAND_PACKADD 45
#define EXPAND_MESSAGES 46
#define EXPAND_MAPCLEAR 47
#define EXPAND_ARGLIST 48
#define EXPAND_DIFF_BUFFERS 49


#define EXMODE_NORMAL 1
#define EXMODE_VIM 2


#define WILD_FREE 1
#define WILD_EXPAND_FREE 2
#define WILD_EXPAND_KEEP 3
#define WILD_NEXT 4
#define WILD_PREV 5
#define WILD_ALL 6
#define WILD_LONGEST 7
#define WILD_ALL_KEEP 8

#define WILD_LIST_NOTFOUND 0x01
#define WILD_HOME_REPLACE 0x02
#define WILD_USE_NL 0x04
#define WILD_NO_BEEP 0x08
#define WILD_ADD_SLASH 0x10
#define WILD_KEEP_ALL 0x20
#define WILD_SILENT 0x40
#define WILD_ESCAPE 0x80
#define WILD_ICASE 0x100
#define WILD_ALLLINKS 0x200
#define WILD_IGNORE_COMPLETESLASH 0x400
#define WILD_NOERROR 0x800 
#define WILD_BUFLASTUSED 0x1000
#define BUF_DIFF_FILTER 0x2000


#define EW_DIR 0x01 
#define EW_FILE 0x02 
#define EW_NOTFOUND 0x04 
#define EW_ADDSLASH 0x08 
#define EW_KEEPALL 0x10 
#define EW_SILENT 0x20 
#define EW_EXEC 0x40 
#define EW_PATH 0x80 
#define EW_ICASE 0x100 
#define EW_NOERROR 0x200 
#define EW_NOTWILD 0x400 
#define EW_KEEPDOLLAR 0x800 


#define EW_ALLLINKS 0x1000 
#define EW_SHELLCMD 0x2000 

#define EW_DODOT 0x4000 
#define EW_EMPTYOK 0x8000 
#define EW_NOTENV 0x10000 


#define FINDFILE_FILE 0 
#define FINDFILE_DIR 1 
#define FINDFILE_BOTH 2 

#define W_ENDCOL(wp) (wp->w_wincol + wp->w_width)
#if defined(FEAT_MENU)
#define W_WINROW(wp) (wp->w_winrow + wp->w_winbar_height)
#else
#define W_WINROW(wp) (wp->w_winrow)
#endif


#define FIND_ANY 1
#define FIND_DEFINE 2
#define CHECK_PATH 3

#define ACTION_SHOW 1
#define ACTION_GOTO 2
#define ACTION_SPLIT 3
#define ACTION_SHOW_ALL 4
#define ACTION_EXPAND 5

#if defined(FEAT_SYN_HL)
#define SST_MIN_ENTRIES 150 
#define SST_MAX_ENTRIES 1000 
#define SST_FIX_STATES 7 
#define SST_DIST 16 
#define SST_INVALID (synstate_T *)-1 

#define HL_CONTAINED 0x01 
#define HL_TRANSP 0x02 
#define HL_ONELINE 0x04 
#define HL_HAS_EOL 0x08 
#define HL_SYNC_HERE 0x10 
#define HL_SYNC_THERE 0x20 
#define HL_MATCH 0x40 
#define HL_SKIPNL 0x80 
#define HL_SKIPWHITE 0x100 
#define HL_SKIPEMPTY 0x200 
#define HL_KEEPEND 0x400 
#define HL_EXCLUDENL 0x800 
#define HL_DISPLAY 0x1000 
#define HL_FOLD 0x2000 
#define HL_EXTEND 0x4000 
#define HL_MATCHCONT 0x8000 
#define HL_TRANS_CONT 0x10000 
#define HL_CONCEAL 0x20000 
#define HL_CONCEALENDS 0x40000 
#endif


#define SEARCH_REV 0x01 
#define SEARCH_ECHO 0x02 
#define SEARCH_MSG 0x0c 
#define SEARCH_NFMSG 0x08 
#define SEARCH_OPT 0x10 
#define SEARCH_HIS 0x20 
#define SEARCH_END 0x40 
#define SEARCH_NOOF 0x80 
#define SEARCH_START 0x100 
#define SEARCH_MARK 0x200 
#define SEARCH_KEEP 0x400 
#define SEARCH_PEEK 0x800 
#define SEARCH_COL 0x1000 


#define FIND_IDENT 1 
#define FIND_STRING 2 
#define FIND_EVAL 4 
#define FIND_NOERROR 8 


#define FNAME_MESS 1 
#define FNAME_EXP 2 
#define FNAME_HYP 4 
#define FNAME_INCL 8 
#define FNAME_REL 16 

#define FNAME_UNESC 32 


#define GETF_SETMARK 0x01 
#define GETF_ALT 0x02 
#define GETF_SWITCH 0x04 


#define GETFILE_ERROR 1 
#define GETFILE_NOT_WRITTEN 2 
#define GETFILE_SAME_FILE 0 
#define GETFILE_OPEN_OTHER -1 
#define GETFILE_UNUSED 8
#define GETFILE_SUCCESS(x) ((x) <= 0)


#define BLN_CURBUF 1 
#define BLN_LISTED 2 
#define BLN_DUMMY 4 
#define BLN_NEW 8 
#define BLN_NOOPT 16 
#define BLN_DUMMY_OK 32 
#define BLN_REUSE 64 


#define KEY_OPEN_FORW 0x101
#define KEY_OPEN_BACK 0x102
#define KEY_COMPLETE 0x103 



#define REMAP_YES 0 
#define REMAP_NONE -1 
#define REMAP_SCRIPT -2 
#define REMAP_SKIP -3 


#define SHELL_FILTER 1 
#define SHELL_EXPAND 2 
#define SHELL_COOKED 4 
#define SHELL_DOOUT 8 
#define SHELL_SILENT 16 
#define SHELL_READ 32 
#define SHELL_WRITE 64 


#define NODE_NORMAL 0 
#define NODE_WRITABLE 1 

#define NODE_OTHER 2 


#define READ_NEW 0x01 
#define READ_FILTER 0x02 
#define READ_STDIN 0x04 
#define READ_BUFFER 0x08 
#define READ_DUMMY 0x10 
#define READ_KEEP_UNDO 0x20 
#define READ_FIFO 0x40 


#define INDENT_SET 1 
#define INDENT_INC 2 
#define INDENT_DEC 3 


#define FM_BACKWARD 0x01 
#define FM_FORWARD 0x02 
#define FM_BLOCKSTOP 0x04 
#define FM_SKIPCOMM 0x08 


#define DOBUF_GOTO 0 
#define DOBUF_SPLIT 1 
#define DOBUF_UNLOAD 2 
#define DOBUF_DEL 3 
#define DOBUF_WIPE 4 
#define DOBUF_WIPE_REUSE 5 


#define DOBUF_CURRENT 0 
#define DOBUF_FIRST 1 
#define DOBUF_LAST 2 
#define DOBUF_MOD 3 



#define RE_SEARCH 0 
#define RE_SUBST 1 
#define RE_BOTH 2 
#define RE_LAST 2 


#define RE_MAGIC 1 
#define RE_STRING 2 
#define RE_STRICT 4 
#define RE_AUTO 8 

#if defined(FEAT_SYN_HL)

#define REX_SET 1 
#define REX_USE 2 
#define REX_ALL (REX_SET | REX_USE)
#endif



#define FPC_SAME 1 
#define FPC_DIFF 2 
#define FPC_NOTX 4 
#define FPC_DIFFX 6 
#define FPC_SAMEX 7 


#define ECMD_HIDE 0x01 
#define ECMD_SET_HELP 0x02 

#define ECMD_OLDBUF 0x04 
#define ECMD_FORCEIT 0x08 
#define ECMD_ADDBUF 0x10 


#define ECMD_LASTL (linenr_T)0 
#define ECMD_LAST (linenr_T)-1 
#define ECMD_ONE (linenr_T)1 


#define DOCMD_VERBOSE 0x01 
#define DOCMD_NOWAIT 0x02 
#define DOCMD_REPEAT 0x04 
#define DOCMD_KEYTYPED 0x08 
#define DOCMD_EXCRESET 0x10 
#define DOCMD_KEEPLINE 0x20 


#define BL_WHITE 1 
#define BL_SOL 2 
#define BL_FIX 4 


#define MFS_ALL 1 
#define MFS_STOP 2 
#define MFS_FLUSH 4 
#define MFS_ZERO 8 


#define BCO_ENTER 1 
#define BCO_ALWAYS 2 
#define BCO_NOHELP 4 


#define PUT_FIXINDENT 1 
#define PUT_CURSEND 2 
#define PUT_CURSLINE 4 
#define PUT_LINE 8 
#define PUT_LINE_SPLIT 16 
#define PUT_LINE_FORWARD 32 


#define SIN_CHANGED 1 
#define SIN_INSERT 2 
#define SIN_UNDO 4 


#define INSCHAR_FORMAT 1 
#define INSCHAR_DO_COM 2 
#define INSCHAR_CTRLV 4 
#define INSCHAR_NO_FEX 8 
#define INSCHAR_COM_LIST 16 


#define OPENLINE_DELSPACES 1 
#define OPENLINE_DO_COM 2 
#define OPENLINE_KEEPTRAIL 4 
#define OPENLINE_MARKFIX 8 
#define OPENLINE_COM_LIST 16 


#define HIST_CMD 0 
#define HIST_SEARCH 1 
#define HIST_EXPR 2 
#define HIST_INPUT 3 
#define HIST_DEBUG 4 
#define HIST_COUNT 5 


#define BARTYPE_VERSION 1
#define BARTYPE_HISTORY 2
#define BARTYPE_REGISTER 3
#define BARTYPE_MARK 4

#define VIMINFO_VERSION 4
#define VIMINFO_VERSION_WITH_HISTORY 2
#define VIMINFO_VERSION_WITH_REGISTERS 3
#define VIMINFO_VERSION_WITH_MARKS 4




#define DT_TAG 1 
#define DT_POP 2 
#define DT_NEXT 3 
#define DT_PREV 4 
#define DT_FIRST 5 
#define DT_LAST 6 
#define DT_SELECT 7 
#define DT_HELP 8 
#define DT_JUMP 9 
#define DT_CSCOPE 10 
#define DT_LTAG 11 
#define DT_FREE 99 




#define TAG_HELP 1 
#define TAG_NAMES 2 
#define TAG_REGEXP 4 
#define TAG_NOIC 8 
#if defined(FEAT_CSCOPE)
#define TAG_CSCOPE 16 
#endif
#define TAG_VERBOSE 32 
#define TAG_INS_COMP 64 
#define TAG_KEEP_LANG 128 
#define TAG_NO_TAGFUNC 256 

#define TAG_MANY 300 





#define VIM_GENERIC 0
#define VIM_ERROR 1
#define VIM_WARNING 2
#define VIM_INFO 3
#define VIM_QUESTION 4
#define VIM_LAST_TYPE 4 




#define VIM_YES 2
#define VIM_NO 3
#define VIM_CANCEL 4
#define VIM_ALL 5
#define VIM_DISCARDALL 6




#define WSP_ROOM 1 
#define WSP_VERT 2 
#define WSP_TOP 4 
#define WSP_BOT 8 
#define WSP_HELP 16 
#define WSP_BELOW 32 
#define WSP_ABOVE 64 
#define WSP_NEWLOC 128 




#define RESIZE_VERT 1 
#define RESIZE_HOR 2 
#define RESIZE_BOTH 15 




#define CCGD_AW 1 
#define CCGD_MULTWIN 2 
#define CCGD_FORCEIT 4 
#define CCGD_ALLBUF 8 
#define CCGD_EXCMD 16 






#define OPT_FREE 0x01 
#define OPT_GLOBAL 0x02 
#define OPT_LOCAL 0x04 
#define OPT_MODELINE 0x08 
#define OPT_WINONLY 0x10 
#define OPT_NOWIN 0x20 
#define OPT_ONECOLUMN 0x40 


#define DLG_BUTTON_SEP '\n'
#define DLG_HOTKEY_CHAR '&'


#define NO_SCREEN 2 
#define NO_BUFFERS 1 



#define SEA_NONE 0 
#define SEA_DIALOG 1 
#define SEA_QUIT 2 
#define SEA_RECOVER 3 







#define MIN_SWAP_PAGE_SIZE 1048
#define MAX_SWAP_PAGE_SIZE 50000


#define SID_MODELINE -1 
#define SID_CMDARG -2 
#define SID_CARG -3 
#define SID_ENV -4 
#define SID_ERROR -5 
#define SID_NONE -6 




enum auto_event
{
EVENT_BUFADD = 0, 
EVENT_BUFDELETE, 
EVENT_BUFENTER, 
EVENT_BUFFILEPOST, 
EVENT_BUFFILEPRE, 
EVENT_BUFHIDDEN, 
EVENT_BUFLEAVE, 
EVENT_BUFNEW, 
EVENT_BUFNEWFILE, 
EVENT_BUFREADCMD, 
EVENT_BUFREADPOST, 
EVENT_BUFREADPRE, 
EVENT_BUFUNLOAD, 
EVENT_BUFWINENTER, 
EVENT_BUFWINLEAVE, 
EVENT_BUFWIPEOUT, 
EVENT_BUFWRITECMD, 
EVENT_BUFWRITEPOST, 
EVENT_BUFWRITEPRE, 
EVENT_CMDLINECHANGED, 
EVENT_CMDLINEENTER, 
EVENT_CMDLINELEAVE, 
EVENT_CMDUNDEFINED, 
EVENT_CMDWINENTER, 
EVENT_CMDWINLEAVE, 
EVENT_COLORSCHEME, 
EVENT_COLORSCHEMEPRE, 
EVENT_COMPLETECHANGED, 
EVENT_COMPLETEDONE, 
EVENT_COMPLETEDONEPRE, 
EVENT_CURSORHOLD, 
EVENT_CURSORHOLDI, 
EVENT_CURSORMOVED, 
EVENT_CURSORMOVEDI, 
EVENT_DIFFUPDATED, 
EVENT_DIRCHANGED, 
EVENT_ENCODINGCHANGED, 
EVENT_EXITPRE, 
EVENT_FILEAPPENDCMD, 
EVENT_FILEAPPENDPOST, 
EVENT_FILEAPPENDPRE, 
EVENT_FILECHANGEDRO, 
EVENT_FILECHANGEDSHELL, 
EVENT_FILECHANGEDSHELLPOST, 
EVENT_FILEREADCMD, 
EVENT_FILEREADPOST, 
EVENT_FILEREADPRE, 
EVENT_FILETYPE, 
EVENT_FILEWRITECMD, 
EVENT_FILEWRITEPOST, 
EVENT_FILEWRITEPRE, 
EVENT_FILTERREADPOST, 
EVENT_FILTERREADPRE, 
EVENT_FILTERWRITEPOST, 
EVENT_FILTERWRITEPRE, 
EVENT_FOCUSGAINED, 
EVENT_FOCUSLOST, 
EVENT_FUNCUNDEFINED, 
EVENT_GUIENTER, 
EVENT_GUIFAILED, 
EVENT_INSERTCHANGE, 
EVENT_INSERTCHARPRE, 
EVENT_INSERTENTER, 
EVENT_INSERTLEAVE, 
EVENT_MENUPOPUP, 
EVENT_OPTIONSET, 
EVENT_QUICKFIXCMDPOST, 
EVENT_QUICKFIXCMDPRE, 
EVENT_QUITPRE, 
EVENT_REMOTEREPLY, 
EVENT_SAFESTATE, 
EVENT_SAFESTATEAGAIN, 
EVENT_SESSIONLOADPOST, 
EVENT_SHELLCMDPOST, 
EVENT_SHELLFILTERPOST, 
EVENT_SOURCECMD, 
EVENT_SOURCEPRE, 
EVENT_SOURCEPOST, 
EVENT_SPELLFILEMISSING, 
EVENT_STDINREADPOST, 
EVENT_STDINREADPRE, 
EVENT_SWAPEXISTS, 
EVENT_SYNTAX, 
EVENT_TABCLOSED, 
EVENT_TABENTER, 
EVENT_TABLEAVE, 
EVENT_TABNEW, 
EVENT_TERMCHANGED, 
EVENT_TERMINALOPEN, 
EVENT_TERMINALWINOPEN, 
EVENT_TERMRESPONSE, 
EVENT_TEXTCHANGED, 
EVENT_TEXTCHANGEDI, 
EVENT_TEXTCHANGEDP, 
EVENT_TEXTYANKPOST, 
EVENT_USER, 
EVENT_VIMENTER, 
EVENT_VIMLEAVE, 
EVENT_VIMLEAVEPRE, 
EVENT_VIMRESIZED, 
EVENT_WINENTER, 
EVENT_WINLEAVE, 
EVENT_WINNEW, 

NUM_EVENTS 
};

typedef enum auto_event event_T;






typedef enum
{
HLF_8 = 0 

, HLF_EOB 
, HLF_AT 

, HLF_D 
, HLF_E 
, HLF_H 
, HLF_I 
, HLF_L 
, HLF_M 
, HLF_CM 
, HLF_N 
, HLF_LNA 
, HLF_LNB 
, HLF_CLN 
, HLF_R 
, HLF_S 
, HLF_SNC 
, HLF_C 
, HLF_T 
, HLF_V 
, HLF_VNC 
, HLF_W 
, HLF_WM 
, HLF_FL 
, HLF_FC 
, HLF_ADD 
, HLF_CHD 
, HLF_DED 
, HLF_TXD 
, HLF_CONCEAL 
, HLF_SC 
, HLF_SPB 
, HLF_SPC 
, HLF_SPR 
, HLF_SPL 
, HLF_PNI 
, HLF_PSI 
, HLF_PSB 
, HLF_PST 
, HLF_TP 
, HLF_TPS 
, HLF_TPF 
, HLF_CUC 
, HLF_CUL 
, HLF_MC 
, HLF_QFL 
, HLF_ST 
, HLF_STNC 
, HLF_COUNT 
} hlf_T;



#define HL_FLAGS {'8', '~', '@', 'd', 'e', 'h', 'i', 'l', 'm', 'M', 'n', 'a', 'b', 'N', 'r', 's', 'S', 'c', 't', 'v', 'V', 'w', 'W', 'f', 'F', 'A', 'C', 'D', 'T', '-', '>', 'B', 'P', 'R', 'L', '+', '=', 'x', 'X', '*', '#', '_', '!', '.', 'o', 'q', 'z', 'Z'}









#if !defined(TRUE)
#define FALSE 0 
#define TRUE 1
#endif

#define MAYBE 2 

#if !defined(UINT32_T)
typedef UINT32_TYPEDEF UINT32_T;
#endif




#define OP_NOP 0 
#define OP_DELETE 1 
#define OP_YANK 2 
#define OP_CHANGE 3 
#define OP_LSHIFT 4 
#define OP_RSHIFT 5 
#define OP_FILTER 6 
#define OP_TILDE 7 
#define OP_INDENT 8 
#define OP_FORMAT 9 
#define OP_COLON 10 
#define OP_UPPER 11 
#define OP_LOWER 12 
#define OP_JOIN 13 
#define OP_JOIN_NS 14 
#define OP_ROT13 15 
#define OP_REPLACE 16 
#define OP_INSERT 17 
#define OP_APPEND 18 
#define OP_FOLD 19 
#define OP_FOLDOPEN 20 
#define OP_FOLDOPENREC 21 
#define OP_FOLDCLOSE 22 
#define OP_FOLDCLOSEREC 23 
#define OP_FOLDDEL 24 
#define OP_FOLDDELREC 25 
#define OP_FORMAT2 26 
#define OP_FUNCTION 27 
#define OP_NR_ADD 28 

#define OP_NR_SUB 29 





#define MCHAR 0 
#define MLINE 1 
#define MBLOCK 2 

#define MAUTO 0xff 




#define MIN_COLUMNS 12 
#define MIN_LINES 2 
#define STATUS_HEIGHT 1 
#if defined(FEAT_MENU)
#define WINBAR_HEIGHT(wp) (wp)->w_winbar_height
#define VISIBLE_HEIGHT(wp) ((wp)->w_height + (wp)->w_winbar_height)
#else
#define WINBAR_HEIGHT(wp) 0
#define VISIBLE_HEIGHT(wp) (wp)->w_height
#endif
#define QF_WINHEIGHT 10 




#if !defined(CMDBUFFSIZE)
#define CMDBUFFSIZE 256 
#endif

#define LSIZE 512 

#define IOSIZE (1024+1) 

#define DIALOG_MSG_SIZE 1000 

#define MSG_BUF_LEN 480 
#define MSG_BUF_CLEN (MSG_BUF_LEN / 6) 


#define FOLD_TEXT_LEN 51 





#define TBUFSZ 2048 





#define MAXMAPLEN 50


#define UNDO_HASH_SIZE 32

#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#if defined(BINARY_FILE_IO)
#define WRITEBIN "wb" 
#define READBIN "rb"
#define APPENDBIN "ab"
#else
#define WRITEBIN "w"
#define READBIN "r"
#define APPENDBIN "a"
#endif





#if defined(__CYGWIN32__)
#define O_EXTRA O_BINARY
#else
#define O_EXTRA 0
#endif

#if !defined(O_NOFOLLOW)
#define O_NOFOLLOW 0
#endif

#if !defined(W_OK)
#define W_OK 2 
#endif
#if !defined(R_OK)
#define R_OK 4 
#endif



#define ALLOC_ONE(type) (type *)alloc(sizeof(type))
#define ALLOC_MULT(type, count) (type *)alloc(sizeof(type) * (count))
#define ALLOC_CLEAR_ONE(type) (type *)alloc_clear(sizeof(type))
#define ALLOC_CLEAR_MULT(type, count) (type *)alloc_clear(sizeof(type) * (count))
#define LALLOC_CLEAR_ONE(type) (type *)lalloc_clear(sizeof(type), FALSE)
#define LALLOC_CLEAR_MULT(type, count) (type *)lalloc_clear(sizeof(type) * (count), FALSE)
#define LALLOC_MULT(type, count) (type *)lalloc(sizeof(type) * (count), FALSE)





#define STRLEN(s) strlen((char *)(s))
#define STRCPY(d, s) strcpy((char *)(d), (char *)(s))
#define STRNCPY(d, s, n) strncpy((char *)(d), (char *)(s), (size_t)(n))
#define STRCMP(d, s) strcmp((char *)(d), (char *)(s))
#define STRNCMP(d, s, n) strncmp((char *)(d), (char *)(s), (size_t)(n))
#if defined(HAVE_STRCASECMP)
#define STRICMP(d, s) strcasecmp((char *)(d), (char *)(s))
#else
#if defined(HAVE_STRICMP)
#define STRICMP(d, s) stricmp((char *)(d), (char *)(s))
#else
#define STRICMP(d, s) vim_stricmp((char *)(d), (char *)(s))
#endif
#endif


#define STRMOVE(d, s) mch_memmove((d), (s), STRLEN(s) + 1)

#if defined(HAVE_STRNCASECMP)
#define STRNICMP(d, s, n) strncasecmp((char *)(d), (char *)(s), (size_t)(n))
#else
#if defined(HAVE_STRNICMP)
#define STRNICMP(d, s, n) strnicmp((char *)(d), (char *)(s), (size_t)(n))
#else
#define STRNICMP(d, s, n) vim_strnicmp((char *)(d), (char *)(s), (size_t)(n))
#endif
#endif







#define MB_STRICMP(d, s) mb_strnicmp((char_u *)(d), (char_u *)(s), (int)MAXCOL)
#define MB_STRNICMP(d, s, n) mb_strnicmp((char_u *)(d), (char_u *)(s), (int)(n))

#define STRCAT(d, s) strcat((char *)(d), (char *)(s))
#define STRNCAT(d, s, n) strncat((char *)(d), (char *)(s), (size_t)(n))

#if defined(HAVE_STRPBRK)
#define vim_strpbrk(s, cs) (char_u *)strpbrk((char *)(s), (char *)(cs))
#endif

#define OUT_STR(s) out_str((char_u *)(s))
#define OUT_STR_NF(s) out_str_nf((char_u *)(s))

#if defined(FEAT_GUI)
#if defined(FEAT_TERMGUICOLORS)
#define GUI_FUNCTION(f) (gui.in_use ? gui_##f : termgui_##f)
#define GUI_FUNCTION2(f, pixel) (gui.in_use ? ((pixel) != INVALCOLOR ? gui_##f((pixel)) : INVALCOLOR) : termgui_##f((pixel)))




#define USE_24BIT (gui.in_use || p_tgc)
#else
#define GUI_FUNCTION(f) gui_##f
#define GUI_FUNCTION2(f,pixel) ((pixel) != INVALCOLOR ? gui_##f((pixel)) : INVALCOLOR)


#define USE_24BIT gui.in_use
#endif
#else
#if defined(FEAT_TERMGUICOLORS)
#define GUI_FUNCTION(f) termgui_##f
#define GUI_FUNCTION2(f, pixel) termgui_##f((pixel))
#define USE_24BIT p_tgc
#endif
#endif
#if defined(FEAT_TERMGUICOLORS)
#define IS_CTERM (t_colors > 1 || p_tgc)
#else
#define IS_CTERM (t_colors > 1)
#endif
#if defined(GUI_FUNCTION)
#define GUI_MCH_GET_RGB GUI_FUNCTION(mch_get_rgb)
#define GUI_MCH_GET_RGB2(pixel) GUI_FUNCTION2(mch_get_rgb, (pixel))
#define GUI_MCH_GET_COLOR GUI_FUNCTION(mch_get_color)
#define GUI_GET_COLOR GUI_FUNCTION(get_color)
#endif



#if defined(HAVE_STRERROR)
#define PERROR(msg) (void)semsg("%s: %s", (char *)msg, strerror(errno))
#else
#define PERROR(msg) do_perror(msg)
#endif

typedef long linenr_T; 
typedef int colnr_T; 
typedef unsigned short disptick_T; 










#if defined(__MVS__)
#define MAXCOL (0x3fffffffL) 
#define MAXLNUM (0x3fffffffL) 
#else
#define MAXCOL INT_MAX 
#define MAXLNUM LONG_MAX 
#endif

#define SHOWCMD_COLS 10 
#define STL_MAX_ITEM 80 

typedef void *vim_acl_T; 

#if !defined(mch_memmove)
#define mch_memmove(to, from, len) memmove((char*)(to), (char*)(from), (size_t)(len))
#endif







#define fnamecmp(x, y) vim_fnamecmp((char_u *)(x), (char_u *)(y))
#define fnamencmp(x, y, n) vim_fnamencmp((char_u *)(x), (char_u *)(y), (size_t)(n))

#if defined(HAVE_MEMSET)
#define vim_memset(ptr, c, size) memset((ptr), (c), (size))
#else
void *vim_memset(void *, int, size_t);
#endif

#if defined(UNIX) || defined(FEAT_GUI) || defined(VMS) || defined(FEAT_CLIENTSERVER)

#define USE_INPUT_BUF
#endif

#if !defined(EINTR)
#define read_eintr(fd, buf, count) vim_read((fd), (buf), (count))
#define write_eintr(fd, buf, count) vim_write((fd), (buf), (count))
#endif

#if defined(MSWIN)


#define vim_read(fd, buf, count) read((fd), (char *)(buf), (unsigned int)(count))
#define vim_write(fd, buf, count) write((fd), (char *)(buf), (unsigned int)(count))
#else
#define vim_read(fd, buf, count) read((fd), (char *)(buf), (size_t) (count))
#define vim_write(fd, buf, count) write((fd), (char *)(buf), (size_t) (count))
#endif




#define HL_ATTR(n) highlight_attr[(int)(n)]
#define TERM_STR(n) term_strings[(int)(n)]





#if !defined(EXTERN)
#define EXTERN extern
#define INIT(x)
#define INIT2(a, b)
#define INIT3(a, b, c)
#define INIT4(a, b, c, d)
#define INIT5(a, b, c, d, e)
#else
#if !defined(INIT)
#define INIT(x) x
#define INIT2(a, b) = {a, b}
#define INIT3(a, b, c) = {a, b, c}
#define INIT4(a, b, c, d) = {a, b, c, d}
#define INIT5(a, b, c, d, e) = {a, b, c, d, e}
#define DO_INIT
#endif
#endif

#define MAX_MCO 6 




#define MB_MAXBYTES 21

#if (defined(FEAT_PROFILE) || defined(FEAT_RELTIME)) && !defined(PROTO)
#if defined(MSWIN)
typedef LARGE_INTEGER proftime_T;
#else
typedef struct timeval proftime_T;
#endif
#else
typedef int proftime_T; 
#endif






#if defined(PROTO)
typedef long time_T;
#else
#if defined(MSWIN)
typedef __time64_t time_T;
#else
typedef time_t time_T;
#endif
#endif

#if defined(_WIN64)
typedef __int64 sock_T;
#else
typedef int sock_T;
#endif



#include "option.h" 

#include "beval.h" 



#include "structs.h" 

#include "alloc.h"


#define PROF_NONE 0 
#define PROF_YES 1 
#define PROF_PAUSED 2 



#define MOUSE_LEFT 0x00
#define MOUSE_MIDDLE 0x01
#define MOUSE_RIGHT 0x02
#define MOUSE_RELEASE 0x03


#define MOUSE_SHIFT 0x04
#define MOUSE_ALT 0x08
#define MOUSE_CTRL 0x10





#define MOUSE_4 0x100 
#define MOUSE_5 0x200 

#define MOUSE_X1 0x300 
#define MOUSE_X2 0x400 

#define MOUSE_6 0x500 
#define MOUSE_7 0x600 


#define MOUSE_DRAG_XTERM 0x40

#define MOUSE_DRAG (0x40 | MOUSE_RELEASE)


#define MOUSEWHEEL_LOW 0x60

#define MOUSE_CLICK_MASK 0x03

#define NUM_MOUSE_CLICKS(code) (((unsigned)((code) & 0xC0) >> 6) + 1)


#define SET_NUM_MOUSE_CLICKS(code, num) (code) = ((code) & 0x3f) | ((((num) - 1) & 3) << 6)





#define MOUSE_COLOFF 10000





#define IN_UNKNOWN 0
#define IN_BUFFER 1
#define IN_STATUS_LINE 2 
#define IN_SEP_LINE 4 
#define IN_OTHER_WIN 8 
#define CURSOR_MOVED 0x100
#define MOUSE_FOLD_CLOSE 0x200 
#define MOUSE_FOLD_OPEN 0x400 
#define MOUSE_WINBAR 0x800 


#define MOUSE_FOCUS 0x01 
#define MOUSE_MAY_VIS 0x02 
#define MOUSE_DID_MOVE 0x04 
#define MOUSE_SETPOS 0x08 
#define MOUSE_MAY_STOP_VIS 0x10 
#define MOUSE_RELEASED 0x20 

#if defined(UNIX) && defined(HAVE_GETTIMEOFDAY) && defined(HAVE_SYS_TIME_H)
#define CHECK_DOUBLE_CLICK 1 
#endif



#define VALID_PATH 1
#define VALID_HEAD 2


#define VV_COUNT 0
#define VV_COUNT1 1
#define VV_PREVCOUNT 2
#define VV_ERRMSG 3
#define VV_WARNINGMSG 4
#define VV_STATUSMSG 5
#define VV_SHELL_ERROR 6
#define VV_THIS_SESSION 7
#define VV_VERSION 8
#define VV_LNUM 9
#define VV_TERMRESPONSE 10
#define VV_FNAME 11
#define VV_LANG 12
#define VV_LC_TIME 13
#define VV_CTYPE 14
#define VV_CC_FROM 15
#define VV_CC_TO 16
#define VV_FNAME_IN 17
#define VV_FNAME_OUT 18
#define VV_FNAME_NEW 19
#define VV_FNAME_DIFF 20
#define VV_CMDARG 21
#define VV_FOLDSTART 22
#define VV_FOLDEND 23
#define VV_FOLDDASHES 24
#define VV_FOLDLEVEL 25
#define VV_PROGNAME 26
#define VV_SEND_SERVER 27
#define VV_DYING 28
#define VV_EXCEPTION 29
#define VV_THROWPOINT 30
#define VV_REG 31
#define VV_CMDBANG 32
#define VV_INSERTMODE 33
#define VV_VAL 34
#define VV_KEY 35
#define VV_PROFILING 36
#define VV_FCS_REASON 37
#define VV_FCS_CHOICE 38
#define VV_BEVAL_BUFNR 39
#define VV_BEVAL_WINNR 40
#define VV_BEVAL_WINID 41
#define VV_BEVAL_LNUM 42
#define VV_BEVAL_COL 43
#define VV_BEVAL_TEXT 44
#define VV_SCROLLSTART 45
#define VV_SWAPNAME 46
#define VV_SWAPCHOICE 47
#define VV_SWAPCOMMAND 48
#define VV_CHAR 49
#define VV_MOUSE_WIN 50
#define VV_MOUSE_WINID 51
#define VV_MOUSE_LNUM 52
#define VV_MOUSE_COL 53
#define VV_OP 54
#define VV_SEARCHFORWARD 55
#define VV_HLSEARCH 56
#define VV_OLDFILES 57
#define VV_WINDOWID 58
#define VV_PROGPATH 59
#define VV_COMPLETED_ITEM 60
#define VV_OPTION_NEW 61
#define VV_OPTION_OLD 62
#define VV_OPTION_OLDLOCAL 63
#define VV_OPTION_OLDGLOBAL 64
#define VV_OPTION_COMMAND 65
#define VV_OPTION_TYPE 66
#define VV_ERRORS 67
#define VV_FALSE 68
#define VV_TRUE 69
#define VV_NONE 70
#define VV_NULL 71
#define VV_NUMBERSIZE 72
#define VV_VIM_DID_ENTER 73
#define VV_TESTING 74
#define VV_TYPE_NUMBER 75
#define VV_TYPE_STRING 76
#define VV_TYPE_FUNC 77
#define VV_TYPE_LIST 78
#define VV_TYPE_DICT 79
#define VV_TYPE_FLOAT 80
#define VV_TYPE_BOOL 81
#define VV_TYPE_NONE 82
#define VV_TYPE_JOB 83
#define VV_TYPE_CHANNEL 84
#define VV_TYPE_BLOB 85
#define VV_TERMRFGRESP 86
#define VV_TERMRBGRESP 87
#define VV_TERMU7RESP 88
#define VV_TERMSTYLERESP 89
#define VV_TERMBLINKRESP 90
#define VV_EVENT 91
#define VV_VERSIONLONG 92
#define VV_ECHOSPACE 93
#define VV_ARGV 94
#define VV_LEN 95 


#define VVAL_FALSE 0L 
#define VVAL_TRUE 1L 
#define VVAL_NONE 2L 
#define VVAL_NULL 3L 


#define VAR_TYPE_NUMBER 0
#define VAR_TYPE_STRING 1
#define VAR_TYPE_FUNC 2
#define VAR_TYPE_LIST 3
#define VAR_TYPE_DICT 4
#define VAR_TYPE_FLOAT 5
#define VAR_TYPE_BOOL 6
#define VAR_TYPE_NONE 7
#define VAR_TYPE_JOB 8
#define VAR_TYPE_CHANNEL 9
#define VAR_TYPE_BLOB 10

#define DICT_MAXNEST 100 

#if defined(FEAT_CLIPBOARD)




#define VIM_ATOM_NAME "_VIM_TEXT"
#define VIMENC_ATOM_NAME "_VIMENC_TEXT"


#define SELECT_CLEARED 0
#define SELECT_IN_PROGRESS 1
#define SELECT_DONE 2

#define SELECT_MODE_CHAR 0
#define SELECT_MODE_WORD 1
#define SELECT_MODE_LINE 2

#if defined(FEAT_GUI_MSWIN)
#if defined(FEAT_OLE)
#define WM_OLE (WM_APP+0)
#endif
#endif


typedef struct
{
int available; 
int owned; 
pos_T start; 
pos_T end; 
int vmode; 


short_u origin_row;
short_u origin_start_col;
short_u origin_end_col;
short_u word_start_col;
short_u word_end_col;
#if defined(FEAT_PROP_POPUP)

short_u min_col;
short_u max_col;
short_u min_row;
short_u max_row;
#endif

pos_T prev; 
short_u state; 
short_u mode; 

#if defined(FEAT_GUI_X11) || defined(FEAT_XCLIPBOARD)
Atom sel_atom; 
#endif

#if defined(FEAT_GUI_GTK)
GdkAtom gtk_sel_atom; 
#endif

#if defined(MSWIN) || defined(FEAT_CYGWIN_WIN32_CLIPBOARD)
int_u format; 
int_u format_raw; 
#endif
#if defined(FEAT_GUI_HAIKU)

#endif
} Clipboard_T;
#else
typedef int Clipboard_T; 
#endif


#if (defined(_MSC_VER) && (_MSC_VER >= 1300)) || defined(__MINGW32__)
#define HAVE_STAT64
typedef struct _stat64 stat_T;
#else
typedef struct stat stat_T;
#endif

#if defined(__GNUC__) && !defined(__MINGW32__)
#define USE_PRINTF_FORMAT_ATTRIBUTE
#endif

typedef enum
{
ASSERT_EQUAL,
ASSERT_NOTEQUAL,
ASSERT_MATCH,
ASSERT_NOTMATCH,
ASSERT_OTHER
} assert_type_T;


typedef enum {
PASTE_INSERT, 
PASTE_CMDLINE, 
PASTE_EX, 
PASTE_ONE_CHAR 
} paste_mode_T;


typedef enum {
FLUSH_MINIMAL,
FLUSH_TYPEAHEAD, 
FLUSH_INPUT 
} flush_buffers_T;


typedef enum {
USEPOPUP_NONE,
USEPOPUP_NORMAL, 
USEPOPUP_HIDDEN 
} use_popup_T;


#define LET_IS_CONST 1 
#define LET_NO_COMMAND 2 

#include "ex_cmds.h" 
#include "spell.h" 

#include "proto.h" 





#if !defined(MSWIN) && !defined(FEAT_GUI_X11) && !defined(FEAT_GUI_HAIKU) && !defined(FEAT_GUI_GTK) && !defined(FEAT_GUI_MAC) && !defined(PROTO)

#define mch_errmsg(str) fprintf(stderr, "%s", (str))
#define display_errors() fflush(stderr)
#define mch_msg(str) printf("%s", (str))
#else
#define USE_MCH_ERRMSG
#endif

#if defined(FEAT_EVAL) && (!defined(FEAT_GUI_MSWIN) || !(defined(FEAT_MBYTE_IME) || defined(GLOBAL_IME))) && !(defined(FEAT_GUI_MAC) && defined(MACOS_CONVERT))







#define IME_WITHOUT_XIM
#endif

#if defined(FEAT_XIM) || defined(IME_WITHOUT_XIM) || (defined(FEAT_GUI_MSWIN) && (defined(FEAT_MBYTE_IME) || defined(GLOBAL_IME))) || defined(FEAT_GUI_MAC)





#define HAVE_INPUT_METHOD
#endif

#if !defined(FEAT_LINEBREAK)

#define number_width(x) 7
#endif



#if !defined(MSWIN) && !defined(VMS)
#define mch_open(n, m, p) open((n), (m), (p))
#define mch_fopen(n, p) fopen((n), (p))
#endif

#include "globals.h" 




#if defined(FEAT_GUI_DIALOG) && !defined(FEAT_CON_DIALOG)
#define do_dialog gui_mch_dialog
#endif








#if defined(FEAT_BROWSE)
#if defined(BACKSLASH_IN_FILENAME)
#define BROWSE_FILTER_MACROS (char_u *)N_("Vim macro files (*.vim)\t*.vim\nAll Files (*.*)\t*.*\n")

#define BROWSE_FILTER_ALL_FILES (char_u *)N_("All Files (*.*)\t*.*\n")
#define BROWSE_FILTER_DEFAULT (char_u *)N_("All Files (*.*)\t*.*\nC source (*.c, *.h)\t*.c;*.h\nC++ source (*.cpp, *.hpp)\t*.cpp;*.hpp\nVB code (*.bas, *.frm)\t*.bas;*.frm\nVim files (*.vim, _vimrc, _gvimrc)\t*.vim;_vimrc;_gvimrc\n")

#else
#define BROWSE_FILTER_MACROS (char_u *)N_("Vim macro files (*.vim)\t*.vim\nAll Files (*)\t*\n")

#define BROWSE_FILTER_ALL_FILES (char_u *)N_("All Files (*)\t*\n")
#define BROWSE_FILTER_DEFAULT (char_u *)N_("All Files (*)\t*\nC source (*.c, *.h)\t*.c;*.h\nC++ source (*.cpp, *.hpp)\t*.cpp;*.hpp\nVim files (*.vim, _vimrc, _gvimrc)\t*.vim;_vimrc;_gvimrc\n")

#endif
#define BROWSE_SAVE 1 
#define BROWSE_DIR 2 
#endif

#if defined(_MSC_VER)

#pragma warning(disable : 4312)

#pragma warning(disable : 4131)

#pragma warning(disable : 4244)

#pragma warning(disable : 4306)

#pragma warning(disable : 4100)

#pragma warning(disable : 4057)

#pragma warning(disable : 4127)

#pragma warning(disable : 4706)
#endif


#if defined(MEM_PROFILE)
#define vim_realloc(ptr, size) mem_realloc((ptr), (size))
#else
#define vim_realloc(ptr, size) realloc((ptr), (size))
#endif







#define MB_BYTE2LEN(b) mb_bytelen_tab[b]
#define MB_BYTE2LEN_CHECK(b) (((b) < 0 || (b) > 255) ? 1 : mb_bytelen_tab[b])


#define ENC_8BIT 0x01
#define ENC_DBCS 0x02
#define ENC_UNICODE 0x04

#define ENC_ENDIAN_B 0x10 
#define ENC_ENDIAN_L 0x20 

#define ENC_2BYTE 0x40 
#define ENC_4BYTE 0x80 
#define ENC_2WORD 0x100 

#define ENC_LATIN1 0x200 
#define ENC_LATIN9 0x400 
#define ENC_MACROMAN 0x800 

#if defined(USE_ICONV)
#if !defined(EILSEQ)
#define EILSEQ 123
#endif
#if defined(DYNAMIC_ICONV)

#define ICONV_ERRNO (*iconv_errno())
#define ICONV_E2BIG 7
#define ICONV_EINVAL 22
#define ICONV_EILSEQ 42
#else
#define ICONV_ERRNO errno
#define ICONV_E2BIG E2BIG
#define ICONV_EINVAL EINVAL
#define ICONV_EILSEQ EILSEQ
#endif
#endif

#define SIGN_BYTE 1 


#if defined(FEAT_NETBEANS_INTG)
#define MULTISIGN_BYTE 2 

#endif

#if defined(FEAT_GUI) && defined(FEAT_XCLIPBOARD)
#if defined(FEAT_GUI_GTK)


#define X_DISPLAY ((gui.in_use) ? gui_mch_get_display() : xterm_dpy)
#else
#define X_DISPLAY (gui.in_use ? gui.dpy : xterm_dpy)
#endif
#else
#if defined(FEAT_GUI)
#if defined(FEAT_GUI_GTK)
#define X_DISPLAY ((gui.in_use) ? gui_mch_get_display() : (Display *)NULL)
#else
#define X_DISPLAY gui.dpy
#endif
#else
#define X_DISPLAY xterm_dpy
#endif
#endif

#if defined(FEAT_BROWSE) && defined(GTK_CHECK_VERSION)
#if GTK_CHECK_VERSION(2,4,0)
#define USE_FILE_CHOOSER
#endif
#endif

#if defined(FEAT_GUI_GTK)
#if !GTK_CHECK_VERSION(2,14,0)
#define gtk_widget_get_window(wid) ((wid)->window)
#define gtk_plug_get_socket_window(wid) ((wid)->socket_window)
#define gtk_selection_data_get_data(sel) ((sel)->data)
#define gtk_selection_data_get_data_type(sel) ((sel)->type)
#define gtk_selection_data_get_format(sel) ((sel)->format)
#define gtk_selection_data_get_length(sel) ((sel)->length)
#define gtk_adjustment_set_lower(adj, low) do { (adj)->lower = low; } while (0)

#define gtk_adjustment_set_upper(adj, up) do { (adj)->upper = up; } while (0)

#define gtk_adjustment_set_page_size(adj, size) do { (adj)->page_size = size; } while (0)

#define gtk_adjustment_set_page_increment(adj, inc) do { (adj)->page_increment = inc; } while (0)

#define gtk_adjustment_set_step_increment(adj, inc) do { (adj)->step_increment = inc; } while (0)

#endif
#if !GTK_CHECK_VERSION(2,16,0)
#define gtk_selection_data_get_selection(sel) ((sel)->selection)
#endif
#if !GTK_CHECK_VERSION(2,18,0)
#define gtk_widget_get_allocation(wid, alloc) do { *(alloc) = (wid)->allocation; } while (0)

#define gtk_widget_set_allocation(wid, alloc) do { (wid)->allocation = *(alloc); } while (0)

#define gtk_widget_get_has_window(wid) !GTK_WIDGET_NO_WINDOW(wid)
#define gtk_widget_get_sensitive(wid) GTK_WIDGET_SENSITIVE(wid)
#define gtk_widget_get_visible(wid) GTK_WIDGET_VISIBLE(wid)
#define gtk_widget_has_focus(wid) GTK_WIDGET_HAS_FOCUS(wid)
#define gtk_widget_set_window(wid, win) do { (wid)->window = (win); } while (0)

#define gtk_widget_set_can_default(wid, can) do { if (can) { GTK_WIDGET_SET_FLAGS(wid, GTK_CAN_DEFAULT); } else { GTK_WIDGET_UNSET_FLAGS(wid, GTK_CAN_DEFAULT); } } while (0)


#define gtk_widget_set_can_focus(wid, can) do { if (can) { GTK_WIDGET_SET_FLAGS(wid, GTK_CAN_FOCUS); } else { GTK_WIDGET_UNSET_FLAGS(wid, GTK_CAN_FOCUS); } } while (0)


#define gtk_widget_set_visible(wid, vis) do { if (vis) { gtk_widget_show(wid); } else { gtk_widget_hide(wid); } } while (0)


#endif
#if !GTK_CHECK_VERSION(2,20,0)
#define gtk_widget_get_mapped(wid) GTK_WIDGET_MAPPED(wid)
#define gtk_widget_get_realized(wid) GTK_WIDGET_REALIZED(wid)
#define gtk_widget_set_mapped(wid, map) do { if (map) { GTK_WIDGET_SET_FLAGS(wid, GTK_MAPPED); } else { GTK_WIDGET_UNSET_FLAGS(wid, GTK_MAPPED); } } while (0)


#define gtk_widget_set_realized(wid, rea) do { if (rea) { GTK_WIDGET_SET_FLAGS(wid, GTK_REALIZED); } else { GTK_WIDGET_UNSET_FLAGS(wid, GTK_REALIZED); } } while (0)


#endif
#endif

#if !defined(FEAT_NETBEANS_INTG)
#undef NBDEBUG
#endif
#if defined(NBDEBUG)
#include "nbdebug.h"
#else
#define nbdebug(a)
#endif

#if defined(IN_PERL_FILE)



#undef NORMAL
#undef STRLEN
#undef FF
#undef OP_DELETE
#undef OP_JOIN

#if defined(MAX)
#undef MAX
#endif
#if defined(MIN)
#undef MIN
#endif

#if defined(_)
#undef _
#endif
#if defined(DEBUG)
#undef DEBUG
#endif
#if defined(_DEBUG)
#undef _DEBUG
#endif
#if defined(instr)
#undef instr
#endif


#if (defined(MACOS_X) && !defined(MAC_OS_X_VERSION_10_6)) && defined(bool) && !defined(FEAT_PERL)

#undef bool
#endif

#endif


#define SIGNAL_BLOCK -1
#define SIGNAL_UNBLOCK -2
#if !defined(UNIX) && !defined(VMS)
#define vim_handle_signal(x) 0
#endif


#define VGR_GLOBAL 1
#define VGR_NOJUMP 2


#define BAD_REPLACE '?' 
#define BAD_KEEP -1 
#define BAD_DROP -2 


#define DOSO_NONE 0
#define DOSO_VIMRC 1 
#define DOSO_GVIMRC 2 


#define VIF_WANT_INFO 1 
#define VIF_WANT_MARKS 2 
#define VIF_FORCEIT 4 
#define VIF_GET_OLDFILES 8 


#define BFA_DEL 1 
#define BFA_WIPE 2 
#define BFA_KEEP_UNDO 4 
#define BFA_IGNORE_ABORT 8 


#define MSCR_DOWN 0 
#define MSCR_UP 1
#define MSCR_LEFT -1
#define MSCR_RIGHT -2

#define KEYLEN_PART_KEY -1 
#define KEYLEN_PART_MAP -2 
#define KEYLEN_REMOVED 9999 


#define FILEINFO_OK 0
#define FILEINFO_ENC_FAIL 1 
#define FILEINFO_READ_FAIL 2 
#define FILEINFO_INFO_FAIL 3 


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


#define GREG_NO_EXPR 1 
#define GREG_EXPR_SRC 2 
#define GREG_LIST 4 


#define AUTOLOAD_CHAR '#'

#if defined(FEAT_JOB_CHANNEL)
#define MAX_OPEN_CHANNELS 10
#else
#define MAX_OPEN_CHANNELS 0
#endif

#if defined(MSWIN)
#define MAX_NAMED_PIPE_SIZE 65535
#endif


#define JSON_JS 1 
#define JSON_NO_NONE 2 
#define JSON_NL 4 


#define DIP_ALL 0x01 
#define DIP_DIR 0x02 
#define DIP_ERR 0x04 
#define DIP_START 0x08 
#define DIP_OPT 0x10 
#define DIP_NORTP 0x20 
#define DIP_NOAFTER 0x40 
#define DIP_AFTER 0x80 


#define LOWEST_WIN_ID 1000


#define COPYID_INC 2
#define COPYID_MASK (~0x1)


#define TFN_INT 1 
#define TFN_QUIET 2 
#define TFN_NO_AUTOLOAD 4 
#define TFN_NO_DEREF 8 
#define TFN_READ_ONLY 16 


#define GLV_QUIET TFN_QUIET 
#define GLV_NO_AUTOLOAD TFN_NO_AUTOLOAD 
#define GLV_READ_ONLY TFN_READ_ONLY 

#define DO_NOT_FREE_CNT 99999 



#define FCERR_UNKNOWN 0
#define FCERR_TOOMANY 1
#define FCERR_TOOFEW 2
#define FCERR_SCRIPT 3
#define FCERR_DICT 4
#define FCERR_NONE 5
#define FCERR_OTHER 6
#define FCERR_DELETED 7
#define FCERR_NOTMETHOD 8 


#define FLEN_FIXED 40


#define FNE_INCL_BR 1 
#define FNE_CHECK_START 2 


#if (defined(SUN_SYSTEM) || defined(BSD) || defined(__FreeBSD_kernel__)) && (defined(S_ISCHR) || defined(S_IFCHR))

#define OPEN_CHR_FILES
#endif


#if !defined(S_ISDIR)
#if defined(S_IFDIR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#else
#define S_ISDIR(m) 0
#endif
#endif
#if !defined(S_ISREG)
#if defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#else
#define S_ISREG(m) 0
#endif
#endif
#if !defined(S_ISBLK)
#if defined(S_IFBLK)
#define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#else
#define S_ISBLK(m) 0
#endif
#endif
#if !defined(S_ISSOCK)
#if defined(S_IFSOCK)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#else
#define S_ISSOCK(m) 0
#endif
#endif
#if !defined(S_ISFIFO)
#if defined(S_IFIFO)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#else
#define S_ISFIFO(m) 0
#endif
#endif
#if !defined(S_ISCHR)
#if defined(S_IFCHR)
#define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#else
#define S_ISCHR(m) 0
#endif
#endif
#if !defined(S_ISLNK)
#if defined(S_IFLNK)
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#else
#define S_ISLNK(m) 0
#endif
#endif

#if defined(HAVE_GETTIMEOFDAY) && defined(HAVE_SYS_TIME_H)
#define ELAPSED_TIMEVAL
#define ELAPSED_INIT(v) gettimeofday(&v, NULL)
#define ELAPSED_FUNC(v) elapsed(&v)
typedef struct timeval elapsed_T;
long elapsed(struct timeval *start_tv);
#elif defined(MSWIN)
#define ELAPSED_TICKCOUNT
#define ELAPSED_INIT(v) v = GetTickCount()
#define ELAPSED_FUNC(v) elapsed(v)
#if defined(PROTO)
typedef int DWORD;
#endif
typedef DWORD elapsed_T;
#if !defined(PROTO)
long elapsed(DWORD start_tick);
#endif
#endif


#define REPLACE_CR_NCHAR -1
#define REPLACE_NL_NCHAR -2


#define TERM_START_NOJOB 1
#define TERM_START_FORCEIT 2
#define TERM_START_SYSTEM 4


#define SAVE_RESTORE_TITLE 1
#define SAVE_RESTORE_ICON 2
#define SAVE_RESTORE_BOTH (SAVE_RESTORE_TITLE | SAVE_RESTORE_ICON)


#define APC_SAVE_FOR_UNDO 1 
#define APC_SUBSTITUTE 2 

#define CLIP_ZINDEX 32000


#define REPTERM_FROM_PART 1
#define REPTERM_DO_LT 2
#define REPTERM_SPECIAL 4
#define REPTERM_NO_SIMPLIFY 8

#endif 
