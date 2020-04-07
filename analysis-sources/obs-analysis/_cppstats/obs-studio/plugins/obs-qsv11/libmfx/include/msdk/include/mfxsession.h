#include "mfxcommon.h"
#if defined(__cplusplus)
extern "C"
{
#endif 
typedef struct _mfxSession *mfxSession;
mfxStatus MFX_CDECL MFXInit(mfxIMPL impl, mfxVersion *ver, mfxSession *session);
mfxStatus MFX_CDECL MFXInitEx(mfxInitParam par, mfxSession *session);
mfxStatus MFX_CDECL MFXClose(mfxSession session);
mfxStatus MFX_CDECL MFXQueryIMPL(mfxSession session, mfxIMPL *impl);
mfxStatus MFX_CDECL MFXQueryVersion(mfxSession session, mfxVersion *version);
mfxStatus MFX_CDECL MFXJoinSession(mfxSession session, mfxSession child);
mfxStatus MFX_CDECL MFXDisjoinSession(mfxSession session);
mfxStatus MFX_CDECL MFXCloneSession(mfxSession session, mfxSession *clone);
mfxStatus MFX_CDECL MFXSetPriority(mfxSession session, mfxPriority priority);
mfxStatus MFX_CDECL MFXGetPriority(mfxSession session, mfxPriority *priority);
mfxStatus MFX_CDECL MFXDoWork(mfxSession session);
#if defined(__cplusplus)
}
#endif 
