


























#if !defined(BMD_DECKLINKAPI_v10_2_H)
#define BMD_DECKLINKAPI_v10_2_H

#include "DeckLinkAPI.h"





typedef uint32_t BMDDeckLinkConfigurationID_v10_2;
enum _BMDDeckLinkConfigurationID_v10_2 {


bmdDeckLinkConfig3GBpsVideoOutput_v10_2 = '3gbs',
};



typedef uint32_t BMDAudioConnection_v10_2;
enum _BMDAudioConnection_v10_2 {
bmdAudioConnectionEmbedded_v10_2 = 0x656D6264,
bmdAudioConnectionAESEBU_v10_2 = 0x61657320,
bmdAudioConnectionAnalog_v10_2 = 0x616E6C67,
bmdAudioConnectionAnalogXLR_v10_2 = 0x61786C72,
bmdAudioConnectionAnalogRCA_v10_2 = 0x61726361
};

#endif 
