


























#include "vim.h"
#if defined(USE_GRESOURCE)
#include "auto/gui_gtk_gresources.h"
#endif

#if defined(FEAT_GUI_GNOME)

#if defined(_)
#undef _
#endif
#if defined(N_)
#undef N_
#endif
#if defined(textdomain)
#undef textdomain
#endif
#if defined(bindtextdomain)
#undef bindtextdomain
#endif
#if defined(bind_textdomain_codeset)
#undef bind_textdomain_codeset
#endif
#if defined(FEAT_GETTEXT) && !defined(ENABLE_NLS)
#define ENABLE_NLS 
#endif
#include <gnome.h>
#include "version.h"

extern void bonobo_dock_item_set_behavior(BonoboDockItem *dock_item, BonoboDockItemBehavior beh);
#endif

#if !defined(FEAT_GUI_GTK) && defined(PROTO)

#define GdkAtom int
#define GdkEventExpose int
#define GdkEventFocus int
#define GdkEventVisibility int
#define GdkEventProperty int
#define GtkContainer int
#define GtkTargetEntry int
#define GtkType int
#define GtkWidget int
#define gint int
#define gpointer int
#define guint int
#define GdkEventKey int
#define GdkEventSelection int
#define GtkSelectionData int
#define GdkEventMotion int
#define GdkEventButton int
#define GdkDragContext int
#define GdkEventConfigure int
#define GdkEventClient int
#else
#if GTK_CHECK_VERSION(3,0,0)
#include <gdk/gdkkeysyms-compat.h>
#include <gtk/gtkx.h>
#else
#include <gdk/gdkkeysyms.h>
#endif
#include <gdk/gdk.h>
#if defined(MSWIN)
#include <gdk/gdkwin32.h>
#else
#include <gdk/gdkx.h>
#endif
#include <gtk/gtk.h>
#include "gui_gtk_f.h"
#endif

#if defined(HAVE_X11_SUNKEYSYM_H)
#include <X11/Sunkeysym.h>
#endif




#define GET_X_ATOM(atom) gdk_x11_atom_to_xatom_for_display( gtk_widget_get_display(gui.mainwin), atom)



enum
{
TARGET_TYPE_NONE,
TARGET_UTF8_STRING,
TARGET_STRING,
TARGET_COMPOUND_TEXT,
TARGET_HTML,
TARGET_TEXT,
TARGET_TEXT_URI_LIST,
TARGET_TEXT_PLAIN,
TARGET_VIM,
TARGET_VIMENC
};





static const GtkTargetEntry selection_targets[] =
{
{VIMENC_ATOM_NAME, 0, TARGET_VIMENC},
{VIM_ATOM_NAME, 0, TARGET_VIM},
{"text/html", 0, TARGET_HTML},
{"UTF8_STRING", 0, TARGET_UTF8_STRING},
{"COMPOUND_TEXT", 0, TARGET_COMPOUND_TEXT},
{"TEXT", 0, TARGET_TEXT},
{"STRING", 0, TARGET_STRING}
};
#define N_SELECTION_TARGETS (sizeof(selection_targets) / sizeof(selection_targets[0]))

#if defined(FEAT_DND)




static const GtkTargetEntry dnd_targets[] =
{
{"text/uri-list", 0, TARGET_TEXT_URI_LIST},
{"text/html", 0, TARGET_HTML},
{"UTF8_STRING", 0, TARGET_UTF8_STRING},
{"STRING", 0, TARGET_STRING},
{"text/plain", 0, TARGET_TEXT_PLAIN}
};
#define N_DND_TARGETS (sizeof(dnd_targets) / sizeof(dnd_targets[0]))
#endif






#define DEFAULT_FONT "Monospace 10"

#if defined(FEAT_GUI_GNOME) && defined(FEAT_SESSION)
#define USE_GNOME_SESSION
#endif

#if !defined(FEAT_GUI_GNOME)




static GdkAtom wm_protocols_atom = GDK_NONE;
static GdkAtom save_yourself_atom = GDK_NONE;
#endif




static GdkAtom html_atom = GDK_NONE;
static GdkAtom utf8_string_atom = GDK_NONE;
static GdkAtom vim_atom = GDK_NONE; 
static GdkAtom vimenc_atom = GDK_NONE; 






static struct special_key
{
guint key_sym;
char_u code0;
char_u code1;
}
const special_keys[] =
{
{GDK_Up, 'k', 'u'},
{GDK_Down, 'k', 'd'},
{GDK_Left, 'k', 'l'},
{GDK_Right, 'k', 'r'},
{GDK_F1, 'k', '1'},
{GDK_F2, 'k', '2'},
{GDK_F3, 'k', '3'},
{GDK_F4, 'k', '4'},
{GDK_F5, 'k', '5'},
{GDK_F6, 'k', '6'},
{GDK_F7, 'k', '7'},
{GDK_F8, 'k', '8'},
{GDK_F9, 'k', '9'},
{GDK_F10, 'k', ';'},
{GDK_F11, 'F', '1'},
{GDK_F12, 'F', '2'},
{GDK_F13, 'F', '3'},
{GDK_F14, 'F', '4'},
{GDK_F15, 'F', '5'},
{GDK_F16, 'F', '6'},
{GDK_F17, 'F', '7'},
{GDK_F18, 'F', '8'},
{GDK_F19, 'F', '9'},
{GDK_F20, 'F', 'A'},
{GDK_F21, 'F', 'B'},
{GDK_Pause, 'F', 'B'}, 
{GDK_F22, 'F', 'C'},
{GDK_F23, 'F', 'D'},
{GDK_F24, 'F', 'E'},
{GDK_F25, 'F', 'F'},
{GDK_F26, 'F', 'G'},
{GDK_F27, 'F', 'H'},
{GDK_F28, 'F', 'I'},
{GDK_F29, 'F', 'J'},
{GDK_F30, 'F', 'K'},
{GDK_F31, 'F', 'L'},
{GDK_F32, 'F', 'M'},
{GDK_F33, 'F', 'N'},
{GDK_F34, 'F', 'O'},
{GDK_F35, 'F', 'P'},
#if defined(SunXK_F36)
{SunXK_F36, 'F', 'Q'},
{SunXK_F37, 'F', 'R'},
#endif
{GDK_Help, '%', '1'},
{GDK_Undo, '&', '8'},
{GDK_BackSpace, 'k', 'b'},
{GDK_Insert, 'k', 'I'},
{GDK_Delete, 'k', 'D'},
{GDK_3270_BackTab, 'k', 'B'},
{GDK_Clear, 'k', 'C'},
{GDK_Home, 'k', 'h'},
{GDK_End, '@', '7'},
{GDK_Prior, 'k', 'P'},
{GDK_Next, 'k', 'N'},
{GDK_Print, '%', '9'},

{GDK_KP_Left, 'k', 'l'},
{GDK_KP_Right, 'k', 'r'},
{GDK_KP_Up, 'k', 'u'},
{GDK_KP_Down, 'k', 'd'},
{GDK_KP_Insert, KS_EXTRA, (char_u)KE_KINS},
{GDK_KP_Delete, KS_EXTRA, (char_u)KE_KDEL},
{GDK_KP_Home, 'K', '1'},
{GDK_KP_End, 'K', '4'},
{GDK_KP_Prior, 'K', '3'}, 
{GDK_KP_Next, 'K', '5'}, 

{GDK_KP_Add, 'K', '6'},
{GDK_KP_Subtract, 'K', '7'},
{GDK_KP_Divide, 'K', '8'},
{GDK_KP_Multiply, 'K', '9'},
{GDK_KP_Enter, 'K', 'A'},
{GDK_KP_Decimal, 'K', 'B'},

{GDK_KP_0, 'K', 'C'},
{GDK_KP_1, 'K', 'D'},
{GDK_KP_2, 'K', 'E'},
{GDK_KP_3, 'K', 'F'},
{GDK_KP_4, 'K', 'G'},
{GDK_KP_5, 'K', 'H'},
{GDK_KP_6, 'K', 'I'},
{GDK_KP_7, 'K', 'J'},
{GDK_KP_8, 'K', 'K'},
{GDK_KP_9, 'K', 'L'},


{0, 0, 0}
};




#define ARG_FONT 1
#define ARG_GEOMETRY 2
#define ARG_REVERSE 3
#define ARG_NOREVERSE 4
#define ARG_BACKGROUND 5
#define ARG_FOREGROUND 6
#define ARG_ICONIC 7
#define ARG_ROLE 8
#define ARG_NETBEANS 9
#define ARG_XRM 10 
#define ARG_MENUFONT 11 
#define ARG_INDEX_MASK 0x00ff
#define ARG_HAS_VALUE 0x0100 
#define ARG_NEEDS_GUI 0x0200 
#define ARG_FOR_GTK 0x0400 
#define ARG_COMPAT_LONG 0x0800 
#define ARG_KEEP 0x1000 








typedef struct
{
const char *name;
unsigned int flags;
}
cmdline_option_T;

static const cmdline_option_T cmdline_options[] =
{

{"-fn", ARG_FONT|ARG_HAS_VALUE},
{"-font", ARG_FONT|ARG_HAS_VALUE},
{"-geom", ARG_GEOMETRY|ARG_HAS_VALUE},
{"-geometry", ARG_GEOMETRY|ARG_HAS_VALUE},
{"-rv", ARG_REVERSE},
{"-reverse", ARG_REVERSE},
{"+rv", ARG_NOREVERSE},
{"+reverse", ARG_NOREVERSE},
{"-bg", ARG_BACKGROUND|ARG_HAS_VALUE},
{"-background", ARG_BACKGROUND|ARG_HAS_VALUE},
{"-fg", ARG_FOREGROUND|ARG_HAS_VALUE},
{"-foreground", ARG_FOREGROUND|ARG_HAS_VALUE},
{"-iconic", ARG_ICONIC},
{"--role", ARG_ROLE|ARG_HAS_VALUE},
#if defined(FEAT_NETBEANS_INTG)
{"-nb", ARG_NETBEANS}, 
{"-xrm", ARG_XRM|ARG_HAS_VALUE}, 
{"-mf", ARG_MENUFONT|ARG_HAS_VALUE}, 
{"-menufont", ARG_MENUFONT|ARG_HAS_VALUE}, 
#endif

{"--g-fatal-warnings", ARG_FOR_GTK},
{"--gdk-debug", ARG_FOR_GTK|ARG_HAS_VALUE},
{"--gdk-no-debug", ARG_FOR_GTK|ARG_HAS_VALUE},
{"--gtk-debug", ARG_FOR_GTK|ARG_HAS_VALUE},
{"--gtk-no-debug", ARG_FOR_GTK|ARG_HAS_VALUE},
{"--gtk-module", ARG_FOR_GTK|ARG_HAS_VALUE},
{"--sync", ARG_FOR_GTK},
{"--display", ARG_FOR_GTK|ARG_HAS_VALUE|ARG_COMPAT_LONG},
{"--name", ARG_FOR_GTK|ARG_HAS_VALUE|ARG_COMPAT_LONG},
{"--class", ARG_FOR_GTK|ARG_HAS_VALUE|ARG_COMPAT_LONG},
{"--screen", ARG_FOR_GTK|ARG_HAS_VALUE},
{"--gxid-host", ARG_FOR_GTK|ARG_HAS_VALUE},
{"--gxid-port", ARG_FOR_GTK|ARG_HAS_VALUE},
#if defined(FEAT_GUI_GNOME)
{"--load-modules", ARG_FOR_GTK|ARG_HAS_VALUE},
{"--sm-client-id", ARG_FOR_GTK|ARG_HAS_VALUE},
{"--sm-config-prefix", ARG_FOR_GTK|ARG_HAS_VALUE},
{"--sm-disable", ARG_FOR_GTK},
{"--oaf-ior-fd", ARG_FOR_GTK|ARG_HAS_VALUE},
{"--oaf-activate-iid", ARG_FOR_GTK|ARG_HAS_VALUE},
{"--oaf-private", ARG_FOR_GTK},
{"--enable-sound", ARG_FOR_GTK},
{"--disable-sound", ARG_FOR_GTK},
{"--espeaker", ARG_FOR_GTK|ARG_HAS_VALUE},
{"-?", ARG_FOR_GTK|ARG_NEEDS_GUI},
{"--help", ARG_FOR_GTK|ARG_NEEDS_GUI|ARG_KEEP},
{"--usage", ARG_FOR_GTK|ARG_NEEDS_GUI},
#if 0 
{"--version", ARG_FOR_GTK|ARG_NEEDS_GUI},
#endif
{"--disable-crash-dialog", ARG_FOR_GTK},
#endif
{NULL, 0}
};

static int gui_argc = 0;
static char **gui_argv = NULL;

static const char *role_argument = NULL;
#if defined(USE_GNOME_SESSION)
static const char *restart_command = NULL;
static char *abs_restart_command = NULL;
#endif
static int found_iconic_arg = FALSE;

#if defined(FEAT_GUI_GNOME)



static int using_gnome = 0;
#else
#define using_gnome 0
#endif






void
gui_mch_prepare(int *argc, char **argv)
{
const cmdline_option_T *option;
int i = 0;
int len = 0;

#if defined(USE_GNOME_SESSION)





restart_command = argv[0];

if (strchr(argv[0], G_DIR_SEPARATOR) != NULL)
{
char_u buf[MAXPATHL];

if (mch_FullName((char_u *)argv[0], buf, (int)sizeof(buf), TRUE) == OK)
{
abs_restart_command = (char *)vim_strsave(buf);
restart_command = abs_restart_command;
}
}
#endif





gui_argc = 0;
gui_argv = ALLOC_MULT(char *, *argc + 1);

g_return_if_fail(gui_argv != NULL);

gui_argv[gui_argc++] = argv[i++];

while (i < *argc)
{

if (argv[i][0] != '-' && argv[i][0] != '+')
{
++i;
continue;
}


for (option = &cmdline_options[0]; option->name != NULL; ++option)
{
len = strlen(option->name);

if (strncmp(argv[i], option->name, len) == 0)
{
if (argv[i][len] == '\0')
break;

if (argv[i][len] == '=' && (option->flags & ARG_HAS_VALUE))
break;
#if defined(FEAT_NETBEANS_INTG)

if (vim_strchr((char_u *)":=", argv[i][len]) != NULL
&& (option->flags & ARG_INDEX_MASK) == ARG_NETBEANS)
break;
#endif
}
else if ((option->flags & ARG_COMPAT_LONG)
&& strcmp(argv[i], option->name + 1) == 0)
{


argv[i] = (char *)option->name;
break;
}
}
if (option->name == NULL) 
{
++i;
continue;
}

if (option->flags & ARG_FOR_GTK)
{


gui_argv[gui_argc++] = argv[i];
}
else
{
char *value = NULL;



if (option->flags & ARG_HAS_VALUE)
{
if (argv[i][len] == '=')
value = &argv[i][len + 1];
else if (i + 1 < *argc && strcmp(argv[i + 1], "--") != 0)
value = argv[i + 1];
}


switch (option->flags & ARG_INDEX_MASK)
{
case ARG_REVERSE:
found_reverse_arg = TRUE;
break;
case ARG_NOREVERSE:
found_reverse_arg = FALSE;
break;
case ARG_FONT:
font_argument = value;
break;
case ARG_GEOMETRY:
if (value != NULL)
gui.geom = vim_strsave((char_u *)value);
break;
case ARG_BACKGROUND:
background_argument = value;
break;
case ARG_FOREGROUND:
foreground_argument = value;
break;
case ARG_ICONIC:
found_iconic_arg = TRUE;
break;
case ARG_ROLE:
role_argument = value; 
break;
#if defined(FEAT_NETBEANS_INTG)
case ARG_NETBEANS:
gui.dofork = FALSE; 
netbeansArg = argv[i];
break;
#endif
default:
break;
}
}




if ((option->flags & ARG_NEEDS_GUI)
&& gui_mch_early_init_check(FALSE) == OK)
gui.starting = TRUE;

if (option->flags & ARG_KEEP)
++i;
else
{

if (--*argc > i)
{
int n_strip = 1;


if ((option->flags & ARG_HAS_VALUE)
&& argv[i][len] != '='
&& strcmp(argv[i + 1], "--") != 0)
{
++n_strip;
--*argc;
if (option->flags & ARG_FOR_GTK)
gui_argv[gui_argc++] = argv[i + 1];
}

if (*argc > i)
mch_memmove(&argv[i], &argv[i + n_strip],
(*argc - i) * sizeof(char *));
}
argv[*argc] = NULL;
}
}

gui_argv[gui_argc] = NULL;
}

#if defined(EXITFREE) || defined(PROTO)
void
gui_mch_free_all(void)
{
vim_free(gui_argv);
#if defined(USE_GNOME_SESSION)
vim_free(abs_restart_command);
#endif
}
#endif

#if !GTK_CHECK_VERSION(3,0,0)





static gint
visibility_event(GtkWidget *widget UNUSED,
GdkEventVisibility *event,
gpointer data UNUSED)
{
gui.visibility = event->state;





if (gui.text_gc != NULL)
gdk_gc_set_exposures(gui.text_gc,
gui.visibility != GDK_VISIBILITY_UNOBSCURED);
return FALSE;
}
#endif 




#if GTK_CHECK_VERSION(3,0,0)
static gboolean is_key_pressed = FALSE;
static gboolean blink_mode = TRUE;

static gboolean gui_gtk_is_blink_on(void);

static void
gui_gtk3_redraw(int x, int y, int width, int height)
{

gui_redraw_block(Y_2_ROW(y), X_2_COL(x),
Y_2_ROW(y + height - 1), X_2_COL(x + width - 1),
GUI_MON_NOCLEAR);
}

static void
gui_gtk3_update_cursor(cairo_t *cr)
{
if (gui.row == gui.cursor_row)
{
gui.by_signal = TRUE;
if (State & CMDLINE)
gui_update_cursor(TRUE, FALSE);
else
gui_update_cursor(TRUE, TRUE);
gui.by_signal = FALSE;
cairo_paint(cr);
}
}

static gboolean
gui_gtk3_should_draw_cursor(void)
{
unsigned int cond = 0;
cond |= gui_gtk_is_blink_on();
if (gui.cursor_col >= gui.col)
cond |= is_key_pressed;
cond |= gui.in_focus == FALSE;
return cond;
}

static gboolean
draw_event(GtkWidget *widget UNUSED,
cairo_t *cr,
gpointer user_data UNUSED)
{

if (gui.starting)
return FALSE;

out_flush(); 


cairo_set_source_surface(cr, gui.surface, 0, 0);


gui.by_signal = TRUE;
{
cairo_rectangle_list_t *list = NULL;

list = cairo_copy_clip_rectangle_list(cr);
if (list->status != CAIRO_STATUS_CLIP_NOT_REPRESENTABLE)
{
int i;



for (i = 0; i < list->num_rectangles; i++)
{
const cairo_rectangle_t rect = list->rectangles[i];

gui_mch_clear_block(Y_2_ROW((int)rect.y), 0,
Y_2_ROW((int)(rect.y + rect.height)) - 1, Columns - 1);
}

for (i = 0; i < list->num_rectangles; i++)
{
const cairo_rectangle_t rect = list->rectangles[i];

if (blink_mode)
gui_gtk3_redraw(rect.x, rect.y, rect.width, rect.height);
else
{
if (get_real_state() & VISUAL)
gui_gtk3_redraw(rect.x, rect.y,
rect.width, rect.height);
else
gui_redraw(rect.x, rect.y, rect.width, rect.height);
}
}
}
cairo_rectangle_list_destroy(list);

if (get_real_state() & VISUAL)
{
if (gui.cursor_row == gui.row && gui.cursor_col >= gui.col)
gui_update_cursor(TRUE, TRUE);
}

cairo_paint(cr);
}
gui.by_signal = FALSE;


if (gui_gtk3_should_draw_cursor() && blink_mode)
gui_gtk3_update_cursor(cr);

return FALSE;
}
#else 
static gint
expose_event(GtkWidget *widget UNUSED,
GdkEventExpose *event,
gpointer data UNUSED)
{

if (gui.starting)
return FALSE;

out_flush(); 
gui_redraw(event->area.x, event->area.y,
event->area.width, event->area.height);


if (event->area.x < FILL_X(0))
gdk_window_clear_area(gui.drawarea->window, 0, 0, FILL_X(0), 0);
if (event->area.y < FILL_Y(0))
gdk_window_clear_area(gui.drawarea->window, 0, 0, 0, FILL_Y(0));
if (event->area.x > FILL_X(Columns))
gdk_window_clear_area(gui.drawarea->window,
FILL_X((int)Columns), 0, 0, 0);
if (event->area.y > FILL_Y(Rows))
gdk_window_clear_area(gui.drawarea->window, 0, FILL_Y((int)Rows), 0, 0);

return FALSE;
}
#endif 

#if defined(FEAT_CLIENTSERVER)



static gint
property_event(GtkWidget *widget,
GdkEventProperty *event,
gpointer data UNUSED)
{
if (event->type == GDK_PROPERTY_NOTIFY
&& event->state == (int)GDK_PROPERTY_NEW_VALUE
&& GDK_WINDOW_XID(event->window) == commWindow
&& GET_X_ATOM(event->atom) == commProperty)
{
XEvent xev;


xev.xproperty.type = PropertyNotify;
xev.xproperty.atom = commProperty;
xev.xproperty.window = commWindow;
xev.xproperty.state = PropertyNewValue;
serverEventProc(GDK_WINDOW_XDISPLAY(gtk_widget_get_window(widget)),
&xev, 0);
}
return FALSE;
}
#endif 




static void
gtk_settings_xft_dpi_changed_cb(GtkSettings *gtk_settings UNUSED,
GParamSpec *pspec UNUSED,
gpointer data UNUSED)
{


if (gui.text_context != NULL)
g_object_unref(gui.text_context);

gui.text_context = gtk_widget_create_pango_context(gui.mainwin);
pango_context_set_base_dir(gui.text_context, PANGO_DIRECTION_LTR);

if (gui.norm_font != NULL)
{

gui_mch_init_font(*p_guifont == NUL ? NULL : p_guifont, FALSE);
gui_set_shellsize(TRUE, FALSE, RESIZE_BOTH);
}
}

typedef gboolean timeout_cb_type;





static guint
timeout_add(int time, timeout_cb_type (*callback)(gpointer), int *flagp)
{
return g_timeout_add((guint)time, (GSourceFunc)callback, flagp);
}

static void
timeout_remove(guint timer)
{
g_source_remove(timer);
}













#define BLINK_NONE 0
#define BLINK_OFF 1
#define BLINK_ON 2

static int blink_state = BLINK_NONE;
static long_u blink_waittime = 700;
static long_u blink_ontime = 400;
static long_u blink_offtime = 250;
static guint blink_timer = 0;

#if GTK_CHECK_VERSION(3,0,0)
static gboolean
gui_gtk_is_blink_on(void)
{
return blink_state == BLINK_ON;
}
#endif

int
gui_mch_is_blinking(void)
{
return blink_state != BLINK_NONE;
}

int
gui_mch_is_blink_off(void)
{
return blink_state == BLINK_OFF;
}

void
gui_mch_set_blinking(long waittime, long on, long off)
{
#if GTK_CHECK_VERSION(3,0,0)
if (waittime == 0 || on == 0 || off == 0)
{
blink_mode = FALSE;

blink_waittime = 700;
blink_ontime = 400;
blink_offtime = 250;
}
else
{
blink_mode = TRUE;

blink_waittime = waittime;
blink_ontime = on;
blink_offtime = off;
}
#else
blink_waittime = waittime;
blink_ontime = on;
blink_offtime = off;
#endif
}




void
gui_mch_stop_blink(int may_call_gui_update_cursor)
{
if (blink_timer)
{
timeout_remove(blink_timer);
blink_timer = 0;
}
if (blink_state == BLINK_OFF && may_call_gui_update_cursor)
{
gui_update_cursor(TRUE, FALSE);
gui_mch_flush();
}
blink_state = BLINK_NONE;
}

static timeout_cb_type
blink_cb(gpointer data UNUSED)
{
if (blink_state == BLINK_ON)
{
gui_undraw_cursor();
blink_state = BLINK_OFF;
blink_timer = timeout_add(blink_offtime, blink_cb, NULL);
}
else
{
gui_update_cursor(TRUE, FALSE);
blink_state = BLINK_ON;
blink_timer = timeout_add(blink_ontime, blink_cb, NULL);
}
gui_mch_flush();

return FALSE; 
}





void
gui_mch_start_blink(void)
{
if (blink_timer)
{
timeout_remove(blink_timer);
blink_timer = 0;
}

if (blink_waittime && blink_ontime && blink_offtime && gui.in_focus)
{
blink_timer = timeout_add(blink_waittime, blink_cb, NULL);
blink_state = BLINK_ON;
gui_update_cursor(TRUE, FALSE);
gui_mch_flush();
}
}

static gint
enter_notify_event(GtkWidget *widget UNUSED,
GdkEventCrossing *event UNUSED,
gpointer data UNUSED)
{
if (blink_state == BLINK_NONE)
gui_mch_start_blink();


if (gtk_socket_id == 0 || !gtk_widget_has_focus(gui.drawarea))
gtk_widget_grab_focus(gui.drawarea);

return FALSE;
}

static gint
leave_notify_event(GtkWidget *widget UNUSED,
GdkEventCrossing *event UNUSED,
gpointer data UNUSED)
{
if (blink_state != BLINK_NONE)
gui_mch_stop_blink(TRUE);

return FALSE;
}

static gint
focus_in_event(GtkWidget *widget,
GdkEventFocus *event UNUSED,
gpointer data UNUSED)
{
gui_focus_change(TRUE);

if (blink_state == BLINK_NONE)
gui_mch_start_blink();



if (widget != gui.drawarea)
gtk_widget_grab_focus(gui.drawarea);

return TRUE;
}

static gint
focus_out_event(GtkWidget *widget UNUSED,
GdkEventFocus *event UNUSED,
gpointer data UNUSED)
{
gui_focus_change(FALSE);

if (blink_state != BLINK_NONE)
gui_mch_stop_blink(TRUE);

return TRUE;
}











static int
keyval_to_string(unsigned int keyval, unsigned int state, char_u *string)
{
int len;
guint32 uc;

uc = gdk_keyval_to_unicode(keyval);
if (uc != 0)
{

if ((state & GDK_CONTROL_MASK) && uc >= 0x20 && uc < 0x80)
{




if (uc >= '@')
string[0] = uc & 0x1F;
else if (uc == '2')
string[0] = NUL;
else if (uc >= '3' && uc <= '7')
string[0] = uc ^ 0x28;
else if (uc == '8')
string[0] = BS;
else if (uc == '?')
string[0] = DEL;
else
string[0] = uc;
len = 1;
}
else
{


len = utf_char2bytes((int)uc, string);
}
}
else
{



len = 1;
switch (keyval)
{
case GDK_Tab: case GDK_KP_Tab: case GDK_ISO_Left_Tab:
string[0] = TAB;
break;
case GDK_Linefeed:
string[0] = NL;
break;
case GDK_Return: case GDK_ISO_Enter: case GDK_3270_Enter:
string[0] = CAR;
break;
case GDK_Escape:
string[0] = ESC;
break;
default:
len = 0;
break;
}
}
string[len] = NUL;

return len;
}

static int
modifiers_gdk2vim(guint state)
{
int modifiers = 0;

if (state & GDK_SHIFT_MASK)
modifiers |= MOD_MASK_SHIFT;
if (state & GDK_CONTROL_MASK)
modifiers |= MOD_MASK_CTRL;
if (state & GDK_MOD1_MASK)
modifiers |= MOD_MASK_ALT;
#if GTK_CHECK_VERSION(2,10,0)
if (state & GDK_SUPER_MASK)
modifiers |= MOD_MASK_META;
#endif
if (state & GDK_MOD4_MASK)
modifiers |= MOD_MASK_META;

return modifiers;
}

static int
modifiers_gdk2mouse(guint state)
{
int modifiers = 0;

if (state & GDK_SHIFT_MASK)
modifiers |= MOUSE_SHIFT;
if (state & GDK_CONTROL_MASK)
modifiers |= MOUSE_CTRL;
if (state & GDK_MOD1_MASK)
modifiers |= MOUSE_ALT;

return modifiers;
}




static gint
key_press_event(GtkWidget *widget UNUSED,
GdkEventKey *event,
gpointer data UNUSED)
{


char_u string[32], string2[32];
guint key_sym;
int len;
int i;
int modifiers;
int key;
guint state;
char_u *s, *d;

#if GTK_CHECK_VERSION(3,0,0)
is_key_pressed = TRUE;
gui_mch_stop_blink(TRUE);
#endif

gui.event_time = event->time;
key_sym = event->keyval;
state = event->state;

#if defined(FEAT_XIM)
if (xim_queue_key_press_event(event, TRUE))
return TRUE;
#endif

#if defined(SunXK_F36)





if (key_sym == SunXK_F36 || key_sym == SunXK_F37)
len = 0;
else
#endif
{
len = keyval_to_string(key_sym, state, string2);



if (len > 1 && input_conv.vc_type != CONV_NONE)
len = convert_input(string2, len, sizeof(string2));

s = string2;
d = string;
for (i = 0; i < len; ++i)
{
*d++ = s[i];
if (d[-1] == CSI && d + 2 < string + sizeof(string))
{

*d++ = KS_EXTRA;
*d++ = (int)KE_CSI;
}
}
len = d - string;
}


if (key_sym == GDK_ISO_Left_Tab)
{
key_sym = GDK_Tab;
state |= GDK_SHIFT_MASK;
}

#if defined(FEAT_MENU)


if ((state & GDK_MOD1_MASK)
&& gui.menu_is_active
&& (*p_wak == 'y'
|| (*p_wak == 'm'
&& len == 1
&& gui_is_menu_shortcut(string[0]))))


return FALSE;
#endif






if (len == 1
&& ((state & GDK_MOD1_MASK)
#if GTK_CHECK_VERSION(2,10,0)
|| (state & GDK_SUPER_MASK)
#endif
)
&& !(key_sym == GDK_BackSpace || key_sym == GDK_Delete)
&& (string[0] & 0x80) == 0
&& !(key_sym == GDK_Tab && (state & GDK_SHIFT_MASK))
&& !enc_dbcs
)
{
string[0] |= 0x80;
state &= ~GDK_MOD1_MASK; 
if (enc_utf8) 
{
string[1] = string[0] & 0xbf;
string[0] = ((unsigned)string[0] >> 6) + 0xc0;
if (string[1] == CSI)
{
string[2] = KS_EXTRA;
string[3] = (int)KE_CSI;
len = 4;
}
else
len = 2;
}
}



if (len == 0 || len == 1)
{
for (i = 0; special_keys[i].key_sym != 0; i++)
{
if (special_keys[i].key_sym == key_sym)
{
string[0] = CSI;
string[1] = special_keys[i].code0;
string[2] = special_keys[i].code1;
len = -3;
break;
}
}
}

if (len == 0) 
return TRUE;



if (len == -3 || key_sym == GDK_space || key_sym == GDK_Tab
|| key_sym == GDK_Return || key_sym == GDK_Linefeed
|| key_sym == GDK_Escape || key_sym == GDK_KP_Tab
|| key_sym == GDK_ISO_Enter || key_sym == GDK_3270_Enter
|| (enc_dbcs && len == 1 && ((state & GDK_MOD1_MASK)
#if GTK_CHECK_VERSION(2,10,0)
|| (state & GDK_SUPER_MASK)
#endif
)))
{
modifiers = modifiers_gdk2vim(state);





if (len == -3)
key = TO_SPECIAL(string[1], string[2]);
else
key = string[0];

key = simplify_key(key, &modifiers);
if (key == CSI)
key = K_CSI;
if (IS_SPECIAL(key))
{
string[0] = CSI;
string[1] = K_SECOND(key);
string[2] = K_THIRD(key);
len = 3;
}
else
{
string[0] = key;
len = 1;
}

if (modifiers != 0)
{
string2[0] = CSI;
string2[1] = KS_MODIFIER;
string2[2] = modifiers;
add_to_input_buf(string2, 3);
}
}

if (len == 1 && ((string[0] == Ctrl_C && ctrl_c_interrupts)
|| (string[0] == intr_char && intr_char != Ctrl_C)))
{
trash_input_buf();
got_int = TRUE;
}

add_to_input_buf(string, len);


if (p_mh)
gui_mch_mousehide(TRUE);

return TRUE;
}

#if defined(FEAT_XIM) || GTK_CHECK_VERSION(3,0,0)
static gboolean
key_release_event(GtkWidget *widget UNUSED,
GdkEventKey *event,
gpointer data UNUSED)
{
#if GTK_CHECK_VERSION(3,0,0)
is_key_pressed = FALSE;
gui_mch_start_blink();
#endif
#if defined(FEAT_XIM)
gui.event_time = event->time;





return xim_queue_key_press_event(event, FALSE);
#else
return TRUE;
#endif
}
#endif







static int in_selection_clear_event = FALSE;

static gint
selection_clear_event(GtkWidget *widget UNUSED,
GdkEventSelection *event,
gpointer user_data UNUSED)
{
in_selection_clear_event = TRUE;
if (event->selection == clip_plus.gtk_sel_atom)
clip_lose_selection(&clip_plus);
else
clip_lose_selection(&clip_star);
in_selection_clear_event = FALSE;

return TRUE;
}

#define RS_NONE 0 
#define RS_OK 1 
#define RS_FAIL 2 
static int received_selection = RS_NONE;

static void
selection_received_cb(GtkWidget *widget UNUSED,
GtkSelectionData *data,
guint time_ UNUSED,
gpointer user_data UNUSED)
{
Clipboard_T *cbd;
char_u *text;
char_u *tmpbuf = NULL;
guchar *tmpbuf_utf8 = NULL;
int len;
int motion_type = MAUTO;

if (gtk_selection_data_get_selection(data) == clip_plus.gtk_sel_atom)
cbd = &clip_plus;
else
cbd = &clip_star;

text = (char_u *)gtk_selection_data_get_data(data);
len = gtk_selection_data_get_length(data);

if (text == NULL || len <= 0)
{
received_selection = RS_FAIL;


return;
}

if (gtk_selection_data_get_data_type(data) == vim_atom)
{
motion_type = *text++;
--len;
}
else if (gtk_selection_data_get_data_type(data) == vimenc_atom)
{
char_u *enc;
vimconv_T conv;

motion_type = *text++;
--len;

enc = text;
text += STRLEN(text) + 1;
len -= text - enc;



conv.vc_type = CONV_NONE;
convert_setup(&conv, enc, p_enc);
if (conv.vc_type != CONV_NONE)
{
tmpbuf = string_convert(&conv, text, &len);
if (tmpbuf != NULL)
text = tmpbuf;
convert_setup(&conv, NULL, NULL);
}
}



else
{
tmpbuf_utf8 = gtk_selection_data_get_text(data);
if (tmpbuf_utf8 != NULL)
{
len = STRLEN(tmpbuf_utf8);
if (input_conv.vc_type != CONV_NONE)
{
tmpbuf = string_convert(&input_conv, tmpbuf_utf8, &len);
if (tmpbuf != NULL)
text = tmpbuf;
}
else
text = tmpbuf_utf8;
}
else if (len >= 2 && text[0] == 0xff && text[1] == 0xfe)
{
vimconv_T conv;


conv.vc_type = CONV_NONE;
convert_setup_ext(&conv, (char_u *)"utf-16le", FALSE, p_enc, TRUE);

if (conv.vc_type != CONV_NONE)
{
text += 2;
len -= 2;
tmpbuf = string_convert(&conv, text, &len);
convert_setup(&conv, NULL, NULL);
}
if (tmpbuf != NULL)
text = tmpbuf;
}
}


while (len > 0 && text[len - 1] == NUL)
--len;

clip_yank_selection(motion_type, text, (long)len, cbd);
received_selection = RS_OK;
vim_free(tmpbuf);
g_free(tmpbuf_utf8);
}





static void
selection_get_cb(GtkWidget *widget UNUSED,
GtkSelectionData *selection_data,
guint info,
guint time_ UNUSED,
gpointer user_data UNUSED)
{
char_u *string;
char_u *tmpbuf;
long_u tmplen;
int length;
int motion_type;
GdkAtom type;
Clipboard_T *cbd;

if (gtk_selection_data_get_selection(selection_data)
== clip_plus.gtk_sel_atom)
cbd = &clip_plus;
else
cbd = &clip_star;

if (!cbd->owned)
return; 

if (info != (guint)TARGET_STRING
&& (!clip_html || info != (guint)TARGET_HTML)
&& info != (guint)TARGET_UTF8_STRING
&& info != (guint)TARGET_VIMENC
&& info != (guint)TARGET_VIM
&& info != (guint)TARGET_COMPOUND_TEXT
&& info != (guint)TARGET_TEXT)
return;


clip_get_selection(cbd);

motion_type = clip_convert_selection(&string, &tmplen, cbd);
if (motion_type < 0 || string == NULL)
return;



length = MIN(tmplen, (long_u)(G_MAXINT - 1));

if (info == (guint)TARGET_VIM)
{
tmpbuf = alloc(length + 1);
if (tmpbuf != NULL)
{
tmpbuf[0] = motion_type;
mch_memmove(tmpbuf + 1, string, (size_t)length);
}

++length;
vim_free(string);
string = tmpbuf;
type = vim_atom;
}

else if (info == (guint)TARGET_HTML)
{
vimconv_T conv;


conv.vc_type = CONV_NONE;
convert_setup_ext(&conv, p_enc, TRUE, (char_u *)"utf-16le", FALSE);
if (conv.vc_type != CONV_NONE)
{
tmpbuf = string_convert(&conv, string, &length);
convert_setup(&conv, NULL, NULL);
vim_free(string);
string = tmpbuf;
}


if (string != NULL)
{
tmpbuf = alloc(length + 2);
if (tmpbuf != NULL)
{
tmpbuf[0] = 0xff;
tmpbuf[1] = 0xfe;
mch_memmove(tmpbuf + 2, string, (size_t)length);
vim_free(string);
string = tmpbuf;
length += 2;
}

#if !GTK_CHECK_VERSION(3,0,0)


selection_data->type = selection_data->target;
selection_data->format = 16; 
#endif
gtk_selection_data_set(selection_data, html_atom, 16,
string, length);
vim_free(string);
}
return;
}
else if (info == (guint)TARGET_VIMENC)
{
int l = STRLEN(p_enc);


tmpbuf = alloc(length + l + 2);
if (tmpbuf != NULL)
{
tmpbuf[0] = motion_type;
STRCPY(tmpbuf + 1, p_enc);
mch_memmove(tmpbuf + l + 2, string, (size_t)length);
length += l + 2;
vim_free(string);
string = tmpbuf;
}
type = vimenc_atom;
}



else
{
if (output_conv.vc_type != CONV_NONE)
{
tmpbuf = string_convert(&output_conv, string, &length);
vim_free(string);
if (tmpbuf == NULL)
return;
string = tmpbuf;
}

if (g_utf8_validate((const char *)string, (gssize)length, NULL))
{
gtk_selection_data_set_text(selection_data,
(const char *)string, length);
}
vim_free(string);
return;
}

if (string != NULL)
{
#if !GTK_CHECK_VERSION(3,0,0)


selection_data->type = selection_data->target;
selection_data->format = 8; 
#endif
gtk_selection_data_set(selection_data, type, 8, string, length);
vim_free(string);
}
}






int
gui_mch_early_init_check(int give_message)
{
char_u *p;


p = mch_getenv((char_u *)"DISPLAY");
if (p == NULL || *p == NUL)
{
gui.dying = TRUE;
if (give_message)
emsg(_((char *)e_opendisp));
return FAIL;
}
return OK;
}






int
gui_mch_init_check(void)
{
#if defined(USE_GRESOURCE)
static int res_registered = FALSE;

if (!res_registered)
{


res_registered = TRUE;
gui_gtk_register_resource();
}
#endif

#if GTK_CHECK_VERSION(3,10,0)


gdk_set_allowed_backends ("x11");
#endif

#if defined(FEAT_GUI_GNOME)
if (gtk_socket_id == 0)
using_gnome = 1;
#endif




g_set_prgname("gvim");


if (!gtk_init_check(&gui_argc, &gui_argv))
{
gui.dying = TRUE;
emsg(_((char *)e_opendisp));
return FAIL;
}

return OK;
}





static guint mouse_click_timer = 0;
static int mouse_timed_out = TRUE;




static timeout_cb_type
mouse_click_timer_cb(gpointer data)
{

int *timed_out = (int *) data;

*timed_out = TRUE;
return FALSE; 
}

static guint motion_repeat_timer = 0;
static int motion_repeat_offset = FALSE;
static timeout_cb_type motion_repeat_timer_cb(gpointer);

static void
process_motion_notify(int x, int y, GdkModifierType state)
{
int button;
int_u vim_modifiers;
GtkAllocation allocation;

button = (state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK |
GDK_BUTTON3_MASK | GDK_BUTTON4_MASK |
GDK_BUTTON5_MASK))
? MOUSE_DRAG : ' ';


gui_mch_mousehide(FALSE);



if (button != MOUSE_DRAG)
{
gui_mouse_moved(x, y);
return;
}


vim_modifiers = modifiers_gdk2mouse(state);


gui_send_mouse_event(button, x, y, FALSE, vim_modifiers);




gtk_widget_get_allocation(gui.drawarea, &allocation);

if (x < 0 || y < 0
|| x >= allocation.width
|| y >= allocation.height)
{

int dx;
int dy;
int offshoot;
int delay = 10;



dx = x < 0 ? -x : x - allocation.width;
dy = y < 0 ? -y : y - allocation.height;

offshoot = dx > dy ? dx : dy;









if (offshoot > 127)
{

delay = 5;
}
else
{
delay = (130 * (127 - offshoot)) / 127 + 5;
}


if (!motion_repeat_timer)
motion_repeat_timer = timeout_add(delay, motion_repeat_timer_cb,
NULL);
}
}

#if GTK_CHECK_VERSION(3,0,0)
static GdkDevice *
gui_gtk_get_pointer_device(GtkWidget *widget)
{
GdkWindow * const win = gtk_widget_get_window(widget);
GdkDisplay * const dpy = gdk_window_get_display(win);
#if GTK_CHECK_VERSION(3,20,0)
GdkSeat * const seat = gdk_display_get_default_seat(dpy);
return gdk_seat_get_pointer(seat);
#else
GdkDeviceManager * const mngr = gdk_display_get_device_manager(dpy);
return gdk_device_manager_get_client_pointer(mngr);
#endif
}

static GdkWindow *
gui_gtk_get_pointer(GtkWidget *widget,
gint *x,
gint *y,
GdkModifierType *state)
{
GdkWindow * const win = gtk_widget_get_window(widget);
GdkDevice * const dev = gui_gtk_get_pointer_device(widget);
return gdk_window_get_device_position(win, dev , x, y, state);
}

#if defined(FEAT_GUI_TABLINE) || defined(PROTO)
static GdkWindow *
gui_gtk_window_at_position(GtkWidget *widget,
gint *x,
gint *y)
{
GdkDevice * const dev = gui_gtk_get_pointer_device(widget);
return gdk_device_get_window_at_position(dev, x, y);
}
#endif
#else 
#define gui_gtk_get_pointer(wid, x, y, s) gdk_window_get_pointer((wid)->window, x, y, s)

#define gui_gtk_window_at_position(wid, x, y) gdk_window_at_pointer(x, y)
#endif




static timeout_cb_type
motion_repeat_timer_cb(gpointer data UNUSED)
{
int x;
int y;
GdkModifierType state;

gui_gtk_get_pointer(gui.drawarea, &x, &y, &state);

if (!(state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK |
GDK_BUTTON3_MASK | GDK_BUTTON4_MASK |
GDK_BUTTON5_MASK)))
{
motion_repeat_timer = 0;
return FALSE;
}



if (vim_used_in_input_buf() > 10)
return TRUE;

motion_repeat_timer = 0;







if (motion_repeat_offset)
x += gui.char_width;

motion_repeat_offset = !motion_repeat_offset;
process_motion_notify(x, y, state);



return FALSE;
}

static gint
motion_notify_event(GtkWidget *widget,
GdkEventMotion *event,
gpointer data UNUSED)
{
if (event->is_hint)
{
int x;
int y;
GdkModifierType state;

gui_gtk_get_pointer(widget, &x, &y, &state);
process_motion_notify(x, y, state);
}
else
{
process_motion_notify((int)event->x, (int)event->y,
(GdkModifierType)event->state);
}

return TRUE; 
}







static gint
button_press_event(GtkWidget *widget,
GdkEventButton *event,
gpointer data UNUSED)
{
int button;
int repeated_click = FALSE;
int x, y;
int_u vim_modifiers;

gui.event_time = event->time;


if (gtk_socket_id != 0 && !gtk_widget_has_focus(widget))
gtk_widget_grab_focus(widget);





if (event->type != GDK_BUTTON_PRESS)
return FALSE;

x = event->x;
y = event->y;


if (!mouse_timed_out && mouse_click_timer)
{
timeout_remove(mouse_click_timer);
mouse_click_timer = 0;
repeated_click = TRUE;
}

mouse_timed_out = FALSE;
mouse_click_timer = timeout_add(p_mouset, mouse_click_timer_cb,
&mouse_timed_out);

switch (event->button)
{


case 1: button = MOUSE_LEFT; break;
case 2: button = MOUSE_MIDDLE; break;
case 3: button = MOUSE_RIGHT; break;
case 8: button = MOUSE_X1; break;
case 9: button = MOUSE_X2; break;
default:
return FALSE; 
}

#if defined(FEAT_XIM)

if (im_is_preediting())
xim_reset();
#endif

vim_modifiers = modifiers_gdk2mouse(event->state);

gui_send_mouse_event(button, x, y, repeated_click, vim_modifiers);

return TRUE;
}




static gboolean
scroll_event(GtkWidget *widget,
GdkEventScroll *event,
gpointer data UNUSED)
{
int button;
int_u vim_modifiers;

if (gtk_socket_id != 0 && !gtk_widget_has_focus(widget))
gtk_widget_grab_focus(widget);

switch (event->direction)
{
case GDK_SCROLL_UP:
button = MOUSE_4;
break;
case GDK_SCROLL_DOWN:
button = MOUSE_5;
break;
case GDK_SCROLL_LEFT:
button = MOUSE_7;
break;
case GDK_SCROLL_RIGHT:
button = MOUSE_6;
break;
default: 
return FALSE;
}

#if defined(FEAT_XIM)

if (im_is_preediting())
xim_reset();
#endif

vim_modifiers = modifiers_gdk2mouse(event->state);

gui_send_mouse_event(button, (int)event->x, (int)event->y,
FALSE, vim_modifiers);

return TRUE;
}


static gint
button_release_event(GtkWidget *widget UNUSED,
GdkEventButton *event,
gpointer data UNUSED)
{
int x, y;
int_u vim_modifiers;

gui.event_time = event->time;




if (motion_repeat_timer)
{
timeout_remove(motion_repeat_timer);
motion_repeat_timer = 0;
}

x = event->x;
y = event->y;

vim_modifiers = modifiers_gdk2mouse(event->state);

gui_send_mouse_event(MOUSE_RELEASE, x, y, FALSE, vim_modifiers);

return TRUE;
}


#if defined(FEAT_DND)









static int
count_and_decode_uri_list(char_u *out, char_u *raw, int len)
{
int i;
char_u *p = out;
int count = 0;

for (i = 0; i < len; ++i)
{
if (raw[i] == NUL || raw[i] == '\n' || raw[i] == '\r')
{
if (p > out && p[-1] != NUL)
{
++count;
*p++ = NUL;
}
}
else if (raw[i] == '%' && i + 2 < len && hexhex2nr(raw + i + 1) > 0)
{
*p++ = hexhex2nr(raw + i + 1);
i += 2;
}
else
*p++ = raw[i];
}
if (p > out && p[-1] != NUL)
{
*p = NUL; 
++count;
}
return count;
}






static int
filter_uri_list(char_u **outlist, int max, char_u *src)
{
int i, j;

for (i = j = 0; i < max; ++i)
{
outlist[i] = NULL;
if (STRNCMP(src, "file:", 5) == 0)
{
src += 5;
if (STRNCMP(src, "//localhost", 11) == 0)
src += 11;
while (src[0] == '/' && src[1] == '/')
++src;
outlist[j++] = vim_strsave(src);
}
src += STRLEN(src) + 1;
}
return j;
}

static char_u **
parse_uri_list(int *count, char_u *data, int len)
{
int n = 0;
char_u *tmp = NULL;
char_u **array = NULL;

if (data != NULL && len > 0 && (tmp = alloc(len + 1)) != NULL)
{
n = count_and_decode_uri_list(tmp, data, len);
if (n > 0 && (array = ALLOC_MULT(char_u *, n)) != NULL)
n = filter_uri_list(array, n, tmp);
}
vim_free(tmp);
*count = n;
return array;
}

static void
drag_handle_uri_list(GdkDragContext *context,
GtkSelectionData *data,
guint time_,
GdkModifierType state,
gint x,
gint y)
{
char_u **fnames;
int nfiles = 0;

fnames = parse_uri_list(&nfiles,
(char_u *)gtk_selection_data_get_data(data),
gtk_selection_data_get_length(data));

if (fnames != NULL && nfiles > 0)
{
int_u modifiers;

gtk_drag_finish(context, TRUE, FALSE, time_); 

modifiers = modifiers_gdk2mouse(state);

gui_handle_drop(x, y, modifiers, fnames, nfiles);
}
else
vim_free(fnames);
}

static void
drag_handle_text(GdkDragContext *context,
GtkSelectionData *data,
guint time_,
GdkModifierType state)
{
char_u dropkey[6] = {CSI, KS_MODIFIER, 0, CSI, KS_EXTRA, (char_u)KE_DROP};
char_u *text;
int len;
char_u *tmpbuf = NULL;

text = (char_u *)gtk_selection_data_get_data(data);
len = gtk_selection_data_get_length(data);

if (gtk_selection_data_get_data_type(data) == utf8_string_atom)
{
if (input_conv.vc_type != CONV_NONE)
tmpbuf = string_convert(&input_conv, text, &len);
if (tmpbuf != NULL)
text = tmpbuf;
}

dnd_yank_drag_data(text, (long)len);
gtk_drag_finish(context, TRUE, FALSE, time_); 
vim_free(tmpbuf);

dropkey[2] = modifiers_gdk2vim(state);

if (dropkey[2] != 0)
add_to_input_buf(dropkey, (int)sizeof(dropkey));
else
add_to_input_buf(dropkey + 3, (int)(sizeof(dropkey) - 3));
}




static void
drag_data_received_cb(GtkWidget *widget,
GdkDragContext *context,
gint x,
gint y,
GtkSelectionData *data,
guint info,
guint time_,
gpointer user_data UNUSED)
{
GdkModifierType state;


const guchar * const data_data = gtk_selection_data_get_data(data);
const gint data_length = gtk_selection_data_get_length(data);
const gint data_format = gtk_selection_data_get_format(data);

if (data_data == NULL
|| data_length <= 0
|| data_format != 8
|| data_data[data_length] != '\0')
{
gtk_drag_finish(context, FALSE, FALSE, time_);
return;
}



gui_gtk_get_pointer(widget, NULL, NULL, &state);


if (info == (guint)TARGET_TEXT_URI_LIST)
drag_handle_uri_list(context, data, time_, state, x, y);
else
drag_handle_text(context, data, time_, state);

}
#endif 


#if defined(USE_GNOME_SESSION)





static void
sm_client_check_changed_any(GnomeClient *client UNUSED,
gint key,
GnomeDialogType type UNUSED,
gpointer data UNUSED)
{
cmdmod_T save_cmdmod;
gboolean shutdown_cancelled;

save_cmdmod = cmdmod;

#if defined(FEAT_BROWSE)
cmdmod.browse = TRUE;
#endif
#if defined(FEAT_GUI_DIALOG) || defined(FEAT_CON_DIALOG)
cmdmod.confirm = TRUE;
#endif




shutdown_cancelled = check_changed_any(FALSE, FALSE);

exiting = FALSE;
cmdmod = save_cmdmod;
setcursor(); 
out_flush();




gnome_interaction_key_return(key, shutdown_cancelled);
}






static gboolean
sm_client_save_yourself(GnomeClient *client,
gint phase UNUSED,
GnomeSaveStyle save_style UNUSED,
gboolean shutdown UNUSED,
GnomeInteractStyle interact_style,
gboolean fast UNUSED,
gpointer data UNUSED)
{
static const char suffix[] = "-session.vim";
char *session_file;
unsigned int len;
gboolean success;





if (interact_style == GNOME_INTERACT_ANY)
gnome_client_request_interaction(client, GNOME_DIALOG_NORMAL,
&sm_client_check_changed_any,
NULL);
out_flush();
ml_sync_all(FALSE, FALSE); 




session_file = gnome_config_get_real_path(
gnome_client_get_config_prefix(client));
len = strlen(session_file);

if (len > 0 && session_file[len-1] == G_DIR_SEPARATOR)
--len; 

session_file = g_renew(char, session_file, len + sizeof(suffix));
memcpy(session_file + len, suffix, sizeof(suffix));

success = write_session_file((char_u *)session_file);

if (success)
{
const char *argv[8];
int i;





i = 0;
argv[i++] = "rm";
argv[i++] = session_file;
argv[i] = NULL;

gnome_client_set_discard_command(client, i, (char **)argv);






i = 0;
argv[i++] = restart_command;
argv[i++] = "-f";
argv[i++] = "-g";
argv[i++] = "--role";
argv[i++] = gtk_window_get_role(GTK_WINDOW(gui.mainwin));
argv[i++] = "-S";
argv[i++] = session_file;
argv[i] = NULL;

gnome_client_set_restart_command(client, i, (char **)argv);
gnome_client_set_clone_command(client, 0, NULL);
}

g_free(session_file);

return success;
}






static void
sm_client_die(GnomeClient *client UNUSED, gpointer data UNUSED)
{

full_screen = FALSE;

vim_strncpy(IObuff, (char_u *)
_("Vim: Received \"die\" request from session manager\n"),
IOSIZE - 1);
preserve_exit();
}




static void
setup_save_yourself(void)
{
GnomeClient *client;

client = gnome_master_client();

if (client != NULL)
{


gtk_signal_connect(GTK_OBJECT(client), "save_yourself",
GTK_SIGNAL_FUNC(&sm_client_save_yourself), NULL);
gtk_signal_connect(GTK_OBJECT(client), "die",
GTK_SIGNAL_FUNC(&sm_client_die), NULL);
}
}

#else 

#if defined(USE_XSMP)



static gboolean
local_xsmp_handle_requests(
GIOChannel *source UNUSED,
GIOCondition condition,
gpointer data)
{
if (condition == G_IO_IN)
{

if (xsmp_handle_requests() == FAIL)
g_io_channel_unref((GIOChannel *)data);
return TRUE;
}

g_io_channel_unref((GIOChannel *)data);
xsmp_close();
return TRUE;
}
#endif 





static void
setup_save_yourself(void)
{
Atom *existing_atoms = NULL;
int count = 0;

#if defined(USE_XSMP)
if (xsmp_icefd != -1)
{




GIOChannel *g_io = g_io_channel_unix_new(xsmp_icefd);

g_io_add_watch(g_io, G_IO_IN | G_IO_ERR | G_IO_HUP,
local_xsmp_handle_requests, (gpointer)g_io);
g_io_channel_unref(g_io);
}
else
#endif
{



GdkWindow * const mainwin_win = gtk_widget_get_window(gui.mainwin);

if (XGetWMProtocols(GDK_WINDOW_XDISPLAY(mainwin_win),
GDK_WINDOW_XID(mainwin_win),
&existing_atoms, &count))
{
Atom *new_atoms;
Atom save_yourself_xatom;
int i;

save_yourself_xatom = GET_X_ATOM(save_yourself_atom);


for (i = 0; i < count; ++i)
if (existing_atoms[i] == save_yourself_xatom)
break;

if (i == count)
{

new_atoms = ALLOC_MULT(Atom, count + 1);
if (new_atoms != NULL)
{
memcpy(new_atoms, existing_atoms, count * sizeof(Atom));
new_atoms[count] = save_yourself_xatom;
XSetWMProtocols(GDK_WINDOW_XDISPLAY(mainwin_win),
GDK_WINDOW_XID(mainwin_win),
new_atoms, count + 1);
vim_free(new_atoms);
}
}
XFree(existing_atoms);
}
}
}
















static GdkFilterReturn
global_event_filter(GdkXEvent *xev,
GdkEvent *event UNUSED,
gpointer data UNUSED)
{
XEvent *xevent = (XEvent *)xev;

if (xevent != NULL
&& xevent->type == ClientMessage
&& xevent->xclient.message_type == GET_X_ATOM(wm_protocols_atom)
&& (long_u)xevent->xclient.data.l[0]
== GET_X_ATOM(save_yourself_atom))
{
out_flush();
ml_sync_all(FALSE, FALSE); 





XSetCommand(GDK_WINDOW_XDISPLAY(gtk_widget_get_window(gui.mainwin)),
GDK_WINDOW_XID(gtk_widget_get_window(gui.mainwin)),
NULL, 0);
return GDK_FILTER_REMOVE;
}

return GDK_FILTER_CONTINUE;
}
#endif 





static void
mainwin_realize(GtkWidget *widget UNUSED, gpointer data UNUSED)
{


#if defined(magick)
#undef magick
#endif

#define static static const
#define magick vim32x32
#include "../runtime/vim32x32.xpm"
#undef magick
#define magick vim16x16
#include "../runtime/vim16x16.xpm"
#undef magick
#define magick vim48x48
#include "../runtime/vim48x48.xpm"
#undef magick
#undef static

GdkWindow * const mainwin_win = gtk_widget_get_window(gui.mainwin);


if (echo_wid_arg)
{
printf("WID: %ld\n", (long)GDK_WINDOW_XID(mainwin_win));
fflush(stdout);
}

if (vim_strchr(p_go, GO_ICON) != NULL)
{



GList *icons = NULL;

icons = g_list_prepend(icons, gdk_pixbuf_new_from_xpm_data(vim16x16));
icons = g_list_prepend(icons, gdk_pixbuf_new_from_xpm_data(vim32x32));
icons = g_list_prepend(icons, gdk_pixbuf_new_from_xpm_data(vim48x48));

gtk_window_set_icon_list(GTK_WINDOW(gui.mainwin), icons);

g_list_foreach(icons, (GFunc)&g_object_unref, NULL);
g_list_free(icons);
}

#if !defined(USE_GNOME_SESSION)

gdk_window_add_filter(NULL, &global_event_filter, NULL);
#endif



#if defined(USE_GNOME_SESSION)
if (using_gnome)
#endif
setup_save_yourself();

#if defined(FEAT_CLIENTSERVER)
if (serverName == NULL && serverDelayedStartName != NULL)
{

commWindow = GDK_WINDOW_XID(mainwin_win);

(void)serverRegisterName(GDK_WINDOW_XDISPLAY(mainwin_win),
serverDelayedStartName);
}
else
{





serverChangeRegisteredWindow(GDK_WINDOW_XDISPLAY(mainwin_win),
GDK_WINDOW_XID(mainwin_win));
}
gtk_widget_add_events(gui.mainwin, GDK_PROPERTY_CHANGE_MASK);
g_signal_connect(G_OBJECT(gui.mainwin), "property-notify-event",
G_CALLBACK(property_event), NULL);
#endif
}

static GdkCursor *
create_blank_pointer(void)
{
GdkWindow *root_window = NULL;
#if GTK_CHECK_VERSION(3,0,0)
GdkPixbuf *blank_mask;
#else
GdkPixmap *blank_mask;
#endif
GdkCursor *cursor;
#if GTK_CHECK_VERSION(3,0,0)
GdkRGBA color = { 0.0, 0.0, 0.0, 1.0 };
#else
GdkColor color = { 0, 0, 0, 0 };
char blank_data[] = { 0x0 };
#endif

#if GTK_CHECK_VERSION(3,12,0)
{
GdkWindow * const win = gtk_widget_get_window(gui.mainwin);
GdkScreen * const scrn = gdk_window_get_screen(win);
root_window = gdk_screen_get_root_window(scrn);
}
#else
root_window = gtk_widget_get_root_window(gui.mainwin);
#endif



#if GTK_CHECK_VERSION(3,0,0)
{
cairo_surface_t *surf;
cairo_t *cr;

surf = cairo_image_surface_create(CAIRO_FORMAT_A1, 1, 1);
cr = cairo_create(surf);

cairo_set_source_rgba(cr,
color.red,
color.green,
color.blue,
color.alpha);
cairo_rectangle(cr, 0, 0, 1, 1);
cairo_fill(cr);
cairo_destroy(cr);

blank_mask = gdk_pixbuf_get_from_surface(surf, 0, 0, 1, 1);
cairo_surface_destroy(surf);

cursor = gdk_cursor_new_from_pixbuf(gdk_window_get_display(root_window),
blank_mask, 0, 0);
g_object_unref(blank_mask);
}
#else
blank_mask = gdk_bitmap_create_from_data(root_window, blank_data, 1, 1);
cursor = gdk_cursor_new_from_pixmap(blank_mask, blank_mask,
&color, &color, 0, 0);
gdk_bitmap_unref(blank_mask);
#endif

return cursor;
}

static void
mainwin_screen_changed_cb(GtkWidget *widget,
GdkScreen *previous_screen UNUSED,
gpointer data UNUSED)
{
if (!gtk_widget_has_screen(widget))
return;




if (gui.blank_pointer != NULL)
#if GTK_CHECK_VERSION(3,0,0)
g_object_unref(G_OBJECT(gui.blank_pointer));
#else
gdk_cursor_unref(gui.blank_pointer);
#endif

gui.blank_pointer = create_blank_pointer();

if (gui.pointer_hidden && gtk_widget_get_window(gui.drawarea) != NULL)
gdk_window_set_cursor(gtk_widget_get_window(gui.drawarea),
gui.blank_pointer);





if (gui.text_context != NULL)
g_object_unref(gui.text_context);

gui.text_context = gtk_widget_create_pango_context(widget);
pango_context_set_base_dir(gui.text_context, PANGO_DIRECTION_LTR);

if (gui.norm_font != NULL)
{
gui_mch_init_font(p_guifont, FALSE);
gui_set_shellsize(TRUE, FALSE, RESIZE_BOTH);
}
}








static void
drawarea_realize_cb(GtkWidget *widget, gpointer data UNUSED)
{
GtkWidget *sbar;
GtkAllocation allocation;

#if defined(FEAT_XIM)
xim_init();
#endif
gui_mch_new_colors();
#if GTK_CHECK_VERSION(3,0,0)
gui.surface = gdk_window_create_similar_surface(
gtk_widget_get_window(widget),
CAIRO_CONTENT_COLOR_ALPHA,
gtk_widget_get_allocated_width(widget),
gtk_widget_get_allocated_height(widget));
#else
gui.text_gc = gdk_gc_new(gui.drawarea->window);
#endif

gui.blank_pointer = create_blank_pointer();
if (gui.pointer_hidden)
gdk_window_set_cursor(gtk_widget_get_window(widget), gui.blank_pointer);


sbar = firstwin->w_scrollbars[SBAR_LEFT].id;
if (!sbar || (!gui.which_scrollbars[SBAR_LEFT]
&& firstwin->w_scrollbars[SBAR_RIGHT].id))
sbar = firstwin->w_scrollbars[SBAR_RIGHT].id;
gtk_widget_get_allocation(sbar, &allocation);
if (sbar && gtk_widget_get_realized(sbar) && allocation.width)
gui.scrollbar_width = allocation.width;

sbar = gui.bottom_sbar.id;
if (sbar && gtk_widget_get_realized(sbar) && allocation.height)
gui.scrollbar_height = allocation.height;
}




static void
drawarea_unrealize_cb(GtkWidget *widget UNUSED, gpointer data UNUSED)
{

full_screen = FALSE;

#if defined(FEAT_XIM)
im_shutdown();
#endif
if (gui.ascii_glyphs != NULL)
{
pango_glyph_string_free(gui.ascii_glyphs);
gui.ascii_glyphs = NULL;
}
if (gui.ascii_font != NULL)
{
g_object_unref(gui.ascii_font);
gui.ascii_font = NULL;
}
g_object_unref(gui.text_context);
gui.text_context = NULL;

#if GTK_CHECK_VERSION(3,0,0)
if (gui.surface != NULL)
{
cairo_surface_destroy(gui.surface);
gui.surface = NULL;
}
#else
g_object_unref(gui.text_gc);
gui.text_gc = NULL;
#endif

#if GTK_CHECK_VERSION(3,0,0)
g_object_unref(G_OBJECT(gui.blank_pointer));
#else
gdk_cursor_unref(gui.blank_pointer);
#endif
gui.blank_pointer = NULL;
}

#if GTK_CHECK_VERSION(3,22,2)
static void
drawarea_style_updated_cb(GtkWidget *widget UNUSED,
gpointer data UNUSED)
#else
static void
drawarea_style_set_cb(GtkWidget *widget UNUSED,
GtkStyle *previous_style UNUSED,
gpointer data UNUSED)
#endif
{
gui_mch_new_colors();
}

#if GTK_CHECK_VERSION(3,0,0)
static gboolean
drawarea_configure_event_cb(GtkWidget *widget,
GdkEventConfigure *event,
gpointer data UNUSED)
{
static int cur_width = 0;
static int cur_height = 0;

g_return_val_if_fail(event
&& event->width >= 1 && event->height >= 1, TRUE);

#if GTK_CHECK_VERSION(3,22,2) && !GTK_CHECK_VERSION(3,22,4)
























if (event->send_event == FALSE)
return TRUE;
#endif

if (event->width == cur_width && event->height == cur_height)
return TRUE;

cur_width = event->width;
cur_height = event->height;

if (gui.surface != NULL)
cairo_surface_destroy(gui.surface);

gui.surface = gdk_window_create_similar_surface(
gtk_widget_get_window(widget),
CAIRO_CONTENT_COLOR_ALPHA,
event->width, event->height);

gtk_widget_queue_draw(widget);

return TRUE;
}
#endif





static gint
delete_event_cb(GtkWidget *widget UNUSED,
GdkEventAny *event UNUSED,
gpointer data UNUSED)
{
gui_shell_closed();
return TRUE;
}

#if defined(FEAT_MENU) || defined(FEAT_TOOLBAR) || defined(FEAT_GUI_TABLINE)
static int
get_item_dimensions(GtkWidget *widget, GtkOrientation orientation)
{
#if defined(FEAT_GUI_GNOME)
GtkOrientation item_orientation = GTK_ORIENTATION_HORIZONTAL;

if (using_gnome && widget != NULL)
{
GtkWidget *parent;
BonoboDockItem *dockitem;

parent = gtk_widget_get_parent(widget);
if (G_TYPE_FROM_INSTANCE(parent) == BONOBO_TYPE_DOCK_ITEM)
{


widget = parent;
dockitem = BONOBO_DOCK_ITEM(widget);

if (dockitem == NULL || dockitem->is_floating)
return 0;
item_orientation = bonobo_dock_item_get_orientation(dockitem);
}
}
#else
#define item_orientation GTK_ORIENTATION_HORIZONTAL
#endif

if (widget != NULL
&& item_orientation == orientation
&& gtk_widget_get_realized(widget)
&& gtk_widget_get_visible(widget))
{
#if GTK_CHECK_VERSION(3,0,0) || !defined(FEAT_GUI_GNOME)
GtkAllocation allocation;

gtk_widget_get_allocation(widget, &allocation);
return allocation.height;
#else
if (orientation == GTK_ORIENTATION_HORIZONTAL)
return widget->allocation.height;
else
return widget->allocation.width;
#endif
}
return 0;
}
#endif

static int
get_menu_tool_width(void)
{
int width = 0;

#if defined(FEAT_GUI_GNOME)
#if defined(FEAT_MENU)
width += get_item_dimensions(gui.menubar, GTK_ORIENTATION_VERTICAL);
#endif
#if defined(FEAT_TOOLBAR)
width += get_item_dimensions(gui.toolbar, GTK_ORIENTATION_VERTICAL);
#endif
#if defined(FEAT_GUI_TABLINE)
if (gui.tabline != NULL)
width += get_item_dimensions(gui.tabline, GTK_ORIENTATION_VERTICAL);
#endif
#endif

return width;
}

static int
get_menu_tool_height(void)
{
int height = 0;

#if defined(FEAT_MENU)
height += get_item_dimensions(gui.menubar, GTK_ORIENTATION_HORIZONTAL);
#endif
#if defined(FEAT_TOOLBAR)
height += get_item_dimensions(gui.toolbar, GTK_ORIENTATION_HORIZONTAL);
#endif
#if defined(FEAT_GUI_TABLINE)
if (gui.tabline != NULL)
height += get_item_dimensions(gui.tabline, GTK_ORIENTATION_HORIZONTAL);
#endif

return height;
}






static int init_window_hints_state = 0;

static void
update_window_manager_hints(int force_width, int force_height)
{
static int old_width = 0;
static int old_height = 0;
static int old_min_width = 0;
static int old_min_height = 0;
static int old_char_width = 0;
static int old_char_height = 0;

int width;
int height;
int min_width;
int min_height;




if (!(force_width && force_height) && init_window_hints_state > 0)
{

init_window_hints_state = 2;
return;
}



width = gui_get_base_width();
height = gui_get_base_height();
#if defined(FEAT_MENU)
height += tabline_height() * gui.char_height;
#endif
width += get_menu_tool_width();
height += get_menu_tool_height();






if (force_width && force_height)
{
min_width = force_width;
min_height = force_height;
}
else
{
min_width = width + MIN_COLUMNS * gui.char_width;
min_height = height + MIN_LINES * gui.char_height;
}


if (width != old_width
|| height != old_height
|| min_width != old_min_width
|| min_height != old_min_height
|| gui.char_width != old_char_width
|| gui.char_height != old_char_height)
{
GdkGeometry geometry;
GdkWindowHints geometry_mask;

geometry.width_inc = gui.char_width;
geometry.height_inc = gui.char_height;
geometry.base_width = width;
geometry.base_height = height;
geometry.min_width = min_width;
geometry.min_height = min_height;
geometry_mask = GDK_HINT_BASE_SIZE|GDK_HINT_RESIZE_INC
|GDK_HINT_MIN_SIZE;



gtk_window_set_geometry_hints(GTK_WINDOW(gui.mainwin), gui.mainwin,
&geometry, geometry_mask);
old_width = width;
old_height = height;
old_min_width = min_width;
old_min_height = min_height;
old_char_width = gui.char_width;
old_char_height = gui.char_height;
}
}

#if defined(FEAT_GUI_DARKTHEME) || defined(PROTO)
void
gui_mch_set_dark_theme(int dark)
{
#if GTK_CHECK_VERSION(3,0,0)
GtkSettings *gtk_settings;

gtk_settings = gtk_settings_get_for_screen(gdk_screen_get_default());
g_object_set(gtk_settings, "gtk-application-prefer-dark-theme", (gboolean)dark, NULL);
#endif
}
#endif 

#if defined(FEAT_TOOLBAR)






static void
icon_size_changed_foreach(GtkWidget *widget, gpointer user_data)
{
if (GTK_IS_IMAGE(widget))
{
GtkImage *image = (GtkImage *)widget;

#if GTK_CHECK_VERSION(3,10,0)
if (gtk_image_get_storage_type(image) == GTK_IMAGE_ICON_NAME)
{
const GtkIconSize icon_size = GPOINTER_TO_INT(user_data);
const gchar *icon_name;

gtk_image_get_icon_name(image, &icon_name, NULL);

gtk_image_set_from_icon_name(image, icon_name, icon_size);
}
#else

if (gtk_image_get_storage_type(image) == GTK_IMAGE_ICON_SET)
{
GtkIconSet *icon_set;
GtkIconSize icon_size;

gtk_image_get_icon_set(image, &icon_set, &icon_size);
icon_size = (GtkIconSize)(long)user_data;

gtk_icon_set_ref(icon_set);
gtk_image_set_from_icon_set(image, icon_set, icon_size);
gtk_icon_set_unref(icon_set);
}
#endif
}
else if (GTK_IS_CONTAINER(widget))
{
gtk_container_foreach((GtkContainer *)widget,
&icon_size_changed_foreach,
user_data);
}
}

static void
set_toolbar_style(GtkToolbar *toolbar)
{
GtkToolbarStyle style;
GtkIconSize size;
GtkIconSize oldsize;

if ((toolbar_flags & (TOOLBAR_TEXT | TOOLBAR_ICONS | TOOLBAR_HORIZ))
== (TOOLBAR_TEXT | TOOLBAR_ICONS | TOOLBAR_HORIZ))
style = GTK_TOOLBAR_BOTH_HORIZ;
else if ((toolbar_flags & (TOOLBAR_TEXT | TOOLBAR_ICONS))
== (TOOLBAR_TEXT | TOOLBAR_ICONS))
style = GTK_TOOLBAR_BOTH;
else if (toolbar_flags & TOOLBAR_TEXT)
style = GTK_TOOLBAR_TEXT;
else
style = GTK_TOOLBAR_ICONS;

gtk_toolbar_set_style(toolbar, style);
#if !GTK_CHECK_VERSION(3,0,0)
gtk_toolbar_set_tooltips(toolbar, (toolbar_flags & TOOLBAR_TOOLTIPS) != 0);
#endif

switch (tbis_flags)
{
case TBIS_TINY: size = GTK_ICON_SIZE_MENU; break;
case TBIS_SMALL: size = GTK_ICON_SIZE_SMALL_TOOLBAR; break;
case TBIS_MEDIUM: size = GTK_ICON_SIZE_BUTTON; break;
case TBIS_LARGE: size = GTK_ICON_SIZE_LARGE_TOOLBAR; break;
case TBIS_HUGE: size = GTK_ICON_SIZE_DND; break;
case TBIS_GIANT: size = GTK_ICON_SIZE_DIALOG; break;
default: size = GTK_ICON_SIZE_INVALID; break;
}
oldsize = gtk_toolbar_get_icon_size(toolbar);

if (size == GTK_ICON_SIZE_INVALID)
{

gtk_toolbar_unset_icon_size(toolbar);
size = gtk_toolbar_get_icon_size(toolbar);
}
if (size != oldsize)
{
gtk_container_foreach(GTK_CONTAINER(toolbar),
&icon_size_changed_foreach,
GINT_TO_POINTER((int)size));
}
gtk_toolbar_set_icon_size(toolbar, size);
}

#endif 

#if defined(FEAT_GUI_TABLINE) || defined(PROTO)
static int ignore_tabline_evt = FALSE;
static GtkWidget *tabline_menu;
#if !GTK_CHECK_VERSION(3,0,0)
static GtkTooltips *tabline_tooltip;
#endif
static int clicked_page; 




static void
tabline_menu_handler(GtkMenuItem *item UNUSED, gpointer user_data)
{

send_tabline_menu_event(clicked_page, (int)(long)user_data);
}

static void
add_tabline_menu_item(GtkWidget *menu, char_u *text, int resp)
{
GtkWidget *item;
char_u *utf_text;

utf_text = CONVERT_TO_UTF8(text);
item = gtk_menu_item_new_with_label((const char *)utf_text);
gtk_widget_show(item);
CONVERT_TO_UTF8_FREE(utf_text);

gtk_container_add(GTK_CONTAINER(menu), item);
g_signal_connect(G_OBJECT(item), "activate",
G_CALLBACK(tabline_menu_handler),
GINT_TO_POINTER(resp));
}




static GtkWidget *
create_tabline_menu(void)
{
GtkWidget *menu;

menu = gtk_menu_new();
add_tabline_menu_item(menu, (char_u *)_("Close tab"), TABLINE_MENU_CLOSE);
add_tabline_menu_item(menu, (char_u *)_("New tab"), TABLINE_MENU_NEW);
add_tabline_menu_item(menu, (char_u *)_("Open Tab..."), TABLINE_MENU_OPEN);

return menu;
}

static gboolean
on_tabline_menu(GtkWidget *widget, GdkEvent *event)
{

if (event->type == GDK_BUTTON_PRESS)
{
GdkEventButton *bevent = (GdkEventButton *)event;
int x = bevent->x;
int y = bevent->y;
GtkWidget *tabwidget;
GdkWindow *tabwin;



if (hold_gui_events
#if defined(FEAT_CMDWIN)
|| cmdwin_type != 0
#endif
)
return TRUE;

tabwin = gui_gtk_window_at_position(gui.mainwin, &x, &y);

gdk_window_get_user_data(tabwin, (gpointer)&tabwidget);
clicked_page = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(tabwidget),
"tab_num"));


if (bevent->button == 3)
{
#if GTK_CHECK_VERSION(3,22,2)
gtk_menu_popup_at_pointer(GTK_MENU(widget), event);
#else
gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL,
bevent->button, bevent->time);
#endif

return TRUE;
}
else if (bevent->button == 1)
{
if (clicked_page == 0)
{


send_tabline_event(x < 50 ? -1 : 0);
}
}
}


return FALSE;
}




static void
on_select_tab(
GtkNotebook *notebook UNUSED,
gpointer *page UNUSED,
gint idx,
gpointer data UNUSED)
{
if (!ignore_tabline_evt)
send_tabline_event(idx + 1);
}

#if GTK_CHECK_VERSION(2,10,0)



static void
on_tab_reordered(
GtkNotebook *notebook UNUSED,
gpointer *page UNUSED,
gint idx,
gpointer data UNUSED)
{
if (!ignore_tabline_evt)
{
if ((tabpage_index(curtab) - 1) < idx)
tabpage_move(idx + 1);
else
tabpage_move(idx);
}
}
#endif




void
gui_mch_show_tabline(int showit)
{
if (gui.tabline == NULL)
return;

if (!showit != !gtk_notebook_get_show_tabs(GTK_NOTEBOOK(gui.tabline)))
{

gtk_notebook_set_show_tabs(GTK_NOTEBOOK(gui.tabline), showit);
update_window_manager_hints(0, 0);
if (showit)
gtk_widget_set_can_focus(GTK_WIDGET(gui.tabline), FALSE);
}

gui_mch_update();
}




int
gui_mch_showing_tabline(void)
{
return gui.tabline != NULL
&& gtk_notebook_get_show_tabs(GTK_NOTEBOOK(gui.tabline));
}




void
gui_mch_update_tabline(void)
{
GtkWidget *page;
GtkWidget *event_box;
GtkWidget *label;
tabpage_T *tp;
int nr = 0;
int tab_num;
int curtabidx = 0;
char_u *labeltext;

if (gui.tabline == NULL)
return;

ignore_tabline_evt = TRUE;


for (tp = first_tabpage; tp != NULL; tp = tp->tp_next, ++nr)
{
if (tp == curtab)
curtabidx = nr;

tab_num = nr + 1;

page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(gui.tabline), nr);
if (page == NULL)
{

#if GTK_CHECK_VERSION(3,2,0)
page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
gtk_box_set_homogeneous(GTK_BOX(page), FALSE);
#else
page = gtk_vbox_new(FALSE, 0);
#endif
gtk_widget_show(page);
event_box = gtk_event_box_new();
gtk_widget_show(event_box);
label = gtk_label_new("-Empty-");
#if !GTK_CHECK_VERSION(3,14,0)
gtk_misc_set_padding(GTK_MISC(label), 2, 2);
#endif
gtk_container_add(GTK_CONTAINER(event_box), label);
gtk_widget_show(label);
gtk_notebook_insert_page(GTK_NOTEBOOK(gui.tabline),
page,
event_box,
nr++);
#if GTK_CHECK_VERSION(2,10,0)
gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(gui.tabline),
page,
TRUE);
#endif
}

event_box = gtk_notebook_get_tab_label(GTK_NOTEBOOK(gui.tabline), page);
g_object_set_data(G_OBJECT(event_box), "tab_num",
GINT_TO_POINTER(tab_num));
label = gtk_bin_get_child(GTK_BIN(event_box));
get_tabline_label(tp, FALSE);
labeltext = CONVERT_TO_UTF8(NameBuff);
gtk_label_set_text(GTK_LABEL(label), (const char *)labeltext);
CONVERT_TO_UTF8_FREE(labeltext);

get_tabline_label(tp, TRUE);
labeltext = CONVERT_TO_UTF8(NameBuff);
#if GTK_CHECK_VERSION(3,0,0)
gtk_widget_set_tooltip_text(event_box, (const gchar *)labeltext);
#else
gtk_tooltips_set_tip(GTK_TOOLTIPS(tabline_tooltip), event_box,
(const char *)labeltext, NULL);
#endif
CONVERT_TO_UTF8_FREE(labeltext);
}


while (gtk_notebook_get_nth_page(GTK_NOTEBOOK(gui.tabline), nr) != NULL)
gtk_notebook_remove_page(GTK_NOTEBOOK(gui.tabline), nr);

if (gtk_notebook_get_current_page(GTK_NOTEBOOK(gui.tabline)) != curtabidx)
gtk_notebook_set_current_page(GTK_NOTEBOOK(gui.tabline), curtabidx);


gui_mch_update();

ignore_tabline_evt = FALSE;
}




void
gui_mch_set_curtab(int nr)
{
if (gui.tabline == NULL)
return;

ignore_tabline_evt = TRUE;
if (gtk_notebook_get_current_page(GTK_NOTEBOOK(gui.tabline)) != nr - 1)
gtk_notebook_set_current_page(GTK_NOTEBOOK(gui.tabline), nr - 1);
ignore_tabline_evt = FALSE;
}

#endif 




void
gui_gtk_set_selection_targets(void)
{
int i, j = 0;
int n_targets = N_SELECTION_TARGETS;
GtkTargetEntry targets[N_SELECTION_TARGETS];

for (i = 0; i < (int)N_SELECTION_TARGETS; ++i)
{



if (!clip_html && selection_targets[i].info == TARGET_HTML)
n_targets--;
else
targets[j++] = selection_targets[i];
}

gtk_selection_clear_targets(gui.drawarea, (GdkAtom)GDK_SELECTION_PRIMARY);
gtk_selection_clear_targets(gui.drawarea, (GdkAtom)clip_plus.gtk_sel_atom);
gtk_selection_add_targets(gui.drawarea,
(GdkAtom)GDK_SELECTION_PRIMARY,
targets, n_targets);
gtk_selection_add_targets(gui.drawarea,
(GdkAtom)clip_plus.gtk_sel_atom,
targets, n_targets);
}




void
gui_gtk_set_dnd_targets(void)
{
int i, j = 0;
int n_targets = N_DND_TARGETS;
GtkTargetEntry targets[N_DND_TARGETS];

for (i = 0; i < (int)N_DND_TARGETS; ++i)
{
if (!clip_html && dnd_targets[i].info == TARGET_HTML)
n_targets--;
else
targets[j++] = dnd_targets[i];
}

gtk_drag_dest_unset(gui.drawarea);
gtk_drag_dest_set(gui.drawarea,
GTK_DEST_DEFAULT_ALL,
targets, n_targets,
GDK_ACTION_COPY | GDK_ACTION_MOVE);
}





int
gui_mch_init(void)
{
GtkWidget *vbox;

#if defined(FEAT_GUI_GNOME)



if (using_gnome)
{
gnome_program_init(VIMPACKAGE, VIM_VERSION_SHORT,
LIBGNOMEUI_MODULE, gui_argc, gui_argv, NULL);
#if defined(FEAT_FLOAT) && defined(LC_NUMERIC)
{
char *p = setlocale(LC_NUMERIC, NULL);



if (p == NULL || strcmp(p, "C") != 0)
setlocale(LC_NUMERIC, "C");
}
#endif
}
#endif
VIM_CLEAR(gui_argv);

#if GLIB_CHECK_VERSION(2,1,3)

g_set_application_name("Vim");
#endif





set_option_value((char_u *)"termencoding", 0L, (char_u *)"utf-8", 0);

#if defined(FEAT_TOOLBAR)
gui_gtk_register_stock_icons();
#endif


#if !GTK_CHECK_VERSION(3,0,0)
#if 0
gtk_rc_parse("gtkrc");
#endif
#endif


gui.border_width = 2;
gui.scrollbar_width = SB_DEFAULT_WIDTH;
gui.scrollbar_height = SB_DEFAULT_WIDTH;
#if GTK_CHECK_VERSION(3,0,0)
gui.fgcolor = g_new(GdkRGBA, 1);
gui.bgcolor = g_new(GdkRGBA, 1);
gui.spcolor = g_new(GdkRGBA, 1);
#else

gui.fgcolor = g_new0(GdkColor, 1);

gui.bgcolor = g_new0(GdkColor, 1);

gui.spcolor = g_new0(GdkColor, 1);
#endif


html_atom = gdk_atom_intern("text/html", FALSE);
utf8_string_atom = gdk_atom_intern("UTF8_STRING", FALSE);


gui.norm_pixel = gui.def_norm_pixel;
gui.back_pixel = gui.def_back_pixel;

if (gtk_socket_id != 0)
{
GtkWidget *plug;


plug = gtk_plug_new_for_display(gdk_display_get_default(),
gtk_socket_id);
if (plug != NULL && gtk_plug_get_socket_window(GTK_PLUG(plug)) != NULL)
{
gui.mainwin = plug;
}
else
{
g_warning("Connection to GTK+ socket (ID %u) failed",
(unsigned int)gtk_socket_id);

gtk_socket_id = 0;
}
}

if (gtk_socket_id == 0)
{
#if defined(FEAT_GUI_GNOME)
if (using_gnome)
{
gui.mainwin = gnome_app_new("Vim", NULL);
#if defined(USE_XSMP)

xsmp_close();
#endif
}
else
#endif
gui.mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
}

gtk_widget_set_name(gui.mainwin, "vim-main-window");


gui.text_context = gtk_widget_create_pango_context(gui.mainwin);
pango_context_set_base_dir(gui.text_context, PANGO_DIRECTION_LTR);

gtk_container_set_border_width(GTK_CONTAINER(gui.mainwin), 0);
gtk_widget_add_events(gui.mainwin, GDK_VISIBILITY_NOTIFY_MASK);

g_signal_connect(G_OBJECT(gui.mainwin), "delete-event",
G_CALLBACK(&delete_event_cb), NULL);

g_signal_connect(G_OBJECT(gui.mainwin), "realize",
G_CALLBACK(&mainwin_realize), NULL);

g_signal_connect(G_OBJECT(gui.mainwin), "screen-changed",
G_CALLBACK(&mainwin_screen_changed_cb), NULL);

gui.accel_group = gtk_accel_group_new();
gtk_window_add_accel_group(GTK_WINDOW(gui.mainwin), gui.accel_group);


#if GTK_CHECK_VERSION(3,2,0)
vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
gtk_box_set_homogeneous(GTK_BOX(vbox), FALSE);
#else
vbox = gtk_vbox_new(FALSE, 0);
#endif

#if defined(FEAT_GUI_GNOME)
if (using_gnome)
{
#if defined(FEAT_MENU)

gnome_app_enable_layout_config(GNOME_APP(gui.mainwin), TRUE);
#endif
gnome_app_set_contents(GNOME_APP(gui.mainwin), vbox);
}
else
#endif
{
gtk_container_add(GTK_CONTAINER(gui.mainwin), vbox);
gtk_widget_show(vbox);
}

#if defined(FEAT_MENU)



gui.menubar = gtk_menu_bar_new();
gtk_widget_set_name(gui.menubar, "vim-menubar");


{
GtkSettings *gtk_settings;

gtk_settings = gtk_settings_get_for_screen(gdk_screen_get_default());
g_object_set(gtk_settings, "gtk-menu-bar-accel", NULL, NULL);
}


#if defined(FEAT_GUI_GNOME)
if (using_gnome)
{
BonoboDockItem *dockitem;

gnome_app_set_menus(GNOME_APP(gui.mainwin), GTK_MENU_BAR(gui.menubar));
dockitem = gnome_app_get_dock_item_by_name(GNOME_APP(gui.mainwin),
GNOME_APP_MENUBAR_NAME);

bonobo_dock_item_set_behavior(dockitem,
bonobo_dock_item_get_behavior(dockitem)
| BONOBO_DOCK_ITEM_BEH_NEVER_FLOATING);
gui.menubar_h = GTK_WIDGET(dockitem);
}
else
#endif 
{


gtk_widget_show(gui.menubar);
gtk_box_pack_start(GTK_BOX(vbox), gui.menubar, FALSE, FALSE, 0);
}
#endif 

#if defined(FEAT_TOOLBAR)




#if defined(USE_GTK3)



#else
gtk_rc_parse_string(
"style \"vim-toolbar-style\" {\n"
" GtkToolbar::button_relief = GTK_RELIEF_NONE\n"
"}\n"
"widget \"*.vim-toolbar\" style \"vim-toolbar-style\"\n");
#endif
gui.toolbar = gtk_toolbar_new();
gtk_widget_set_name(gui.toolbar, "vim-toolbar");
set_toolbar_style(GTK_TOOLBAR(gui.toolbar));

#if defined(FEAT_GUI_GNOME)
if (using_gnome)
{
BonoboDockItem *dockitem;

gnome_app_set_toolbar(GNOME_APP(gui.mainwin), GTK_TOOLBAR(gui.toolbar));
dockitem = gnome_app_get_dock_item_by_name(GNOME_APP(gui.mainwin),
GNOME_APP_TOOLBAR_NAME);
gui.toolbar_h = GTK_WIDGET(dockitem);


bonobo_dock_item_set_behavior(dockitem,
bonobo_dock_item_get_behavior(dockitem)
| BONOBO_DOCK_ITEM_BEH_NEVER_FLOATING);
gtk_container_set_border_width(GTK_CONTAINER(gui.toolbar), 0);
}
else
#endif 
{
if (vim_strchr(p_go, GO_TOOLBAR) != NULL
&& (toolbar_flags & (TOOLBAR_TEXT | TOOLBAR_ICONS)))
gtk_widget_show(gui.toolbar);
gtk_box_pack_start(GTK_BOX(vbox), gui.toolbar, FALSE, FALSE, 0);
}
#endif 

#if defined(FEAT_GUI_TABLINE)




gui.tabline = gtk_notebook_new();
gtk_widget_show(gui.tabline);
gtk_box_pack_start(GTK_BOX(vbox), gui.tabline, FALSE, FALSE, 0);
gtk_notebook_set_show_border(GTK_NOTEBOOK(gui.tabline), FALSE);
gtk_notebook_set_show_tabs(GTK_NOTEBOOK(gui.tabline), FALSE);
gtk_notebook_set_scrollable(GTK_NOTEBOOK(gui.tabline), TRUE);
#if !GTK_CHECK_VERSION(3,0,0)
gtk_notebook_set_tab_border(GTK_NOTEBOOK(gui.tabline), FALSE);
#endif

#if !GTK_CHECK_VERSION(3,0,0)
tabline_tooltip = gtk_tooltips_new();
gtk_tooltips_enable(GTK_TOOLTIPS(tabline_tooltip));
#endif

{
GtkWidget *page, *label, *event_box;


#if GTK_CHECK_VERSION(3,2,0)
page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
gtk_box_set_homogeneous(GTK_BOX(page), FALSE);
#else
page = gtk_vbox_new(FALSE, 0);
#endif
gtk_widget_show(page);
gtk_container_add(GTK_CONTAINER(gui.tabline), page);
label = gtk_label_new("-Empty-");
gtk_widget_show(label);
event_box = gtk_event_box_new();
gtk_widget_show(event_box);
g_object_set_data(G_OBJECT(event_box), "tab_num", GINT_TO_POINTER(1L));
#if !GTK_CHECK_VERSION(3,14,0)
gtk_misc_set_padding(GTK_MISC(label), 2, 2);
#endif
gtk_container_add(GTK_CONTAINER(event_box), label);
gtk_notebook_set_tab_label(GTK_NOTEBOOK(gui.tabline), page, event_box);
#if GTK_CHECK_VERSION(2,10,0)
gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(gui.tabline), page, TRUE);
#endif
}

g_signal_connect(G_OBJECT(gui.tabline), "switch-page",
G_CALLBACK(on_select_tab), NULL);
#if GTK_CHECK_VERSION(2,10,0)
g_signal_connect(G_OBJECT(gui.tabline), "page-reordered",
G_CALLBACK(on_tab_reordered), NULL);
#endif


tabline_menu = create_tabline_menu();
g_signal_connect_swapped(G_OBJECT(gui.tabline), "button-press-event",
G_CALLBACK(on_tabline_menu), G_OBJECT(tabline_menu));
#endif 

gui.formwin = gtk_form_new();
gtk_container_set_border_width(GTK_CONTAINER(gui.formwin), 0);
#if !GTK_CHECK_VERSION(3,0,0)
gtk_widget_set_events(gui.formwin, GDK_EXPOSURE_MASK);
#endif

gui.drawarea = gtk_drawing_area_new();
#if GTK_CHECK_VERSION(3,22,2)
gtk_widget_set_name(gui.drawarea, "vim-gui-drawarea");
#endif
#if GTK_CHECK_VERSION(3,0,0)
gui.surface = NULL;
gui.by_signal = FALSE;
#endif


gtk_widget_set_events(gui.drawarea,
GDK_EXPOSURE_MASK |
GDK_ENTER_NOTIFY_MASK |
GDK_LEAVE_NOTIFY_MASK |
GDK_BUTTON_PRESS_MASK |
GDK_BUTTON_RELEASE_MASK |
GDK_SCROLL_MASK |
GDK_KEY_PRESS_MASK |
GDK_KEY_RELEASE_MASK |
GDK_POINTER_MOTION_MASK |
GDK_POINTER_MOTION_HINT_MASK);

gtk_widget_show(gui.drawarea);
gtk_form_put(GTK_FORM(gui.formwin), gui.drawarea, 0, 0);
gtk_widget_show(gui.formwin);
gtk_box_pack_start(GTK_BOX(vbox), gui.formwin, TRUE, TRUE, 0);



g_signal_connect((gtk_socket_id == 0) ? G_OBJECT(gui.mainwin)
: G_OBJECT(gui.drawarea),
"key-press-event",
G_CALLBACK(key_press_event), NULL);
#if defined(FEAT_XIM) || GTK_CHECK_VERSION(3,0,0)


g_signal_connect((gtk_socket_id == 0) ? G_OBJECT(gui.mainwin)
: G_OBJECT(gui.drawarea),
"key-release-event",
G_CALLBACK(&key_release_event), NULL);
#endif
g_signal_connect(G_OBJECT(gui.drawarea), "realize",
G_CALLBACK(drawarea_realize_cb), NULL);
g_signal_connect(G_OBJECT(gui.drawarea), "unrealize",
G_CALLBACK(drawarea_unrealize_cb), NULL);
#if GTK_CHECK_VERSION(3,0,0)
g_signal_connect(G_OBJECT(gui.drawarea), "configure-event",
G_CALLBACK(drawarea_configure_event_cb), NULL);
#endif
#if GTK_CHECK_VERSION(3,22,2)
g_signal_connect_after(G_OBJECT(gui.drawarea), "style-updated",
G_CALLBACK(&drawarea_style_updated_cb), NULL);
#else
g_signal_connect_after(G_OBJECT(gui.drawarea), "style-set",
G_CALLBACK(&drawarea_style_set_cb), NULL);
#endif

#if !GTK_CHECK_VERSION(3,0,0)
gui.visibility = GDK_VISIBILITY_UNOBSCURED;
#endif

#if !defined(USE_GNOME_SESSION)
wm_protocols_atom = gdk_atom_intern("WM_PROTOCOLS", FALSE);
save_yourself_atom = gdk_atom_intern("WM_SAVE_YOURSELF", FALSE);
#endif

if (gtk_socket_id != 0)

gtk_widget_set_can_focus(gui.drawarea, TRUE);




vim_atom = gdk_atom_intern(VIM_ATOM_NAME, FALSE);
vimenc_atom = gdk_atom_intern(VIMENC_ATOM_NAME, FALSE);
clip_star.gtk_sel_atom = GDK_SELECTION_PRIMARY;
clip_plus.gtk_sel_atom = gdk_atom_intern("CLIPBOARD", FALSE);




gui.border_offset = gui.border_width;

#if GTK_CHECK_VERSION(3,0,0)
g_signal_connect(G_OBJECT(gui.drawarea), "draw",
G_CALLBACK(draw_event), NULL);
#else
gtk_signal_connect(GTK_OBJECT(gui.mainwin), "visibility_notify_event",
GTK_SIGNAL_FUNC(visibility_event), NULL);
gtk_signal_connect(GTK_OBJECT(gui.drawarea), "expose_event",
GTK_SIGNAL_FUNC(expose_event), NULL);
#endif





if (vim_strchr(p_go, GO_POINTER) != NULL)
{
g_signal_connect(G_OBJECT(gui.drawarea), "leave-notify-event",
G_CALLBACK(leave_notify_event), NULL);
g_signal_connect(G_OBJECT(gui.drawarea), "enter-notify-event",
G_CALLBACK(enter_notify_event), NULL);
}




if (gtk_socket_id == 0)
{
g_signal_connect(G_OBJECT(gui.mainwin), "focus-out-event",
G_CALLBACK(focus_out_event), NULL);
g_signal_connect(G_OBJECT(gui.mainwin), "focus-in-event",
G_CALLBACK(focus_in_event), NULL);
}
else
{
g_signal_connect(G_OBJECT(gui.drawarea), "focus-out-event",
G_CALLBACK(focus_out_event), NULL);
g_signal_connect(G_OBJECT(gui.drawarea), "focus-in-event",
G_CALLBACK(focus_in_event), NULL);
#if defined(FEAT_GUI_TABLINE)
g_signal_connect(G_OBJECT(gui.tabline), "focus-out-event",
G_CALLBACK(focus_out_event), NULL);
g_signal_connect(G_OBJECT(gui.tabline), "focus-in-event",
G_CALLBACK(focus_in_event), NULL);
#endif 
}

g_signal_connect(G_OBJECT(gui.drawarea), "motion-notify-event",
G_CALLBACK(motion_notify_event), NULL);
g_signal_connect(G_OBJECT(gui.drawarea), "button-press-event",
G_CALLBACK(button_press_event), NULL);
g_signal_connect(G_OBJECT(gui.drawarea), "button-release-event",
G_CALLBACK(button_release_event), NULL);
g_signal_connect(G_OBJECT(gui.drawarea), "scroll-event",
G_CALLBACK(&scroll_event), NULL);




g_signal_connect(G_OBJECT(gui.drawarea), "selection-clear-event",
G_CALLBACK(selection_clear_event), NULL);
g_signal_connect(G_OBJECT(gui.drawarea), "selection-received",
G_CALLBACK(selection_received_cb), NULL);

gui_gtk_set_selection_targets();

g_signal_connect(G_OBJECT(gui.drawarea), "selection-get",
G_CALLBACK(selection_get_cb), NULL);


gui.in_focus = FALSE;


{
GtkSettings *gtk_settings =
gtk_settings_get_for_screen(gdk_screen_get_default());

g_signal_connect(gtk_settings, "notify::gtk-xft-dpi",
G_CALLBACK(gtk_settings_xft_dpi_changed_cb), NULL);
}

return OK;
}

#if defined(USE_GNOME_SESSION) || defined(PROTO)




void
gui_mch_forked(void)
{
if (using_gnome)
{
GnomeClient *client;

client = gnome_master_client();

if (client != NULL)
gnome_client_set_process_id(client, getpid());
}
}
#endif 

#if GTK_CHECK_VERSION(3,0,0)
static GdkRGBA
color_to_rgba(guicolor_T color)
{
GdkRGBA rgba;
rgba.red = ((color & 0xff0000) >> 16) / 255.0;
rgba.green = ((color & 0xff00) >> 8) / 255.0;
rgba.blue = ((color & 0xff)) / 255.0;
rgba.alpha = 1.0;
return rgba;
}

static void
set_cairo_source_rgba_from_color(cairo_t *cr, guicolor_T color)
{
const GdkRGBA rgba = color_to_rgba(color);
cairo_set_source_rgba(cr, rgba.red, rgba.green, rgba.blue, rgba.alpha);
}
#endif 






void
gui_mch_new_colors(void)
{
if (gui.drawarea != NULL && gtk_widget_get_window(gui.drawarea) != NULL)
{
#if !GTK_CHECK_VERSION(3,22,2)
GdkWindow * const da_win = gtk_widget_get_window(gui.drawarea);
#endif

#if GTK_CHECK_VERSION(3,22,2)
GtkStyleContext * const context
= gtk_widget_get_style_context(gui.drawarea);
GtkCssProvider * const provider = gtk_css_provider_new();
gchar * const css = g_strdup_printf(
"widget#vim-gui-drawarea {\n"
" background-color: #%.2lx%.2lx%.2lx;\n"
"}\n",
(gui.back_pixel >> 16) & 0xff,
(gui.back_pixel >> 8) & 0xff,
gui.back_pixel & 0xff);

gtk_css_provider_load_from_data(provider, css, -1, NULL);
gtk_style_context_add_provider(context,
GTK_STYLE_PROVIDER(provider), G_MAXUINT);

g_free(css);
g_object_unref(provider);
#elif GTK_CHECK_VERSION(3,4,0) 
GdkRGBA rgba;

rgba = color_to_rgba(gui.back_pixel);
{
cairo_pattern_t * const pat = cairo_pattern_create_rgba(
rgba.red, rgba.green, rgba.blue, rgba.alpha);
if (pat != NULL)
{
gdk_window_set_background_pattern(da_win, pat);
cairo_pattern_destroy(pat);
}
else
gdk_window_set_background_rgba(da_win, &rgba);
}
#else 
GdkColor color = { 0, 0, 0, 0 };

color.pixel = gui.back_pixel;
gdk_window_set_background(da_win, &color);
#endif 
}
}




static gint
form_configure_event(GtkWidget *widget UNUSED,
GdkEventConfigure *event,
gpointer data UNUSED)
{
int usable_height = event->height;

#if GTK_CHECK_VERSION(3,22,2) && !GTK_CHECK_VERSION(3,22,4)



















if (event->window != gtk_widget_get_window(gui.formwin))
return TRUE;
#endif




if (gtk_socket_id != 0)
usable_height -= (gui.char_height - (gui.char_height/2)); 

gtk_form_freeze(GTK_FORM(gui.formwin));
gui_resize_shell(event->width, usable_height);
gtk_form_thaw(GTK_FORM(gui.formwin));

return TRUE;
}






static void
mainwin_destroy_cb(GObject *object UNUSED, gpointer data UNUSED)
{

full_screen = FALSE;

gui.mainwin = NULL;
gui.drawarea = NULL;

if (!exiting) 
{
vim_strncpy(IObuff,
(char_u *)_("Vim: Main window unexpectedly destroyed\n"),
IOSIZE - 1);
preserve_exit();
}
#if defined(USE_GRESOURCE)
gui_gtk_unregister_resource();
#endif
}














static timeout_cb_type
check_startup_plug_hints(gpointer data UNUSED)
{
if (init_window_hints_state == 1)
{

init_window_hints_state = 0;
update_window_manager_hints(0, 0);
return FALSE; 
}


init_window_hints_state = 1;
return TRUE;
}




int
gui_mch_open(void)
{
guicolor_T fg_pixel = INVALCOLOR;
guicolor_T bg_pixel = INVALCOLOR;
guint pixel_width;
guint pixel_height;






if (role_argument != NULL)
{
gtk_window_set_role(GTK_WINDOW(gui.mainwin), role_argument);
}
else
{
char *role;


role = g_strdup_printf("vim-%u-%u-%u",
(unsigned)mch_get_pid(),
(unsigned)g_random_int(),
(unsigned)time(NULL));

gtk_window_set_role(GTK_WINDOW(gui.mainwin), role);
g_free(role);
}

if (gui_win_x != -1 && gui_win_y != -1)
gtk_window_move(GTK_WINDOW(gui.mainwin), gui_win_x, gui_win_y);


if (gui.geom != NULL)
{
int mask;
unsigned int w, h;
int x = 0;
int y = 0;

mask = XParseGeometry((char *)gui.geom, &x, &y, &w, &h);

if (mask & WidthValue)
Columns = w;
if (mask & HeightValue)
{
if (p_window > (long)h - 1 || !option_was_set((char_u *)"window"))
p_window = h - 1;
Rows = h;
}
limit_screen_size();

pixel_width = (guint)(gui_get_base_width() + Columns * gui.char_width);
pixel_height = (guint)(gui_get_base_height() + Rows * gui.char_height);

pixel_width += get_menu_tool_width();
pixel_height += get_menu_tool_height();

if (mask & (XValue | YValue))
{
int ww, hh;
gui_mch_get_screen_dimensions(&ww, &hh);
hh += p_ghr + get_menu_tool_height();
ww += get_menu_tool_width();
if (mask & XNegative)
x += ww - pixel_width;
if (mask & YNegative)
y += hh - pixel_height;
gtk_window_move(GTK_WINDOW(gui.mainwin), x, y);
}
VIM_CLEAR(gui.geom);





if (gtk_socket_id != 0 && (mask & WidthValue || mask & HeightValue))
{
update_window_manager_hints(pixel_width, pixel_height);
init_window_hints_state = 1;
timeout_add(1000, check_startup_plug_hints, NULL);
}
}

pixel_width = (guint)(gui_get_base_width() + Columns * gui.char_width);
pixel_height = (guint)(gui_get_base_height() + Rows * gui.char_height);


if (gtk_socket_id == 0)
gtk_window_resize(GTK_WINDOW(gui.mainwin), pixel_width, pixel_height);
update_window_manager_hints(0, 0);

if (foreground_argument != NULL)
fg_pixel = gui_get_color((char_u *)foreground_argument);
if (fg_pixel == INVALCOLOR)
fg_pixel = gui_get_color((char_u *)"Black");

if (background_argument != NULL)
bg_pixel = gui_get_color((char_u *)background_argument);
if (bg_pixel == INVALCOLOR)
bg_pixel = gui_get_color((char_u *)"White");

if (found_reverse_arg)
{
gui.def_norm_pixel = bg_pixel;
gui.def_back_pixel = fg_pixel;
}
else
{
gui.def_norm_pixel = fg_pixel;
gui.def_back_pixel = bg_pixel;
}



set_normal_colors();


gui_check_colors();



highlight_gui_started(); 

g_signal_connect(G_OBJECT(gui.mainwin), "destroy",
G_CALLBACK(mainwin_destroy_cb), NULL);











g_signal_connect(G_OBJECT(gui.formwin), "configure-event",
G_CALLBACK(form_configure_event), NULL);

#if defined(FEAT_DND)

gui_gtk_set_dnd_targets();

g_signal_connect(G_OBJECT(gui.drawarea), "drag-data-received",
G_CALLBACK(drag_data_received_cb), NULL);
#endif



if (found_iconic_arg && gtk_socket_id == 0)
gui_mch_iconify();

{
#if defined(FEAT_GUI_GNOME) && defined(FEAT_MENU)
unsigned long menu_handler = 0;
#if defined(FEAT_TOOLBAR)
unsigned long tool_handler = 0;
#endif








if (using_gnome && vim_strchr(p_go, GO_MENUS) == NULL)
menu_handler = g_signal_connect_after(gui.menubar_h, "show",
G_CALLBACK(&gtk_widget_hide),
NULL);
#if defined(FEAT_TOOLBAR)
if (using_gnome && vim_strchr(p_go, GO_TOOLBAR) == NULL
&& (toolbar_flags & (TOOLBAR_TEXT | TOOLBAR_ICONS)))
tool_handler = g_signal_connect_after(gui.toolbar_h, "show",
G_CALLBACK(&gtk_widget_hide),
NULL);
#endif
#endif
gtk_widget_show(gui.mainwin);

#if defined(FEAT_GUI_GNOME) && defined(FEAT_MENU)
if (menu_handler != 0)
g_signal_handler_disconnect(gui.menubar_h, menu_handler);
#if defined(FEAT_TOOLBAR)
if (tool_handler != 0)
g_signal_handler_disconnect(gui.toolbar_h, tool_handler);
#endif
#endif
}

return OK;
}


void
gui_mch_exit(int rc UNUSED)
{
if (gui.mainwin != NULL)
gtk_widget_destroy(gui.mainwin);
}




int
gui_mch_get_winpos(int *x, int *y)
{
gtk_window_get_position(GTK_WINDOW(gui.mainwin), x, y);
return OK;
}





void
gui_mch_set_winpos(int x, int y)
{
gtk_window_move(GTK_WINDOW(gui.mainwin), x, y);
}

#if !GTK_CHECK_VERSION(3,0,0)
#if 0
static int resize_idle_installed = FALSE;













static gboolean
force_shell_resize_idle(gpointer data)
{
if (gui.mainwin != NULL
&& GTK_WIDGET_REALIZED(gui.mainwin)
&& GTK_WIDGET_VISIBLE(gui.mainwin))
{
int width;
int height;

gtk_window_get_size(GTK_WINDOW(gui.mainwin), &width, &height);

width -= get_menu_tool_width();
height -= get_menu_tool_height();

gui_resize_shell(width, height);
}

resize_idle_installed = FALSE;
return FALSE; 
}
#endif
#endif 




int
gui_mch_maximized(void)
{
return (gui.mainwin != NULL && gtk_widget_get_window(gui.mainwin) != NULL
&& (gdk_window_get_state(gtk_widget_get_window(gui.mainwin))
& GDK_WINDOW_STATE_MAXIMIZED));
}




void
gui_mch_unmaximize(void)
{
if (gui.mainwin != NULL)
gtk_window_unmaximize(GTK_WINDOW(gui.mainwin));
}






void
gui_mch_newfont(void)
{
int w, h;

gtk_window_get_size(GTK_WINDOW(gui.mainwin), &w, &h);
w -= get_menu_tool_width();
h -= get_menu_tool_height();
gui_resize_shell(w, h);
}




void
gui_mch_set_shellsize(int width, int height,
int min_width UNUSED, int min_height UNUSED,
int base_width UNUSED, int base_height UNUSED,
int direction UNUSED)
{

gui_mch_update();


if (gtk_socket_id == 0)
update_window_manager_hints(0, 0);




width += get_menu_tool_width();
height += get_menu_tool_height();

if (gtk_socket_id == 0)
gtk_window_resize(GTK_WINDOW(gui.mainwin), width, height);
else
update_window_manager_hints(width, height);

#if !GTK_CHECK_VERSION(3,0,0)
#if 0
if (!resize_idle_installed)
{
g_idle_add_full(GDK_PRIORITY_EVENTS + 10,
&force_shell_resize_idle, NULL, NULL);
resize_idle_installed = TRUE;
}
#endif
#endif 







gui_mch_update();
}

void
gui_gtk_get_screen_geom_of_win(
GtkWidget *wid,
int *screen_x,
int *screen_y,
int *width,
int *height)
{
GdkRectangle geometry;
GdkWindow *win = gtk_widget_get_window(wid);
#if GTK_CHECK_VERSION(3,22,0)
GdkDisplay *dpy = gtk_widget_get_display(wid);
GdkMonitor *monitor = gdk_display_get_monitor_at_window(dpy, win);

gdk_monitor_get_geometry(monitor, &geometry);
#else
GdkScreen* screen;
int monitor;

if (wid != NULL && gtk_widget_has_screen(wid))
screen = gtk_widget_get_screen(wid);
else
screen = gdk_screen_get_default();
monitor = gdk_screen_get_monitor_at_window(screen, win);
gdk_screen_get_monitor_geometry(screen, monitor, &geometry);
#endif
*screen_x = geometry.x;
*screen_y = geometry.y;
*width = geometry.width;
*height = geometry.height;
}






void
gui_mch_get_screen_dimensions(int *screen_w, int *screen_h)
{
int x, y;

gui_gtk_get_screen_geom_of_win(gui.mainwin, &x, &y, screen_w, screen_h);



*screen_h -= p_ghr;







*screen_w -= get_menu_tool_width();
*screen_h -= get_menu_tool_height();
}

#if defined(FEAT_TITLE) || defined(PROTO)
void
gui_mch_settitle(char_u *title, char_u *icon UNUSED)
{
if (title != NULL && output_conv.vc_type != CONV_NONE)
title = string_convert(&output_conv, title, NULL);

gtk_window_set_title(GTK_WINDOW(gui.mainwin), (const char *)title);

if (output_conv.vc_type != CONV_NONE)
vim_free(title);
}
#endif 

#if defined(FEAT_MENU) || defined(PROTO)
void
gui_mch_enable_menu(int showit)
{
GtkWidget *widget;

#if defined(FEAT_GUI_GNOME)
if (using_gnome)
widget = gui.menubar_h;
else
#endif
widget = gui.menubar;


if (!showit != !gtk_widget_get_visible(widget) && !gui.starting)
{
if (showit)
gtk_widget_show(widget);
else
gtk_widget_hide(widget);

update_window_manager_hints(0, 0);
}
}
#endif 

#if defined(FEAT_TOOLBAR) || defined(PROTO)
void
gui_mch_show_toolbar(int showit)
{
GtkWidget *widget;

if (gui.toolbar == NULL)
return;

#if defined(FEAT_GUI_GNOME)
if (using_gnome)
widget = gui.toolbar_h;
else
#endif
widget = gui.toolbar;

if (showit)
set_toolbar_style(GTK_TOOLBAR(gui.toolbar));

if (!showit != !gtk_widget_get_visible(widget))
{
if (showit)
gtk_widget_show(widget);
else
gtk_widget_hide(widget);

update_window_manager_hints(0, 0);
}
}
#endif 








static int
is_cjk_font(PangoFontDescription *font_desc)
{
static const char * const cjk_langs[] =
{"zh_CN", "zh_TW", "zh_HK", "ja", "ko"};

PangoFont *font;
unsigned i;
int is_cjk = FALSE;

font = pango_context_load_font(gui.text_context, font_desc);

if (font == NULL)
return FALSE;

for (i = 0; !is_cjk && i < G_N_ELEMENTS(cjk_langs); ++i)
{
PangoCoverage *coverage;
gunichar uc;



coverage = pango_font_get_coverage(
font, pango_language_from_string(cjk_langs[i]));

if (coverage != NULL)
{
uc = (cjk_langs[i][0] == 'k') ? 0xAC00 : 0x4E00;
is_cjk = (pango_coverage_get(coverage, uc) == PANGO_COVERAGE_EXACT);
pango_coverage_unref(coverage);
}
}

g_object_unref(font);

return is_cjk;
}




int
gui_mch_adjust_charheight(void)
{
PangoFontMetrics *metrics;
int ascent;
int descent;

metrics = pango_context_get_metrics(gui.text_context, gui.norm_font,
pango_context_get_language(gui.text_context));
ascent = pango_font_metrics_get_ascent(metrics);
descent = pango_font_metrics_get_descent(metrics);

pango_font_metrics_unref(metrics);

gui.char_height = (ascent + descent + PANGO_SCALE - 1) / PANGO_SCALE
+ p_linespace;

gui.char_ascent = PANGO_PIXELS(ascent + p_linespace * PANGO_SCALE / 2);



gui.char_ascent = MAX(gui.char_ascent, 0);
gui.char_height = MAX(gui.char_height, gui.char_ascent + 1);

return OK;
}

#if GTK_CHECK_VERSION(3,0,0)

static gboolean
font_filter(const PangoFontFamily *family,
const PangoFontFace *face UNUSED,
gpointer data UNUSED)
{
return pango_font_family_is_monospace((PangoFontFamily *)family);
}
#endif










char_u *
gui_mch_font_dialog(char_u *oldval)
{
GtkWidget *dialog;
int response;
char_u *fontname = NULL;
char_u *oldname;

#if GTK_CHECK_VERSION(3,2,0)
dialog = gtk_font_chooser_dialog_new(NULL, NULL);
gtk_font_chooser_set_filter_func(GTK_FONT_CHOOSER(dialog), font_filter,
NULL, NULL);
#else
dialog = gtk_font_selection_dialog_new(NULL);
#endif

gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gui.mainwin));
gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

if (oldval != NULL && oldval[0] != NUL)
{
if (output_conv.vc_type != CONV_NONE)
oldname = string_convert(&output_conv, oldval, NULL);
else
oldname = oldval;



if (!vim_isdigit(oldname[STRLEN(oldname) - 1]))
{
char_u *p = vim_strnsave(oldname, STRLEN(oldname) + 3);

if (p != NULL)
{
STRCPY(p + STRLEN(p), " 10");
if (oldname != oldval)
vim_free(oldname);
oldname = p;
}
}

#if GTK_CHECK_VERSION(3,2,0)
gtk_font_chooser_set_font(
GTK_FONT_CHOOSER(dialog), (const gchar *)oldname);
#else
gtk_font_selection_dialog_set_font_name(
GTK_FONT_SELECTION_DIALOG(dialog), (const char *)oldname);
#endif

if (oldname != oldval)
vim_free(oldname);
}
else
#if GTK_CHECK_VERSION(3,2,0)
gtk_font_chooser_set_font(
GTK_FONT_CHOOSER(dialog), DEFAULT_FONT);
#else
gtk_font_selection_dialog_set_font_name(
GTK_FONT_SELECTION_DIALOG(dialog), DEFAULT_FONT);
#endif

response = gtk_dialog_run(GTK_DIALOG(dialog));

if (response == GTK_RESPONSE_OK)
{
char *name;

#if GTK_CHECK_VERSION(3,2,0)
name = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dialog));
#else
name = gtk_font_selection_dialog_get_font_name(
GTK_FONT_SELECTION_DIALOG(dialog));
#endif
if (name != NULL)
{
char_u *p;



p = vim_strsave_escaped((char_u *)name, (char_u *)",");
g_free(name);
if (p != NULL && input_conv.vc_type != CONV_NONE)
{
fontname = string_convert(&input_conv, p, NULL);
vim_free(p);
}
else
fontname = p;
}
}

if (response != GTK_RESPONSE_NONE)
gtk_widget_destroy(dialog);

return fontname;
}












static void
get_styled_font_variants(void)
{
PangoFontDescription *bold_font_desc;
PangoFont *plain_font;
PangoFont *bold_font;

gui.font_can_bold = FALSE;

plain_font = pango_context_load_font(gui.text_context, gui.norm_font);

if (plain_font == NULL)
return;

bold_font_desc = pango_font_description_copy_static(gui.norm_font);
pango_font_description_set_weight(bold_font_desc, PANGO_WEIGHT_BOLD);

bold_font = pango_context_load_font(gui.text_context, bold_font_desc);





if (bold_font != NULL)
{
gui.font_can_bold = (bold_font != plain_font);
g_object_unref(bold_font);
}

pango_font_description_free(bold_font_desc);
g_object_unref(plain_font);
}

static PangoEngineShape *default_shape_engine = NULL;






static void
ascii_glyph_table_init(void)
{
char_u ascii_chars[2 * 128];
PangoAttrList *attr_list;
GList *item_list;
int i;

if (gui.ascii_glyphs != NULL)
pango_glyph_string_free(gui.ascii_glyphs);
if (gui.ascii_font != NULL)
g_object_unref(gui.ascii_font);

gui.ascii_glyphs = NULL;
gui.ascii_font = NULL;



for (i = 0; i < 128; ++i)
{
if (i >= 32 && i < 127)
ascii_chars[2 * i] = i;
else
ascii_chars[2 * i] = '?';
ascii_chars[2 * i + 1] = ' ';
}

attr_list = pango_attr_list_new();
item_list = pango_itemize(gui.text_context, (const char *)ascii_chars,
0, sizeof(ascii_chars), attr_list, NULL);

if (item_list != NULL && item_list->next == NULL) 
{
PangoItem *item;
int width;

item = (PangoItem *)item_list->data;
width = gui.char_width * PANGO_SCALE;


default_shape_engine = item->analysis.shape_engine;

gui.ascii_font = item->analysis.font;
g_object_ref(gui.ascii_font);

gui.ascii_glyphs = pango_glyph_string_new();

pango_shape((const char *)ascii_chars, sizeof(ascii_chars),
&item->analysis, gui.ascii_glyphs);

g_return_if_fail(gui.ascii_glyphs->num_glyphs == sizeof(ascii_chars));

for (i = 0; i < gui.ascii_glyphs->num_glyphs; ++i)
{
PangoGlyphGeometry *geom;

geom = &gui.ascii_glyphs->glyphs[i].geometry;
geom->x_offset += MAX(0, width - geom->width) / 2;
geom->width = width;
}
}

g_list_foreach(item_list, (GFunc)&pango_item_free, NULL);
g_list_free(item_list);
pango_attr_list_unref(attr_list);
}





int
gui_mch_init_font(char_u *font_name, int fontset UNUSED)
{
PangoFontDescription *font_desc;
PangoLayout *layout;
int width;



if (font_name == NULL)
font_name = (char_u *)DEFAULT_FONT;

font_desc = gui_mch_get_font(font_name, FALSE);

if (font_desc == NULL)
return FAIL;

gui_mch_free_font(gui.norm_font);
gui.norm_font = font_desc;

pango_context_set_font_description(gui.text_context, font_desc);

layout = pango_layout_new(gui.text_context);
pango_layout_set_text(layout, "MW", 2);
pango_layout_get_size(layout, &width, NULL);



















if (is_cjk_font(gui.norm_font))
{
int cjk_width;


pango_layout_set_text(layout, "\344\270\200\344\272\214", -1);
pango_layout_get_size(layout, &cjk_width, NULL);

if (width == cjk_width) 
width /= 2;
}
g_object_unref(layout);

gui.char_width = (width / 2 + PANGO_SCALE - 1) / PANGO_SCALE;


if (gui.char_width <= 0)
gui.char_width = 8;

gui_mch_adjust_charheight();


hl_set_font_name(font_name);

get_styled_font_variants();
ascii_glyph_table_init();


if (gui.wide_font != NULL
&& pango_font_description_equal(gui.norm_font, gui.wide_font))
{
pango_font_description_free(gui.wide_font);
gui.wide_font = NULL;
}

if (gui_mch_maximized())
{


gui_mch_newfont();
}
else
{

update_window_manager_hints(0, 0);
}

return OK;
}





GuiFont
gui_mch_get_font(char_u *name, int report_error)
{
PangoFontDescription *font;


if (!gui.in_use || name == NULL)
return NULL;

if (output_conv.vc_type != CONV_NONE)
{
char_u *buf;

buf = string_convert(&output_conv, name, NULL);
if (buf != NULL)
{
font = pango_font_description_from_string((const char *)buf);
vim_free(buf);
}
else
font = NULL;
}
else
font = pango_font_description_from_string((const char *)name);

if (font != NULL)
{
PangoFont *real_font;


if (pango_font_description_get_size(font) <= 0)
pango_font_description_set_size(font, 10 * PANGO_SCALE);

real_font = pango_context_load_font(gui.text_context, font);

if (real_font == NULL)
{
pango_font_description_free(font);
font = NULL;
}
else
g_object_unref(real_font);
}

if (font == NULL)
{
if (report_error)
semsg(_((char *)e_font), name);
return NULL;
}

return font;
}

#if defined(FEAT_EVAL) || defined(PROTO)



char_u *
gui_mch_get_fontname(GuiFont font, char_u *name UNUSED)
{
if (font != NOFONT)
{
char *pangoname = pango_font_description_to_string(font);

if (pangoname != NULL)
{
char_u *s = vim_strsave((char_u *)pangoname);

g_free(pangoname);
return s;
}
}
return NULL;
}
#endif




void
gui_mch_free_font(GuiFont font)
{
if (font != NOFONT)
pango_font_description_free(font);
}






guicolor_T
gui_mch_get_color(char_u *name)
{
guicolor_T color = INVALCOLOR;

if (!gui.in_use) 
return color;

if (name != NULL)
color = gui_get_color_cmn(name);

#if GTK_CHECK_VERSION(3,0,0)
return color;
#else
if (color == INVALCOLOR)
return INVALCOLOR;

return gui_mch_get_rgb_color(
(color & 0xff0000) >> 16,
(color & 0xff00) >> 8,
color & 0xff);
#endif
}





guicolor_T
gui_mch_get_rgb_color(int r, int g, int b)
{
#if GTK_CHECK_VERSION(3,0,0)
return gui_get_rgb_color_cmn(r, g, b);
#else
GdkColor gcolor;
int ret;

gcolor.red = (guint16)(r / 255.0 * 65535 + 0.5);
gcolor.green = (guint16)(g / 255.0 * 65535 + 0.5);
gcolor.blue = (guint16)(b / 255.0 * 65535 + 0.5);

ret = gdk_colormap_alloc_color(gtk_widget_get_colormap(gui.drawarea),
&gcolor, FALSE, TRUE);

return ret != 0 ? (guicolor_T)gcolor.pixel : INVALCOLOR;
#endif
}




void
gui_mch_set_fg_color(guicolor_T color)
{
#if GTK_CHECK_VERSION(3,0,0)
*gui.fgcolor = color_to_rgba(color);
#else
gui.fgcolor->pixel = (unsigned long)color;
#endif
}




void
gui_mch_set_bg_color(guicolor_T color)
{
#if GTK_CHECK_VERSION(3,0,0)
*gui.bgcolor = color_to_rgba(color);
#else
gui.bgcolor->pixel = (unsigned long)color;
#endif
}




void
gui_mch_set_sp_color(guicolor_T color)
{
#if GTK_CHECK_VERSION(3,0,0)
*gui.spcolor = color_to_rgba(color);
#else
gui.spcolor->pixel = (unsigned long)color;
#endif
}




#define INSERT_PANGO_ATTR(Attribute, AttrList, Start, End) G_STMT_START{ PangoAttribute *tmp_attr_; tmp_attr_ = (Attribute); tmp_attr_->start_index = (Start); tmp_attr_->end_index = (End); pango_attr_list_insert((AttrList), tmp_attr_); }G_STMT_END








static void
apply_wide_font_attr(char_u *s, int len, PangoAttrList *attr_list)
{
char_u *start = NULL;
char_u *p;
int uc;

for (p = s; p < s + len; p += utf_byte2len(*p))
{
uc = utf_ptr2char(p);

if (start == NULL)
{
if (uc >= 0x80 && utf_char2cells(uc) == 2)
start = p;
}
else if (uc < 0x80 
|| (utf_char2cells(uc) != 2 && !utf_iscomposing(uc)))
{
INSERT_PANGO_ATTR(pango_attr_font_desc_new(gui.wide_font),
attr_list, start - s, p - s);
start = NULL;
}
}

if (start != NULL)
INSERT_PANGO_ATTR(pango_attr_font_desc_new(gui.wide_font),
attr_list, start - s, len);
}

static int
count_cluster_cells(char_u *s, PangoItem *item,
PangoGlyphString* glyphs, int i,
int *cluster_width,
int *last_glyph_rbearing)
{
char_u *p;
int next; 
int start, end; 
int width; 
int uc;
int cellcount = 0;

width = glyphs->glyphs[i].geometry.width;

for (next = i + 1; next < glyphs->num_glyphs; ++next)
{
if (glyphs->glyphs[next].attr.is_cluster_start)
break;
else if (glyphs->glyphs[next].geometry.width > width)
width = glyphs->glyphs[next].geometry.width;
}

start = item->offset + glyphs->log_clusters[i];
end = item->offset + ((next < glyphs->num_glyphs) ?
glyphs->log_clusters[next] : item->length);

for (p = s + start; p < s + end; p += utf_byte2len(*p))
{
uc = utf_ptr2char(p);
if (uc < 0x80)
++cellcount;
else if (!utf_iscomposing(uc))
cellcount += utf_char2cells(uc);
}

if (last_glyph_rbearing != NULL
&& cellcount > 0 && next == glyphs->num_glyphs)
{
PangoRectangle ink_rect;





pango_font_get_glyph_extents(item->analysis.font,
glyphs->glyphs[i].glyph,
&ink_rect, NULL);

if (PANGO_RBEARING(ink_rect) > 0)
*last_glyph_rbearing = PANGO_RBEARING(ink_rect);
}

if (cellcount > 0)
*cluster_width = width;

return cellcount;
}
















static void
setup_zero_width_cluster(PangoItem *item, PangoGlyphInfo *glyph,
int last_cellcount, int last_cluster_width,
int last_glyph_rbearing)
{
PangoRectangle ink_rect;
PangoRectangle logical_rect;
int width;

width = last_cellcount * gui.char_width * PANGO_SCALE;
glyph->geometry.x_offset = -width + MAX(0, width - last_cluster_width) / 2;
glyph->geometry.width = 0;

pango_font_get_glyph_extents(item->analysis.font,
glyph->glyph,
&ink_rect, &logical_rect);
if (ink_rect.x < 0)
{
glyph->geometry.x_offset += last_glyph_rbearing;
glyph->geometry.y_offset = logical_rect.height
- (gui.char_height - p_linespace) * PANGO_SCALE;
}
else


glyph->geometry.x_offset = -width + MAX(0, width - ink_rect.width) / 2;
}

#if GTK_CHECK_VERSION(3,0,0)
static void
draw_glyph_string(int row, int col, int num_cells, int flags,
PangoFont *font, PangoGlyphString *glyphs,
cairo_t *cr)
#else
static void
draw_glyph_string(int row, int col, int num_cells, int flags,
PangoFont *font, PangoGlyphString *glyphs)
#endif
{
if (!(flags & DRAW_TRANSP))
{
#if GTK_CHECK_VERSION(3,0,0)
cairo_set_source_rgba(cr,
gui.bgcolor->red, gui.bgcolor->green, gui.bgcolor->blue,
gui.bgcolor->alpha);
cairo_rectangle(cr,
FILL_X(col), FILL_Y(row),
num_cells * gui.char_width, gui.char_height);
cairo_fill(cr);
#else
gdk_gc_set_foreground(gui.text_gc, gui.bgcolor);

gdk_draw_rectangle(gui.drawarea->window,
gui.text_gc,
TRUE,
FILL_X(col),
FILL_Y(row),
num_cells * gui.char_width,
gui.char_height);
#endif
}

#if GTK_CHECK_VERSION(3,0,0)
cairo_set_source_rgba(cr,
gui.fgcolor->red, gui.fgcolor->green, gui.fgcolor->blue,
gui.fgcolor->alpha);
cairo_move_to(cr, TEXT_X(col), TEXT_Y(row));
pango_cairo_show_glyph_string(cr, font, glyphs);
#else
gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);

gdk_draw_glyphs(gui.drawarea->window,
gui.text_gc,
font,
TEXT_X(col),
TEXT_Y(row),
glyphs);
#endif


if ((flags & DRAW_BOLD) && !gui.font_can_bold)
#if GTK_CHECK_VERSION(3,0,0)
{
cairo_set_source_rgba(cr,
gui.fgcolor->red, gui.fgcolor->green, gui.fgcolor->blue,
gui.fgcolor->alpha);
cairo_move_to(cr, TEXT_X(col) + 1, TEXT_Y(row));
pango_cairo_show_glyph_string(cr, font, glyphs);
}
#else
gdk_draw_glyphs(gui.drawarea->window,
gui.text_gc,
font,
TEXT_X(col) + 1,
TEXT_Y(row),
glyphs);
#endif
}




#if GTK_CHECK_VERSION(3,0,0)
static void
draw_under(int flags, int row, int col, int cells, cairo_t *cr)
#else
static void
draw_under(int flags, int row, int col, int cells)
#endif
{
int i;
int offset;
static const int val[8] = {1, 0, 0, 0, 1, 2, 2, 2 };
int y = FILL_Y(row + 1) - 1;


if (flags & DRAW_UNDERC)
{
#if GTK_CHECK_VERSION(3,0,0)
cairo_set_line_width(cr, 1.0);
cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
cairo_set_source_rgba(cr,
gui.spcolor->red, gui.spcolor->green, gui.spcolor->blue,
gui.spcolor->alpha);
for (i = FILL_X(col); i < FILL_X(col + cells); ++i)
{
offset = val[i % 8];
cairo_line_to(cr, i, y - offset + 0.5);
}
cairo_stroke(cr);
#else
gdk_gc_set_foreground(gui.text_gc, gui.spcolor);
for (i = FILL_X(col); i < FILL_X(col + cells); ++i)
{
offset = val[i % 8];
gdk_draw_point(gui.drawarea->window, gui.text_gc, i, y - offset);
}
gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);
#endif
}


if (flags & DRAW_STRIKE)
{
#if GTK_CHECK_VERSION(3,0,0)
cairo_set_line_width(cr, 1.0);
cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
cairo_set_source_rgba(cr,
gui.spcolor->red, gui.spcolor->green, gui.spcolor->blue,
gui.spcolor->alpha);
cairo_move_to(cr, FILL_X(col), y + 1 - gui.char_height/2 + 0.5);
cairo_line_to(cr, FILL_X(col + cells), y + 1 - gui.char_height/2 + 0.5);
cairo_stroke(cr);
#else
gdk_gc_set_foreground(gui.text_gc, gui.spcolor);
gdk_draw_line(gui.drawarea->window, gui.text_gc,
FILL_X(col), y + 1 - gui.char_height/2,
FILL_X(col + cells), y + 1 - gui.char_height/2);
gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);
#endif
}


if (flags & DRAW_UNDERL)
{


if (p_linespace > 1)
y -= p_linespace - 1;
#if GTK_CHECK_VERSION(3,0,0)
cairo_set_line_width(cr, 1.0);
cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
cairo_set_source_rgba(cr,
gui.fgcolor->red, gui.fgcolor->green, gui.fgcolor->blue,
gui.fgcolor->alpha);
cairo_move_to(cr, FILL_X(col), y + 0.5);
cairo_line_to(cr, FILL_X(col + cells), y + 0.5);
cairo_stroke(cr);
#else
gdk_draw_line(gui.drawarea->window, gui.text_gc,
FILL_X(col), y,
FILL_X(col + cells) - 1, y);
#endif
}
}

int
gui_gtk2_draw_string(int row, int col, char_u *s, int len, int flags)
{
GdkRectangle area; 
PangoGlyphString *glyphs; 
int column_offset = 0; 
int i;
char_u *conv_buf = NULL; 
char_u *new_conv_buf;
int convlen;
char_u *sp, *bp;
int plen;
#if GTK_CHECK_VERSION(3,0,0)
cairo_t *cr;
#endif

if (gui.text_context == NULL || gtk_widget_get_window(gui.drawarea) == NULL)
return len;

if (output_conv.vc_type != CONV_NONE)
{






convlen = len;
conv_buf = string_convert(&output_conv, s, &convlen);
g_return_val_if_fail(conv_buf != NULL, len);




for (sp = s, bp = conv_buf; sp < s + len && bp < conv_buf + convlen; )
{
plen = utf_ptr2len(bp);
if ((*mb_ptr2cells)(sp) == 2 && utf_ptr2cells(bp) == 1)
{
new_conv_buf = alloc(convlen + 2);
if (new_conv_buf == NULL)
return len;
plen += bp - conv_buf;
mch_memmove(new_conv_buf, conv_buf, plen);
new_conv_buf[plen] = ' ';
mch_memmove(new_conv_buf + plen + 1, conv_buf + plen,
convlen - plen + 1);
vim_free(conv_buf);
conv_buf = new_conv_buf;
++convlen;
bp = conv_buf + plen;
plen = 1;
}
sp += (*mb_ptr2len)(sp);
bp += plen;
}
s = conv_buf;
len = convlen;
}





area.x = gui.border_offset;
area.y = FILL_Y(row);
area.width = gui.num_cols * gui.char_width;
area.height = gui.char_height;

#if GTK_CHECK_VERSION(3,0,0)
cr = cairo_create(gui.surface);
cairo_rectangle(cr, area.x, area.y, area.width, area.height);
cairo_clip(cr);
#else
gdk_gc_set_clip_origin(gui.text_gc, 0, 0);
gdk_gc_set_clip_rectangle(gui.text_gc, &area);
#endif

glyphs = pango_glyph_string_new();






if (!(flags & DRAW_ITALIC)
&& !((flags & DRAW_BOLD) && gui.font_can_bold)
&& gui.ascii_glyphs != NULL)
{
char_u *p;

for (p = s; p < s + len; ++p)
if (*p & 0x80)
goto not_ascii;

pango_glyph_string_set_size(glyphs, len);

for (i = 0; i < len; ++i)
{
glyphs->glyphs[i] = gui.ascii_glyphs->glyphs[2 * s[i]];
glyphs->log_clusters[i] = i;
}

#if GTK_CHECK_VERSION(3,0,0)
draw_glyph_string(row, col, len, flags, gui.ascii_font, glyphs, cr);
#else
draw_glyph_string(row, col, len, flags, gui.ascii_font, glyphs);
#endif

column_offset = len;
}
else
not_ascii:
{
PangoAttrList *attr_list;
GList *item_list;
int cluster_width;
int last_glyph_rbearing;
int cells = 0; 



if (!utf_valid_string(s, s + len))
{
column_offset = len;
goto skipitall;
}


cluster_width = PANGO_SCALE * gui.char_width;


last_glyph_rbearing = PANGO_SCALE * gui.char_width;

attr_list = pango_attr_list_new();



if (gui.wide_font != NULL)
apply_wide_font_attr(s, len, attr_list);

if ((flags & DRAW_BOLD) && gui.font_can_bold)
INSERT_PANGO_ATTR(pango_attr_weight_new(PANGO_WEIGHT_BOLD),
attr_list, 0, len);
if (flags & DRAW_ITALIC)
INSERT_PANGO_ATTR(pango_attr_style_new(PANGO_STYLE_ITALIC),
attr_list, 0, len);






item_list = pango_itemize(gui.text_context,
(const char *)s, 0, len, attr_list, NULL);

while (item_list != NULL)
{
PangoItem *item;
int item_cells = 0; 

item = (PangoItem *)item_list->data;
item_list = g_list_delete_link(item_list, item_list);












item->analysis.level = (item->analysis.level + 1) & (~1U);



item->analysis.shape_engine = default_shape_engine;

#if defined(HAVE_PANGO_SHAPE_FULL)
pango_shape_full((const char *)s + item->offset, item->length,
(const char *)s, len, &item->analysis, glyphs);
#else
pango_shape((const char *)s + item->offset, item->length,
&item->analysis, glyphs);
#endif









for (i = 0; i < glyphs->num_glyphs; ++i)
{
PangoGlyphInfo *glyph;

glyph = &glyphs->glyphs[i];

if (glyph->attr.is_cluster_start)
{
int cellcount;

cellcount = count_cluster_cells(
s, item, glyphs, i, &cluster_width,
(item_list != NULL) ? &last_glyph_rbearing : NULL);

if (cellcount > 0)
{
int width;

width = cellcount * gui.char_width * PANGO_SCALE;
glyph->geometry.x_offset +=
MAX(0, width - cluster_width) / 2;
glyph->geometry.width = width;
}
else
{




setup_zero_width_cluster(item, glyph, cells,
cluster_width,
last_glyph_rbearing);
}

item_cells += cellcount;
cells = cellcount;
}
else if (i > 0)
{
int width;











if (glyph->geometry.x_offset >= 0)
{
glyphs->glyphs[i].geometry.width =
glyphs->glyphs[i - 1].geometry.width;
glyphs->glyphs[i - 1].geometry.width = 0;
}
width = cells * gui.char_width * PANGO_SCALE;
glyph->geometry.x_offset +=
MAX(0, width - cluster_width) / 2;
}
else 
{
glyph->geometry.width = 0;
}
}


#if GTK_CHECK_VERSION(3,0,0)
draw_glyph_string(row, col + column_offset, item_cells,
flags, item->analysis.font, glyphs,
cr);
#else
draw_glyph_string(row, col + column_offset, item_cells,
flags, item->analysis.font, glyphs);
#endif

pango_item_free(item);

column_offset += item_cells;
}

pango_attr_list_unref(attr_list);
}

skipitall:

#if GTK_CHECK_VERSION(3,0,0)
draw_under(flags, row, col, column_offset, cr);
#else
draw_under(flags, row, col, column_offset);
#endif

pango_glyph_string_free(glyphs);
vim_free(conv_buf);

#if GTK_CHECK_VERSION(3,0,0)
cairo_destroy(cr);
if (!gui.by_signal)
gdk_window_invalidate_rect(gtk_widget_get_window(gui.drawarea),
&area, FALSE);
#else
gdk_gc_set_clip_rectangle(gui.text_gc, NULL);
#endif

return column_offset;
}




int
gui_mch_haskey(char_u *name)
{
int i;

for (i = 0; special_keys[i].key_sym != 0; i++)
if (name[0] == special_keys[i].code0
&& name[1] == special_keys[i].code1)
return OK;
return FAIL;
}

#if defined(FEAT_TITLE) || defined(FEAT_EVAL) || defined(PROTO)



int
gui_get_x11_windis(Window *win, Display **dis)
{
if (gui.mainwin != NULL && gtk_widget_get_window(gui.mainwin) != NULL)
{
*dis = GDK_WINDOW_XDISPLAY(gtk_widget_get_window(gui.mainwin));
*win = GDK_WINDOW_XID(gtk_widget_get_window(gui.mainwin));
return OK;
}

*dis = NULL;
*win = 0;
return FAIL;
}
#endif

#if defined(FEAT_CLIENTSERVER) || (defined(FEAT_X11) && defined(FEAT_CLIPBOARD)) || defined(PROTO)


Display *
gui_mch_get_display(void)
{
if (gui.mainwin != NULL && gtk_widget_get_window(gui.mainwin) != NULL)
return GDK_WINDOW_XDISPLAY(gtk_widget_get_window(gui.mainwin));
else
return NULL;
}
#endif

void
gui_mch_beep(void)
{
GdkDisplay *display;

if (gui.mainwin != NULL && gtk_widget_get_realized(gui.mainwin))
display = gtk_widget_get_display(gui.mainwin);
else
display = gdk_display_get_default();

if (display != NULL)
gdk_display_beep(display);
}

void
gui_mch_flash(int msec)
{
#if GTK_CHECK_VERSION(3,0,0)

(void)msec;
#else
GdkGCValues values;
GdkGC *invert_gc;

if (gui.drawarea->window == NULL)
return;

values.foreground.pixel = gui.norm_pixel ^ gui.back_pixel;
values.background.pixel = gui.norm_pixel ^ gui.back_pixel;
values.function = GDK_XOR;
invert_gc = gdk_gc_new_with_values(gui.drawarea->window,
&values,
GDK_GC_FOREGROUND |
GDK_GC_BACKGROUND |
GDK_GC_FUNCTION);
gdk_gc_set_exposures(invert_gc,
gui.visibility != GDK_VISIBILITY_UNOBSCURED);







gdk_draw_rectangle(gui.drawarea->window, invert_gc,
TRUE,
0, 0,
FILL_X((int)Columns) + gui.border_offset,
FILL_Y((int)Rows) + gui.border_offset);

gui_mch_flush();
ui_delay((long)msec, TRUE); 

gdk_draw_rectangle(gui.drawarea->window, invert_gc,
TRUE,
0, 0,
FILL_X((int)Columns) + gui.border_offset,
FILL_Y((int)Rows) + gui.border_offset);

gdk_gc_destroy(invert_gc);
#endif
}




void
gui_mch_invert_rectangle(int r, int c, int nr, int nc)
{
#if GTK_CHECK_VERSION(3,0,0)
const GdkRectangle rect = {
FILL_X(c), FILL_Y(r), nc * gui.char_width, nr * gui.char_height
};
cairo_t * const cr = cairo_create(gui.surface);

set_cairo_source_rgba_from_color(cr, gui.norm_pixel ^ gui.back_pixel);
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1,9,2)
cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
#else

#endif
gdk_cairo_rectangle(cr, &rect);
cairo_fill(cr);

cairo_destroy(cr);

if (!gui.by_signal)
gtk_widget_queue_draw_area(gui.drawarea, rect.x, rect.y,
rect.width, rect.height);
#else
GdkGCValues values;
GdkGC *invert_gc;

if (gui.drawarea->window == NULL)
return;

values.foreground.pixel = gui.norm_pixel ^ gui.back_pixel;
values.background.pixel = gui.norm_pixel ^ gui.back_pixel;
values.function = GDK_XOR;
invert_gc = gdk_gc_new_with_values(gui.drawarea->window,
&values,
GDK_GC_FOREGROUND |
GDK_GC_BACKGROUND |
GDK_GC_FUNCTION);
gdk_gc_set_exposures(invert_gc, gui.visibility !=
GDK_VISIBILITY_UNOBSCURED);
gdk_draw_rectangle(gui.drawarea->window, invert_gc,
TRUE,
FILL_X(c), FILL_Y(r),
(nc) * gui.char_width, (nr) * gui.char_height);
gdk_gc_destroy(invert_gc);
#endif
}




void
gui_mch_iconify(void)
{
gtk_window_iconify(GTK_WINDOW(gui.mainwin));
}

#if defined(FEAT_EVAL) || defined(PROTO)



void
gui_mch_set_foreground(void)
{
gtk_window_present(GTK_WINDOW(gui.mainwin));
}
#endif




void
gui_mch_draw_hollow_cursor(guicolor_T color)
{
int i = 1;
#if GTK_CHECK_VERSION(3,0,0)
cairo_t *cr;
#endif

if (gtk_widget_get_window(gui.drawarea) == NULL)
return;

#if GTK_CHECK_VERSION(3,0,0)
cr = cairo_create(gui.surface);
#endif

gui_mch_set_fg_color(color);

#if GTK_CHECK_VERSION(3,0,0)
cairo_set_source_rgba(cr,
gui.fgcolor->red, gui.fgcolor->green, gui.fgcolor->blue,
gui.fgcolor->alpha);
#else
gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);
#endif
if (mb_lefthalve(gui.row, gui.col))
i = 2;
#if GTK_CHECK_VERSION(3,0,0)
cairo_set_line_width(cr, 1.0);
cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
cairo_rectangle(cr,
FILL_X(gui.col) + 0.5, FILL_Y(gui.row) + 0.5,
i * gui.char_width - 1, gui.char_height - 1);
cairo_stroke(cr);
cairo_destroy(cr);
#else
gdk_draw_rectangle(gui.drawarea->window, gui.text_gc,
FALSE,
FILL_X(gui.col), FILL_Y(gui.row),
i * gui.char_width - 1, gui.char_height - 1);
#endif
}





void
gui_mch_draw_part_cursor(int w, int h, guicolor_T color)
{
if (gtk_widget_get_window(gui.drawarea) == NULL)
return;

gui_mch_set_fg_color(color);

#if GTK_CHECK_VERSION(3,0,0)
{
cairo_t *cr;

cr = cairo_create(gui.surface);
cairo_set_source_rgba(cr,
gui.fgcolor->red, gui.fgcolor->green, gui.fgcolor->blue,
gui.fgcolor->alpha);
cairo_rectangle(cr,
#if defined(FEAT_RIGHTLEFT)

CURSOR_BAR_RIGHT ? FILL_X(gui.col + 1) - w :
#endif
FILL_X(gui.col), FILL_Y(gui.row) + gui.char_height - h,
w, h);
cairo_fill(cr);
cairo_destroy(cr);
}
#else 
gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);
gdk_draw_rectangle(gui.drawarea->window, gui.text_gc,
TRUE,
#if defined(FEAT_RIGHTLEFT)

CURSOR_BAR_RIGHT ? FILL_X(gui.col + 1) - w :
#endif
FILL_X(gui.col),
FILL_Y(gui.row) + gui.char_height - h,
w, h);
#endif 
}








void
gui_mch_update(void)
{
while (g_main_context_pending(NULL) && !vim_is_input_buf_full())
g_main_context_iteration(NULL, TRUE);
}

static timeout_cb_type
input_timer_cb(gpointer data)
{
int *timed_out = (int *) data;


*timed_out = TRUE;

return FALSE; 
}

#if defined(FEAT_JOB_CHANNEL)
static timeout_cb_type
channel_poll_cb(gpointer data UNUSED)
{


channel_handle_events(TRUE);
parse_queued_messages();

return TRUE; 
}
#endif










int
gui_mch_wait_for_chars(long wtime)
{
int focus;
guint timer;
static int timed_out;
int retval = FAIL;
#if defined(FEAT_JOB_CHANNEL)
guint channel_timer = 0;
#endif

timed_out = FALSE;



if (wtime >= 0)
timer = timeout_add(wtime == 0 ? 1L : wtime,
input_timer_cb, &timed_out);
else
timer = 0;

#if defined(FEAT_JOB_CHANNEL)


if (channel_any_keep_open())
channel_timer = timeout_add(20, channel_poll_cb, NULL);
#endif

focus = gui.in_focus;

do
{

if (gui.in_focus != focus)
{
if (gui.in_focus)
gui_mch_start_blink();
else
gui_mch_stop_blink(TRUE);
focus = gui.in_focus;
}

#if defined(MESSAGE_QUEUE)
#if defined(FEAT_TIMERS)
did_add_timer = FALSE;
#endif
parse_queued_messages();
#if defined(FEAT_TIMERS)
if (did_add_timer)

goto theend;
#endif
#endif






if (!input_available())
g_main_context_iteration(NULL, TRUE);


if (input_available())
{
retval = OK;
goto theend;
}
} while (wtime < 0 || !timed_out);




gui_mch_update();

theend:
if (timer != 0 && !timed_out)
timeout_remove(timer);
#if defined(FEAT_JOB_CHANNEL)
if (channel_timer != 0)
timeout_remove(channel_timer);
#endif

return retval;
}








void
gui_mch_flush(void)
{
if (gui.mainwin != NULL && gtk_widget_get_realized(gui.mainwin))
#if GTK_CHECK_VERSION(2,4,0)
gdk_display_flush(gtk_widget_get_display(gui.mainwin));
#else
gdk_display_sync(gtk_widget_get_display(gui.mainwin));
#endif
}





void
gui_mch_clear_block(int row1arg, int col1arg, int row2arg, int col2arg)
{

int col1 = check_col(col1arg);
int col2 = check_col(col2arg);
int row1 = check_row(row1arg);
int row2 = check_row(row2arg);

#if GTK_CHECK_VERSION(3,0,0)
if (gtk_widget_get_window(gui.drawarea) == NULL)
return;
#else
GdkColor color;

if (gui.drawarea->window == NULL)
return;

color.pixel = gui.back_pixel;
#endif

#if GTK_CHECK_VERSION(3,0,0)
{


const GdkRectangle rect = {
FILL_X(col1), FILL_Y(row1),
(col2 - col1 + 1) * gui.char_width + (col2 == Columns - 1),
(row2 - row1 + 1) * gui.char_height
};
GdkWindow * const win = gtk_widget_get_window(gui.drawarea);
cairo_t * const cr = cairo_create(gui.surface);
#if GTK_CHECK_VERSION(3,22,2)
set_cairo_source_rgba_from_color(cr, gui.back_pixel);
#else
cairo_pattern_t * const pat = gdk_window_get_background_pattern(win);
if (pat != NULL)
cairo_set_source(cr, pat);
else
set_cairo_source_rgba_from_color(cr, gui.back_pixel);
#endif
gdk_cairo_rectangle(cr, &rect);
cairo_fill(cr);
cairo_destroy(cr);

if (!gui.by_signal)
gdk_window_invalidate_rect(win, &rect, FALSE);
}
#else 
gdk_gc_set_foreground(gui.text_gc, &color);



gdk_draw_rectangle(gui.drawarea->window, gui.text_gc, TRUE,
FILL_X(col1), FILL_Y(row1),
(col2 - col1 + 1) * gui.char_width
+ (col2 == Columns - 1),
(row2 - row1 + 1) * gui.char_height);
#endif 
}

#if GTK_CHECK_VERSION(3,0,0)
static void
gui_gtk_window_clear(GdkWindow *win)
{
const GdkRectangle rect = {
0, 0, gdk_window_get_width(win), gdk_window_get_height(win)
};
cairo_t * const cr = cairo_create(gui.surface);
#if GTK_CHECK_VERSION(3,22,2)
set_cairo_source_rgba_from_color(cr, gui.back_pixel);
#else
cairo_pattern_t * const pat = gdk_window_get_background_pattern(win);
if (pat != NULL)
cairo_set_source(cr, pat);
else
set_cairo_source_rgba_from_color(cr, gui.back_pixel);
#endif
gdk_cairo_rectangle(cr, &rect);
cairo_fill(cr);
cairo_destroy(cr);

if (!gui.by_signal)
gdk_window_invalidate_rect(win, &rect, FALSE);
}
#else
#define gui_gtk_window_clear(win) gdk_window_clear(win)
#endif

void
gui_mch_clear_all(void)
{
if (gtk_widget_get_window(gui.drawarea) != NULL)
gui_gtk_window_clear(gtk_widget_get_window(gui.drawarea));
}

#if !GTK_CHECK_VERSION(3,0,0)



static void
check_copy_area(void)
{
GdkEvent *event;
int expose_count;

if (gui.visibility != GDK_VISIBILITY_PARTIAL)
return;





gui_dont_update_cursor(TRUE);

do
{

event = gdk_event_get_graphics_expose(gui.drawarea->window);

if (event == NULL)
break; 

gui_redraw(event->expose.area.x, event->expose.area.y,
event->expose.area.width, event->expose.area.height);

expose_count = event->expose.count;
gdk_event_free(event);
}
while (expose_count > 0); 

gui_can_update_cursor();
}
#endif 

#if GTK_CHECK_VERSION(3,0,0)
static void
gui_gtk_surface_copy_rect(int dest_x, int dest_y,
int src_x, int src_y,
int width, int height)
{
cairo_t * const cr = cairo_create(gui.surface);

cairo_rectangle(cr, dest_x, dest_y, width, height);
cairo_clip(cr);
cairo_push_group(cr);
cairo_set_source_surface(cr, gui.surface, dest_x - src_x, dest_y - src_y);
cairo_paint(cr);
cairo_pop_group_to_source(cr);
cairo_paint(cr);

cairo_destroy(cr);
}
#endif





void
gui_mch_delete_lines(int row, int num_lines)
{
#if GTK_CHECK_VERSION(3,0,0)
const int ncols = gui.scroll_region_right - gui.scroll_region_left + 1;
const int nrows = gui.scroll_region_bot - row + 1;
const int src_nrows = nrows - num_lines;

gui_gtk_surface_copy_rect(
FILL_X(gui.scroll_region_left), FILL_Y(row),
FILL_X(gui.scroll_region_left), FILL_Y(row + num_lines),
gui.char_width * ncols + 1, gui.char_height * src_nrows);
gui_clear_block(
gui.scroll_region_bot - num_lines + 1, gui.scroll_region_left,
gui.scroll_region_bot, gui.scroll_region_right);
gui_gtk3_redraw(
FILL_X(gui.scroll_region_left), FILL_Y(row),
gui.char_width * ncols + 1, gui.char_height * nrows);
if (!gui.by_signal)
gtk_widget_queue_draw_area(gui.drawarea,
FILL_X(gui.scroll_region_left), FILL_Y(row),
gui.char_width * ncols + 1, gui.char_height * nrows);
#else
if (gui.visibility == GDK_VISIBILITY_FULLY_OBSCURED)
return; 

gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);
gdk_gc_set_background(gui.text_gc, gui.bgcolor);


gdk_window_copy_area(gui.drawarea->window, gui.text_gc,
FILL_X(gui.scroll_region_left), FILL_Y(row),
gui.drawarea->window,
FILL_X(gui.scroll_region_left),
FILL_Y(row + num_lines),
gui.char_width * (gui.scroll_region_right
- gui.scroll_region_left + 1) + 1,
gui.char_height * (gui.scroll_region_bot - row - num_lines + 1));

gui_clear_block(gui.scroll_region_bot - num_lines + 1,
gui.scroll_region_left,
gui.scroll_region_bot, gui.scroll_region_right);
check_copy_area();
#endif 
}





void
gui_mch_insert_lines(int row, int num_lines)
{
#if GTK_CHECK_VERSION(3,0,0)
const int ncols = gui.scroll_region_right - gui.scroll_region_left + 1;
const int nrows = gui.scroll_region_bot - row + 1;
const int src_nrows = nrows - num_lines;

gui_gtk_surface_copy_rect(
FILL_X(gui.scroll_region_left), FILL_Y(row + num_lines),
FILL_X(gui.scroll_region_left), FILL_Y(row),
gui.char_width * ncols + 1, gui.char_height * src_nrows);
gui_mch_clear_block(
row, gui.scroll_region_left,
row + num_lines - 1, gui.scroll_region_right);
gui_gtk3_redraw(
FILL_X(gui.scroll_region_left), FILL_Y(row),
gui.char_width * ncols + 1, gui.char_height * nrows);
if (!gui.by_signal)
gtk_widget_queue_draw_area(gui.drawarea,
FILL_X(gui.scroll_region_left), FILL_Y(row),
gui.char_width * ncols + 1, gui.char_height * nrows);
#else
if (gui.visibility == GDK_VISIBILITY_FULLY_OBSCURED)
return; 

gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);
gdk_gc_set_background(gui.text_gc, gui.bgcolor);


gdk_window_copy_area(gui.drawarea->window, gui.text_gc,
FILL_X(gui.scroll_region_left), FILL_Y(row + num_lines),
gui.drawarea->window,
FILL_X(gui.scroll_region_left), FILL_Y(row),
gui.char_width * (gui.scroll_region_right
- gui.scroll_region_left + 1) + 1,
gui.char_height * (gui.scroll_region_bot - row - num_lines + 1));

gui_clear_block(row, gui.scroll_region_left,
row + num_lines - 1, gui.scroll_region_right);
check_copy_area();
#endif 
}




void
clip_mch_request_selection(Clipboard_T *cbd)
{
GdkAtom target;
unsigned i;
time_t start;

for (i = 0; i < N_SELECTION_TARGETS; ++i)
{
if (!clip_html && selection_targets[i].info == TARGET_HTML)
continue;
received_selection = RS_NONE;
target = gdk_atom_intern(selection_targets[i].target, FALSE);

gtk_selection_convert(gui.drawarea,
cbd->gtk_sel_atom, target,
(guint32)GDK_CURRENT_TIME);




start = time(NULL);
while (received_selection == RS_NONE && time(NULL) < start + 3)
g_main_context_iteration(NULL, TRUE); 

if (received_selection != RS_FAIL)
return;
}


yank_cut_buffer0(GDK_WINDOW_XDISPLAY(gtk_widget_get_window(gui.mainwin)),
cbd);
}




void
clip_mch_lose_selection(Clipboard_T *cbd UNUSED)
{
if (!in_selection_clear_event)
{
gtk_selection_owner_set(NULL, cbd->gtk_sel_atom, gui.event_time);
gui_mch_update();
}
}




int
clip_mch_own_selection(Clipboard_T *cbd)
{
int success;

success = gtk_selection_owner_set(gui.drawarea, cbd->gtk_sel_atom,
gui.event_time);
gui_mch_update();
return (success) ? OK : FAIL;
}





void
clip_mch_set_selection(Clipboard_T *cbd UNUSED)
{
}

#if (defined(FEAT_XCLIPBOARD) && defined(USE_SYSTEM)) || defined(PROTO)
int
clip_gtk_owner_exists(Clipboard_T *cbd)
{
return gdk_selection_owner_get(cbd->gtk_sel_atom) != NULL;
}
#endif


#if defined(FEAT_MENU) || defined(PROTO)



void
gui_mch_menu_grey(vimmenu_T *menu, int grey)
{
if (menu->id == NULL)
return;

if (menu_is_separator(menu->name))
grey = TRUE;

gui_mch_menu_hidden(menu, FALSE);

if (!gtk_widget_get_sensitive(menu->id) == !grey)
{
gtk_widget_set_sensitive(menu->id, !grey);
gui_mch_update();
}
}




void
gui_mch_menu_hidden(vimmenu_T *menu, int hidden)
{
if (menu->id == 0)
return;

if (hidden)
{
if (gtk_widget_get_visible(menu->id))
{
gtk_widget_hide(menu->id);
gui_mch_update();
}
}
else
{
if (!gtk_widget_get_visible(menu->id))
{
gtk_widget_show(menu->id);
gui_mch_update();
}
}
}




void
gui_mch_draw_menubar(void)
{

gui_mch_update();
}
#endif 




void
gui_mch_enable_scrollbar(scrollbar_T *sb, int flag)
{
if (sb->id == NULL)
return;

gtk_widget_set_visible(sb->id, flag);
update_window_manager_hints(0, 0);
}





guicolor_T
gui_mch_get_rgb(guicolor_T pixel)
{
#if GTK_CHECK_VERSION(3,0,0)
return (long_u)pixel;
#else
GdkColor color;

gdk_colormap_query_color(gtk_widget_get_colormap(gui.drawarea),
(unsigned long)pixel, &color);

return (guicolor_T)(
(((unsigned)color.red & 0xff00) << 8)
| ((unsigned)color.green & 0xff00)
| (((unsigned)color.blue & 0xff00) >> 8));
#endif
}




void
gui_mch_getmouse(int *x, int *y)
{
gui_gtk_get_pointer(gui.drawarea, x, y, NULL);
}

void
gui_mch_setmouse(int x, int y)
{



XWarpPointer(GDK_WINDOW_XDISPLAY(gtk_widget_get_window(gui.drawarea)),
(Window)0, GDK_WINDOW_XID(gtk_widget_get_window(gui.drawarea)),
0, 0, 0U, 0U, x, y);
}


#if defined(FEAT_MOUSESHAPE)


static int last_shape = 0;
#endif






void
gui_mch_mousehide(int hide)
{
if (gui.pointer_hidden != hide)
{
gui.pointer_hidden = hide;
if (gtk_widget_get_window(gui.drawarea) && gui.blank_pointer != NULL)
{
if (hide)
gdk_window_set_cursor(gtk_widget_get_window(gui.drawarea),
gui.blank_pointer);
else
#if defined(FEAT_MOUSESHAPE)
mch_set_mouse_shape(last_shape);
#else
gdk_window_set_cursor(gtk_widget_get_window(gui.drawarea), NULL);
#endif
}
}
}

#if defined(FEAT_MOUSESHAPE) || defined(PROTO)



static const int mshape_ids[] =
{
GDK_LEFT_PTR, 
GDK_CURSOR_IS_PIXMAP, 
GDK_XTERM, 
GDK_SB_V_DOUBLE_ARROW, 
GDK_SIZING, 
GDK_SB_H_DOUBLE_ARROW, 
GDK_SIZING, 
GDK_WATCH, 
GDK_X_CURSOR, 
GDK_CROSSHAIR, 
GDK_HAND1, 
GDK_HAND2, 
GDK_PENCIL, 
GDK_QUESTION_ARROW, 
GDK_RIGHT_PTR, 
GDK_CENTER_PTR, 
GDK_LEFT_PTR 
};

void
mch_set_mouse_shape(int shape)
{
int id;
GdkCursor *c;

if (gtk_widget_get_window(gui.drawarea) == NULL)
return;

if (shape == MSHAPE_HIDE || gui.pointer_hidden)
gdk_window_set_cursor(gtk_widget_get_window(gui.drawarea),
gui.blank_pointer);
else
{
if (shape >= MSHAPE_NUMBERED)
{
id = shape - MSHAPE_NUMBERED;
if (id >= GDK_LAST_CURSOR)
id = GDK_LEFT_PTR;
else
id &= ~1; 
}
else if (shape < (int)(sizeof(mshape_ids) / sizeof(int)))
id = mshape_ids[shape];
else
return;
c = gdk_cursor_new_for_display(
gtk_widget_get_display(gui.drawarea), (GdkCursorType)id);
gdk_window_set_cursor(gtk_widget_get_window(gui.drawarea), c);
#if GTK_CHECK_VERSION(3,0,0)
g_object_unref(G_OBJECT(c));
#else
gdk_cursor_destroy(c); 
#endif
}
if (shape != MSHAPE_HIDE)
last_shape = shape;
}
#endif 


#if defined(FEAT_SIGN_ICONS) || defined(PROTO)







#define SIGN_WIDTH (2 * gui.char_width)
#define SIGN_HEIGHT (gui.char_height)
#define SIGN_ASPECT ((double)SIGN_HEIGHT / (double)SIGN_WIDTH)

void
gui_mch_drawsign(int row, int col, int typenr)
{
GdkPixbuf *sign;

sign = (GdkPixbuf *)sign_get_image(typenr);

if (sign != NULL && gui.drawarea != NULL
&& gtk_widget_get_window(gui.drawarea) != NULL)
{
int width;
int height;
int xoffset;
int yoffset;
int need_scale;

width = gdk_pixbuf_get_width(sign);
height = gdk_pixbuf_get_height(sign);






need_scale = (width > SIGN_WIDTH + 2
|| height != SIGN_HEIGHT
|| (width < 3 * SIGN_WIDTH / 4
&& height < 3 * SIGN_HEIGHT / 4));
if (need_scale)
{
double aspect;
int w = width;
int h = height;


aspect = (double)height / (double)width;
width = (double)SIGN_WIDTH * SIGN_ASPECT / aspect;
width = MIN(width, SIGN_WIDTH);
if (((double)(MAX(height, SIGN_HEIGHT)) /
(double)(MIN(height, SIGN_HEIGHT))) < 1.15)
{


height = (double)SIGN_HEIGHT * SIGN_ASPECT / aspect;
height = MIN(height, SIGN_HEIGHT);
}
else
height = (double)width * aspect;

if (w == width && h == height)
{


need_scale = FALSE;
}
else
{


sign = gdk_pixbuf_scale_simple(sign, width, height,
GDK_INTERP_BILINEAR);
if (sign == NULL)
return; 
}
}




xoffset = (width - SIGN_WIDTH) / 2;
yoffset = (height - SIGN_HEIGHT) / 2;

#if GTK_CHECK_VERSION(3,0,0)
{
cairo_t *cr;
cairo_surface_t *bg_surf;
cairo_t *bg_cr;
cairo_surface_t *sign_surf;
cairo_t *sign_cr;

cr = cairo_create(gui.surface);

bg_surf = cairo_surface_create_similar(gui.surface,
cairo_surface_get_content(gui.surface),
SIGN_WIDTH, SIGN_HEIGHT);
bg_cr = cairo_create(bg_surf);
cairo_set_source_rgba(bg_cr,
gui.bgcolor->red, gui.bgcolor->green, gui.bgcolor->blue,
gui.bgcolor->alpha);
cairo_paint(bg_cr);

sign_surf = cairo_surface_create_similar(gui.surface,
cairo_surface_get_content(gui.surface),
SIGN_WIDTH, SIGN_HEIGHT);
sign_cr = cairo_create(sign_surf);
gdk_cairo_set_source_pixbuf(sign_cr, sign, -xoffset, -yoffset);
cairo_paint(sign_cr);

cairo_set_operator(sign_cr, CAIRO_OPERATOR_DEST_OVER);
cairo_set_source_surface(sign_cr, bg_surf, 0, 0);
cairo_paint(sign_cr);

cairo_set_source_surface(cr, sign_surf, FILL_X(col), FILL_Y(row));
cairo_paint(cr);

cairo_destroy(sign_cr);
cairo_surface_destroy(sign_surf);
cairo_destroy(bg_cr);
cairo_surface_destroy(bg_surf);
cairo_destroy(cr);

if (!gui.by_signal)
gtk_widget_queue_draw_area(gui.drawarea,
FILL_X(col), FILL_Y(col), width, height);

}
#else 
gdk_gc_set_foreground(gui.text_gc, gui.bgcolor);

gdk_draw_rectangle(gui.drawarea->window,
gui.text_gc,
TRUE,
FILL_X(col),
FILL_Y(row),
SIGN_WIDTH,
SIGN_HEIGHT);

gdk_pixbuf_render_to_drawable_alpha(sign,
gui.drawarea->window,
MAX(0, xoffset),
MAX(0, yoffset),
FILL_X(col) - MIN(0, xoffset),
FILL_Y(row) - MIN(0, yoffset),
MIN(width, SIGN_WIDTH),
MIN(height, SIGN_HEIGHT),
GDK_PIXBUF_ALPHA_BILEVEL,
127,
GDK_RGB_DITHER_NORMAL,
0, 0);
#endif 
if (need_scale)
g_object_unref(sign);
}
}

void *
gui_mch_register_sign(char_u *signfile)
{
if (signfile[0] != NUL && signfile[0] != '-' && gui.in_use)
{
GdkPixbuf *sign;
GError *error = NULL;
char_u *message;

sign = gdk_pixbuf_new_from_file((const char *)signfile, &error);

if (error == NULL)
return sign;

message = (char_u *)error->message;

if (message != NULL && input_conv.vc_type != CONV_NONE)
message = string_convert(&input_conv, message, NULL);

if (message != NULL)
{


semsg("E255: %s", message);

if (input_conv.vc_type != CONV_NONE)
vim_free(message);
}
g_error_free(error);
}

return NULL;
}

void
gui_mch_destroy_sign(void *sign)
{
if (sign != NULL)
g_object_unref(sign);
}

#endif 
