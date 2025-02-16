#include <deque>
namespace dshow {
struct VLCMediaSample
{
ComPtr<IMediaSample> p_sample;
vlc_tick_t i_timestamp;
};
void WINAPI FreeMediaType( AM_MEDIA_TYPE& mt );
HRESULT WINAPI CopyMediaType( AM_MEDIA_TYPE *pmtTarget,
const AM_MEDIA_TYPE *pmtSource );
int GetFourCCFromMediaType(const AM_MEDIA_TYPE &media_type);
class CaptureFilter;
class CapturePin: public IPin, public IMemInputPin
{
friend class CaptureEnumMediaTypes;
vlc_object_t *p_input;
access_sys_t *p_sys;
CaptureFilter* p_filter;
ComPtr<IPin> p_connected_pin;
AM_MEDIA_TYPE *media_types;
size_t media_type_count;
AM_MEDIA_TYPE cx_media_type;
std::deque<VLCMediaSample> samples_queue;
long i_ref;
public:
CapturePin( vlc_object_t *_p_input, access_sys_t *p_sys,
CaptureFilter* _p_filter,
AM_MEDIA_TYPE *mt, size_t mt_count );
STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
STDMETHODIMP_(ULONG) AddRef();
STDMETHODIMP_(ULONG) Release();
STDMETHODIMP Connect( IPin * pReceivePin, const AM_MEDIA_TYPE *pmt );
STDMETHODIMP ReceiveConnection( IPin * pConnector,
const AM_MEDIA_TYPE *pmt );
STDMETHODIMP Disconnect();
STDMETHODIMP ConnectedTo( IPin **pPin );
STDMETHODIMP ConnectionMediaType( AM_MEDIA_TYPE *pmt );
STDMETHODIMP QueryPinInfo( PIN_INFO * pInfo );
STDMETHODIMP QueryDirection( PIN_DIRECTION * pPinDir );
STDMETHODIMP QueryId( LPWSTR * Id );
STDMETHODIMP QueryAccept( const AM_MEDIA_TYPE *pmt );
STDMETHODIMP EnumMediaTypes( IEnumMediaTypes **ppEnum );
STDMETHODIMP QueryInternalConnections( IPin* *apPin, ULONG *nPin );
STDMETHODIMP EndOfStream( void );
STDMETHODIMP BeginFlush( void );
STDMETHODIMP EndFlush( void );
STDMETHODIMP NewSegment( REFERENCE_TIME tStart, REFERENCE_TIME tStop,
double dRate );
STDMETHODIMP GetAllocator( IMemAllocator **ppAllocator );
STDMETHODIMP NotifyAllocator( IMemAllocator *pAllocator, BOOL bReadOnly );
STDMETHODIMP GetAllocatorRequirements( ALLOCATOR_PROPERTIES *pProps );
STDMETHODIMP Receive( IMediaSample *pSample );
STDMETHODIMP ReceiveMultiple( IMediaSample **pSamples, long nSamples,
long *nSamplesProcessed );
STDMETHODIMP ReceiveCanBlock( void );
HRESULT CustomGetSample( VLCMediaSample * );
HRESULT CustomGetSamples( std::deque<VLCMediaSample> &external_queue );
AM_MEDIA_TYPE &CustomGetMediaType();
private:
virtual ~CapturePin();
};
class CaptureFilter : public IBaseFilter
{
friend class CapturePin;
vlc_object_t *p_input;
ComPtr<CapturePin> p_pin;
ComPtr<IFilterGraph> p_graph;
FILTER_STATE state;
long i_ref;
public:
CaptureFilter( vlc_object_t *_p_input, access_sys_t *p_sys,
AM_MEDIA_TYPE *mt, size_t mt_count );
STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
STDMETHODIMP_(ULONG) AddRef();
STDMETHODIMP_(ULONG) Release();
STDMETHODIMP GetClassID(CLSID *pClsID);
STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);
STDMETHODIMP SetSyncSource(IReferenceClock *pClock);
STDMETHODIMP GetSyncSource(IReferenceClock **pClock);
STDMETHODIMP Stop();
STDMETHODIMP Pause();
STDMETHODIMP Run(REFERENCE_TIME tStart);
STDMETHODIMP EnumPins( IEnumPins ** ppEnum );
STDMETHODIMP FindPin( LPCWSTR Id, IPin ** ppPin );
STDMETHODIMP QueryFilterInfo( FILTER_INFO * pInfo );
STDMETHODIMP JoinFilterGraph( IFilterGraph * pGraph, LPCWSTR pName );
STDMETHODIMP QueryVendorInfo( LPWSTR* pVendorInfo );
ComPtr<CapturePin>& CustomGetPin();
private:
virtual ~CaptureFilter();
};
class CaptureEnumPins : public IEnumPins
{
vlc_object_t *p_input;
ComPtr<CaptureFilter> p_filter;
int i_position;
long i_ref;
public:
CaptureEnumPins( vlc_object_t *_p_input, ComPtr<CaptureFilter> _p_filter,
ComPtr<CaptureEnumPins> pEnumPins );
STDMETHODIMP QueryInterface( REFIID riid, void **ppv );
STDMETHODIMP_(ULONG) AddRef();
STDMETHODIMP_(ULONG) Release();
STDMETHODIMP Next( ULONG cPins, IPin ** ppPins, ULONG * pcFetched );
STDMETHODIMP Skip( ULONG cPins );
STDMETHODIMP Reset();
STDMETHODIMP Clone( IEnumPins **ppEnum );
private:
virtual ~CaptureEnumPins();
};
class CaptureEnumMediaTypes : public IEnumMediaTypes
{
vlc_object_t *p_input;
ComPtr<CapturePin> p_pin;
AM_MEDIA_TYPE cx_media_type;
size_t i_position;
long i_ref;
public:
CaptureEnumMediaTypes( vlc_object_t *_p_input, ComPtr<CapturePin> _p_pin,
CaptureEnumMediaTypes *pEnumMediaTypes );
STDMETHODIMP QueryInterface( REFIID riid, void **ppv );
STDMETHODIMP_(ULONG) AddRef();
STDMETHODIMP_(ULONG) Release();
STDMETHODIMP Next( ULONG cMediaTypes, AM_MEDIA_TYPE ** ppMediaTypes,
ULONG * pcFetched );
STDMETHODIMP Skip( ULONG cMediaTypes );
STDMETHODIMP Reset();
STDMETHODIMP Clone( IEnumMediaTypes **ppEnum );
private:
virtual ~CaptureEnumMediaTypes();
};
} 
