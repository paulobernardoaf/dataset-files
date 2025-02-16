#include "mfxdefs.h"
#if defined(__cplusplus)
extern "C"
{
#endif 
enum {
MFX_CODEC_JPEG = MFX_MAKEFOURCC('J','P','E','G')
};
enum
{
MFX_PROFILE_JPEG_BASELINE = 1
};
enum
{
MFX_ROTATION_0 = 0,
MFX_ROTATION_90 = 1,
MFX_ROTATION_180 = 2,
MFX_ROTATION_270 = 3
};
enum {
MFX_EXTBUFF_JPEG_QT = MFX_MAKEFOURCC('J','P','G','Q'),
MFX_EXTBUFF_JPEG_HUFFMAN = MFX_MAKEFOURCC('J','P','G','H')
};
enum {
MFX_JPEG_COLORFORMAT_UNKNOWN = 0,
MFX_JPEG_COLORFORMAT_YCbCr = 1,
MFX_JPEG_COLORFORMAT_RGB = 2
};
enum {
MFX_SCANTYPE_UNKNOWN = 0,
MFX_SCANTYPE_INTERLEAVED = 1,
MFX_SCANTYPE_NONINTERLEAVED = 2
};
enum {
MFX_CHROMAFORMAT_JPEG_SAMPLING = 6
};
typedef struct {
mfxExtBuffer Header;
mfxU16 reserved[7];
mfxU16 NumTable;
mfxU16 Qm[4][64];
} mfxExtJPEGQuantTables;
typedef struct {
mfxExtBuffer Header;
mfxU16 reserved[2];
mfxU16 NumDCTable;
mfxU16 NumACTable;
struct {
mfxU8 Bits[16];
mfxU8 Values[12];
} DCTables[4];
struct {
mfxU8 Bits[16];
mfxU8 Values[162];
} ACTables[4];
} mfxExtJPEGHuffmanTables;
#if defined(__cplusplus)
} 
#endif 
