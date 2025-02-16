























#include "utf8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const utf8_char_t* utf8_char_next(const utf8_char_t* c)
{
const utf8_char_t* n = c + utf8_char_length(c);
return n == c ? 0 : n;
}


size_t utf8_char_length(const utf8_char_t* c)
{

if (!c || 0x00 == c[0]) {
return 0;
}

static const size_t _utf8_char_length[] = {
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
};

return _utf8_char_length[(c[0] >> 3) & 0x1F];
}

int utf8_char_whitespace(const utf8_char_t* c)
{

if (!c || (c[0] >= 0 && c[0] <= ' ') || c[0] == 0x7F) {
return 1;
}


if (0xC2 == (unsigned char)c[0] && 0xA0 == (unsigned char)c[1]) {
return 1;
}

return 0;
}



size_t utf8_string_length(const utf8_char_t* data, utf8_size_t size)
{
size_t char_length, byts = 0;

if (0 == size) {
size = utf8_char_count(data, 0);
}

for (; 0 < size; --size) {
if (0 == (char_length = utf8_char_length(data))) {
break;
}

data += char_length;
byts += char_length;
}

return byts;
}

size_t utf8_char_copy(utf8_char_t* dst, const utf8_char_t* src)
{
size_t bytes = utf8_char_length(src);

if (bytes && dst) {
memcpy(dst, src, bytes);
dst[bytes] = '\0';
}

return bytes;
}



utf8_size_t utf8_char_count(const char* data, size_t size)
{
size_t i, bytes = 0;
utf8_size_t count = 0;

if (0 == size) {
size = strlen(data);
}

for (i = 0; i < size; ++count, i += bytes) {
if (0 == (bytes = utf8_char_length(&data[i]))) {
break;
}
}

return count;
}


size_t utf8_trimmed_length(const utf8_char_t* data, utf8_size_t charcters)
{
size_t l, t = 0, split_at = 0;
for (size_t c = 0; (*data) && c < charcters; ++c) {
l = utf8_char_length(data);
t += l, data += l;
if (!utf8_char_whitespace(data)) {
split_at = t;
}
}

return split_at;
}

size_t _utf8_newline(const utf8_char_t* data)
{
if ('\r' == data[0]) {
return '\n' == data[1] ? 2 : 1; 
} else if ('\n' == data[0]) {
return '\r' == data[1] ? 2 : 1; 
} else {
return 0;
}
}


size_t utf8_line_length(const utf8_char_t* data)
{
size_t n, len = 0;

for (len = 0; 0 != data[len]; ++len) {
if (0 < (n = _utf8_newline(data))) {
return len + n;
}
}

return len;
}


utf8_size_t utf8_wrap_length(const utf8_char_t* data, utf8_size_t size)
{

size_t char_length, char_count, split_at = size;

for (char_count = 0; char_count <= size; ++char_count) {
if (_utf8_newline(data)) {
return char_count;
} else if (utf8_char_whitespace(data)) {
split_at = char_count;
}

char_length = utf8_char_length(data);
data += char_length;
}

return split_at;
}

int utf8_line_count(const utf8_char_t* data)
{
size_t len = 0;
int count = 0;

do {
len = utf8_line_length(data);
data += len;
++count;
} while (0 < len);

return count - 1;
}

utf8_char_t* utf8_load_text_file(const char* path, size_t* size)
{
utf8_char_t* data = NULL;
FILE* file = fopen(path, "r");

if (file) {
fseek(file, 0, SEEK_END);
size_t file_size = ftell(file);
fseek(file, 0, SEEK_SET);

if (0 == (*size) || file_size <= (*size)) {
(*size) = 0;
data = (utf8_char_t*)malloc(1 + file_size);
memset(data, '\0', file_size);

if (data) {
utf8_char_t* pos = data;
size_t bytes_read = 0;

while (0 < (bytes_read = fread(pos, 1, file_size - (*size), file))) {
pos += bytes_read;
(*size) += bytes_read;
}
}

fclose(file);
}
}

data[*size] = 0;
return data;
}

#if !defined(strnstr)
char* strnstr(const char* string1, const char* string2, size_t len)
{
size_t length2;

length2 = strlen(string2);
if (!length2) {
return (char*)string1;
}

while (len >= length2) {
len--;
if (!memcmp(string1, string2, length2))
return (char*)string1;
string1++;
}
return NULL;
}
#endif