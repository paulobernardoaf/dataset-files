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
BMD_CONST REFIID IID_IDeckLink = {0xC4,0x18,0xFB,0xDD,0x05,0x87,0x48,0xED,0x8F,0xE5,0x64,0x0F,0x0A,0x14,0xAF,0x91};
class IDeckLink;
class BMD_PUBLIC IDeckLink : public IUnknown
{
public:
virtual HRESULT GetModelName ( CFStringRef *modelName) = 0;
virtual HRESULT GetDisplayName ( CFStringRef *displayName) = 0;
protected:
virtual ~IDeckLink () {} 
};
extern "C" {
}
