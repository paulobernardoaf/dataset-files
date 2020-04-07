















































#include "raylib.h" 


#if !defined(EXTERNAL_CONFIG_FLAGS)
#include "config.h" 
#endif

#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <stdarg.h> 
#include <ctype.h> 

#include "utils.h" 

#if defined(SUPPORT_FILEFORMAT_TTF)
#define STB_RECT_PACK_IMPLEMENTATION
#include "external/stb_rect_pack.h" 

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "external/stb_truetype.h" 
#endif




#define MAX_TEXT_BUFFER_LENGTH 1024 


#define MAX_TEXT_UNICODE_CHARS 512 

#if !defined(TEXTSPLIT_MAX_TEXT_BUFFER_LENGTH)
#define TEXTSPLIT_MAX_TEXT_BUFFER_LENGTH 1024 
#endif

#if !defined(TEXTSPLIT_MAX_SUBSTRINGS_COUNT)
#define TEXTSPLIT_MAX_SUBSTRINGS_COUNT 128 
#endif









#if defined(SUPPORT_DEFAULT_FONT)
static Font defaultFont = { 0 }; 

#endif









#if defined(SUPPORT_FILEFORMAT_FNT)
static Font LoadBMFont(const char *fileName); 
#endif

#if defined(SUPPORT_DEFAULT_FONT)
extern void LoadFontDefault(void);
extern void UnloadFontDefault(void);
#endif




#if defined(SUPPORT_DEFAULT_FONT)


extern void LoadFontDefault(void)
{
#define BIT_CHECK(a,b) ((a) & (1u << (b)))




defaultFont.charsCount = 224; 




unsigned int defaultFontData[512] = {
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200020, 0x0001b000, 0x00000000, 0x00000000, 0x8ef92520, 0x00020a00, 0x7dbe8000, 0x1f7df45f,
0x4a2bf2a0, 0x0852091e, 0x41224000, 0x10041450, 0x2e292020, 0x08220812, 0x41222000, 0x10041450, 0x10f92020, 0x3efa084c, 0x7d22103c, 0x107df7de,
0xe8a12020, 0x08220832, 0x05220800, 0x10450410, 0xa4a3f000, 0x08520832, 0x05220400, 0x10450410, 0xe2f92020, 0x0002085e, 0x7d3e0281, 0x107df41f,
0x00200000, 0x8001b000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc0000fbe, 0xfbf7e00f, 0x5fbf7e7d, 0x0050bee8, 0x440808a2, 0x0a142fe8, 0x50810285, 0x0050a048,
0x49e428a2, 0x0a142828, 0x40810284, 0x0048a048, 0x10020fbe, 0x09f7ebaf, 0xd89f3e84, 0x0047a04f, 0x09e48822, 0x0a142aa1, 0x50810284, 0x0048a048,
0x04082822, 0x0a142fa0, 0x50810285, 0x0050a248, 0x00008fbe, 0xfbf42021, 0x5f817e7d, 0x07d09ce8, 0x00008000, 0x00000fe0, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000c0180,
0xdfbf4282, 0x0bfbf7ef, 0x42850505, 0x004804bf, 0x50a142c6, 0x08401428, 0x42852505, 0x00a808a0, 0x50a146aa, 0x08401428, 0x42852505, 0x00081090,
0x5fa14a92, 0x0843f7e8, 0x7e792505, 0x00082088, 0x40a15282, 0x08420128, 0x40852489, 0x00084084, 0x40a16282, 0x0842022a, 0x40852451, 0x00088082,
0xc0bf4282, 0xf843f42f, 0x7e85fc21, 0x3e0900bf, 0x00000000, 0x00000004, 0x00000000, 0x000c0180, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000402, 0x41482000, 0x00000000, 0x00000800,
0x04000404, 0x4100203c, 0x00000000, 0x00000800, 0xf7df7df0, 0x514bef85, 0xbefbefbe, 0x04513bef, 0x14414500, 0x494a2885, 0xa28a28aa, 0x04510820,
0xf44145f0, 0x474a289d, 0xa28a28aa, 0x04510be0, 0x14414510, 0x494a2884, 0xa28a28aa, 0x02910a00, 0xf7df7df0, 0xd14a2f85, 0xbefbe8aa, 0x011f7be0,
0x00000000, 0x00400804, 0x20080000, 0x00000000, 0x00000000, 0x00600f84, 0x20080000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0xac000000, 0x00000f01, 0x00000000, 0x00000000, 0x24000000, 0x00000f01, 0x00000000, 0x06000000, 0x24000000, 0x00000f01, 0x00000000, 0x09108000,
0x24fa28a2, 0x00000f01, 0x00000000, 0x013e0000, 0x2242252a, 0x00000f52, 0x00000000, 0x038a8000, 0x2422222a, 0x00000f29, 0x00000000, 0x010a8000,
0x2412252a, 0x00000f01, 0x00000000, 0x010a8000, 0x24fbe8be, 0x00000f01, 0x00000000, 0x0ebe8000, 0xac020000, 0x00000f01, 0x00000000, 0x00048000,
0x0003e000, 0x00000f00, 0x00000000, 0x00008000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000038, 0x8443b80e, 0x00203a03,
0x02bea080, 0xf0000020, 0xc452208a, 0x04202b02, 0xf8029122, 0x07f0003b, 0xe44b388e, 0x02203a02, 0x081e8a1c, 0x0411e92a, 0xf4420be0, 0x01248202,
0xe8140414, 0x05d104ba, 0xe7c3b880, 0x00893a0a, 0x283c0e1c, 0x04500902, 0xc4400080, 0x00448002, 0xe8208422, 0x04500002, 0x80400000, 0x05200002,
0x083e8e00, 0x04100002, 0x804003e0, 0x07000042, 0xf8008400, 0x07f00003, 0x80400000, 0x04000022, 0x00000000, 0x00000000, 0x80400000, 0x04000002,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00800702, 0x1848a0c2, 0x84010000, 0x02920921, 0x01042642, 0x00005121, 0x42023f7f, 0x00291002,
0xefc01422, 0x7efdfbf7, 0xefdfa109, 0x03bbbbf7, 0x28440f12, 0x42850a14, 0x20408109, 0x01111010, 0x28440408, 0x42850a14, 0x2040817f, 0x01111010,
0xefc78204, 0x7efdfbf7, 0xe7cf8109, 0x011111f3, 0x2850a932, 0x42850a14, 0x2040a109, 0x01111010, 0x2850b840, 0x42850a14, 0xefdfbf79, 0x03bbbbf7,
0x001fa020, 0x00000000, 0x00001000, 0x00000000, 0x00002070, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x08022800, 0x00012283, 0x02430802, 0x01010001, 0x8404147c, 0x20000144, 0x80048404, 0x00823f08, 0xdfbf4284, 0x7e03f7ef, 0x142850a1, 0x0000210a,
0x50a14684, 0x528a1428, 0x142850a1, 0x03efa17a, 0x50a14a9e, 0x52521428, 0x142850a1, 0x02081f4a, 0x50a15284, 0x4a221428, 0xf42850a1, 0x03efa14b,
0x50a16284, 0x4a521428, 0x042850a1, 0x0228a17a, 0xdfbf427c, 0x7e8bf7ef, 0xf7efdfbf, 0x03efbd0b, 0x00000000, 0x04000000, 0x00000000, 0x00000008,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200508, 0x00840400, 0x11458122, 0x00014210,
0x00514294, 0x51420800, 0x20a22a94, 0x0050a508, 0x00200000, 0x00000000, 0x00050000, 0x08000000, 0xfefbefbe, 0xfbefbefb, 0xfbeb9114, 0x00fbefbe,
0x20820820, 0x8a28a20a, 0x8a289114, 0x3e8a28a2, 0xfefbefbe, 0xfbefbe0b, 0x8a289114, 0x008a28a2, 0x228a28a2, 0x08208208, 0x8a289114, 0x088a28a2,
0xfefbefbe, 0xfbefbefb, 0xfa2f9114, 0x00fbefbe, 0x00000000, 0x00000040, 0x00000000, 0x00000000, 0x00000000, 0x00000020, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00210100, 0x00000004, 0x00000000, 0x00000000, 0x14508200, 0x00001402, 0x00000000, 0x00000000,
0x00000010, 0x00000020, 0x00000000, 0x00000000, 0xa28a28be, 0x00002228, 0x00000000, 0x00000000, 0xa28a28aa, 0x000022e8, 0x00000000, 0x00000000,
0xa28a28aa, 0x000022a8, 0x00000000, 0x00000000, 0xa28a28aa, 0x000022e8, 0x00000000, 0x00000000, 0xbefbefbe, 0x00003e2f, 0x00000000, 0x00000000,
0x00000004, 0x00002028, 0x00000000, 0x00000000, 0x80000000, 0x00003e0f, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

int charsHeight = 10;
int charsDivisor = 1; 

int charsWidth[224] = { 3, 1, 4, 6, 5, 7, 6, 2, 3, 3, 5, 5, 2, 4, 1, 7, 5, 2, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1, 3, 4, 3, 6,
7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 5, 6, 5, 7, 6, 6, 6, 6, 6, 6, 7, 6, 7, 7, 6, 6, 6, 2, 7, 2, 3, 5,
2, 5, 5, 5, 5, 5, 4, 5, 5, 1, 2, 5, 2, 5, 5, 5, 5, 5, 5, 5, 4, 5, 5, 5, 5, 5, 5, 3, 1, 3, 4, 4,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 5, 5, 5, 7, 1, 5, 3, 7, 3, 5, 4, 1, 7, 4, 3, 5, 3, 3, 2, 5, 6, 1, 2, 2, 3, 5, 6, 6, 6, 6,
6, 6, 6, 6, 6, 6, 7, 6, 6, 6, 6, 6, 3, 3, 3, 3, 7, 6, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 4, 6,
5, 5, 5, 5, 5, 5, 9, 5, 5, 5, 5, 5, 2, 2, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 5 };



int imWidth = 128;
int imHeight = 128;

Color *imagePixels = (Color *)RL_MALLOC(imWidth*imHeight*sizeof(Color));

for (int i = 0; i < imWidth*imHeight; i++) imagePixels[i] = BLANK; 

int counter = 0; 


for (int i = 0; i < imWidth*imHeight; i += 32)
{
for (int j = 31; j >= 0; j--)
{
if (BIT_CHECK(defaultFontData[counter], j)) imagePixels[i+j] = WHITE;
}

counter++;

if (counter > 512) counter = 0; 
}

Image imFont = LoadImageEx(imagePixels, imWidth, imHeight);
ImageFormat(&imFont, UNCOMPRESSED_GRAY_ALPHA);

RL_FREE(imagePixels);

defaultFont.texture = LoadTextureFromImage(imFont);






defaultFont.chars = (CharInfo *)RL_MALLOC(defaultFont.charsCount*sizeof(CharInfo));
defaultFont.recs = (Rectangle *)RL_MALLOC(defaultFont.charsCount*sizeof(Rectangle));

int currentLine = 0;
int currentPosX = charsDivisor;
int testPosX = charsDivisor;

for (int i = 0; i < defaultFont.charsCount; i++)
{
defaultFont.chars[i].value = 32 + i; 

defaultFont.recs[i].x = (float)currentPosX;
defaultFont.recs[i].y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
defaultFont.recs[i].width = (float)charsWidth[i];
defaultFont.recs[i].height = (float)charsHeight;

testPosX += (int)(defaultFont.recs[i].width + (float)charsDivisor);

if (testPosX >= defaultFont.texture.width)
{
currentLine++;
currentPosX = 2*charsDivisor + charsWidth[i];
testPosX = currentPosX;

defaultFont.recs[i].x = (float)charsDivisor;
defaultFont.recs[i].y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
}
else currentPosX = testPosX;


defaultFont.chars[i].offsetX = 0;
defaultFont.chars[i].offsetY = 0;
defaultFont.chars[i].advanceX = 0;


defaultFont.chars[i].image = ImageFromImage(imFont, defaultFont.recs[i]);
}

UnloadImage(imFont);

defaultFont.baseSize = (int)defaultFont.recs[0].height;

TRACELOG(LOG_INFO, "FONT: Default font loaded successfully");
}


extern void UnloadFontDefault(void)
{
for (int i = 0; i < defaultFont.charsCount; i++) UnloadImage(defaultFont.chars[i].image);
UnloadTexture(defaultFont.texture);
RL_FREE(defaultFont.chars);
RL_FREE(defaultFont.recs);
}
#endif 


Font GetFontDefault()
{
#if defined(SUPPORT_DEFAULT_FONT)
return defaultFont;
#else
Font font = { 0 };
return font;
#endif
}


Font LoadFont(const char *fileName)
{

#define DEFAULT_TTF_FONTSIZE 32 
#define DEFAULT_TTF_NUMCHARS 95 
#define DEFAULT_FIRST_CHAR 32 

Font font = { 0 };

#if defined(SUPPORT_FILEFORMAT_TTF)
if (IsFileExtension(fileName, ".ttf;.otf")) font = LoadFontEx(fileName, DEFAULT_TTF_FONTSIZE, NULL, DEFAULT_TTF_NUMCHARS);
else
#endif
#if defined(SUPPORT_FILEFORMAT_FNT)
if (IsFileExtension(fileName, ".fnt")) font = LoadBMFont(fileName);
else
#endif
{
Image image = LoadImage(fileName);
if (image.data != NULL) font = LoadFontFromImage(image, MAGENTA, DEFAULT_FIRST_CHAR);
UnloadImage(image);
}

if (font.texture.id == 0)
{
TRACELOG(LOG_WARNING, "FONT: [%s] Failed to load font texture -> Using default font", fileName);
font = GetFontDefault();
}
else SetTextureFilter(font.texture, FILTER_POINT); 

return font;
}




Font LoadFontEx(const char *fileName, int fontSize, int *fontChars, int charsCount)
{
Font font = { 0 };

#if defined(SUPPORT_FILEFORMAT_TTF)
font.baseSize = fontSize;
font.charsCount = (charsCount > 0)? charsCount : 95;
font.chars = LoadFontData(fileName, font.baseSize, fontChars, font.charsCount, FONT_DEFAULT);

if (font.chars != NULL)
{
Image atlas = GenImageFontAtlas(font.chars, &font.recs, font.charsCount, font.baseSize, 2, 0);
font.texture = LoadTextureFromImage(atlas);


for (int i = 0; i < font.charsCount; i++)
{
UnloadImage(font.chars[i].image);
font.chars[i].image = ImageFromImage(atlas, font.recs[i]);
}

UnloadImage(atlas);
}
else font = GetFontDefault();
#else
font = GetFontDefault();
#endif

return font;
}


Font LoadFontFromImage(Image image, Color key, int firstChar)
{
#define COLOR_EQUAL(col1, col2) ((col1.r == col2.r)&&(col1.g == col2.g)&&(col1.b == col2.b)&&(col1.a == col2.a))

int charSpacing = 0;
int lineSpacing = 0;

int x = 0;
int y = 0;


#define MAX_FONTCHARS 256



int tempCharValues[MAX_FONTCHARS];
Rectangle tempCharRecs[MAX_FONTCHARS];

Color *pixels = GetImageData(image);


for (y = 0; y < image.height; y++)
{
for (x = 0; x < image.width; x++)
{
if (!COLOR_EQUAL(pixels[y*image.width + x], key)) break;
}

if (!COLOR_EQUAL(pixels[y*image.width + x], key)) break;
}

charSpacing = x;
lineSpacing = y;

int charHeight = 0;
int j = 0;

while (!COLOR_EQUAL(pixels[(lineSpacing + j)*image.width + charSpacing], key)) j++;

charHeight = j;


int index = 0;
int lineToRead = 0;
int xPosToRead = charSpacing;


while ((lineSpacing + lineToRead*(charHeight + lineSpacing)) < image.height)
{
while ((xPosToRead < image.width) &&
!COLOR_EQUAL((pixels[(lineSpacing + (charHeight+lineSpacing)*lineToRead)*image.width + xPosToRead]), key))
{
tempCharValues[index] = firstChar + index;

tempCharRecs[index].x = (float)xPosToRead;
tempCharRecs[index].y = (float)(lineSpacing + lineToRead*(charHeight + lineSpacing));
tempCharRecs[index].height = (float)charHeight;

int charWidth = 0;

while (!COLOR_EQUAL(pixels[(lineSpacing + (charHeight+lineSpacing)*lineToRead)*image.width + xPosToRead + charWidth], key)) charWidth++;

tempCharRecs[index].width = (float)charWidth;

index++;

xPosToRead += (charWidth + charSpacing);
}

lineToRead++;
xPosToRead = charSpacing;
}



for (int i = 0; i < image.height*image.width; i++) if (COLOR_EQUAL(pixels[i], key)) pixels[i] = BLANK;


Image fontClear = LoadImageEx(pixels, image.width, image.height);

RL_FREE(pixels); 


Font font = { 0 };

font.texture = LoadTextureFromImage(fontClear); 
font.charsCount = index;



font.chars = (CharInfo *)RL_MALLOC(font.charsCount*sizeof(CharInfo));
font.recs = (Rectangle *)RL_MALLOC(font.charsCount*sizeof(Rectangle));

for (int i = 0; i < font.charsCount; i++)
{
font.chars[i].value = tempCharValues[i];


font.recs[i] = tempCharRecs[i];


font.chars[i].offsetX = 0;
font.chars[i].offsetY = 0;
font.chars[i].advanceX = 0;


font.chars[i].image = ImageFromImage(fontClear, tempCharRecs[i]);
}

UnloadImage(fontClear); 

font.baseSize = (int)font.recs[0].height;

return font;
}



CharInfo *LoadFontData(const char *fileName, int fontSize, int *fontChars, int charsCount, int type)
{


#define SDF_CHAR_PADDING 4
#define SDF_ON_EDGE_VALUE 128
#define SDF_PIXEL_DIST_SCALE 64.0f

#define BITMAP_ALPHA_THRESHOLD 80

CharInfo *chars = NULL;

#if defined(SUPPORT_FILEFORMAT_TTF)



unsigned int dataSize = 0;
unsigned char *fileData = LoadFileData(fileName, &dataSize);

if (fileData != NULL)
{
int genFontChars = false;
stbtt_fontinfo fontInfo = { 0 };

if (stbtt_InitFont(&fontInfo, fileData, 0)) 
{

float scaleFactor = stbtt_ScaleForPixelHeight(&fontInfo, (float)fontSize);



int ascent, descent, lineGap;
stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);


charsCount = (charsCount > 0)? charsCount : 95;




if (fontChars == NULL)
{
fontChars = (int *)RL_MALLOC(charsCount*sizeof(int));
for (int i = 0; i < charsCount; i++) fontChars[i] = i + 32;
genFontChars = true;
}

chars = (CharInfo *)RL_MALLOC(charsCount*sizeof(CharInfo));


for (int i = 0; i < charsCount; i++)
{
int chw = 0, chh = 0; 
int ch = fontChars[i]; 
chars[i].value = ch;






if (type != FONT_SDF) chars[i].image.data = stbtt_GetCodepointBitmap(&fontInfo, scaleFactor, scaleFactor, ch, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY);
else if (ch != 32) chars[i].image.data = stbtt_GetCodepointSDF(&fontInfo, scaleFactor, ch, SDF_CHAR_PADDING, SDF_ON_EDGE_VALUE, SDF_PIXEL_DIST_SCALE, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY);
else chars[i].image.data = NULL;

stbtt_GetCodepointHMetrics(&fontInfo, ch, &chars[i].advanceX, NULL);
chars[i].advanceX = (int)((float)chars[i].advanceX*scaleFactor);


chars[i].image.width = chw;
chars[i].image.height = chh;
chars[i].image.mipmaps = 1;
chars[i].image.format = UNCOMPRESSED_GRAYSCALE;

chars[i].offsetY += (int)((float)ascent*scaleFactor);


if (ch == 32)
{
chars[i].image = GenImageColor(chars[i].advanceX, fontSize, BLANK);
ImageFormat(&chars[i].image, UNCOMPRESSED_GRAYSCALE);
}

if (type == FONT_BITMAP)
{


for (int p = 0; p < chw*chh; p++)
{
if (((unsigned char *)chars[i].image.data)[p] < BITMAP_ALPHA_THRESHOLD) ((unsigned char *)chars[i].image.data)[p] = 0;
else ((unsigned char *)chars[i].image.data)[p] = 255;
}
}









}
}
else TRACELOG(LOG_WARNING, "FONT: Failed to process TTF font data");

RL_FREE(fileData);
if (genFontChars) RL_FREE(fontChars);
}
#endif

return chars;
}



#if defined(SUPPORT_FILEFORMAT_TTF)
Image GenImageFontAtlas(const CharInfo *chars, Rectangle **charRecs, int charsCount, int fontSize, int padding, int packMethod)
{
Image atlas = { 0 };

*charRecs = NULL;


charsCount = (charsCount > 0)? charsCount : 95;


Rectangle *recs = (Rectangle *)RL_MALLOC(charsCount*sizeof(Rectangle));





float requiredArea = 0;
for (int i = 0; i < charsCount; i++) requiredArea += ((chars[i].image.width + 2*padding)*(chars[i].image.height + 2*padding));
float guessSize = sqrtf(requiredArea)*1.3f;
int imageSize = (int)powf(2, ceilf(logf((float)guessSize)/logf(2))); 

atlas.width = imageSize; 
atlas.height = imageSize; 
atlas.data = (unsigned char *)RL_CALLOC(1, atlas.width*atlas.height); 
atlas.format = UNCOMPRESSED_GRAYSCALE;
atlas.mipmaps = 1;




if (packMethod == 0) 
{
int offsetX = padding;
int offsetY = padding;


for (int i = 0; i < charsCount; i++)
{

for (int y = 0; y < chars[i].image.height; y++)
{
for (int x = 0; x < chars[i].image.width; x++)
{
((unsigned char *)atlas.data)[(offsetY + y)*atlas.width + (offsetX + x)] = ((unsigned char *)chars[i].image.data)[y*chars[i].image.width + x];
}
}


recs[i].x = (float)offsetX;
recs[i].y = (float)offsetY;
recs[i].width = (float)chars[i].image.width;
recs[i].height = (float)chars[i].image.height;


offsetX += (chars[i].image.width + 2*padding);

if (offsetX >= (atlas.width - chars[i].image.width - 2*padding))
{
offsetX = padding;




offsetY += (fontSize + 2*padding);

if (offsetY > (atlas.height - fontSize - padding)) break;
}
}
}
else if (packMethod == 1) 
{
stbrp_context *context = (stbrp_context *)RL_MALLOC(sizeof(*context));
stbrp_node *nodes = (stbrp_node *)RL_MALLOC(charsCount*sizeof(*nodes));

stbrp_init_target(context, atlas.width, atlas.height, nodes, charsCount);
stbrp_rect *rects = (stbrp_rect *)RL_MALLOC(charsCount*sizeof(stbrp_rect));


for (int i = 0; i < charsCount; i++)
{
rects[i].id = i;
rects[i].w = chars[i].image.width + 2*padding;
rects[i].h = chars[i].image.height + 2*padding;
}


stbrp_pack_rects(context, rects, charsCount);

for (int i = 0; i < charsCount; i++)
{

recs[i].x = rects[i].x + (float)padding;
recs[i].y = rects[i].y + (float)padding;
recs[i].width = (float)chars[i].image.width;
recs[i].height = (float)chars[i].image.height;

if (rects[i].was_packed)
{

for (int y = 0; y < chars[i].image.height; y++)
{
for (int x = 0; x < chars[i].image.width; x++)
{
((unsigned char *)atlas.data)[(rects[i].y + padding + y)*atlas.width + (rects[i].x + padding + x)] = ((unsigned char *)chars[i].image.data)[y*chars[i].image.width + x];
}
}
}
else TRACELOG(LOG_WARNING, "FONT: Failed to package character (%i)", i);
}

RL_FREE(rects);
RL_FREE(nodes);
RL_FREE(context);
}





unsigned char *dataGrayAlpha = (unsigned char *)RL_MALLOC(atlas.width*atlas.height*sizeof(unsigned char)*2); 

for (int i = 0, k = 0; i < atlas.width*atlas.height; i++, k += 2)
{
dataGrayAlpha[k] = 255;
dataGrayAlpha[k + 1] = ((unsigned char *)atlas.data)[i];
}

RL_FREE(atlas.data);
atlas.data = dataGrayAlpha;
atlas.format = UNCOMPRESSED_GRAY_ALPHA;

*charRecs = recs;

return atlas;
}
#endif


void UnloadFont(Font font)
{

if (font.texture.id != GetFontDefault().texture.id)
{
for (int i = 0; i < font.charsCount; i++) UnloadImage(font.chars[i].image);

UnloadTexture(font.texture);
RL_FREE(font.chars);
RL_FREE(font.recs);

TRACELOGD("FONT: Unloaded font data from RAM and VRAM");
}
}



void DrawFPS(int posX, int posY)
{
DrawText(TextFormat("%2i FPS", GetFPS()), posX, posY, 20, LIME);
}




void DrawText(const char *text, int posX, int posY, int fontSize, Color color)
{

if (GetFontDefault().texture.id != 0)
{
Vector2 position = { (float)posX, (float)posY };

int defaultFontSize = 10; 
if (fontSize < defaultFontSize) fontSize = defaultFontSize;
int spacing = fontSize/defaultFontSize;

DrawTextEx(GetFontDefault(), text, position, (float)fontSize, (float)spacing, color);
}
}


void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float scale, Color tint)
{


int index = GetGlyphIndex(font, codepoint);



Rectangle rec = { position.x, position.y, font.recs[index].width*scale, font.recs[index].height*scale };

DrawTexturePro(font.texture, font.recs[index], rec, (Vector2){ 0, 0 }, 0.0f, tint);
}



void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint)
{
int length = TextLength(text); 

int textOffsetY = 0; 
float textOffsetX = 0.0f; 

float scaleFactor = fontSize/font.baseSize; 

for (int i = 0; i < length; i++)
{

int codepointByteCount = 0;
int codepoint = GetNextCodepoint(&text[i], &codepointByteCount);
int index = GetGlyphIndex(font, codepoint);



if (codepoint == 0x3f) codepointByteCount = 1;

if (codepoint == '\n')
{


textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
textOffsetX = 0.0f;
}
else
{
if ((codepoint != ' ') && (codepoint != '\t'))
{
Rectangle rec = { position.x + textOffsetX + font.chars[index].offsetX*scaleFactor,
position.y + textOffsetY + font.chars[index].offsetY*scaleFactor,
font.recs[index].width*scaleFactor,
font.recs[index].height*scaleFactor };

DrawTexturePro(font.texture, font.recs[index], rec, (Vector2){ 0, 0 }, 0.0f, tint);
}

if (font.chars[index].advanceX == 0) textOffsetX += ((float)font.recs[index].width*scaleFactor + spacing);
else textOffsetX += ((float)font.chars[index].advanceX*scaleFactor + spacing);
}

i += (codepointByteCount - 1); 
}
}


void DrawTextRec(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint)
{
DrawTextRecEx(font, text, rec, fontSize, spacing, wordWrap, tint, 0, 0, WHITE, WHITE);
}


void DrawTextRecEx(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint)
{
int length = TextLength(text); 

int textOffsetY = 0; 
float textOffsetX = 0.0f; 

float scaleFactor = fontSize/font.baseSize; 


enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
int state = wordWrap? MEASURE_STATE : DRAW_STATE;

int startLine = -1; 
int endLine = -1; 
int lastk = -1; 

for (int i = 0, k = 0; i < length; i++, k++)
{

int codepointByteCount = 0;
int codepoint = GetNextCodepoint(&text[i], &codepointByteCount);
int index = GetGlyphIndex(font, codepoint);



if (codepoint == 0x3f) codepointByteCount = 1;
i += (codepointByteCount - 1);

int glyphWidth = 0;
if (codepoint != '\n')
{
glyphWidth = (font.chars[index].advanceX == 0)?
(int)(font.recs[index].width*scaleFactor + spacing):
(int)(font.chars[index].advanceX*scaleFactor + spacing);
}






if (state == MEASURE_STATE)
{


if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n')) endLine = i;

if ((textOffsetX + glyphWidth + 1) >= rec.width)
{
endLine = (endLine < 1)? i : endLine;
if (i == endLine) endLine -= codepointByteCount;
if ((startLine + codepointByteCount) == endLine) endLine = (i - codepointByteCount);

state = !state;
}
else if ((i + 1) == length)
{
endLine = i;

state = !state;
}
else if (codepoint == '\n') state = !state;

if (state == DRAW_STATE)
{
textOffsetX = 0;
i = startLine;
glyphWidth = 0;


int tmp = lastk;
lastk = k - 1;
k = tmp;
}
}
else
{
if (codepoint == '\n')
{
if (!wordWrap)
{
textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
textOffsetX = 0;
}
}
else
{
if (!wordWrap && ((textOffsetX + glyphWidth + 1) >= rec.width))
{
textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
textOffsetX = 0;
}


if ((textOffsetY + (int)(font.baseSize*scaleFactor)) > rec.height) break;


bool isGlyphSelected = false;
if ((selectStart >= 0) && (k >= selectStart) && (k < (selectStart + selectLength)))
{
DrawRectangleRec((Rectangle){ rec.x + textOffsetX - 1, rec.y + textOffsetY, glyphWidth, (int)((float)font.baseSize*scaleFactor) }, selectBackTint);
isGlyphSelected = true;
}


if ((codepoint != ' ') && (codepoint != '\t'))
{
DrawTexturePro(font.texture, font.recs[index],
(Rectangle){ rec.x + textOffsetX + font.chars[index].offsetX*scaleFactor,
rec.y + textOffsetY + font.chars[index].offsetY*scaleFactor,
font.recs[index].width*scaleFactor, font.recs[index].height*scaleFactor },
(Vector2){ 0, 0 }, 0.0f, (!isGlyphSelected)? tint : selectTint);
}
}

if (wordWrap && (i == endLine))
{
textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
textOffsetX = 0;
startLine = endLine;
endLine = -1;
glyphWidth = 0;
selectStart += lastk - k;
k = lastk;

state = !state;
}
}

textOffsetX += glyphWidth;
}
}


int MeasureText(const char *text, int fontSize)
{
Vector2 vec = { 0.0f, 0.0f };


if (GetFontDefault().texture.id != 0)
{
int defaultFontSize = 10; 
if (fontSize < defaultFontSize) fontSize = defaultFontSize;
int spacing = fontSize/defaultFontSize;

vec = MeasureTextEx(GetFontDefault(), text, (float)fontSize, (float)spacing);
}

return (int)vec.x;
}


Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing)
{
int len = TextLength(text);
int tempLen = 0; 
int lenCounter = 0;

float textWidth = 0.0f;
float tempTextWidth = 0.0f; 

float textHeight = (float)font.baseSize;
float scaleFactor = fontSize/(float)font.baseSize;

int letter = 0; 
int index = 0; 

for (int i = 0; i < len; i++)
{
lenCounter++;

int next = 0;
letter = GetNextCodepoint(&text[i], &next);
index = GetGlyphIndex(font, letter);



if (letter == 0x3f) next = 1;
i += next - 1;

if (letter != '\n')
{
if (font.chars[index].advanceX != 0) textWidth += font.chars[index].advanceX;
else textWidth += (font.recs[index].width + font.chars[index].offsetX);
}
else
{
if (tempTextWidth < textWidth) tempTextWidth = textWidth;
lenCounter = 0;
textWidth = 0;
textHeight += ((float)font.baseSize*1.5f); 
}

if (tempLen < lenCounter) tempLen = lenCounter;
}

if (tempTextWidth < textWidth) tempTextWidth = textWidth;

Vector2 vec = { 0 };
vec.x = tempTextWidth*scaleFactor + (float)((tempLen - 1)*spacing); 
vec.y = textHeight*scaleFactor;

return vec;
}


int GetGlyphIndex(Font font, int codepoint)
{
#define TEXT_CHARACTER_NOTFOUND 63 

#define UNORDERED_CHARSET
#if defined(UNORDERED_CHARSET)
int index = TEXT_CHARACTER_NOTFOUND;

for (int i = 0; i < font.charsCount; i++)
{
if (font.chars[i].value == codepoint)
{
index = i;
break;
}
}

return index;
#else
return (codepoint - 32);
#endif
}






int TextCopy(char *dst, const char *src)
{
int bytes = 0;

if (dst != NULL)
{
while (*src != '\0')
{
*dst = *src;
dst++;
src++;

bytes++;
}

*dst = '\0';
}

return bytes;
}



bool TextIsEqual(const char *text1, const char *text2)
{
bool result = false;

if (strcmp(text1, text2) == 0) result = true;

return result;
}


unsigned int TextLength(const char *text)
{
unsigned int length = 0; 

if (text != NULL)
{
while (*text++) length++;
}

return length;
}



const char *TextFormat(const char *text, ...)
{
#define MAX_TEXTFORMAT_BUFFERS 4


static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = { 0 };
static int index = 0;

char *currentBuffer = buffers[index];
memset(currentBuffer, 0, MAX_TEXT_BUFFER_LENGTH); 

va_list args;
va_start(args, text);
vsprintf(currentBuffer, text, args);
va_end(args);

index += 1; 
if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;

return currentBuffer;
}


const char *TextSubtext(const char *text, int position, int length)
{
static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

int textLength = TextLength(text);

if (position >= textLength)
{
position = textLength - 1;
length = 0;
}

if (length >= textLength) length = textLength;

for (int c = 0 ; c < length ; c++)
{
*(buffer + c) = *(text + position);
text++;
}

*(buffer + length) = '\0';

return buffer;
}




char *TextReplace(char *text, const char *replace, const char *by)
{

if (!text || !replace || !by || by[0] == '\0') return NULL;

char *result;

char *insertPoint; 
char *temp; 
int replaceLen; 
int byLen; 
int lastReplacePos; 
int count; 

replaceLen = TextLength(replace);
if (replaceLen == 0) return NULL; 

byLen = TextLength(by);


insertPoint = text;
for (count = 0; (temp = strstr(insertPoint, replace)); count++) insertPoint = temp + replaceLen;


temp = result = RL_MALLOC(TextLength(text) + (byLen - replaceLen)*count + 1);

if (!result) return NULL; 





while (count--)
{
insertPoint = strstr(text, replace);
lastReplacePos = insertPoint - text;
temp = strncpy(temp, text, lastReplacePos) + lastReplacePos;
temp = strcpy(temp, by) + byLen;
text += lastReplacePos + replaceLen; 
}


strcpy(temp, text);

return result;
}



char *TextInsert(const char *text, const char *insert, int position)
{
int textLen = TextLength(text);
int insertLen = TextLength(insert);

char *result = (char *)RL_MALLOC(textLen + insertLen + 1);

for (int i = 0; i < position; i++) result[i] = text[i];
for (int i = position; i < insertLen + position; i++) result[i] = insert[i];
for (int i = (insertLen + position); i < (textLen + insertLen); i++) result[i] = text[i];

result[textLen + insertLen] = '\0'; 

return result;
}



const char *TextJoin(const char **textList, int count, const char *delimiter)
{
static char text[MAX_TEXT_BUFFER_LENGTH] = { 0 };
memset(text, 0, MAX_TEXT_BUFFER_LENGTH);

int totalLength = 0;
int delimiterLen = TextLength(delimiter);

for (int i = 0; i < count; i++)
{
int textListLength = TextLength(textList[i]);


if ((totalLength + textListLength) < MAX_TEXT_BUFFER_LENGTH)
{
strcat(text, textList[i]);
totalLength += textListLength;

if ((delimiterLen > 0) && (i < (count - 1)))
{
strcat(text, delimiter);
totalLength += delimiterLen;
}
}
}

return text;
}


const char **TextSplit(const char *text, char delimiter, int *count)
{






static const char *result[TEXTSPLIT_MAX_SUBSTRINGS_COUNT] = { NULL };
static char buffer[TEXTSPLIT_MAX_TEXT_BUFFER_LENGTH] = { 0 };
memset(buffer, 0, TEXTSPLIT_MAX_TEXT_BUFFER_LENGTH);

result[0] = buffer;
int counter = 0;

if (text != NULL)
{
counter = 1;


for (int i = 0; i < TEXTSPLIT_MAX_TEXT_BUFFER_LENGTH; i++)
{
buffer[i] = text[i];
if (buffer[i] == '\0') break;
else if (buffer[i] == delimiter)
{
buffer[i] = '\0'; 
result[counter] = buffer + i + 1;
counter++;

if (counter == TEXTSPLIT_MAX_SUBSTRINGS_COUNT) break;
}
}
}

*count = counter;
return result;
}



void TextAppend(char *text, const char *append, int *position)
{
strcpy(text + *position, append);
*position += TextLength(append);
}



int TextFindIndex(const char *text, const char *find)
{
int position = -1;

char *ptr = strstr(text, find);

if (ptr != NULL) position = ptr - text;

return position;
}


const char *TextToUpper(const char *text)
{
static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
{
if (text[i] != '\0')
{
buffer[i] = (char)toupper(text[i]);




}
else { buffer[i] = '\0'; break; }
}

return buffer;
}


const char *TextToLower(const char *text)
{
static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
{
if (text[i] != '\0')
{
buffer[i] = (char)tolower(text[i]);

}
else { buffer[i] = '\0'; break; }
}

return buffer;
}


const char *TextToPascal(const char *text)
{
static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

buffer[0] = (char)toupper(text[0]);

for (int i = 1, j = 1; i < MAX_TEXT_BUFFER_LENGTH; i++, j++)
{
if (text[j] != '\0')
{
if (text[j] != '_') buffer[i] = text[j];
else
{
j++;
buffer[i] = (char)toupper(text[j]);
}
}
else { buffer[i] = '\0'; break; }
}

return buffer;
}



int TextToInteger(const char *text)
{
int value = 0;
int sign = 1;

if ((text[0] == '+') || (text[0] == '-'))
{
if (text[0] == '-') sign = -1;
text++;
}

for (int i = 0; ((text[i] >= '0') && (text[i] <= '9')); ++i) value = value*10 + (int)(text[i] - '0');

return value*sign;
}


char *TextToUtf8(int *codepoints, int length)
{


char *text = (char *)RL_CALLOC(length*5, 1);
const char *utf8 = NULL;
int size = 0;

for (int i = 0, bytes = 0; i < length; i++)
{
utf8 = CodepointToUtf8(codepoints[i], &bytes);
strncpy(text + size, utf8, bytes);
size += bytes;
}


void *ptr = RL_REALLOC(text, size + 1);

if (ptr != NULL) text = (char *)ptr;

return text;
}


int *GetCodepoints(const char *text, int *count)
{
static int codepoints[MAX_TEXT_UNICODE_CHARS] = { 0 };
memset(codepoints, 0, MAX_TEXT_UNICODE_CHARS*sizeof(int));

int bytesProcessed = 0;
int textLength = TextLength(text);
int codepointsCount = 0;

for (int i = 0; i < textLength; codepointsCount++)
{
codepoints[codepointsCount] = GetNextCodepoint(text + i, &bytesProcessed);
i += bytesProcessed;
}

*count = codepointsCount;

return codepoints;
}



int GetCodepointsCount(const char *text)
{
unsigned int len = 0;
char *ptr = (char *)&text[0];

while (*ptr != '\0')
{
int next = 0;
int letter = GetNextCodepoint(ptr, &next);

if (letter == 0x3f) ptr += 1;
else ptr += next;

len++;
}

return len;
}








int GetNextCodepoint(const char *text, int *bytesProcessed)
{













int code = 0x3f; 
int octet = (unsigned char)(text[0]); 
*bytesProcessed = 1;

if (octet <= 0x7f)
{

code = text[0];
}
else if ((octet & 0xe0) == 0xc0)
{


unsigned char octet1 = text[1];

if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } 

if ((octet >= 0xc2) && (octet <= 0xdf))
{
code = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
*bytesProcessed = 2;
}
}
else if ((octet & 0xf0) == 0xe0)
{

unsigned char octet1 = text[1];
unsigned char octet2 = '\0';

if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } 

octet2 = text[2];

if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytesProcessed = 3; return code; } 








if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f)))) { *bytesProcessed = 2; return code; }

if ((octet >= 0xe0) && (0 <= 0xef))
{
code = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
*bytesProcessed = 3;
}
}
else if ((octet & 0xf8) == 0xf0)
{

if (octet > 0xf4) return code;

unsigned char octet1 = text[1];
unsigned char octet2 = '\0';
unsigned char octet3 = '\0';

if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } 

octet2 = text[2];

if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytesProcessed = 3; return code; } 

octet3 = text[3];

if ((octet3 == '\0') || ((octet3 >> 6) != 2)) { *bytesProcessed = 4; return code; } 







if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f)))) { *bytesProcessed = 2; return code; } 

if (octet >= 0xf0)
{
code = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
*bytesProcessed = 4;
}
}

if (code > 0x10ffff) code = 0x3f; 

return code;
}


RLAPI const char *CodepointToUtf8(int codepoint, int *byteLength)
{
static char utf8[6] = { 0 };
int length = 0;

if (codepoint <= 0x7f)
{
utf8[0] = (char)codepoint;
length = 1;
}
else if (codepoint <= 0x7ff)
{
utf8[0] = (char)(((codepoint >> 6) & 0x1f) | 0xc0);
utf8[1] = (char)((codepoint & 0x3f) | 0x80);
length = 2;
}
else if (codepoint <= 0xffff)
{
utf8[0] = (char)(((codepoint >> 12) & 0x0f) | 0xe0);
utf8[1] = (char)(((codepoint >> 6) & 0x3f) | 0x80);
utf8[2] = (char)((codepoint & 0x3f) | 0x80);
length = 3;
}
else if (codepoint <= 0x10ffff)
{
utf8[0] = (char)(((codepoint >> 18) & 0x07) | 0xf0);
utf8[1] = (char)(((codepoint >> 12) & 0x3f) | 0x80);
utf8[2] = (char)(((codepoint >> 6) & 0x3f) | 0x80);
utf8[3] = (char)((codepoint & 0x3f) | 0x80);
length = 4;
}

*byteLength = length;

return utf8;
}





#if defined(SUPPORT_FILEFORMAT_FNT)

static Font LoadBMFont(const char *fileName)
{
#define MAX_BUFFER_SIZE 256

Font font = { 0 };

char buffer[MAX_BUFFER_SIZE] = { 0 };
char *searchPoint = NULL;

int fontSize = 0;
int texWidth = 0;
int texHeight = 0;
char texFileName[129];
int charsCount = 0;

int base = 0; 

FILE *fntFile = NULL;

fntFile = fopen(fileName, "rt");

if (fntFile == NULL)
{
TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open FNT file", fileName);
return font;
}


fgets(buffer, MAX_BUFFER_SIZE, fntFile);



fgets(buffer, MAX_BUFFER_SIZE, fntFile);
searchPoint = strstr(buffer, "lineHeight");
sscanf(searchPoint, "lineHeight=%i base=%i scaleW=%i scaleH=%i", &fontSize, &base, &texWidth, &texHeight);

TRACELOGD("FONT: [%s] Loaded font info:", fileName);
TRACELOGD(" > Base size: %i", fontSize);
TRACELOGD(" > Texture scale: %ix%i", texWidth, texHeight);

fgets(buffer, MAX_BUFFER_SIZE, fntFile);
searchPoint = strstr(buffer, "file");
sscanf(searchPoint, "file=\"%128[^\"]\"", texFileName);

TRACELOGD(" > Texture filename: %s", texFileName);

fgets(buffer, MAX_BUFFER_SIZE, fntFile);
searchPoint = strstr(buffer, "count");
sscanf(searchPoint, "count=%i", &charsCount);

TRACELOGD(" > Chars count: %i", charsCount);


char *texPath = NULL;
char *lastSlash = NULL;

lastSlash = strrchr(fileName, '/');
if (lastSlash == NULL)
{
lastSlash = strrchr(fileName, '\\');
}


texPath = RL_MALLOC(TextLength(fileName) - TextLength(lastSlash) + TextLength(texFileName) + 4);


*texPath = '\0';
strncat(texPath, fileName, TextLength(fileName) - TextLength(lastSlash) + 1);
strncat(texPath, texFileName, TextLength(texFileName));

TRACELOGD(" > Texture loading path: %s", texPath);

Image imFont = LoadImage(texPath);

if (imFont.format == UNCOMPRESSED_GRAYSCALE)
{

ImageAlphaMask(&imFont, imFont);
for (int p = 0; p < (imFont.width*imFont.height*2); p += 2) ((unsigned char *)(imFont.data))[p] = 0xff;
}

font.texture = LoadTextureFromImage(imFont);

RL_FREE(texPath);


font.baseSize = fontSize;
font.charsCount = charsCount;
font.chars = (CharInfo *)RL_MALLOC(charsCount*sizeof(CharInfo));
font.recs = (Rectangle *)RL_MALLOC(charsCount*sizeof(Rectangle));

int charId, charX, charY, charWidth, charHeight, charOffsetX, charOffsetY, charAdvanceX;

for (int i = 0; i < charsCount; i++)
{
fgets(buffer, MAX_BUFFER_SIZE, fntFile);
sscanf(buffer, "char id=%i x=%i y=%i width=%i height=%i xoffset=%i yoffset=%i xadvance=%i",
&charId, &charX, &charY, &charWidth, &charHeight, &charOffsetX, &charOffsetY, &charAdvanceX);


font.recs[i] = (Rectangle){ (float)charX, (float)charY, (float)charWidth, (float)charHeight };


font.chars[i].value = charId;
font.chars[i].offsetX = charOffsetX;
font.chars[i].offsetY = charOffsetY;
font.chars[i].advanceX = charAdvanceX;


font.chars[i].image = ImageFromImage(imFont, font.recs[i]);
}

UnloadImage(imFont);

fclose(fntFile);

if (font.texture.id == 0)
{
UnloadFont(font);
font = GetFontDefault();
TRACELOG(LOG_WARNING, "FONT: [%s] Failed to load texture, reverted to default font", fileName);
}
else TRACELOG(LOG_INFO, "FONT: [%s] Font loaded successfully", fileName);

return font;
}
#endif
