#include "DeckLinkAPI.h"
#define IID_IDeckLink_v8_0 (REFIID){0x62,0xBF,0xF7,0x5D,0x65,0x69,0x4E,0x55,0x8D,0x4D,0x66,0xAA,0x03,0x82,0x9A,0xBC}
#define IID_IDeckLinkIterator_v8_0 (REFIID){0x74,0xE9,0x36,0xFC,0xCC,0x28,0x4A,0x67,0x81,0xA0,0x1E,0x94,0xE5,0x2D,0x4E,0x69}
#if defined (__cplusplus)
class BMD_PUBLIC IDeckLink_v8_0 : public IUnknown
{
public:
virtual HRESULT GetModelName ( const char **modelName) = 0;
};
class BMD_PUBLIC IDeckLinkIterator_v8_0 : public IUnknown
{
public:
virtual HRESULT Next ( IDeckLink_v8_0 **deckLinkInstance) = 0;
};
extern "C" {
IDeckLinkIterator_v8_0* BMD_PUBLIC CreateDeckLinkIteratorInstance_v8_0 (void);
};
#endif 
