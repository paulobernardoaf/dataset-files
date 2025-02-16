




















#include "dshow_capture.h"

DECLARE_QUERYINTERFACE(libAVEnumMediaTypes,
{ {&IID_IUnknown,0}, {&IID_IEnumMediaTypes,0} })
DECLARE_ADDREF(libAVEnumMediaTypes)
DECLARE_RELEASE(libAVEnumMediaTypes)

long WINAPI
libAVEnumMediaTypes_Next(libAVEnumMediaTypes *this, unsigned long n,
AM_MEDIA_TYPE **types, unsigned long *fetched)
{
int count = 0;
dshowdebug("libAVEnumMediaTypes_Next(%p)\n", this);
if (!types)
return E_POINTER;
if (!this->pos && n == 1) {
if (!IsEqualGUID(&this->type.majortype, &GUID_NULL)) {
AM_MEDIA_TYPE *type = av_malloc(sizeof(AM_MEDIA_TYPE));
if (!type)
return E_OUTOFMEMORY;
ff_copy_dshow_media_type(type, &this->type);
*types = type;
count = 1;
}
this->pos = 1;
}
if (fetched)
*fetched = count;
if (!count)
return S_FALSE;
return S_OK;
}
long WINAPI
libAVEnumMediaTypes_Skip(libAVEnumMediaTypes *this, unsigned long n)
{
dshowdebug("libAVEnumMediaTypes_Skip(%p)\n", this);
if (n) 
return S_FALSE;
return S_OK;
}
long WINAPI
libAVEnumMediaTypes_Reset(libAVEnumMediaTypes *this)
{
dshowdebug("libAVEnumMediaTypes_Reset(%p)\n", this);
this->pos = 0;
return S_OK;
}
long WINAPI
libAVEnumMediaTypes_Clone(libAVEnumMediaTypes *this, libAVEnumMediaTypes **enums)
{
libAVEnumMediaTypes *new;
dshowdebug("libAVEnumMediaTypes_Clone(%p)\n", this);
if (!enums)
return E_POINTER;
new = libAVEnumMediaTypes_Create(&this->type);
if (!new)
return E_OUTOFMEMORY;
new->pos = this->pos;
*enums = new;
return S_OK;
}

static int
libAVEnumMediaTypes_Setup(libAVEnumMediaTypes *this, const AM_MEDIA_TYPE *type)
{
IEnumMediaTypesVtbl *vtbl = this->vtbl;
SETVTBL(vtbl, libAVEnumMediaTypes, QueryInterface);
SETVTBL(vtbl, libAVEnumMediaTypes, AddRef);
SETVTBL(vtbl, libAVEnumMediaTypes, Release);
SETVTBL(vtbl, libAVEnumMediaTypes, Next);
SETVTBL(vtbl, libAVEnumMediaTypes, Skip);
SETVTBL(vtbl, libAVEnumMediaTypes, Reset);
SETVTBL(vtbl, libAVEnumMediaTypes, Clone);

if (!type) {
this->type.majortype = GUID_NULL;
} else {
ff_copy_dshow_media_type(&this->type, type);
}

return 1;
}
DECLARE_CREATE(libAVEnumMediaTypes, libAVEnumMediaTypes_Setup(this, type), const AM_MEDIA_TYPE *type)
DECLARE_DESTROY(libAVEnumMediaTypes, nothing)
