
#include "yaml_private.h"





YAML_DECLARE(const char *)
yaml_get_version_string(void)
{
return YAML_VERSION_STRING;
}





YAML_DECLARE(void)
yaml_get_version(int *major, int *minor, int *patch)
{
*major = YAML_VERSION_MAJOR;
*minor = YAML_VERSION_MINOR;
*patch = YAML_VERSION_PATCH;
}





YAML_DECLARE(void *)
yaml_malloc(size_t size)
{
return malloc(size ? size : 1);
}





YAML_DECLARE(void *)
yaml_realloc(void *ptr, size_t size)
{
return ptr ? realloc(ptr, size ? size : 1) : malloc(size ? size : 1);
}





YAML_DECLARE(void)
yaml_free(void *ptr)
{
if (ptr) free(ptr);
}





YAML_DECLARE(yaml_char_t *)
yaml_strdup(const yaml_char_t *str)
{
if (!str)
return NULL;

return (yaml_char_t *)strdup((char *)str);
}





YAML_DECLARE(int)
yaml_string_extend(yaml_char_t **start,
yaml_char_t **pointer, yaml_char_t **end)
{
yaml_char_t *new_start = (yaml_char_t *)yaml_realloc((void*)*start, (*end - *start)*2);

if (!new_start) return 0;

memset(new_start + (*end - *start), 0, *end - *start);

*pointer = new_start + (*pointer - *start);
*end = new_start + (*end - *start)*2;
*start = new_start;

return 1;
}





YAML_DECLARE(int)
yaml_string_join(
yaml_char_t **a_start, yaml_char_t **a_pointer, yaml_char_t **a_end,
yaml_char_t **b_start, yaml_char_t **b_pointer, SHIM(yaml_char_t **b_end))
{
UNUSED_PARAM(b_end)
if (*b_start == *b_pointer)
return 1;

while (*a_end - *a_pointer <= *b_pointer - *b_start) {
if (!yaml_string_extend(a_start, a_pointer, a_end))
return 0;
}

memcpy(*a_pointer, *b_start, *b_pointer - *b_start);
*a_pointer += *b_pointer - *b_start;

return 1;
}





YAML_DECLARE(int)
yaml_stack_extend(void **start, void **top, void **end)
{
void *new_start = yaml_realloc(*start, ((char *)*end - (char *)*start)*2);

if (!new_start) return 0;

*top = (char *)new_start + ((char *)*top - (char *)*start);
*end = (char *)new_start + ((char *)*end - (char *)*start)*2;
*start = new_start;

return 1;
}





YAML_DECLARE(int)
yaml_queue_extend(void **start, void **head, void **tail, void **end)
{


if (*start == *head && *tail == *end) {
void *new_start = yaml_realloc(*start,
((char *)*end - (char *)*start)*2);

if (!new_start) return 0;

*head = (char *)new_start + ((char *)*head - (char *)*start);
*tail = (char *)new_start + ((char *)*tail - (char *)*start);
*end = (char *)new_start + ((char *)*end - (char *)*start)*2;
*start = new_start;
}



if (*tail == *end) {
if (*head != *tail) {
memmove(*start, *head, (char *)*tail - (char *)*head);
}
*tail = (char *)*tail - (char *)*head + (char *)*start;
*head = *start;
}

return 1;
}






YAML_DECLARE(int)
yaml_parser_initialize(yaml_parser_t *parser)
{
assert(parser); 

memset(parser, 0, sizeof(yaml_parser_t));
if (!BUFFER_INIT(parser, parser->raw_buffer, INPUT_RAW_BUFFER_SIZE))
goto error;
if (!BUFFER_INIT(parser, parser->buffer, INPUT_BUFFER_SIZE))
goto error;
if (!QUEUE_INIT(parser, parser->tokens, INITIAL_QUEUE_SIZE, yaml_token_t*))
goto error;
if (!STACK_INIT(parser, parser->indents, int*))
goto error;
if (!STACK_INIT(parser, parser->simple_keys, yaml_simple_key_t*))
goto error;
if (!STACK_INIT(parser, parser->states, yaml_parser_state_t*))
goto error;
if (!STACK_INIT(parser, parser->marks, yaml_mark_t*))
goto error;
if (!STACK_INIT(parser, parser->tag_directives, yaml_tag_directive_t*))
goto error;

return 1;

error:

BUFFER_DEL(parser, parser->raw_buffer);
BUFFER_DEL(parser, parser->buffer);
QUEUE_DEL(parser, parser->tokens);
STACK_DEL(parser, parser->indents);
STACK_DEL(parser, parser->simple_keys);
STACK_DEL(parser, parser->states);
STACK_DEL(parser, parser->marks);
STACK_DEL(parser, parser->tag_directives);

return 0;
}





YAML_DECLARE(void)
yaml_parser_delete(yaml_parser_t *parser)
{
assert(parser); 

BUFFER_DEL(parser, parser->raw_buffer);
BUFFER_DEL(parser, parser->buffer);
while (!QUEUE_EMPTY(parser, parser->tokens)) {
yaml_token_delete(&DEQUEUE(parser, parser->tokens));
}
QUEUE_DEL(parser, parser->tokens);
STACK_DEL(parser, parser->indents);
STACK_DEL(parser, parser->simple_keys);
STACK_DEL(parser, parser->states);
STACK_DEL(parser, parser->marks);
while (!STACK_EMPTY(parser, parser->tag_directives)) {
yaml_tag_directive_t tag_directive = POP(parser, parser->tag_directives);
yaml_free(tag_directive.handle);
yaml_free(tag_directive.prefix);
}
STACK_DEL(parser, parser->tag_directives);

memset(parser, 0, sizeof(yaml_parser_t));
}





static int
yaml_string_read_handler(void *data, unsigned char *buffer, size_t size,
size_t *size_read)
{
yaml_parser_t *parser = (yaml_parser_t *)data;

if (parser->input.string.current == parser->input.string.end) {
*size_read = 0;
return 1;
}

if (size > (size_t)(parser->input.string.end
- parser->input.string.current)) {
size = parser->input.string.end - parser->input.string.current;
}

memcpy(buffer, parser->input.string.current, size);
parser->input.string.current += size;
*size_read = size;
return 1;
}





static int
yaml_file_read_handler(void *data, unsigned char *buffer, size_t size,
size_t *size_read)
{
yaml_parser_t *parser = (yaml_parser_t *)data;

*size_read = fread(buffer, 1, size, parser->input.file);
return !ferror(parser->input.file);
}





YAML_DECLARE(void)
yaml_parser_set_input_string(yaml_parser_t *parser,
const unsigned char *input, size_t size)
{
assert(parser); 
assert(!parser->read_handler); 
assert(input); 

parser->read_handler = yaml_string_read_handler;
parser->read_handler_data = parser;

parser->input.string.start = input;
parser->input.string.current = input;
parser->input.string.end = input+size;
}





YAML_DECLARE(void)
yaml_parser_set_input_file(yaml_parser_t *parser, FILE *file)
{
assert(parser); 
assert(!parser->read_handler); 
assert(file); 

parser->read_handler = yaml_file_read_handler;
parser->read_handler_data = parser;

parser->input.file = file;
}





YAML_DECLARE(void)
yaml_parser_set_input(yaml_parser_t *parser,
yaml_read_handler_t *handler, void *data)
{
assert(parser); 
assert(!parser->read_handler); 
assert(handler); 

parser->read_handler = handler;
parser->read_handler_data = data;
}





YAML_DECLARE(void)
yaml_parser_set_encoding(yaml_parser_t *parser, yaml_encoding_t encoding)
{
assert(parser); 
assert(!parser->encoding); 

parser->encoding = encoding;
}





YAML_DECLARE(int)
yaml_emitter_initialize(yaml_emitter_t *emitter)
{
assert(emitter); 

memset(emitter, 0, sizeof(yaml_emitter_t));
if (!BUFFER_INIT(emitter, emitter->buffer, OUTPUT_BUFFER_SIZE))
goto error;
if (!BUFFER_INIT(emitter, emitter->raw_buffer, OUTPUT_RAW_BUFFER_SIZE))
goto error;
if (!STACK_INIT(emitter, emitter->states, yaml_emitter_state_t*))
goto error;
if (!QUEUE_INIT(emitter, emitter->events, INITIAL_QUEUE_SIZE, yaml_event_t*))
goto error;
if (!STACK_INIT(emitter, emitter->indents, int*))
goto error;
if (!STACK_INIT(emitter, emitter->tag_directives, yaml_tag_directive_t*))
goto error;

return 1;

error:

BUFFER_DEL(emitter, emitter->buffer);
BUFFER_DEL(emitter, emitter->raw_buffer);
STACK_DEL(emitter, emitter->states);
QUEUE_DEL(emitter, emitter->events);
STACK_DEL(emitter, emitter->indents);
STACK_DEL(emitter, emitter->tag_directives);

return 0;
}





YAML_DECLARE(void)
yaml_emitter_delete(yaml_emitter_t *emitter)
{
assert(emitter); 

BUFFER_DEL(emitter, emitter->buffer);
BUFFER_DEL(emitter, emitter->raw_buffer);
STACK_DEL(emitter, emitter->states);
while (!QUEUE_EMPTY(emitter, emitter->events)) {
yaml_event_delete(&DEQUEUE(emitter, emitter->events));
}
QUEUE_DEL(emitter, emitter->events);
STACK_DEL(emitter, emitter->indents);
while (!STACK_EMPTY(empty, emitter->tag_directives)) {
yaml_tag_directive_t tag_directive = POP(emitter, emitter->tag_directives);
yaml_free(tag_directive.handle);
yaml_free(tag_directive.prefix);
}
STACK_DEL(emitter, emitter->tag_directives);
yaml_free(emitter->anchors);

memset(emitter, 0, sizeof(yaml_emitter_t));
}





static int
yaml_string_write_handler(void *data, unsigned char *buffer, size_t size)
{
yaml_emitter_t *emitter = (yaml_emitter_t *)data;

if (emitter->output.string.size - *emitter->output.string.size_written
< size) {
memcpy(emitter->output.string.buffer
+ *emitter->output.string.size_written,
buffer,
emitter->output.string.size
- *emitter->output.string.size_written);
*emitter->output.string.size_written = emitter->output.string.size;
return 0;
}

memcpy(emitter->output.string.buffer
+ *emitter->output.string.size_written, buffer, size);
*emitter->output.string.size_written += size;
return 1;
}





static int
yaml_file_write_handler(void *data, unsigned char *buffer, size_t size)
{
yaml_emitter_t *emitter = (yaml_emitter_t *)data;

return (fwrite(buffer, 1, size, emitter->output.file) == size);
}




YAML_DECLARE(void)
yaml_emitter_set_output_string(yaml_emitter_t *emitter,
unsigned char *output, size_t size, size_t *size_written)
{
assert(emitter); 
assert(!emitter->write_handler); 
assert(output); 

emitter->write_handler = yaml_string_write_handler;
emitter->write_handler_data = emitter;

emitter->output.string.buffer = output;
emitter->output.string.size = size;
emitter->output.string.size_written = size_written;
*size_written = 0;
}





YAML_DECLARE(void)
yaml_emitter_set_output_file(yaml_emitter_t *emitter, FILE *file)
{
assert(emitter); 
assert(!emitter->write_handler); 
assert(file); 

emitter->write_handler = yaml_file_write_handler;
emitter->write_handler_data = emitter;

emitter->output.file = file;
}





YAML_DECLARE(void)
yaml_emitter_set_output(yaml_emitter_t *emitter,
yaml_write_handler_t *handler, void *data)
{
assert(emitter); 
assert(!emitter->write_handler); 
assert(handler); 

emitter->write_handler = handler;
emitter->write_handler_data = data;
}





YAML_DECLARE(void)
yaml_emitter_set_encoding(yaml_emitter_t *emitter, yaml_encoding_t encoding)
{
assert(emitter); 
assert(!emitter->encoding); 

emitter->encoding = encoding;
}





YAML_DECLARE(void)
yaml_emitter_set_canonical(yaml_emitter_t *emitter, int canonical)
{
assert(emitter); 

emitter->canonical = (canonical != 0);
}





YAML_DECLARE(void)
yaml_emitter_set_indent(yaml_emitter_t *emitter, int indent)
{
assert(emitter); 

emitter->best_indent = (1 < indent && indent < 10) ? indent : 2;
}





YAML_DECLARE(void)
yaml_emitter_set_width(yaml_emitter_t *emitter, int width)
{
assert(emitter); 

emitter->best_width = (width >= 0) ? width : -1;
}





YAML_DECLARE(void)
yaml_emitter_set_unicode(yaml_emitter_t *emitter, int unicode)
{
assert(emitter); 

emitter->unicode = (unicode != 0);
}





YAML_DECLARE(void)
yaml_emitter_set_break(yaml_emitter_t *emitter, yaml_break_t line_break)
{
assert(emitter); 

emitter->line_break = line_break;
}





YAML_DECLARE(void)
yaml_token_delete(yaml_token_t *token)
{
assert(token); 

switch (token->type)
{
case YAML_TAG_DIRECTIVE_TOKEN:
yaml_free(token->data.tag_directive.handle);
yaml_free(token->data.tag_directive.prefix);
break;

case YAML_ALIAS_TOKEN:
yaml_free(token->data.alias.value);
break;

case YAML_ANCHOR_TOKEN:
yaml_free(token->data.anchor.value);
break;

case YAML_TAG_TOKEN:
yaml_free(token->data.tag.handle);
yaml_free(token->data.tag.suffix);
break;

case YAML_SCALAR_TOKEN:
yaml_free(token->data.scalar.value);
break;

default:
break;
}

memset(token, 0, sizeof(yaml_token_t));
}







static int
yaml_check_utf8(yaml_char_t *start, size_t length)
{
yaml_char_t *end = start+length;
yaml_char_t *pointer = start;

while (pointer < end) {
unsigned char octet;
unsigned int width;
unsigned int value;
size_t k;

octet = pointer[0];
width = (octet & 0x80) == 0x00 ? 1 :
(octet & 0xE0) == 0xC0 ? 2 :
(octet & 0xF0) == 0xE0 ? 3 :
(octet & 0xF8) == 0xF0 ? 4 : 0;
value = (octet & 0x80) == 0x00 ? octet & 0x7F :
(octet & 0xE0) == 0xC0 ? octet & 0x1F :
(octet & 0xF0) == 0xE0 ? octet & 0x0F :
(octet & 0xF8) == 0xF0 ? octet & 0x07 : 0;
if (!width) return 0;
if (pointer+width > end) return 0;
for (k = 1; k < width; k ++) {
octet = pointer[k];
if ((octet & 0xC0) != 0x80) return 0;
value = (value << 6) + (octet & 0x3F);
}
if (!((width == 1) ||
(width == 2 && value >= 0x80) ||
(width == 3 && value >= 0x800) ||
(width == 4 && value >= 0x10000))) return 0;

pointer += width;
}

return 1;
}





YAML_DECLARE(int)
yaml_stream_start_event_initialize(yaml_event_t *event,
yaml_encoding_t encoding)
{
yaml_mark_t mark = { 0, 0, 0 };

assert(event); 

STREAM_START_EVENT_INIT(*event, encoding, mark, mark);

return 1;
}





YAML_DECLARE(int)
yaml_stream_end_event_initialize(yaml_event_t *event)
{
yaml_mark_t mark = { 0, 0, 0 };

assert(event); 

STREAM_END_EVENT_INIT(*event, mark, mark);

return 1;
}





YAML_DECLARE(int)
yaml_document_start_event_initialize(yaml_event_t *event,
yaml_version_directive_t *version_directive,
yaml_tag_directive_t *tag_directives_start,
yaml_tag_directive_t *tag_directives_end,
int implicit)
{
struct {
yaml_error_type_t error;
} context;
yaml_mark_t mark = { 0, 0, 0 };
yaml_version_directive_t *version_directive_copy = NULL;
struct {
yaml_tag_directive_t *start;
yaml_tag_directive_t *end;
yaml_tag_directive_t *top;
} tag_directives_copy = { NULL, NULL, NULL };
yaml_tag_directive_t value = { NULL, NULL };

assert(event); 
assert((tag_directives_start && tag_directives_end) ||
(tag_directives_start == tag_directives_end));


if (version_directive) {
version_directive_copy = YAML_MALLOC_STATIC(yaml_version_directive_t);
if (!version_directive_copy) goto error;
version_directive_copy->major = version_directive->major;
version_directive_copy->minor = version_directive->minor;
}

if (tag_directives_start != tag_directives_end) {
yaml_tag_directive_t *tag_directive;
if (!STACK_INIT(&context, tag_directives_copy, yaml_tag_directive_t*))
goto error;
for (tag_directive = tag_directives_start;
tag_directive != tag_directives_end; tag_directive ++) {
assert(tag_directive->handle);
assert(tag_directive->prefix);
if (!yaml_check_utf8(tag_directive->handle,
strlen((char *)tag_directive->handle)))
goto error;
if (!yaml_check_utf8(tag_directive->prefix,
strlen((char *)tag_directive->prefix)))
goto error;
value.handle = yaml_strdup(tag_directive->handle);
value.prefix = yaml_strdup(tag_directive->prefix);
if (!value.handle || !value.prefix) goto error;
if (!PUSH(&context, tag_directives_copy, value))
goto error;
value.handle = NULL;
value.prefix = NULL;
}
}

DOCUMENT_START_EVENT_INIT(*event, version_directive_copy,
tag_directives_copy.start, tag_directives_copy.top,
implicit, mark, mark);

return 1;

error:
yaml_free(version_directive_copy);
while (!STACK_EMPTY(context, tag_directives_copy)) {
yaml_tag_directive_t value = POP(context, tag_directives_copy);
yaml_free(value.handle);
yaml_free(value.prefix);
}
STACK_DEL(context, tag_directives_copy);
yaml_free(value.handle);
yaml_free(value.prefix);

return 0;
}





YAML_DECLARE(int)
yaml_document_end_event_initialize(yaml_event_t *event, int implicit)
{
yaml_mark_t mark = { 0, 0, 0 };

assert(event); 

DOCUMENT_END_EVENT_INIT(*event, implicit, mark, mark);

return 1;
}





YAML_DECLARE(int)
yaml_alias_event_initialize(yaml_event_t *event, yaml_char_t *anchor)
{
yaml_mark_t mark = { 0, 0, 0 };
yaml_char_t *anchor_copy = NULL;

assert(event); 
assert(anchor); 

if (!yaml_check_utf8(anchor, strlen((char *)anchor))) return 0;

anchor_copy = yaml_strdup(anchor);
if (!anchor_copy)
return 0;

ALIAS_EVENT_INIT(*event, anchor_copy, mark, mark);

return 1;
}





YAML_DECLARE(int)
yaml_scalar_event_initialize(yaml_event_t *event,
yaml_char_t *anchor, yaml_char_t *tag,
yaml_char_t *value, int length,
int plain_implicit, int quoted_implicit,
yaml_scalar_style_t style)
{
yaml_mark_t mark = { 0, 0, 0 };
yaml_char_t *anchor_copy = NULL;
yaml_char_t *tag_copy = NULL;
yaml_char_t *value_copy = NULL;

assert(event); 
assert(value); 

if (anchor) {
if (!yaml_check_utf8(anchor, strlen((char *)anchor))) goto error;
anchor_copy = yaml_strdup(anchor);
if (!anchor_copy) goto error;
}

if (tag) {
if (!yaml_check_utf8(tag, strlen((char *)tag))) goto error;
tag_copy = yaml_strdup(tag);
if (!tag_copy) goto error;
}

if (length < 0) {
length = (int)strlen((char *)value);
}

if (!yaml_check_utf8(value, length)) goto error;
value_copy = YAML_MALLOC(length+1);
if (!value_copy) goto error;
memcpy(value_copy, value, length);
value_copy[length] = '\0';

SCALAR_EVENT_INIT(*event, anchor_copy, tag_copy, value_copy, length,
plain_implicit, quoted_implicit, style, mark, mark);

return 1;

error:
yaml_free(anchor_copy);
yaml_free(tag_copy);
yaml_free(value_copy);

return 0;
}





YAML_DECLARE(int)
yaml_sequence_start_event_initialize(yaml_event_t *event,
yaml_char_t *anchor, yaml_char_t *tag, int implicit,
yaml_sequence_style_t style)
{
yaml_mark_t mark = { 0, 0, 0 };
yaml_char_t *anchor_copy = NULL;
yaml_char_t *tag_copy = NULL;

assert(event); 

if (anchor) {
if (!yaml_check_utf8(anchor, strlen((char *)anchor))) goto error;
anchor_copy = yaml_strdup(anchor);
if (!anchor_copy) goto error;
}

if (tag) {
if (!yaml_check_utf8(tag, strlen((char *)tag))) goto error;
tag_copy = yaml_strdup(tag);
if (!tag_copy) goto error;
}

SEQUENCE_START_EVENT_INIT(*event, anchor_copy, tag_copy,
implicit, style, mark, mark);

return 1;

error:
yaml_free(anchor_copy);
yaml_free(tag_copy);

return 0;
}





YAML_DECLARE(int)
yaml_sequence_end_event_initialize(yaml_event_t *event)
{
yaml_mark_t mark = { 0, 0, 0 };

assert(event); 

SEQUENCE_END_EVENT_INIT(*event, mark, mark);

return 1;
}





YAML_DECLARE(int)
yaml_mapping_start_event_initialize(yaml_event_t *event,
yaml_char_t *anchor, yaml_char_t *tag, int implicit,
yaml_mapping_style_t style)
{
yaml_mark_t mark = { 0, 0, 0 };
yaml_char_t *anchor_copy = NULL;
yaml_char_t *tag_copy = NULL;

assert(event); 

if (anchor) {
if (!yaml_check_utf8(anchor, strlen((char *)anchor))) goto error;
anchor_copy = yaml_strdup(anchor);
if (!anchor_copy) goto error;
}

if (tag) {
if (!yaml_check_utf8(tag, strlen((char *)tag))) goto error;
tag_copy = yaml_strdup(tag);
if (!tag_copy) goto error;
}

MAPPING_START_EVENT_INIT(*event, anchor_copy, tag_copy,
implicit, style, mark, mark);

return 1;

error:
yaml_free(anchor_copy);
yaml_free(tag_copy);

return 0;
}





YAML_DECLARE(int)
yaml_mapping_end_event_initialize(yaml_event_t *event)
{
yaml_mark_t mark = { 0, 0, 0 };

assert(event); 

MAPPING_END_EVENT_INIT(*event, mark, mark);

return 1;
}





YAML_DECLARE(void)
yaml_event_delete(yaml_event_t *event)
{
yaml_tag_directive_t *tag_directive;

assert(event); 

switch (event->type)
{
case YAML_DOCUMENT_START_EVENT:
yaml_free(event->data.document_start.version_directive);
for (tag_directive = event->data.document_start.tag_directives.start;
tag_directive != event->data.document_start.tag_directives.end;
tag_directive++) {
yaml_free(tag_directive->handle);
yaml_free(tag_directive->prefix);
}
yaml_free(event->data.document_start.tag_directives.start);
break;

case YAML_ALIAS_EVENT:
yaml_free(event->data.alias.anchor);
break;

case YAML_SCALAR_EVENT:
yaml_free(event->data.scalar.anchor);
yaml_free(event->data.scalar.tag);
yaml_free(event->data.scalar.value);
break;

case YAML_SEQUENCE_START_EVENT:
yaml_free(event->data.sequence_start.anchor);
yaml_free(event->data.sequence_start.tag);
break;

case YAML_MAPPING_START_EVENT:
yaml_free(event->data.mapping_start.anchor);
yaml_free(event->data.mapping_start.tag);
break;

default:
break;
}

memset(event, 0, sizeof(yaml_event_t));
}





YAML_DECLARE(int)
yaml_document_initialize(yaml_document_t *document,
yaml_version_directive_t *version_directive,
yaml_tag_directive_t *tag_directives_start,
yaml_tag_directive_t *tag_directives_end,
int start_implicit, int end_implicit)
{
struct {
yaml_error_type_t error;
} context;
struct {
yaml_node_t *start;
yaml_node_t *end;
yaml_node_t *top;
} nodes = { NULL, NULL, NULL };
yaml_version_directive_t *version_directive_copy = NULL;
struct {
yaml_tag_directive_t *start;
yaml_tag_directive_t *end;
yaml_tag_directive_t *top;
} tag_directives_copy = { NULL, NULL, NULL };
yaml_tag_directive_t value = { NULL, NULL };
yaml_mark_t mark = { 0, 0, 0 };

assert(document); 
assert((tag_directives_start && tag_directives_end) ||
(tag_directives_start == tag_directives_end));


if (!STACK_INIT(&context, nodes, yaml_node_t*)) goto error;

if (version_directive) {
version_directive_copy = YAML_MALLOC_STATIC(yaml_version_directive_t);
if (!version_directive_copy) goto error;
version_directive_copy->major = version_directive->major;
version_directive_copy->minor = version_directive->minor;
}

if (tag_directives_start != tag_directives_end) {
yaml_tag_directive_t *tag_directive;
if (!STACK_INIT(&context, tag_directives_copy, yaml_tag_directive_t*))
goto error;
for (tag_directive = tag_directives_start;
tag_directive != tag_directives_end; tag_directive ++) {
assert(tag_directive->handle);
assert(tag_directive->prefix);
if (!yaml_check_utf8(tag_directive->handle,
strlen((char *)tag_directive->handle)))
goto error;
if (!yaml_check_utf8(tag_directive->prefix,
strlen((char *)tag_directive->prefix)))
goto error;
value.handle = yaml_strdup(tag_directive->handle);
value.prefix = yaml_strdup(tag_directive->prefix);
if (!value.handle || !value.prefix) goto error;
if (!PUSH(&context, tag_directives_copy, value))
goto error;
value.handle = NULL;
value.prefix = NULL;
}
}

DOCUMENT_INIT(*document, nodes.start, nodes.end, version_directive_copy,
tag_directives_copy.start, tag_directives_copy.top,
start_implicit, end_implicit, mark, mark);

return 1;

error:
STACK_DEL(&context, nodes);
yaml_free(version_directive_copy);
while (!STACK_EMPTY(&context, tag_directives_copy)) {
yaml_tag_directive_t value = POP(&context, tag_directives_copy);
yaml_free(value.handle);
yaml_free(value.prefix);
}
STACK_DEL(&context, tag_directives_copy);
yaml_free(value.handle);
yaml_free(value.prefix);

return 0;
}





YAML_DECLARE(void)
yaml_document_delete(yaml_document_t *document)
{
struct {
yaml_error_type_t error;
} context;
yaml_tag_directive_t *tag_directive;


context.error = YAML_NO_ERROR;
(void)context.error;

assert(document); 

while (!STACK_EMPTY(&context, document->nodes)) {
yaml_node_t node = POP(&context, document->nodes);
yaml_free(node.tag);
switch (node.type) {
case YAML_SCALAR_NODE:
yaml_free(node.data.scalar.value);
break;
case YAML_SEQUENCE_NODE:
STACK_DEL(&context, node.data.sequence.items);
break;
case YAML_MAPPING_NODE:
STACK_DEL(&context, node.data.mapping.pairs);
break;
default:
assert(0); 
}
}
STACK_DEL(&context, document->nodes);

yaml_free(document->version_directive);
for (tag_directive = document->tag_directives.start;
tag_directive != document->tag_directives.end;
tag_directive++) {
yaml_free(tag_directive->handle);
yaml_free(tag_directive->prefix);
}
yaml_free(document->tag_directives.start);

memset(document, 0, sizeof(yaml_document_t));
}





YAML_DECLARE(yaml_node_t *)
yaml_document_get_node(yaml_document_t *document, int index)
{
assert(document); 

if (index > 0 && document->nodes.start + index <= document->nodes.top) {
return document->nodes.start + index - 1;
}
return NULL;
}





YAML_DECLARE(yaml_node_t *)
yaml_document_get_root_node(yaml_document_t *document)
{
assert(document); 

if (document->nodes.top != document->nodes.start) {
return document->nodes.start;
}
return NULL;
}





YAML_DECLARE(int)
yaml_document_add_scalar(yaml_document_t *document,
yaml_char_t *tag, yaml_char_t *value, int length,
yaml_scalar_style_t style)
{
struct {
yaml_error_type_t error;
} context;
yaml_mark_t mark = { 0, 0, 0 };
yaml_char_t *tag_copy = NULL;
yaml_char_t *value_copy = NULL;
yaml_node_t node;

assert(document); 
assert(value); 

if (!tag) {
tag = (yaml_char_t *)YAML_DEFAULT_SCALAR_TAG;
}

if (!yaml_check_utf8(tag, strlen((char *)tag))) goto error;
tag_copy = yaml_strdup(tag);
if (!tag_copy) goto error;

if (length < 0) {
length = (int)strlen((char *)value);
}

if (!yaml_check_utf8(value, length)) goto error;
value_copy = YAML_MALLOC(length+1);
if (!value_copy) goto error;
memcpy(value_copy, value, length);
value_copy[length] = '\0';

SCALAR_NODE_INIT(node, tag_copy, value_copy, length, style, mark, mark);
if (!PUSH(&context, document->nodes, node)) goto error;

return (int)(document->nodes.top - document->nodes.start);

error:
yaml_free(tag_copy);
yaml_free(value_copy);

return 0;
}





YAML_DECLARE(int)
yaml_document_add_sequence(yaml_document_t *document,
yaml_char_t *tag, yaml_sequence_style_t style)
{
struct {
yaml_error_type_t error;
} context;
yaml_mark_t mark = { 0, 0, 0 };
yaml_char_t *tag_copy = NULL;
struct {
yaml_node_item_t *start;
yaml_node_item_t *end;
yaml_node_item_t *top;
} items = { NULL, NULL, NULL };
yaml_node_t node;

assert(document); 

if (!tag) {
tag = (yaml_char_t *)YAML_DEFAULT_SEQUENCE_TAG;
}

if (!yaml_check_utf8(tag, strlen((char *)tag))) goto error;
tag_copy = yaml_strdup(tag);
if (!tag_copy) goto error;

if (!STACK_INIT(&context, items, yaml_node_item_t*)) goto error;

SEQUENCE_NODE_INIT(node, tag_copy, items.start, items.end,
style, mark, mark);
if (!PUSH(&context, document->nodes, node)) goto error;

return (int)(document->nodes.top - document->nodes.start);

error:
STACK_DEL(&context, items);
yaml_free(tag_copy);

return 0;
}





YAML_DECLARE(int)
yaml_document_add_mapping(yaml_document_t *document,
yaml_char_t *tag, yaml_mapping_style_t style)
{
struct {
yaml_error_type_t error;
} context;
yaml_mark_t mark = { 0, 0, 0 };
yaml_char_t *tag_copy = NULL;
struct {
yaml_node_pair_t *start;
yaml_node_pair_t *end;
yaml_node_pair_t *top;
} pairs = { NULL, NULL, NULL };
yaml_node_t node;

assert(document); 

if (!tag) {
tag = (yaml_char_t *)YAML_DEFAULT_MAPPING_TAG;
}

if (!yaml_check_utf8(tag, strlen((char *)tag))) goto error;
tag_copy = yaml_strdup(tag);
if (!tag_copy) goto error;

if (!STACK_INIT(&context, pairs, yaml_node_pair_t*)) goto error;

MAPPING_NODE_INIT(node, tag_copy, pairs.start, pairs.end,
style, mark, mark);
if (!PUSH(&context, document->nodes, node)) goto error;

return (int)(document->nodes.top - document->nodes.start);

error:
STACK_DEL(&context, pairs);
yaml_free(tag_copy);

return 0;
}





YAML_DECLARE(int)
yaml_document_append_sequence_item(yaml_document_t *document,
int sequence, int item)
{
struct {
yaml_error_type_t error;
} context;

assert(document); 
assert(sequence > 0
&& document->nodes.start + sequence <= document->nodes.top);

assert(document->nodes.start[sequence-1].type == YAML_SEQUENCE_NODE);

assert(item > 0 && document->nodes.start + item <= document->nodes.top);


if (!PUSH(&context,
document->nodes.start[sequence-1].data.sequence.items, item))
return 0;

return 1;
}





YAML_DECLARE(int)
yaml_document_append_mapping_pair(yaml_document_t *document,
int mapping, int key, int value)
{
struct {
yaml_error_type_t error;
} context;

yaml_node_pair_t pair;

assert(document); 
assert(mapping > 0
&& document->nodes.start + mapping <= document->nodes.top);

assert(document->nodes.start[mapping-1].type == YAML_MAPPING_NODE);

assert(key > 0 && document->nodes.start + key <= document->nodes.top);

assert(value > 0 && document->nodes.start + value <= document->nodes.top);


pair.key = key;
pair.value = value;

if (!PUSH(&context,
document->nodes.start[mapping-1].data.mapping.pairs, pair))
return 0;

return 1;
}


