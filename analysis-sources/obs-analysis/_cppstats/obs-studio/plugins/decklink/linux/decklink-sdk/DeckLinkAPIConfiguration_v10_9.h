#include "DeckLinkAPIConfiguration.h"
BMD_CONST REFIID IID_IDeckLinkConfiguration_v10_9 = {0xCB,0x71,0x73,0x4A,0xFE,0x37,0x4E,0x8D,0x8E,0x13,0x80,0x21,0x33,0xA1,0xC3,0xF2};
class IDeckLinkConfiguration_v10_9;
class BMD_PUBLIC IDeckLinkConfiguration_v10_9 : public IUnknown
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
virtual ~IDeckLinkConfiguration_v10_9 () {} 
};
