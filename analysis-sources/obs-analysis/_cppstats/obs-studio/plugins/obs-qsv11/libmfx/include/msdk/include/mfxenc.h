#include "mfxdefs.h"
#include "mfxvstructures.h"
#if defined(__cplusplus)
extern "C"
{
#endif 
typedef struct _mfxENCInput mfxENCInput;
struct _mfxENCInput{
mfxU32 reserved[32];
mfxFrameSurface1 *InSurface;
mfxU16 NumFrameL0;
mfxFrameSurface1 **L0Surface;
mfxU16 NumFrameL1;
mfxFrameSurface1 **L1Surface;
mfxU16 NumExtParam;
mfxExtBuffer **ExtParam;
} ;
typedef struct _mfxENCOutput mfxENCOutput;
struct _mfxENCOutput{
mfxU32 reserved[32];
mfxFrameSurface1 *OutSurface;
mfxU16 NumExtParam;
mfxExtBuffer **ExtParam;
} ;
mfxStatus MFX_CDECL MFXVideoENC_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);
mfxStatus MFX_CDECL MFXVideoENC_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request);
mfxStatus MFX_CDECL MFXVideoENC_Init(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoENC_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoENC_Close(mfxSession session);
mfxStatus MFX_CDECL MFXVideoENC_ProcessFrameAsync(mfxSession session, mfxENCInput *in, mfxENCOutput *out, mfxSyncPoint *syncp);
mfxStatus MFX_CDECL MFXVideoENC_GetVideoParam(mfxSession session, mfxVideoParam *par);
#if defined(__cplusplus)
} 
#endif 
