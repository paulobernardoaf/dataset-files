#include "raylib.h" 
#if !defined(EXTERNAL_CONFIG_FLAGS)
#include "config.h" 
#endif
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <math.h> 
#include "utils.h" 
#include "rlgl.h" 
#if !defined(SUPPORT_FILEFORMAT_BMP)
#define STBI_NO_BMP
#endif
#if !defined(SUPPORT_FILEFORMAT_PNG)
#define STBI_NO_PNG
#endif
#if !defined(SUPPORT_FILEFORMAT_TGA)
#define STBI_NO_TGA
#endif
#if !defined(SUPPORT_FILEFORMAT_JPG)
#define STBI_NO_JPEG 
#endif
#if !defined(SUPPORT_FILEFORMAT_PSD)
#define STBI_NO_PSD
#endif
#if !defined(SUPPORT_FILEFORMAT_GIF)
#define STBI_NO_GIF
#endif
#if !defined(SUPPORT_FILEFORMAT_PIC)
#define STBI_NO_PIC
#endif
#if !defined(SUPPORT_FILEFORMAT_HDR)
#define STBI_NO_HDR
#endif
#define STBI_NO_PIC
#define STBI_NO_PNM 
#if (defined(SUPPORT_FILEFORMAT_BMP) || defined(SUPPORT_FILEFORMAT_PNG) || defined(SUPPORT_FILEFORMAT_TGA) || defined(SUPPORT_FILEFORMAT_JPG) || defined(SUPPORT_FILEFORMAT_PSD) || defined(SUPPORT_FILEFORMAT_GIF) || defined(SUPPORT_FILEFORMAT_PIC) || defined(SUPPORT_FILEFORMAT_HDR))
#define STBI_MALLOC RL_MALLOC
#define STBI_FREE RL_FREE
#define STBI_REALLOC RL_REALLOC
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h" 
#endif
#if defined(SUPPORT_IMAGE_EXPORT)
#define STBIW_MALLOC RL_MALLOC
#define STBIW_FREE RL_FREE
#define STBIW_REALLOC RL_REALLOC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb_image_write.h" 
#endif
#if defined(SUPPORT_IMAGE_MANIPULATION)
#define STBIR_MALLOC(size,c) ((void)(c), RL_MALLOC(size))
#define STBIR_FREE(ptr,c) ((void)(c), RL_FREE(ptr))
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "external/stb_image_resize.h" 
#endif
#if defined(SUPPORT_IMAGE_GENERATION)
#define STB_PERLIN_IMPLEMENTATION
#include "external/stb_perlin.h" 
#endif
#if defined(SUPPORT_FILEFORMAT_GIF)
static Image LoadAnimatedGIF(const char *fileName, int *frames, int **delays); 
#endif
#if defined(SUPPORT_FILEFORMAT_DDS)
static Image LoadDDS(const char *fileName); 
#endif
#if defined(SUPPORT_FILEFORMAT_PKM)
static Image LoadPKM(const char *fileName); 
#endif
#if defined(SUPPORT_FILEFORMAT_KTX)
static Image LoadKTX(const char *fileName); 
static int SaveKTX(Image image, const char *fileName); 
#endif
#if defined(SUPPORT_FILEFORMAT_PVR)
static Image LoadPVR(const char *fileName); 
#endif
#if defined(SUPPORT_FILEFORMAT_ASTC)
static Image LoadASTC(const char *fileName); 
#endif
Image LoadImage(const char *fileName)
{
Image image = { 0 };
#if defined(SUPPORT_FILEFORMAT_PNG) || defined(SUPPORT_FILEFORMAT_BMP) || defined(SUPPORT_FILEFORMAT_TGA) || defined(SUPPORT_FILEFORMAT_GIF) || defined(SUPPORT_FILEFORMAT_PIC) || defined(SUPPORT_FILEFORMAT_HDR) || defined(SUPPORT_FILEFORMAT_PSD)
#define STBI_REQUIRED
#endif
#if defined(SUPPORT_FILEFORMAT_PNG)
if ((IsFileExtension(fileName, ".png"))
#else
if ((false)
#endif
#if defined(SUPPORT_FILEFORMAT_BMP)
|| (IsFileExtension(fileName, ".bmp"))
#endif
#if defined(SUPPORT_FILEFORMAT_TGA)
|| (IsFileExtension(fileName, ".tga"))
#endif
#if defined(SUPPORT_FILEFORMAT_JPG)
|| (IsFileExtension(fileName, ".jpg"))
#endif
#if defined(SUPPORT_FILEFORMAT_GIF)
|| (IsFileExtension(fileName, ".gif"))
#endif
#if defined(SUPPORT_FILEFORMAT_PIC)
|| (IsFileExtension(fileName, ".pic"))
#endif
#if defined(SUPPORT_FILEFORMAT_PSD)
|| (IsFileExtension(fileName, ".psd"))
#endif
)
{
#if defined(STBI_REQUIRED)
unsigned int dataSize = 0;
unsigned char *fileData = LoadFileData(fileName, &dataSize);
if (fileData != NULL)
{
int comp = 0;
image.data = stbi_load_from_memory(fileData, dataSize, &image.width, &image.height, &comp, 0);
image.mipmaps = 1;
if (comp == 1) image.format = UNCOMPRESSED_GRAYSCALE;
else if (comp == 2) image.format = UNCOMPRESSED_GRAY_ALPHA;
else if (comp == 3) image.format = UNCOMPRESSED_R8G8B8;
else if (comp == 4) image.format = UNCOMPRESSED_R8G8B8A8;
RL_FREE(fileData);
}
#endif
}
#if defined(SUPPORT_FILEFORMAT_HDR)
else if (IsFileExtension(fileName, ".hdr"))
{
#if defined(STBI_REQUIRED)
unsigned int dataSize = 0;
unsigned char *fileData = LoadFileData(fileName, &dataSize);
if (fileData != NULL)
{
int comp = 0;
image.data = stbi_loadf_from_memory(fileData, dataSize, &image.width, &image.height, &comp, 0);
image.mipmaps = 1;
if (comp == 1) image.format = UNCOMPRESSED_R32;
else if (comp == 3) image.format = UNCOMPRESSED_R32G32B32;
else if (comp == 4) image.format = UNCOMPRESSED_R32G32B32A32;
else
{
TRACELOG(LOG_WARNING, "IMAGE: [%s] HDR fileformat not supported", fileName);
UnloadImage(image);
}
RL_FREE(fileData);
}
#endif
}
#endif
#if defined(SUPPORT_FILEFORMAT_DDS)
else if (IsFileExtension(fileName, ".dds")) image = LoadDDS(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_PKM)
else if (IsFileExtension(fileName, ".pkm")) image = LoadPKM(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_KTX)
else if (IsFileExtension(fileName, ".ktx")) image = LoadKTX(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_PVR)
else if (IsFileExtension(fileName, ".pvr")) image = LoadPVR(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_ASTC)
else if (IsFileExtension(fileName, ".astc")) image = LoadASTC(fileName);
#endif
else TRACELOG(LOG_WARNING, "IMAGE: [%s] Fileformat not supported", fileName);
if (image.data != NULL) TRACELOG(LOG_INFO, "IMAGE: [%s] Data loaded successfully (%ix%i)", fileName, image.width, image.height);
else TRACELOG(LOG_WARNING, "IMAGE: [%s] Failed to load data", fileName);
return image;
}
Image LoadImageEx(Color *pixels, int width, int height)
{
Image image = { 0 };
image.data = NULL;
image.width = width;
image.height = height;
image.mipmaps = 1;
image.format = UNCOMPRESSED_R8G8B8A8;
int k = 0;
image.data = (unsigned char *)RL_MALLOC(image.width*image.height*4*sizeof(unsigned char));
for (int i = 0; i < image.width*image.height*4; i += 4)
{
((unsigned char *)image.data)[i] = pixels[k].r;
((unsigned char *)image.data)[i + 1] = pixels[k].g;
((unsigned char *)image.data)[i + 2] = pixels[k].b;
((unsigned char *)image.data)[i + 3] = pixels[k].a;
k++;
}
return image;
}
Image LoadImagePro(void *data, int width, int height, int format)
{
Image srcImage = { 0 };
srcImage.data = data;
srcImage.width = width;
srcImage.height = height;
srcImage.mipmaps = 1;
srcImage.format = format;
Image dstImage = ImageCopy(srcImage);
return dstImage;
}
Image LoadImageRaw(const char *fileName, int width, int height, int format, int headerSize)
{
Image image = { 0 };
unsigned int dataSize = 0;
unsigned char *fileData = LoadFileData(fileName, &dataSize);
if (fileData != NULL)
{
unsigned char *dataPtr = fileData;
unsigned int size = GetPixelDataSize(width, height, format);
if (headerSize > 0) dataPtr += headerSize;
image.data = RL_MALLOC(size); 
memcpy(image.data, dataPtr, size); 
image.width = width;
image.height = height;
image.mipmaps = 1;
image.format = format;
RL_FREE(fileData);
}
return image;
}
Texture2D LoadTexture(const char *fileName)
{
Texture2D texture = { 0 };
Image image = LoadImage(fileName);
if (image.data != NULL)
{
texture = LoadTextureFromImage(image);
UnloadImage(image);
}
return texture;
}
Texture2D LoadTextureFromImage(Image image)
{
Texture2D texture = { 0 };
if ((image.data != NULL) && (image.width != 0) && (image.height != 0))
{
texture.id = rlLoadTexture(image.data, image.width, image.height, image.format, image.mipmaps);
}
else TRACELOG(LOG_WARNING, "IMAGE: Data is not valid to load texture");
texture.width = image.width;
texture.height = image.height;
texture.mipmaps = image.mipmaps;
texture.format = image.format;
return texture;
}
RenderTexture2D LoadRenderTexture(int width, int height)
{
RenderTexture2D target = rlLoadRenderTexture(width, height, UNCOMPRESSED_R8G8B8A8, 24, false);
return target;
}
void UnloadImage(Image image)
{
RL_FREE(image.data);
}
void UnloadTexture(Texture2D texture)
{
if (texture.id > 0)
{
rlDeleteTextures(texture.id);
TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Unloaded texture data from VRAM (GPU)", texture.id);
}
}
void UnloadRenderTexture(RenderTexture2D target)
{
if (target.id > 0) rlDeleteRenderTextures(target);
}
Color *GetImageData(Image image)
{
if ((image.width == 0) || (image.height == 0)) return NULL;
Color *pixels = (Color *)RL_MALLOC(image.width*image.height*sizeof(Color));
if (image.format >= COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "IMAGE: Pixel data retrieval not supported for compressed image formats");
else
{
if ((image.format == UNCOMPRESSED_R32) ||
(image.format == UNCOMPRESSED_R32G32B32) ||
(image.format == UNCOMPRESSED_R32G32B32A32)) TRACELOG(LOG_WARNING, "IMAGE: Pixel format converted from 32bit to 8bit per channel");
for (int i = 0, k = 0; i < image.width*image.height; i++)
{
switch (image.format)
{
case UNCOMPRESSED_GRAYSCALE:
{
pixels[i].r = ((unsigned char *)image.data)[i];
pixels[i].g = ((unsigned char *)image.data)[i];
pixels[i].b = ((unsigned char *)image.data)[i];
pixels[i].a = 255;
} break;
case UNCOMPRESSED_GRAY_ALPHA:
{
pixels[i].r = ((unsigned char *)image.data)[k];
pixels[i].g = ((unsigned char *)image.data)[k];
pixels[i].b = ((unsigned char *)image.data)[k];
pixels[i].a = ((unsigned char *)image.data)[k + 1];
k += 2;
} break;
case UNCOMPRESSED_R5G5B5A1:
{
unsigned short pixel = ((unsigned short *)image.data)[i];
pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111000000) >> 6)*(255/31));
pixels[i].b = (unsigned char)((float)((pixel & 0b0000000000111110) >> 1)*(255/31));
pixels[i].a = (unsigned char)((pixel & 0b0000000000000001)*255);
} break;
case UNCOMPRESSED_R5G6B5:
{
unsigned short pixel = ((unsigned short *)image.data)[i];
pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111100000) >> 5)*(255/63));
pixels[i].b = (unsigned char)((float)(pixel & 0b0000000000011111)*(255/31));
pixels[i].a = 255;
} break;
case UNCOMPRESSED_R4G4B4A4:
{
unsigned short pixel = ((unsigned short *)image.data)[i];
pixels[i].r = (unsigned char)((float)((pixel & 0b1111000000000000) >> 12)*(255/15));
pixels[i].g = (unsigned char)((float)((pixel & 0b0000111100000000) >> 8)*(255/15));
pixels[i].b = (unsigned char)((float)((pixel & 0b0000000011110000) >> 4)*(255/15));
pixels[i].a = (unsigned char)((float)(pixel & 0b0000000000001111)*(255/15));
} break;
case UNCOMPRESSED_R8G8B8A8:
{
pixels[i].r = ((unsigned char *)image.data)[k];
pixels[i].g = ((unsigned char *)image.data)[k + 1];
pixels[i].b = ((unsigned char *)image.data)[k + 2];
pixels[i].a = ((unsigned char *)image.data)[k + 3];
k += 4;
} break;
case UNCOMPRESSED_R8G8B8:
{
pixels[i].r = (unsigned char)((unsigned char *)image.data)[k];
pixels[i].g = (unsigned char)((unsigned char *)image.data)[k + 1];
pixels[i].b = (unsigned char)((unsigned char *)image.data)[k + 2];
pixels[i].a = 255;
k += 3;
} break;
case UNCOMPRESSED_R32:
{
pixels[i].r = (unsigned char)(((float *)image.data)[k]*255.0f);
pixels[i].g = 0;
pixels[i].b = 0;
pixels[i].a = 255;
} break;
case UNCOMPRESSED_R32G32B32:
{
pixels[i].r = (unsigned char)(((float *)image.data)[k]*255.0f);
pixels[i].g = (unsigned char)(((float *)image.data)[k + 1]*255.0f);
pixels[i].b = (unsigned char)(((float *)image.data)[k + 2]*255.0f);
pixels[i].a = 255;
k += 3;
} break;
case UNCOMPRESSED_R32G32B32A32:
{
pixels[i].r = (unsigned char)(((float *)image.data)[k]*255.0f);
pixels[i].g = (unsigned char)(((float *)image.data)[k]*255.0f);
pixels[i].b = (unsigned char)(((float *)image.data)[k]*255.0f);
pixels[i].a = (unsigned char)(((float *)image.data)[k]*255.0f);
k += 4;
} break;
default: break;
}
}
}
return pixels;
}
Vector4 *GetImageDataNormalized(Image image)
{
Vector4 *pixels = (Vector4 *)RL_MALLOC(image.width*image.height*sizeof(Vector4));
if (image.format >= COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "IMAGE: Pixel data retrieval not supported for compressed image formats");
else
{
for (int i = 0, k = 0; i < image.width*image.height; i++)
{
switch (image.format)
{
case UNCOMPRESSED_GRAYSCALE:
{
pixels[i].x = (float)((unsigned char *)image.data)[i]/255.0f;
pixels[i].y = (float)((unsigned char *)image.data)[i]/255.0f;
pixels[i].z = (float)((unsigned char *)image.data)[i]/255.0f;
pixels[i].w = 1.0f;
} break;
case UNCOMPRESSED_GRAY_ALPHA:
{
pixels[i].x = (float)((unsigned char *)image.data)[k]/255.0f;
pixels[i].y = (float)((unsigned char *)image.data)[k]/255.0f;
pixels[i].z = (float)((unsigned char *)image.data)[k]/255.0f;
pixels[i].w = (float)((unsigned char *)image.data)[k + 1]/255.0f;
k += 2;
} break;
case UNCOMPRESSED_R5G5B5A1:
{
unsigned short pixel = ((unsigned short *)image.data)[i];
pixels[i].x = (float)((pixel & 0b1111100000000000) >> 11)*(1.0f/31);
pixels[i].y = (float)((pixel & 0b0000011111000000) >> 6)*(1.0f/31);
pixels[i].z = (float)((pixel & 0b0000000000111110) >> 1)*(1.0f/31);
pixels[i].w = ((pixel & 0b0000000000000001) == 0)? 0.0f : 1.0f;
} break;
case UNCOMPRESSED_R5G6B5:
{
unsigned short pixel = ((unsigned short *)image.data)[i];
pixels[i].x = (float)((pixel & 0b1111100000000000) >> 11)*(1.0f/31);
pixels[i].y = (float)((pixel & 0b0000011111100000) >> 5)*(1.0f/63);
pixels[i].z = (float)(pixel & 0b0000000000011111)*(1.0f/31);
pixels[i].w = 1.0f;
} break;
case UNCOMPRESSED_R4G4B4A4:
{
unsigned short pixel = ((unsigned short *)image.data)[i];
pixels[i].x = (float)((pixel & 0b1111000000000000) >> 12)*(1.0f/15);
pixels[i].y = (float)((pixel & 0b0000111100000000) >> 8)*(1.0f/15);
pixels[i].z = (float)((pixel & 0b0000000011110000) >> 4)*(1.0f/15);
pixels[i].w = (float)(pixel & 0b0000000000001111)*(1.0f/15);
} break;
case UNCOMPRESSED_R8G8B8A8:
{
pixels[i].x = (float)((unsigned char *)image.data)[k]/255.0f;
pixels[i].y = (float)((unsigned char *)image.data)[k + 1]/255.0f;
pixels[i].z = (float)((unsigned char *)image.data)[k + 2]/255.0f;
pixels[i].w = (float)((unsigned char *)image.data)[k + 3]/255.0f;
k += 4;
} break;
case UNCOMPRESSED_R8G8B8:
{
pixels[i].x = (float)((unsigned char *)image.data)[k]/255.0f;
pixels[i].y = (float)((unsigned char *)image.data)[k + 1]/255.0f;
pixels[i].z = (float)((unsigned char *)image.data)[k + 2]/255.0f;
pixels[i].w = 1.0f;
k += 3;
} break;
case UNCOMPRESSED_R32:
{
pixels[i].x = ((float *)image.data)[k];
pixels[i].y = 0.0f;
pixels[i].z = 0.0f;
pixels[i].w = 1.0f;
} break;
case UNCOMPRESSED_R32G32B32:
{
pixels[i].x = ((float *)image.data)[k];
pixels[i].y = ((float *)image.data)[k + 1];
pixels[i].z = ((float *)image.data)[k + 2];
pixels[i].w = 1.0f;
k += 3;
} break;
case UNCOMPRESSED_R32G32B32A32:
{
pixels[i].x = ((float *)image.data)[k];
pixels[i].y = ((float *)image.data)[k + 1];
pixels[i].z = ((float *)image.data)[k + 2];
pixels[i].w = ((float *)image.data)[k + 3];
k += 4;
}
default: break;
}
}
}
return pixels;
}
Rectangle GetImageAlphaBorder(Image image, float threshold)
{
Rectangle crop = { 0 };
Color *pixels = GetImageData(image);
if (pixels != NULL)
{
int xMin = 65536; 
int xMax = 0;
int yMin = 65536;
int yMax = 0;
for (int y = 0; y < image.height; y++)
{
for (int x = 0; x < image.width; x++)
{
if (pixels[y*image.width + x].a > (unsigned char)(threshold*255.0f))
{
if (x < xMin) xMin = x;
if (x > xMax) xMax = x;
if (y < yMin) yMin = y;
if (y > yMax) yMax = y;
}
}
}
if ((xMin != 65536) && (xMax != 65536))
{
crop = (Rectangle){ xMin, yMin, (xMax + 1) - xMin, (yMax + 1) - yMin };
}
RL_FREE(pixels);
}
return crop;
}
int GetPixelDataSize(int width, int height, int format)
{
int dataSize = 0; 
int bpp = 0; 
switch (format)
{
case UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
case UNCOMPRESSED_GRAY_ALPHA:
case UNCOMPRESSED_R5G6B5:
case UNCOMPRESSED_R5G5B5A1:
case UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
case UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
case UNCOMPRESSED_R8G8B8: bpp = 24; break;
case UNCOMPRESSED_R32: bpp = 32; break;
case UNCOMPRESSED_R32G32B32: bpp = 32*3; break;
case UNCOMPRESSED_R32G32B32A32: bpp = 32*4; break;
case COMPRESSED_DXT1_RGB:
case COMPRESSED_DXT1_RGBA:
case COMPRESSED_ETC1_RGB:
case COMPRESSED_ETC2_RGB:
case COMPRESSED_PVRT_RGB:
case COMPRESSED_PVRT_RGBA: bpp = 4; break;
case COMPRESSED_DXT3_RGBA:
case COMPRESSED_DXT5_RGBA:
case COMPRESSED_ETC2_EAC_RGBA:
case COMPRESSED_ASTC_4x4_RGBA: bpp = 8; break;
case COMPRESSED_ASTC_8x8_RGBA: bpp = 2; break;
default: break;
}
dataSize = width*height*bpp/8; 
return dataSize;
}
Image GetTextureData(Texture2D texture)
{
Image image = { 0 };
if (texture.format < 8)
{
image.data = rlReadTexturePixels(texture);
if (image.data != NULL)
{
image.width = texture.width;
image.height = texture.height;
image.format = texture.format;
image.mipmaps = 1;
#if defined(GRAPHICS_API_OPENGL_ES2)
image.format = UNCOMPRESSED_R8G8B8A8;
#endif
TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Pixel data retrieved successfully", texture.id);
}
else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Failed to retrieve pixel data", texture.id);
}
else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Failed to retrieve compressed pixel data", texture.id);
return image;
}
Image GetScreenData(void)
{
Image image = { 0 };
image.width = GetScreenWidth();
image.height = GetScreenHeight();
image.mipmaps = 1;
image.format = UNCOMPRESSED_R8G8B8A8;
image.data = rlReadScreenPixels(image.width, image.height);
return image;
}
void UpdateTexture(Texture2D texture, const void *pixels)
{
rlUpdateTexture(texture.id, texture.width, texture.height, texture.format, pixels);
}
void ExportImage(Image image, const char *fileName)
{
int success = 0;
#if defined(SUPPORT_IMAGE_EXPORT)
unsigned char *imgData = (unsigned char *)GetImageData(image);
#if defined(SUPPORT_FILEFORMAT_PNG)
if (IsFileExtension(fileName, ".png")) success = stbi_write_png(fileName, image.width, image.height, 4, imgData, image.width*4);
#else
if (false) {}
#endif
#if defined(SUPPORT_FILEFORMAT_BMP)
else if (IsFileExtension(fileName, ".bmp")) success = stbi_write_bmp(fileName, image.width, image.height, 4, imgData);
#endif
#if defined(SUPPORT_FILEFORMAT_TGA)
else if (IsFileExtension(fileName, ".tga")) success = stbi_write_tga(fileName, image.width, image.height, 4, imgData);
#endif
#if defined(SUPPORT_FILEFORMAT_JPG)
else if (IsFileExtension(fileName, ".jpg")) success = stbi_write_jpg(fileName, image.width, image.height, 4, imgData, 80); 
#endif
#if defined(SUPPORT_FILEFORMAT_KTX)
else if (IsFileExtension(fileName, ".ktx")) success = SaveKTX(image, fileName);
#endif
else if (IsFileExtension(fileName, ".raw"))
{
SaveFileData(fileName, image.data, GetPixelDataSize(image.width, image.height, image.format));
success = true;
}
RL_FREE(imgData);
#endif
if (success != 0) TRACELOG(LOG_INFO, "FILEIO: [%s] Image exported successfully", fileName);
else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to export image", fileName);
}
void ExportImageAsCode(Image image, const char *fileName)
{
#define BYTES_TEXT_PER_LINE 20
FILE *txtFile = fopen(fileName, "wt");
if (txtFile != NULL)
{
char varFileName[256] = { 0 };
int dataSize = GetPixelDataSize(image.width, image.height, image.format);
fprintf(txtFile, "////////////////////////////////////////////////////////////////////////////////////////\n");
fprintf(txtFile, "// //\n");
fprintf(txtFile, "// ImageAsCode exporter v1.0 - Image pixel data exported as an array of bytes //\n");
fprintf(txtFile, "// //\n");
fprintf(txtFile, "// more info and bugs-report: github.com/raysan5/raylib //\n");
fprintf(txtFile, "// feedback and support: ray[at]raylib.com //\n");
fprintf(txtFile, "// //\n");
fprintf(txtFile, "// Copyright (c) 2020 Ramon Santamaria (@raysan5) //\n");
fprintf(txtFile, "// //\n");
fprintf(txtFile, "////////////////////////////////////////////////////////////////////////////////////////\n\n");
strcpy(varFileName, GetFileNameWithoutExt(fileName));
for (int i = 0; varFileName[i] != '\0'; i++) if ((varFileName[i] >= 'a') && (varFileName[i] <= 'z')) { varFileName[i] = varFileName[i] - 32; }
fprintf(txtFile, "// Image data information\n");
fprintf(txtFile, "#define %s_WIDTH %i\n", varFileName, image.width);
fprintf(txtFile, "#define %s_HEIGHT %i\n", varFileName, image.height);
fprintf(txtFile, "#define %s_FORMAT %i // raylib internal pixel format\n\n", varFileName, image.format);
fprintf(txtFile, "static unsigned char %s_DATA[%i] = { ", varFileName, dataSize);
for (int i = 0; i < dataSize - 1; i++) fprintf(txtFile, ((i%BYTES_TEXT_PER_LINE == 0)? "0x%x,\n" : "0x%x, "), ((unsigned char *)image.data)[i]);
fprintf(txtFile, "0x%x };\n", ((unsigned char *)image.data)[dataSize - 1]);
fclose(txtFile);
}
}
Image ImageCopy(Image image)
{
Image newImage = { 0 };
int width = image.width;
int height = image.height;
int size = 0;
for (int i = 0; i < image.mipmaps; i++)
{
size += GetPixelDataSize(width, height, image.format);
width /= 2;
height /= 2;
if (width < 1) width = 1;
if (height < 1) height = 1;
}
newImage.data = RL_MALLOC(size);
if (newImage.data != NULL)
{
memcpy(newImage.data, image.data, size);
newImage.width = image.width;
newImage.height = image.height;
newImage.mipmaps = image.mipmaps;
newImage.format = image.format;
}
return newImage;
}
Image ImageFromImage(Image image, Rectangle rec)
{
Image result = ImageCopy(image);
#if defined(SUPPORT_IMAGE_MANIPULATION)
ImageCrop(&result, rec);
#endif
return result;
}
void ImageToPOT(Image *image, Color fillColor)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
int potWidth = (int)powf(2, ceilf(logf((float)image->width)/logf(2)));
int potHeight = (int)powf(2, ceilf(logf((float)image->height)/logf(2)));
if ((potWidth != image->width) || (potHeight != image->height))
{
Color *pixels = GetImageData(*image); 
Color *pixelsPOT = NULL;
pixelsPOT = (Color *)RL_MALLOC(potWidth*potHeight*sizeof(Color));
for (int j = 0; j < potHeight; j++)
{
for (int i = 0; i < potWidth; i++)
{
if ((j < image->height) && (i < image->width)) pixelsPOT[j*potWidth + i] = pixels[j*image->width + i];
else pixelsPOT[j*potWidth + i] = fillColor;
}
}
RL_FREE(pixels); 
RL_FREE(image->data); 
int format = image->format; 
*image = LoadImageEx(pixelsPOT, potWidth, potHeight);
RL_FREE(pixelsPOT); 
ImageFormat(image, format); 
TRACELOG(LOG_WARNING, "IMAGE: Converted to POT: (%ix%i) -> (%ix%i)", image->width, image->height, potWidth, potHeight);
}
}
void ImageFormat(Image *image, int newFormat)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
if ((newFormat != 0) && (image->format != newFormat))
{
if ((image->format < COMPRESSED_DXT1_RGB) && (newFormat < COMPRESSED_DXT1_RGB))
{
Vector4 *pixels = GetImageDataNormalized(*image); 
RL_FREE(image->data); 
image->data = NULL;
image->format = newFormat;
int k = 0;
switch (image->format)
{
case UNCOMPRESSED_GRAYSCALE:
{
image->data = (unsigned char *)RL_MALLOC(image->width*image->height*sizeof(unsigned char));
for (int i = 0; i < image->width*image->height; i++)
{
((unsigned char *)image->data)[i] = (unsigned char)((pixels[i].x*0.299f + pixels[i].y*0.587f + pixels[i].z*0.114f)*255.0f);
}
} break;
case UNCOMPRESSED_GRAY_ALPHA:
{
image->data = (unsigned char *)RL_MALLOC(image->width*image->height*2*sizeof(unsigned char));
for (int i = 0; i < image->width*image->height*2; i += 2, k++)
{
((unsigned char *)image->data)[i] = (unsigned char)((pixels[k].x*0.299f + (float)pixels[k].y*0.587f + (float)pixels[k].z*0.114f)*255.0f);
((unsigned char *)image->data)[i + 1] = (unsigned char)(pixels[k].w*255.0f);
}
} break;
case UNCOMPRESSED_R5G6B5:
{
image->data = (unsigned short *)RL_MALLOC(image->width*image->height*sizeof(unsigned short));
unsigned char r = 0;
unsigned char g = 0;
unsigned char b = 0;
for (int i = 0; i < image->width*image->height; i++)
{
r = (unsigned char)(round(pixels[i].x*31.0f));
g = (unsigned char)(round(pixels[i].y*63.0f));
b = (unsigned char)(round(pixels[i].z*31.0f));
((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 5 | (unsigned short)b;
}
} break;
case UNCOMPRESSED_R8G8B8:
{
image->data = (unsigned char *)RL_MALLOC(image->width*image->height*3*sizeof(unsigned char));
for (int i = 0, k = 0; i < image->width*image->height*3; i += 3, k++)
{
((unsigned char *)image->data)[i] = (unsigned char)(pixels[k].x*255.0f);
((unsigned char *)image->data)[i + 1] = (unsigned char)(pixels[k].y*255.0f);
((unsigned char *)image->data)[i + 2] = (unsigned char)(pixels[k].z*255.0f);
}
} break;
case UNCOMPRESSED_R5G5B5A1:
{
#define ALPHA_THRESHOLD 50
image->data = (unsigned short *)RL_MALLOC(image->width*image->height*sizeof(unsigned short));
unsigned char r = 0;
unsigned char g = 0;
unsigned char b = 0;
unsigned char a = 0;
for (int i = 0; i < image->width*image->height; i++)
{
r = (unsigned char)(round(pixels[i].x*31.0f));
g = (unsigned char)(round(pixels[i].y*31.0f));
b = (unsigned char)(round(pixels[i].z*31.0f));
a = (pixels[i].w > ((float)ALPHA_THRESHOLD/255.0f))? 1 : 0;
((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 6 | (unsigned short)b << 1 | (unsigned short)a;
}
} break;
case UNCOMPRESSED_R4G4B4A4:
{
image->data = (unsigned short *)RL_MALLOC(image->width*image->height*sizeof(unsigned short));
unsigned char r = 0;
unsigned char g = 0;
unsigned char b = 0;
unsigned char a = 0;
for (int i = 0; i < image->width*image->height; i++)
{
r = (unsigned char)(round(pixels[i].x*15.0f));
g = (unsigned char)(round(pixels[i].y*15.0f));
b = (unsigned char)(round(pixels[i].z*15.0f));
a = (unsigned char)(round(pixels[i].w*15.0f));
((unsigned short *)image->data)[i] = (unsigned short)r << 12 | (unsigned short)g << 8 | (unsigned short)b << 4 | (unsigned short)a;
}
} break;
case UNCOMPRESSED_R8G8B8A8:
{
image->data = (unsigned char *)RL_MALLOC(image->width*image->height*4*sizeof(unsigned char));
for (int i = 0, k = 0; i < image->width*image->height*4; i += 4, k++)
{
((unsigned char *)image->data)[i] = (unsigned char)(pixels[k].x*255.0f);
((unsigned char *)image->data)[i + 1] = (unsigned char)(pixels[k].y*255.0f);
((unsigned char *)image->data)[i + 2] = (unsigned char)(pixels[k].z*255.0f);
((unsigned char *)image->data)[i + 3] = (unsigned char)(pixels[k].w*255.0f);
}
} break;
case UNCOMPRESSED_R32:
{
image->data = (float *)RL_MALLOC(image->width*image->height*sizeof(float));
for (int i = 0; i < image->width*image->height; i++)
{
((float *)image->data)[i] = (float)(pixels[i].x*0.299f + pixels[i].y*0.587f + pixels[i].z*0.114f);
}
} break;
case UNCOMPRESSED_R32G32B32:
{
image->data = (float *)RL_MALLOC(image->width*image->height*3*sizeof(float));
for (int i = 0, k = 0; i < image->width*image->height*3; i += 3, k++)
{
((float *)image->data)[i] = pixels[k].x;
((float *)image->data)[i + 1] = pixels[k].y;
((float *)image->data)[i + 2] = pixels[k].z;
}
} break;
case UNCOMPRESSED_R32G32B32A32:
{
image->data = (float *)RL_MALLOC(image->width*image->height*4*sizeof(float));
for (int i = 0, k = 0; i < image->width*image->height*4; i += 4, k++)
{
((float *)image->data)[i] = pixels[k].x;
((float *)image->data)[i + 1] = pixels[k].y;
((float *)image->data)[i + 2] = pixels[k].z;
((float *)image->data)[i + 3] = pixels[k].w;
}
} break;
default: break;
}
RL_FREE(pixels);
pixels = NULL;
if (image->mipmaps > 1)
{
image->mipmaps = 1;
#if defined(SUPPORT_IMAGE_MANIPULATION)
if (image->data != NULL) ImageMipmaps(image);
#endif
}
}
else TRACELOG(LOG_WARNING, "IMAGE: Data format is compressed, can not be converted");
}
}
void ImageAlphaMask(Image *image, Image alphaMask)
{
if ((image->width != alphaMask.width) || (image->height != alphaMask.height))
{
TRACELOG(LOG_WARNING, "IMAGE: Alpha mask must be same size as image");
}
else if (image->format >= COMPRESSED_DXT1_RGB)
{
TRACELOG(LOG_WARNING, "IMAGE: Alpha mask can not be applied to compressed data formats");
}
else
{
Image mask = ImageCopy(alphaMask);
if (mask.format != UNCOMPRESSED_GRAYSCALE) ImageFormat(&mask, UNCOMPRESSED_GRAYSCALE);
if (image->format == UNCOMPRESSED_GRAYSCALE)
{
unsigned char *data = (unsigned char *)RL_MALLOC(image->width*image->height*2);
for (int i = 0, k = 0; (i < mask.width*mask.height) || (i < image->width*image->height); i++, k += 2)
{
data[k] = ((unsigned char *)image->data)[i];
data[k + 1] = ((unsigned char *)mask.data)[i];
}
RL_FREE(image->data);
image->data = data;
image->format = UNCOMPRESSED_GRAY_ALPHA;
}
else
{
if (image->format != UNCOMPRESSED_R8G8B8A8) ImageFormat(image, UNCOMPRESSED_R8G8B8A8);
for (int i = 0, k = 3; (i < mask.width*mask.height) || (i < image->width*image->height); i++, k += 4)
{
((unsigned char *)image->data)[k] = ((unsigned char *)mask.data)[i];
}
}
UnloadImage(mask);
}
}
void ImageAlphaClear(Image *image, Color color, float threshold)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
Color *pixels = GetImageData(*image);
for (int i = 0; i < image->width*image->height; i++) if (pixels[i].a <= (unsigned char)(threshold*255.0f)) pixels[i] = color;
UnloadImage(*image);
int prevFormat = image->format;
*image = LoadImageEx(pixels, image->width, image->height);
ImageFormat(image, prevFormat);
RL_FREE(pixels);
}
void ImageAlphaPremultiply(Image *image)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
float alpha = 0.0f;
Color *pixels = GetImageData(*image);
for (int i = 0; i < image->width*image->height; i++)
{
alpha = (float)pixels[i].a/255.0f;
pixels[i].r = (unsigned char)((float)pixels[i].r*alpha);
pixels[i].g = (unsigned char)((float)pixels[i].g*alpha);
pixels[i].b = (unsigned char)((float)pixels[i].b*alpha);
}
UnloadImage(*image);
int prevFormat = image->format;
*image = LoadImageEx(pixels, image->width, image->height);
ImageFormat(image, prevFormat);
RL_FREE(pixels);
}
#if defined(SUPPORT_IMAGE_MANIPULATION)
TextureCubemap LoadTextureCubemap(Image image, int layoutType)
{
TextureCubemap cubemap = { 0 };
if (layoutType == CUBEMAP_AUTO_DETECT) 
{
if (image.width > image.height)
{
if ((image.width/6) == image.height) { layoutType = CUBEMAP_LINE_HORIZONTAL; cubemap.width = image.width/6; }
else if ((image.width/4) == (image.height/3)) { layoutType = CUBEMAP_CROSS_FOUR_BY_THREE; cubemap.width = image.width/4; }
else if (image.width >= (int)((float)image.height*1.85f)) { layoutType = CUBEMAP_PANORAMA; cubemap.width = image.width/4; }
}
else if (image.height > image.width)
{
if ((image.height/6) == image.width) { layoutType = CUBEMAP_LINE_VERTICAL; cubemap.width = image.height/6; }
else if ((image.width/3) == (image.height/4)) { layoutType = CUBEMAP_CROSS_THREE_BY_FOUR; cubemap.width = image.width/3; }
}
cubemap.height = cubemap.width;
}
if (layoutType != CUBEMAP_AUTO_DETECT)
{
int size = cubemap.width;
Image faces = { 0 }; 
Rectangle faceRecs[6] = { 0 }; 
for (int i = 0; i < 6; i++) faceRecs[i] = (Rectangle){ 0, 0, size, size };
if (layoutType == CUBEMAP_LINE_VERTICAL)
{
faces = image;
for (int i = 0; i < 6; i++) faceRecs[i].y = size*i;
}
else if (layoutType == CUBEMAP_PANORAMA)
{
}
else
{
if (layoutType == CUBEMAP_LINE_HORIZONTAL) for (int i = 0; i < 6; i++) faceRecs[i].x = size*i;
else if (layoutType == CUBEMAP_CROSS_THREE_BY_FOUR)
{
faceRecs[0].x = size; faceRecs[0].y = size;
faceRecs[1].x = size; faceRecs[1].y = 3*size;
faceRecs[2].x = size; faceRecs[2].y = 0;
faceRecs[3].x = size; faceRecs[3].y = 2*size;
faceRecs[4].x = 0; faceRecs[4].y = size;
faceRecs[5].x = 2*size; faceRecs[5].y = size;
}
else if (layoutType == CUBEMAP_CROSS_FOUR_BY_THREE)
{
faceRecs[0].x = 2*size; faceRecs[0].y = size;
faceRecs[1].x = 0; faceRecs[1].y = size;
faceRecs[2].x = size; faceRecs[2].y = 0;
faceRecs[3].x = size; faceRecs[3].y = 2*size;
faceRecs[4].x = size; faceRecs[4].y = size;
faceRecs[5].x = 3*size; faceRecs[5].y = size;
}
faces = GenImageColor(size, size*6, MAGENTA);
ImageFormat(&faces, image.format);
}
for (int i = 0; i < 6; i++) ImageDraw(&faces, image, faceRecs[i], (Rectangle){ 0, size*i, size, size }, WHITE);
cubemap.id = rlLoadTextureCubemap(faces.data, size, faces.format);
if (cubemap.id == 0) TRACELOG(LOG_WARNING, "IMAGE: Failed to load cubemap image");
UnloadImage(faces);
}
else TRACELOG(LOG_WARNING, "IMAGE: Failed to detect cubemap image layout");
return cubemap;
}
void ImageCrop(Image *image, Rectangle crop)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
if (crop.x < 0) { crop.width += crop.x; crop.x = 0; }
if (crop.y < 0) { crop.height += crop.y; crop.y = 0; }
if ((crop.x + crop.width) > image->width) crop.width = image->width - crop.x;
if ((crop.y + crop.height) > image->height) crop.height = image->height - crop.y;
if ((crop.x < image->width) && (crop.y < image->height))
{
Color *pixels = GetImageData(*image); 
Color *cropPixels = (Color *)RL_MALLOC((int)crop.width*(int)crop.height*sizeof(Color));
for (int j = (int)crop.y; j < (int)(crop.y + crop.height); j++)
{
for (int i = (int)crop.x; i < (int)(crop.x + crop.width); i++)
{
cropPixels[(j - (int)crop.y)*(int)crop.width + (i - (int)crop.x)] = pixels[j*image->width + i];
}
}
RL_FREE(pixels);
int format = image->format;
UnloadImage(*image);
*image = LoadImageEx(cropPixels, (int)crop.width, (int)crop.height);
RL_FREE(cropPixels);
ImageFormat(image, format);
}
else TRACELOG(LOG_WARNING, "IMAGE: Failed to crop, rectangle out of bounds");
}
void ImageAlphaCrop(Image *image, float threshold)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
Color *pixels = GetImageData(*image);
int xMin = 65536; 
int xMax = 0;
int yMin = 65536;
int yMax = 0;
for (int y = 0; y < image->height; y++)
{
for (int x = 0; x < image->width; x++)
{
if (pixels[y*image->width + x].a > (unsigned char)(threshold*255.0f))
{
if (x < xMin) xMin = x;
if (x > xMax) xMax = x;
if (y < yMin) yMin = y;
if (y > yMax) yMax = y;
}
}
}
Rectangle crop = { xMin, yMin, (xMax + 1) - xMin, (yMax + 1) - yMin };
RL_FREE(pixels);
if (!((xMax < xMin) || (yMax < yMin))) ImageCrop(image, crop);
}
void ImageResize(Image *image, int newWidth, int newHeight)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
Color *pixels = GetImageData(*image);
Color *output = (Color *)RL_MALLOC(newWidth*newHeight*sizeof(Color));
stbir_resize_uint8((unsigned char *)pixels, image->width, image->height, 0, (unsigned char *)output, newWidth, newHeight, 0, 4);
int format = image->format;
UnloadImage(*image);
*image = LoadImageEx(output, newWidth, newHeight);
ImageFormat(image, format); 
RL_FREE(output);
RL_FREE(pixels);
}
void ImageResizeNN(Image *image,int newWidth,int newHeight)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
Color *pixels = GetImageData(*image);
Color *output = (Color *)RL_MALLOC(newWidth*newHeight*sizeof(Color));
int xRatio = (int)((image->width << 16)/newWidth) + 1;
int yRatio = (int)((image->height << 16)/newHeight) + 1;
int x2, y2;
for (int y = 0; y < newHeight; y++)
{
for (int x = 0; x < newWidth; x++)
{
x2 = ((x*xRatio) >> 16);
y2 = ((y*yRatio) >> 16);
output[(y*newWidth) + x] = pixels[(y2*image->width) + x2] ;
}
}
int format = image->format;
UnloadImage(*image);
*image = LoadImageEx(output, newWidth, newHeight);
ImageFormat(image, format); 
RL_FREE(output);
RL_FREE(pixels);
}
void ImageResizeCanvas(Image *image, int newWidth, int newHeight, int offsetX, int offsetY, Color color)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
if ((newWidth != image->width) || (newHeight != image->height))
{
if (offsetX < 0)
{
ImageCrop(image, (Rectangle) { -offsetX, 0, image->width + offsetX, image->height });
offsetX = 0;
}
else if (offsetX > (newWidth - image->width))
{
ImageCrop(image, (Rectangle) { 0, 0, image->width - (offsetX - (newWidth - image->width)), image->height });
offsetX = newWidth - image->width;
}
if (offsetY < 0)
{
ImageCrop(image, (Rectangle) { 0, -offsetY, image->width, image->height + offsetY });
offsetY = 0;
}
else if (offsetY > (newHeight - image->height))
{
ImageCrop(image, (Rectangle) { 0, 0, image->width, image->height - (offsetY - (newHeight - image->height)) });
offsetY = newHeight - image->height;
}
if ((newWidth > image->width) && (newHeight > image->height))
{
Image imTemp = GenImageColor(newWidth, newHeight, color);
Rectangle srcRec = { 0.0f, 0.0f, (float)image->width, (float)image->height };
Rectangle dstRec = { (float)offsetX, (float)offsetY, srcRec.width, srcRec.height };
ImageDraw(&imTemp, *image, srcRec, dstRec, WHITE);
ImageFormat(&imTemp, image->format);
UnloadImage(*image);
*image = imTemp;
}
else if ((newWidth < image->width) && (newHeight < image->height))
{
Rectangle crop = { (float)offsetX, (float)offsetY, (float)newWidth, (float)newHeight };
ImageCrop(image, crop);
}
else 
{
Image imTemp = GenImageColor(newWidth, newHeight, color);
Rectangle srcRec = { 0.0f, 0.0f, (float)image->width, (float)image->height };
Rectangle dstRec = { (float)offsetX, (float)offsetY, (float)image->width, (float)image->height };
if (newWidth < image->width)
{
srcRec.x = offsetX;
srcRec.width = newWidth;
dstRec.x = 0.0f;
}
if (newHeight < image->height)
{
srcRec.y = offsetY;
srcRec.height = newHeight;
dstRec.y = 0.0f;
}
ImageDraw(&imTemp, *image, srcRec, dstRec, WHITE);
ImageFormat(&imTemp, image->format);
UnloadImage(*image);
*image = imTemp;
}
}
}
void ImageMipmaps(Image *image)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
int mipCount = 1; 
int mipWidth = image->width; 
int mipHeight = image->height; 
int mipSize = GetPixelDataSize(mipWidth, mipHeight, image->format); 
while ((mipWidth != 1) || (mipHeight != 1))
{
if (mipWidth != 1) mipWidth /= 2;
if (mipHeight != 1) mipHeight /= 2;
if (mipWidth < 1) mipWidth = 1;
if (mipHeight < 1) mipHeight = 1;
TRACELOGD("IMAGE: Next mipmap level: %i x %i - current size %i", mipWidth, mipHeight, mipSize);
mipCount++;
mipSize += GetPixelDataSize(mipWidth, mipHeight, image->format); 
}
if (image->mipmaps < mipCount)
{
void *temp = RL_REALLOC(image->data, mipSize);
if (temp != NULL) image->data = temp; 
else TRACELOG(LOG_WARNING, "IMAGE: Mipmaps required memory could not be allocated");
unsigned char *nextmip = (unsigned char *)image->data + GetPixelDataSize(image->width, image->height, image->format);
mipWidth = image->width/2;
mipHeight = image->height/2;
mipSize = GetPixelDataSize(mipWidth, mipHeight, image->format);
Image imCopy = ImageCopy(*image);
for (int i = 1; i < mipCount; i++)
{
TRACELOGD("IMAGE: Generating mipmap level: %i (%i x %i) - size: %i - offset: 0x%x", i, mipWidth, mipHeight, mipSize, nextmip);
ImageResize(&imCopy, mipWidth, mipHeight); 
memcpy(nextmip, imCopy.data, mipSize);
nextmip += mipSize;
image->mipmaps++;
mipWidth /= 2;
mipHeight /= 2;
if (mipWidth < 1) mipWidth = 1;
if (mipHeight < 1) mipHeight = 1;
mipSize = GetPixelDataSize(mipWidth, mipHeight, image->format);
}
UnloadImage(imCopy);
}
else TRACELOG(LOG_WARNING, "IMAGE: Mipmaps already available");
}
void ImageDither(Image *image, int rBpp, int gBpp, int bBpp, int aBpp)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
if (image->format >= COMPRESSED_DXT1_RGB)
{
TRACELOG(LOG_WARNING, "IMAGE: Compressed data formats can not be dithered");
return;
}
if ((rBpp + gBpp + bBpp + aBpp) > 16)
{
TRACELOG(LOG_WARNING, "IMAGE: Unsupported dithering bpps (%ibpp), only 16bpp or lower modes supported", (rBpp+gBpp+bBpp+aBpp));
}
else
{
Color *pixels = GetImageData(*image);
RL_FREE(image->data); 
if ((image->format != UNCOMPRESSED_R8G8B8) && (image->format != UNCOMPRESSED_R8G8B8A8))
{
TRACELOG(LOG_WARNING, "IMAGE: Format is already 16bpp or lower, dithering could have no effect");
}
if ((rBpp == 5) && (gBpp == 6) && (bBpp == 5) && (aBpp == 0)) image->format = UNCOMPRESSED_R5G6B5;
else if ((rBpp == 5) && (gBpp == 5) && (bBpp == 5) && (aBpp == 1)) image->format = UNCOMPRESSED_R5G5B5A1;
else if ((rBpp == 4) && (gBpp == 4) && (bBpp == 4) && (aBpp == 4)) image->format = UNCOMPRESSED_R4G4B4A4;
else
{
image->format = 0;
TRACELOG(LOG_WARNING, "IMAGE: Unsupported dithered OpenGL internal format: %ibpp (R%iG%iB%iA%i)", (rBpp+gBpp+bBpp+aBpp), rBpp, gBpp, bBpp, aBpp);
}
image->data = (unsigned short *)RL_MALLOC(image->width*image->height*sizeof(unsigned short));
Color oldPixel = WHITE;
Color newPixel = WHITE;
int rError, gError, bError;
unsigned short rPixel, gPixel, bPixel, aPixel; 
#define MIN(a,b) (((a)<(b))?(a):(b))
for (int y = 0; y < image->height; y++)
{
for (int x = 0; x < image->width; x++)
{
oldPixel = pixels[y*image->width + x];
newPixel.r = oldPixel.r >> (8 - rBpp); 
newPixel.g = oldPixel.g >> (8 - gBpp); 
newPixel.b = oldPixel.b >> (8 - bBpp); 
newPixel.a = oldPixel.a >> (8 - aBpp); 
rError = (int)oldPixel.r - (int)(newPixel.r << (8 - rBpp));
gError = (int)oldPixel.g - (int)(newPixel.g << (8 - gBpp));
bError = (int)oldPixel.b - (int)(newPixel.b << (8 - bBpp));
pixels[y*image->width + x] = newPixel;
if (x < (image->width - 1))
{
pixels[y*image->width + x+1].r = MIN((int)pixels[y*image->width + x+1].r + (int)((float)rError*7.0f/16), 0xff);
pixels[y*image->width + x+1].g = MIN((int)pixels[y*image->width + x+1].g + (int)((float)gError*7.0f/16), 0xff);
pixels[y*image->width + x+1].b = MIN((int)pixels[y*image->width + x+1].b + (int)((float)bError*7.0f/16), 0xff);
}
if ((x > 0) && (y < (image->height - 1)))
{
pixels[(y+1)*image->width + x-1].r = MIN((int)pixels[(y+1)*image->width + x-1].r + (int)((float)rError*3.0f/16), 0xff);
pixels[(y+1)*image->width + x-1].g = MIN((int)pixels[(y+1)*image->width + x-1].g + (int)((float)gError*3.0f/16), 0xff);
pixels[(y+1)*image->width + x-1].b = MIN((int)pixels[(y+1)*image->width + x-1].b + (int)((float)bError*3.0f/16), 0xff);
}
if (y < (image->height - 1))
{
pixels[(y+1)*image->width + x].r = MIN((int)pixels[(y+1)*image->width + x].r + (int)((float)rError*5.0f/16), 0xff);
pixels[(y+1)*image->width + x].g = MIN((int)pixels[(y+1)*image->width + x].g + (int)((float)gError*5.0f/16), 0xff);
pixels[(y+1)*image->width + x].b = MIN((int)pixels[(y+1)*image->width + x].b + (int)((float)bError*5.0f/16), 0xff);
}
if ((x < (image->width - 1)) && (y < (image->height - 1)))
{
pixels[(y+1)*image->width + x+1].r = MIN((int)pixels[(y+1)*image->width + x+1].r + (int)((float)rError*1.0f/16), 0xff);
pixels[(y+1)*image->width + x+1].g = MIN((int)pixels[(y+1)*image->width + x+1].g + (int)((float)gError*1.0f/16), 0xff);
pixels[(y+1)*image->width + x+1].b = MIN((int)pixels[(y+1)*image->width + x+1].b + (int)((float)bError*1.0f/16), 0xff);
}
rPixel = (unsigned short)newPixel.r;
gPixel = (unsigned short)newPixel.g;
bPixel = (unsigned short)newPixel.b;
aPixel = (unsigned short)newPixel.a;
((unsigned short *)image->data)[y*image->width + x] = (rPixel << (gBpp + bBpp + aBpp)) | (gPixel << (bBpp + aBpp)) | (bPixel << aBpp) | aPixel;
}
}
RL_FREE(pixels);
}
}
Color *ImageExtractPalette(Image image, int maxPaletteSize, int *extractCount)
{
#define COLOR_EQUAL(col1, col2) ((col1.r == col2.r)&&(col1.g == col2.g)&&(col1.b == col2.b)&&(col1.a == col2.a))
Color *pixels = GetImageData(image);
Color *palette = (Color *)RL_MALLOC(maxPaletteSize*sizeof(Color));
int palCount = 0;
for (int i = 0; i < maxPaletteSize; i++) palette[i] = BLANK; 
for (int i = 0; i < image.width*image.height; i++)
{
if (pixels[i].a > 0)
{
bool colorInPalette = false;
for (int j = 0; j < maxPaletteSize; j++)
{
if (COLOR_EQUAL(pixels[i], palette[j]))
{
colorInPalette = true;
break;
}
}
if (!colorInPalette)
{
palette[palCount] = pixels[i]; 
palCount++;
if (palCount >= maxPaletteSize)
{
i = image.width*image.height; 
TRACELOG(LOG_WARNING, "IMAGE: Palette is greater than %i colors", maxPaletteSize);
}
}
}
}
RL_FREE(pixels);
*extractCount = palCount;
return palette;
}
void ImageDraw(Image *dst, Image src, Rectangle srcRec, Rectangle dstRec, Color tint)
{
if ((dst->data == NULL) || (dst->width == 0) || (dst->height == 0) ||
(src.data == NULL) || (src.width == 0) || (src.height == 0)) return;
if (srcRec.x < 0) srcRec.x = 0;
if (srcRec.y < 0) srcRec.y = 0;
if ((srcRec.x + srcRec.width) > src.width)
{
srcRec.width = src.width - srcRec.x;
TRACELOG(LOG_WARNING, "IMAGE: Source rectangle width out of bounds, rescaled width: %i", srcRec.width);
}
if ((srcRec.y + srcRec.height) > src.height)
{
srcRec.height = src.height - srcRec.y;
TRACELOG(LOG_WARNING, "IMAGE: Source rectangle height out of bounds, rescaled height: %i", srcRec.height);
}
Image srcCopy = ImageCopy(src); 
if ((src.width != (int)srcRec.width) && (src.height != (int)srcRec.height)) ImageCrop(&srcCopy, srcRec);
if (((int)dstRec.width != (int)srcRec.width) || ((int)dstRec.height != (int)srcRec.height))
{
ImageResize(&srcCopy, (int)dstRec.width, (int)dstRec.height);
}
if (dstRec.x < 0)
{
ImageCrop(&srcCopy, (Rectangle) { -dstRec.x, 0, dstRec.width + dstRec.x, dstRec.height });
dstRec.width = dstRec.width + dstRec.x;
dstRec.x = 0;
}
if ((dstRec.x + dstRec.width) > dst->width)
{
ImageCrop(&srcCopy, (Rectangle) { 0, 0, dst->width - dstRec.x, dstRec.height });
dstRec.width = dst->width - dstRec.x;
}
if (dstRec.y < 0)
{
ImageCrop(&srcCopy, (Rectangle) { 0, -dstRec.y, dstRec.width, dstRec.height + dstRec.y });
dstRec.height = dstRec.height + dstRec.y;
dstRec.y = 0;
}
if ((dstRec.y + dstRec.height) > dst->height)
{
ImageCrop(&srcCopy, (Rectangle) { 0, 0, dstRec.width, dst->height - dstRec.y });
dstRec.height = dst->height - dstRec.y;
}
Color *dstPixels = GetImageData(*dst);
Color *srcPixels = GetImageData(srcCopy);
UnloadImage(srcCopy); 
Vector4 fsrc, fdst, fout; 
Vector4 ftint = ColorNormalize(tint); 
for (int j = (int)dstRec.y; j < (int)(dstRec.y + dstRec.height); j++)
{
for (int i = (int)dstRec.x; i < (int)(dstRec.x + dstRec.width); i++)
{
fdst = ColorNormalize(dstPixels[j*(int)dst->width + i]);
fsrc = ColorNormalize(srcPixels[(j - (int)dstRec.y)*(int)dstRec.width + (i - (int)dstRec.x)]);
fsrc.x *= ftint.x; fsrc.y *= ftint.y; fsrc.z *= ftint.z; fsrc.w *= ftint.w;
fout.w = fsrc.w + fdst.w*(1.0f - fsrc.w);
if (fout.w <= 0.0f)
{
fout.x = 0.0f;
fout.y = 0.0f;
fout.z = 0.0f;
}
else
{
fout.x = (fsrc.x*fsrc.w + fdst.x*fdst.w*(1 - fsrc.w))/fout.w;
fout.y = (fsrc.y*fsrc.w + fdst.y*fdst.w*(1 - fsrc.w))/fout.w;
fout.z = (fsrc.z*fsrc.w + fdst.z*fdst.w*(1 - fsrc.w))/fout.w;
}
dstPixels[j*(int)dst->width + i] = (Color){ (unsigned char)(fout.x*255.0f),
(unsigned char)(fout.y*255.0f),
(unsigned char)(fout.z*255.0f),
(unsigned char)(fout.w*255.0f) };
}
}
UnloadImage(*dst);
*dst = LoadImageEx(dstPixels, (int)dst->width, (int)dst->height);
ImageFormat(dst, dst->format);
RL_FREE(srcPixels);
RL_FREE(dstPixels);
}
Image ImageText(const char *text, int fontSize, Color color)
{
int defaultFontSize = 10; 
if (fontSize < defaultFontSize) fontSize = defaultFontSize;
int spacing = fontSize / defaultFontSize;
Image imText = ImageTextEx(GetFontDefault(), text, (float)fontSize, (float)spacing, color);
return imText;
}
Image ImageTextEx(Font font, const char *text, float fontSize, float spacing, Color tint)
{
int length = strlen(text);
int textOffsetX = 0; 
int textOffsetY = 0; 
Vector2 imSize = MeasureTextEx(font, text, (float)font.baseSize, spacing);
Image imText = GenImageColor((int)imSize.x, (int)imSize.y, BLANK);
for (int i = 0; i < length; i++)
{
int codepointByteCount = 0;
int codepoint = GetNextCodepoint(&text[i], &codepointByteCount);
int index = GetGlyphIndex(font, codepoint);
if (codepoint == 0x3f) codepointByteCount = 1;
if (codepoint == '\n')
{
textOffsetY += (font.baseSize + font.baseSize/2);
textOffsetX = 0.0f;
}
else
{
if ((codepoint != ' ') && (codepoint != '\t'))
{
Rectangle rec = { textOffsetX + font.chars[index].offsetX, textOffsetY + font.chars[index].offsetY, font.recs[index].width, font.recs[index].height };
ImageDraw(&imText, font.chars[index].image, (Rectangle){ 0, 0, font.chars[index].image.width, font.chars[index].image.height }, rec, tint);
}
if (font.chars[index].advanceX == 0) textOffsetX += (int)(font.recs[index].width + spacing);
else textOffsetX += font.chars[index].advanceX + (int)spacing;
}
i += (codepointByteCount - 1); 
}
if (fontSize > imSize.y)
{
float scaleFactor = fontSize/imSize.y;
TRACELOG(LOG_INFO, "IMAGE: Text scaled by factor: %f", scaleFactor);
if (font.texture.id == GetFontDefault().texture.id) ImageResizeNN(&imText, (int)(imSize.x*scaleFactor), (int)(imSize.y*scaleFactor));
else ImageResize(&imText, (int)(imSize.x*scaleFactor), (int)(imSize.y*scaleFactor));
}
return imText;
}
void ImageDrawRectangle(Image *dst, int posX, int posY, int width, int height, Color color)
{
ImageDrawRectangleRec(dst, (Rectangle){ posX, posY, width, height }, color);
}
void ImageDrawRectangleV(Image *dst, Vector2 position, Vector2 size, Color color)
{
ImageDrawRectangle(dst, position.x, position.y, size.x, size.y, color);
}
void ImageDrawRectangleRec(Image *dst, Rectangle rec, Color color)
{
if ((dst->data == NULL) || (dst->width == 0) || (dst->height == 0)) return;
Image imRec = GenImageColor((int)rec.width, (int)rec.height, color);
ImageDraw(dst, imRec, (Rectangle){ 0, 0, rec.width, rec.height }, rec, WHITE);
UnloadImage(imRec);
}
void ImageDrawRectangleLines(Image *dst, Rectangle rec, int thick, Color color)
{
ImageDrawRectangle(dst, rec.x, rec.y, rec.width, thick, color);
ImageDrawRectangle(dst, rec.x, rec.y + thick, thick, rec.height - thick*2, color);
ImageDrawRectangle(dst, rec.x + rec.width - thick, rec.y + thick, thick, rec.height - thick*2, color);
ImageDrawRectangle(dst, rec.x, rec.y + rec.height - thick, rec.width, thick, color);
}
void ImageClearBackground(Image *dst, Color color)
{
ImageDrawRectangle(dst, 0, 0, dst->width, dst->height, color);
}
void ImageDrawPixel(Image *dst, int x, int y, Color color)
{
ImageDrawRectangle(dst, x, y, 1, 1, color);
}
void ImageDrawPixelV(Image *dst, Vector2 position, Color color)
{
ImageDrawRectangle(dst, (int)position.x, (int)position.y, 1, 1, color);
}
void ImageDrawCircle(Image *dst, int centerX, int centerY, int radius, Color color)
{
int x = 0, y = radius;
int decesionParameter = 3 - 2*radius;
while (y >= x)
{
ImageDrawPixel(dst, centerX + x, centerY + y, color);
ImageDrawPixel(dst, centerX - x, centerY + y, color);
ImageDrawPixel(dst, centerX + x, centerY - y, color);
ImageDrawPixel(dst, centerX - x, centerY - y, color);
ImageDrawPixel(dst, centerX + y, centerY + x, color);
ImageDrawPixel(dst, centerX - y, centerY + x, color);
ImageDrawPixel(dst, centerX + y, centerY - x, color);
ImageDrawPixel(dst, centerX - y, centerY - x, color);
x++;
if (decesionParameter > 0)
{
y--;
decesionParameter = decesionParameter + 4*(x - y) + 10;
}
else decesionParameter = decesionParameter + 4*x + 6;
}
}
void ImageDrawCircleV(Image *dst, Vector2 center, int radius, Color color)
{
ImageDrawCircle(dst, (int)center.x, (int)center.y, radius, color);
}
void ImageDrawLine(Image *dst, int startPosX, int startPosY, int endPosX, int endPosY, Color color)
{
int m = 2*(endPosY - startPosY);
int slopeError = m - (startPosY - startPosX);
for (int x = startPosX, y = startPosY; x <= startPosY; x++)
{
ImageDrawPixel(dst, x, y, color);
slopeError += m;
if (slopeError >= 0)
{
y++;
slopeError -= 2*(startPosY - startPosX);
}
}
}
void ImageDrawLineV(Image *dst, Vector2 start, Vector2 end, Color color)
{
ImageDrawLine(dst, (int)start.x, (int)start.y, (int)end.x, (int)end.y, color);
}
void ImageDrawText(Image *dst, Vector2 position, const char *text, int fontSize, Color color)
{
ImageDrawTextEx(dst, position, GetFontDefault(), text, (float)fontSize, (float)fontSize/10, color);
}
void ImageDrawTextEx(Image *dst, Vector2 position, Font font, const char *text, float fontSize, float spacing, Color color)
{
Image imText = ImageTextEx(font, text, fontSize, spacing, color);
Rectangle srcRec = { 0.0f, 0.0f, (float)imText.width, (float)imText.height };
Rectangle dstRec = { position.x, position.y, (float)imText.width, (float)imText.height };
ImageDraw(dst, imText, srcRec, dstRec, WHITE);
UnloadImage(imText);
}
void ImageFlipVertical(Image *image)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
Color *srcPixels = GetImageData(*image);
Color *dstPixels = (Color *)RL_MALLOC(image->width*image->height*sizeof(Color));
for (int y = 0; y < image->height; y++)
{
for (int x = 0; x < image->width; x++)
{
dstPixels[y*image->width + x] = srcPixels[(image->height - 1 - y)*image->width + x];
}
}
Image processed = LoadImageEx(dstPixels, image->width, image->height);
ImageFormat(&processed, image->format);
UnloadImage(*image);
RL_FREE(srcPixels);
RL_FREE(dstPixels);
image->data = processed.data;
}
void ImageFlipHorizontal(Image *image)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
Color *srcPixels = GetImageData(*image);
Color *dstPixels = (Color *)RL_MALLOC(image->width*image->height*sizeof(Color));
for (int y = 0; y < image->height; y++)
{
for (int x = 0; x < image->width; x++)
{
dstPixels[y*image->width + x] = srcPixels[y*image->width + (image->width - 1 - x)];
}
}
Image processed = LoadImageEx(dstPixels, image->width, image->height);
ImageFormat(&processed, image->format);
UnloadImage(*image);
RL_FREE(srcPixels);
RL_FREE(dstPixels);
image->data = processed.data;
}
void ImageRotateCW(Image *image)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
Color *srcPixels = GetImageData(*image);
Color *rotPixels = (Color *)RL_MALLOC(image->width*image->height*sizeof(Color));
for (int y = 0; y < image->height; y++)
{
for (int x = 0; x < image->width; x++)
{
rotPixels[x*image->height + (image->height - y - 1)] = srcPixels[y*image->width + x];
}
}
Image processed = LoadImageEx(rotPixels, image->height, image->width);
ImageFormat(&processed, image->format);
UnloadImage(*image);
RL_FREE(srcPixels);
RL_FREE(rotPixels);
image->data = processed.data;
image->width = processed.width;
image->height = processed.height;
}
void ImageRotateCCW(Image *image)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
Color *srcPixels = GetImageData(*image);
Color *rotPixels = (Color *)RL_MALLOC(image->width*image->height*sizeof(Color));
for (int y = 0; y < image->height; y++)
{
for (int x = 0; x < image->width; x++)
{
rotPixels[x*image->height + y] = srcPixels[y*image->width + (image->width - x - 1)];
}
}
Image processed = LoadImageEx(rotPixels, image->height, image->width);
ImageFormat(&processed, image->format);
UnloadImage(*image);
RL_FREE(srcPixels);
RL_FREE(rotPixels);
image->data = processed.data;
image->width = processed.width;
image->height = processed.height;
}
void ImageColorTint(Image *image, Color color)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
Color *pixels = GetImageData(*image);
float cR = (float)color.r/255;
float cG = (float)color.g/255;
float cB = (float)color.b/255;
float cA = (float)color.a/255;
for (int y = 0; y < image->height; y++)
{
for (int x = 0; x < image->width; x++)
{
int index = y * image->width + x;
unsigned char r = 255*((float)pixels[index].r/255*cR);
unsigned char g = 255*((float)pixels[index].g/255*cG);
unsigned char b = 255*((float)pixels[index].b/255*cB);
unsigned char a = 255*((float)pixels[index].a/255*cA);
pixels[y*image->width + x].r = r;
pixels[y*image->width + x].g = g;
pixels[y*image->width + x].b = b;
pixels[y*image->width + x].a = a;
}
}
Image processed = LoadImageEx(pixels, image->width, image->height);
ImageFormat(&processed, image->format);
UnloadImage(*image);
RL_FREE(pixels);
image->data = processed.data;
}
void ImageColorInvert(Image *image)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
Color *pixels = GetImageData(*image);
for (int y = 0; y < image->height; y++)
{
for (int x = 0; x < image->width; x++)
{
pixels[y*image->width + x].r = 255 - pixels[y*image->width + x].r;
pixels[y*image->width + x].g = 255 - pixels[y*image->width + x].g;
pixels[y*image->width + x].b = 255 - pixels[y*image->width + x].b;
}
}
Image processed = LoadImageEx(pixels, image->width, image->height);
ImageFormat(&processed, image->format);
UnloadImage(*image);
RL_FREE(pixels);
image->data = processed.data;
}
void ImageColorGrayscale(Image *image)
{
ImageFormat(image, UNCOMPRESSED_GRAYSCALE);
}
void ImageColorContrast(Image *image, float contrast)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
if (contrast < -100) contrast = -100;
if (contrast > 100) contrast = 100;
contrast = (100.0f + contrast)/100.0f;
contrast *= contrast;
Color *pixels = GetImageData(*image);
for (int y = 0; y < image->height; y++)
{
for (int x = 0; x < image->width; x++)
{
float pR = (float)pixels[y*image->width + x].r/255.0f;
pR -= 0.5;
pR *= contrast;
pR += 0.5;
pR *= 255;
if (pR < 0) pR = 0;
if (pR > 255) pR = 255;
float pG = (float)pixels[y*image->width + x].g/255.0f;
pG -= 0.5;
pG *= contrast;
pG += 0.5;
pG *= 255;
if (pG < 0) pG = 0;
if (pG > 255) pG = 255;
float pB = (float)pixels[y*image->width + x].b/255.0f;
pB -= 0.5;
pB *= contrast;
pB += 0.5;
pB *= 255;
if (pB < 0) pB = 0;
if (pB > 255) pB = 255;
pixels[y*image->width + x].r = (unsigned char)pR;
pixels[y*image->width + x].g = (unsigned char)pG;
pixels[y*image->width + x].b = (unsigned char)pB;
}
}
Image processed = LoadImageEx(pixels, image->width, image->height);
ImageFormat(&processed, image->format);
UnloadImage(*image);
RL_FREE(pixels);
image->data = processed.data;
}
void ImageColorBrightness(Image *image, int brightness)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
if (brightness < -255) brightness = -255;
if (brightness > 255) brightness = 255;
Color *pixels = GetImageData(*image);
for (int y = 0; y < image->height; y++)
{
for (int x = 0; x < image->width; x++)
{
int cR = pixels[y*image->width + x].r + brightness;
int cG = pixels[y*image->width + x].g + brightness;
int cB = pixels[y*image->width + x].b + brightness;
if (cR < 0) cR = 1;
if (cR > 255) cR = 255;
if (cG < 0) cG = 1;
if (cG > 255) cG = 255;
if (cB < 0) cB = 1;
if (cB > 255) cB = 255;
pixels[y*image->width + x].r = (unsigned char)cR;
pixels[y*image->width + x].g = (unsigned char)cG;
pixels[y*image->width + x].b = (unsigned char)cB;
}
}
Image processed = LoadImageEx(pixels, image->width, image->height);
ImageFormat(&processed, image->format);
UnloadImage(*image);
RL_FREE(pixels);
image->data = processed.data;
}
void ImageColorReplace(Image *image, Color color, Color replace)
{
if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
Color *pixels = GetImageData(*image);
for (int y = 0; y < image->height; y++)
{
for (int x = 0; x < image->width; x++)
{
if ((pixels[y*image->width + x].r == color.r) &&
(pixels[y*image->width + x].g == color.g) &&
(pixels[y*image->width + x].b == color.b) &&
(pixels[y*image->width + x].a == color.a))
{
pixels[y*image->width + x].r = replace.r;
pixels[y*image->width + x].g = replace.g;
pixels[y*image->width + x].b = replace.b;
pixels[y*image->width + x].a = replace.a;
}
}
}
Image processed = LoadImageEx(pixels, image->width, image->height);
ImageFormat(&processed, image->format);
UnloadImage(*image);
RL_FREE(pixels);
image->data = processed.data;
}
#endif 
Image GenImageColor(int width, int height, Color color)
{
Color *pixels = (Color *)RL_CALLOC(width*height, sizeof(Color));
for (int i = 0; i < width*height; i++) pixels[i] = color;
Image image = LoadImageEx(pixels, width, height);
RL_FREE(pixels);
return image;
}
#if defined(SUPPORT_IMAGE_GENERATION)
Image GenImageGradientV(int width, int height, Color top, Color bottom)
{
Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));
for (int j = 0; j < height; j++)
{
float factor = (float)j/(float)height;
for (int i = 0; i < width; i++)
{
pixels[j*width + i].r = (int)((float)bottom.r*factor + (float)top.r*(1.f - factor));
pixels[j*width + i].g = (int)((float)bottom.g*factor + (float)top.g*(1.f - factor));
pixels[j*width + i].b = (int)((float)bottom.b*factor + (float)top.b*(1.f - factor));
pixels[j*width + i].a = (int)((float)bottom.a*factor + (float)top.a*(1.f - factor));
}
}
Image image = LoadImageEx(pixels, width, height);
RL_FREE(pixels);
return image;
}
Image GenImageGradientH(int width, int height, Color left, Color right)
{
Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));
for (int i = 0; i < width; i++)
{
float factor = (float)i/(float)width;
for (int j = 0; j < height; j++)
{
pixels[j*width + i].r = (int)((float)right.r*factor + (float)left.r*(1.f - factor));
pixels[j*width + i].g = (int)((float)right.g*factor + (float)left.g*(1.f - factor));
pixels[j*width + i].b = (int)((float)right.b*factor + (float)left.b*(1.f - factor));
pixels[j*width + i].a = (int)((float)right.a*factor + (float)left.a*(1.f - factor));
}
}
Image image = LoadImageEx(pixels, width, height);
RL_FREE(pixels);
return image;
}
Image GenImageGradientRadial(int width, int height, float density, Color inner, Color outer)
{
Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));
float radius = (width < height)? (float)width/2.0f : (float)height/2.0f;
float centerX = (float)width/2.0f;
float centerY = (float)height/2.0f;
for (int y = 0; y < height; y++)
{
for (int x = 0; x < width; x++)
{
float dist = hypotf((float)x - centerX, (float)y - centerY);
float factor = (dist - radius*density)/(radius*(1.0f - density));
factor = (float)fmax(factor, 0.f);
factor = (float)fmin(factor, 1.f); 
pixels[y*width + x].r = (int)((float)outer.r*factor + (float)inner.r*(1.0f - factor));
pixels[y*width + x].g = (int)((float)outer.g*factor + (float)inner.g*(1.0f - factor));
pixels[y*width + x].b = (int)((float)outer.b*factor + (float)inner.b*(1.0f - factor));
pixels[y*width + x].a = (int)((float)outer.a*factor + (float)inner.a*(1.0f - factor));
}
}
Image image = LoadImageEx(pixels, width, height);
RL_FREE(pixels);
return image;
}
Image GenImageChecked(int width, int height, int checksX, int checksY, Color col1, Color col2)
{
Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));
for (int y = 0; y < height; y++)
{
for (int x = 0; x < width; x++)
{
if ((x/checksX + y/checksY)%2 == 0) pixels[y*width + x] = col1;
else pixels[y*width + x] = col2;
}
}
Image image = LoadImageEx(pixels, width, height);
RL_FREE(pixels);
return image;
}
Image GenImageWhiteNoise(int width, int height, float factor)
{
Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));
for (int i = 0; i < width*height; i++)
{
if (GetRandomValue(0, 99) < (int)(factor*100.0f)) pixels[i] = WHITE;
else pixels[i] = BLACK;
}
Image image = LoadImageEx(pixels, width, height);
RL_FREE(pixels);
return image;
}
Image GenImagePerlinNoise(int width, int height, int offsetX, int offsetY, float scale)
{
Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));
for (int y = 0; y < height; y++)
{
for (int x = 0; x < width; x++)
{
float nx = (float)(x + offsetX)*scale/(float)width;
float ny = (float)(y + offsetY)*scale/(float)height;
float p = (stb_perlin_fbm_noise3(nx, ny, 1.0f, 2.0f, 0.5f, 6) + 1.0f)/2.0f;
int intensity = (int)(p*255.0f);
pixels[y*width + x] = (Color){intensity, intensity, intensity, 255};
}
}
Image image = LoadImageEx(pixels, width, height);
RL_FREE(pixels);
return image;
}
Image GenImageCellular(int width, int height, int tileSize)
{
Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));
int seedsPerRow = width/tileSize;
int seedsPerCol = height/tileSize;
int seedsCount = seedsPerRow * seedsPerCol;
Vector2 *seeds = (Vector2 *)RL_MALLOC(seedsCount*sizeof(Vector2));
for (int i = 0; i < seedsCount; i++)
{
int y = (i/seedsPerRow)*tileSize + GetRandomValue(0, tileSize - 1);
int x = (i%seedsPerRow)*tileSize + GetRandomValue(0, tileSize - 1);
seeds[i] = (Vector2){ (float)x, (float)y};
}
for (int y = 0; y < height; y++)
{
int tileY = y/tileSize;
for (int x = 0; x < width; x++)
{
int tileX = x/tileSize;
float minDistance = (float)strtod("Inf", NULL);
for (int i = -1; i < 2; i++)
{
if ((tileX + i < 0) || (tileX + i >= seedsPerRow)) continue;
for (int j = -1; j < 2; j++)
{
if ((tileY + j < 0) || (tileY + j >= seedsPerCol)) continue;
Vector2 neighborSeed = seeds[(tileY + j)*seedsPerRow + tileX + i];
float dist = (float)hypot(x - (int)neighborSeed.x, y - (int)neighborSeed.y);
minDistance = (float)fmin(minDistance, dist);
}
}
int intensity = (int)(minDistance*256.0f/tileSize);
if (intensity > 255) intensity = 255;
pixels[y*width + x] = (Color){ intensity, intensity, intensity, 255 };
}
}
RL_FREE(seeds);
Image image = LoadImageEx(pixels, width, height);
RL_FREE(pixels);
return image;
}
#endif 
void GenTextureMipmaps(Texture2D *texture)
{
rlGenerateMipmaps(texture);
}
void SetTextureFilter(Texture2D texture, int filterMode)
{
switch (filterMode)
{
case FILTER_POINT:
{
if (texture.mipmaps > 1)
{
rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_FILTER_MIP_NEAREST);
rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_FILTER_NEAREST);
}
else
{
rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_FILTER_NEAREST);
rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_FILTER_NEAREST);
}
} break;
case FILTER_BILINEAR:
{
if (texture.mipmaps > 1)
{
rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_FILTER_LINEAR_MIP_NEAREST);
rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_FILTER_LINEAR);
}
else
{
rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_FILTER_LINEAR);
rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_FILTER_LINEAR);
}
} break;
case FILTER_TRILINEAR:
{
if (texture.mipmaps > 1)
{
rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_FILTER_MIP_LINEAR);
rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_FILTER_LINEAR);
}
else
{
TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] No mipmaps available for TRILINEAR texture filtering", texture.id);
rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_FILTER_LINEAR);
rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_FILTER_LINEAR);
}
} break;
case FILTER_ANISOTROPIC_4X: rlTextureParameters(texture.id, RL_TEXTURE_ANISOTROPIC_FILTER, 4); break;
case FILTER_ANISOTROPIC_8X: rlTextureParameters(texture.id, RL_TEXTURE_ANISOTROPIC_FILTER, 8); break;
case FILTER_ANISOTROPIC_16X: rlTextureParameters(texture.id, RL_TEXTURE_ANISOTROPIC_FILTER, 16); break;
default: break;
}
}
void SetTextureWrap(Texture2D texture, int wrapMode)
{
switch (wrapMode)
{
case WRAP_REPEAT:
{
rlTextureParameters(texture.id, RL_TEXTURE_WRAP_S, RL_WRAP_REPEAT);
rlTextureParameters(texture.id, RL_TEXTURE_WRAP_T, RL_WRAP_REPEAT);
} break;
case WRAP_CLAMP:
{
rlTextureParameters(texture.id, RL_TEXTURE_WRAP_S, RL_WRAP_CLAMP);
rlTextureParameters(texture.id, RL_TEXTURE_WRAP_T, RL_WRAP_CLAMP);
} break;
case WRAP_MIRROR_REPEAT:
{
rlTextureParameters(texture.id, RL_TEXTURE_WRAP_S, RL_WRAP_MIRROR_REPEAT);
rlTextureParameters(texture.id, RL_TEXTURE_WRAP_T, RL_WRAP_MIRROR_REPEAT);
} break;
case WRAP_MIRROR_CLAMP:
{
rlTextureParameters(texture.id, RL_TEXTURE_WRAP_S, RL_WRAP_MIRROR_CLAMP);
rlTextureParameters(texture.id, RL_TEXTURE_WRAP_T, RL_WRAP_MIRROR_CLAMP);
} break;
default: break;
}
}
void DrawTexture(Texture2D texture, int posX, int posY, Color tint)
{
DrawTextureEx(texture, (Vector2){ (float)posX, (float)posY }, 0.0f, 1.0f, tint);
}
void DrawTextureV(Texture2D texture, Vector2 position, Color tint)
{
DrawTextureEx(texture, position, 0, 1.0f, tint);
}
void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint)
{
Rectangle sourceRec = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
Rectangle destRec = { position.x, position.y, (float)texture.width*scale, (float)texture.height*scale };
Vector2 origin = { 0.0f, 0.0f };
DrawTexturePro(texture, sourceRec, destRec, origin, rotation, tint);
}
void DrawTextureRec(Texture2D texture, Rectangle sourceRec, Vector2 position, Color tint)
{
Rectangle destRec = { position.x, position.y, fabsf(sourceRec.width), fabsf(sourceRec.height) };
Vector2 origin = { 0.0f, 0.0f };
DrawTexturePro(texture, sourceRec, destRec, origin, 0.0f, tint);
}
void DrawTextureQuad(Texture2D texture, Vector2 tiling, Vector2 offset, Rectangle quad, Color tint)
{
Rectangle source = { offset.x*texture.width, offset.y*texture.height, tiling.x*texture.width, tiling.y*texture.height };
Vector2 origin = { 0.0f, 0.0f };
DrawTexturePro(texture, source, quad, origin, 0.0f, tint);
}
void DrawTexturePro(Texture2D texture, Rectangle sourceRec, Rectangle destRec, Vector2 origin, float rotation, Color tint)
{
if (texture.id > 0)
{
float width = (float)texture.width;
float height = (float)texture.height;
bool flipX = false;
if (sourceRec.width < 0) { flipX = true; sourceRec.width *= -1; }
if (sourceRec.height < 0) sourceRec.y -= sourceRec.height;
rlEnableTexture(texture.id);
rlPushMatrix();
rlTranslatef(destRec.x, destRec.y, 0.0f);
rlRotatef(rotation, 0.0f, 0.0f, 1.0f);
rlTranslatef(-origin.x, -origin.y, 0.0f);
rlBegin(RL_QUADS);
rlColor4ub(tint.r, tint.g, tint.b, tint.a);
rlNormal3f(0.0f, 0.0f, 1.0f); 
if (flipX) rlTexCoord2f((sourceRec.x + sourceRec.width)/width, sourceRec.y/height);
else rlTexCoord2f(sourceRec.x/width, sourceRec.y/height);
rlVertex2f(0.0f, 0.0f);
if (flipX) rlTexCoord2f((sourceRec.x + sourceRec.width)/width, (sourceRec.y + sourceRec.height)/height);
else rlTexCoord2f(sourceRec.x/width, (sourceRec.y + sourceRec.height)/height);
rlVertex2f(0.0f, destRec.height);
if (flipX) rlTexCoord2f(sourceRec.x/width, (sourceRec.y + sourceRec.height)/height);
else rlTexCoord2f((sourceRec.x + sourceRec.width)/width, (sourceRec.y + sourceRec.height)/height);
rlVertex2f(destRec.width, destRec.height);
if (flipX) rlTexCoord2f(sourceRec.x/width, sourceRec.y/height);
else rlTexCoord2f((sourceRec.x + sourceRec.width)/width, sourceRec.y/height);
rlVertex2f(destRec.width, 0.0f);
rlEnd();
rlPopMatrix();
rlDisableTexture();
}
}
void DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo, Rectangle destRec, Vector2 origin, float rotation, Color tint)
{
if (texture.id > 0)
{
float width = (float)texture.width;
float height = (float)texture.height;
float patchWidth = (destRec.width <= 0.0f)? 0.0f : destRec.width;
float patchHeight = (destRec.height <= 0.0f)? 0.0f : destRec.height;
if (nPatchInfo.sourceRec.width < 0) nPatchInfo.sourceRec.x -= nPatchInfo.sourceRec.width;
if (nPatchInfo.sourceRec.height < 0) nPatchInfo.sourceRec.y -= nPatchInfo.sourceRec.height;
if (nPatchInfo.type == NPT_3PATCH_HORIZONTAL) patchHeight = nPatchInfo.sourceRec.height;
if (nPatchInfo.type == NPT_3PATCH_VERTICAL) patchWidth = nPatchInfo.sourceRec.width;
bool drawCenter = true;
bool drawMiddle = true;
float leftBorder = (float)nPatchInfo.left;
float topBorder = (float)nPatchInfo.top;
float rightBorder = (float)nPatchInfo.right;
float bottomBorder = (float)nPatchInfo.bottom;
if (patchWidth <= (leftBorder + rightBorder) && nPatchInfo.type != NPT_3PATCH_VERTICAL)
{
drawCenter = false;
leftBorder = (leftBorder / (leftBorder + rightBorder)) * patchWidth;
rightBorder = patchWidth - leftBorder;
}
if (patchHeight <= (topBorder + bottomBorder) && nPatchInfo.type != NPT_3PATCH_HORIZONTAL)
{
drawMiddle = false;
topBorder = (topBorder / (topBorder + bottomBorder)) * patchHeight;
bottomBorder = patchHeight - topBorder;
}
Vector2 vertA, vertB, vertC, vertD;
vertA.x = 0.0f; 
vertA.y = 0.0f; 
vertB.x = leftBorder; 
vertB.y = topBorder; 
vertC.x = patchWidth - rightBorder; 
vertC.y = patchHeight - bottomBorder; 
vertD.x = patchWidth; 
vertD.y = patchHeight; 
Vector2 coordA, coordB, coordC, coordD;
coordA.x = nPatchInfo.sourceRec.x / width;
coordA.y = nPatchInfo.sourceRec.y / height;
coordB.x = (nPatchInfo.sourceRec.x + leftBorder) / width;
coordB.y = (nPatchInfo.sourceRec.y + topBorder) / height;
coordC.x = (nPatchInfo.sourceRec.x + nPatchInfo.sourceRec.width - rightBorder) / width;
coordC.y = (nPatchInfo.sourceRec.y + nPatchInfo.sourceRec.height - bottomBorder) / height;
coordD.x = (nPatchInfo.sourceRec.x + nPatchInfo.sourceRec.width) / width;
coordD.y = (nPatchInfo.sourceRec.y + nPatchInfo.sourceRec.height) / height;
rlEnableTexture(texture.id);
rlPushMatrix();
rlTranslatef(destRec.x, destRec.y, 0.0f);
rlRotatef(rotation, 0.0f, 0.0f, 1.0f);
rlTranslatef(-origin.x, -origin.y, 0.0f);
rlBegin(RL_QUADS);
rlColor4ub(tint.r, tint.g, tint.b, tint.a);
rlNormal3f(0.0f, 0.0f, 1.0f); 
if (nPatchInfo.type == NPT_9PATCH)
{
rlTexCoord2f(coordA.x, coordB.y); rlVertex2f(vertA.x, vertB.y); 
rlTexCoord2f(coordB.x, coordB.y); rlVertex2f(vertB.x, vertB.y); 
rlTexCoord2f(coordB.x, coordA.y); rlVertex2f(vertB.x, vertA.y); 
rlTexCoord2f(coordA.x, coordA.y); rlVertex2f(vertA.x, vertA.y); 
if (drawCenter)
{
rlTexCoord2f(coordB.x, coordB.y); rlVertex2f(vertB.x, vertB.y); 
rlTexCoord2f(coordC.x, coordB.y); rlVertex2f(vertC.x, vertB.y); 
rlTexCoord2f(coordC.x, coordA.y); rlVertex2f(vertC.x, vertA.y); 
rlTexCoord2f(coordB.x, coordA.y); rlVertex2f(vertB.x, vertA.y); 
}
rlTexCoord2f(coordC.x, coordB.y); rlVertex2f(vertC.x, vertB.y); 
rlTexCoord2f(coordD.x, coordB.y); rlVertex2f(vertD.x, vertB.y); 
rlTexCoord2f(coordD.x, coordA.y); rlVertex2f(vertD.x, vertA.y); 
rlTexCoord2f(coordC.x, coordA.y); rlVertex2f(vertC.x, vertA.y); 
if (drawMiddle)
{
rlTexCoord2f(coordA.x, coordC.y); rlVertex2f(vertA.x, vertC.y); 
rlTexCoord2f(coordB.x, coordC.y); rlVertex2f(vertB.x, vertC.y); 
rlTexCoord2f(coordB.x, coordB.y); rlVertex2f(vertB.x, vertB.y); 
rlTexCoord2f(coordA.x, coordB.y); rlVertex2f(vertA.x, vertB.y); 
if (drawCenter)
{
rlTexCoord2f(coordB.x, coordC.y); rlVertex2f(vertB.x, vertC.y); 
rlTexCoord2f(coordC.x, coordC.y); rlVertex2f(vertC.x, vertC.y); 
rlTexCoord2f(coordC.x, coordB.y); rlVertex2f(vertC.x, vertB.y); 
rlTexCoord2f(coordB.x, coordB.y); rlVertex2f(vertB.x, vertB.y); 
}
rlTexCoord2f(coordC.x, coordC.y); rlVertex2f(vertC.x, vertC.y); 
rlTexCoord2f(coordD.x, coordC.y); rlVertex2f(vertD.x, vertC.y); 
rlTexCoord2f(coordD.x, coordB.y); rlVertex2f(vertD.x, vertB.y); 
rlTexCoord2f(coordC.x, coordB.y); rlVertex2f(vertC.x, vertB.y); 
}
rlTexCoord2f(coordA.x, coordD.y); rlVertex2f(vertA.x, vertD.y); 
rlTexCoord2f(coordB.x, coordD.y); rlVertex2f(vertB.x, vertD.y); 
rlTexCoord2f(coordB.x, coordC.y); rlVertex2f(vertB.x, vertC.y); 
rlTexCoord2f(coordA.x, coordC.y); rlVertex2f(vertA.x, vertC.y); 
if (drawCenter)
{
rlTexCoord2f(coordB.x, coordD.y); rlVertex2f(vertB.x, vertD.y); 
rlTexCoord2f(coordC.x, coordD.y); rlVertex2f(vertC.x, vertD.y); 
rlTexCoord2f(coordC.x, coordC.y); rlVertex2f(vertC.x, vertC.y); 
rlTexCoord2f(coordB.x, coordC.y); rlVertex2f(vertB.x, vertC.y); 
}
rlTexCoord2f(coordC.x, coordD.y); rlVertex2f(vertC.x, vertD.y); 
rlTexCoord2f(coordD.x, coordD.y); rlVertex2f(vertD.x, vertD.y); 
rlTexCoord2f(coordD.x, coordC.y); rlVertex2f(vertD.x, vertC.y); 
rlTexCoord2f(coordC.x, coordC.y); rlVertex2f(vertC.x, vertC.y); 
}
else if (nPatchInfo.type == NPT_3PATCH_VERTICAL)
{
rlTexCoord2f(coordA.x, coordB.y); rlVertex2f(vertA.x, vertB.y); 
rlTexCoord2f(coordD.x, coordB.y); rlVertex2f(vertD.x, vertB.y); 
rlTexCoord2f(coordD.x, coordA.y); rlVertex2f(vertD.x, vertA.y); 
rlTexCoord2f(coordA.x, coordA.y); rlVertex2f(vertA.x, vertA.y); 
if (drawCenter)
{
rlTexCoord2f(coordA.x, coordC.y); rlVertex2f(vertA.x, vertC.y); 
rlTexCoord2f(coordD.x, coordC.y); rlVertex2f(vertD.x, vertC.y); 
rlTexCoord2f(coordD.x, coordB.y); rlVertex2f(vertD.x, vertB.y); 
rlTexCoord2f(coordA.x, coordB.y); rlVertex2f(vertA.x, vertB.y); 
}
rlTexCoord2f(coordA.x, coordD.y); rlVertex2f(vertA.x, vertD.y); 
rlTexCoord2f(coordD.x, coordD.y); rlVertex2f(vertD.x, vertD.y); 
rlTexCoord2f(coordD.x, coordC.y); rlVertex2f(vertD.x, vertC.y); 
rlTexCoord2f(coordA.x, coordC.y); rlVertex2f(vertA.x, vertC.y); 
}
else if (nPatchInfo.type == NPT_3PATCH_HORIZONTAL)
{
rlTexCoord2f(coordA.x, coordD.y); rlVertex2f(vertA.x, vertD.y); 
rlTexCoord2f(coordB.x, coordD.y); rlVertex2f(vertB.x, vertD.y); 
rlTexCoord2f(coordB.x, coordA.y); rlVertex2f(vertB.x, vertA.y); 
rlTexCoord2f(coordA.x, coordA.y); rlVertex2f(vertA.x, vertA.y); 
if (drawCenter)
{
rlTexCoord2f(coordB.x, coordD.y); rlVertex2f(vertB.x, vertD.y); 
rlTexCoord2f(coordC.x, coordD.y); rlVertex2f(vertC.x, vertD.y); 
rlTexCoord2f(coordC.x, coordA.y); rlVertex2f(vertC.x, vertA.y); 
rlTexCoord2f(coordB.x, coordA.y); rlVertex2f(vertB.x, vertA.y); 
}
rlTexCoord2f(coordC.x, coordD.y); rlVertex2f(vertC.x, vertD.y); 
rlTexCoord2f(coordD.x, coordD.y); rlVertex2f(vertD.x, vertD.y); 
rlTexCoord2f(coordD.x, coordA.y); rlVertex2f(vertD.x, vertA.y); 
rlTexCoord2f(coordC.x, coordA.y); rlVertex2f(vertC.x, vertA.y); 
}
rlEnd();
rlPopMatrix();
rlDisableTexture();
}
}
#if defined(SUPPORT_FILEFORMAT_GIF)
static Image LoadAnimatedGIF(const char *fileName, int *frames, int **delays)
{
Image image = { 0 };
unsigned int dataSize = 0;
unsigned char *fileData = LoadFileData(fileName, &dataSize);
if (fileData != NULL)
{
int comp = 0;
image.data = stbi_load_gif_from_memory(fileData, dataSize, delays, &image.width, &image.height, frames, &comp, 4);
image.mipmaps = 1;
image.format = UNCOMPRESSED_R8G8B8A8;
RL_FREE(fileData);
}
return image;
}
#endif
#if defined(SUPPORT_FILEFORMAT_DDS)
static Image LoadDDS(const char *fileName)
{
#define FOURCC_DXT1 0x31545844 
#define FOURCC_DXT3 0x33545844 
#define FOURCC_DXT5 0x35545844 
typedef struct {
unsigned int size;
unsigned int flags;
unsigned int fourCC;
unsigned int rgbBitCount;
unsigned int rBitMask;
unsigned int gBitMask;
unsigned int bBitMask;
unsigned int aBitMask;
} DDSPixelFormat;
typedef struct {
unsigned int size;
unsigned int flags;
unsigned int height;
unsigned int width;
unsigned int pitchOrLinearSize;
unsigned int depth;
unsigned int mipmapCount;
unsigned int reserved1[11];
DDSPixelFormat ddspf;
unsigned int caps;
unsigned int caps2;
unsigned int caps3;
unsigned int caps4;
unsigned int reserved2;
} DDSHeader;
Image image = { 0 };
FILE *ddsFile = fopen(fileName, "rb");
if (ddsFile == NULL)
{
TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open DDS file", fileName);
}
else
{
char ddsHeaderId[4] = { 0 };
fread(ddsHeaderId, 4, 1, ddsFile);
if ((ddsHeaderId[0] != 'D') || (ddsHeaderId[1] != 'D') || (ddsHeaderId[2] != 'S') || (ddsHeaderId[3] != ' '))
{
TRACELOG(LOG_WARNING, "IMAGE: [%s] DDS file not a valid image", fileName);
}
else
{
DDSHeader ddsHeader = { 0 };
fread(&ddsHeader, sizeof(DDSHeader), 1, ddsFile);
TRACELOGD("IMAGE: [%s] DDS file info:", fileName);
TRACELOGD(" > Header size: %i", fileName, sizeof(DDSHeader));
TRACELOGD(" > Pixel format size: %i", fileName, ddsHeader.ddspf.size);
TRACELOGD(" > Pixel format flags: 0x%x", fileName, ddsHeader.ddspf.flags);
TRACELOGD(" > File format: 0x%x", fileName, ddsHeader.ddspf.fourCC);
TRACELOGD(" > File bit count: 0x%x", fileName, ddsHeader.ddspf.rgbBitCount);
image.width = ddsHeader.width;
image.height = ddsHeader.height;
if (ddsHeader.mipmapCount == 0) image.mipmaps = 1; 
else image.mipmaps = ddsHeader.mipmapCount;
if (ddsHeader.ddspf.rgbBitCount == 16) 
{
if (ddsHeader.ddspf.flags == 0x40) 
{
image.data = (unsigned short *)RL_MALLOC(image.width*image.height*sizeof(unsigned short));
fread(image.data, image.width*image.height*sizeof(unsigned short), 1, ddsFile);
image.format = UNCOMPRESSED_R5G6B5;
}
else if (ddsHeader.ddspf.flags == 0x41) 
{
if (ddsHeader.ddspf.aBitMask == 0x8000) 
{
image.data = (unsigned short *)RL_MALLOC(image.width*image.height*sizeof(unsigned short));
fread(image.data, image.width*image.height*sizeof(unsigned short), 1, ddsFile);
unsigned char alpha = 0;
for (int i = 0; i < image.width*image.height; i++)
{
alpha = ((unsigned short *)image.data)[i] >> 15;
((unsigned short *)image.data)[i] = ((unsigned short *)image.data)[i] << 1;
((unsigned short *)image.data)[i] += alpha;
}
image.format = UNCOMPRESSED_R5G5B5A1;
}
else if (ddsHeader.ddspf.aBitMask == 0xf000) 
{
image.data = (unsigned short *)RL_MALLOC(image.width*image.height*sizeof(unsigned short));
fread(image.data, image.width*image.height*sizeof(unsigned short), 1, ddsFile);
unsigned char alpha = 0;
for (int i = 0; i < image.width*image.height; i++)
{
alpha = ((unsigned short *)image.data)[i] >> 12;
((unsigned short *)image.data)[i] = ((unsigned short *)image.data)[i] << 4;
((unsigned short *)image.data)[i] += alpha;
}
image.format = UNCOMPRESSED_R4G4B4A4;
}
}
}
else if (ddsHeader.ddspf.flags == 0x40 && ddsHeader.ddspf.rgbBitCount == 24) 
{
image.data = (unsigned char *)RL_MALLOC(image.width*image.height*3*sizeof(unsigned char));
fread(image.data, image.width*image.height*3, 1, ddsFile);
image.format = UNCOMPRESSED_R8G8B8;
}
else if (ddsHeader.ddspf.flags == 0x41 && ddsHeader.ddspf.rgbBitCount == 32) 
{
image.data = (unsigned char *)RL_MALLOC(image.width*image.height*4*sizeof(unsigned char));
fread(image.data, image.width*image.height*4, 1, ddsFile);
unsigned char blue = 0;
for (int i = 0; i < image.width*image.height*4; i += 4)
{
blue = ((unsigned char *)image.data)[i];
((unsigned char *)image.data)[i] = ((unsigned char *)image.data)[i + 2];
((unsigned char *)image.data)[i + 2] = blue;
}
image.format = UNCOMPRESSED_R8G8B8A8;
}
else if (((ddsHeader.ddspf.flags == 0x04) || (ddsHeader.ddspf.flags == 0x05)) && (ddsHeader.ddspf.fourCC > 0)) 
{
int size; 
if (ddsHeader.mipmapCount > 1) size = ddsHeader.pitchOrLinearSize*2;
else size = ddsHeader.pitchOrLinearSize;
image.data = (unsigned char *)RL_MALLOC(size*sizeof(unsigned char));
fread(image.data, size, 1, ddsFile);
switch (ddsHeader.ddspf.fourCC)
{
case FOURCC_DXT1:
{
if (ddsHeader.ddspf.flags == 0x04) image.format = COMPRESSED_DXT1_RGB;
else image.format = COMPRESSED_DXT1_RGBA;
} break;
case FOURCC_DXT3: image.format = COMPRESSED_DXT3_RGBA; break;
case FOURCC_DXT5: image.format = COMPRESSED_DXT5_RGBA; break;
default: break;
}
}
}
fclose(ddsFile); 
}
return image;
}
#endif
#if defined(SUPPORT_FILEFORMAT_PKM)
static Image LoadPKM(const char *fileName)
{
typedef struct {
char id[4]; 
char version[2]; 
unsigned short format; 
unsigned short width; 
unsigned short height; 
unsigned short origWidth; 
unsigned short origHeight; 
} PKMHeader;
Image image = { 0 };
FILE *pkmFile = fopen(fileName, "rb");
if (pkmFile == NULL)
{
TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open PKM file", fileName);
}
else
{
PKMHeader pkmHeader = { 0 };
fread(&pkmHeader, sizeof(PKMHeader), 1, pkmFile);
if ((pkmHeader.id[0] != 'P') || (pkmHeader.id[1] != 'K') || (pkmHeader.id[2] != 'M') || (pkmHeader.id[3] != ' '))
{
TRACELOG(LOG_WARNING, "IMAGE: [%s] PKM file not a valid image", fileName);
}
else
{
pkmHeader.format = ((pkmHeader.format & 0x00FF) << 8) | ((pkmHeader.format & 0xFF00) >> 8);
pkmHeader.width = ((pkmHeader.width & 0x00FF) << 8) | ((pkmHeader.width & 0xFF00) >> 8);
pkmHeader.height = ((pkmHeader.height & 0x00FF) << 8) | ((pkmHeader.height & 0xFF00) >> 8);
TRACELOGD("IMAGE: [%s] PKM file info:", fileName);
TRACELOGD(" > Image width: %i", pkmHeader.width);
TRACELOGD(" > Image height: %i", pkmHeader.height);
TRACELOGD(" > Image format: %i", pkmHeader.format);
image.width = pkmHeader.width;
image.height = pkmHeader.height;
image.mipmaps = 1;
int bpp = 4;
if (pkmHeader.format == 3) bpp = 8;
int size = image.width*image.height*bpp/8; 
image.data = (unsigned char *)RL_MALLOC(size*sizeof(unsigned char));
fread(image.data, size, 1, pkmFile);
if (pkmHeader.format == 0) image.format = COMPRESSED_ETC1_RGB;
else if (pkmHeader.format == 1) image.format = COMPRESSED_ETC2_RGB;
else if (pkmHeader.format == 3) image.format = COMPRESSED_ETC2_EAC_RGBA;
}
fclose(pkmFile); 
}
return image;
}
#endif
#if defined(SUPPORT_FILEFORMAT_KTX)
static Image LoadKTX(const char *fileName)
{
typedef struct {
char id[12]; 
unsigned int endianness; 
unsigned int glType; 
unsigned int glTypeSize; 
unsigned int glFormat; 
unsigned int glInternalFormat; 
unsigned int glBaseInternalFormat; 
unsigned int width; 
unsigned int height; 
unsigned int depth; 
unsigned int elements; 
unsigned int faces; 
unsigned int mipmapLevels; 
unsigned int keyValueDataSize; 
} KTXHeader;
Image image = { 0 };
FILE *ktxFile = fopen(fileName, "rb");
if (ktxFile == NULL)
{
TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to load KTX file", fileName);
}
else
{
KTXHeader ktxHeader = { 0 };
fread(&ktxHeader, sizeof(KTXHeader), 1, ktxFile);
if ((ktxHeader.id[1] != 'K') || (ktxHeader.id[2] != 'T') || (ktxHeader.id[3] != 'X') ||
(ktxHeader.id[4] != ' ') || (ktxHeader.id[5] != '1') || (ktxHeader.id[6] != '1'))
{
TRACELOG(LOG_WARNING, "IMAGE: [%s] KTX file not a valid image", fileName);
}
else
{
image.width = ktxHeader.width;
image.height = ktxHeader.height;
image.mipmaps = ktxHeader.mipmapLevels;
TRACELOGD("IMAGE: [%s] KTX file info:", fileName);
TRACELOGD(" > Image width: %i", ktxHeader.width);
TRACELOGD(" > Image height: %i", ktxHeader.height);
TRACELOGD(" > Image format: 0x%x", ktxHeader.glInternalFormat);
unsigned char unused;
if (ktxHeader.keyValueDataSize > 0)
{
for (unsigned int i = 0; i < ktxHeader.keyValueDataSize; i++) fread(&unused, sizeof(unsigned char), 1U, ktxFile);
}
int dataSize;
fread(&dataSize, sizeof(unsigned int), 1, ktxFile);
image.data = (unsigned char *)RL_MALLOC(dataSize*sizeof(unsigned char));
fread(image.data, dataSize, 1, ktxFile);
if (ktxHeader.glInternalFormat == 0x8D64) image.format = COMPRESSED_ETC1_RGB;
else if (ktxHeader.glInternalFormat == 0x9274) image.format = COMPRESSED_ETC2_RGB;
else if (ktxHeader.glInternalFormat == 0x9278) image.format = COMPRESSED_ETC2_EAC_RGBA;
}
fclose(ktxFile); 
}
return image;
}
static int SaveKTX(Image image, const char *fileName)
{
int success = 0;
typedef struct {
char id[12]; 
unsigned int endianness; 
unsigned int glType; 
unsigned int glTypeSize; 
unsigned int glFormat; 
unsigned int glInternalFormat; 
unsigned int glBaseInternalFormat; 
unsigned int width; 
unsigned int height; 
unsigned int depth; 
unsigned int elements; 
unsigned int faces; 
unsigned int mipmapLevels; 
unsigned int keyValueDataSize; 
} KTXHeader;
FILE *ktxFile = fopen(fileName, "wb");
if (ktxFile == NULL) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open KTX file", fileName);
else
{
KTXHeader ktxHeader = { 0 };
const char ktxIdentifier[12] = { 0xAB, 'K', 'T', 'X', ' ', '1', '1', 0xBB, '\r', '\n', 0x1A, '\n' };
strncpy(ktxHeader.id, ktxIdentifier, 12); 
ktxHeader.endianness = 0;
ktxHeader.glType = 0; 
ktxHeader.glTypeSize = 1;
ktxHeader.glFormat = 0; 
ktxHeader.glInternalFormat = 0; 
ktxHeader.glBaseInternalFormat = 0;
ktxHeader.width = image.width;
ktxHeader.height = image.height;
ktxHeader.depth = 0;
ktxHeader.elements = 0;
ktxHeader.faces = 1;
ktxHeader.mipmapLevels = image.mipmaps; 
ktxHeader.keyValueDataSize = 0; 
rlGetGlTextureFormats(image.format, &ktxHeader.glInternalFormat, &ktxHeader.glFormat, &ktxHeader.glType); 
ktxHeader.glBaseInternalFormat = ktxHeader.glFormat; 
if (ktxHeader.glFormat == -1) TRACELOG(LOG_WARNING, "IMAGE: GL format not supported for KTX export (%i)", ktxHeader.glFormat);
else
{
success = fwrite(&ktxHeader, sizeof(KTXHeader), 1, ktxFile);
int width = image.width;
int height = image.height;
int dataOffset = 0;
for (int i = 0; i < image.mipmaps; i++)
{
unsigned int dataSize = GetPixelDataSize(width, height, image.format);
success = fwrite(&dataSize, sizeof(unsigned int), 1, ktxFile);
success = fwrite((unsigned char *)image.data + dataOffset, dataSize, 1, ktxFile);
width /= 2;
height /= 2;
dataOffset += dataSize;
}
}
fclose(ktxFile); 
}
return success;
}
#endif
#if defined(SUPPORT_FILEFORMAT_PVR)
static Image LoadPVR(const char *fileName)
{
#if 0 
typedef struct {
unsigned int headerLength;
unsigned int height;
unsigned int width;
unsigned int numMipmaps;
unsigned int flags;
unsigned int dataLength;
unsigned int bpp;
unsigned int bitmaskRed;
unsigned int bitmaskGreen;
unsigned int bitmaskBlue;
unsigned int bitmaskAlpha;
unsigned int pvrTag;
unsigned int numSurfs;
} PVRHeaderV2;
#endif
typedef struct {
char id[4];
unsigned int flags;
unsigned char channels[4]; 
unsigned char channelDepth[4]; 
unsigned int colourSpace;
unsigned int channelType;
unsigned int height;
unsigned int width;
unsigned int depth;
unsigned int numSurfaces;
unsigned int numFaces;
unsigned int numMipmaps;
unsigned int metaDataSize;
} PVRHeaderV3;
#if 0 
typedef struct {
unsigned int devFOURCC;
unsigned int key;
unsigned int dataSize; 
unsigned char *data; 
} PVRMetadata;
#endif
Image image = { 0 };
FILE *pvrFile = fopen(fileName, "rb");
if (pvrFile == NULL)
{
TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to load PVR file", fileName);
}
else
{
unsigned char pvrVersion = 0;
fread(&pvrVersion, sizeof(unsigned char), 1, pvrFile);
fseek(pvrFile, 0, SEEK_SET);
if (pvrVersion == 0x50)
{
PVRHeaderV3 pvrHeader = { 0 };
fread(&pvrHeader, sizeof(PVRHeaderV3), 1, pvrFile);
if ((pvrHeader.id[0] != 'P') || (pvrHeader.id[1] != 'V') || (pvrHeader.id[2] != 'R') || (pvrHeader.id[3] != 3))
{
TRACELOG(LOG_WARNING, "IMAGE: [%s] PVR file not a valid image", fileName);
}
else
{
image.width = pvrHeader.width;
image.height = pvrHeader.height;
image.mipmaps = pvrHeader.numMipmaps;
if (((pvrHeader.channels[0] == 'l') && (pvrHeader.channels[1] == 0)) && (pvrHeader.channelDepth[0] == 8))
image.format = UNCOMPRESSED_GRAYSCALE;
else if (((pvrHeader.channels[0] == 'l') && (pvrHeader.channels[1] == 'a')) && ((pvrHeader.channelDepth[0] == 8) && (pvrHeader.channelDepth[1] == 8)))
image.format = UNCOMPRESSED_GRAY_ALPHA;
else if ((pvrHeader.channels[0] == 'r') && (pvrHeader.channels[1] == 'g') && (pvrHeader.channels[2] == 'b'))
{
if (pvrHeader.channels[3] == 'a')
{
if ((pvrHeader.channelDepth[0] == 5) && (pvrHeader.channelDepth[1] == 5) && (pvrHeader.channelDepth[2] == 5) && (pvrHeader.channelDepth[3] == 1))
image.format = UNCOMPRESSED_R5G5B5A1;
else if ((pvrHeader.channelDepth[0] == 4) && (pvrHeader.channelDepth[1] == 4) && (pvrHeader.channelDepth[2] == 4) && (pvrHeader.channelDepth[3] == 4))
image.format = UNCOMPRESSED_R4G4B4A4;
else if ((pvrHeader.channelDepth[0] == 8) && (pvrHeader.channelDepth[1] == 8) && (pvrHeader.channelDepth[2] == 8) && (pvrHeader.channelDepth[3] == 8))
image.format = UNCOMPRESSED_R8G8B8A8;
}
else if (pvrHeader.channels[3] == 0)
{
if ((pvrHeader.channelDepth[0] == 5) && (pvrHeader.channelDepth[1] == 6) && (pvrHeader.channelDepth[2] == 5)) image.format = UNCOMPRESSED_R5G6B5;
else if ((pvrHeader.channelDepth[0] == 8) && (pvrHeader.channelDepth[1] == 8) && (pvrHeader.channelDepth[2] == 8)) image.format = UNCOMPRESSED_R8G8B8;
}
}
else if (pvrHeader.channels[0] == 2) image.format = COMPRESSED_PVRT_RGB;
else if (pvrHeader.channels[0] == 3) image.format = COMPRESSED_PVRT_RGBA;
unsigned char unused = 0;
for (int i = 0; i < pvrHeader.metaDataSize; i++) fread(&unused, sizeof(unsigned char), 1, pvrFile);
int bpp = 0;
switch (image.format)
{
case UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
case UNCOMPRESSED_GRAY_ALPHA:
case UNCOMPRESSED_R5G5B5A1:
case UNCOMPRESSED_R5G6B5:
case UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
case UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
case UNCOMPRESSED_R8G8B8: bpp = 24; break;
case COMPRESSED_PVRT_RGB:
case COMPRESSED_PVRT_RGBA: bpp = 4; break;
default: break;
}
int dataSize = image.width*image.height*bpp/8; 
image.data = (unsigned char *)RL_MALLOC(dataSize*sizeof(unsigned char));
fread(image.data, dataSize, 1, pvrFile);
}
}
else if (pvrVersion == 52) TRACELOG(LOG_INFO, "IMAGE: [%s] PVRv2 format not supported, update your files to PVRv3", fileName);
fclose(pvrFile); 
}
return image;
}
#endif
#if defined(SUPPORT_FILEFORMAT_ASTC)
static Image LoadASTC(const char *fileName)
{
typedef struct {
unsigned char id[4]; 
unsigned char blockX; 
unsigned char blockY; 
unsigned char blockZ; 
unsigned char width[3]; 
unsigned char height[3]; 
unsigned char length[3]; 
} ASTCHeader;
Image image = { 0 };
FILE *astcFile = fopen(fileName, "rb");
if (astcFile == NULL)
{
TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to load ASTC file", fileName);
}
else
{
ASTCHeader astcHeader = { 0 };
fread(&astcHeader, sizeof(ASTCHeader), 1, astcFile);
if ((astcHeader.id[3] != 0x5c) || (astcHeader.id[2] != 0xa1) || (astcHeader.id[1] != 0xab) || (astcHeader.id[0] != 0x13))
{
TRACELOG(LOG_WARNING, "IMAGE: [%s] ASTC file not a valid image", fileName);
}
else
{
image.width = 0x00000000 | ((int)astcHeader.width[2] << 16) | ((int)astcHeader.width[1] << 8) | ((int)astcHeader.width[0]);
image.height = 0x00000000 | ((int)astcHeader.height[2] << 16) | ((int)astcHeader.height[1] << 8) | ((int)astcHeader.height[0]);
TRACELOGD("IMAGE: [%s] ASTC file info:", fileName);
TRACELOGD(" > Image width: %i", image.width);
TRACELOGD(" > Image height: %i", image.height);
TRACELOGD(" > Image blocks: %ix%i", astcHeader.blockX, astcHeader.blockY);
image.mipmaps = 1; 
int bpp = 128/(astcHeader.blockX*astcHeader.blockY);
if ((bpp == 8) || (bpp == 2))
{
int dataSize = image.width*image.height*bpp/8; 
image.data = (unsigned char *)RL_MALLOC(dataSize*sizeof(unsigned char));
fread(image.data, dataSize, 1, astcFile);
if (bpp == 8) image.format = COMPRESSED_ASTC_4x4_RGBA;
else if (bpp == 2) image.format = COMPRESSED_ASTC_8x8_RGBA;
}
else TRACELOG(LOG_WARNING, "IMAGE: [%s] ASTC block size configuration not supported", fileName);
}
fclose(astcFile);
}
return image;
}
#endif
