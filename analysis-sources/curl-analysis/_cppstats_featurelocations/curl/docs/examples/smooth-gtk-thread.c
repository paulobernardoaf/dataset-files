


































#include <stdio.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <unistd.h>
#include <pthread.h>

#include <curl/curl.h>

#define NUMT 4

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int j = 0;
gint num_urls = 9; 
const char * const urls[]= {
"90022",
"90023",
"90024",
"90025",
"90026",
"90027",
"90028",
"90029",
"90030"
};

size_t write_file(void *ptr, size_t size, size_t nmemb, FILE *stream)
{

return fwrite(ptr, size, nmemb, stream);
}


void *pull_one_url(void *NaN)
{

pthread_mutex_lock(&lock);
while(j < num_urls) {
CURL *curl;
gchar *http;

printf("j = %d\n", j);

http =
g_strdup_printf("xoap.weather.com/weather/local/%s?cc=*&dayf=5&unit=i\n",
urls[j]);

printf("http %s", http);

curl = curl_easy_init();
if(curl) {

FILE *outfile = fopen(urls[j], "wb");


curl_easy_setopt(curl, CURLOPT_URL, http);


curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);

j++; 
pthread_mutex_unlock(&lock);

curl_easy_perform(curl);

fclose(outfile);
printf("fclose\n");

curl_easy_cleanup(curl);
}
g_free(http);


sleep(1);

} 
return NULL;
}


gboolean pulse_bar(gpointer data)
{
gdk_threads_enter();
gtk_progress_bar_pulse(GTK_PROGRESS_BAR (data));
gdk_threads_leave();




return TRUE;
}

void *create_thread(void *progress_bar)
{
pthread_t tid[NUMT];
int i;


for(i = 0; i < NUMT && i < num_urls ; i++) {
int error = pthread_create(&tid[i],
NULL, 
pull_one_url,
NULL);
if(0 != error)
fprintf(stderr, "Couldn't run thread number %d, errno %d\n", i, error);
else
fprintf(stderr, "Thread %d, gets %s\n", i, urls[i]);
}


for(i = 0; i < NUMT && i < num_urls; i++) {
pthread_join(tid[i], NULL);
fprintf(stderr, "Thread %d terminated\n", i);
}



g_source_remove(GPOINTER_TO_INT(g_object_get_data(G_OBJECT(progress_bar),
"pulse_id")));


gtk_widget_destroy(progress_bar);





return NULL;

}

static gboolean cb_delete(GtkWidget *window, gpointer data)
{
gtk_main_quit();
return FALSE;
}

int main(int argc, char **argv)
{
GtkWidget *top_window, *outside_frame, *inside_frame, *progress_bar;


curl_global_init(CURL_GLOBAL_ALL);


g_thread_init(NULL);
gdk_threads_init();
gdk_threads_enter();

gtk_init(&argc, &argv);


top_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);


outside_frame = gtk_frame_new(NULL);
gtk_frame_set_shadow_type(GTK_FRAME(outside_frame), GTK_SHADOW_OUT);
gtk_container_add(GTK_CONTAINER(top_window), outside_frame);


inside_frame = gtk_frame_new(NULL);
gtk_frame_set_shadow_type(GTK_FRAME(inside_frame), GTK_SHADOW_IN);
gtk_container_set_border_width(GTK_CONTAINER(inside_frame), 5);
gtk_container_add(GTK_CONTAINER(outside_frame), inside_frame);


progress_bar = gtk_progress_bar_new();
gtk_progress_bar_pulse(GTK_PROGRESS_BAR (progress_bar));

gint pulse_ref = g_timeout_add(300, pulse_bar, progress_bar);
g_object_set_data(G_OBJECT(progress_bar), "pulse_id",
GINT_TO_POINTER(pulse_ref));
gtk_container_add(GTK_CONTAINER(inside_frame), progress_bar);

gtk_widget_show_all(top_window);
printf("gtk_widget_show_all\n");

g_signal_connect(G_OBJECT (top_window), "delete-event",
G_CALLBACK(cb_delete), NULL);

if(!g_thread_create(&create_thread, progress_bar, FALSE, NULL) != 0)
g_warning("can't create the thread");

gtk_main();
gdk_threads_leave();
printf("gdk_threads_leave\n");

return 0;
}
