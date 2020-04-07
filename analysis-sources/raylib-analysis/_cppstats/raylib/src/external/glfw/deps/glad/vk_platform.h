#if defined(__cplusplus)
extern "C"
{
#endif 
#if defined(_WIN32)
#define VKAPI_ATTR
#define VKAPI_CALL __stdcall
#define VKAPI_PTR VKAPI_CALL
#elif defined(__ANDROID__) && defined(__ARM_ARCH) && __ARM_ARCH < 7
#error "Vulkan isn't supported for the 'armeabi' NDK ABI"
#elif defined(__ANDROID__) && defined(__ARM_ARCH) && __ARM_ARCH >= 7 && defined(__ARM_32BIT_STATE)
#define VKAPI_ATTR __attribute__((pcs("aapcs-vfp")))
#define VKAPI_CALL
#define VKAPI_PTR VKAPI_ATTR
#else
#define VKAPI_ATTR
#define VKAPI_CALL
#define VKAPI_PTR
#endif
#include <stddef.h>
#if !defined(VK_NO_STDINT_H)
#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef signed __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef signed __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef signed __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif
#endif 
#if defined(__cplusplus)
} 
#endif 
