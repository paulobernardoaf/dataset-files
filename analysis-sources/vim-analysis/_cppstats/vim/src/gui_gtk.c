#include "vim.h"
#if defined(FEAT_GUI_GTK)
#include "gui_gtk_f.h"
#endif
#if defined(MIN)
#undef MIN
#endif
#if defined(MAX)
#undef MAX
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
#endif
#if defined(FEAT_GUI_GTK)
#if GTK_CHECK_VERSION(3,0,0)
#include <gdk/gdkkeysyms-compat.h>
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
#else
typedef int GtkWidget;
#define gpointer int
#define guint8 int
#define GdkPixmap int
#define GdkBitmap int
#define GtkIconFactory int
#define GtkToolbar int
#define GtkAdjustment int
#define gboolean int
#define GdkEventKey int
#define CancelData int
#endif
static void entry_activate_cb(GtkWidget *widget, gpointer data);
static void entry_changed_cb(GtkWidget *entry, GtkWidget *dialog);
static void find_replace_cb(GtkWidget *widget, gpointer data);
#if defined(FEAT_BROWSE) || defined(PROTO)
static void recent_func_log_func(
const gchar *log_domain,
GLogLevelFlags log_level,
const gchar *message,
gpointer user_data);
#endif
#if defined(FEAT_TOOLBAR)
#if GTK_CHECK_VERSION(3,10,0)
static const char * const menu_themed_names[] =
{
"document-new", 
"document-open", 
"document-save", 
"edit-undo", 
"edit-redo", 
"edit-cut", 
"edit-copy", 
"edit-paste", 
"document-print", 
"help-browser", 
"edit-find", 
#if GTK_CHECK_VERSION(3,14,0)
"stock_vim_save_all",
"stock_vim_session_save",
"stock_vim_session_new",
"stock_vim_session_load",
#else
"vim-save-all",
"vim-session-save",
"vim-session-new",
"vim-session-load",
#endif
"system-run", 
"edit-find-replace", 
"window-close", 
#if GTK_CHECK_VERSION(3,14,0)
"stock_vim_window_maximize",
"stock_vim_window_minimize",
"stock_vim_window_split",
"stock_vim_shell",
#else
"vim-window-maximize",
"vim-window-minimize",
"vim-window-split",
"vim-shell",
#endif
"go-previous", 
"go-next", 
#if GTK_CHECK_VERSION(3,14,0)
"stock_vim_find_help",
#else
"vim-find-help",
#endif
"gtk-convert", 
"go-jump", 
#if GTK_CHECK_VERSION(3,14,0)
"stock_vim_build_tags",
"stock_vim_window_split_vertical",
"stock_vim_window_maximize_width",
"stock_vim_window_minimize_width",
#else
"vim-build-tags",
"vim-window-split-vertical",
"vim-window-maximize-width",
"vim-window-minimize-width",
#endif
"application-exit", 
};
#else 
static const char * const menu_stock_ids[] =
{
GTK_STOCK_NEW,
GTK_STOCK_OPEN,
GTK_STOCK_SAVE,
GTK_STOCK_UNDO,
GTK_STOCK_REDO,
GTK_STOCK_CUT,
GTK_STOCK_COPY,
GTK_STOCK_PASTE,
GTK_STOCK_PRINT,
GTK_STOCK_HELP,
GTK_STOCK_FIND,
"vim-save-all",
"vim-session-save",
"vim-session-new",
"vim-session-load",
GTK_STOCK_EXECUTE,
GTK_STOCK_FIND_AND_REPLACE,
GTK_STOCK_CLOSE, 
"vim-window-maximize",
"vim-window-minimize",
"vim-window-split",
"vim-shell",
GTK_STOCK_GO_BACK,
GTK_STOCK_GO_FORWARD,
"vim-find-help",
GTK_STOCK_CONVERT,
GTK_STOCK_JUMP_TO,
"vim-build-tags",
"vim-window-split-vertical",
"vim-window-maximize-width",
"vim-window-minimize-width",
GTK_STOCK_QUIT
};
#endif 
#if defined(USE_GRESOURCE)
#if !GTK_CHECK_VERSION(3,10,0)
typedef struct IconNames {
const char *icon_name;
const char *file_name;
} IconNames;
static IconNames stock_vim_icons[] = {
{ "vim-build-tags", "stock_vim_build_tags.png" },
{ "vim-find-help", "stock_vim_find_help.png" },
{ "vim-save-all", "stock_vim_save_all.png" },
{ "vim-session-load", "stock_vim_session_load.png" },
{ "vim-session-new", "stock_vim_session_new.png" },
{ "vim-session-save", "stock_vim_session_save.png" },
{ "vim-shell", "stock_vim_shell.png" },
{ "vim-window-maximize", "stock_vim_window_maximize.png" },
{ "vim-window-maximize-width", "stock_vim_window_maximize_width.png" },
{ "vim-window-minimize", "stock_vim_window_minimize.png" },
{ "vim-window-minimize-width", "stock_vim_window_minimize_width.png" },
{ "vim-window-split", "stock_vim_window_split.png" },
{ "vim-window-split-vertical", "stock_vim_window_split_vertical.png" },
{ NULL, NULL }
};
#endif
#endif 
#if !defined(USE_GRESOURCE)
static void
add_stock_icon(GtkIconFactory *factory,
const char *stock_id,
const guint8 *inline_data,
int data_length)
{
GdkPixbuf *pixbuf;
GtkIconSet *icon_set;
pixbuf = gdk_pixbuf_new_from_inline(data_length, inline_data, FALSE, NULL);
icon_set = gtk_icon_set_new_from_pixbuf(pixbuf);
gtk_icon_factory_add(factory, stock_id, icon_set);
gtk_icon_set_unref(icon_set);
g_object_unref(pixbuf);
}
#endif
static int
lookup_menu_iconfile(char_u *iconfile, char_u *dest)
{
expand_env(iconfile, dest, MAXPATHL);
if (mch_isFullName(dest))
{
return vim_fexists(dest);
}
else
{
static const char suffixes[][4] = {"png", "xpm", "bmp"};
char_u buf[MAXPATHL];
unsigned int i;
for (i = 0; i < G_N_ELEMENTS(suffixes); ++i)
if (gui_find_bitmap(dest, buf, (char *)suffixes[i]) == OK)
{
STRCPY(dest, buf);
return TRUE;
}
return FALSE;
}
}
static GtkWidget *
load_menu_iconfile(char_u *name, GtkIconSize icon_size)
{
GtkWidget *image = NULL;
#if GTK_CHECK_VERSION(3,10,0)
int pixel_size = -1;
switch (icon_size)
{
case GTK_ICON_SIZE_MENU:
pixel_size = 16;
break;
case GTK_ICON_SIZE_SMALL_TOOLBAR:
pixel_size = 16;
break;
case GTK_ICON_SIZE_LARGE_TOOLBAR:
pixel_size = 24;
break;
case GTK_ICON_SIZE_BUTTON:
pixel_size = 16;
break;
case GTK_ICON_SIZE_DND:
pixel_size = 32;
break;
case GTK_ICON_SIZE_DIALOG:
pixel_size = 48;
break;
case GTK_ICON_SIZE_INVALID:
default:
pixel_size = 0;
break;
}
if (pixel_size > 0 || pixel_size == -1)
{
GdkPixbuf * const pixbuf
= gdk_pixbuf_new_from_file_at_scale((const char *)name,
pixel_size, pixel_size, TRUE, NULL);
if (pixbuf != NULL)
{
image = gtk_image_new_from_pixbuf(pixbuf);
g_object_unref(pixbuf);
}
}
if (image == NULL)
image = gtk_image_new_from_icon_name("image-missing", icon_size);
return image;
#else 
GtkIconSet *icon_set;
GtkIconSource *icon_source;
icon_set = gtk_icon_set_new();
icon_source = gtk_icon_source_new();
gtk_icon_source_set_filename(icon_source, (const char *)name);
gtk_icon_set_add_source(icon_set, icon_source);
image = gtk_image_new_from_icon_set(icon_set, icon_size);
gtk_icon_source_free(icon_source);
gtk_icon_set_unref(icon_set);
return image;
#endif 
}
static GtkWidget *
create_menu_icon(vimmenu_T *menu, GtkIconSize icon_size)
{
GtkWidget *image = NULL;
char_u buf[MAXPATHL];
if (menu->iconfile != NULL && lookup_menu_iconfile(menu->iconfile, buf))
image = load_menu_iconfile(buf, icon_size);
if (image == NULL && !menu->icon_builtin
&& lookup_menu_iconfile(menu->name, buf))
image = load_menu_iconfile(buf, icon_size);
if (image == NULL)
{
#if GTK_CHECK_VERSION(3,10,0)
const char *icon_name = NULL;
const int n_names = G_N_ELEMENTS(menu_themed_names);
if (menu->iconidx >= 0 && menu->iconidx < n_names)
icon_name = menu_themed_names[menu->iconidx];
if (icon_name == NULL)
icon_name = "image-missing";
image = gtk_image_new_from_icon_name(icon_name, icon_size);
#else
const char *stock_id;
const int n_ids = G_N_ELEMENTS(menu_stock_ids);
if (menu->iconidx >= 0 && menu->iconidx < n_ids)
stock_id = menu_stock_ids[menu->iconidx];
else
stock_id = GTK_STOCK_MISSING_IMAGE;
image = gtk_image_new_from_stock(stock_id, icon_size);
#endif
}
return image;
}
static gint
toolbar_button_focus_in_event(GtkWidget *widget UNUSED,
GdkEventFocus *event UNUSED,
gpointer data UNUSED)
{
if (gtk_socket_id != 0)
gtk_widget_grab_focus(gui.drawarea);
return TRUE;
}
#endif 
#if defined(FEAT_TOOLBAR) || defined(PROTO)
void
gui_gtk_register_stock_icons(void)
{
#if !defined(USE_GRESOURCE)
#include "../pixmaps/stock_icons.h"
GtkIconFactory *factory;
factory = gtk_icon_factory_new();
#define ADD_ICON(Name, Data) add_stock_icon(factory, Name, Data, (int)sizeof(Data))
ADD_ICON("vim-build-tags", stock_vim_build_tags);
ADD_ICON("vim-find-help", stock_vim_find_help);
ADD_ICON("vim-save-all", stock_vim_save_all);
ADD_ICON("vim-session-load", stock_vim_session_load);
ADD_ICON("vim-session-new", stock_vim_session_new);
ADD_ICON("vim-session-save", stock_vim_session_save);
ADD_ICON("vim-shell", stock_vim_shell);
ADD_ICON("vim-window-maximize", stock_vim_window_maximize);
ADD_ICON("vim-window-maximize-width", stock_vim_window_maximize_width);
ADD_ICON("vim-window-minimize", stock_vim_window_minimize);
ADD_ICON("vim-window-minimize-width", stock_vim_window_minimize_width);
ADD_ICON("vim-window-split", stock_vim_window_split);
ADD_ICON("vim-window-split-vertical", stock_vim_window_split_vertical);
#undef ADD_ICON
gtk_icon_factory_add_default(factory);
g_object_unref(factory);
#else 
const char * const path_prefix = "/org/vim/gui/icon";
#if GTK_CHECK_VERSION(3,14,0)
GdkScreen *screen = NULL;
GtkIconTheme *icon_theme = NULL;
if (GTK_IS_WIDGET(gui.mainwin))
screen = gtk_widget_get_screen(gui.mainwin);
else
screen = gdk_screen_get_default();
icon_theme = gtk_icon_theme_get_for_screen(screen);
gtk_icon_theme_add_resource_path(icon_theme, path_prefix);
#elif GTK_CHECK_VERSION(3,0,0)
IconNames *names;
for (names = stock_vim_icons; names->icon_name != NULL; names++)
{
char path[MAXPATHL];
vim_snprintf(path, MAXPATHL, "%s/%s", path_prefix, names->file_name);
GdkPixbuf *pixbuf = NULL;
pixbuf = gdk_pixbuf_new_from_resource(path, NULL);
if (pixbuf != NULL)
{
const gint size = MAX(gdk_pixbuf_get_width(pixbuf),
gdk_pixbuf_get_height(pixbuf));
if (size > 16)
{
GdkPixbuf *src = pixbuf;
pixbuf = gdk_pixbuf_scale_simple(src,
16, 16,
GDK_INTERP_BILINEAR);
if (pixbuf == NULL)
pixbuf = src;
else
g_object_unref(src);
}
gtk_icon_theme_add_builtin_icon(names->icon_name, size, pixbuf);
g_object_unref(pixbuf);
}
}
#else 
GtkIconFactory * const factory = gtk_icon_factory_new();
IconNames *names;
for (names = stock_vim_icons; names->icon_name != NULL; names++)
{
char path[MAXPATHL];
GdkPixbuf *pixbuf;
vim_snprintf(path, MAXPATHL, "%s/%s", path_prefix, names->file_name);
pixbuf = gdk_pixbuf_new_from_resource(path, NULL);
if (pixbuf != NULL)
{
GtkIconSet *icon_set = gtk_icon_set_new_from_pixbuf(pixbuf);
gtk_icon_factory_add(factory, names->icon_name, icon_set);
gtk_icon_set_unref(icon_set);
g_object_unref(pixbuf);
}
}
gtk_icon_factory_add_default(factory);
g_object_unref(factory);
#endif 
#endif 
}
#endif 
#if defined(FEAT_MENU) || defined(PROTO)
static char_u *
translate_mnemonic_tag(char_u *name, int use_mnemonic)
{
char_u *buf;
char_u *psrc;
char_u *pdest;
int n_underscores = 0;
name = CONVERT_TO_UTF8(name);
if (name == NULL)
return NULL;
for (psrc = name; *psrc != NUL && *psrc != TAB; ++psrc)
if (*psrc == '_')
++n_underscores;
buf = alloc(psrc - name + n_underscores + 1);
if (buf != NULL)
{
pdest = buf;
for (psrc = name; *psrc != NUL && *psrc != TAB; ++psrc)
{
if (*psrc == '_')
{
*pdest++ = '_';
*pdest++ = '_';
}
else if (*psrc != '&')
{
*pdest++ = *psrc;
}
else if (*(psrc + 1) == '&')
{
*pdest++ = *psrc++;
}
else if (use_mnemonic)
{
*pdest++ = '_';
}
}
*pdest = NUL;
}
CONVERT_TO_UTF8_FREE(name);
return buf;
}
static void
menu_item_new(vimmenu_T *menu, GtkWidget *parent_widget)
{
GtkWidget *box;
char_u *text;
int use_mnemonic;
menu->id = gtk_menu_item_new();
#if GTK_CHECK_VERSION(3,2,0)
box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
gtk_box_set_homogeneous(GTK_BOX(box), FALSE);
#else
box = gtk_hbox_new(FALSE, 20);
#endif
use_mnemonic = (p_wak[0] != 'n' || !GTK_IS_MENU_BAR(parent_widget));
text = translate_mnemonic_tag(menu->name, use_mnemonic);
menu->label = gtk_label_new_with_mnemonic((const char *)text);
vim_free(text);
gtk_box_pack_start(GTK_BOX(box), menu->label, FALSE, FALSE, 0);
if (menu->actext != NULL && menu->actext[0] != NUL)
{
text = CONVERT_TO_UTF8(menu->actext);
gtk_box_pack_end(GTK_BOX(box),
gtk_label_new((const char *)text),
FALSE, FALSE, 0);
CONVERT_TO_UTF8_FREE(text);
}
gtk_container_add(GTK_CONTAINER(menu->id), box);
gtk_widget_show_all(menu->id);
}
void
gui_mch_add_menu(vimmenu_T *menu, int idx)
{
vimmenu_T *parent;
GtkWidget *parent_widget;
if (menu->name[0] == ']' || menu_is_popup(menu->name))
{
menu->submenu_id = gtk_menu_new();
return;
}
parent = menu->parent;
if ((parent != NULL && parent->submenu_id == NULL)
|| !menu_is_menubar(menu->name))
return;
parent_widget = (parent != NULL) ? parent->submenu_id : gui.menubar;
menu_item_new(menu, parent_widget);
#if !GTK_CHECK_VERSION(3,4,0)
if (parent != NULL && !menu_is_popup(parent->name))
++idx;
#endif
gtk_menu_shell_insert(GTK_MENU_SHELL(parent_widget), menu->id, idx);
menu->submenu_id = gtk_menu_new();
gtk_menu_set_accel_group(GTK_MENU(menu->submenu_id), gui.accel_group);
gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu->id), menu->submenu_id);
#if !GTK_CHECK_VERSION(3,4,0)
menu->tearoff_handle = gtk_tearoff_menu_item_new();
if (vim_strchr(p_go, GO_TEAROFF) != NULL)
gtk_widget_show(menu->tearoff_handle);
#if GTK_CHECK_VERSION(3,0,0)
gtk_menu_shell_prepend(GTK_MENU_SHELL(menu->submenu_id),
menu->tearoff_handle);
#else
gtk_menu_prepend(GTK_MENU(menu->submenu_id), menu->tearoff_handle);
#endif
#endif
}
static void
menu_item_activate(GtkWidget *widget UNUSED, gpointer data)
{
gui_menu_cb((vimmenu_T *)data);
}
void
gui_mch_add_menu_item(vimmenu_T *menu, int idx)
{
vimmenu_T *parent;
parent = menu->parent;
#if defined(FEAT_TOOLBAR)
if (menu_is_toolbar(parent->name))
{
GtkToolbar *toolbar;
toolbar = GTK_TOOLBAR(gui.toolbar);
menu->submenu_id = NULL;
if (menu_is_separator(menu->name))
{
#if GTK_CHECK_VERSION(3,0,0)
GtkToolItem *item = gtk_separator_tool_item_new();
gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(item),
TRUE);
gtk_tool_item_set_expand(GTK_TOOL_ITEM(item), FALSE);
gtk_widget_show(GTK_WIDGET(item));
gtk_toolbar_insert(toolbar, item, idx);
#else
gtk_toolbar_insert_space(toolbar, idx);
#endif
menu->id = NULL;
}
else
{
char_u *text;
char_u *tooltip;
text = CONVERT_TO_UTF8(menu->dname);
tooltip = CONVERT_TO_UTF8(menu->strings[MENU_INDEX_TIP]);
if (tooltip != NULL && !utf_valid_string(tooltip, NULL))
CONVERT_TO_UTF8_FREE(tooltip);
#if GTK_CHECK_VERSION(3,0,0)
{
GtkWidget *icon;
GtkToolItem *item;
icon = create_menu_icon(menu,
gtk_toolbar_get_icon_size(toolbar));
item = gtk_tool_button_new(icon, (const gchar *)text);
gtk_tool_item_set_tooltip_text(item, (const gchar *)tooltip);
g_signal_connect(G_OBJECT(item), "clicked",
G_CALLBACK(&menu_item_activate), menu);
gtk_widget_show_all(GTK_WIDGET(item));
gtk_toolbar_insert(toolbar, item, idx);
menu->id = GTK_WIDGET(item);
}
#else
menu->id = gtk_toolbar_insert_item(
toolbar,
(const char *)text,
(const char *)tooltip,
NULL,
create_menu_icon(menu, gtk_toolbar_get_icon_size(toolbar)),
G_CALLBACK(&menu_item_activate),
menu,
idx);
#endif
if (gtk_socket_id != 0)
g_signal_connect(G_OBJECT(menu->id), "focus-in-event",
G_CALLBACK(toolbar_button_focus_in_event), NULL);
CONVERT_TO_UTF8_FREE(text);
CONVERT_TO_UTF8_FREE(tooltip);
}
}
else
#endif 
{
if (parent == NULL || parent->submenu_id == NULL)
return;
#if !GTK_CHECK_VERSION(3,4,0)
if (!menu_is_popup(parent->name))
++idx;
#endif
if (menu_is_separator(menu->name))
{
#if GTK_CHECK_VERSION(3,0,0)
menu->id = gtk_separator_menu_item_new();
#else
menu->id = gtk_menu_item_new();
gtk_widget_set_sensitive(menu->id, FALSE);
#endif
gtk_widget_show(menu->id);
gtk_menu_shell_insert(GTK_MENU_SHELL(parent->submenu_id),
menu->id, idx);
return;
}
menu_item_new(menu, parent->submenu_id);
gtk_widget_show(menu->id);
gtk_menu_shell_insert(GTK_MENU_SHELL(parent->submenu_id),
menu->id, idx);
if (menu->id != NULL)
g_signal_connect(G_OBJECT(menu->id), "activate",
G_CALLBACK(menu_item_activate), menu);
}
}
#endif 
void
gui_mch_set_text_area_pos(int x, int y, int w, int h)
{
gtk_form_move_resize(GTK_FORM(gui.formwin), gui.drawarea, x, y, w, h);
}
#if defined(FEAT_MENU) || defined(PROTO)
void
gui_gtk_set_mnemonics(int enable)
{
vimmenu_T *menu;
char_u *name;
for (menu = root_menu; menu != NULL; menu = menu->next)
{
if (menu->id == NULL)
continue;
name = translate_mnemonic_tag(menu->name, enable);
gtk_label_set_text_with_mnemonic(GTK_LABEL(menu->label),
(const char *)name);
vim_free(name);
}
}
#if !GTK_CHECK_VERSION(3,4,0)
static void
recurse_tearoffs(vimmenu_T *menu, int val)
{
for (; menu != NULL; menu = menu->next)
{
if (menu->submenu_id != NULL && menu->tearoff_handle != NULL
&& menu->name[0] != ']' && !menu_is_popup(menu->name))
{
if (val)
gtk_widget_show(menu->tearoff_handle);
else
gtk_widget_hide(menu->tearoff_handle);
}
recurse_tearoffs(menu->children, val);
}
}
#endif
#if GTK_CHECK_VERSION(3,4,0)
void
gui_mch_toggle_tearoffs(int enable UNUSED)
{
}
#else
void
gui_mch_toggle_tearoffs(int enable)
{
recurse_tearoffs(root_menu, enable);
}
#endif
#endif 
#if defined(FEAT_TOOLBAR)
static int
get_menu_position(vimmenu_T *menu)
{
vimmenu_T *node;
int idx = 0;
for (node = menu->parent->children; node != menu; node = node->next)
{
g_return_val_if_fail(node != NULL, -1);
++idx;
}
return idx;
}
#endif 
#if defined(FEAT_TOOLBAR) || defined(PROTO)
void
gui_mch_menu_set_tip(vimmenu_T *menu)
{
if (menu->id != NULL && menu->parent != NULL
&& gui.toolbar != NULL && menu_is_toolbar(menu->parent->name))
{
char_u *tooltip;
tooltip = CONVERT_TO_UTF8(menu->strings[MENU_INDEX_TIP]);
if (tooltip != NULL && utf_valid_string(tooltip, NULL))
#if GTK_CHECK_VERSION(3,0,0)
gtk_widget_set_tooltip_text(menu->id, (const gchar *)tooltip);
#else
gtk_tooltips_set_tip(GTK_TOOLBAR(gui.toolbar)->tooltips,
menu->id, (const char *)tooltip, NULL);
#endif
CONVERT_TO_UTF8_FREE(tooltip);
}
}
#endif 
#if defined(FEAT_MENU) || defined(PROTO)
void
gui_mch_destroy_menu(vimmenu_T *menu)
{
if (menu->id != NULL)
g_object_ref(menu->id);
if (menu->id != NULL && gui.menubar != NULL
&& gtk_widget_get_parent(menu->id) == gui.menubar)
gtk_container_remove(GTK_CONTAINER(gui.menubar), menu->id);
#if defined(FEAT_TOOLBAR)
if (menu->parent != NULL && menu_is_toolbar(menu->parent->name))
{
if (menu_is_separator(menu->name))
#if GTK_CHECK_VERSION(3,0,0)
{
GtkToolItem *item = NULL;
item = gtk_toolbar_get_nth_item(GTK_TOOLBAR(gui.toolbar),
get_menu_position(menu));
if (item != NULL)
gtk_container_remove(GTK_CONTAINER(gui.toolbar),
GTK_WIDGET(item));
}
#else
gtk_toolbar_remove_space(GTK_TOOLBAR(gui.toolbar),
get_menu_position(menu));
#endif
else if (menu->id != NULL)
gtk_widget_destroy(menu->id);
}
else
#endif 
{
if (menu->submenu_id != NULL)
gtk_widget_destroy(menu->submenu_id);
if (menu->id != NULL)
gtk_widget_destroy(menu->id);
}
if (menu->id != NULL)
g_object_unref(menu->id);
menu->submenu_id = NULL;
menu->id = NULL;
}
#endif 
void
gui_mch_set_scrollbar_thumb(scrollbar_T *sb, long val, long size, long max)
{
if (sb->id != NULL)
{
GtkAdjustment *adjustment;
adjustment = gtk_range_get_adjustment(GTK_RANGE(sb->id));
gtk_adjustment_set_lower(adjustment, 0.0);
gtk_adjustment_set_value(adjustment, val);
gtk_adjustment_set_upper(adjustment, max + 1);
gtk_adjustment_set_page_size(adjustment, size);
gtk_adjustment_set_page_increment(adjustment,
size < 3L ? 1L : size - 2L);
gtk_adjustment_set_step_increment(adjustment, 1.0);
g_signal_handler_block(G_OBJECT(adjustment), (gulong)sb->handler_id);
#if !GTK_CHECK_VERSION(3,18,0)
gtk_adjustment_changed(adjustment);
#endif
g_signal_handler_unblock(G_OBJECT(adjustment),
(gulong)sb->handler_id);
}
}
void
gui_mch_set_scrollbar_pos(scrollbar_T *sb, int x, int y, int w, int h)
{
if (sb->id != NULL)
gtk_form_move_resize(GTK_FORM(gui.formwin), sb->id, x, y, w, h);
}
static void
adjustment_value_changed(GtkAdjustment *adjustment, gpointer data)
{
scrollbar_T *sb;
long value;
int dragging = FALSE;
#if defined(FEAT_XIM)
if (im_is_preediting())
xim_reset();
#endif
sb = gui_find_scrollbar((long)data);
value = gtk_adjustment_get_value(adjustment);
#if !GTK_CHECK_VERSION(3,0,0)
if (sb != NULL)
{
dragging = TRUE;
if (sb->wp != NULL)
{
int x;
int y;
GdkModifierType state;
int width;
int height;
gdk_window_get_pointer(sb->id->window, &x, &y, &state);
gdk_window_get_size(sb->id->window, &width, &height);
if (x >= 0 && x < width && y >= 0 && y < height)
{
if (y < width)
{
dragging = FALSE;
value = sb->wp->w_topline - 2;
}
else if (y > height - width)
{
dragging = FALSE;
value = sb->wp->w_topline;
}
}
}
}
#endif 
gui_drag_scrollbar(sb, value, dragging);
}
void
gui_mch_create_scrollbar(scrollbar_T *sb, int orient)
{
if (orient == SBAR_HORIZ)
#if GTK_CHECK_VERSION(3,2,0)
sb->id = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, NULL);
#else
sb->id = gtk_hscrollbar_new(NULL);
#endif
else if (orient == SBAR_VERT)
#if GTK_CHECK_VERSION(3,2,0)
sb->id = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, NULL);
#else
sb->id = gtk_vscrollbar_new(NULL);
#endif
if (sb->id != NULL)
{
GtkAdjustment *adjustment;
gtk_widget_set_can_focus(sb->id, FALSE);
gtk_form_put(GTK_FORM(gui.formwin), sb->id, 0, 0);
adjustment = gtk_range_get_adjustment(GTK_RANGE(sb->id));
sb->handler_id = g_signal_connect(
G_OBJECT(adjustment), "value-changed",
G_CALLBACK(adjustment_value_changed),
GINT_TO_POINTER(sb->ident));
gui_mch_update();
}
}
void
gui_mch_destroy_scrollbar(scrollbar_T *sb)
{
if (sb->id != NULL)
{
gtk_widget_destroy(sb->id);
sb->id = NULL;
}
gui_mch_update();
}
#if defined(FEAT_BROWSE) || defined(PROTO)
#if !defined(USE_FILE_CHOOSER)
static void
browse_ok_cb(GtkWidget *widget UNUSED, gpointer cbdata)
{
gui_T *vw = (gui_T *)cbdata;
if (vw->browse_fname != NULL)
g_free(vw->browse_fname);
vw->browse_fname = (char_u *)g_strdup(gtk_file_selection_get_filename(
GTK_FILE_SELECTION(vw->filedlg)));
gtk_widget_hide(vw->filedlg);
}
static void
browse_cancel_cb(GtkWidget *widget UNUSED, gpointer cbdata)
{
gui_T *vw = (gui_T *)cbdata;
if (vw->browse_fname != NULL)
{
g_free(vw->browse_fname);
vw->browse_fname = NULL;
}
gtk_widget_hide(vw->filedlg);
}
static gboolean
browse_destroy_cb(GtkWidget *widget UNUSED)
{
if (gui.browse_fname != NULL)
{
g_free(gui.browse_fname);
gui.browse_fname = NULL;
}
gui.filedlg = NULL;
gtk_main_quit();
return FALSE;
}
#endif
char_u *
gui_mch_browse(int saving UNUSED,
char_u *title,
char_u *dflt,
char_u *ext UNUSED,
char_u *initdir,
char_u *filter)
{
#if defined(USE_FILE_CHOOSER)
GtkWidget *fc;
#endif
char_u dirbuf[MAXPATHL];
guint log_handler;
const gchar *domain = "Gtk";
title = CONVERT_TO_UTF8(title);
if (initdir == NULL || *initdir == NUL)
mch_dirname(dirbuf, MAXPATHL);
else if (vim_FullName(initdir, dirbuf, MAXPATHL - 2, FALSE) == FAIL)
dirbuf[0] = NUL;
add_pathsep(dirbuf);
gui_mch_mousehide(FALSE);
log_handler = g_log_set_handler(domain, G_LOG_LEVEL_WARNING,
recent_func_log_func, NULL);
#if defined(USE_FILE_CHOOSER)
fc = gtk_file_chooser_dialog_new((const gchar *)title,
GTK_WINDOW(gui.mainwin),
saving ? GTK_FILE_CHOOSER_ACTION_SAVE
: GTK_FILE_CHOOSER_ACTION_OPEN,
#if GTK_CHECK_VERSION(3,10,0)
_("_Cancel"), GTK_RESPONSE_CANCEL,
saving ? _("_Save") : _("_Open"), GTK_RESPONSE_ACCEPT,
#else
GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
saving ? GTK_STOCK_SAVE : GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
NULL);
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fc),
(const gchar *)dirbuf);
if (filter != NULL && *filter != NUL)
{
int i = 0;
char_u *patt;
char_u *p = filter;
GtkFileFilter *gfilter;
gfilter = gtk_file_filter_new();
patt = alloc(STRLEN(filter));
while (p != NULL && *p != NUL)
{
if (*p == '\n' || *p == ';' || *p == '\t')
{
STRNCPY(patt, filter, i);
patt[i] = '\0';
if (*p == '\t')
gtk_file_filter_set_name(gfilter, (gchar *)patt);
else
{
gtk_file_filter_add_pattern(gfilter, (gchar *)patt);
if (*p == '\n')
{
gtk_file_chooser_add_filter((GtkFileChooser *)fc,
gfilter);
if (*(p + 1) != NUL)
gfilter = gtk_file_filter_new();
}
}
filter = ++p;
i = 0;
}
else
{
p++;
i++;
}
}
vim_free(patt);
}
if (saving && dflt != NULL && *dflt != NUL)
gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(fc), (char *)dflt);
gui.browse_fname = NULL;
if (gtk_dialog_run(GTK_DIALOG(fc)) == GTK_RESPONSE_ACCEPT)
{
char *filename;
filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc));
gui.browse_fname = (char_u *)g_strdup(filename);
g_free(filename);
}
gtk_widget_destroy(GTK_WIDGET(fc));
#else 
if (gui.filedlg == NULL)
{
GtkFileSelection *fs; 
gui.filedlg = gtk_file_selection_new((const gchar *)title);
gtk_window_set_modal(GTK_WINDOW(gui.filedlg), TRUE);
gtk_window_set_transient_for(GTK_WINDOW(gui.filedlg),
GTK_WINDOW(gui.mainwin));
fs = GTK_FILE_SELECTION(gui.filedlg);
gtk_container_border_width(GTK_CONTAINER(fs), 4);
gtk_signal_connect(GTK_OBJECT(fs->ok_button),
"clicked", GTK_SIGNAL_FUNC(browse_ok_cb), &gui);
gtk_signal_connect(GTK_OBJECT(fs->cancel_button),
"clicked", GTK_SIGNAL_FUNC(browse_cancel_cb), &gui);
gtk_signal_connect_object(GTK_OBJECT(gui.filedlg),
"destroy", GTK_SIGNAL_FUNC(browse_destroy_cb),
GTK_OBJECT(gui.filedlg));
}
else
gtk_window_set_title(GTK_WINDOW(gui.filedlg), (const gchar *)title);
if (dflt != NULL && *dflt != NUL
&& STRLEN(dirbuf) + 2 + STRLEN(dflt) < MAXPATHL)
STRCAT(dirbuf, dflt);
gtk_file_selection_set_filename(GTK_FILE_SELECTION(gui.filedlg),
(const gchar *)dirbuf);
gtk_widget_show(gui.filedlg);
gtk_main();
#endif 
g_log_remove_handler(domain, log_handler);
CONVERT_TO_UTF8_FREE(title);
if (gui.browse_fname == NULL)
return NULL;
return vim_strsave(shorten_fname1(gui.browse_fname));
}
char_u *
gui_mch_browsedir(
char_u *title,
char_u *initdir)
{
#if defined(GTK_FILE_CHOOSER) 
char_u dirbuf[MAXPATHL];
char_u *p;
GtkWidget *dirdlg; 
char_u *dirname = NULL;
title = CONVERT_TO_UTF8(title);
dirdlg = gtk_file_chooser_dialog_new(
(const gchar *)title,
GTK_WINDOW(gui.mainwin),
GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
#if GTK_CHECK_VERSION(3,10,0)
_("_Cancel"), GTK_RESPONSE_CANCEL,
_("_OK"), GTK_RESPONSE_ACCEPT,
#else
GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
#endif
NULL);
CONVERT_TO_UTF8_FREE(title);
gui_mch_mousehide(FALSE);
if (initdir == NULL || *initdir == NUL
|| vim_FullName(initdir, dirbuf, MAXPATHL - 10, FALSE) == FAIL)
mch_dirname(dirbuf, MAXPATHL - 10);
add_pathsep(dirbuf);
STRCAT(dirbuf, "@zd(*&1|");
gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dirdlg),
(const gchar *)dirbuf);
if (gtk_dialog_run(GTK_DIALOG(dirdlg)) == GTK_RESPONSE_ACCEPT)
dirname = (char_u *)gtk_file_chooser_get_filename(
GTK_FILE_CHOOSER(dirdlg));
gtk_widget_destroy(dirdlg);
if (dirname == NULL)
return NULL;
p = vim_strsave(shorten_fname1(dirname));
g_free(dirname);
return p;
#else 
return gui_mch_browse(0, title, NULL, NULL, initdir, NULL);
#endif 
}
#endif 
#if defined(FEAT_GUI_DIALOG) || defined(PROTO)
static GtkWidget *
create_message_dialog(int type, char_u *title, char_u *message)
{
GtkWidget *dialog;
GtkMessageType message_type;
switch (type)
{
case VIM_ERROR: message_type = GTK_MESSAGE_ERROR; break;
case VIM_WARNING: message_type = GTK_MESSAGE_WARNING; break;
case VIM_QUESTION: message_type = GTK_MESSAGE_QUESTION; break;
default: message_type = GTK_MESSAGE_INFO; break;
}
message = CONVERT_TO_UTF8(message);
dialog = gtk_message_dialog_new(GTK_WINDOW(gui.mainwin),
GTK_DIALOG_DESTROY_WITH_PARENT,
message_type,
GTK_BUTTONS_NONE,
"%s", (const char *)message);
CONVERT_TO_UTF8_FREE(message);
if (title != NULL)
{
title = CONVERT_TO_UTF8(title);
gtk_window_set_title(GTK_WINDOW(dialog), (const char *)title);
CONVERT_TO_UTF8_FREE(title);
}
else if (type == VIM_GENERIC)
{
gtk_window_set_title(GTK_WINDOW(dialog), "VIM");
}
return dialog;
}
static char **
split_button_string(char_u *button_string, int *n_buttons)
{
char **array;
char_u *p;
unsigned int count = 1;
for (p = button_string; *p != NUL; ++p)
if (*p == DLG_BUTTON_SEP)
++count;
array = ALLOC_MULT(char *, count + 1);
count = 0;
if (array != NULL)
{
array[count++] = (char *)button_string;
for (p = button_string; *p != NUL; )
{
if (*p == DLG_BUTTON_SEP)
{
*p++ = NUL;
array[count++] = (char *)p;
}
else if (*p == DLG_HOTKEY_CHAR)
*p++ = '_';
else
MB_PTR_ADV(p);
}
array[count] = NULL; 
}
*n_buttons = count;
return array;
}
static char **
split_button_translation(const char *message)
{
char **buttons = NULL;
char_u *str;
int n_buttons = 0;
int n_expected = 1;
for (str = (char_u *)message; *str != NUL; ++str)
if (*str == DLG_BUTTON_SEP)
++n_expected;
str = (char_u *)_(message);
if (str != NULL)
{
if (output_conv.vc_type != CONV_NONE)
str = string_convert(&output_conv, str, NULL);
else
str = vim_strsave(str);
if (str != NULL)
buttons = split_button_string(str, &n_buttons);
}
if (buttons == NULL || n_buttons != n_expected)
{
vim_free(buttons);
vim_free(str);
buttons = NULL;
str = vim_strsave((char_u *)message);
if (str != NULL)
buttons = split_button_string(str, &n_buttons);
if (buttons == NULL)
vim_free(str);
}
return buttons;
}
static int
button_equal(const char *a, const char *b)
{
while (*a != '\0' && *b != '\0')
{
if (*a == '_' && *++a == '\0')
break;
if (*b == '_' && *++b == '\0')
break;
if (g_unichar_tolower(g_utf8_get_char(a))
!= g_unichar_tolower(g_utf8_get_char(b)))
return FALSE;
a = g_utf8_next_char(a);
b = g_utf8_next_char(b);
}
return (*a == '\0' && *b == '\0');
}
static void
dialog_add_buttons(GtkDialog *dialog, char_u *button_string)
{
char **ok;
char **ync; 
char **buttons;
int n_buttons = 0;
int idx;
button_string = vim_strsave(button_string); 
if (button_string == NULL)
return;
if (vim_strchr(p_go, GO_VERTICAL) != NULL)
{
#if GTK_CHECK_VERSION(3,0,0)
#else
GtkWidget *vbutton_box;
vbutton_box = gtk_vbutton_box_new();
gtk_widget_show(vbutton_box);
gtk_box_pack_end(GTK_BOX(GTK_DIALOG(dialog)->vbox),
vbutton_box, TRUE, FALSE, 0);
GTK_DIALOG(dialog)->action_area = vbutton_box;
#endif
}
ok = split_button_translation(N_("&Ok"));
ync = split_button_translation(N_("&Yes\n&No\n&Cancel"));
buttons = split_button_string(button_string, &n_buttons);
for (idx = 1; idx <= n_buttons; ++idx)
{
char *label;
char_u *label8;
label = buttons[idx - 1];
if (ok != NULL && ync != NULL) 
{
#if GTK_CHECK_VERSION(3,10,0)
if (button_equal(label, ok[0])) label = _("OK");
else if (button_equal(label, ync[0])) label = _("Yes");
else if (button_equal(label, ync[1])) label = _("No");
else if (button_equal(label, ync[2])) label = _("Cancel");
else if (button_equal(label, "Ok")) label = _("OK");
else if (button_equal(label, "Yes")) label = _("Yes");
else if (button_equal(label, "No")) label = _("No");
else if (button_equal(label, "Cancel")) label = _("Cancel");
#else
if (button_equal(label, ok[0])) label = GTK_STOCK_OK;
else if (button_equal(label, ync[0])) label = GTK_STOCK_YES;
else if (button_equal(label, ync[1])) label = GTK_STOCK_NO;
else if (button_equal(label, ync[2])) label = GTK_STOCK_CANCEL;
else if (button_equal(label, "Ok")) label = GTK_STOCK_OK;
else if (button_equal(label, "Yes")) label = GTK_STOCK_YES;
else if (button_equal(label, "No")) label = GTK_STOCK_NO;
else if (button_equal(label, "Cancel")) label = GTK_STOCK_CANCEL;
#endif
}
label8 = CONVERT_TO_UTF8((char_u *)label);
gtk_dialog_add_button(dialog, (const gchar *)label8, idx);
CONVERT_TO_UTF8_FREE(label8);
}
if (ok != NULL)
vim_free(*ok);
if (ync != NULL)
vim_free(*ync);
vim_free(ok);
vim_free(ync);
vim_free(buttons);
vim_free(button_string);
}
typedef struct _DialogInfo
{
int ignore_enter; 
int noalt; 
GtkDialog *dialog; 
} DialogInfo;
static gboolean
dialog_key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
DialogInfo *di = (DialogInfo *)data;
if (di->ignore_enter && (event->keyval == GDK_Return
|| event->keyval == ' '))
return TRUE;
else 
di->ignore_enter = FALSE;
if (event->keyval == GDK_Escape)
{
gtk_dialog_response(di->dialog, GTK_RESPONSE_REJECT);
return TRUE;
}
if (di->noalt
&& (event->state & gtk_accelerator_get_default_mod_mask()) == 0)
{
return gtk_window_mnemonic_activate(
GTK_WINDOW(widget), event->keyval,
gtk_window_get_mnemonic_modifier(GTK_WINDOW(widget)));
}
return FALSE; 
}
int
gui_mch_dialog(int type, 
char_u *title, 
char_u *message, 
char_u *buttons, 
int def_but, 
char_u *textfield, 
int ex_cmd UNUSED)
{
GtkWidget *dialog;
GtkWidget *entry = NULL;
char_u *text;
int response;
DialogInfo dialoginfo;
dialog = create_message_dialog(type, title, message);
dialoginfo.dialog = GTK_DIALOG(dialog);
dialog_add_buttons(GTK_DIALOG(dialog), buttons);
if (textfield != NULL)
{
GtkWidget *alignment;
entry = gtk_entry_new();
gtk_widget_show(entry);
gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
text = CONVERT_TO_UTF8(textfield);
gtk_entry_set_text(GTK_ENTRY(entry), (const char *)text);
CONVERT_TO_UTF8_FREE(text);
#if GTK_CHECK_VERSION(3,14,0)
gtk_widget_set_halign(GTK_WIDGET(entry), GTK_ALIGN_CENTER);
gtk_widget_set_valign(GTK_WIDGET(entry), GTK_ALIGN_CENTER);
gtk_widget_set_hexpand(GTK_WIDGET(entry), TRUE);
gtk_widget_set_vexpand(GTK_WIDGET(entry), TRUE);
alignment = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
alignment = gtk_alignment_new((float)0.5, (float)0.5,
(float)1.0, (float)1.0);
#endif
gtk_container_add(GTK_CONTAINER(alignment), entry);
gtk_container_set_border_width(GTK_CONTAINER(alignment), 5);
gtk_widget_show(alignment);
#if GTK_CHECK_VERSION(3,0,0)
{
GtkWidget * const vbox
= gtk_dialog_get_content_area(GTK_DIALOG(dialog));
gtk_box_pack_start(GTK_BOX(vbox),
alignment, TRUE, FALSE, 0);
}
#else
gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
alignment, TRUE, FALSE, 0);
#endif
dialoginfo.noalt = FALSE;
}
else
dialoginfo.noalt = TRUE;
g_signal_connect(G_OBJECT(dialog), "key-press-event",
G_CALLBACK(&dialog_key_press_event_cb), &dialoginfo);
if (def_but > 0)
{
gtk_dialog_set_default_response(GTK_DIALOG(dialog), def_but);
dialoginfo.ignore_enter = FALSE;
}
else
dialoginfo.ignore_enter = TRUE;
gui_mch_mousehide(FALSE);
response = gtk_dialog_run(GTK_DIALOG(dialog));
if (response != GTK_RESPONSE_NONE)
{
if (response == GTK_RESPONSE_ACCEPT) 
response = def_but;
if (textfield != NULL)
{
text = (char_u *)gtk_entry_get_text(GTK_ENTRY(entry));
text = CONVERT_FROM_UTF8(text);
vim_strncpy(textfield, text, IOSIZE - 1);
CONVERT_FROM_UTF8_FREE(text);
}
gtk_widget_destroy(dialog);
}
return response > 0 ? response : 0;
}
#endif 
#if defined(FEAT_MENU) || defined(PROTO)
void
gui_mch_show_popupmenu(vimmenu_T *menu)
{
#if defined(FEAT_XIM)
#if !GTK_CHECK_VERSION(3,10,0)
if (xic != NULL && g_object_get_data(G_OBJECT(menu->submenu_id),
"vim-has-im-menu") == NULL)
{
GtkWidget *menuitem;
GtkWidget *submenu;
char_u *name;
menuitem = gtk_separator_menu_item_new();
gtk_widget_show(menuitem);
gtk_menu_shell_append(GTK_MENU_SHELL(menu->submenu_id), menuitem);
name = (char_u *)_("Input _Methods");
name = CONVERT_TO_UTF8(name);
menuitem = gtk_menu_item_new_with_mnemonic((const char *)name);
CONVERT_TO_UTF8_FREE(name);
gtk_widget_show(menuitem);
submenu = gtk_menu_new();
gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);
gtk_menu_shell_append(GTK_MENU_SHELL(menu->submenu_id), menuitem);
gtk_im_multicontext_append_menuitems(GTK_IM_MULTICONTEXT(xic),
GTK_MENU_SHELL(submenu));
g_object_set_data(G_OBJECT(menu->submenu_id),
"vim-has-im-menu", GINT_TO_POINTER(TRUE));
}
#endif
#endif 
#if GTK_CHECK_VERSION(3,22,2)
{
GdkEventButton trigger;
trigger.type = GDK_BUTTON_PRESS;
trigger.window = gtk_widget_get_window(gui.drawarea);
trigger.send_event = FALSE;
trigger.time = gui.event_time;
trigger.x = 0.0;
trigger.y = 0.0;
trigger.axes = NULL;
trigger.state = 0;
trigger.button = 3;
trigger.device = NULL;
trigger.x_root = 0.0;
trigger.y_root = 0.0;
gtk_menu_popup_at_pointer(GTK_MENU(menu->submenu_id),
(GdkEvent *)&trigger);
}
#else
gtk_menu_popup(GTK_MENU(menu->submenu_id),
NULL, NULL,
(GtkMenuPositionFunc)NULL, NULL,
3U, gui.event_time);
#endif
}
static int popup_mouse_pos;
static void
popup_menu_position_func(GtkMenu *menu UNUSED,
gint *x, gint *y,
gboolean *push_in UNUSED,
gpointer user_data UNUSED)
{
gdk_window_get_origin(gtk_widget_get_window(gui.drawarea), x, y);
if (popup_mouse_pos)
{
int mx, my;
gui_mch_getmouse(&mx, &my);
*x += mx;
*y += my;
}
else if (curwin != NULL && gui.drawarea != NULL &&
gtk_widget_get_window(gui.drawarea) != NULL)
{
*x += FILL_X(curwin->w_wincol + curwin->w_wcol + 1) + 1;
*y += FILL_Y(W_WINROW(curwin) + curwin->w_wrow + 1) + 1;
}
}
void
gui_make_popup(char_u *path_name, int mouse_pos)
{
vimmenu_T *menu;
popup_mouse_pos = mouse_pos;
menu = gui_find_menu(path_name);
if (menu != NULL && menu->submenu_id != NULL)
{
#if GTK_CHECK_VERSION(3,22,2)
GdkWindow * const win = gtk_widget_get_window(gui.drawarea);
GdkEventButton trigger;
trigger.type = GDK_BUTTON_PRESS;
trigger.window = win;
trigger.send_event = FALSE;
trigger.time = GDK_CURRENT_TIME;
trigger.x = 0.0;
trigger.y = 0.0;
trigger.axes = NULL;
trigger.state = 0;
trigger.button = 0;
trigger.device = NULL;
trigger.x_root = 0.0;
trigger.y_root = 0.0;
if (mouse_pos)
gtk_menu_popup_at_pointer(GTK_MENU(menu->submenu_id),
(GdkEvent *)&trigger);
else
{
gint origin_x, origin_y;
GdkRectangle rect = { 0, 0, 0, 0 };
gdk_window_get_origin(win, &origin_x, &origin_y);
popup_menu_position_func(NULL, &rect.x, &rect.y, NULL, NULL);
rect.x -= origin_x;
rect.y -= origin_y;
gtk_menu_popup_at_rect(GTK_MENU(menu->submenu_id),
win,
&rect,
GDK_GRAVITY_SOUTH_EAST,
GDK_GRAVITY_NORTH_WEST,
(GdkEvent *)&trigger);
}
#else
gtk_menu_popup(GTK_MENU(menu->submenu_id),
NULL, NULL,
&popup_menu_position_func, NULL,
0U, (guint32)GDK_CURRENT_TIME);
#endif
}
}
#endif 
typedef struct _SharedFindReplace
{
GtkWidget *dialog; 
GtkWidget *wword; 
GtkWidget *mcase; 
GtkWidget *up; 
GtkWidget *down; 
GtkWidget *what; 
GtkWidget *with; 
GtkWidget *find; 
GtkWidget *replace; 
GtkWidget *all; 
} SharedFindReplace;
static SharedFindReplace find_widgets = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static SharedFindReplace repl_widgets = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static int
find_key_press_event(
GtkWidget *widget UNUSED,
GdkEventKey *event,
SharedFindReplace *frdp)
{
if (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))
return FALSE;
if (event->keyval == GDK_Escape)
{
gtk_widget_hide(frdp->dialog);
return TRUE;
}
return FALSE;
}
static GtkWidget *
#if GTK_CHECK_VERSION(3,10,0)
create_image_button(const char *stock_id UNUSED,
const char *label)
#else
create_image_button(const char *stock_id,
const char *label)
#endif
{
char_u *text;
GtkWidget *box;
GtkWidget *alignment;
GtkWidget *button;
text = CONVERT_TO_UTF8((char_u *)label);
#if GTK_CHECK_VERSION(3,2,0)
box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
gtk_box_set_homogeneous(GTK_BOX(box), FALSE);
#else
box = gtk_hbox_new(FALSE, 3);
#endif
#if !GTK_CHECK_VERSION(3,10,0)
if (stock_id != NULL)
gtk_box_pack_start(GTK_BOX(box),
gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_BUTTON),
FALSE, FALSE, 0);
#endif
gtk_box_pack_start(GTK_BOX(box),
gtk_label_new((const char *)text),
FALSE, FALSE, 0);
CONVERT_TO_UTF8_FREE(text);
#if GTK_CHECK_VERSION(3,14,0)
gtk_widget_set_halign(GTK_WIDGET(box), GTK_ALIGN_CENTER);
gtk_widget_set_valign(GTK_WIDGET(box), GTK_ALIGN_CENTER);
gtk_widget_set_hexpand(GTK_WIDGET(box), TRUE);
gtk_widget_set_vexpand(GTK_WIDGET(box), TRUE);
alignment = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
alignment = gtk_alignment_new((float)0.5, (float)0.5,
(float)0.0, (float)0.0);
#endif
gtk_container_add(GTK_CONTAINER(alignment), box);
gtk_widget_show_all(alignment);
button = gtk_button_new();
gtk_container_add(GTK_CONTAINER(button), alignment);
return button;
}
static const char *
convert_localized_message(char_u **buffer, const char *message)
{
if (output_conv.vc_type == CONV_NONE)
return message;
vim_free(*buffer);
*buffer = string_convert(&output_conv, (char_u *)message, NULL);
return (const char *)*buffer;
}
static unsigned long
entry_get_text_length(GtkEntry *entry)
{
g_return_val_if_fail(entry != NULL, 0);
g_return_val_if_fail(GTK_IS_ENTRY(entry) == TRUE, 0);
#if GTK_CHECK_VERSION(2,18,0)
return gtk_entry_buffer_get_length(gtk_entry_get_buffer(entry));
#elif GTK_CHECK_VERSION(2,14,0)
return gtk_entry_get_text_length(entry);
#else
return g_utf8_strlen(gtk_entry_get_text(entry), -1);
#endif
}
static void
find_replace_dialog_create(char_u *arg, int do_replace)
{
GtkWidget *hbox; 
GtkWidget *actionarea;
GtkWidget *table;
GtkWidget *tmp;
GtkWidget *vbox;
gboolean sensitive;
SharedFindReplace *frdp;
char_u *entry_text;
int wword = FALSE;
int mcase = !p_ic;
char_u *conv_buffer = NULL;
#define CONV(message) convert_localized_message(&conv_buffer, (message))
frdp = (do_replace) ? (&repl_widgets) : (&find_widgets);
entry_text = get_find_dialog_text(arg, &wword, &mcase);
if (entry_text != NULL && output_conv.vc_type != CONV_NONE)
{
char_u *old_text = entry_text;
entry_text = string_convert(&output_conv, entry_text, NULL);
vim_free(old_text);
}
if (frdp->dialog)
{
if (entry_text != NULL)
{
gtk_entry_set_text(GTK_ENTRY(frdp->what), (char *)entry_text);
gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(frdp->wword),
(gboolean)wword);
gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(frdp->mcase),
(gboolean)mcase);
}
gtk_window_present(GTK_WINDOW(frdp->dialog));
gtk_widget_grab_focus(frdp->what);
if (do_replace && entry_get_text_length(GTK_ENTRY(frdp->what)) > 0)
gtk_widget_grab_focus(frdp->with);
vim_free(entry_text);
return;
}
frdp->dialog = gtk_dialog_new();
#if GTK_CHECK_VERSION(3,0,0)
#else
gtk_dialog_set_has_separator(GTK_DIALOG(frdp->dialog), FALSE);
#endif
gtk_window_set_transient_for(GTK_WINDOW(frdp->dialog), GTK_WINDOW(gui.mainwin));
gtk_window_set_destroy_with_parent(GTK_WINDOW(frdp->dialog), TRUE);
if (do_replace)
{
gtk_window_set_title(GTK_WINDOW(frdp->dialog),
CONV(_("VIM - Search and Replace...")));
}
else
{
gtk_window_set_title(GTK_WINDOW(frdp->dialog),
CONV(_("VIM - Search...")));
}
#if GTK_CHECK_VERSION(3,2,0)
hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
gtk_box_set_homogeneous(GTK_BOX(hbox), FALSE);
#else
hbox = gtk_hbox_new(FALSE, 0);
#endif
gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
#if GTK_CHECK_VERSION(3,0,0)
{
GtkWidget * const dialog_vbox
= gtk_dialog_get_content_area(GTK_DIALOG(frdp->dialog));
gtk_container_add(GTK_CONTAINER(dialog_vbox), hbox);
}
#else
gtk_container_add(GTK_CONTAINER(GTK_DIALOG(frdp->dialog)->vbox), hbox);
#endif
if (do_replace)
#if GTK_CHECK_VERSION(3,4,0)
table = gtk_grid_new();
#else
table = gtk_table_new(1024, 4, FALSE);
#endif
else
#if GTK_CHECK_VERSION(3,4,0)
table = gtk_grid_new();
#else
table = gtk_table_new(1024, 3, FALSE);
#endif
gtk_box_pack_start(GTK_BOX(hbox), table, TRUE, TRUE, 0);
gtk_container_set_border_width(GTK_CONTAINER(table), 4);
tmp = gtk_label_new(CONV(_("Find what:")));
#if GTK_CHECK_VERSION(3,16,0)
gtk_label_set_xalign(GTK_LABEL(tmp), 0.0);
gtk_label_set_yalign(GTK_LABEL(tmp), 0.5);
#elif GTK_CHECK_VERSION(3,14,0)
{
GValue align_val = G_VALUE_INIT;
g_value_init(&align_val, G_TYPE_FLOAT);
g_value_set_float(&align_val, 0.0);
g_object_set_property(G_OBJECT(tmp), "xalign", &align_val);
g_value_set_float(&align_val, 0.5);
g_object_set_property(G_OBJECT(tmp), "yalign", &align_val);
g_value_unset(&align_val);
}
#else
gtk_misc_set_alignment(GTK_MISC(tmp), (gfloat)0.0, (gfloat)0.5);
#endif
#if GTK_CHECK_VERSION(3,4,0)
gtk_grid_attach(GTK_GRID(table), tmp, 0, 0, 2, 1);
#else
gtk_table_attach(GTK_TABLE(table), tmp, 0, 1, 0, 1,
GTK_FILL, GTK_EXPAND, 2, 2);
#endif
frdp->what = gtk_entry_new();
sensitive = (entry_text != NULL && entry_text[0] != NUL);
if (entry_text != NULL)
gtk_entry_set_text(GTK_ENTRY(frdp->what), (char *)entry_text);
g_signal_connect(G_OBJECT(frdp->what), "changed",
G_CALLBACK(entry_changed_cb), frdp->dialog);
g_signal_connect_after(G_OBJECT(frdp->what), "key-press-event",
G_CALLBACK(find_key_press_event),
(gpointer) frdp);
#if GTK_CHECK_VERSION(3,4,0)
gtk_grid_attach(GTK_GRID(table), frdp->what, 2, 0, 5, 1);
#else
gtk_table_attach(GTK_TABLE(table), frdp->what, 1, 1024, 0, 1,
GTK_EXPAND | GTK_FILL, GTK_EXPAND, 2, 2);
#endif
if (do_replace)
{
tmp = gtk_label_new(CONV(_("Replace with:")));
#if GTK_CHECK_VERSION(3,16,0)
gtk_label_set_xalign(GTK_LABEL(tmp), 0.0);
gtk_label_set_yalign(GTK_LABEL(tmp), 0.5);
#elif GTK_CHECK_VERSION(3,14,0)
{
GValue align_val = G_VALUE_INIT;
g_value_init(&align_val, G_TYPE_FLOAT);
g_value_set_float(&align_val, 0.0);
g_object_set_property(G_OBJECT(tmp), "xalign", &align_val);
g_value_set_float(&align_val, 0.5);
g_object_set_property(G_OBJECT(tmp), "yalign", &align_val);
g_value_unset(&align_val);
}
#else
gtk_misc_set_alignment(GTK_MISC(tmp), (gfloat)0.0, (gfloat)0.5);
#endif
#if GTK_CHECK_VERSION(3,4,0)
gtk_grid_attach(GTK_GRID(table), tmp, 0, 1, 2, 1);
#else
gtk_table_attach(GTK_TABLE(table), tmp, 0, 1, 1, 2,
GTK_FILL, GTK_EXPAND, 2, 2);
#endif
frdp->with = gtk_entry_new();
g_signal_connect(G_OBJECT(frdp->with), "activate",
G_CALLBACK(find_replace_cb),
GINT_TO_POINTER(FRD_R_FINDNEXT));
g_signal_connect_after(G_OBJECT(frdp->with), "key-press-event",
G_CALLBACK(find_key_press_event),
(gpointer) frdp);
#if GTK_CHECK_VERSION(3,4,0)
gtk_grid_attach(GTK_GRID(table), frdp->with, 2, 1, 5, 1);
#else
gtk_table_attach(GTK_TABLE(table), frdp->with, 1, 1024, 1, 2,
GTK_EXPAND | GTK_FILL, GTK_EXPAND, 2, 2);
#endif
g_signal_connect(G_OBJECT(frdp->what), "activate",
G_CALLBACK(entry_activate_cb), frdp->with);
}
else
{
g_signal_connect(G_OBJECT(frdp->what), "activate",
G_CALLBACK(find_replace_cb),
GINT_TO_POINTER(FRD_FINDNEXT));
}
frdp->wword = gtk_check_button_new_with_label(CONV(_("Match whole word only")));
gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(frdp->wword),
(gboolean)wword);
if (do_replace)
#if GTK_CHECK_VERSION(3,4,0)
gtk_grid_attach(GTK_GRID(table), frdp->wword, 0, 2, 5, 1);
#else
gtk_table_attach(GTK_TABLE(table), frdp->wword, 0, 1023, 2, 3,
GTK_FILL, GTK_EXPAND, 2, 2);
#endif
else
#if GTK_CHECK_VERSION(3,4,0)
gtk_grid_attach(GTK_GRID(table), frdp->wword, 0, 3, 5, 1);
#else
gtk_table_attach(GTK_TABLE(table), frdp->wword, 0, 1023, 1, 2,
GTK_FILL, GTK_EXPAND, 2, 2);
#endif
frdp->mcase = gtk_check_button_new_with_label(CONV(_("Match case")));
gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(frdp->mcase),
(gboolean)mcase);
if (do_replace)
#if GTK_CHECK_VERSION(3,4,0)
gtk_grid_attach(GTK_GRID(table), frdp->mcase, 0, 3, 5, 1);
#else
gtk_table_attach(GTK_TABLE(table), frdp->mcase, 0, 1023, 3, 4,
GTK_FILL, GTK_EXPAND, 2, 2);
#endif
else
#if GTK_CHECK_VERSION(3,4,0)
gtk_grid_attach(GTK_GRID(table), frdp->mcase, 0, 4, 5, 1);
#else
gtk_table_attach(GTK_TABLE(table), frdp->mcase, 0, 1023, 2, 3,
GTK_FILL, GTK_EXPAND, 2, 2);
#endif
tmp = gtk_frame_new(CONV(_("Direction")));
if (do_replace)
#if GTK_CHECK_VERSION(3,4,0)
gtk_grid_attach(GTK_GRID(table), tmp, 5, 2, 2, 4);
#else
gtk_table_attach(GTK_TABLE(table), tmp, 1023, 1024, 2, 4,
GTK_FILL, GTK_FILL, 2, 2);
#endif
else
#if GTK_CHECK_VERSION(3,4,0)
gtk_grid_attach(GTK_GRID(table), tmp, 5, 2, 1, 3);
#else
gtk_table_attach(GTK_TABLE(table), tmp, 1023, 1024, 1, 3,
GTK_FILL, GTK_FILL, 2, 2);
#endif
#if GTK_CHECK_VERSION(3,2,0)
vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
gtk_box_set_homogeneous(GTK_BOX(vbox), FALSE);
#else
vbox = gtk_vbox_new(FALSE, 0);
#endif
gtk_container_set_border_width(GTK_CONTAINER(vbox), 0);
gtk_container_add(GTK_CONTAINER(tmp), vbox);
frdp->up = gtk_radio_button_new_with_label(NULL, CONV(_("Up")));
gtk_box_pack_start(GTK_BOX(vbox), frdp->up, TRUE, TRUE, 0);
frdp->down = gtk_radio_button_new_with_label(
gtk_radio_button_get_group(GTK_RADIO_BUTTON(frdp->up)),
CONV(_("Down")));
gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(frdp->down), TRUE);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 2);
gtk_box_pack_start(GTK_BOX(vbox), frdp->down, TRUE, TRUE, 0);
#if GTK_CHECK_VERSION(3,2,0)
actionarea = gtk_button_box_new(GTK_ORIENTATION_VERTICAL);
#else
actionarea = gtk_vbutton_box_new();
#endif
gtk_container_set_border_width(GTK_CONTAINER(actionarea), 2);
gtk_box_pack_end(GTK_BOX(hbox), actionarea, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3,10,0)
frdp->find = create_image_button(NULL, _("Find Next"));
#else
frdp->find = create_image_button(GTK_STOCK_FIND, _("Find Next"));
#endif
gtk_widget_set_sensitive(frdp->find, sensitive);
g_signal_connect(G_OBJECT(frdp->find), "clicked",
G_CALLBACK(find_replace_cb),
(do_replace) ? GINT_TO_POINTER(FRD_R_FINDNEXT)
: GINT_TO_POINTER(FRD_FINDNEXT));
gtk_widget_set_can_default(frdp->find, TRUE);
gtk_box_pack_start(GTK_BOX(actionarea), frdp->find, FALSE, FALSE, 0);
gtk_widget_grab_default(frdp->find);
if (do_replace)
{
#if GTK_CHECK_VERSION(3,10,0)
frdp->replace = create_image_button(NULL, _("Replace"));
#else
frdp->replace = create_image_button(GTK_STOCK_CONVERT, _("Replace"));
#endif
gtk_widget_set_sensitive(frdp->replace, sensitive);
gtk_widget_set_can_default(frdp->find, TRUE);
gtk_box_pack_start(GTK_BOX(actionarea), frdp->replace, FALSE, FALSE, 0);
g_signal_connect(G_OBJECT(frdp->replace), "clicked",
G_CALLBACK(find_replace_cb),
GINT_TO_POINTER(FRD_REPLACE));
#if GTK_CHECK_VERSION(3,10,0)
frdp->all = create_image_button(NULL, _("Replace All"));
#else
frdp->all = create_image_button(GTK_STOCK_CONVERT, _("Replace All"));
#endif
gtk_widget_set_sensitive(frdp->all, sensitive);
gtk_widget_set_can_default(frdp->all, TRUE);
gtk_box_pack_start(GTK_BOX(actionarea), frdp->all, FALSE, FALSE, 0);
g_signal_connect(G_OBJECT(frdp->all), "clicked",
G_CALLBACK(find_replace_cb),
GINT_TO_POINTER(FRD_REPLACEALL));
}
#if GTK_CHECK_VERSION(3,10,0)
tmp = gtk_button_new_with_mnemonic(_("_Close"));
#else
tmp = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
#endif
gtk_widget_set_can_default(tmp, TRUE);
gtk_box_pack_end(GTK_BOX(actionarea), tmp, FALSE, FALSE, 0);
g_signal_connect_swapped(G_OBJECT(tmp),
"clicked", G_CALLBACK(gtk_widget_hide),
G_OBJECT(frdp->dialog));
g_signal_connect_swapped(G_OBJECT(frdp->dialog),
"delete-event", G_CALLBACK(gtk_widget_hide_on_delete),
G_OBJECT(frdp->dialog));
#if GTK_CHECK_VERSION(3,2,0)
tmp = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
#else
tmp = gtk_vseparator_new();
#endif
gtk_box_pack_end(GTK_BOX(hbox), tmp, FALSE, FALSE, 10);
#if GTK_CHECK_VERSION(3,0,0)
#if !GTK_CHECK_VERSION(3,12,0)
gtk_widget_hide(gtk_dialog_get_action_area(GTK_DIALOG(frdp->dialog)));
#endif
#else
gtk_widget_hide(GTK_DIALOG(frdp->dialog)->action_area);
#endif
gtk_widget_show_all(hbox);
gtk_widget_show(frdp->dialog);
vim_free(entry_text);
vim_free(conv_buffer);
#undef CONV
}
void
gui_mch_find_dialog(exarg_T *eap)
{
if (gui.in_use)
find_replace_dialog_create(eap->arg, FALSE);
}
void
gui_mch_replace_dialog(exarg_T *eap)
{
if (gui.in_use)
find_replace_dialog_create(eap->arg, TRUE);
}
static void
find_replace_cb(GtkWidget *widget UNUSED, gpointer data)
{
int flags;
char_u *find_text;
char_u *repl_text;
gboolean direction_down;
SharedFindReplace *sfr;
flags = (int)(long)data; 
if (flags == FRD_FINDNEXT)
{
repl_text = NULL;
sfr = &find_widgets;
}
else
{
repl_text = (char_u *)gtk_entry_get_text(GTK_ENTRY(repl_widgets.with));
sfr = &repl_widgets;
}
find_text = (char_u *)gtk_entry_get_text(GTK_ENTRY(sfr->what));
direction_down = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sfr->down));
if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sfr->wword)))
flags |= FRD_WHOLE_WORD;
if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sfr->mcase)))
flags |= FRD_MATCH_CASE;
repl_text = CONVERT_FROM_UTF8(repl_text);
find_text = CONVERT_FROM_UTF8(find_text);
gui_do_findrepl(flags, find_text, repl_text, direction_down);
CONVERT_FROM_UTF8_FREE(repl_text);
CONVERT_FROM_UTF8_FREE(find_text);
}
static void
entry_activate_cb(GtkWidget *widget UNUSED, gpointer data)
{
gtk_widget_grab_focus(GTK_WIDGET(data));
}
static void
entry_changed_cb(GtkWidget * entry, GtkWidget * dialog)
{
const gchar *entry_text;
gboolean nonempty;
entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
if (!entry_text)
return; 
nonempty = (entry_text[0] != '\0');
if (dialog == find_widgets.dialog)
gtk_widget_set_sensitive(find_widgets.find, nonempty);
if (dialog == repl_widgets.dialog)
{
gtk_widget_set_sensitive(repl_widgets.find, nonempty);
gtk_widget_set_sensitive(repl_widgets.replace, nonempty);
gtk_widget_set_sensitive(repl_widgets.all, nonempty);
}
}
void
ex_helpfind(exarg_T *eap UNUSED)
{
do_cmdline_cmd((char_u *)"emenu ToolBar.FindHelp");
}
#if defined(FEAT_BROWSE) || defined(PROTO)
static void
recent_func_log_func(const gchar *log_domain UNUSED,
GLogLevelFlags log_level UNUSED,
const gchar *message UNUSED,
gpointer user_data UNUSED)
{
}
#endif
