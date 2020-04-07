









#pragma once

#include <stdio.h>

#include "mfxvideo++.h"







#if defined(_WIN32) || defined(_WIN64)
#include "bits/windows_defs.h"
#elif defined(__linux__)
#include "bits/linux_defs.h"
#endif



#define MSDK_PRINT_RET_MSG(ERR) { PrintErrString(ERR, __FILE__, __LINE__); }



#define MSDK_CHECK_RESULT(P, X, ERR) { if ((X) > (P)) { MSDK_PRINT_RET_MSG(ERR); return ERR; } }






#define MSDK_CHECK_POINTER(P, ERR) { if (!(P)) { MSDK_PRINT_RET_MSG(ERR); return ERR; } }






#define MSDK_CHECK_ERROR(P, X, ERR) { if ((X) == (P)) { MSDK_PRINT_RET_MSG(ERR); return ERR; } }






#define MSDK_IGNORE_MFX_STS(P, X) { if ((X) == (P)) { P = MFX_ERR_NONE; } }





#define MSDK_BREAK_ON_ERROR(P) { if (MFX_ERR_NONE != (P)) break; }




#define MSDK_SAFE_DELETE_ARRAY(P) { if (P) { delete[] P; P = NULL; } }






#define MSDK_ALIGN32(X) (((mfxU32)((X) + 31)) & (~(mfxU32)31))
#define MSDK_ALIGN16(value) (((value + 15) >> 4) << 4)
#define MSDK_SAFE_RELEASE(X) { if (X) { X->Release(); X = NULL; } }






#define MSDK_MAX(A, B) (((A) > (B)) ? (A) : (B))


#define WILL_READ 0x1000
#define WILL_WRITE 0x2000




mfxStatus simple_alloc(mfxHDL pthis, mfxFrameAllocRequest *request,
mfxFrameAllocResponse *response);
mfxStatus simple_lock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus simple_unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus simple_gethdl(mfxHDL pthis, mfxMemId mid, mfxHDL *handle);
mfxStatus simple_free(mfxHDL pthis, mfxFrameAllocResponse *response);





void PrintErrString(int err, const char *filestr, int line);







mfxStatus LoadRawFrame(mfxFrameSurface1 *pSurface, FILE *fSource);
mfxStatus LoadRawRGBFrame(mfxFrameSurface1 *pSurface, FILE *fSource);


mfxStatus WriteRawFrame(mfxFrameSurface1 *pSurface, FILE *fSink);


mfxStatus WriteBitStreamFrame(mfxBitstream *pMfxBitstream, FILE *fSink);

mfxStatus ReadBitStreamData(mfxBitstream *pBS, FILE *fSource);

void ClearYUVSurfaceSysMem(mfxFrameSurface1 *pSfc, mfxU16 width, mfxU16 height);
void ClearYUVSurfaceVMem(mfxMemId memId);
void ClearRGBSurfaceVMem(mfxMemId memId);


int GetFreeSurfaceIndex(mfxFrameSurface1 **pSurfacesPool, mfxU16 nPoolSize);


typedef struct {
mfxBitstream mfxBS;
mfxSyncPoint syncp;
} Task;


int GetFreeTaskIndex(Task *pTaskPool, mfxU16 nPoolSize);


mfxStatus Initialize(mfxIMPL impl, mfxVersion ver, MFXVideoSession *pSession,
mfxFrameAllocator *pmfxAllocator,
mfxHDL *deviceHandle = NULL,
bool bCreateSharedHandles = false, bool dx9hack = false);


void Release();


char mfxFrameTypeString(mfxU16 FrameType);

void mfxGetTime(mfxTime *timestamp);


double TimeDiffMsec(mfxTime tfinish, mfxTime tstart);
