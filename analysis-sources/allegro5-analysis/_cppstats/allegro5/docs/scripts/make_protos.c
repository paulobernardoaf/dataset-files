#include "dawk.h"
int main(int argc, char *argv[])
{
bool found_tag = false;
bool do_print = false;
dstr line = "";
dstr prefix = "";
d_init(argc, argv);
while (d_getline(line)) {
if (d_match(line, "/[*] (Function|Type|Enum): (.*)")) {
found_tag = true;
do_print = false;
d_assign(prefix, d_submatch(2));
continue;
}
if (found_tag && d_match(line, "^ ?\\*/")) {
found_tag = false;
do_print = true;
continue;
}
if (d_match(line, "^\\{|^$")) {
do_print = false;
continue;
}
if (line[0] == 0) {
do_print = false;
continue;
}
if (do_print) {
printf("%s: %s:%s:%d\n", prefix, line, d_filename,
d_line_num);
}
if (d_match(line, "\\{ *$|; *$")) {
do_print = false;
}
}
return 0;
}
