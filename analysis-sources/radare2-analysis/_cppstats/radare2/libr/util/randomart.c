#include <r_util.h>
#define FLDBASE 8
#define FLDSIZE_Y (FLDBASE + 1)
#define FLDSIZE_X (FLDBASE * 2 + 1)
R_API char *r_print_randomart(const ut8 *dgst_raw, ut32 dgst_raw_len, ut64 addr) {
char *augmentation_string = " .o+=*BOX@%&#/^SE";
char *retval, *p;
ut8 field[FLDSIZE_X][FLDSIZE_Y];
ut32 i, b;
int x, y;
size_t len = strlen(augmentation_string) - 1;
retval = calloc (1, 2*(FLDSIZE_X+3) + (FLDSIZE_Y * (FLDSIZE_X+3))+ 7 + sizeof (PFMT64x));
memset(field, 0, FLDSIZE_X * FLDSIZE_Y * sizeof(char));
x = FLDSIZE_X / 2;
y = FLDSIZE_Y / 2;
for (i = 0; i < dgst_raw_len; i++) {
int input;
input = dgst_raw[i];
for (b = 0; b < 4; b++) {
x += (input & 0x1) ? 1 : -1;
y += (input & 0x2) ? 1 : -1;
x = R_MAX(x, 0);
y = R_MAX(y, 0);
x = R_MIN(x, FLDSIZE_X - 1);
y = R_MIN(y, FLDSIZE_Y - 1);
if (field[x][y] < len - 2) {
field[x][y]++;
}
input = input >> 2;
}
}
field[FLDSIZE_X / 2][FLDSIZE_Y / 2] = len - 1;
field[x][y] = len;
#if 0
snprintf(retval, FLDSIZE_X, "+--[%4s %4u]", key_type(k), key_size(k));
#else
sprintf (retval, "+--[0x%08"PFMT64x"]-", addr);
#endif
p = strchr(retval, '\0');
for (i = p - retval - 1; i < FLDSIZE_X; i++) {
*p++ = '-';
}
*p++ = '+';
*p++ = '\n';
for (y = 0; y < FLDSIZE_Y; y++) {
*p++ = '|';
for (x = 0; x < FLDSIZE_X; x++) {
*p++ = augmentation_string[R_MIN (field[x][y], len)];
}
*p++ = '|';
*p++ = '\n';
}
*p++ = '+';
for (i = 0; i < FLDSIZE_X; i++) {
*p++ = '-';
}
*p++ = '+';
*p++ = 0;
return retval;
}
