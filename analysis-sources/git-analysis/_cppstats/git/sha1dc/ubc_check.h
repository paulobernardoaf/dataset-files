#if defined(__cplusplus)
extern "C" {
#endif
#if !defined(SHA1DC_NO_STANDARD_INCLUDES)
#include <stdint.h>
#endif
#define DVMASKSIZE 1
typedef struct { int dvType; int dvK; int dvB; int testt; int maski; int maskb; uint32_t dm[80]; } dv_info_t;
extern dv_info_t sha1_dvs[];
void ubc_check(const uint32_t W[80], uint32_t dvmask[DVMASKSIZE]);
#define DOSTORESTATE58
#define DOSTORESTATE65
#define CHECK_DVMASK(_DVMASK) (0 != _DVMASK[0])
#if defined(__cplusplus)
}
#endif
#if defined(SHA1DC_CUSTOM_TRAILING_INCLUDE_UBC_CHECK_H)
#include SHA1DC_CUSTOM_TRAILING_INCLUDE_UBC_CHECK_H
#endif
