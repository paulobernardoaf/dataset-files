#include "ccan/check_type/check_type.h"
#define container_of(member_ptr, containing_type, member) ((containing_type *) ((char *)(member_ptr) - container_off(containing_type, member)) + check_types_match(*(member_ptr), ((containing_type *)0)->member))
static inline char *container_of_or_null_(void *member_ptr, size_t offset)
{
return member_ptr ? (char *)member_ptr - offset : NULL;
}
#define container_of_or_null(member_ptr, containing_type, member) ((containing_type *) container_of_or_null_(member_ptr, container_off(containing_type, member)) + check_types_match(*(member_ptr), ((containing_type *)0)->member))
#define container_off(containing_type, member) offsetof(containing_type, member)
#if HAVE_TYPEOF
#define container_of_var(member_ptr, container_var, member) container_of(member_ptr, typeof(*container_var), member)
#else
#define container_of_var(member_ptr, container_var, member) ((void *)((char *)(member_ptr) - container_off_var(container_var, member)))
#endif
#if HAVE_TYPEOF
#define container_off_var(var, member) container_off(typeof(*var), member)
#else
#define container_off_var(var, member) ((const char *)&(var)->member - (const char *)(var))
#endif
