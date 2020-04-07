
#include "yaml_private.h"





static int
yaml_parser_set_reader_error(yaml_parser_t *parser, const char *problem,
size_t offset, int value);

static int
yaml_parser_update_raw_buffer(yaml_parser_t *parser);

static int
yaml_parser_determine_encoding(yaml_parser_t *parser);

YAML_DECLARE(int)
yaml_parser_update_buffer(yaml_parser_t *parser, size_t length);





static int
yaml_parser_set_reader_error(yaml_parser_t *parser, const char *problem,
size_t offset, int value)
{
parser->error = YAML_READER_ERROR;
parser->problem = problem;
parser->problem_offset = offset;
parser->problem_value = value;

return 0;
}





#define BOM_UTF8 "\xef\xbb\xbf"
#define BOM_UTF16LE "\xff\xfe"
#define BOM_UTF16BE "\xfe\xff"






static int
yaml_parser_determine_encoding(yaml_parser_t *parser)
{


while (!parser->eof
&& parser->raw_buffer.last - parser->raw_buffer.pointer < 3) {
if (!yaml_parser_update_raw_buffer(parser)) {
return 0;
}
}



if (parser->raw_buffer.last - parser->raw_buffer.pointer >= 2
&& !memcmp(parser->raw_buffer.pointer, BOM_UTF16LE, 2)) {
parser->encoding = YAML_UTF16LE_ENCODING;
parser->raw_buffer.pointer += 2;
parser->offset += 2;
}
else if (parser->raw_buffer.last - parser->raw_buffer.pointer >= 2
&& !memcmp(parser->raw_buffer.pointer, BOM_UTF16BE, 2)) {
parser->encoding = YAML_UTF16BE_ENCODING;
parser->raw_buffer.pointer += 2;
parser->offset += 2;
}
else if (parser->raw_buffer.last - parser->raw_buffer.pointer >= 3
&& !memcmp(parser->raw_buffer.pointer, BOM_UTF8, 3)) {
parser->encoding = YAML_UTF8_ENCODING;
parser->raw_buffer.pointer += 3;
parser->offset += 3;
}
else {
parser->encoding = YAML_UTF8_ENCODING;
}

return 1;
}





static int
yaml_parser_update_raw_buffer(yaml_parser_t *parser)
{
size_t size_read = 0;



if (parser->raw_buffer.start == parser->raw_buffer.pointer
&& parser->raw_buffer.last == parser->raw_buffer.end)
return 1;



if (parser->eof) return 1;



if (parser->raw_buffer.start < parser->raw_buffer.pointer
&& parser->raw_buffer.pointer < parser->raw_buffer.last) {
memmove(parser->raw_buffer.start, parser->raw_buffer.pointer,
parser->raw_buffer.last - parser->raw_buffer.pointer);
}
parser->raw_buffer.last -=
parser->raw_buffer.pointer - parser->raw_buffer.start;
parser->raw_buffer.pointer = parser->raw_buffer.start;



if (!parser->read_handler(parser->read_handler_data, parser->raw_buffer.last,
parser->raw_buffer.end - parser->raw_buffer.last, &size_read)) {
return yaml_parser_set_reader_error(parser, "input error",
parser->offset, -1);
}
parser->raw_buffer.last += size_read;
if (!size_read) {
parser->eof = 1;
}

return 1;
}








YAML_DECLARE(int)
yaml_parser_update_buffer(yaml_parser_t *parser, size_t length)
{
int first = 1;

assert(parser->read_handler); 



if (parser->eof && parser->raw_buffer.pointer == parser->raw_buffer.last)
return 1;



if (parser->unread >= length)
return 1;



if (!parser->encoding) {
if (!yaml_parser_determine_encoding(parser))
return 0;
}



if (parser->buffer.start < parser->buffer.pointer
&& parser->buffer.pointer < parser->buffer.last) {
size_t size = parser->buffer.last - parser->buffer.pointer;
memmove(parser->buffer.start, parser->buffer.pointer, size);
parser->buffer.pointer = parser->buffer.start;
parser->buffer.last = parser->buffer.start + size;
}
else if (parser->buffer.pointer == parser->buffer.last) {
parser->buffer.pointer = parser->buffer.start;
parser->buffer.last = parser->buffer.start;
}



while (parser->unread < length)
{


if (!first || parser->raw_buffer.pointer == parser->raw_buffer.last) {
if (!yaml_parser_update_raw_buffer(parser)) return 0;
}
first = 0;



while (parser->raw_buffer.pointer != parser->raw_buffer.last)
{
unsigned int value = 0, value2 = 0;
int incomplete = 0;
unsigned char octet;
unsigned int width = 0;
int low, high;
size_t k;
size_t raw_unread = parser->raw_buffer.last - parser->raw_buffer.pointer;



switch (parser->encoding)
{
case YAML_UTF8_ENCODING:























octet = parser->raw_buffer.pointer[0];
width = (octet & 0x80) == 0x00 ? 1 :
(octet & 0xE0) == 0xC0 ? 2 :
(octet & 0xF0) == 0xE0 ? 3 :
(octet & 0xF8) == 0xF0 ? 4 : 0;



if (!width)
return yaml_parser_set_reader_error(parser,
"invalid leading UTF-8 octet",
parser->offset, octet);



if (width > raw_unread) {
if (parser->eof) {
return yaml_parser_set_reader_error(parser,
"incomplete UTF-8 octet sequence",
parser->offset, -1);
}
incomplete = 1;
break;
}



value = (octet & 0x80) == 0x00 ? octet & 0x7F :
(octet & 0xE0) == 0xC0 ? octet & 0x1F :
(octet & 0xF0) == 0xE0 ? octet & 0x0F :
(octet & 0xF8) == 0xF0 ? octet & 0x07 : 0;



for (k = 1; k < width; k ++)
{
octet = parser->raw_buffer.pointer[k];



if ((octet & 0xC0) != 0x80)
return yaml_parser_set_reader_error(parser,
"invalid trailing UTF-8 octet",
parser->offset+k, octet);



value = (value << 6) + (octet & 0x3F);
}



if (!((width == 1) ||
(width == 2 && value >= 0x80) ||
(width == 3 && value >= 0x800) ||
(width == 4 && value >= 0x10000)))
return yaml_parser_set_reader_error(parser,
"invalid length of a UTF-8 sequence",
parser->offset, -1);



if ((value >= 0xD800 && value <= 0xDFFF) || value > 0x10FFFF)
return yaml_parser_set_reader_error(parser,
"invalid Unicode character",
parser->offset, value);

break;

case YAML_UTF16LE_ENCODING:
case YAML_UTF16BE_ENCODING:

low = (parser->encoding == YAML_UTF16LE_ENCODING ? 0 : 1);
high = (parser->encoding == YAML_UTF16LE_ENCODING ? 1 : 0);





























if (raw_unread < 2) {
if (parser->eof) {
return yaml_parser_set_reader_error(parser,
"incomplete UTF-16 character",
parser->offset, -1);
}
incomplete = 1;
break;
}



value = parser->raw_buffer.pointer[low]
+ (parser->raw_buffer.pointer[high] << 8);



if ((value & 0xFC00) == 0xDC00)
return yaml_parser_set_reader_error(parser,
"unexpected low surrogate area",
parser->offset, value);



if ((value & 0xFC00) == 0xD800) {

width = 4;



if (raw_unread < 4) {
if (parser->eof) {
return yaml_parser_set_reader_error(parser,
"incomplete UTF-16 surrogate pair",
parser->offset, -1);
}
incomplete = 1;
break;
}



value2 = parser->raw_buffer.pointer[low+2]
+ (parser->raw_buffer.pointer[high+2] << 8);



if ((value2 & 0xFC00) != 0xDC00)
return yaml_parser_set_reader_error(parser,
"expected low surrogate area",
parser->offset+2, value2);



value = 0x10000 + ((value & 0x3FF) << 10) + (value2 & 0x3FF);
}

else {
width = 2;
}

break;

default:
assert(1); 
}



if (incomplete) break;








if (! (value == 0x09 || value == 0x0A || value == 0x0D
|| (value >= 0x20 && value <= 0x7E)
|| (value == 0x85) || (value >= 0xA0 && value <= 0xD7FF)
|| (value >= 0xE000 && value <= 0xFFFD)
|| (value >= 0x10000 && value <= 0x10FFFF)))
return yaml_parser_set_reader_error(parser,
"control characters are not allowed",
parser->offset, value);



parser->raw_buffer.pointer += width;
parser->offset += width;




if (value <= 0x7F) {
*(parser->buffer.last++) = value;
}

else if (value <= 0x7FF) {
*(parser->buffer.last++) = 0xC0 + (value >> 6);
*(parser->buffer.last++) = 0x80 + (value & 0x3F);
}

else if (value <= 0xFFFF) {
*(parser->buffer.last++) = 0xE0 + (value >> 12);
*(parser->buffer.last++) = 0x80 + ((value >> 6) & 0x3F);
*(parser->buffer.last++) = 0x80 + (value & 0x3F);
}

else {
*(parser->buffer.last++) = 0xF0 + (value >> 18);
*(parser->buffer.last++) = 0x80 + ((value >> 12) & 0x3F);
*(parser->buffer.last++) = 0x80 + ((value >> 6) & 0x3F);
*(parser->buffer.last++) = 0x80 + (value & 0x3F);
}

parser->unread ++;
}



if (parser->eof) {
*(parser->buffer.last++) = '\0';
parser->unread ++;
return 1;
}

}

if (parser->offset >= MAX_FILE_SIZE) {
return yaml_parser_set_reader_error(parser, "input is too long",
parser->offset, -1);
}

return 1;
}
