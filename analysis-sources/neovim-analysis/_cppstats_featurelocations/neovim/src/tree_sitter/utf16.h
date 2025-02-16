#if !defined(TREE_SITTER_UTF16_H_)
#define TREE_SITTER_UTF16_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include "utf8proc.h"




utf8proc_ssize_t utf16_iterate(const utf8proc_uint8_t *, utf8proc_ssize_t, utf8proc_int32_t *);

#if defined(__cplusplus)
}
#endif

#endif 
