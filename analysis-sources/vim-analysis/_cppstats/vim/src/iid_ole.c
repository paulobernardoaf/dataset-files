#if defined(__cplusplus)
extern "C"{
#endif
#if defined(__MINGW32__)
#include <w32api.h>
#if __W32API_MAJOR_VERSION == 3 && __W32API_MINOR_VERSION < 10
#define __IID_DEFINED__
#endif
#endif
#if !defined(__IID_DEFINED__)
#define __IID_DEFINED__
typedef struct _IID
{
unsigned long x;
unsigned short s1;
unsigned short s2;
unsigned char c[8];
} IID;
#endif
#if !defined(CLSID_DEFINED)
#define CLSID_DEFINED
typedef IID CLSID;
#endif
const IID IID_IVim = {0x0F0BFAE2,0x4C90,0x11d1,{0x82,0xD7,0x00,0x04,0xAC,0x36,0x85,0x19}};
const IID LIBID_Vim = {0x0F0BFAE0,0x4C90,0x11d1,{0x82,0xD7,0x00,0x04,0xAC,0x36,0x85,0x19}};
const CLSID CLSID_Vim = {0x0F0BFAE1,0x4C90,0x11d1,{0x82,0xD7,0x00,0x04,0xAC,0x36,0x85,0x19}};
#if defined(__cplusplus)
}
#endif
