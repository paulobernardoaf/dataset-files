#if defined(__cplusplus)
extern "C" {
#endif
#include <inttypes.h>
#include <stddef.h>
typedef char utf8_char_t;
typedef size_t utf8_size_t;
const utf8_char_t* utf8_char_next(const utf8_char_t* c);
size_t utf8_char_length(const utf8_char_t* c);
int utf8_char_whitespace(const utf8_char_t* c);
size_t utf8_string_length(const utf8_char_t* data, utf8_size_t size);
size_t utf8_char_copy(utf8_char_t* dst, const utf8_char_t* src);
utf8_size_t utf8_char_count(const char* data, size_t size);
utf8_size_t utf8_trimmed_length(const utf8_char_t* data, utf8_size_t charcters);
size_t utf8_line_length(const utf8_char_t* data);
utf8_size_t utf8_wrap_length(const utf8_char_t* data, utf8_size_t size);
int utf8_line_count(const utf8_char_t* data);
#define UFTF_DEFAULT_MAX_FILE_SIZE = (50 * 1024 * 1024);
utf8_char_t* utf8_load_text_file(const char* path, size_t* size);
#if !defined(strnstr)
char* strnstr(const char* string1, const char* string2, size_t len);
#endif
#if defined(__cplusplus)
}
#endif
