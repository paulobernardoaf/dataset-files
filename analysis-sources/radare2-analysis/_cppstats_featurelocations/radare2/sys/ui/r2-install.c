



#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>


#define TYPE_MAIN (main_get_type ())
#define MAIN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_MAIN, Main))
#define MAIN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MAIN, MainClass))
#define IS_MAIN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MAIN))
#define IS_MAIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MAIN))
#define MAIN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_MAIN, MainClass))

typedef struct _Main Main;
typedef struct _MainClass MainClass;
typedef struct _MainPrivate MainPrivate;
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

struct _Main {
GtkWindow parent_instance;
MainPrivate * priv;
};

struct _MainClass {
GtkWindowClass parent_class;
};


static gpointer main_parent_class = NULL;

GType main_get_type (void) G_GNUC_CONST;
enum {
MAIN_DUMMY_PROPERTY
};
void main_main (gchar** args, int args_length1);
Main* main_new (void);
Main* main_construct (GType object_type);
static GObject * main_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);


void main_main (gchar** args, int args_length1) {
Main* _tmp0_;
Main* _tmp1_;
Main* m;
gtk_init (&args_length1, &args);
_tmp0_ = main_new ();
_tmp1_ = g_object_ref_sink (_tmp0_);
m = _tmp1_;
gtk_widget_show_all ((GtkWidget*) m);
gtk_main ();
_g_object_unref0 (m);
}


int main (int argc, char ** argv) {
g_type_init ();
main_main (argv, argc);
return 0;
}


Main* main_construct (GType object_type) {
Main * self = NULL;
self = (Main*) g_object_new (object_type, NULL);
return self;
}


Main* main_new (void) {
return main_construct (TYPE_MAIN);
}


static GObject * main_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
GObject * obj;
GObjectClass * parent_class;
Main * self;
GtkVBox* _vbox0 = NULL;
GtkLabel* _label0 = NULL;
GtkHBox* _hbox0 = NULL;
GtkButton* _button0 = NULL;
GtkButton* _button1 = NULL;
GtkButton* _button2 = NULL;
GtkVBox* _tmp0_;
GtkVBox* _tmp1_;
GtkLabel* _tmp2_;
GtkLabel* _tmp3_;
GtkHBox* _tmp4_;
GtkHBox* _tmp5_;
GtkButton* _tmp6_;
GtkButton* _tmp7_;
GtkButton* _tmp8_;
GtkButton* _tmp9_;
GtkButton* _tmp10_;
GtkButton* _tmp11_;
GtkVBox* _tmp12_;
GtkLabel* _tmp13_;
GtkHBox* _tmp14_;
GtkButton* _tmp15_;
GtkHBox* _tmp16_;
GtkButton* _tmp17_;
GtkHBox* _tmp18_;
GtkButton* _tmp19_;
GtkVBox* _tmp20_;
GtkHBox* _tmp21_;
GtkVBox* _tmp22_;
parent_class = G_OBJECT_CLASS (main_parent_class);
obj = parent_class->constructor (type, n_construct_properties, construct_properties);
self = MAIN (obj);
_tmp0_ = (GtkVBox*) gtk_vbox_new (FALSE, 0);
_tmp1_ = g_object_ref_sink (_tmp0_);
_g_object_unref0 (_vbox0);
_vbox0 = _tmp1_;
_tmp2_ = (GtkLabel*) gtk_label_new ("hello world");
_tmp3_ = g_object_ref_sink (_tmp2_);
_g_object_unref0 (_label0);
_label0 = _tmp3_;
_tmp4_ = (GtkHBox*) gtk_hbox_new (FALSE, 0);
_tmp5_ = g_object_ref_sink (_tmp4_);
_g_object_unref0 (_hbox0);
_hbox0 = _tmp5_;
_tmp6_ = (GtkButton*) gtk_button_new_with_label ("Install");
_tmp7_ = g_object_ref_sink (_tmp6_);
_g_object_unref0 (_button0);
_button0 = _tmp7_;
_tmp8_ = (GtkButton*) gtk_button_new_with_label ("Update");
_tmp9_ = g_object_ref_sink (_tmp8_);
_g_object_unref0 (_button1);
_button1 = _tmp9_;
_tmp10_ = (GtkButton*) gtk_button_new_with_label ("Deinstall");
_tmp11_ = g_object_ref_sink (_tmp10_);
_g_object_unref0 (_button2);
_button2 = _tmp11_;
g_object_set ((GtkWindow*) self, "type", GTK_WINDOW_TOPLEVEL, NULL);
_tmp12_ = _vbox0;
_tmp13_ = _label0;
gtk_container_add ((GtkContainer*) _tmp12_, (GtkWidget*) _tmp13_);
_tmp14_ = _hbox0;
_tmp15_ = _button0;
gtk_container_add ((GtkContainer*) _tmp14_, (GtkWidget*) _tmp15_);
_tmp16_ = _hbox0;
_tmp17_ = _button1;
gtk_container_add ((GtkContainer*) _tmp16_, (GtkWidget*) _tmp17_);
_tmp18_ = _hbox0;
_tmp19_ = _button2;
gtk_container_add ((GtkContainer*) _tmp18_, (GtkWidget*) _tmp19_);
_tmp20_ = _vbox0;
_tmp21_ = _hbox0;
gtk_container_add ((GtkContainer*) _tmp20_, (GtkWidget*) _tmp21_);
_tmp22_ = _vbox0;
gtk_container_add ((GtkContainer*) self, (GtkWidget*) _tmp22_);
_g_object_unref0 (_button2);
_g_object_unref0 (_button1);
_g_object_unref0 (_button0);
_g_object_unref0 (_hbox0);
_g_object_unref0 (_label0);
_g_object_unref0 (_vbox0);
return obj;
}


static void main_class_init (MainClass * klass) {
main_parent_class = g_type_class_peek_parent (klass);
G_OBJECT_CLASS (klass)->constructor = main_constructor;
}


static void main_instance_init (Main * self) {
}


GType main_get_type (void) {
static volatile gsize main_type_id__volatile = 0;
if (g_once_init_enter (&main_type_id__volatile)) {
static const GTypeInfo g_define_type_info = { sizeof (MainClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) main_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (Main), 0, (GInstanceInitFunc) main_instance_init, NULL };
GType main_type_id;
main_type_id = g_type_register_static (GTK_TYPE_WINDOW, "Main", &g_define_type_info, 0);
g_once_init_leave (&main_type_id__volatile, main_type_id);
}
return main_type_id__volatile;
}



