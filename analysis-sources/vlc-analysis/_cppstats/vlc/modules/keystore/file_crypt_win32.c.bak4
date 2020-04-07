



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_keystore.h>
#include "file_crypt.h"

#include <windows.h>
#include <dpapi.h>

typedef BOOL (WINAPI *ProcessFunc)(DATA_BLOB*, LPCWSTR, DATA_BLOB*, PVOID,
CRYPTPROTECT_PROMPTSTRUCT*, DWORD, DATA_BLOB*);

static size_t Process(const uint8_t *p_src, size_t i_src_len, uint8_t **pp_dst, ProcessFunc pf_process)
{
DATA_BLOB input_blob =
{
.cbData = i_src_len,
.pbData = (BYTE*)p_src
};
DATA_BLOB output_blob;

if (pf_process( &input_blob, NULL, NULL, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &output_blob) == FALSE)
return 0;
*pp_dst = malloc(output_blob.cbData);
if( unlikely( *pp_dst == NULL ) )
{
LocalFree( output_blob.pbData );
return 0;
}
memcpy( *pp_dst, output_blob.pbData, output_blob.cbData );
LocalFree( output_blob.pbData );
return output_blob.cbData;
}

static size_t Decrypt( vlc_keystore *p_keystore, void *p_ctx, const uint8_t *p_src,
size_t i_src_len, uint8_t ** pp_dst )
{
VLC_UNUSED( p_keystore );
VLC_UNUSED( p_ctx );



return Process( p_src, i_src_len, pp_dst, (ProcessFunc)&CryptUnprotectData );
}

static size_t Encrypt( vlc_keystore *p_keystore, void *p_ctx, const uint8_t *p_src,
size_t i_src_len, uint8_t ** pp_dst )
{
VLC_UNUSED( p_keystore );
VLC_UNUSED( p_ctx );
return Process( p_src, i_src_len, pp_dst, CryptProtectData );
}

int CryptInit(vlc_keystore *p_keystore, struct crypt *p_crypt)
{
VLC_UNUSED( p_keystore );
p_crypt->pf_decrypt = Decrypt;
p_crypt->pf_encrypt = Encrypt;
return VLC_SUCCESS;
}
