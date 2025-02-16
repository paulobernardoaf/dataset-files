

























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_codec.h>

#if !defined(_WIN32)
#define LOADER
#else
#include <objbase.h>
#endif

#if defined(LOADER)
#include <wine/winerror.h>
#include <wine/windef.h>
#endif

#include <vlc_codecs.h>
#include "dmo.h"

static long STDCALL QueryInterface( IUnknown *This,
const GUID *riid, void **ppv )
{
CMediaBuffer *p_mb = (CMediaBuffer *)This;
if( !memcmp( riid, &IID_IUnknown, sizeof(GUID) ) ||
!memcmp( riid, &IID_IMediaBuffer, sizeof(GUID) ) )
{
p_mb->i_ref++;
*ppv = (void *)This;
return NOERROR;
}
else
{
*ppv = NULL;
return E_NOINTERFACE;
}
}

static long STDCALL AddRef( IUnknown *This )
{
CMediaBuffer *p_mb = (CMediaBuffer *)This;
return p_mb->i_ref++;
}

static long STDCALL Release( IUnknown *This )
{
CMediaBuffer *p_mb = (CMediaBuffer *)This;
p_mb->i_ref--;

if( p_mb->i_ref == 0 )
{
if( p_mb->b_own ) block_Release( p_mb->p_block );
free( p_mb->vt );
free( p_mb );
}

return 0;
}

static long STDCALL SetLength( IMediaBuffer *This, uint32_t cbLength )
{
CMediaBuffer *p_mb = (CMediaBuffer *)This;
if( cbLength > (uint32_t)p_mb->i_max_size ) return E_INVALIDARG;
p_mb->p_block->i_buffer = cbLength;
return S_OK;
}

static long STDCALL GetMaxLength( IMediaBuffer *This, uint32_t *pcbMaxLength )
{
CMediaBuffer *p_mb = (CMediaBuffer *)This;
if( !pcbMaxLength ) return E_POINTER;
*pcbMaxLength = p_mb->i_max_size;
return S_OK;
}

static long STDCALL GetBufferAndLength( IMediaBuffer *This,
char **ppBuffer, uint32_t *pcbLength )
{
CMediaBuffer *p_mb = (CMediaBuffer *)This;

if( !ppBuffer && !pcbLength ) return E_POINTER;
if( ppBuffer ) *ppBuffer = (char*)p_mb->p_block->p_buffer;
if( pcbLength ) *pcbLength = p_mb->p_block->i_buffer;
return S_OK;
}

CMediaBuffer *CMediaBufferCreate( block_t *p_block, int i_max_size,
bool b_own )
{
CMediaBuffer *p_mb = (CMediaBuffer *)malloc( sizeof(CMediaBuffer) );
if( !p_mb ) return NULL;

p_mb->vt = (IMediaBuffer_vt *)malloc( sizeof(IMediaBuffer_vt) );
if( !p_mb->vt )
{
free( p_mb );
return NULL;
}

p_mb->i_ref = 1;
p_mb->p_block = p_block;
p_mb->i_max_size = i_max_size;
p_mb->b_own = b_own;

p_mb->vt->QueryInterface = QueryInterface;
p_mb->vt->AddRef = AddRef;
p_mb->vt->Release = Release;

p_mb->vt->SetLength = SetLength;
p_mb->vt->GetMaxLength = GetMaxLength;
p_mb->vt->GetBufferAndLength = GetBufferAndLength;

return p_mb;
}
