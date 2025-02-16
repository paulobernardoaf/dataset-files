#include <mfxvideo.h>
#if !defined(MEDIASDK_UWP_LOADER) && !defined(MEDIASDK_UWP_PROCTABLE)
#include "mfx_win_reg_key.h"
#endif
#include "mfx_dispatcher.h"
#if !defined(_WIN32) && !defined(_WIN64)
struct mfx_disp_adapters
{
mfxU32 vendor_id;
mfxU32 device_id;
};
#if !defined(__APPLE__)
#define MFX_SO_BASE_NAME_LEN 15 
#else
#define MFX_SO_BASE_NAME_LEN 16 
#endif
#define MFX_MIN_REAL_LIBNAME MFX_SO_BASE_NAME_LEN + 4 
#define MFX_MAX_REAL_LIBNAME MFX_MIN_REAL_LIBNAME + 8 
struct mfx_libs
{
char name[MFX_MAX_REAL_LIBNAME+1];
mfxVersion version;
};
#endif
namespace MFX
{
#if defined(_WIN32) || defined(_WIN64)
enum
{
MFX_UNKNOWN_KEY = -1,
MFX_CURRENT_USER_KEY = 0,
MFX_LOCAL_MACHINE_KEY = 1,
MFX_APP_FOLDER = 2,
#if defined(MEDIASDK_USE_REGISTRY) || (!defined(MEDIASDK_UWP_LOADER) && !defined(MEDIASDK_UWP_PROCTABLE))
MFX_PATH_MSDK_FOLDER = 3,
MFX_STORAGE_ID_FIRST = MFX_CURRENT_USER_KEY,
MFX_STORAGE_ID_LAST = MFX_PATH_MSDK_FOLDER
#else
MFX_PATH_MSDK_FOLDER = 3,
MFX_STORAGE_ID_FIRST = MFX_PATH_MSDK_FOLDER,
MFX_STORAGE_ID_LAST = MFX_PATH_MSDK_FOLDER
#endif 
};
#else
enum
{
MFX_UNKNOWN_KEY = -1,
MFX_STORAGE_ID_OPT = 0, 
MFX_APP_FOLDER = 1,
MFX_STORAGE_ID_FIRST = MFX_STORAGE_ID_OPT,
MFX_STORAGE_ID_LAST = MFX_STORAGE_ID_OPT
};
#endif
mfxStatus SelectImplementationType(const mfxU32 adapterNum, mfxIMPL *pImplInterface, mfxU32 *pVendorID, mfxU32 *pDeviceID);
const mfxU32 msdk_disp_path_len = 1024;
class MFXLibraryIterator
{
public:
MFXLibraryIterator(void);
~MFXLibraryIterator(void);
mfxStatus Init(eMfxImplType implType, mfxIMPL implInterface, const mfxU32 adapterNum, int storageID);
mfxStatus SelectDLLVersion(msdk_disp_char *pPath, size_t pathSize,
eMfxImplType *pImplType, mfxVersion minVersion);
mfxIMPL GetImplementationType();
bool GetSubKeyName(msdk_disp_char *subKeyName, size_t length) const;
int GetStorageID() const { return m_StorageID; }
protected:
void Release(void);
mfxStatus InitRegistry(eMfxImplType implType, mfxIMPL implInterface, const mfxU32 adapterNum, int storageID);
mfxStatus InitFolder(eMfxImplType implType, mfxIMPL implInterface, const mfxU32 adapterNum, const msdk_disp_char * path);
eMfxImplType m_implType; 
mfxIMPL m_implInterface; 
mfxU32 m_vendorID; 
mfxU32 m_deviceID; 
bool m_bIsSubKeyValid;
wchar_t m_SubKeyName[MFX_MAX_REGISTRY_KEY_NAME]; 
int m_StorageID;
#if defined(_WIN32) || defined(_WIN64)
#if defined(MEDIASDK_USE_REGISTRY) || (!defined(MEDIASDK_UWP_LOADER) && !defined(MEDIASDK_UWP_PROCTABLE))
WinRegKey m_baseRegKey; 
#endif
mfxU32 m_lastLibIndex; 
mfxU32 m_lastLibMerit; 
#else
int m_lastLibIndex; 
mfxU32 m_adapters_num;
struct mfx_disp_adapters* m_adapters;
int m_selected_adapter;
mfxU32 m_libs_num;
struct mfx_libs* m_libs;
#endif 
msdk_disp_char m_path[msdk_disp_path_len];
private:
MFXLibraryIterator(const MFXLibraryIterator &);
void operator=(const MFXLibraryIterator &);
};
} 
