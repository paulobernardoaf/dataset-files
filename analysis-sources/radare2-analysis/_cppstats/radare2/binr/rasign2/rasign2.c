#include <stdio.h>
#include <string.h>
#include <r_getopt.h>
#include "r_userconf.h"
#include "r_sign.h"
static int rasign_show_help() {
printf ("Usage: rasign2 [options] [file]\n"
" -r show output in radare commands\n"
" -j show output in json\n"
" -s [sigfile] specify one or more signature files\n"
"Examples:\n"
" rasign2 libc.so.6 > libc.sig\n"
" rasign2 -s libc.sig ls.static\n");
return 0;
}
int main(int argc, char **argv) {
int c;
int action = 0;
int rad = 0;
int json = 0;
while ((c=getopt (argc, argv, "o:hrsj:iV")) !=-1) {
switch (c) {
case 'o':
break;
case 's':
action = c;
break;
case 'r':
rad = 1;
break;
case 'j':
json = 1;
break;
case 'V':
return blob_version ("rasign2");
default:
return rasign_show_help ();
}
}
if (argv[optind]==NULL)
return rasign_show_help ();
switch (action) {
case 's':
break;
default:
break;
}
return 0;
}
