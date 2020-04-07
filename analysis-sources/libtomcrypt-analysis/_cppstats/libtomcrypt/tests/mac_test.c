#include <tomcrypt_test.h>
int mac_test(void)
{
#if defined(LTC_HMAC)
DO(hmac_test());
#endif
#if defined(LTC_PMAC)
DO(pmac_test());
#endif
#if defined(LTC_OMAC)
DO(omac_test());
#endif
#if defined(LTC_XCBC)
DO(xcbc_test());
#endif
#if defined(LTC_F9_MODE)
DO(f9_test());
#endif
#if defined(LTC_EAX_MODE)
DO(eax_test());
#endif
#if defined(LTC_OCB_MODE)
DO(ocb_test());
#endif
#if defined(LTC_OCB3_MODE)
DO(ocb3_test());
#endif
#if defined(LTC_CCM_MODE)
DO(ccm_test());
#endif
#if defined(LTC_GCM_MODE)
DO(gcm_test());
#endif
#if defined(LTC_PELICAN)
DO(pelican_test());
#endif
#if defined(LTC_POLY1305)
DO(poly1305_test());
#endif
#if defined(LTC_CHACHA20POLY1305_MODE)
DO(chacha20poly1305_test());
#endif
#if defined(LTC_BLAKE2SMAC)
DO(blake2smac_test());
#endif
#if defined(LTC_BLAKE2BMAC)
DO(blake2bmac_test());
#endif
return 0;
}
