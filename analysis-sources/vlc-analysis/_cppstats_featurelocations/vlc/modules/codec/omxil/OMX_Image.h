


























#if !defined(OMX_Image_h)
#define OMX_Image_h

#if defined(__cplusplus)
extern "C" {
#endif 








#include <OMX_IVCommon.h>










typedef enum OMX_IMAGE_CODINGTYPE {
OMX_IMAGE_CodingUnused, 
OMX_IMAGE_CodingAutoDetect, 
OMX_IMAGE_CodingJPEG, 
OMX_IMAGE_CodingJPEG2K, 
OMX_IMAGE_CodingEXIF, 
OMX_IMAGE_CodingTIFF, 
OMX_IMAGE_CodingGIF, 
OMX_IMAGE_CodingPNG, 
OMX_IMAGE_CodingLZW, 
OMX_IMAGE_CodingBMP, 
OMX_IMAGE_CodingKhronosExtensions = 0x6F000000, 
OMX_IMAGE_CodingVendorStartUnused = 0x7F000000, 
OMX_IMAGE_CodingMax = 0x7FFFFFFF
} OMX_IMAGE_CODINGTYPE;











































typedef struct OMX_IMAGE_PORTDEFINITIONTYPE {
OMX_STRING cMIMEType;
OMX_NATIVE_DEVICETYPE pNativeRender;
OMX_U32 nFrameWidth; 
OMX_U32 nFrameHeight;
OMX_S32 nStride; 
OMX_U32 nSliceHeight;
OMX_BOOL bFlagErrorConcealment;
OMX_IMAGE_CODINGTYPE eCompressionFormat;
OMX_COLOR_FORMATTYPE eColorFormat;
OMX_NATIVE_WINDOWTYPE pNativeWindow;
} OMX_IMAGE_PORTDEFINITIONTYPE;

















typedef struct OMX_IMAGE_PARAM_PORTFORMATTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 nIndex;
OMX_IMAGE_CODINGTYPE eCompressionFormat;
OMX_COLOR_FORMATTYPE eColorFormat;
} OMX_IMAGE_PARAM_PORTFORMATTYPE;








typedef enum OMX_IMAGE_FLASHCONTROLTYPE {
OMX_IMAGE_FlashControlOn = 0,
OMX_IMAGE_FlashControlOff,
OMX_IMAGE_FlashControlAuto,
OMX_IMAGE_FlashControlRedEyeReduction,
OMX_IMAGE_FlashControlFillin,
OMX_IMAGE_FlashControlTorch,
OMX_IMAGE_FlashControlKhronosExtensions = 0x6F000000, 
OMX_IMAGE_FlashControlVendorStartUnused = 0x7F000000, 
OMX_IMAGE_FlashControlMax = 0x7FFFFFFF
} OMX_IMAGE_FLASHCONTROLTYPE;











typedef struct OMX_IMAGE_PARAM_FLASHCONTROLTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_IMAGE_FLASHCONTROLTYPE eFlashControl;
} OMX_IMAGE_PARAM_FLASHCONTROLTYPE;





typedef enum OMX_IMAGE_FOCUSCONTROLTYPE {
OMX_IMAGE_FocusControlOn = 0,
OMX_IMAGE_FocusControlOff,
OMX_IMAGE_FocusControlAuto,
OMX_IMAGE_FocusControlAutoLock,
OMX_IMAGE_FocusControlKhronosExtensions = 0x6F000000, 
OMX_IMAGE_FocusControlVendorStartUnused = 0x7F000000, 
OMX_IMAGE_FocusControlMax = 0x7FFFFFFF
} OMX_IMAGE_FOCUSCONTROLTYPE;














typedef struct OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_IMAGE_FOCUSCONTROLTYPE eFocusControl;
OMX_U32 nFocusSteps;
OMX_U32 nFocusStepIndex;
} OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE;

















typedef struct OMX_IMAGE_PARAM_QFACTORTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 nPortIndex; 
OMX_U32 nQFactor; 
} OMX_IMAGE_PARAM_QFACTORTYPE;





typedef enum OMX_IMAGE_QUANTIZATIONTABLETYPE {
OMX_IMAGE_QuantizationTableLuma = 0,
OMX_IMAGE_QuantizationTableChroma,
OMX_IMAGE_QuantizationTableChromaCb,
OMX_IMAGE_QuantizationTableChromaCr,
OMX_IMAGE_QuantizationTableKhronosExtensions = 0x6F000000, 
OMX_IMAGE_QuantizationTableVendorStartUnused = 0x7F000000, 
OMX_IMAGE_QuantizationTableMax = 0x7FFFFFFF
} OMX_IMAGE_QUANTIZATIONTABLETYPE;


















typedef struct OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_IMAGE_QUANTIZATIONTABLETYPE eQuantizationTable;
OMX_U8 nQuantizationMatrix[64];
} OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE;






typedef enum OMX_IMAGE_HUFFMANTABLETYPE {
OMX_IMAGE_HuffmanTableAC = 0,
OMX_IMAGE_HuffmanTableDC,
OMX_IMAGE_HuffmanTableACLuma,
OMX_IMAGE_HuffmanTableACChroma,
OMX_IMAGE_HuffmanTableDCLuma,
OMX_IMAGE_HuffmanTableDCChroma,
OMX_IMAGE_HuffmanTableKhronosExtensions = 0x6F000000, 
OMX_IMAGE_HuffmanTableVendorStartUnused = 0x7F000000, 
OMX_IMAGE_HuffmanTableMax = 0x7FFFFFFF
} OMX_IMAGE_HUFFMANTABLETYPE;














typedef struct OMX_IMAGE_PARAM_HUFFMANTTABLETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_IMAGE_HUFFMANTABLETYPE eHuffmanTable;
OMX_U8 nNumberOfHuffmanCodeOfLength[16];
OMX_U8 nHuffmanTable[256];
}OMX_IMAGE_PARAM_HUFFMANTTABLETYPE;


#if defined(__cplusplus)
}
#endif 

#endif

