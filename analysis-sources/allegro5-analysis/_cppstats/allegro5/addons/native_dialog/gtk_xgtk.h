#include <gtk/gtk.h>
struct ALLEGRO_DISPLAY_XGLX_GTK {
GtkWidget *gtkwindow;
GtkWidget *gtksocket;
};
bool _al_gtk_set_display_overridable_interface(bool on);
GtkWidget *_al_gtk_get_window(ALLEGRO_DISPLAY *display);
