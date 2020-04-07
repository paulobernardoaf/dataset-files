#if !defined(R_PUNYCODE_H)
#define R_PUNYCODE_H

#if defined(__cplusplus)
extern "C" {
#endif

R_API char *r_punycode_encode(const ut8 *src, int srclen, int *dstlen);
R_API char *r_punycode_decode(const char *src, int srclen, int *dstlen);

#if defined(__cplusplus)
}
#endif

#endif 
