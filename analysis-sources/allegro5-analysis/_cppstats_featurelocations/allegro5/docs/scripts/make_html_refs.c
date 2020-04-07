








#include <string.h>
#include "dawk.h"




static void replace_spaces(char *s)
{
while (*s) {
if (*s == ' ') *s = '-';
s++;
}
}

int main(int argc, char *argv[])
{
dstr line;
dstr file;
const char *name;
dstr sec_id;

d_init(argc, argv);

while (d_getline(line)) {
if (d_match(line, "^#+( API:| ) *")) {
d_basename(d_filename, ".html", file);
name = d_after_match;
d_tolower(name, sec_id);
replace_spaces(sec_id);
d_printf("[%s]: %s#%s\n", name, file, sec_id);
}
}

return 0;
}


