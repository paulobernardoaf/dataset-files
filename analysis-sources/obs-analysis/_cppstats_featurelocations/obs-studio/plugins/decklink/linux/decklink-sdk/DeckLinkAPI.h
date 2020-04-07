


























#if !defined(BMD_DECKLINKAPI_H)
#define BMD_DECKLINKAPI_H


#if !defined(BMD_CONST)
#if defined(_MSC_VER)
#define BMD_CONST __declspec(selectany) static const
#else
#define BMD_CONST static const
#endif
#endif

#if !defined(BMD_PUBLIC)
#define BMD_PUBLIC
#endif



#include <stdint.h>
#include "LinuxCOM.h"

#include "DeckLinkAPITypes.h"
#include "DeckLinkAPIModes.h"
#include "DeckLinkAPIDiscovery.h"
#include "DeckLinkAPIConfiguration.h"
#include "DeckLinkAPIDeckControl.h"

#define BLACKMAGIC_DECKLINK_API_MAGIC 1






BMD_CONST REFIID IID_IDeckLinkVideoOutputCallback = {0x20,0xAA,0x52,0x25,0x19,0x58,0x47,0xCB,0x82,0x0B,0x80,0xA8,0xD5,0x21,0xA6,0xEE};
BMD_CONST REFIID IID_IDeckLinkInputCallback = {0xDD,0x04,0xE5,0xEC,0x74,0x15,0x42,0xAB,0xAE,0x4A,0xE8,0x0C,0x4D,0xFC,0x04,0x4A};
BMD_CONST REFIID IID_IDeckLinkEncoderInputCallback = {0xAC,0xF1,0x3E,0x61,0xF4,0xA0,0x49,0x74,0xA6,0xA7,0x59,0xAF,0xF6,0x26,0x8B,0x31};
BMD_CONST REFIID IID_IDeckLinkMemoryAllocator = {0xB3,0x6E,0xB6,0xE7,0x9D,0x29,0x4A,0xA8,0x92,0xEF,0x84,0x3B,0x87,0xA2,0x89,0xE8};
BMD_CONST REFIID IID_IDeckLinkAudioOutputCallback = {0x40,0x3C,0x68,0x1B,0x7F,0x46,0x4A,0x12,0xB9,0x93,0x2B,0xB1,0x27,0x08,0x4E,0xE6};
BMD_CONST REFIID IID_IDeckLinkIterator = {0x50,0xFB,0x36,0xCD,0x30,0x63,0x4B,0x73,0xBD,0xBB,0x95,0x80,0x87,0xF2,0xD8,0xBA};
BMD_CONST REFIID IID_IDeckLinkAPIInformation = {0x7B,0xEA,0x3C,0x68,0x73,0x0D,0x43,0x22,0xAF,0x34,0x8A,0x71,0x52,0xB5,0x32,0xA4};
BMD_CONST REFIID IID_IDeckLinkOutput = {0xCC,0x5C,0x8A,0x6E,0x3F,0x2F,0x4B,0x3A,0x87,0xEA,0xFD,0x78,0xAF,0x30,0x05,0x64};
BMD_CONST REFIID IID_IDeckLinkInput = {0xAF,0x22,0x76,0x2B,0xDF,0xAC,0x48,0x46,0xAA,0x79,0xFA,0x88,0x83,0x56,0x09,0x95};
BMD_CONST REFIID IID_IDeckLinkHDMIInputEDID = {0xAB,0xBB,0xAC,0xBC,0x45,0xBC,0x46,0x65,0x9D,0x92,0xAC,0xE6,0xE5,0xA9,0x79,0x02};
BMD_CONST REFIID IID_IDeckLinkEncoderInput = {0x27,0x05,0x87,0xDA,0x6B,0x7D,0x42,0xE7,0xA1,0xF0,0x6D,0x85,0x3F,0x58,0x11,0x85};
BMD_CONST REFIID IID_IDeckLinkVideoFrame = {0x3F,0x71,0x6F,0xE0,0xF0,0x23,0x41,0x11,0xBE,0x5D,0xEF,0x44,0x14,0xC0,0x5B,0x17};
BMD_CONST REFIID IID_IDeckLinkMutableVideoFrame = {0x69,0xE2,0x63,0x9F,0x40,0xDA,0x4E,0x19,0xB6,0xF2,0x20,0xAC,0xE8,0x15,0xC3,0x90};
BMD_CONST REFIID IID_IDeckLinkVideoFrame3DExtensions = {0xDA,0x0F,0x7E,0x4A,0xED,0xC7,0x48,0xA8,0x9C,0xDD,0x2D,0xB5,0x1C,0x72,0x9C,0xD7};
BMD_CONST REFIID IID_IDeckLinkVideoFrameMetadataExtensions = {0xD5,0x97,0x3D,0xC9,0x64,0x32,0x46,0xD0,0x8F,0x0B,0x24,0x96,0xF8,0xA1,0x23,0x8F};
BMD_CONST REFIID IID_IDeckLinkVideoInputFrame = {0x05,0xCF,0xE3,0x74,0x53,0x7C,0x40,0x94,0x9A,0x57,0x68,0x05,0x25,0x11,0x8F,0x44};
BMD_CONST REFIID IID_IDeckLinkAncillaryPacket = {0xCC,0x5B,0xBF,0x7E,0x02,0x9C,0x4D,0x3B,0x91,0x58,0x60,0x00,0xEF,0x5E,0x36,0x70};
BMD_CONST REFIID IID_IDeckLinkAncillaryPacketIterator = {0x3F,0xC8,0x99,0x4B,0x88,0xFB,0x4C,0x17,0x96,0x8F,0x9A,0xAB,0x69,0xD9,0x64,0xA7};
BMD_CONST REFIID IID_IDeckLinkVideoFrameAncillaryPackets = {0x6C,0x18,0x6C,0x0F,0x45,0x9E,0x41,0xD8,0xAE,0xE2,0x48,0x12,0xD8,0x1A,0xEE,0x68};
BMD_CONST REFIID IID_IDeckLinkVideoFrameAncillary = {0x73,0x2E,0x72,0x3C,0xD1,0xA4,0x4E,0x29,0x9E,0x8E,0x4A,0x88,0x79,0x7A,0x00,0x04};
BMD_CONST REFIID IID_IDeckLinkEncoderPacket = {0xB6,0x93,0xF3,0x6C,0x31,0x6E,0x4A,0xF1,0xB6,0xC2,0xF3,0x89,0xA4,0xBC,0xA6,0x20};
BMD_CONST REFIID IID_IDeckLinkEncoderVideoPacket = {0x4E,0x7F,0xD9,0x44,0xE8,0xC7,0x4E,0xAC,0xB8,0xC0,0x7B,0x77,0xF8,0x0F,0x5A,0xE0};
BMD_CONST REFIID IID_IDeckLinkEncoderAudioPacket = {0x49,0xE8,0xED,0xC8,0x69,0x3B,0x4E,0x14,0x8E,0xF6,0x12,0xC6,0x58,0xF5,0xA0,0x7A};
BMD_CONST REFIID IID_IDeckLinkH265NALPacket = {0x63,0x9C,0x8E,0x0B,0x68,0xD5,0x4B,0xDE,0xA6,0xD4,0x95,0xF3,0xAE,0xAF,0xF2,0xE7};
BMD_CONST REFIID IID_IDeckLinkAudioInputPacket = {0xE4,0x3D,0x58,0x70,0x28,0x94,0x11,0xDE,0x8C,0x30,0x08,0x00,0x20,0x0C,0x9A,0x66};
BMD_CONST REFIID IID_IDeckLinkScreenPreviewCallback = {0xB1,0xD3,0xF4,0x9A,0x85,0xFE,0x4C,0x5D,0x95,0xC8,0x0B,0x5D,0x5D,0xCC,0xD4,0x38};
BMD_CONST REFIID IID_IDeckLinkGLScreenPreviewHelper = {0x50,0x4E,0x22,0x09,0xCA,0xC7,0x4C,0x1A,0x9F,0xB4,0xC5,0xBB,0x62,0x74,0xD2,0x2F};
BMD_CONST REFIID IID_IDeckLinkNotificationCallback = {0xB0,0x02,0xA1,0xEC,0x07,0x0D,0x42,0x88,0x82,0x89,0xBD,0x5D,0x36,0xE5,0xFF,0x0D};
BMD_CONST REFIID IID_IDeckLinkNotification = {0x0A,0x1F,0xB2,0x07,0xE2,0x15,0x44,0x1B,0x9B,0x19,0x6F,0xA1,0x57,0x59,0x46,0xC5};
BMD_CONST REFIID IID_IDeckLinkAttributes = {0xAB,0xC1,0x18,0x43,0xD9,0x66,0x44,0xCB,0x96,0xE2,0xA1,0xCB,0x5D,0x31,0x35,0xC4};
BMD_CONST REFIID IID_IDeckLinkStatus = {0x5F,0x55,0x82,0x00,0x40,0x28,0x49,0xBC,0xBE,0xAC,0xDB,0x3F,0xA4,0xA9,0x6E,0x46};
BMD_CONST REFIID IID_IDeckLinkKeyer = {0x89,0xAF,0xCA,0xF5,0x65,0xF8,0x42,0x1E,0x98,0xF7,0x96,0xFE,0x5F,0x5B,0xFB,0xA3};
BMD_CONST REFIID IID_IDeckLinkVideoConversion = {0x3B,0xBC,0xB8,0xA2,0xDA,0x2C,0x42,0xD9,0xB5,0xD8,0x88,0x08,0x36,0x44,0xE9,0x9A};
BMD_CONST REFIID IID_IDeckLinkDeviceNotificationCallback = {0x49,0x97,0x05,0x3B,0x0A,0xDF,0x4C,0xC8,0xAC,0x70,0x7A,0x50,0xC4,0xBE,0x72,0x8F};
BMD_CONST REFIID IID_IDeckLinkDiscovery = {0xCD,0xBF,0x63,0x1C,0xBC,0x76,0x45,0xFA,0xB4,0x4D,0xC5,0x50,0x59,0xBC,0x61,0x01};



typedef uint32_t BMDVideoOutputFlags;
enum _BMDVideoOutputFlags {
bmdVideoOutputFlagDefault = 0,
bmdVideoOutputVANC = 1 << 0,
bmdVideoOutputVITC = 1 << 1,
bmdVideoOutputRP188 = 1 << 2,
bmdVideoOutputDualStream3D = 1 << 4
};



typedef uint32_t BMDPacketType;
enum _BMDPacketType {
bmdPacketTypeStreamInterruptedMarker = 0x73696E74, 
bmdPacketTypeStreamData = 0x73646174 
};



typedef uint32_t BMDFrameFlags;
enum _BMDFrameFlags {
bmdFrameFlagDefault = 0,
bmdFrameFlagFlipVertical = 1 << 0,
bmdFrameContainsHDRMetadata = 1 << 1,
bmdFrameContainsCintelMetadata = 1 << 2,



bmdFrameCapturedAsPsF = 1 << 30,
bmdFrameHasNoInputSource = 1 << 31
};



typedef uint32_t BMDVideoInputFlags;
enum _BMDVideoInputFlags {
bmdVideoInputFlagDefault = 0,
bmdVideoInputEnableFormatDetection = 1 << 0,
bmdVideoInputDualStream3D = 1 << 1
};



typedef uint32_t BMDVideoInputFormatChangedEvents;
enum _BMDVideoInputFormatChangedEvents {
bmdVideoInputDisplayModeChanged = 1 << 0,
bmdVideoInputFieldDominanceChanged = 1 << 1,
bmdVideoInputColorspaceChanged = 1 << 2
};



typedef uint32_t BMDDetectedVideoInputFormatFlags;
enum _BMDDetectedVideoInputFormatFlags {
bmdDetectedVideoInputYCbCr422 = 1 << 0,
bmdDetectedVideoInputRGB444 = 1 << 1,
bmdDetectedVideoInputDualStream3D = 1 << 2
};



typedef uint32_t BMDDeckLinkCapturePassthroughMode;
enum _BMDDeckLinkCapturePassthroughMode {
bmdDeckLinkCapturePassthroughModeDisabled = 0x70646973,
bmdDeckLinkCapturePassthroughModeDirect = 0x70646972,
bmdDeckLinkCapturePassthroughModeCleanSwitch = 0x70636C6E
};



typedef uint32_t BMDOutputFrameCompletionResult;
enum _BMDOutputFrameCompletionResult {
bmdOutputFrameCompleted, 
bmdOutputFrameDisplayedLate, 
bmdOutputFrameDropped, 
bmdOutputFrameFlushed 
};



typedef uint32_t BMDReferenceStatus;
enum _BMDReferenceStatus {
bmdReferenceNotSupportedByHardware = 1 << 0,
bmdReferenceLocked = 1 << 1
};



typedef uint32_t BMDAudioFormat;
enum _BMDAudioFormat {
bmdAudioFormatPCM = 0x6C70636D 
};



typedef uint32_t BMDAudioSampleRate;
enum _BMDAudioSampleRate {
bmdAudioSampleRate48kHz = 48000
};



typedef uint32_t BMDAudioSampleType;
enum _BMDAudioSampleType {
bmdAudioSampleType16bitInteger = 16,
bmdAudioSampleType32bitInteger = 32
};



typedef uint32_t BMDAudioOutputStreamType;
enum _BMDAudioOutputStreamType {
bmdAudioOutputStreamContinuous, 
bmdAudioOutputStreamContinuousDontResample, 
bmdAudioOutputStreamTimestamped 
};



typedef uint32_t BMDDisplayModeSupport;
enum _BMDDisplayModeSupport {
bmdDisplayModeNotSupported = 0,
bmdDisplayModeSupported, 
bmdDisplayModeSupportedWithConversion 
};



typedef uint32_t BMDAncillaryPacketFormat;
enum _BMDAncillaryPacketFormat {
bmdAncillaryPacketFormatUInt8 = 0x75693038,
bmdAncillaryPacketFormatUInt16 = 0x75693136,
bmdAncillaryPacketFormatYCbCr10 = 0x76323130
};



typedef uint32_t BMDTimecodeFormat;
enum _BMDTimecodeFormat {
bmdTimecodeRP188VITC1 = 0x72707631, 
bmdTimecodeRP188VITC2 = 0x72703132, 
bmdTimecodeRP188LTC = 0x72706C74, 
bmdTimecodeRP188Any = 0x72703138, 
bmdTimecodeVITC = 0x76697463,
bmdTimecodeVITCField2 = 0x76697432,
bmdTimecodeSerial = 0x73657269
};



typedef uint32_t BMDAnalogVideoFlags;
enum _BMDAnalogVideoFlags {
bmdAnalogVideoFlagCompositeSetup75 = 1 << 0,
bmdAnalogVideoFlagComponentBetacamLevels = 1 << 1
};



typedef uint32_t BMDAudioOutputAnalogAESSwitch;
enum _BMDAudioOutputAnalogAESSwitch {
bmdAudioOutputSwitchAESEBU = 0x61657320,
bmdAudioOutputSwitchAnalog = 0x616E6C67
};



typedef uint32_t BMDVideoOutputConversionMode;
enum _BMDVideoOutputConversionMode {
bmdNoVideoOutputConversion = 0x6E6F6E65,
bmdVideoOutputLetterboxDownconversion = 0x6C746278,
bmdVideoOutputAnamorphicDownconversion = 0x616D7068,
bmdVideoOutputHD720toHD1080Conversion = 0x37323063,
bmdVideoOutputHardwareLetterboxDownconversion = 0x48576C62,
bmdVideoOutputHardwareAnamorphicDownconversion = 0x4857616D,
bmdVideoOutputHardwareCenterCutDownconversion = 0x48576363,
bmdVideoOutputHardware720p1080pCrossconversion = 0x78636170,
bmdVideoOutputHardwareAnamorphic720pUpconversion = 0x75613770,
bmdVideoOutputHardwareAnamorphic1080iUpconversion = 0x75613169,
bmdVideoOutputHardwareAnamorphic149To720pUpconversion = 0x75343770,
bmdVideoOutputHardwareAnamorphic149To1080iUpconversion = 0x75343169,
bmdVideoOutputHardwarePillarbox720pUpconversion = 0x75703770,
bmdVideoOutputHardwarePillarbox1080iUpconversion = 0x75703169
};



typedef uint32_t BMDVideoInputConversionMode;
enum _BMDVideoInputConversionMode {
bmdNoVideoInputConversion = 0x6E6F6E65,
bmdVideoInputLetterboxDownconversionFromHD1080 = 0x31306C62,
bmdVideoInputAnamorphicDownconversionFromHD1080 = 0x3130616D,
bmdVideoInputLetterboxDownconversionFromHD720 = 0x37326C62,
bmdVideoInputAnamorphicDownconversionFromHD720 = 0x3732616D,
bmdVideoInputLetterboxUpconversion = 0x6C627570,
bmdVideoInputAnamorphicUpconversion = 0x616D7570
};



typedef uint32_t BMDVideo3DPackingFormat;
enum _BMDVideo3DPackingFormat {
bmdVideo3DPackingSidebySideHalf = 0x73627368,
bmdVideo3DPackingLinebyLine = 0x6C62796C,
bmdVideo3DPackingTopAndBottom = 0x7461626F,
bmdVideo3DPackingFramePacking = 0x6672706B,
bmdVideo3DPackingLeftOnly = 0x6C656674,
bmdVideo3DPackingRightOnly = 0x72696768
};



typedef uint32_t BMDIdleVideoOutputOperation;
enum _BMDIdleVideoOutputOperation {
bmdIdleVideoOutputBlack = 0x626C6163,
bmdIdleVideoOutputLastFrame = 0x6C616661
};



typedef uint32_t BMDVideoEncoderFrameCodingMode;
enum _BMDVideoEncoderFrameCodingMode {
bmdVideoEncoderFrameCodingModeInter = 0x696E7465,
bmdVideoEncoderFrameCodingModeIntra = 0x696E7472
};



typedef uint32_t BMDDNxHRLevel;
enum _BMDDNxHRLevel {
bmdDNxHRLevelSQ = 0x646E7371,
bmdDNxHRLevelLB = 0x646E6C62,
bmdDNxHRLevelHQ = 0x646E6871,
bmdDNxHRLevelHQX = 0x64687178,
bmdDNxHRLevel444 = 0x64343434
};



typedef uint32_t BMDLinkConfiguration;
enum _BMDLinkConfiguration {
bmdLinkConfigurationSingleLink = 0x6C63736C,
bmdLinkConfigurationDualLink = 0x6C63646C,
bmdLinkConfigurationQuadLink = 0x6C63716C
};



typedef uint32_t BMDDeviceInterface;
enum _BMDDeviceInterface {
bmdDeviceInterfacePCI = 0x70636920,
bmdDeviceInterfaceUSB = 0x75736220,
bmdDeviceInterfaceThunderbolt = 0x7468756E
};



typedef uint32_t BMDColorspace;
enum _BMDColorspace {
bmdColorspaceRec601 = 0x72363031,
bmdColorspaceRec709 = 0x72373039,
bmdColorspaceRec2020 = 0x32303230
};



typedef uint32_t BMDDynamicRange;
enum _BMDDynamicRange {
bmdDynamicRangeSDR = 0,
bmdDynamicRangeHDRStaticPQ = 1 << 29, 
bmdDynamicRangeHDRStaticHLG = 1 << 30 
};



typedef uint32_t BMDDeckLinkHDMIInputEDIDID;
enum _BMDDeckLinkHDMIInputEDIDID {
bmdDeckLinkHDMIInputEDIDDynamicRange = 0x48494479 
};



typedef uint32_t BMDDeckLinkFrameMetadataID;
enum _BMDDeckLinkFrameMetadataID {
bmdDeckLinkFrameMetadataColorspace = 0x63737063, 
bmdDeckLinkFrameMetadataHDRElectroOpticalTransferFunc = 0x656F7466, 
bmdDeckLinkFrameMetadataCintelFilmType = 0x63667479, 
bmdDeckLinkFrameMetadataCintelFilmGauge = 0x63666761, 
bmdDeckLinkFrameMetadataCintelOffsetDetectedHorizontal = 0x6F646668, 
bmdDeckLinkFrameMetadataCintelOffsetDetectedVertical = 0x6F646676, 
bmdDeckLinkFrameMetadataCintelKeykodeLow = 0x636B6B6C, 
bmdDeckLinkFrameMetadataCintelKeykodeHigh = 0x636B6B68, 
bmdDeckLinkFrameMetadataCintelTile1Size = 0x63743173, 
bmdDeckLinkFrameMetadataCintelTile2Size = 0x63743273, 
bmdDeckLinkFrameMetadataCintelTile3Size = 0x63743373, 
bmdDeckLinkFrameMetadataCintelTile4Size = 0x63743473, 
bmdDeckLinkFrameMetadataCintelImageWidth = 0x49575078, 
bmdDeckLinkFrameMetadataCintelImageHeight = 0x49485078, 
bmdDeckLinkFrameMetadataCintelLinearMaskingRedInRed = 0x6D726972, 
bmdDeckLinkFrameMetadataCintelLinearMaskingGreenInRed = 0x6D676972, 
bmdDeckLinkFrameMetadataCintelLinearMaskingBlueInRed = 0x6D626972, 
bmdDeckLinkFrameMetadataCintelLinearMaskingRedInGreen = 0x6D726967, 
bmdDeckLinkFrameMetadataCintelLinearMaskingGreenInGreen = 0x6D676967, 
bmdDeckLinkFrameMetadataCintelLinearMaskingBlueInGreen = 0x6D626967, 
bmdDeckLinkFrameMetadataCintelLinearMaskingRedInBlue = 0x6D726962, 
bmdDeckLinkFrameMetadataCintelLinearMaskingGreenInBlue = 0x6D676962, 
bmdDeckLinkFrameMetadataCintelLinearMaskingBlueInBlue = 0x6D626962, 
bmdDeckLinkFrameMetadataCintelLogMaskingRedInRed = 0x6D6C7272, 
bmdDeckLinkFrameMetadataCintelLogMaskingGreenInRed = 0x6D6C6772, 
bmdDeckLinkFrameMetadataCintelLogMaskingBlueInRed = 0x6D6C6272, 
bmdDeckLinkFrameMetadataCintelLogMaskingRedInGreen = 0x6D6C7267, 
bmdDeckLinkFrameMetadataCintelLogMaskingGreenInGreen = 0x6D6C6767, 
bmdDeckLinkFrameMetadataCintelLogMaskingBlueInGreen = 0x6D6C6267, 
bmdDeckLinkFrameMetadataCintelLogMaskingRedInBlue = 0x6D6C7262, 
bmdDeckLinkFrameMetadataCintelLogMaskingGreenInBlue = 0x6D6C6762, 
bmdDeckLinkFrameMetadataCintelLogMaskingBlueInBlue = 0x6D6C6262, 
bmdDeckLinkFrameMetadataCintelFilmFrameRate = 0x63666672, 
bmdDeckLinkFrameMetadataHDRDisplayPrimariesRedX = 0x68647278, 
bmdDeckLinkFrameMetadataHDRDisplayPrimariesRedY = 0x68647279, 
bmdDeckLinkFrameMetadataHDRDisplayPrimariesGreenX = 0x68646778, 
bmdDeckLinkFrameMetadataHDRDisplayPrimariesGreenY = 0x68646779, 
bmdDeckLinkFrameMetadataHDRDisplayPrimariesBlueX = 0x68646278, 
bmdDeckLinkFrameMetadataHDRDisplayPrimariesBlueY = 0x68646279, 
bmdDeckLinkFrameMetadataHDRWhitePointX = 0x68647778, 
bmdDeckLinkFrameMetadataHDRWhitePointY = 0x68647779, 
bmdDeckLinkFrameMetadataHDRMaxDisplayMasteringLuminance = 0x68646D6C, 
bmdDeckLinkFrameMetadataHDRMinDisplayMasteringLuminance = 0x686D696C, 
bmdDeckLinkFrameMetadataHDRMaximumContentLightLevel = 0x6D636C6C, 
bmdDeckLinkFrameMetadataHDRMaximumFrameAverageLightLevel = 0x66616C6C, 
bmdDeckLinkFrameMetadataCintelOffsetToApplyHorizontal = 0x6F746168, 
bmdDeckLinkFrameMetadataCintelOffsetToApplyVertical = 0x6F746176, 
bmdDeckLinkFrameMetadataCintelGainRed = 0x4C665264, 
bmdDeckLinkFrameMetadataCintelGainGreen = 0x4C664772, 
bmdDeckLinkFrameMetadataCintelGainBlue = 0x4C66426C, 
bmdDeckLinkFrameMetadataCintelLiftRed = 0x476E5264, 
bmdDeckLinkFrameMetadataCintelLiftGreen = 0x476E4772, 
bmdDeckLinkFrameMetadataCintelLiftBlue = 0x476E426C 
};



typedef uint32_t BMDDuplexMode;
enum _BMDDuplexMode {
bmdDuplexModeFull = 0x66647570,
bmdDuplexModeHalf = 0x68647570
};



typedef uint32_t BMDDeckLinkAttributeID;
enum _BMDDeckLinkAttributeID {



BMDDeckLinkSupportsInternalKeying = 0x6B657969,
BMDDeckLinkSupportsExternalKeying = 0x6B657965,
BMDDeckLinkSupportsHDKeying = 0x6B657968,
BMDDeckLinkSupportsInputFormatDetection = 0x696E6664,
BMDDeckLinkHasReferenceInput = 0x6872696E,
BMDDeckLinkHasSerialPort = 0x68737074,
BMDDeckLinkHasAnalogVideoOutputGain = 0x61766F67,
BMDDeckLinkCanOnlyAdjustOverallVideoOutputGain = 0x6F766F67,
BMDDeckLinkHasVideoInputAntiAliasingFilter = 0x6161666C,
BMDDeckLinkHasBypass = 0x62797073,
BMDDeckLinkSupportsClockTimingAdjustment = 0x63746164,
BMDDeckLinkSupportsFullDuplex = 0x66647570,
BMDDeckLinkSupportsFullFrameReferenceInputTimingOffset = 0x6672696E,
BMDDeckLinkSupportsSMPTELevelAOutput = 0x6C766C61,
BMDDeckLinkSupportsDualLinkSDI = 0x73646C73,
BMDDeckLinkSupportsQuadLinkSDI = 0x73716C73,
BMDDeckLinkSupportsIdleOutput = 0x69646F75,
BMDDeckLinkHasLTCTimecodeInput = 0x686C7463,
BMDDeckLinkSupportsDuplexModeConfiguration = 0x64757078,
BMDDeckLinkSupportsHDRMetadata = 0x6864726D,
BMDDeckLinkSupportsColorspaceMetadata = 0x636D6574,



BMDDeckLinkMaximumAudioChannels = 0x6D616368,
BMDDeckLinkMaximumAnalogAudioInputChannels = 0x69616368,
BMDDeckLinkMaximumAnalogAudioOutputChannels = 0x61616368,
BMDDeckLinkNumberOfSubDevices = 0x6E736264,
BMDDeckLinkSubDeviceIndex = 0x73756269,
BMDDeckLinkPersistentID = 0x70656964,
BMDDeckLinkDeviceGroupID = 0x64676964,
BMDDeckLinkTopologicalID = 0x746F6964,
BMDDeckLinkVideoOutputConnections = 0x766F636E, 
BMDDeckLinkVideoInputConnections = 0x7669636E, 
BMDDeckLinkAudioOutputConnections = 0x616F636E, 
BMDDeckLinkAudioInputConnections = 0x6169636E, 
BMDDeckLinkVideoIOSupport = 0x76696F73, 
BMDDeckLinkDeckControlConnections = 0x6463636E, 
BMDDeckLinkDeviceInterface = 0x64627573, 
BMDDeckLinkAudioInputRCAChannelCount = 0x61697263,
BMDDeckLinkAudioInputXLRChannelCount = 0x61697863,
BMDDeckLinkAudioOutputRCAChannelCount = 0x616F7263,
BMDDeckLinkAudioOutputXLRChannelCount = 0x616F7863,
BMDDeckLinkPairedDevicePersistentID = 0x70706964,



BMDDeckLinkVideoInputGainMinimum = 0x7669676D,
BMDDeckLinkVideoInputGainMaximum = 0x76696778,
BMDDeckLinkVideoOutputGainMinimum = 0x766F676D,
BMDDeckLinkVideoOutputGainMaximum = 0x766F6778,
BMDDeckLinkMicrophoneInputGainMinimum = 0x6D69676D,
BMDDeckLinkMicrophoneInputGainMaximum = 0x6D696778,



BMDDeckLinkSerialPortDeviceName = 0x736C706E,
BMDDeckLinkVendorName = 0x766E6472,
BMDDeckLinkDisplayName = 0x6473706E,
BMDDeckLinkModelName = 0x6D646C6E,
BMDDeckLinkDeviceHandle = 0x64657668
};



typedef uint32_t BMDDeckLinkAPIInformationID;
enum _BMDDeckLinkAPIInformationID {
BMDDeckLinkAPIVersion = 0x76657273
};



typedef uint32_t BMDDeckLinkStatusID;
enum _BMDDeckLinkStatusID {



bmdDeckLinkStatusDetectedVideoInputMode = 0x6476696D,
bmdDeckLinkStatusDetectedVideoInputFlags = 0x64766966,
bmdDeckLinkStatusCurrentVideoInputMode = 0x6376696D,
bmdDeckLinkStatusCurrentVideoInputPixelFormat = 0x63766970,
bmdDeckLinkStatusCurrentVideoInputFlags = 0x63766966,
bmdDeckLinkStatusCurrentVideoOutputMode = 0x63766F6D,
bmdDeckLinkStatusCurrentVideoOutputFlags = 0x63766F66,
bmdDeckLinkStatusPCIExpressLinkWidth = 0x70776964,
bmdDeckLinkStatusPCIExpressLinkSpeed = 0x706C6E6B,
bmdDeckLinkStatusLastVideoOutputPixelFormat = 0x6F706978,
bmdDeckLinkStatusReferenceSignalMode = 0x7265666D,
bmdDeckLinkStatusReferenceSignalFlags = 0x72656666,
bmdDeckLinkStatusDuplexMode = 0x64757078,
bmdDeckLinkStatusBusy = 0x62757379,
bmdDeckLinkStatusInterchangeablePanelType = 0x69637074,
bmdDeckLinkStatusDeviceTemperature = 0x64746D70,



bmdDeckLinkStatusVideoInputSignalLocked = 0x7669736C,
bmdDeckLinkStatusReferenceSignalLocked = 0x7265666C,
bmdDeckLinkStatusReceivedEDID = 0x65646964
};



typedef uint32_t BMDDeckLinkVideoStatusFlags;
enum _BMDDeckLinkVideoStatusFlags {
bmdDeckLinkVideoStatusPsF = 1 << 0,
bmdDeckLinkVideoStatusDualStream3D = 1 << 1
};



typedef uint32_t BMDDuplexStatus;
enum _BMDDuplexStatus {
bmdDuplexStatusFullDuplex = 0x66647570,
bmdDuplexStatusHalfDuplex = 0x68647570,
bmdDuplexStatusSimplex = 0x73706C78,
bmdDuplexStatusInactive = 0x696E6163
};



typedef uint32_t BMDPanelType;
enum _BMDPanelType {
bmdPanelNotDetected = 0x6E706E6C,
bmdPanelTeranexMiniSmartPanel = 0x746D736D
};



typedef uint32_t BMDDeviceBusyState;
enum _BMDDeviceBusyState {
bmdDeviceCaptureBusy = 1 << 0,
bmdDevicePlaybackBusy = 1 << 1,
bmdDeviceSerialPortBusy = 1 << 2
};



typedef uint32_t BMDVideoIOSupport;
enum _BMDVideoIOSupport {
bmdDeviceSupportsCapture = 1 << 0,
bmdDeviceSupportsPlayback = 1 << 1
};



typedef uint32_t BMD3DPreviewFormat;
enum _BMD3DPreviewFormat {
bmd3DPreviewFormatDefault = 0x64656661,
bmd3DPreviewFormatLeftOnly = 0x6C656674,
bmd3DPreviewFormatRightOnly = 0x72696768,
bmd3DPreviewFormatSideBySide = 0x73696465,
bmd3DPreviewFormatTopBottom = 0x746F7062
};



typedef uint32_t BMDNotifications;
enum _BMDNotifications {
bmdPreferencesChanged = 0x70726566,
bmdStatusChanged = 0x73746174
};

#if defined(__cplusplus)



class IDeckLinkVideoOutputCallback;
class IDeckLinkInputCallback;
class IDeckLinkEncoderInputCallback;
class IDeckLinkMemoryAllocator;
class IDeckLinkAudioOutputCallback;
class IDeckLinkIterator;
class IDeckLinkAPIInformation;
class IDeckLinkOutput;
class IDeckLinkInput;
class IDeckLinkHDMIInputEDID;
class IDeckLinkEncoderInput;
class IDeckLinkVideoFrame;
class IDeckLinkMutableVideoFrame;
class IDeckLinkVideoFrame3DExtensions;
class IDeckLinkVideoFrameMetadataExtensions;
class IDeckLinkVideoInputFrame;
class IDeckLinkAncillaryPacket;
class IDeckLinkAncillaryPacketIterator;
class IDeckLinkVideoFrameAncillaryPackets;
class IDeckLinkVideoFrameAncillary;
class IDeckLinkEncoderPacket;
class IDeckLinkEncoderVideoPacket;
class IDeckLinkEncoderAudioPacket;
class IDeckLinkH265NALPacket;
class IDeckLinkAudioInputPacket;
class IDeckLinkScreenPreviewCallback;
class IDeckLinkGLScreenPreviewHelper;
class IDeckLinkNotificationCallback;
class IDeckLinkNotification;
class IDeckLinkAttributes;
class IDeckLinkStatus;
class IDeckLinkKeyer;
class IDeckLinkVideoConversion;
class IDeckLinkDeviceNotificationCallback;
class IDeckLinkDiscovery;



class BMD_PUBLIC IDeckLinkVideoOutputCallback : public IUnknown
{
public:
virtual HRESULT ScheduledFrameCompleted ( IDeckLinkVideoFrame *completedFrame, BMDOutputFrameCompletionResult result) = 0;
virtual HRESULT ScheduledPlaybackHasStopped (void) = 0;

protected:
virtual ~IDeckLinkVideoOutputCallback () {} 
};



class BMD_PUBLIC IDeckLinkInputCallback : public IUnknown
{
public:
virtual HRESULT VideoInputFormatChanged ( BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode *newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags) = 0;
virtual HRESULT VideoInputFrameArrived ( IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioPacket) = 0;

protected:
virtual ~IDeckLinkInputCallback () {} 
};



class BMD_PUBLIC IDeckLinkEncoderInputCallback : public IUnknown
{
public:
virtual HRESULT VideoInputSignalChanged ( BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode *newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags) = 0;
virtual HRESULT VideoPacketArrived ( IDeckLinkEncoderVideoPacket* videoPacket) = 0;
virtual HRESULT AudioPacketArrived ( IDeckLinkEncoderAudioPacket* audioPacket) = 0;

protected:
virtual ~IDeckLinkEncoderInputCallback () {} 
};



class BMD_PUBLIC IDeckLinkMemoryAllocator : public IUnknown
{
public:
virtual HRESULT AllocateBuffer ( uint32_t bufferSize, void **allocatedBuffer) = 0;
virtual HRESULT ReleaseBuffer ( void *buffer) = 0;

virtual HRESULT Commit (void) = 0;
virtual HRESULT Decommit (void) = 0;
};



class BMD_PUBLIC IDeckLinkAudioOutputCallback : public IUnknown
{
public:
virtual HRESULT RenderAudioSamples ( bool preroll) = 0;
};



class BMD_PUBLIC IDeckLinkIterator : public IUnknown
{
public:
virtual HRESULT Next ( IDeckLink **deckLinkInstance) = 0;
};



class BMD_PUBLIC IDeckLinkAPIInformation : public IUnknown
{
public:
virtual HRESULT GetFlag ( BMDDeckLinkAPIInformationID cfgID, bool *value) = 0;
virtual HRESULT GetInt ( BMDDeckLinkAPIInformationID cfgID, int64_t *value) = 0;
virtual HRESULT GetFloat ( BMDDeckLinkAPIInformationID cfgID, double *value) = 0;
virtual HRESULT GetString ( BMDDeckLinkAPIInformationID cfgID, const char **value) = 0;

protected:
virtual ~IDeckLinkAPIInformation () {} 
};



class BMD_PUBLIC IDeckLinkOutput : public IUnknown
{
public:
virtual HRESULT DoesSupportVideoMode ( BMDDisplayMode displayMode, BMDPixelFormat pixelFormat, BMDVideoOutputFlags flags, BMDDisplayModeSupport *result, IDeckLinkDisplayMode **resultDisplayMode) = 0;
virtual HRESULT GetDisplayModeIterator ( IDeckLinkDisplayModeIterator **iterator) = 0;

virtual HRESULT SetScreenPreviewCallback ( IDeckLinkScreenPreviewCallback *previewCallback) = 0;



virtual HRESULT EnableVideoOutput ( BMDDisplayMode displayMode, BMDVideoOutputFlags flags) = 0;
virtual HRESULT DisableVideoOutput (void) = 0;

virtual HRESULT SetVideoOutputFrameMemoryAllocator ( IDeckLinkMemoryAllocator *theAllocator) = 0;
virtual HRESULT CreateVideoFrame ( int32_t width, int32_t height, int32_t rowBytes, BMDPixelFormat pixelFormat, BMDFrameFlags flags, IDeckLinkMutableVideoFrame **outFrame) = 0;
virtual HRESULT CreateAncillaryData ( BMDPixelFormat pixelFormat, IDeckLinkVideoFrameAncillary **outBuffer) = 0; 

virtual HRESULT DisplayVideoFrameSync ( IDeckLinkVideoFrame *theFrame) = 0;
virtual HRESULT ScheduleVideoFrame ( IDeckLinkVideoFrame *theFrame, BMDTimeValue displayTime, BMDTimeValue displayDuration, BMDTimeScale timeScale) = 0;
virtual HRESULT SetScheduledFrameCompletionCallback ( IDeckLinkVideoOutputCallback *theCallback) = 0;
virtual HRESULT GetBufferedVideoFrameCount ( uint32_t *bufferedFrameCount) = 0;



virtual HRESULT EnableAudioOutput ( BMDAudioSampleRate sampleRate, BMDAudioSampleType sampleType, uint32_t channelCount, BMDAudioOutputStreamType streamType) = 0;
virtual HRESULT DisableAudioOutput (void) = 0;

virtual HRESULT WriteAudioSamplesSync ( void *buffer, uint32_t sampleFrameCount, uint32_t *sampleFramesWritten) = 0;

virtual HRESULT BeginAudioPreroll (void) = 0;
virtual HRESULT EndAudioPreroll (void) = 0;
virtual HRESULT ScheduleAudioSamples ( void *buffer, uint32_t sampleFrameCount, BMDTimeValue streamTime, BMDTimeScale timeScale, uint32_t *sampleFramesWritten) = 0;

virtual HRESULT GetBufferedAudioSampleFrameCount ( uint32_t *bufferedSampleFrameCount) = 0;
virtual HRESULT FlushBufferedAudioSamples (void) = 0;

virtual HRESULT SetAudioCallback ( IDeckLinkAudioOutputCallback *theCallback) = 0;



virtual HRESULT StartScheduledPlayback ( BMDTimeValue playbackStartTime, BMDTimeScale timeScale, double playbackSpeed) = 0;
virtual HRESULT StopScheduledPlayback ( BMDTimeValue stopPlaybackAtTime, BMDTimeValue *actualStopTime, BMDTimeScale timeScale) = 0;
virtual HRESULT IsScheduledPlaybackRunning ( bool *active) = 0;
virtual HRESULT GetScheduledStreamTime ( BMDTimeScale desiredTimeScale, BMDTimeValue *streamTime, double *playbackSpeed) = 0;
virtual HRESULT GetReferenceStatus ( BMDReferenceStatus *referenceStatus) = 0;



virtual HRESULT GetHardwareReferenceClock ( BMDTimeScale desiredTimeScale, BMDTimeValue *hardwareTime, BMDTimeValue *timeInFrame, BMDTimeValue *ticksPerFrame) = 0;
virtual HRESULT GetFrameCompletionReferenceTimestamp ( IDeckLinkVideoFrame *theFrame, BMDTimeScale desiredTimeScale, BMDTimeValue *frameCompletionTimestamp) = 0;

protected:
virtual ~IDeckLinkOutput () {} 
};



class BMD_PUBLIC IDeckLinkInput : public IUnknown
{
public:
virtual HRESULT DoesSupportVideoMode ( BMDDisplayMode displayMode, BMDPixelFormat pixelFormat, BMDVideoInputFlags flags, BMDDisplayModeSupport *result, IDeckLinkDisplayMode **resultDisplayMode) = 0;
virtual HRESULT GetDisplayModeIterator ( IDeckLinkDisplayModeIterator **iterator) = 0;

virtual HRESULT SetScreenPreviewCallback ( IDeckLinkScreenPreviewCallback *previewCallback) = 0;



virtual HRESULT EnableVideoInput ( BMDDisplayMode displayMode, BMDPixelFormat pixelFormat, BMDVideoInputFlags flags) = 0;
virtual HRESULT DisableVideoInput (void) = 0;
virtual HRESULT GetAvailableVideoFrameCount ( uint32_t *availableFrameCount) = 0;
virtual HRESULT SetVideoInputFrameMemoryAllocator ( IDeckLinkMemoryAllocator *theAllocator) = 0;



virtual HRESULT EnableAudioInput ( BMDAudioSampleRate sampleRate, BMDAudioSampleType sampleType, uint32_t channelCount) = 0;
virtual HRESULT DisableAudioInput (void) = 0;
virtual HRESULT GetAvailableAudioSampleFrameCount ( uint32_t *availableSampleFrameCount) = 0;



virtual HRESULT StartStreams (void) = 0;
virtual HRESULT StopStreams (void) = 0;
virtual HRESULT PauseStreams (void) = 0;
virtual HRESULT FlushStreams (void) = 0;
virtual HRESULT SetCallback ( IDeckLinkInputCallback *theCallback) = 0;



virtual HRESULT GetHardwareReferenceClock ( BMDTimeScale desiredTimeScale, BMDTimeValue *hardwareTime, BMDTimeValue *timeInFrame, BMDTimeValue *ticksPerFrame) = 0;

protected:
virtual ~IDeckLinkInput () {} 
};



class BMD_PUBLIC IDeckLinkHDMIInputEDID : public IUnknown
{
public:
virtual HRESULT SetInt ( BMDDeckLinkHDMIInputEDIDID cfgID, int64_t value) = 0;
virtual HRESULT GetInt ( BMDDeckLinkHDMIInputEDIDID cfgID, int64_t *value) = 0;
virtual HRESULT WriteToEDID (void) = 0;

protected:
virtual ~IDeckLinkHDMIInputEDID () {} 
};



class BMD_PUBLIC IDeckLinkEncoderInput : public IUnknown
{
public:
virtual HRESULT DoesSupportVideoMode ( BMDDisplayMode displayMode, BMDPixelFormat pixelFormat, BMDVideoInputFlags flags, BMDDisplayModeSupport *result, IDeckLinkDisplayMode **resultDisplayMode) = 0;
virtual HRESULT GetDisplayModeIterator ( IDeckLinkDisplayModeIterator **iterator) = 0;



virtual HRESULT EnableVideoInput ( BMDDisplayMode displayMode, BMDPixelFormat pixelFormat, BMDVideoInputFlags flags) = 0;
virtual HRESULT DisableVideoInput (void) = 0;
virtual HRESULT GetAvailablePacketsCount ( uint32_t *availablePacketsCount) = 0;
virtual HRESULT SetMemoryAllocator ( IDeckLinkMemoryAllocator *theAllocator) = 0;



virtual HRESULT EnableAudioInput ( BMDAudioFormat audioFormat, BMDAudioSampleRate sampleRate, BMDAudioSampleType sampleType, uint32_t channelCount) = 0;
virtual HRESULT DisableAudioInput (void) = 0;
virtual HRESULT GetAvailableAudioSampleFrameCount ( uint32_t *availableSampleFrameCount) = 0;



virtual HRESULT StartStreams (void) = 0;
virtual HRESULT StopStreams (void) = 0;
virtual HRESULT PauseStreams (void) = 0;
virtual HRESULT FlushStreams (void) = 0;
virtual HRESULT SetCallback ( IDeckLinkEncoderInputCallback *theCallback) = 0;



virtual HRESULT GetHardwareReferenceClock ( BMDTimeScale desiredTimeScale, BMDTimeValue *hardwareTime, BMDTimeValue *timeInFrame, BMDTimeValue *ticksPerFrame) = 0;

protected:
virtual ~IDeckLinkEncoderInput () {} 
};



class BMD_PUBLIC IDeckLinkVideoFrame : public IUnknown
{
public:
virtual long GetWidth (void) = 0;
virtual long GetHeight (void) = 0;
virtual long GetRowBytes (void) = 0;
virtual BMDPixelFormat GetPixelFormat (void) = 0;
virtual BMDFrameFlags GetFlags (void) = 0;
virtual HRESULT GetBytes ( void **buffer) = 0;

virtual HRESULT GetTimecode ( BMDTimecodeFormat format, IDeckLinkTimecode **timecode) = 0;
virtual HRESULT GetAncillaryData ( IDeckLinkVideoFrameAncillary **ancillary) = 0; 

protected:
virtual ~IDeckLinkVideoFrame () {} 
};



class BMD_PUBLIC IDeckLinkMutableVideoFrame : public IDeckLinkVideoFrame
{
public:
virtual HRESULT SetFlags ( BMDFrameFlags newFlags) = 0;

virtual HRESULT SetTimecode ( BMDTimecodeFormat format, IDeckLinkTimecode *timecode) = 0;
virtual HRESULT SetTimecodeFromComponents ( BMDTimecodeFormat format, uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t frames, BMDTimecodeFlags flags) = 0;
virtual HRESULT SetAncillaryData ( IDeckLinkVideoFrameAncillary *ancillary) = 0;
virtual HRESULT SetTimecodeUserBits ( BMDTimecodeFormat format, BMDTimecodeUserBits userBits) = 0;

protected:
virtual ~IDeckLinkMutableVideoFrame () {} 
};



class BMD_PUBLIC IDeckLinkVideoFrame3DExtensions : public IUnknown
{
public:
virtual BMDVideo3DPackingFormat Get3DPackingFormat (void) = 0;
virtual HRESULT GetFrameForRightEye ( IDeckLinkVideoFrame* *rightEyeFrame) = 0;

protected:
virtual ~IDeckLinkVideoFrame3DExtensions () {} 
};



class BMD_PUBLIC IDeckLinkVideoFrameMetadataExtensions : public IUnknown
{
public:
virtual HRESULT GetInt ( BMDDeckLinkFrameMetadataID metadataID, int64_t *value) = 0;
virtual HRESULT GetFloat ( BMDDeckLinkFrameMetadataID metadataID, double *value) = 0;
virtual HRESULT GetFlag ( BMDDeckLinkFrameMetadataID metadataID, bool* value) = 0;
virtual HRESULT GetString ( BMDDeckLinkFrameMetadataID metadataID, const char **value) = 0;

protected:
virtual ~IDeckLinkVideoFrameMetadataExtensions () {} 
};



class BMD_PUBLIC IDeckLinkVideoInputFrame : public IDeckLinkVideoFrame
{
public:
virtual HRESULT GetStreamTime ( BMDTimeValue *frameTime, BMDTimeValue *frameDuration, BMDTimeScale timeScale) = 0;
virtual HRESULT GetHardwareReferenceTimestamp ( BMDTimeScale timeScale, BMDTimeValue *frameTime, BMDTimeValue *frameDuration) = 0;

protected:
virtual ~IDeckLinkVideoInputFrame () {} 
};



class BMD_PUBLIC IDeckLinkAncillaryPacket : public IUnknown
{
public:

virtual HRESULT GetBytes ( BMDAncillaryPacketFormat format , const void **data , uint32_t *size ) = 0;
virtual uint8_t GetDID (void) = 0;
virtual uint8_t GetSDID (void) = 0;
virtual uint32_t GetLineNumber (void) = 0; 
virtual uint8_t GetDataStreamIndex (void) = 0; 

protected:
virtual ~IDeckLinkAncillaryPacket () {} 
};



class BMD_PUBLIC IDeckLinkAncillaryPacketIterator : public IUnknown
{
public:
virtual HRESULT Next ( IDeckLinkAncillaryPacket **packet) = 0;

protected:
virtual ~IDeckLinkAncillaryPacketIterator () {} 
};



class BMD_PUBLIC IDeckLinkVideoFrameAncillaryPackets : public IUnknown
{
public:

virtual HRESULT GetPacketIterator ( IDeckLinkAncillaryPacketIterator **iterator) = 0;
virtual HRESULT GetFirstPacketByID ( uint8_t DID, uint8_t SDID, IDeckLinkAncillaryPacket **packet) = 0;
virtual HRESULT AttachPacket ( IDeckLinkAncillaryPacket *packet) = 0; 
virtual HRESULT DetachPacket ( IDeckLinkAncillaryPacket *packet) = 0;
virtual HRESULT DetachAllPackets (void) = 0;

protected:
virtual ~IDeckLinkVideoFrameAncillaryPackets () {} 
};



class BMD_PUBLIC IDeckLinkVideoFrameAncillary : public IUnknown
{
public:

virtual HRESULT GetBufferForVerticalBlankingLine ( uint32_t lineNumber, void **buffer) = 0; 
virtual BMDPixelFormat GetPixelFormat (void) = 0;
virtual BMDDisplayMode GetDisplayMode (void) = 0;

protected:
virtual ~IDeckLinkVideoFrameAncillary () {} 
};



class BMD_PUBLIC IDeckLinkEncoderPacket : public IUnknown
{
public:
virtual HRESULT GetBytes ( void **buffer) = 0;
virtual long GetSize (void) = 0;
virtual HRESULT GetStreamTime ( BMDTimeValue *frameTime, BMDTimeScale timeScale) = 0;
virtual BMDPacketType GetPacketType (void) = 0;

protected:
virtual ~IDeckLinkEncoderPacket () {} 
};



class BMD_PUBLIC IDeckLinkEncoderVideoPacket : public IDeckLinkEncoderPacket
{
public:
virtual BMDPixelFormat GetPixelFormat (void) = 0;
virtual HRESULT GetHardwareReferenceTimestamp ( BMDTimeScale timeScale, BMDTimeValue *frameTime, BMDTimeValue *frameDuration) = 0;

virtual HRESULT GetTimecode ( BMDTimecodeFormat format, IDeckLinkTimecode **timecode) = 0;

protected:
virtual ~IDeckLinkEncoderVideoPacket () {} 
};



class BMD_PUBLIC IDeckLinkEncoderAudioPacket : public IDeckLinkEncoderPacket
{
public:
virtual BMDAudioFormat GetAudioFormat (void) = 0;

protected:
virtual ~IDeckLinkEncoderAudioPacket () {} 
};



class BMD_PUBLIC IDeckLinkH265NALPacket : public IDeckLinkEncoderVideoPacket
{
public:
virtual HRESULT GetUnitType ( uint8_t *unitType) = 0;
virtual HRESULT GetBytesNoPrefix ( void **buffer) = 0;
virtual long GetSizeNoPrefix (void) = 0;

protected:
virtual ~IDeckLinkH265NALPacket () {} 
};



class BMD_PUBLIC IDeckLinkAudioInputPacket : public IUnknown
{
public:
virtual long GetSampleFrameCount (void) = 0;
virtual HRESULT GetBytes ( void **buffer) = 0;
virtual HRESULT GetPacketTime ( BMDTimeValue *packetTime, BMDTimeScale timeScale) = 0;

protected:
virtual ~IDeckLinkAudioInputPacket () {} 
};



class BMD_PUBLIC IDeckLinkScreenPreviewCallback : public IUnknown
{
public:
virtual HRESULT DrawFrame ( IDeckLinkVideoFrame *theFrame) = 0;

protected:
virtual ~IDeckLinkScreenPreviewCallback () {} 
};



class BMD_PUBLIC IDeckLinkGLScreenPreviewHelper : public IUnknown
{
public:



virtual HRESULT InitializeGL (void) = 0;
virtual HRESULT PaintGL (void) = 0;
virtual HRESULT SetFrame ( IDeckLinkVideoFrame *theFrame) = 0;
virtual HRESULT Set3DPreviewFormat ( BMD3DPreviewFormat previewFormat) = 0;

protected:
virtual ~IDeckLinkGLScreenPreviewHelper () {} 
};



class BMD_PUBLIC IDeckLinkNotificationCallback : public IUnknown
{
public:
virtual HRESULT Notify ( BMDNotifications topic, uint64_t param1, uint64_t param2) = 0;
};



class BMD_PUBLIC IDeckLinkNotification : public IUnknown
{
public:
virtual HRESULT Subscribe ( BMDNotifications topic, IDeckLinkNotificationCallback *theCallback) = 0;
virtual HRESULT Unsubscribe ( BMDNotifications topic, IDeckLinkNotificationCallback *theCallback) = 0;
};



class BMD_PUBLIC IDeckLinkAttributes : public IUnknown
{
public:
virtual HRESULT GetFlag ( BMDDeckLinkAttributeID cfgID, bool *value) = 0;
virtual HRESULT GetInt ( BMDDeckLinkAttributeID cfgID, int64_t *value) = 0;
virtual HRESULT GetFloat ( BMDDeckLinkAttributeID cfgID, double *value) = 0;
virtual HRESULT GetString ( BMDDeckLinkAttributeID cfgID, const char **value) = 0;

protected:
virtual ~IDeckLinkAttributes () {} 
};



class BMD_PUBLIC IDeckLinkStatus : public IUnknown
{
public:
virtual HRESULT GetFlag ( BMDDeckLinkStatusID statusID, bool *value) = 0;
virtual HRESULT GetInt ( BMDDeckLinkStatusID statusID, int64_t *value) = 0;
virtual HRESULT GetFloat ( BMDDeckLinkStatusID statusID, double *value) = 0;
virtual HRESULT GetString ( BMDDeckLinkStatusID statusID, const char **value) = 0;
virtual HRESULT GetBytes ( BMDDeckLinkStatusID statusID, void *buffer, uint32_t *bufferSize) = 0;

protected:
virtual ~IDeckLinkStatus () {} 
};



class BMD_PUBLIC IDeckLinkKeyer : public IUnknown
{
public:
virtual HRESULT Enable ( bool isExternal) = 0;
virtual HRESULT SetLevel ( uint8_t level) = 0;
virtual HRESULT RampUp ( uint32_t numberOfFrames) = 0;
virtual HRESULT RampDown ( uint32_t numberOfFrames) = 0;
virtual HRESULT Disable (void) = 0;

protected:
virtual ~IDeckLinkKeyer () {} 
};



class BMD_PUBLIC IDeckLinkVideoConversion : public IUnknown
{
public:
virtual HRESULT ConvertFrame ( IDeckLinkVideoFrame* srcFrame, IDeckLinkVideoFrame* dstFrame) = 0;

protected:
virtual ~IDeckLinkVideoConversion () {} 
};



class BMD_PUBLIC IDeckLinkDeviceNotificationCallback : public IUnknown
{
public:
virtual HRESULT DeckLinkDeviceArrived ( IDeckLink* deckLinkDevice) = 0;
virtual HRESULT DeckLinkDeviceRemoved ( IDeckLink* deckLinkDevice) = 0;

protected:
virtual ~IDeckLinkDeviceNotificationCallback () {} 
};



class BMD_PUBLIC IDeckLinkDiscovery : public IUnknown
{
public:
virtual HRESULT InstallDeviceNotifications ( IDeckLinkDeviceNotificationCallback* deviceNotificationCallback) = 0;
virtual HRESULT UninstallDeviceNotifications (void) = 0;

protected:
virtual ~IDeckLinkDiscovery () {} 
};



extern "C" {

IDeckLinkIterator* BMD_PUBLIC CreateDeckLinkIteratorInstance (void);
IDeckLinkDiscovery* BMD_PUBLIC CreateDeckLinkDiscoveryInstance (void);
IDeckLinkAPIInformation* BMD_PUBLIC CreateDeckLinkAPIInformationInstance (void);
IDeckLinkGLScreenPreviewHelper* BMD_PUBLIC CreateOpenGLScreenPreviewHelper (void);
IDeckLinkVideoConversion* BMD_PUBLIC CreateVideoConversionInstance (void);
IDeckLinkVideoFrameAncillaryPackets* BMD_PUBLIC CreateVideoFrameAncillaryPacketsInstance (void); 

}


#endif 
#endif 
