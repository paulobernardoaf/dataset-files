#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <allegro5/allegro5.h>
#if !defined(SCAN_DEPEND)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif
static void usage(char *argv0)
{
printf("Usage: %s [OPTIONS]\n\n", argv0);
printf(" -f FONTNAME Font name pattern ('*' by default).\n"
" -o FILENAME Output file name ('font.pcx' by default).\n"
" -r NUM-NUM Character range, default = 0x20-0xff.\n"
" -c COLOR Character color (default 000000000 -- black).\n"
" -b COLOR Background color (default 255255255 -- white).\n"
" -g COLOR Grid color (default 255000255 -- magenta).\n"
" Colors are specified as RRRGGGBBB values,\n"
" with each RRR, GGG and BBB in the range [0, 255].\n"
" -h This help.\n"
"\n"
"Find the font you want to convert using xlsfonts or xfontsel.\n\n"
"Example 1:\n"
"bash> xlsfonts -fn '*' | less\n"
"bash> %s -f '-adobe-courier-medium-r-normal--34-*'\n"
"bash> grabber\n\n"
"In grabber, create a new font 'Object|New|Font'. Use 'Object|Grab'\n"
"on this object and choose the pcx file with the font (font.pcx).\n\n"
"Example 2:\n"
"./xf2pcx -f '-*-clearlyu-*-*-*-*-17-*-*-*-p--iso10646-1' -r 0x2800-0x28ff -o braille.pcx\n\n"
"Writes the Braille alphabet into braille.pcx.\n", argv0);
}
int main(int argc, char *argv[])
{
char *fontname = "*";
char *filename = "font.pcx";
int start_char = 32, end_char = 255;
int ccolor = 000000000;
int bcolor = 255255255;
int gcolor = 255000255;
Display *display;
int screen_number;
int default_depth;
Window window;
Font font;
GC gc, gc2;
Pixmap pixmap;
XImage *image;
XCharStruct overall;
int bitmap_width, bitmap_height;
int max_ascent, max_descent;
int max_width, max_height;
int i, opt, x, y, cx, cy, sx, sy, lines;
unsigned long black, white;
BITMAP *bitmap;
RGB palette[256];
if (argc == 1) {
usage(argv[0]);
exit(EXIT_SUCCESS);
}
install_allegro(SYSTEM_NONE, &errno, atexit);
opterr = 0;
while ((opt = getopt(argc, argv, "f:o:z:c:b:g:r:h")) != EOF) {
switch (opt) {
case 'f':
fontname = optarg;
break;
case 'o':
filename = optarg;
break;
case 'c':
ccolor = atol(optarg);
break;
case 'b':
bcolor = atol(optarg);
break;
case 'g':
gcolor = atol(optarg);
break;
case 'r':
{
char *str;
start_char = strtol(optarg, &str, 0);
end_char = strtol(str + 1, NULL, 0);
break;
}
case 'h':
usage(argv[0]);
exit(EXIT_SUCCESS);
default:
fprintf(stderr, "%s: unrecognized option -- '%c'\n", argv[0],
optopt);
fprintf(stderr, "%s: try '%s -h' for more information\n",
argv[0], argv[0]);
exit(EXIT_FAILURE);
}
}
display = XOpenDisplay(0);
if (display == 0) {
fprintf(stderr, "%s: XOpenDisplay failed\n", argv[0]);
exit(EXIT_FAILURE);
}
screen_number = XDefaultScreen(display);
default_depth = XDefaultDepth(display, screen_number);
window = XDefaultRootWindow(display);
font = XLoadFont(display, fontname);
{
unsigned long val_mask;
XGCValues val_bits;
val_mask = GCForeground | GCBackground | GCFont | GCFunction;
val_bits.function = GXcopy;
val_bits.foreground = white = WhitePixel(display, screen_number);
val_bits.background = black = BlackPixel(display, screen_number);
val_bits.font = font;
gc = XCreateGC(display, window, val_mask, &val_bits);
val_mask = GCForeground;
val_bits.foreground = black;
gc2 = XCreateGC(display, window, val_mask, &val_bits);
}
{
XFontStruct *xfs;
int min, max;
xfs = XQueryFont(display, font);
max_ascent = xfs->ascent;
max_descent = xfs->descent;
if (xfs->min_byte1 == 0 && xfs->max_byte1 == 0) {
min = xfs->min_char_or_byte2;
max = xfs->max_char_or_byte2;
}
else {
min = (xfs->min_byte1 << 8) + xfs->min_char_or_byte2;
max = (xfs->max_byte1 << 8) + xfs->max_char_or_byte2;
}
if (start_char < min || end_char > max)
fprintf(stderr,
"You specified characters %04x-%04x, but this font "
"only has the range %04x-%04x\n", start_char, end_char,
min, max);
XFreeFontInfo(NULL, xfs, 0);
}
max_width = 0;
lines = 1 + (end_char - start_char) / 16;
for (cy = 0; cy < lines; cy++) {
for (cx = 0; cx < 16 && start_char + cy * 16 + cx <= end_char; cx++) {
int dir, ascent, descent;
int width;
XChar2b string[2] = { {0, 0}, {0, 0} };
string[0].byte1 = (start_char + cy * 16 + cx) >> 8;
string[0].byte2 = (start_char + cy * 16 + cx) & 255;
XQueryTextExtents16(display, font, string, 1, &dir, &ascent,
&descent, &overall);
width = overall.width;
if (width < 1)
width = 1;
if (width > max_width)
max_width = width;
if (max_ascent < overall.ascent)
max_ascent = overall.ascent;
if (max_descent < overall.descent)
max_descent = overall.descent;
}
}
max_height = max_ascent + max_descent;
bitmap_width = (max_width + 1) * 16 + 1;
bitmap_height = (max_height + 1) * lines + 1;
bitmap = create_bitmap(bitmap_width, bitmap_height);
if (bitmap == 0) {
fprintf(stderr, "%s: can not create bitmap\n", argv[0]);
exit(EXIT_FAILURE);
}
clear_to_color(bitmap, 255);
sy = 1;
for (cy = 0; cy < lines; cy++) {
sx = 1;
for (cx = 0; cx < 16 && start_char + cy * 16 + cx <= end_char; cx++) {
int dir, ascent, descent;
XChar2b string[2] = { {0, 0}, {0, 0} };
string[0].byte1 = (start_char + cy * 16 + cx) >> 8;
string[0].byte2 = (start_char + cy * 16 + cx) & 255;
XQueryTextExtents16(display, font, string, 1, &dir, &ascent,
&descent, &overall);
if (overall.width < 1)
overall.width = 1;
pixmap =
XCreatePixmap(display, window, overall.width, max_height,
default_depth);
XFillRectangle(display, pixmap, gc2, 0, 0, overall.width,
max_height);
XDrawImageString16(display, pixmap, gc, 0, max_ascent, string, 1);
image =
XGetImage(display, pixmap, 0, 0, overall.width, max_height,
AllPlanes, ZPixmap);
if (image == 0) {
fprintf(stderr, "%s: can not get image\n", argv[0]);
exit(EXIT_FAILURE);
}
for (y = 0; y < max_height; y++)
for (x = 0; x < overall.width; x++) {
if (XGetPixel(image, x, y) == white)
putpixel(bitmap, sx + x, sy + y, 1);
else
putpixel(bitmap, sx + x, sy + y, 0);
}
XDestroyImage(image);
XFreePixmap(display, pixmap);
sx += max_width + 1;
}
sy += max_height + 1;
}
for (i = 0; i < 256; i++)
palette[i].r = palette[i].g = palette[i].b = 0;
#define CLAMP(v) (((v / 4) > 63) ? 63 : (v / 4))
palette[0].r = CLAMP(bcolor / 1000000);
palette[0].g = CLAMP((bcolor % 1000000) / 1000);
palette[0].b = CLAMP(bcolor % 1000);
palette[1].r = CLAMP(ccolor / 1000000);
palette[1].g = CLAMP((ccolor % 1000000) / 1000);
palette[1].b = CLAMP(ccolor % 1000);
palette[255].r = CLAMP(gcolor / 1000000);
palette[255].g = CLAMP((gcolor % 1000000) / 1000);
palette[255].b = CLAMP(gcolor % 1000);
#undef CLAMP
save_pcx(filename, bitmap, palette);
destroy_bitmap(bitmap);
XFreeGC(display, gc);
XFreeGC(display, gc2);
XUnloadFont(display, font);
XCloseDisplay(display);
exit(EXIT_SUCCESS);
}
