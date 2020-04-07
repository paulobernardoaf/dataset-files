
























#if !defined(__BYTES_H__)
#define __BYTES_H__

#if defined(_WIN32)

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __BYTE_ORDER __LITTLE_ENDIAN
#define __FLOAT_WORD_ORDER __BYTE_ORDER

typedef unsigned char uint8_t;

#else 

#include <sys/param.h>

#if defined(BYTE_ORDER) && !defined(__BYTE_ORDER)
#define __BYTE_ORDER BYTE_ORDER
#endif

#if defined(BIG_ENDIAN) && !defined(__BIG_ENDIAN)
#define __BIG_ENDIAN BIG_ENDIAN
#endif

#if defined(LITTLE_ENDIAN) && !defined(__LITTLE_ENDIAN)
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#endif

#endif 


#if !defined(__LITTLE_ENDIAN)
#define __LITTLE_ENDIAN 1234
#endif

#if !defined(__BIG_ENDIAN)
#define __BIG_ENDIAN 4321
#endif

#if !defined(__BYTE_ORDER)
#warning "Byte order not defined on your system, assuming little endian!"
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif


#if !defined(__FLOAT_WORD_ORDER)

#define __FLOAT_WORD_ORDER __BYTE_ORDER
#endif

#if !defined(__BYTE_ORDER) || !defined(__FLOAT_WORD_ORDER)
#error "Undefined byte or float word order!"
#endif

#if __FLOAT_WORD_ORDER != __BIG_ENDIAN && __FLOAT_WORD_ORDER != __LITTLE_ENDIAN
#error "Unknown/unsupported float word order!"
#endif

#if __BYTE_ORDER != __BIG_ENDIAN && __BYTE_ORDER != __LITTLE_ENDIAN
#error "Unknown/unsupported byte order!"
#endif

#endif

