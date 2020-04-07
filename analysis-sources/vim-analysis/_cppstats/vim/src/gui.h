#if defined(FEAT_GUI_MOTIF)
#include <Xm/Xm.h>
#endif
#if defined(FEAT_GUI_ATHENA)
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#endif
#if defined(FEAT_GUI_GTK)
#if defined(VMS)
#if defined(MAX)
#undef MAX
#endif
#if defined(MIN)
#undef MIN
#endif
#endif
#include <X11/Intrinsic.h>
#include <gtk/gtk.h>
#endif
#if defined(FEAT_GUI_HAIKU)
#include "gui_haiku.h"
#endif
#if defined(FEAT_XCLIPBOARD) && !defined(FEAT_GUI_MOTIF) && !defined(FEAT_GUI_ATHENA) && !defined(FEAT_GUI_GTK)
#include <X11/Intrinsic.h>
#endif
#if defined(FEAT_GUI_MAC)
#include <Types.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Menus.h>
#if !(defined (TARGET_API_MAC_CARBON) && (TARGET_API_MAC_CARBON))
#include <Windows.h>
#endif
#include <Controls.h>
#include <Dialogs.h>
#include <OSUtils.h>
#endif
#if defined(FEAT_GUI_PHOTON)
#include <Ph.h>
#include <Pt.h>
#include "photon/PxProto.h"
#endif
#if defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_MAC) || defined(FEAT_GUI_GTK)
#define USE_ON_FLY_SCROLL
#endif
#if (defined(FEAT_DND) && defined(FEAT_GUI_GTK)) || defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_MAC) || defined(FEAT_GUI_HAIKU)
#define HAVE_DROP_FILE
#endif
#if defined(FEAT_XFONTSET)
#define FONTSET_ALWAYS
#endif
#if defined(FEAT_GUI_MSWIN)
#define TEXT_X(col) ((col) * gui.char_width)
#define TEXT_Y(row) ((row) * gui.char_height + gui.char_ascent)
#define FILL_X(col) ((col) * gui.char_width)
#define FILL_Y(row) ((row) * gui.char_height)
#define X_2_COL(x) ((x) / gui.char_width)
#define Y_2_ROW(y) ((y) / gui.char_height)
#else
#define TEXT_X(col) ((col) * gui.char_width + gui.border_offset)
#define FILL_X(col) ((col) * gui.char_width + gui.border_offset)
#define X_2_COL(x) (((x) - gui.border_offset) / gui.char_width)
#define TEXT_Y(row) ((row) * gui.char_height + gui.char_ascent + gui.border_offset)
#define FILL_Y(row) ((row) * gui.char_height + gui.border_offset)
#define Y_2_ROW(y) (((y) - gui.border_offset) / gui.char_height)
#endif
#define SBAR_NONE -1
#define SBAR_LEFT 0
#define SBAR_RIGHT 1
#define SBAR_BOTTOM 2
#define SBAR_VERT 0
#define SBAR_HORIZ 1
#define SB_DEFAULT_WIDTH 16
#define MENU_DEFAULT_HEIGHT 1 
#define GUI_MON_WRAP_CURSOR 0x01 
#define GUI_MON_INVERT 0x02 
#define GUI_MON_IS_CURSOR 0x04 
#define GUI_MON_TRS_CURSOR 0x08 
#define GUI_MON_NOCLEAR 0x10 
#define DRAW_TRANSP 0x01 
#define DRAW_BOLD 0x02 
#define DRAW_UNDERL 0x04 
#define DRAW_UNDERC 0x08 
#if defined(FEAT_GUI_GTK)
#define DRAW_ITALIC 0x10 
#endif
#define DRAW_CURSOR 0x20 
#define DRAW_STRIKE 0x40 
#define TEAR_STRING "-->Detach"
#define TEAR_LEN (9) 
#define TOOLBAR_BUTTON_HEIGHT 18
#define TOOLBAR_BUTTON_WIDTH 18
#define TOOLBAR_BORDER_HEIGHT 12 
#if defined(FEAT_GUI_MSWIN)
#define TABLINE_HEIGHT 22
#endif
#if defined(FEAT_GUI_MOTIF)
#define TABLINE_HEIGHT 30
#endif
#if defined(NO_CONSOLE) || defined(FEAT_GUI_GTK) || defined(FEAT_GUI_X11)
#define NO_CONSOLE_INPUT 
#endif
typedef struct GuiScrollbar
{
long ident; 
win_T *wp; 
int type; 
long value; 
#if defined(FEAT_GUI_ATHENA)
int pixval; 
#endif
long size; 
long max; 
int top; 
int height; 
int width; 
int status_height; 
#if defined(FEAT_GUI_X11)
Widget id; 
#endif
#if defined(FEAT_GUI_GTK)
GtkWidget *id; 
unsigned long handler_id; 
#endif
#if defined(FEAT_GUI_MSWIN)
HWND id; 
int scroll_shift; 
#endif
#if FEAT_GUI_HAIKU
VimScrollBar *id; 
#endif
#if defined(FEAT_GUI_MAC)
ControlHandle id; 
#endif
#if defined(FEAT_GUI_PHOTON)
PtWidget_t *id;
#endif
} scrollbar_T;
typedef long guicolor_T; 
#define INVALCOLOR (guicolor_T)-11111 
#define CTERMCOLOR (guicolor_T)-11110 
#if defined(FEAT_GUI_GTK)
typedef PangoFontDescription *GuiFont; 
typedef PangoFontDescription *GuiFontset; 
#define NOFONT (GuiFont)NULL
#define NOFONTSET (GuiFontset)NULL
#else
#if defined(FEAT_GUI_PHOTON)
typedef char *GuiFont;
typedef char *GuiFontset;
#define NOFONT (GuiFont)NULL
#define NOFONTSET (GuiFontset)NULL
#else
#if defined(FEAT_GUI_X11)
typedef XFontStruct *GuiFont; 
typedef XFontSet GuiFontset; 
#define NOFONT (GuiFont)0
#define NOFONTSET (GuiFontset)0
#else
typedef long_u GuiFont; 
typedef long_u GuiFontset; 
#define NOFONT (GuiFont)0
#define NOFONTSET (GuiFontset)0
#endif
#endif
#endif
#if defined(VIMDLL)
#define GUI_MAY_SPAWN
#endif
typedef struct Gui
{
int in_focus; 
int in_use; 
int starting; 
int shell_created; 
int dying; 
int dofork; 
#if defined(GUI_MAY_SPAWN)
int dospawn; 
#endif
int dragged_sb; 
win_T *dragged_wp; 
int pointer_hidden; 
int col; 
int row; 
int cursor_col; 
int cursor_row; 
char cursor_is_valid; 
int num_cols; 
int num_rows; 
int scroll_region_top; 
int scroll_region_bot; 
int scroll_region_left; 
int scroll_region_right; 
int highlight_mask; 
int scrollbar_width; 
int scrollbar_height; 
int left_sbar_x; 
int right_sbar_x; 
#if defined(FEAT_MENU)
#if !defined(FEAT_GUI_GTK)
int menu_height; 
int menu_width; 
#endif
char menu_is_active; 
#if defined(FEAT_GUI_ATHENA)
char menu_height_fixed; 
#endif
#endif
scrollbar_T bottom_sbar; 
int which_scrollbars[3];
int prev_wrap; 
int char_width; 
int char_height; 
int char_ascent; 
int border_width; 
int border_offset; 
GuiFont norm_font; 
#if !defined(FEAT_GUI_GTK)
GuiFont bold_font; 
GuiFont ital_font; 
GuiFont boldital_font; 
#else
int font_can_bold; 
#endif
#if defined(FEAT_MENU) && !defined(FEAT_GUI_GTK)
#if defined(FONTSET_ALWAYS)
GuiFontset menu_fontset; 
#else
GuiFont menu_font; 
#endif
#endif
GuiFont wide_font; 
#if !defined(FEAT_GUI_GTK)
GuiFont wide_bold_font; 
GuiFont wide_ital_font; 
GuiFont wide_boldital_font; 
#endif
#if defined(FEAT_XFONTSET)
GuiFontset fontset; 
#endif
guicolor_T back_pixel; 
guicolor_T norm_pixel; 
guicolor_T def_back_pixel; 
guicolor_T def_norm_pixel; 
#if defined(FEAT_GUI_X11)
char *rsrc_menu_fg_name; 
guicolor_T menu_fg_pixel; 
char *rsrc_menu_bg_name; 
guicolor_T menu_bg_pixel; 
char *rsrc_scroll_fg_name; 
guicolor_T scroll_fg_pixel; 
char *rsrc_scroll_bg_name; 
guicolor_T scroll_bg_pixel; 
#if defined(FEAT_GUI_MOTIF)
guicolor_T menu_def_fg_pixel; 
guicolor_T menu_def_bg_pixel; 
guicolor_T scroll_def_fg_pixel; 
guicolor_T scroll_def_bg_pixel; 
#endif
Display *dpy; 
Window wid; 
int visibility; 
GC text_gc;
GC back_gc;
GC invert_gc;
Cursor blank_pointer; 
char_u *rsrc_font_name; 
char_u *rsrc_bold_font_name; 
char_u *rsrc_ital_font_name; 
char_u *rsrc_boldital_font_name; 
char_u *rsrc_menu_font_name; 
Bool rsrc_rev_video; 
char_u *geom; 
Bool color_approx; 
#endif
#if defined(FEAT_GUI_GTK)
#if !defined(USE_GTK3)
int visibility; 
#endif
GdkCursor *blank_pointer; 
char_u *geom; 
GtkWidget *mainwin; 
GtkWidget *formwin; 
GtkWidget *drawarea; 
#if defined(FEAT_MENU)
GtkWidget *menubar; 
#endif
#if defined(FEAT_TOOLBAR)
GtkWidget *toolbar; 
#endif
#if defined(FEAT_GUI_GNOME)
GtkWidget *menubar_h; 
GtkWidget *toolbar_h; 
#endif
#if defined(USE_GTK3)
GdkRGBA *fgcolor; 
GdkRGBA *bgcolor; 
GdkRGBA *spcolor; 
#else
GdkColor *fgcolor; 
GdkColor *bgcolor; 
GdkColor *spcolor; 
#endif
#if defined(USE_GTK3)
cairo_surface_t *surface; 
gboolean by_signal; 
#else
GdkGC *text_gc; 
#endif
PangoContext *text_context; 
PangoFont *ascii_font; 
PangoGlyphString *ascii_glyphs; 
#if defined(FEAT_GUI_TABLINE)
GtkWidget *tabline; 
#endif
GtkAccelGroup *accel_group;
GtkWidget *filedlg; 
char_u *browse_fname; 
guint32 event_time;
#endif 
#if defined(FEAT_GUI_TABLINE) && (defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_MAC) || defined(FEAT_GUI_HAIKU))
int tabline_height;
#endif
#if defined(FEAT_FOOTER)
int footer_height; 
#endif
#if defined(FEAT_TOOLBAR) && (defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_HAIKU))
int toolbar_height; 
#endif
#if defined(FEAT_BEVAL_TIP)
char_u *rsrc_tooltip_font_name; 
char *rsrc_tooltip_fg_name; 
char *rsrc_tooltip_bg_name; 
guicolor_T tooltip_fg_pixel; 
guicolor_T tooltip_bg_pixel; 
XFontSet tooltip_fontset; 
#endif
#if defined(FEAT_GUI_MSWIN)
GuiFont currFont; 
guicolor_T currFgColor; 
guicolor_T currBgColor; 
guicolor_T currSpColor; 
#endif
#if defined(FEAT_GUI_HAIKU)
VimApp *vimApp;
VimWindow *vimWindow;
VimFormView *vimForm;
VimTextAreaView *vimTextArea;
int vdcmp; 
#endif
#if defined(FEAT_GUI_MAC)
WindowPtr VimWindow;
MenuHandle MacOSHelpMenu; 
int MacOSHelpItems; 
WindowPtr wid; 
int visibility; 
#endif
#if defined(FEAT_GUI_PHOTON)
PtWidget_t *vimWindow; 
PtWidget_t *vimTextArea; 
PtWidget_t *vimContainer; 
#if defined(FEAT_MENU) || defined(FEAT_TOOLBAR)
PtWidget_t *vimToolBarGroup;
#endif
#if defined(FEAT_MENU)
PtWidget_t *vimMenuBar;
#endif
#if defined(FEAT_TOOLBAR)
PtWidget_t *vimToolBar;
int toolbar_height;
#endif
PhEvent_t *event_buffer;
#endif
#if defined(FEAT_XIM)
char *rsrc_input_method;
char *rsrc_preedit_type_name;
#endif
} gui_T;
extern gui_T gui; 
typedef enum
{
VW_POS_MOUSE,
VW_POS_CENTER,
VW_POS_TOP_CENTER
} gui_win_pos_T;
#if defined(FIND_REPLACE_DIALOG)
#define FRD_FINDNEXT 1 
#define FRD_R_FINDNEXT 2 
#define FRD_REPLACE 3 
#define FRD_REPLACEALL 4 
#define FRD_UNDO 5 
#define FRD_TYPE_MASK 7 
#define FRD_WHOLE_WORD 0x08 
#define FRD_MATCH_CASE 0x10 
#endif
#if defined(FEAT_GUI_GTK)
#define CONVERT_TO_UTF8(String) ((output_conv.vc_type == CONV_NONE || (String) == NULL) ? (String) : string_convert(&output_conv, (String), NULL))
#define CONVERT_TO_UTF8_FREE(String) ((String) = ((output_conv.vc_type == CONV_NONE) ? (char_u *)NULL : (vim_free(String), (char_u *)NULL)))
#define CONVERT_FROM_UTF8(String) ((input_conv.vc_type == CONV_NONE || (String) == NULL) ? (String) : string_convert(&input_conv, (String), NULL))
#define CONVERT_FROM_UTF8_FREE(String) ((String) = ((input_conv.vc_type == CONV_NONE) ? (char_u *)NULL : (vim_free(String), (char_u *)NULL)))
#else
#define CONVERT_TO_UTF8(String) (String)
#define CONVERT_TO_UTF8_FREE(String) ((String) = (char_u *)NULL)
#define CONVERT_FROM_UTF8(String) (String)
#define CONVERT_FROM_UTF8_FREE(String) ((String) = (char_u *)NULL)
#endif 
#if defined(FEAT_GUI_GTK)
#if defined(__SUNPRO_C) && defined(USE_GTK3)
#define FUNC2GENERIC(func) (void *)(func)
#else
#define FUNC2GENERIC(func) G_CALLBACK(func)
#endif
#endif 
#if defined(UNIX) && !defined(FEAT_GUI_MAC)
#define GUI_MAY_FORK
#endif
