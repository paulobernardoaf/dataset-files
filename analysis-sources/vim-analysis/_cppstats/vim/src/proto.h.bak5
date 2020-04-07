















#if !defined(PROTO) && !defined(NOPROTO)





#if !defined(FEAT_X11) && !defined(FEAT_GUI_GTK)
#define Display int
#define Widget int
#endif
#if !defined(FEAT_GUI_GTK)
#define GdkEvent int
#define GdkEventKey int
#endif
#if !defined(FEAT_X11)
#define XImage int
#endif

#if defined(AMIGA)
#include "os_amiga.pro"
#endif
#if defined(UNIX) || defined(VMS)
#include "os_unix.pro"
#endif
#if defined(MSWIN)
#include "os_win32.pro"
#include "os_mswin.pro"
#include "winclip.pro"
#if (defined(__GNUC__) && !defined(__MINGW32__))
extern int _stricoll(char *a, char *b);
#endif
#endif
#if defined(VMS)
#include "os_vms.pro"
#endif
#if defined(__BEOS__)
#include "os_beos.pro"
#endif
#if defined(__QNX__)
#include "os_qnx.pro"
#endif

#if defined(FEAT_CRYPT)
#include "blowfish.pro"
#include "crypt.pro"
#include "crypt_zip.pro"
#endif
#include "arglist.pro"
#include "autocmd.pro"
#include "buffer.pro"
#include "bufwrite.pro"
#include "change.pro"
#include "charset.pro"
#include "cindent.pro"
#include "clipboard.pro"
#include "cmdexpand.pro"
#include "cmdhist.pro"
#include "if_cscope.pro"
#include "debugger.pro"
#include "dict.pro"
#include "diff.pro"
#include "digraph.pro"
#include "drawline.pro"
#include "drawscreen.pro"
#include "edit.pro"
#include "eval.pro"
#include "evalbuffer.pro"
#include "evalfunc.pro"
#include "evalvars.pro"
#include "evalwindow.pro"
#include "ex_cmds.pro"
#include "ex_cmds2.pro"
#include "ex_docmd.pro"
#include "ex_eval.pro"
#include "ex_getln.pro"
#include "fileio.pro"
#include "filepath.pro"
#include "findfile.pro"
#include "fold.pro"
#include "getchar.pro"
#include "hardcopy.pro"
#include "hashtab.pro"
#include "highlight.pro"
#include "indent.pro"
#include "insexpand.pro"
#include "json.pro"
#include "list.pro"
#include "blob.pro"
#include "main.pro"
#include "map.pro"
#include "mark.pro"
#include "memfile.pro"
#include "memline.pro"
#if defined(FEAT_MENU)
#include "menu.pro"
#endif
#if defined(FEAT_ARABIC)
#include "arabic.pro"
#endif
#if defined(FEAT_VIMINFO)
#include "viminfo.pro"
#endif


int smsg(const char *, ...)
#if defined(USE_PRINTF_FORMAT_ATTRIBUTE)
__attribute__((format(printf, 1, 2)))
#endif
;

int smsg_attr(int, const char *, ...)
#if defined(USE_PRINTF_FORMAT_ATTRIBUTE)
__attribute__((format(printf, 2, 3)))
#endif
;

int smsg_attr_keep(int, const char *, ...)
#if defined(USE_PRINTF_FORMAT_ATTRIBUTE)
__attribute__((format(printf, 2, 3)))
#endif
;


int semsg(const char *, ...)
#if defined(USE_PRINTF_FORMAT_ATTRIBUTE)
__attribute__((format(printf, 1, 2)))
#endif
;


void siemsg(const char *, ...)
#if defined(USE_PRINTF_FORMAT_ATTRIBUTE)
__attribute__((format(printf, 1, 2)))
#endif
;

int vim_snprintf_add(char *, size_t, const char *, ...)
#if defined(USE_PRINTF_FORMAT_ATTRIBUTE)
__attribute__((format(printf, 3, 4)))
#endif
;

int vim_snprintf(char *, size_t, const char *, ...)
#if defined(USE_PRINTF_FORMAT_ATTRIBUTE)
__attribute__((format(printf, 3, 4)))
#endif
;

int vim_vsnprintf(char *str, size_t str_m, const char *fmt, va_list ap);
int vim_vsnprintf_typval(char *str, size_t str_m, const char *fmt, va_list ap, typval_T *tvs);

#include "message.pro"
#include "misc1.pro"
#include "misc2.pro"
#if !defined(HAVE_STRPBRK)
char_u *vim_strpbrk(char_u *s, char_u *charset);
#endif
#if !defined(HAVE_QSORT)

void qsort(void *base, size_t elm_count, size_t elm_size, int (*cmp)(const void *, const void *));
#endif
#include "mouse.pro"
#include "move.pro"
#include "mbyte.pro"
#if defined(VIMDLL)

int mbyte_im_get_status(void);
void mbyte_im_set_active(int active_arg);
#endif
#include "normal.pro"
#include "ops.pro"
#include "option.pro"
#include "optionstr.pro"
#include "popupmenu.pro"
#if defined(FEAT_PROFILE) || defined(FEAT_RELTIME)
#include "profiler.pro"
#endif
#include "quickfix.pro"
#include "regexp.pro"
#include "register.pro"
#include "scriptfile.pro"
#include "screen.pro"
#include "session.pro"
#if defined(FEAT_CRYPT) || defined(FEAT_PERSISTENT_UNDO)
#include "sha256.pro"
#endif
#include "search.pro"
#if defined(FEAT_SIGNS)
#include "sign.pro"
#endif
#include "sound.pro"
#include "spell.pro"
#include "spellfile.pro"
#include "spellsuggest.pro"
#include "syntax.pro"
#include "tag.pro"
#include "term.pro"
#if defined(FEAT_TERMINAL)
#include "terminal.pro"
#endif
#if defined(HAVE_TGETENT) && (defined(AMIGA) || defined(VMS))
#include "termlib.pro"
#endif
#if defined(FEAT_PROP_POPUP)
#include "popupwin.pro"
#include "textprop.pro"
#endif
#include "testing.pro"
#include "time.pro"
#include "ui.pro"
#include "undo.pro"
#include "usercmd.pro"
#include "userfunc.pro"
#include "version.pro"
#if defined(FEAT_EVAL)
#include "vim9compile.pro"
#include "vim9execute.pro"
#include "vim9script.pro"
#endif
#include "window.pro"

#if defined(FEAT_LUA)
#include "if_lua.pro"
#endif

#if defined(FEAT_MZSCHEME)
#include "if_mzsch.pro"
#endif

#if defined(FEAT_PYTHON)
#include "if_python.pro"
#endif

#if defined(FEAT_PYTHON3)
#include "if_python3.pro"
#endif

#if defined(FEAT_TCL)
#include "if_tcl.pro"
#endif

#if defined(FEAT_RUBY)
#include "if_ruby.pro"
#endif



#if !defined(FEAT_BEVAL)
#define BalloonEval int
#endif
#if defined(FEAT_BEVAL) || defined(FEAT_PROP_POPUP)
#include "beval.pro"
#endif

#if defined(FEAT_NETBEANS_INTG)
#include "netbeans.pro"
#endif
#if defined(FEAT_JOB_CHANNEL)
#include "channel.pro"


void ch_log(channel_T *ch, const char *fmt, ...)
#if defined(USE_PRINTF_FORMAT_ATTRIBUTE)
__attribute__((format(printf, 2, 3)))
#endif
;

#endif

#if defined(FEAT_GUI) || defined(FEAT_JOB_CHANNEL)
#if defined(UNIX) || defined(MACOS_X) || defined(VMS)
#include "pty.pro"
#endif
#endif

#if defined(FEAT_GUI)
#include "gui.pro"
#if !defined(HAVE_SETENV) && !defined(HAVE_PUTENV) && !defined(VMS)
extern int putenv(const char *string); 
#if defined(USE_VIMPTY_GETENV)
extern char_u *vimpty_getenv(const char_u *string); 
#endif
#endif
#if defined(FEAT_GUI_MSWIN)
#include "gui_w32.pro"
#endif
#if defined(FEAT_GUI_GTK)
#include "gui_gtk.pro"
#include "gui_gtk_x11.pro"
#endif
#if defined(FEAT_GUI_MOTIF)
#include "gui_motif.pro"
#include "gui_xmdlg.pro"
#endif
#if defined(FEAT_GUI_ATHENA)
#include "gui_athena.pro"
#if defined(FEAT_BROWSE)
extern char *vim_SelFile(Widget toplevel, char *prompt, char *init_path, int (*show_entry)(), int x, int y, guicolor_T fg, guicolor_T bg, guicolor_T scroll_fg, guicolor_T scroll_bg);
#endif
#endif
#if defined(FEAT_GUI_HAIKU)
#include "gui_haiku.pro"
#endif
#if defined(FEAT_GUI_MAC)
#include "gui_mac.pro"
#endif
#if defined(FEAT_GUI_X11)
#include "gui_x11.pro"
#endif
#if defined(FEAT_GUI_PHOTON)
#include "gui_photon.pro"
#endif
#endif 

#if defined(FEAT_OLE)
#include "if_ole.pro"
#endif
#if defined(FEAT_CLIENTSERVER) && defined(FEAT_X11)
#include "if_xcmdsrv.pro"
#endif








#if defined(FEAT_PERL) && !defined(IN_PERL_FILE)
#define CV void
#include "if_perl.pro"
#include "if_perlsfio.pro"
#endif

#if defined(MACOS_CONVERT)
#include "os_mac_conv.pro"
#endif
#if defined(MACOS_X_DARWIN) && defined(FEAT_CLIPBOARD) && !defined(FEAT_GUI)

void clip_mch_lose_selection(Clipboard_T *cbd);
int clip_mch_own_selection(Clipboard_T *cbd);
void clip_mch_request_selection(Clipboard_T *cbd);
void clip_mch_set_selection(Clipboard_T *cbd);
#endif
#endif 
