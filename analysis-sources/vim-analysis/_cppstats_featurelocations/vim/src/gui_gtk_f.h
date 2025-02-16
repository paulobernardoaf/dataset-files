







#if !defined(__GTK_FORM_H__)
#define __GTK_FORM_H__

#if defined(USE_GTK3)
#include <gtk/gtk.h>
#else
#include <gdk/gdk.h>
#include <gtk/gtkcontainer.h>
#endif


#if defined(__cplusplus)
extern "C" {
#endif

#define GTK_TYPE_FORM (gtk_form_get_type ())
#if defined(USE_GTK3)
#define GTK_FORM(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_FORM, GtkForm))
#define GTK_FORM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_FORM, GtkFormClass))
#define GTK_IS_FORM(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_FORM))
#define GTK_IS_FORM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_FORM))
#else
#define GTK_FORM(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_FORM, GtkForm))
#define GTK_FORM_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_FORM, GtkFormClass))
#define GTK_IS_FORM(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_FORM))
#define GTK_IS_FORM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_FORM))
#endif


typedef struct _GtkForm GtkForm;
typedef struct _GtkFormClass GtkFormClass;

struct _GtkForm
{
GtkContainer container;

GList *children;
GdkWindow *bin_window;
gint freeze_count;
};

struct _GtkFormClass
{
GtkContainerClass parent_class;
};

#if defined(USE_GTK3)
GType gtk_form_get_type(void);
#else
GtkType gtk_form_get_type(void);
#endif

GtkWidget *gtk_form_new(void);

void gtk_form_put(GtkForm * form, GtkWidget * widget,
gint x, gint y);

void gtk_form_move(GtkForm *form, GtkWidget * widget,
gint x, gint y);

void gtk_form_move_resize(GtkForm * form, GtkWidget * widget,
gint x, gint y,
gint w, gint h);





void gtk_form_freeze(GtkForm *form);
void gtk_form_thaw(GtkForm *form);


#if defined(__cplusplus)
}
#endif
#endif 
