#include "./ptypes.h" 
#include <stddef.h>
#if defined(__cplusplus)
#define U_CFUNC extern "C"
#define U_CDECL_BEGIN extern "C" {
#define U_CDECL_END }
#else
#define U_CFUNC extern
#define U_CDECL_BEGIN
#define U_CDECL_END
#endif
#if !defined(U_ATTRIBUTE_DEPRECATED)
#if U_GCC_MAJOR_MINOR >= 302
#define U_ATTRIBUTE_DEPRECATED __attribute__ ((deprecated))
#elif defined(_MSC_VER) && (_MSC_VER >= 1400)
#define U_ATTRIBUTE_DEPRECATED __declspec(deprecated)
#else
#define U_ATTRIBUTE_DEPRECATED
#endif
#endif
#define U_CAPI U_CFUNC U_EXPORT
#define U_STABLE U_CAPI
#define U_DRAFT U_CAPI
#define U_DEPRECATED U_CAPI U_ATTRIBUTE_DEPRECATED
#define U_OBSOLETE U_CAPI
#define U_INTERNAL U_CAPI
#if !defined(U_OVERRIDE)
#define U_OVERRIDE override
#endif
#if !defined(U_FINAL) || defined(U_IN_DOXYGEN)
#define U_FINAL final
#endif
#if !defined(UPRV_BLOCK_MACRO_BEGIN)
#define UPRV_BLOCK_MACRO_BEGIN do
#endif
#if !defined(UPRV_BLOCK_MACRO_END)
#define UPRV_BLOCK_MACRO_END while (FALSE)
#endif
#if !defined(INT8_MIN)
#define INT8_MIN ((int8_t)(-128))
#endif
#if !defined(INT16_MIN)
#define INT16_MIN ((int16_t)(-32767-1))
#endif
#if !defined(INT32_MIN)
#define INT32_MIN ((int32_t)(-2147483647-1))
#endif
#if !defined(INT8_MAX)
#define INT8_MAX ((int8_t)(127))
#endif
#if !defined(INT16_MAX)
#define INT16_MAX ((int16_t)(32767))
#endif
#if !defined(INT32_MAX)
#define INT32_MAX ((int32_t)(2147483647))
#endif
#if !defined(UINT8_MAX)
#define UINT8_MAX ((uint8_t)(255U))
#endif
#if !defined(UINT16_MAX)
#define UINT16_MAX ((uint16_t)(65535U))
#endif
#if !defined(UINT32_MAX)
#define UINT32_MAX ((uint32_t)(4294967295U))
#endif
#if defined(U_INT64_T_UNAVAILABLE)
#error int64_t is required for decimal format and rule-based number format.
#else
#if !defined(INT64_C)
#define INT64_C(c) c ##LL
#endif
#if !defined(UINT64_C)
#define UINT64_C(c) c ##ULL
#endif
#if !defined(U_INT64_MIN)
#define U_INT64_MIN ((int64_t)(INT64_C(-9223372036854775807)-1))
#endif
#if !defined(U_INT64_MAX)
#define U_INT64_MAX ((int64_t)(INT64_C(9223372036854775807)))
#endif
#if !defined(U_UINT64_MAX)
#define U_UINT64_MAX ((uint64_t)(UINT64_C(18446744073709551615)))
#endif
#endif
typedef int8_t UBool;
#if !defined(TRUE)
#define TRUE 1
#endif
#if !defined(FALSE)
#define FALSE 0
#endif
#if !defined(U_WCHAR_IS_UTF16) && !defined(U_WCHAR_IS_UTF32)
#if defined(__STDC_ISO_10646__)
#if (U_SIZEOF_WCHAR_T==2)
#define U_WCHAR_IS_UTF16
#elif (U_SIZEOF_WCHAR_T==4)
#define U_WCHAR_IS_UTF32
#endif
#elif defined __UCS2__
#if (U_PF_OS390 <= U_PLATFORM && U_PLATFORM <= U_PF_OS400) && (U_SIZEOF_WCHAR_T==2)
#define U_WCHAR_IS_UTF16
#endif
#elif defined(__UCS4__) || (U_PLATFORM == U_PF_OS400 && defined(__UTF32__))
#if (U_SIZEOF_WCHAR_T==4)
#define U_WCHAR_IS_UTF32
#endif
#elif U_PLATFORM_IS_DARWIN_BASED || (U_SIZEOF_WCHAR_T==4 && U_PLATFORM_IS_LINUX_BASED)
#define U_WCHAR_IS_UTF32
#elif U_PLATFORM_HAS_WIN32_API
#define U_WCHAR_IS_UTF16
#endif
#endif
#define U_SIZEOF_UCHAR 2
#if (U_PLATFORM == U_PF_AIX) && defined(__cplusplus) &&(U_CPLUSPLUS_VERSION < 11)
#include <uchar.h>
#define U_CHAR16_IS_TYPEDEF 1
#elif defined(_MSC_VER) && (_MSC_VER < 1900)
#define U_CHAR16_IS_TYPEDEF 1
#else
#define U_CHAR16_IS_TYPEDEF 0
#endif
#if 1
#elif !defined(UCHAR_TYPE)
#define UCHAR_TYPE uint16_t
#endif
#if defined(U_COMBINED_IMPLEMENTATION) || defined(U_COMMON_IMPLEMENTATION) || defined(U_I18N_IMPLEMENTATION) || defined(U_IO_IMPLEMENTATION)
typedef char16_t UChar;
#elif defined(UCHAR_TYPE)
typedef UCHAR_TYPE UChar;
#elif defined(__cplusplus)
typedef char16_t UChar;
#else
typedef uint16_t UChar;
#endif
#if U_SIZEOF_WCHAR_T==2
typedef wchar_t OldUChar;
#elif defined(__CHAR16_TYPE__)
typedef __CHAR16_TYPE__ OldUChar;
#else
typedef uint16_t OldUChar;
#endif
typedef int32_t UChar32;
#define U_SENTINEL (-1)
#include "./urename.h"
