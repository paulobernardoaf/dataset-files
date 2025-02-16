




























#if !defined(__MFXVIDEO_H__)
#define __MFXVIDEO_H__
#include "mfxsession.h"
#include "mfxvstructures.h"

#if defined(__cplusplus)
extern "C"
{
#endif


typedef struct {
mfxU32 reserved[4];
mfxHDL pthis;
mfxStatus (MFX_CDECL *Alloc) (mfxHDL pthis, mfxU32 nbytes, mfxU16 type, mfxMemId *mid);
mfxStatus (MFX_CDECL *Lock) (mfxHDL pthis, mfxMemId mid, mfxU8 **ptr);
mfxStatus (MFX_CDECL *Unlock) (mfxHDL pthis, mfxMemId mid);
mfxStatus (MFX_CDECL *Free) (mfxHDL pthis, mfxMemId mid);
} mfxBufferAllocator;

typedef struct {
mfxU32 reserved[4];
mfxHDL pthis;

mfxStatus (MFX_CDECL *Alloc) (mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);
mfxStatus (MFX_CDECL *Lock) (mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus (MFX_CDECL *Unlock) (mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus (MFX_CDECL *GetHDL) (mfxHDL pthis, mfxMemId mid, mfxHDL *handle);
mfxStatus (MFX_CDECL *Free) (mfxHDL pthis, mfxFrameAllocResponse *response);
} mfxFrameAllocator;


mfxStatus MFX_CDECL MFXVideoCORE_SetBufferAllocator(mfxSession session, mfxBufferAllocator *allocator);
mfxStatus MFX_CDECL MFXVideoCORE_SetFrameAllocator(mfxSession session, mfxFrameAllocator *allocator);
mfxStatus MFX_CDECL MFXVideoCORE_SetHandle(mfxSession session, mfxHandleType type, mfxHDL hdl);
mfxStatus MFX_CDECL MFXVideoCORE_GetHandle(mfxSession session, mfxHandleType type, mfxHDL *hdl);
mfxStatus MFX_CDECL MFXVideoCORE_QueryPlatform(mfxSession session, mfxPlatform* platform);
mfxStatus MFX_CDECL MFXVideoCORE_SyncOperation(mfxSession session, mfxSyncPoint syncp, mfxU32 wait);


mfxStatus MFX_CDECL MFXVideoENCODE_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);
mfxStatus MFX_CDECL MFXVideoENCODE_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request);
mfxStatus MFX_CDECL MFXVideoENCODE_Init(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoENCODE_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoENCODE_Close(mfxSession session);

mfxStatus MFX_CDECL MFXVideoENCODE_GetVideoParam(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoENCODE_GetEncodeStat(mfxSession session, mfxEncodeStat *stat);
mfxStatus MFX_CDECL MFXVideoENCODE_EncodeFrameAsync(mfxSession session, mfxEncodeCtrl *ctrl, mfxFrameSurface1 *surface, mfxBitstream *bs, mfxSyncPoint *syncp);


mfxStatus MFX_CDECL MFXVideoDECODE_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);
mfxStatus MFX_CDECL MFXVideoDECODE_DecodeHeader(mfxSession session, mfxBitstream *bs, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoDECODE_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request);
mfxStatus MFX_CDECL MFXVideoDECODE_Init(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoDECODE_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoDECODE_Close(mfxSession session);

mfxStatus MFX_CDECL MFXVideoDECODE_GetVideoParam(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoDECODE_GetDecodeStat(mfxSession session, mfxDecodeStat *stat);
mfxStatus MFX_CDECL MFXVideoDECODE_SetSkipMode(mfxSession session, mfxSkipMode mode);
mfxStatus MFX_CDECL MFXVideoDECODE_GetPayload(mfxSession session, mfxU64 *ts, mfxPayload *payload);
mfxStatus MFX_CDECL MFXVideoDECODE_DecodeFrameAsync(mfxSession session, mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 **surface_out, mfxSyncPoint *syncp);


mfxStatus MFX_CDECL MFXVideoVPP_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);
mfxStatus MFX_CDECL MFXVideoVPP_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest request[2]);
mfxStatus MFX_CDECL MFXVideoVPP_Init(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoVPP_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoVPP_Close(mfxSession session);

mfxStatus MFX_CDECL MFXVideoVPP_GetVideoParam(mfxSession session, mfxVideoParam *par);
mfxStatus MFX_CDECL MFXVideoVPP_GetVPPStat(mfxSession session, mfxVPPStat *stat);
mfxStatus MFX_CDECL MFXVideoVPP_RunFrameVPPAsync(mfxSession session, mfxFrameSurface1 *in, mfxFrameSurface1 *out, mfxExtVppAuxData *aux, mfxSyncPoint *syncp);
mfxStatus MFX_CDECL MFXVideoVPP_RunFrameVPPAsyncEx(mfxSession session, mfxFrameSurface1 *in, mfxFrameSurface1 *surface_work, mfxFrameSurface1 **surface_out, mfxSyncPoint *syncp);

#if defined(__cplusplus)
} 
#endif

#endif
