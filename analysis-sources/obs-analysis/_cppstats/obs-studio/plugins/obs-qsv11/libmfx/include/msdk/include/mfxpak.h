#include "mfxdefs.h"
#include "mfxvstructures.h"
#if defined(__cplusplus)
extern "C"
{
#endif 
typedef struct {
mfxU16 reserved[32];
mfxFrameSurface1 *InSurface;
mfxU16 NumFrameL0;
mfxFrameSurface1 **L0Surface;
mfxU16 NumFrameL1;
mfxFrameSurface1 **L1Surface;
mfxU16 NumExtParam;
mfxExtBuffer **ExtParam;
mfxU16 NumPayload;
mfxPayload **Payload;
} mfxPAKInput;
typedef struct {
mfxU16 reserved[32];
mfxBitstream *Bs;
mfxFrameSurface1 *OutSurface;
mfxU16 NumExtParam;
mfxExtBuffer **ExtParam;
} mfxPAKOutput;
typedef struct _mfxSession *mfxSession;
mfxStatus MFX_CDECL MFXVideoPAK_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);
mfxStatus MFX_CDECL MFXVideoPAK_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest request[2]);
mfxStatus MFX_CDECL MFXVideoPAK_Init(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoPAK_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoPAK_Close(mfxSession session);
mfxStatus MFX_CDECL MFXVideoPAK_ProcessFrameAsync(mfxSession session, mfxPAKInput *in, mfxPAKOutput *out, mfxSyncPoint *syncp);
mfxStatus MFX_CDECL MFXVideoPAK_GetVideoParam(mfxSession session, mfxVideoParam *par);
#if defined(__cplusplus)
} 
#endif 
