#if !defined(INTERNAL_SANITIZERS_H)
#define INTERNAL_SANITIZERS_H









#include "ruby/config.h"
#include "internal/compilers.h" 

#if defined(HAVE_VALGRIND_MEMCHECK_H)
#include <valgrind/memcheck.h>
#endif

#if defined(HAVE_SANITIZER_ASAN_INTERFACE_H)
#include <sanitizer/asan_interface.h>
#endif

#if defined(HAVE_SANITIZER_MSAN_INTERFACE_H)
#if __has_feature(memory_sanitizer)
#include <sanitizer/msan_interface.h>
#endif
#endif

#include "internal/stdbool.h" 
#include "ruby/ruby.h" 

#if 0
#elif __has_feature(memory_sanitizer) && __has_feature(address_sanitizer)
#define ATTRIBUTE_NO_ADDRESS_SAFETY_ANALYSIS(x) __attribute__((__no_sanitize__("memory, address"), __noinline__)) x

#elif __has_feature(address_sanitizer)
#define ATTRIBUTE_NO_ADDRESS_SAFETY_ANALYSIS(x) __attribute__((__no_sanitize__("address"), __noinline__)) x

#elif defined(NO_SANITIZE_ADDRESS)
#define ATTRIBUTE_NO_ADDRESS_SAFETY_ANALYSIS(x) NO_SANITIZE_ADDRESS(NOINLINE(x))

#elif defined(NO_ADDRESS_SAFETY_ANALYSIS)
#define ATTRIBUTE_NO_ADDRESS_SAFETY_ANALYSIS(x) NO_ADDRESS_SAFETY_ANALYSIS(NOINLINE(x))

#else
#define ATTRIBUTE_NO_ADDRESS_SAFETY_ANALYSIS(x) x
#endif

#if defined(NO_SANITIZE) && defined(__GNUC__) &&! defined(__clang__)

#include "internal/warnings.h"
#undef NO_SANITIZE
#define NO_SANITIZE(x, y) COMPILER_WARNING_PUSH; COMPILER_WARNING_IGNORED(-Wattributes); __attribute__((__no_sanitize__(x))) y; COMPILER_WARNING_POP




#endif

#if !defined(NO_SANITIZE)
#define NO_SANITIZE(x, y) y
#endif

#if !__has_feature(address_sanitizer)
#define __asan_poison_memory_region(x, y)
#define __asan_unpoison_memory_region(x, y)
#define __asan_region_is_poisoned(x, y) 0
#endif

#if !__has_feature(memory_sanitizer)
#define __msan_allocated_memory(x, y) ((void)(x), (void)(y))
#define __msan_poison(x, y) ((void)(x), (void)(y))
#define __msan_unpoison(x, y) ((void)(x), (void)(y))
#define __msan_unpoison_string(x) ((void)(x))
#endif

#if defined(VALGRIND_MAKE_READABLE)
#define VALGRIND_MAKE_MEM_DEFINED(p, n) VALGRIND_MAKE_READABLE((p), (n))
#endif

#if defined(VALGRIND_MAKE_WRITABLE)
#define VALGRIND_MAKE_MEM_UNDEFINED(p, n) VALGRIND_MAKE_WRITABLE((p), (n))
#endif

#if !defined(VALGRIND_MAKE_MEM_DEFINED)
#define VALGRIND_MAKE_MEM_DEFINED(p, n) 0
#endif

#if !defined(VALGRIND_MAKE_MEM_UNDEFINED)
#define VALGRIND_MAKE_MEM_UNDEFINED(p, n) 0
#endif

#if !defined(MJIT_HEADER)














static inline void
asan_poison_memory_region(const volatile void *ptr, size_t size)
{
__msan_poison(ptr, size);
__asan_poison_memory_region(ptr, size);
}






static inline void
asan_poison_object(VALUE obj)
{
MAYBE_UNUSED(struct RVALUE *) ptr = (void *)obj;
asan_poison_memory_region(ptr, SIZEOF_VALUE);
}

#if !__has_feature(address_sanitizer)
#define asan_poison_object_if(ptr, obj) ((void)(ptr), (void)(obj))
#else
#define asan_poison_object_if(ptr, obj) do { if (ptr) asan_poison_object(obj); } while (0)


#endif








static inline void *
asan_poisoned_object_p(VALUE obj)
{
MAYBE_UNUSED(struct RVALUE *) ptr = (void *)obj;
return __asan_region_is_poisoned(ptr, SIZEOF_VALUE);
}
















static inline void
asan_unpoison_memory_region(const volatile void *ptr, size_t size, bool malloc_p)
{
__asan_unpoison_memory_region(ptr, size);
if (malloc_p) {
__msan_allocated_memory(ptr, size);
}
else {
__msan_unpoison(ptr, size);
}
}







static inline void
asan_unpoison_object(VALUE obj, bool newobj_p)
{
MAYBE_UNUSED(struct RVALUE *) ptr = (void *)obj;
asan_unpoison_memory_region(ptr, SIZEOF_VALUE, newobj_p);
}

#endif 

#endif 
