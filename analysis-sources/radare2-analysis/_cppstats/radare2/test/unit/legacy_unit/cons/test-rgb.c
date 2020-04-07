#include <r_cons.h>
int main() {
int i,j ,k;
char *str ="\x1b\\pop\x1b[0m";
i=j=k =0;
r_cons_new ();
printf ("3 == %d\n", r_str_ansi_len (str));
for (i=0;i<255;i+=40) {
for (j=0;j<255;j+=40) { for (k=0;k<255;k+=40) {
r_cons_rgb (i, j, k, 0);
r_cons_rgb (i, j, k, 1);
r_cons_print ("__");
r_cons_reset_colors ();
r_cons_rgb (i, j, k, 0);
r_cons_printf (" RGB %d %d %d", i, j, k);
r_cons_reset_colors ();
r_cons_newline ();
}
}}
r_cons_flush ();
return 0;
}
