#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/internal/aintern_image.h"

#include "iio.h"

ALLEGRO_DEBUG_CHANNEL("image")


#define INT_TO_BOOL(x) ((x) != 0)

ALLEGRO_BITMAP *_al_load_pcx_f(ALLEGRO_FILE *f, int flags)
{
ALLEGRO_BITMAP *b;
int c;
int width, height;
int bpp, bytes_per_line;
int x, xx, y;
char ch;
ALLEGRO_LOCKED_REGION *lr;
unsigned char *buf;
PalEntry pal[256];
bool keep_index;
ASSERT(f);

al_fgetc(f); 
al_fgetc(f); 
al_fgetc(f); 

char color_plane = al_fgetc(f);
if (color_plane != 8) { 
ALLEGRO_ERROR("Invalid color plane %d.\n", color_plane);
return NULL;
}

width = -(al_fread16le(f)); 
height = -(al_fread16le(f)); 
width += al_fread16le(f) + 1; 
height += al_fread16le(f) + 1; 

al_fread32le(f); 

for (c = 0; c < 16 * 3; c++) { 
al_fgetc(f);
}

al_fgetc(f);

bpp = al_fgetc(f) * 8; 

if ((bpp != 8) && (bpp != 24)) {
ALLEGRO_ERROR("Invalid bpp %d.\n", color_plane);
return NULL;
}

bytes_per_line = al_fread16le(f);

for (c = 0; c < 60; c++) 
al_fgetc(f);

if (al_feof(f) || al_ferror(f)) {
ALLEGRO_ERROR("Unexpected EOF/error.\n");
return NULL;
}

b = al_create_bitmap(width, height);
if (!b) {
ALLEGRO_ERROR("Failed to create bitmap.\n");
return NULL;
}

keep_index = INT_TO_BOOL(flags & ALLEGRO_KEEP_INDEX);

al_set_errno(0);

if (bpp == 8) {



buf = (unsigned char *)al_malloc(bytes_per_line * height);
}
else {

buf = (unsigned char *)al_malloc(bytes_per_line * 3);
}

if (bpp == 8 && keep_index) {
lr = al_lock_bitmap(b, ALLEGRO_PIXEL_FORMAT_SINGLE_CHANNEL_8, ALLEGRO_LOCK_WRITEONLY);
}
else {
lr = al_lock_bitmap(b, ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_WRITEONLY);
}
if (!lr) {
ALLEGRO_ERROR("Failed to lock bitmap.\n");
al_free(buf);
return NULL;
}

xx = 0; 

for (y = 0; y < height; y++) { 

x = 0;

while (x < bytes_per_line * bpp / 8) {
ch = al_fgetc(f);
if ((ch & 0xC0) == 0xC0) { 
c = (ch & 0x3F);
ch = al_fgetc(f);
}
else {
c = 1; 
}

if (bpp == 8) {
while (c--) {
if (x < width) 
buf[xx++] = ch;
x++;
}
}
else {
while (c--) {
if (x < width * 3) 
buf[x] = ch;
x++;
}
}
}
if (bpp == 24) {
char *dest = (char*)lr->data + y*lr->pitch;
for (x = 0; x < width; x++) {
dest[x*4 ] = buf[x];
dest[x*4 + 1] = buf[x + width];
dest[x*4 + 2] = buf[x + width * 2];
dest[x*4 + 3] = 255;
}
}
}

if (bpp == 8) { 
while ((c = al_fgetc(f)) != EOF) {
if (c == 12) {
for (c = 0; c < 256; c++) {
pal[c].r = al_fgetc(f);
pal[c].g = al_fgetc(f);
pal[c].b = al_fgetc(f);
}
break;
}
}
for (y = 0; y < height; y++) {
char *dest = (char*)lr->data + y*lr->pitch;
for (x = 0; x < width; x++) {
int index = buf[y * width + x];
if (keep_index) {
dest[x] = index;
}
else {
dest[x*4 ] = pal[index].r;
dest[x*4 + 1] = pal[index].g;
dest[x*4 + 2] = pal[index].b;
dest[x*4 + 3] = 255;
}
}
}
}

al_unlock_bitmap(b);

al_free(buf);

if (al_get_errno()) {
ALLEGRO_ERROR("Error detected: %d.\n", al_get_errno());
al_destroy_bitmap(b);
return NULL;
}

return b;
}

bool _al_save_pcx_f(ALLEGRO_FILE *f, ALLEGRO_BITMAP *bmp)
{
int c;
int x, y;
int i;
int w, h;
unsigned char *buf;
ASSERT(f);
ASSERT(bmp);

al_set_errno(0);

w = al_get_bitmap_width(bmp);
h = al_get_bitmap_height(bmp);

al_fputc(f, 10); 
al_fputc(f, 5); 
al_fputc(f, 1); 
al_fputc(f, 8); 
al_fwrite16le(f, 0); 
al_fwrite16le(f, 0); 
al_fwrite16le(f, w - 1); 
al_fwrite16le(f, h - 1); 
al_fwrite16le(f, 320); 
al_fwrite16le(f, 200); 

for (c = 0; c < 16 * 3; c++) {
al_fputc(f, 0);
}

al_fputc(f, 0); 
al_fputc(f, 3); 
al_fwrite16le(f, w); 
al_fwrite16le(f, 1); 
al_fwrite16le(f, w); 
al_fwrite16le(f, h); 
for (c = 0; c < 54; c++) 
al_fputc(f, 0);

buf = al_malloc(w * 3);

al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);

for (y = 0; y < h; y++) { 
for (x = 0; x < w; x++) {
ALLEGRO_COLOR c = al_get_pixel(bmp, x, y);
unsigned char r, g, b;
al_unmap_rgb(c, &r, &g, &b);
buf[x] = r;
buf[x + w] = g;
buf[x + w * 2] = b;
}

for (i = 0; i < 3; i++) {
int color;
int count;
x = 0;
for (;;) {
count = 0;
color = buf[x + w * i];
do {
count++;
x++;
} while ((count < 63) && (x < w) && (color == buf[x + w * i]));
al_fputc(f, count | 0xC0);
al_fputc(f, color);
if (x >= w)
break;
}
}
}

al_free(buf);

al_unlock_bitmap(bmp);

if (al_get_errno()) {
ALLEGRO_ERROR("Error detected: %d.\n", al_get_errno());
return false;
}
else
return true;
}

ALLEGRO_BITMAP *_al_load_pcx(const char *filename, int flags)
{
ALLEGRO_FILE *f;
ALLEGRO_BITMAP *bmp;
ASSERT(filename);

f = al_fopen(filename, "rb");
if (!f) {
ALLEGRO_ERROR("Unable to open %s for reading.\n", filename);
return NULL;
}

bmp = _al_load_pcx_f(f, flags);

al_fclose(f);

return bmp;
}

bool _al_save_pcx(const char *filename, ALLEGRO_BITMAP *bmp)
{
ALLEGRO_FILE *f;
bool retsave;
bool retclose;
ASSERT(filename);

f = al_fopen(filename, "wb");
if (!f) {
ALLEGRO_ERROR("Unable to open %s for writing.\n", filename);
return false;
}

retsave = _al_save_pcx_f(f, bmp);

retclose = al_fclose(f);

return retsave && retclose;
}

bool _al_identify_pcx(ALLEGRO_FILE *f)
{
uint8_t x[4];
al_fread(f, x, 4);

if (x[0] != 0x0a) 
return false;
if (x[1] == 1 || x[1] > 5) 
return false;
if (x[2] > 1) 
return false;
if (x[3] != 8) 
return false;
return true;
}



