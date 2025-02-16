#include <stdio.h>
#include "allegro5/allegro.h"

#include "common.c"

static void show_path(int id, const char *label)
{
ALLEGRO_PATH *path;
const char *path_str;

path = al_get_standard_path(id);
path_str = (path) ? al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP) : "<none>";
log_printf("%s: %s\n", label, path_str);
al_destroy_path(path);
}

int main(int argc, char **argv)
{
int pass;

(void)argc;
(void)argv;


al_set_org_name("liballeg.org");


al_set_app_name("ex_get_path");

if (!al_init()) {
abort_example("Could not init Allegro.\n");
}
open_log();

for (pass = 1; pass <= 3; pass++) {
if (pass == 1) {
log_printf("With default exe name:\n");
}
else if (pass == 2) {
log_printf("\nOverriding exe name to blahblah\n");
al_set_exe_name("blahblah");
}
else if (pass == 3) {
log_printf("\nOverriding exe name to /tmp/blahblah.exe:\n");
al_set_exe_name("/tmp/blahblah.exe");
}

show_path(ALLEGRO_RESOURCES_PATH, "RESOURCES_PATH");
show_path(ALLEGRO_TEMP_PATH, "TEMP_PATH");
show_path(ALLEGRO_USER_DATA_PATH, "USER_DATA_PATH");
show_path(ALLEGRO_USER_SETTINGS_PATH, "USER_SETTINGS_PATH");
show_path(ALLEGRO_USER_HOME_PATH, "USER_HOME_PATH");
show_path(ALLEGRO_USER_DOCUMENTS_PATH, "USER_DOCUMENTS_PATH");
show_path(ALLEGRO_EXENAME_PATH, "EXENAME_PATH");
}

close_log(true);
return 0;
}



