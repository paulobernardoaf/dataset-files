




























#if !defined(OMX_Component_h)
#define OMX_Component_h

#if defined(__cplusplus)
extern "C" {
#endif 








#include <OMX_Audio.h>
#include <OMX_Video.h>
#include <OMX_Image.h>
#include <OMX_Other.h>


typedef enum OMX_PORTDOMAINTYPE { 
OMX_PortDomainAudio, 
OMX_PortDomainVideo, 
OMX_PortDomainImage, 
OMX_PortDomainOther,
OMX_PortDomainKhronosExtensions = 0x6F000000, 
OMX_PortDomainVendorStartUnused = 0x7F000000, 
OMX_PortDomainMax = 0x7ffffff
} OMX_PORTDOMAINTYPE;


typedef struct OMX_PARAM_PORTDEFINITIONTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 nPortIndex; 
OMX_DIRTYPE eDir; 
OMX_U32 nBufferCountActual; 
OMX_U32 nBufferCountMin; 
OMX_U32 nBufferSize; 
OMX_BOOL bEnabled; 



OMX_BOOL bPopulated; 



OMX_PORTDOMAINTYPE eDomain; 
union {
OMX_AUDIO_PORTDEFINITIONTYPE audio;
OMX_VIDEO_PORTDEFINITIONTYPE video;
OMX_IMAGE_PORTDEFINITIONTYPE image;
OMX_OTHER_PORTDEFINITIONTYPE other;
} format;
OMX_BOOL bBuffersContiguous;
OMX_U32 nBufferAlignment;
} OMX_PARAM_PORTDEFINITIONTYPE;


typedef struct OMX_PARAM_U32TYPE { 
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 nPortIndex; 
OMX_U32 nU32; 
} OMX_PARAM_U32TYPE;


typedef enum OMX_SUSPENSIONPOLICYTYPE {
OMX_SuspensionDisabled, 
OMX_SuspensionEnabled, 
OMX_SuspensionPolicyKhronosExtensions = 0x6F000000, 
OMX_SuspensionPolicyStartUnused = 0x7F000000, 
OMX_SuspensionPolicyMax = 0x7fffffff
} OMX_SUSPENSIONPOLICYTYPE;


typedef struct OMX_PARAM_SUSPENSIONPOLICYTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_SUSPENSIONPOLICYTYPE ePolicy;
} OMX_PARAM_SUSPENSIONPOLICYTYPE;


typedef enum OMX_SUSPENSIONTYPE {
OMX_NotSuspended, 
OMX_Suspended, 
OMX_SuspensionKhronosExtensions = 0x6F000000, 
OMX_SuspensionVendorStartUnused = 0x7F000000, 
OMX_SuspendMax = 0x7FFFFFFF
} OMX_SUSPENSIONTYPE;


typedef struct OMX_PARAM_SUSPENSIONTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_SUSPENSIONTYPE eType; 
} OMX_PARAM_SUSPENSIONTYPE ;

typedef struct OMX_CONFIG_BOOLEANTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_BOOL bEnabled; 
} OMX_CONFIG_BOOLEANTYPE;



typedef struct OMX_PARAM_CONTENTURITYPE
{
OMX_U32 nSize; 

OMX_VERSIONTYPE nVersion; 
OMX_U8 contentURI[1]; 
} OMX_PARAM_CONTENTURITYPE;



typedef struct OMX_PARAM_CONTENTPIPETYPE
{
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_HANDLETYPE hPipe; 
} OMX_PARAM_CONTENTPIPETYPE;


typedef struct OMX_RESOURCECONCEALMENTTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_BOOL bResourceConcealmentForbidden; 



} OMX_RESOURCECONCEALMENTTYPE;



typedef enum OMX_METADATACHARSETTYPE {
OMX_MetadataCharsetUnknown = 0,
OMX_MetadataCharsetASCII,
OMX_MetadataCharsetBinary,
OMX_MetadataCharsetCodePage1252,
OMX_MetadataCharsetUTF8,
OMX_MetadataCharsetJavaConformantUTF8,
OMX_MetadataCharsetUTF7,
OMX_MetadataCharsetImapUTF7,
OMX_MetadataCharsetUTF16LE, 
OMX_MetadataCharsetUTF16BE,
OMX_MetadataCharsetGB12345,
OMX_MetadataCharsetHZGB2312,
OMX_MetadataCharsetGB2312,
OMX_MetadataCharsetGB18030,
OMX_MetadataCharsetGBK,
OMX_MetadataCharsetBig5,
OMX_MetadataCharsetISO88591,
OMX_MetadataCharsetISO88592,
OMX_MetadataCharsetISO88593,
OMX_MetadataCharsetISO88594,
OMX_MetadataCharsetISO88595,
OMX_MetadataCharsetISO88596,
OMX_MetadataCharsetISO88597,
OMX_MetadataCharsetISO88598,
OMX_MetadataCharsetISO88599,
OMX_MetadataCharsetISO885910,
OMX_MetadataCharsetISO885913,
OMX_MetadataCharsetISO885914,
OMX_MetadataCharsetISO885915,
OMX_MetadataCharsetShiftJIS,
OMX_MetadataCharsetISO2022JP,
OMX_MetadataCharsetISO2022JP1,
OMX_MetadataCharsetISOEUCJP,
OMX_MetadataCharsetSMS7Bit,
OMX_MetadataCharsetKhronosExtensions = 0x6F000000, 
OMX_MetadataCharsetVendorStartUnused = 0x7F000000, 
OMX_MetadataCharsetTypeMax= 0x7FFFFFFF
} OMX_METADATACHARSETTYPE;


typedef enum OMX_METADATASCOPETYPE
{
OMX_MetadataScopeAllLevels,
OMX_MetadataScopeTopLevel,
OMX_MetadataScopePortLevel,
OMX_MetadataScopeNodeLevel,
OMX_MetadataScopeKhronosExtensions = 0x6F000000, 
OMX_MetadataScopeVendorStartUnused = 0x7F000000, 
OMX_MetadataScopeTypeMax = 0x7fffffff
} OMX_METADATASCOPETYPE;


typedef enum OMX_METADATASEARCHMODETYPE
{
OMX_MetadataSearchValueSizeByIndex,
OMX_MetadataSearchItemByIndex,
OMX_MetadataSearchNextItemByKey,
OMX_MetadataSearchKhronosExtensions = 0x6F000000, 
OMX_MetadataSearchVendorStartUnused = 0x7F000000, 
OMX_MetadataSearchTypeMax = 0x7fffffff
} OMX_METADATASEARCHMODETYPE;

typedef struct OMX_CONFIG_METADATAITEMCOUNTTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_METADATASCOPETYPE eScopeMode;
OMX_U32 nScopeSpecifier;
OMX_U32 nMetadataItemCount;
} OMX_CONFIG_METADATAITEMCOUNTTYPE;


typedef struct OMX_CONFIG_METADATAITEMTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_METADATASCOPETYPE eScopeMode;
OMX_U32 nScopeSpecifier;
OMX_U32 nMetadataItemIndex; 
OMX_METADATASEARCHMODETYPE eSearchMode;
OMX_METADATACHARSETTYPE eKeyCharset;
OMX_U8 nKeySizeUsed;
OMX_U8 nKey[128];
OMX_METADATACHARSETTYPE eValueCharset;
OMX_STRING sLanguageCountry;
OMX_U32 nValueMaxSize;
OMX_U32 nValueSizeUsed;
OMX_U8 nValue[1];
} OMX_CONFIG_METADATAITEMTYPE;


typedef struct OMX_CONFIG_CONTAINERNODECOUNTTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_BOOL bAllKeys;
OMX_U32 nParentNodeID;
OMX_U32 nNumNodes;
} OMX_CONFIG_CONTAINERNODECOUNTTYPE;


typedef struct OMX_CONFIG_CONTAINERNODEIDTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_BOOL bAllKeys;
OMX_U32 nParentNodeID;
OMX_U32 nNodeIndex; 
OMX_U32 nNodeID; 
OMX_STRING cNodeName;
OMX_BOOL bIsLeafType;
} OMX_CONFIG_CONTAINERNODEIDTYPE;


typedef struct OMX_PARAM_METADATAFILTERTYPE 
{ 
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_BOOL bAllKeys; 

OMX_METADATACHARSETTYPE eKeyCharset;
OMX_U32 nKeySizeUsed; 
OMX_U8 nKey [128]; 
OMX_U32 nLanguageCountrySizeUsed;
OMX_U8 nLanguageCountry[128];
OMX_BOOL bEnabled; 


} OMX_PARAM_METADATAFILTERTYPE; 












typedef struct OMX_COMPONENTTYPE
{




OMX_U32 nSize;






OMX_VERSIONTYPE nVersion;





OMX_PTR pComponentPrivate;





OMX_PTR pApplicationPrivate;




OMX_ERRORTYPE (*GetComponentVersion)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_OUT OMX_STRING pComponentName,
OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
OMX_OUT OMX_UUIDTYPE* pComponentUUID);




OMX_ERRORTYPE (*SendCommand)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_IN OMX_COMMANDTYPE Cmd,
OMX_IN OMX_U32 nParam1,
OMX_IN OMX_PTR pCmdData);




OMX_ERRORTYPE (*GetParameter)(
OMX_IN OMX_HANDLETYPE hComponent, 
OMX_IN OMX_INDEXTYPE nParamIndex, 
OMX_INOUT OMX_PTR pComponentParameterStructure);





OMX_ERRORTYPE (*SetParameter)(
OMX_IN OMX_HANDLETYPE hComponent, 
OMX_IN OMX_INDEXTYPE nIndex,
OMX_IN OMX_PTR pComponentParameterStructure);





OMX_ERRORTYPE (*GetConfig)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_IN OMX_INDEXTYPE nIndex, 
OMX_INOUT OMX_PTR pComponentConfigStructure);





OMX_ERRORTYPE (*SetConfig)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_IN OMX_INDEXTYPE nIndex, 
OMX_IN OMX_PTR pComponentConfigStructure);





OMX_ERRORTYPE (*GetExtensionIndex)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_IN OMX_STRING cParameterName,
OMX_OUT OMX_INDEXTYPE* pIndexType);





OMX_ERRORTYPE (*GetState)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_OUT OMX_STATETYPE* pState);































































OMX_ERRORTYPE (*ComponentTunnelRequest)(
OMX_IN OMX_HANDLETYPE hComp,
OMX_IN OMX_U32 nPort,
OMX_IN OMX_HANDLETYPE hTunneledComp,
OMX_IN OMX_U32 nTunneledPort,
OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup); 





OMX_ERRORTYPE (*UseBuffer)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
OMX_IN OMX_U32 nPortIndex,
OMX_IN OMX_PTR pAppPrivate,
OMX_IN OMX_U32 nSizeBytes,
OMX_IN OMX_U8* pBuffer);





OMX_ERRORTYPE (*AllocateBuffer)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
OMX_IN OMX_U32 nPortIndex,
OMX_IN OMX_PTR pAppPrivate,
OMX_IN OMX_U32 nSizeBytes);





OMX_ERRORTYPE (*FreeBuffer)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_IN OMX_U32 nPortIndex,
OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);





OMX_ERRORTYPE (*EmptyThisBuffer)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);





OMX_ERRORTYPE (*FillThisBuffer)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);



















OMX_ERRORTYPE (*SetCallbacks)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_IN OMX_CALLBACKTYPE* pCallbacks, 
OMX_IN OMX_PTR pAppData);












OMX_ERRORTYPE (*ComponentDeInit)(
OMX_IN OMX_HANDLETYPE hComponent);


OMX_ERRORTYPE (*UseEGLImage)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
OMX_IN OMX_U32 nPortIndex,
OMX_IN OMX_PTR pAppPrivate,
OMX_IN void* eglImage);

OMX_ERRORTYPE (*ComponentRoleEnum)(
OMX_IN OMX_HANDLETYPE hComponent,
OMX_OUT OMX_U8 *cRole,
OMX_IN OMX_U32 nIndex);

} OMX_COMPONENTTYPE;

#if defined(__cplusplus)
}
#endif 

#endif

