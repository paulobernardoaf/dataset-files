















#pragma once





#if defined(__cplusplus)
extern "C" {
#endif

#define UTF8_IGNORE_ERROR 0x01
#define UTF8_SKIP_BOM 0x02

size_t utf8_to_wchar(const char *in, size_t insize, wchar_t *out,
size_t outsize, int flags);
size_t wchar_to_utf8(const wchar_t *in, size_t insize, char *out,
size_t outsize, int flags);

#if defined(__cplusplus)
}
#endif
