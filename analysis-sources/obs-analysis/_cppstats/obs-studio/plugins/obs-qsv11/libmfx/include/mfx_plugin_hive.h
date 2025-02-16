#pragma once
#include "mfx_dispatcher_defs.h"
#include "mfxplugin.h"
#include "mfx_win_reg_key.h"
#include "mfx_vector.h"
#include <string.h>
#include <memory>
#include <stdio.h>
struct MFX_DISP_HANDLE;
namespace MFX {
inline bool operator == (const mfxPluginUID &lhs, const mfxPluginUID & rhs) 
{
return !memcmp(lhs.Data, rhs.Data, sizeof(mfxPluginUID));
}
inline bool operator != (const mfxPluginUID &lhs, const mfxPluginUID & rhs) 
{
return !(lhs == rhs);
}
#if defined(_WIN32)
#pragma warning (disable: 4351)
#endif
class PluginDescriptionRecord : public mfxPluginParam 
{
public:
msdk_disp_char sPath[MAX_PLUGIN_PATH];
char sName[MAX_PLUGIN_NAME];
bool onlyVersionRegistered;
bool Default;
PluginDescriptionRecord()
: mfxPluginParam()
, sPath()
, sName()
, onlyVersionRegistered()
, Default()
{
}
};
typedef MFXVector<PluginDescriptionRecord> MFXPluginStorage;
class MFXPluginStorageBase : public MFXPluginStorage 
{
protected:
mfxVersion mCurrentAPIVersion;
protected:
MFXPluginStorageBase(mfxVersion currentAPIVersion) 
: mCurrentAPIVersion(currentAPIVersion)
{
}
void ConvertAPIVersion( mfxU32 APIVersion, PluginDescriptionRecord &descriptionRecord) const
{
descriptionRecord.APIVersion.Minor = static_cast<mfxU16> (APIVersion & 0x0ff);
descriptionRecord.APIVersion.Major = static_cast<mfxU16> (APIVersion >> 8);
}
};
class MFXPluginsInHive : public MFXPluginStorageBase
{
public:
MFXPluginsInHive(int mfxStorageID, const msdk_disp_char *msdkLibSubKey, mfxVersion currentAPIVersion);
};
#if defined(MEDIASDK_USE_CFGFILES) || (!defined(MEDIASDK_UWP_LOADER) && !defined(MEDIASDK_UWP_PROCTABLE))
class MFXPluginsInFS : public MFXPluginStorageBase
{
bool mIsVersionParsed;
bool mIsAPIVersionParsed;
public:
MFXPluginsInFS(mfxVersion currentAPIVersion);
private:
bool ParseFile(FILE * f, PluginDescriptionRecord & des);
bool ParseKVPair( msdk_disp_char *key, msdk_disp_char * value, PluginDescriptionRecord & des);
};
#endif 
class MFXDefaultPlugins : public MFXPluginStorageBase
{
public:
MFXDefaultPlugins(mfxVersion currentAPIVersion, MFX_DISP_HANDLE * hdl, int implType);
private:
};
}
