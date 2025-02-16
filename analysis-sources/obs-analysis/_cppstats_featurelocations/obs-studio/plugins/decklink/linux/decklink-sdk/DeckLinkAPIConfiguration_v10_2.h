


























#if !defined(BMD_DECKLINKAPICONFIGURATION_v10_2_H)
#define BMD_DECKLINKAPICONFIGURATION_v10_2_H

#include "DeckLinkAPIConfiguration.h"



BMD_CONST REFIID IID_IDeckLinkConfiguration_v10_2 = {0xC6,0x79,0xA3,0x5B,0x61,0x0C,0x4D,0x09,0xB7,0x48,0x1D,0x04,0x78,0x10,0x0F,0xC0};



class IDeckLinkConfiguration_v10_2;



class BMD_PUBLIC IDeckLinkConfiguration_v10_2 : public IUnknown
{
public:
virtual HRESULT SetFlag ( BMDDeckLinkConfigurationID cfgID, bool value) = 0;
virtual HRESULT GetFlag ( BMDDeckLinkConfigurationID cfgID, bool *value) = 0;
virtual HRESULT SetInt ( BMDDeckLinkConfigurationID cfgID, int64_t value) = 0;
virtual HRESULT GetInt ( BMDDeckLinkConfigurationID cfgID, int64_t *value) = 0;
virtual HRESULT SetFloat ( BMDDeckLinkConfigurationID cfgID, double value) = 0;
virtual HRESULT GetFloat ( BMDDeckLinkConfigurationID cfgID, double *value) = 0;
virtual HRESULT SetString ( BMDDeckLinkConfigurationID cfgID, const char *value) = 0;
virtual HRESULT GetString ( BMDDeckLinkConfigurationID cfgID, const char **value) = 0;
virtual HRESULT WriteConfigurationToPreferences (void) = 0;

protected:
virtual ~IDeckLinkConfiguration_v10_2 () {} 
};

#endif 
