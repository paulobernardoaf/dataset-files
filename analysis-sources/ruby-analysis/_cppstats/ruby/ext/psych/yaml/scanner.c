#include "yaml_private.h"
#define CACHE(parser,length) (parser->unread >= (length) ? 1 : yaml_parser_update_buffer(parser, (length)))
#define SKIP(parser) (parser->mark.index ++, parser->mark.column ++, parser->unread --, parser->buffer.pointer += WIDTH(parser->buffer))
#define SKIP_LINE(parser) (IS_CRLF(parser->buffer) ? (parser->mark.index += 2, parser->mark.column = 0, parser->mark.line ++, parser->unread -= 2, parser->buffer.pointer += 2) : IS_BREAK(parser->buffer) ? (parser->mark.index ++, parser->mark.column = 0, parser->mark.line ++, parser->unread --, parser->buffer.pointer += WIDTH(parser->buffer)) : 0)
#define READ(parser,string) (STRING_EXTEND(parser,string) ? (COPY(string,parser->buffer), parser->mark.index ++, parser->mark.column ++, parser->unread --, 1) : 0)
#define READ_LINE(parser,string) (STRING_EXTEND(parser,string) ? (((CHECK_AT(parser->buffer,'\r',0) && CHECK_AT(parser->buffer,'\n',1)) ? (*((string).pointer++) = (yaml_char_t) '\n', parser->buffer.pointer += 2, parser->mark.index += 2, parser->mark.column = 0, parser->mark.line ++, parser->unread -= 2) : (CHECK_AT(parser->buffer,'\r',0) || CHECK_AT(parser->buffer,'\n',0)) ? (*((string).pointer++) = (yaml_char_t) '\n', parser->buffer.pointer ++, parser->mark.index ++, parser->mark.column = 0, parser->mark.line ++, parser->unread --) : (CHECK_AT(parser->buffer,'\xC2',0) && CHECK_AT(parser->buffer,'\x85',1)) ? (*((string).pointer++) = (yaml_char_t) '\n', parser->buffer.pointer += 2, parser->mark.index ++, parser->mark.column = 0, parser->mark.line ++, parser->unread --) : (CHECK_AT(parser->buffer,'\xE2',0) && CHECK_AT(parser->buffer,'\x80',1) && (CHECK_AT(parser->buffer,'\xA8',2) || CHECK_AT(parser->buffer,'\xA9',2))) ? (*((string).pointer++) = *(parser->buffer.pointer++), *((string).pointer++) = *(parser->buffer.pointer++), *((string).pointer++) = *(parser->buffer.pointer++), parser->mark.index ++, parser->mark.column = 0, parser->mark.line ++, parser->unread --) : 0), 1) : 0)
YAML_DECLARE(int)
yaml_parser_scan(yaml_parser_t *parser, yaml_token_t *token);
static int
yaml_parser_set_scanner_error(yaml_parser_t *parser, const char *context,
yaml_mark_t context_mark, const char *problem);
YAML_DECLARE(int)
yaml_parser_fetch_more_tokens(yaml_parser_t *parser);
static int
yaml_parser_fetch_next_token(yaml_parser_t *parser);
static int
yaml_parser_stale_simple_keys(yaml_parser_t *parser);
static int
yaml_parser_save_simple_key(yaml_parser_t *parser);
static int
yaml_parser_remove_simple_key(yaml_parser_t *parser);
static int
yaml_parser_increase_flow_level(yaml_parser_t *parser);
static int
yaml_parser_decrease_flow_level(yaml_parser_t *parser);
static int
yaml_parser_roll_indent(yaml_parser_t *parser, ptrdiff_t column,
ptrdiff_t number, yaml_token_type_t type, yaml_mark_t mark);
static int
yaml_parser_unroll_indent(yaml_parser_t *parser, ptrdiff_t column);
static int
yaml_parser_fetch_stream_start(yaml_parser_t *parser);
static int
yaml_parser_fetch_stream_end(yaml_parser_t *parser);
static int
yaml_parser_fetch_directive(yaml_parser_t *parser);
static int
yaml_parser_fetch_document_indicator(yaml_parser_t *parser,
yaml_token_type_t type);
static int
yaml_parser_fetch_flow_collection_start(yaml_parser_t *parser,
yaml_token_type_t type);
static int
yaml_parser_fetch_flow_collection_end(yaml_parser_t *parser,
yaml_token_type_t type);
static int
yaml_parser_fetch_flow_entry(yaml_parser_t *parser);
static int
yaml_parser_fetch_block_entry(yaml_parser_t *parser);
static int
yaml_parser_fetch_key(yaml_parser_t *parser);
static int
yaml_parser_fetch_value(yaml_parser_t *parser);
static int
yaml_parser_fetch_anchor(yaml_parser_t *parser, yaml_token_type_t type);
static int
yaml_parser_fetch_tag(yaml_parser_t *parser);
static int
yaml_parser_fetch_block_scalar(yaml_parser_t *parser, int literal);
static int
yaml_parser_fetch_flow_scalar(yaml_parser_t *parser, int single);
static int
yaml_parser_fetch_plain_scalar(yaml_parser_t *parser);
static int
yaml_parser_scan_to_next_token(yaml_parser_t *parser);
static int
yaml_parser_scan_directive(yaml_parser_t *parser, yaml_token_t *token);
static int
yaml_parser_scan_directive_name(yaml_parser_t *parser,
yaml_mark_t start_mark, yaml_char_t **name);
static int
yaml_parser_scan_version_directive_value(yaml_parser_t *parser,
yaml_mark_t start_mark, int *major, int *minor);
static int
yaml_parser_scan_version_directive_number(yaml_parser_t *parser,
yaml_mark_t start_mark, int *number);
static int
yaml_parser_scan_tag_directive_value(yaml_parser_t *parser,
yaml_mark_t mark, yaml_char_t **handle, yaml_char_t **prefix);
static int
yaml_parser_scan_anchor(yaml_parser_t *parser, yaml_token_t *token,
yaml_token_type_t type);
static int
yaml_parser_scan_tag(yaml_parser_t *parser, yaml_token_t *token);
static int
yaml_parser_scan_tag_handle(yaml_parser_t *parser, int directive,
yaml_mark_t start_mark, yaml_char_t **handle);
static int
yaml_parser_scan_tag_uri(yaml_parser_t *parser, int directive,
yaml_char_t *head, yaml_mark_t start_mark, yaml_char_t **uri);
static int
yaml_parser_scan_uri_escapes(yaml_parser_t *parser, int directive,
yaml_mark_t start_mark, yaml_string_t *string);
static int
yaml_parser_scan_block_scalar(yaml_parser_t *parser, yaml_token_t *token,
int literal);
static int
yaml_parser_scan_block_scalar_breaks(yaml_parser_t *parser,
int *indent, yaml_string_t *breaks,
yaml_mark_t start_mark, yaml_mark_t *end_mark);
static int
yaml_parser_scan_flow_scalar(yaml_parser_t *parser, yaml_token_t *token,
int single);
static int
yaml_parser_scan_plain_scalar(yaml_parser_t *parser, yaml_token_t *token);
YAML_DECLARE(int)
yaml_parser_scan(yaml_parser_t *parser, yaml_token_t *token)
{
assert(parser); 
assert(token); 
memset(token, 0, sizeof(yaml_token_t));
if (parser->stream_end_produced || parser->error) {
return 1;
}
if (!parser->token_available) {
if (!yaml_parser_fetch_more_tokens(parser))
return 0;
}
*token = DEQUEUE(parser, parser->tokens);
parser->token_available = 0;
parser->tokens_parsed ++;
if (token->type == YAML_STREAM_END_TOKEN) {
parser->stream_end_produced = 1;
}
return 1;
}
static int
yaml_parser_set_scanner_error(yaml_parser_t *parser, const char *context,
yaml_mark_t context_mark, const char *problem)
{
parser->error = YAML_SCANNER_ERROR;
parser->context = context;
parser->context_mark = context_mark;
parser->problem = problem;
parser->problem_mark = parser->mark;
return 0;
}
YAML_DECLARE(int)
yaml_parser_fetch_more_tokens(yaml_parser_t *parser)
{
int need_more_tokens;
while (1)
{
need_more_tokens = 0;
if (parser->tokens.head == parser->tokens.tail)
{
need_more_tokens = 1;
}
else
{
yaml_simple_key_t *simple_key;
if (!yaml_parser_stale_simple_keys(parser))
return 0;
for (simple_key = parser->simple_keys.start;
simple_key != parser->simple_keys.top; simple_key++) {
if (simple_key->possible
&& simple_key->token_number == parser->tokens_parsed) {
need_more_tokens = 1;
break;
}
}
}
if (!need_more_tokens)
break;
if (!yaml_parser_fetch_next_token(parser))
return 0;
}
parser->token_available = 1;
return 1;
}
static int
yaml_parser_fetch_next_token(yaml_parser_t *parser)
{
if (!CACHE(parser, 1))
return 0;
if (!parser->stream_start_produced)
return yaml_parser_fetch_stream_start(parser);
if (!yaml_parser_scan_to_next_token(parser))
return 0;
if (!yaml_parser_stale_simple_keys(parser))
return 0;
if (!yaml_parser_unroll_indent(parser, parser->mark.column))
return 0;
if (!CACHE(parser, 4))
return 0;
if (IS_Z(parser->buffer))
return yaml_parser_fetch_stream_end(parser);
if (parser->mark.column == 0 && CHECK(parser->buffer, '%'))
return yaml_parser_fetch_directive(parser);
if (parser->mark.column == 0
&& CHECK_AT(parser->buffer, '-', 0)
&& CHECK_AT(parser->buffer, '-', 1)
&& CHECK_AT(parser->buffer, '-', 2)
&& IS_BLANKZ_AT(parser->buffer, 3))
return yaml_parser_fetch_document_indicator(parser,
YAML_DOCUMENT_START_TOKEN);
if (parser->mark.column == 0
&& CHECK_AT(parser->buffer, '.', 0)
&& CHECK_AT(parser->buffer, '.', 1)
&& CHECK_AT(parser->buffer, '.', 2)
&& IS_BLANKZ_AT(parser->buffer, 3))
return yaml_parser_fetch_document_indicator(parser,
YAML_DOCUMENT_END_TOKEN);
if (CHECK(parser->buffer, '['))
return yaml_parser_fetch_flow_collection_start(parser,
YAML_FLOW_SEQUENCE_START_TOKEN);
if (CHECK(parser->buffer, '{'))
return yaml_parser_fetch_flow_collection_start(parser,
YAML_FLOW_MAPPING_START_TOKEN);
if (CHECK(parser->buffer, ']'))
return yaml_parser_fetch_flow_collection_end(parser,
YAML_FLOW_SEQUENCE_END_TOKEN);
if (CHECK(parser->buffer, '}'))
return yaml_parser_fetch_flow_collection_end(parser,
YAML_FLOW_MAPPING_END_TOKEN);
if (CHECK(parser->buffer, ','))
return yaml_parser_fetch_flow_entry(parser);
if (CHECK(parser->buffer, '-') && IS_BLANKZ_AT(parser->buffer, 1))
return yaml_parser_fetch_block_entry(parser);
if (CHECK(parser->buffer, '?')
&& (parser->flow_level || IS_BLANKZ_AT(parser->buffer, 1)))
return yaml_parser_fetch_key(parser);
if (CHECK(parser->buffer, ':')
&& (parser->flow_level || IS_BLANKZ_AT(parser->buffer, 1)))
return yaml_parser_fetch_value(parser);
if (CHECK(parser->buffer, '*'))
return yaml_parser_fetch_anchor(parser, YAML_ALIAS_TOKEN);
if (CHECK(parser->buffer, '&'))
return yaml_parser_fetch_anchor(parser, YAML_ANCHOR_TOKEN);
if (CHECK(parser->buffer, '!'))
return yaml_parser_fetch_tag(parser);
if (CHECK(parser->buffer, '|') && !parser->flow_level)
return yaml_parser_fetch_block_scalar(parser, 1);
if (CHECK(parser->buffer, '>') && !parser->flow_level)
return yaml_parser_fetch_block_scalar(parser, 0);
if (CHECK(parser->buffer, '\''))
return yaml_parser_fetch_flow_scalar(parser, 1);
if (CHECK(parser->buffer, '"'))
return yaml_parser_fetch_flow_scalar(parser, 0);
if (!(IS_BLANKZ(parser->buffer) || CHECK(parser->buffer, '-')
|| CHECK(parser->buffer, '?') || CHECK(parser->buffer, ':')
|| CHECK(parser->buffer, ',') || CHECK(parser->buffer, '[')
|| CHECK(parser->buffer, ']') || CHECK(parser->buffer, '{')
|| CHECK(parser->buffer, '}') || CHECK(parser->buffer, '#')
|| CHECK(parser->buffer, '&') || CHECK(parser->buffer, '*')
|| CHECK(parser->buffer, '!') || CHECK(parser->buffer, '|')
|| CHECK(parser->buffer, '>') || CHECK(parser->buffer, '\'')
|| CHECK(parser->buffer, '"') || CHECK(parser->buffer, '%')
|| CHECK(parser->buffer, '@') || CHECK(parser->buffer, '`')) ||
(CHECK(parser->buffer, '-') && !IS_BLANK_AT(parser->buffer, 1)) ||
(!parser->flow_level &&
(CHECK(parser->buffer, '?') || CHECK(parser->buffer, ':'))
&& !IS_BLANKZ_AT(parser->buffer, 1)))
return yaml_parser_fetch_plain_scalar(parser);
return yaml_parser_set_scanner_error(parser,
"while scanning for the next token", parser->mark,
"found character that cannot start any token");
}
static int
yaml_parser_stale_simple_keys(yaml_parser_t *parser)
{
yaml_simple_key_t *simple_key;
for (simple_key = parser->simple_keys.start;
simple_key != parser->simple_keys.top; simple_key ++)
{
if (simple_key->possible
&& (simple_key->mark.line < parser->mark.line
|| simple_key->mark.index+1024 < parser->mark.index)) {
if (simple_key->required) {
return yaml_parser_set_scanner_error(parser,
"while scanning a simple key", simple_key->mark,
"could not find expected ':'");
}
simple_key->possible = 0;
}
}
return 1;
}
static int
yaml_parser_save_simple_key(yaml_parser_t *parser)
{
int required = (!parser->flow_level
&& parser->indent == (ptrdiff_t)parser->mark.column);
if (parser->simple_key_allowed)
{
yaml_simple_key_t simple_key;
simple_key.possible = 1;
simple_key.required = required;
simple_key.token_number =
parser->tokens_parsed + (parser->tokens.tail - parser->tokens.head);
simple_key.mark = parser->mark;
if (!yaml_parser_remove_simple_key(parser)) return 0;
*(parser->simple_keys.top-1) = simple_key;
}
return 1;
}
static int
yaml_parser_remove_simple_key(yaml_parser_t *parser)
{
yaml_simple_key_t *simple_key = parser->simple_keys.top-1;
if (simple_key->possible)
{
if (simple_key->required) {
return yaml_parser_set_scanner_error(parser,
"while scanning a simple key", simple_key->mark,
"could not find expected ':'");
}
}
simple_key->possible = 0;
return 1;
}
static int
yaml_parser_increase_flow_level(yaml_parser_t *parser)
{
yaml_simple_key_t empty_simple_key = { 0, 0, 0, { 0, 0, 0 } };
if (!PUSH(parser, parser->simple_keys, empty_simple_key))
return 0;
if (parser->flow_level == INT_MAX) {
parser->error = YAML_MEMORY_ERROR;
return 0;
}
parser->flow_level++;
return 1;
}
static int
yaml_parser_decrease_flow_level(yaml_parser_t *parser)
{
if (parser->flow_level) {
parser->flow_level --;
(void)POP(parser, parser->simple_keys);
}
return 1;
}
static int
yaml_parser_roll_indent(yaml_parser_t *parser, ptrdiff_t column,
ptrdiff_t number, yaml_token_type_t type, yaml_mark_t mark)
{
yaml_token_t token;
if (parser->flow_level)
return 1;
if (parser->indent < column)
{
if (!PUSH(parser, parser->indents, parser->indent))
return 0;
if (column > INT_MAX) {
parser->error = YAML_MEMORY_ERROR;
return 0;
}
parser->indent = (int)column;
TOKEN_INIT(token, type, mark, mark);
if (number == -1) {
if (!ENQUEUE(parser, parser->tokens, token))
return 0;
}
else {
if (!QUEUE_INSERT(parser,
parser->tokens, number - parser->tokens_parsed, token))
return 0;
}
}
return 1;
}
static int
yaml_parser_unroll_indent(yaml_parser_t *parser, ptrdiff_t column)
{
yaml_token_t token;
if (parser->flow_level)
return 1;
while (parser->indent > column)
{
TOKEN_INIT(token, YAML_BLOCK_END_TOKEN, parser->mark, parser->mark);
if (!ENQUEUE(parser, parser->tokens, token))
return 0;
parser->indent = POP(parser, parser->indents);
}
return 1;
}
static int
yaml_parser_fetch_stream_start(yaml_parser_t *parser)
{
yaml_simple_key_t simple_key = { 0, 0, 0, { 0, 0, 0 } };
yaml_token_t token;
parser->indent = -1;
if (!PUSH(parser, parser->simple_keys, simple_key))
return 0;
parser->simple_key_allowed = 1;
parser->stream_start_produced = 1;
STREAM_START_TOKEN_INIT(token, parser->encoding,
parser->mark, parser->mark);
if (!ENQUEUE(parser, parser->tokens, token))
return 0;
return 1;
}
static int
yaml_parser_fetch_stream_end(yaml_parser_t *parser)
{
yaml_token_t token;
if (parser->mark.column != 0) {
parser->mark.column = 0;
parser->mark.line ++;
}
if (!yaml_parser_unroll_indent(parser, -1))
return 0;
if (!yaml_parser_remove_simple_key(parser))
return 0;
parser->simple_key_allowed = 0;
STREAM_END_TOKEN_INIT(token, parser->mark, parser->mark);
if (!ENQUEUE(parser, parser->tokens, token))
return 0;
return 1;
}
static int
yaml_parser_fetch_directive(yaml_parser_t *parser)
{
yaml_token_t token;
if (!yaml_parser_unroll_indent(parser, -1))
return 0;
if (!yaml_parser_remove_simple_key(parser))
return 0;
parser->simple_key_allowed = 0;
if (!yaml_parser_scan_directive(parser, &token))
return 0;
if (!ENQUEUE(parser, parser->tokens, token)) {
yaml_token_delete(&token);
return 0;
}
return 1;
}
static int
yaml_parser_fetch_document_indicator(yaml_parser_t *parser,
yaml_token_type_t type)
{
yaml_mark_t start_mark, end_mark;
yaml_token_t token;
if (!yaml_parser_unroll_indent(parser, -1))
return 0;
if (!yaml_parser_remove_simple_key(parser))
return 0;
parser->simple_key_allowed = 0;
start_mark = parser->mark;
SKIP(parser);
SKIP(parser);
SKIP(parser);
end_mark = parser->mark;
TOKEN_INIT(token, type, start_mark, end_mark);
if (!ENQUEUE(parser, parser->tokens, token))
return 0;
return 1;
}
static int
yaml_parser_fetch_flow_collection_start(yaml_parser_t *parser,
yaml_token_type_t type)
{
yaml_mark_t start_mark, end_mark;
yaml_token_t token;
if (!yaml_parser_save_simple_key(parser))
return 0;
if (!yaml_parser_increase_flow_level(parser))
return 0;
parser->simple_key_allowed = 1;
start_mark = parser->mark;
SKIP(parser);
end_mark = parser->mark;
TOKEN_INIT(token, type, start_mark, end_mark);
if (!ENQUEUE(parser, parser->tokens, token))
return 0;
return 1;
}
static int
yaml_parser_fetch_flow_collection_end(yaml_parser_t *parser,
yaml_token_type_t type)
{
yaml_mark_t start_mark, end_mark;
yaml_token_t token;
if (!yaml_parser_remove_simple_key(parser))
return 0;
if (!yaml_parser_decrease_flow_level(parser))
return 0;
parser->simple_key_allowed = 0;
start_mark = parser->mark;
SKIP(parser);
end_mark = parser->mark;
TOKEN_INIT(token, type, start_mark, end_mark);
if (!ENQUEUE(parser, parser->tokens, token))
return 0;
return 1;
}
static int
yaml_parser_fetch_flow_entry(yaml_parser_t *parser)
{
yaml_mark_t start_mark, end_mark;
yaml_token_t token;
if (!yaml_parser_remove_simple_key(parser))
return 0;
parser->simple_key_allowed = 1;
start_mark = parser->mark;
SKIP(parser);
end_mark = parser->mark;
TOKEN_INIT(token, YAML_FLOW_ENTRY_TOKEN, start_mark, end_mark);
if (!ENQUEUE(parser, parser->tokens, token))
return 0;
return 1;
}
static int
yaml_parser_fetch_block_entry(yaml_parser_t *parser)
{
yaml_mark_t start_mark, end_mark;
yaml_token_t token;
if (!parser->flow_level)
{
if (!parser->simple_key_allowed) {
return yaml_parser_set_scanner_error(parser, NULL, parser->mark,
"block sequence entries are not allowed in this context");
}
if (!yaml_parser_roll_indent(parser, parser->mark.column, -1,
YAML_BLOCK_SEQUENCE_START_TOKEN, parser->mark))
return 0;
}
else
{
}
if (!yaml_parser_remove_simple_key(parser))
return 0;
parser->simple_key_allowed = 1;
start_mark = parser->mark;
SKIP(parser);
end_mark = parser->mark;
TOKEN_INIT(token, YAML_BLOCK_ENTRY_TOKEN, start_mark, end_mark);
if (!ENQUEUE(parser, parser->tokens, token))
return 0;
return 1;
}
static int
yaml_parser_fetch_key(yaml_parser_t *parser)
{
yaml_mark_t start_mark, end_mark;
yaml_token_t token;
if (!parser->flow_level)
{
if (!parser->simple_key_allowed) {
return yaml_parser_set_scanner_error(parser, NULL, parser->mark,
"mapping keys are not allowed in this context");
}
if (!yaml_parser_roll_indent(parser, parser->mark.column, -1,
YAML_BLOCK_MAPPING_START_TOKEN, parser->mark))
return 0;
}
if (!yaml_parser_remove_simple_key(parser))
return 0;
parser->simple_key_allowed = (!parser->flow_level);
start_mark = parser->mark;
SKIP(parser);
end_mark = parser->mark;
TOKEN_INIT(token, YAML_KEY_TOKEN, start_mark, end_mark);
if (!ENQUEUE(parser, parser->tokens, token))
return 0;
return 1;
}
static int
yaml_parser_fetch_value(yaml_parser_t *parser)
{
yaml_mark_t start_mark, end_mark;
yaml_token_t token;
yaml_simple_key_t *simple_key = parser->simple_keys.top-1;
if (simple_key->possible)
{
TOKEN_INIT(token, YAML_KEY_TOKEN, simple_key->mark, simple_key->mark);
if (!QUEUE_INSERT(parser, parser->tokens,
simple_key->token_number - parser->tokens_parsed, token))
return 0;
if (!yaml_parser_roll_indent(parser, simple_key->mark.column,
simple_key->token_number,
YAML_BLOCK_MAPPING_START_TOKEN, simple_key->mark))
return 0;
simple_key->possible = 0;
parser->simple_key_allowed = 0;
}
else
{
if (!parser->flow_level)
{
if (!parser->simple_key_allowed) {
return yaml_parser_set_scanner_error(parser, NULL, parser->mark,
"mapping values are not allowed in this context");
}
if (!yaml_parser_roll_indent(parser, parser->mark.column, -1,
YAML_BLOCK_MAPPING_START_TOKEN, parser->mark))
return 0;
}
parser->simple_key_allowed = (!parser->flow_level);
}
start_mark = parser->mark;
SKIP(parser);
end_mark = parser->mark;
TOKEN_INIT(token, YAML_VALUE_TOKEN, start_mark, end_mark);
if (!ENQUEUE(parser, parser->tokens, token))
return 0;
return 1;
}
static int
yaml_parser_fetch_anchor(yaml_parser_t *parser, yaml_token_type_t type)
{
yaml_token_t token;
if (!yaml_parser_save_simple_key(parser))
return 0;
parser->simple_key_allowed = 0;
if (!yaml_parser_scan_anchor(parser, &token, type))
return 0;
if (!ENQUEUE(parser, parser->tokens, token)) {
yaml_token_delete(&token);
return 0;
}
return 1;
}
static int
yaml_parser_fetch_tag(yaml_parser_t *parser)
{
yaml_token_t token;
if (!yaml_parser_save_simple_key(parser))
return 0;
parser->simple_key_allowed = 0;
if (!yaml_parser_scan_tag(parser, &token))
return 0;
if (!ENQUEUE(parser, parser->tokens, token)) {
yaml_token_delete(&token);
return 0;
}
return 1;
}
static int
yaml_parser_fetch_block_scalar(yaml_parser_t *parser, int literal)
{
yaml_token_t token;
if (!yaml_parser_remove_simple_key(parser))
return 0;
parser->simple_key_allowed = 1;
if (!yaml_parser_scan_block_scalar(parser, &token, literal))
return 0;
if (!ENQUEUE(parser, parser->tokens, token)) {
yaml_token_delete(&token);
return 0;
}
return 1;
}
static int
yaml_parser_fetch_flow_scalar(yaml_parser_t *parser, int single)
{
yaml_token_t token;
if (!yaml_parser_save_simple_key(parser))
return 0;
parser->simple_key_allowed = 0;
if (!yaml_parser_scan_flow_scalar(parser, &token, single))
return 0;
if (!ENQUEUE(parser, parser->tokens, token)) {
yaml_token_delete(&token);
return 0;
}
return 1;
}
static int
yaml_parser_fetch_plain_scalar(yaml_parser_t *parser)
{
yaml_token_t token;
if (!yaml_parser_save_simple_key(parser))
return 0;
parser->simple_key_allowed = 0;
if (!yaml_parser_scan_plain_scalar(parser, &token))
return 0;
if (!ENQUEUE(parser, parser->tokens, token)) {
yaml_token_delete(&token);
return 0;
}
return 1;
}
static int
yaml_parser_scan_to_next_token(yaml_parser_t *parser)
{
while (1)
{
if (!CACHE(parser, 1)) return 0;
if (parser->mark.column == 0 && IS_BOM(parser->buffer))
SKIP(parser);
if (!CACHE(parser, 1)) return 0;
while (CHECK(parser->buffer,' ') ||
((parser->flow_level || !parser->simple_key_allowed) &&
CHECK(parser->buffer, '\t'))) {
SKIP(parser);
if (!CACHE(parser, 1)) return 0;
}
if (CHECK(parser->buffer, '#')) {
while (!IS_BREAKZ(parser->buffer)) {
SKIP(parser);
if (!CACHE(parser, 1)) return 0;
}
}
if (IS_BREAK(parser->buffer))
{
if (!CACHE(parser, 2)) return 0;
SKIP_LINE(parser);
if (!parser->flow_level) {
parser->simple_key_allowed = 1;
}
}
else
{
break;
}
}
return 1;
}
int
yaml_parser_scan_directive(yaml_parser_t *parser, yaml_token_t *token)
{
yaml_mark_t start_mark, end_mark;
yaml_char_t *name = NULL;
int major, minor;
yaml_char_t *handle = NULL, *prefix = NULL;
start_mark = parser->mark;
SKIP(parser);
if (!yaml_parser_scan_directive_name(parser, start_mark, &name))
goto error;
if (strcmp((char *)name, "YAML") == 0)
{
if (!yaml_parser_scan_version_directive_value(parser, start_mark,
&major, &minor))
goto error;
end_mark = parser->mark;
VERSION_DIRECTIVE_TOKEN_INIT(*token, major, minor,
start_mark, end_mark);
}
else if (strcmp((char *)name, "TAG") == 0)
{
if (!yaml_parser_scan_tag_directive_value(parser, start_mark,
&handle, &prefix))
goto error;
end_mark = parser->mark;
TAG_DIRECTIVE_TOKEN_INIT(*token, handle, prefix,
start_mark, end_mark);
}
else
{
yaml_parser_set_scanner_error(parser, "while scanning a directive",
start_mark, "found unknown directive name");
goto error;
}
if (!CACHE(parser, 1)) goto error;
while (IS_BLANK(parser->buffer)) {
SKIP(parser);
if (!CACHE(parser, 1)) goto error;
}
if (CHECK(parser->buffer, '#')) {
while (!IS_BREAKZ(parser->buffer)) {
SKIP(parser);
if (!CACHE(parser, 1)) goto error;
}
}
if (!IS_BREAKZ(parser->buffer)) {
yaml_parser_set_scanner_error(parser, "while scanning a directive",
start_mark, "did not find expected comment or line break");
goto error;
}
if (IS_BREAK(parser->buffer)) {
if (!CACHE(parser, 2)) goto error;
SKIP_LINE(parser);
}
yaml_free(name);
return 1;
error:
yaml_free(prefix);
yaml_free(handle);
yaml_free(name);
return 0;
}
static int
yaml_parser_scan_directive_name(yaml_parser_t *parser,
yaml_mark_t start_mark, yaml_char_t **name)
{
yaml_string_t string = NULL_STRING;
if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;
if (!CACHE(parser, 1)) goto error;
while (IS_ALPHA(parser->buffer))
{
if (!READ(parser, string)) goto error;
if (!CACHE(parser, 1)) goto error;
}
if (string.start == string.pointer) {
yaml_parser_set_scanner_error(parser, "while scanning a directive",
start_mark, "could not find expected directive name");
goto error;
}
if (!IS_BLANKZ(parser->buffer)) {
yaml_parser_set_scanner_error(parser, "while scanning a directive",
start_mark, "found unexpected non-alphabetical character");
goto error;
}
*name = string.start;
return 1;
error:
STRING_DEL(parser, string);
return 0;
}
static int
yaml_parser_scan_version_directive_value(yaml_parser_t *parser,
yaml_mark_t start_mark, int *major, int *minor)
{
if (!CACHE(parser, 1)) return 0;
while (IS_BLANK(parser->buffer)) {
SKIP(parser);
if (!CACHE(parser, 1)) return 0;
}
if (!yaml_parser_scan_version_directive_number(parser, start_mark, major))
return 0;
if (!CHECK(parser->buffer, '.')) {
return yaml_parser_set_scanner_error(parser, "while scanning a %YAML directive",
start_mark, "did not find expected digit or '.' character");
}
SKIP(parser);
if (!yaml_parser_scan_version_directive_number(parser, start_mark, minor))
return 0;
return 1;
}
#define MAX_NUMBER_LENGTH 9
static int
yaml_parser_scan_version_directive_number(yaml_parser_t *parser,
yaml_mark_t start_mark, int *number)
{
int value = 0;
size_t length = 0;
if (!CACHE(parser, 1)) return 0;
while (IS_DIGIT(parser->buffer))
{
if (++length > MAX_NUMBER_LENGTH) {
return yaml_parser_set_scanner_error(parser, "while scanning a %YAML directive",
start_mark, "found extremely long version number");
}
value = value*10 + AS_DIGIT(parser->buffer);
SKIP(parser);
if (!CACHE(parser, 1)) return 0;
}
if (!length) {
return yaml_parser_set_scanner_error(parser, "while scanning a %YAML directive",
start_mark, "did not find expected version number");
}
*number = value;
return 1;
}
static int
yaml_parser_scan_tag_directive_value(yaml_parser_t *parser,
yaml_mark_t start_mark, yaml_char_t **handle, yaml_char_t **prefix)
{
yaml_char_t *handle_value = NULL;
yaml_char_t *prefix_value = NULL;
if (!CACHE(parser, 1)) goto error;
while (IS_BLANK(parser->buffer)) {
SKIP(parser);
if (!CACHE(parser, 1)) goto error;
}
if (!yaml_parser_scan_tag_handle(parser, 1, start_mark, &handle_value))
goto error;
if (!CACHE(parser, 1)) goto error;
if (!IS_BLANK(parser->buffer)) {
yaml_parser_set_scanner_error(parser, "while scanning a %TAG directive",
start_mark, "did not find expected whitespace");
goto error;
}
while (IS_BLANK(parser->buffer)) {
SKIP(parser);
if (!CACHE(parser, 1)) goto error;
}
if (!yaml_parser_scan_tag_uri(parser, 1, NULL, start_mark, &prefix_value))
goto error;
if (!CACHE(parser, 1)) goto error;
if (!IS_BLANKZ(parser->buffer)) {
yaml_parser_set_scanner_error(parser, "while scanning a %TAG directive",
start_mark, "did not find expected whitespace or line break");
goto error;
}
*handle = handle_value;
*prefix = prefix_value;
return 1;
error:
yaml_free(handle_value);
yaml_free(prefix_value);
return 0;
}
static int
yaml_parser_scan_anchor(yaml_parser_t *parser, yaml_token_t *token,
yaml_token_type_t type)
{
int length = 0;
yaml_mark_t start_mark, end_mark;
yaml_string_t string = NULL_STRING;
if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;
start_mark = parser->mark;
SKIP(parser);
if (!CACHE(parser, 1)) goto error;
while (IS_ALPHA(parser->buffer)) {
if (!READ(parser, string)) goto error;
if (!CACHE(parser, 1)) goto error;
length ++;
}
end_mark = parser->mark;
if (!length || !(IS_BLANKZ(parser->buffer) || CHECK(parser->buffer, '?')
|| CHECK(parser->buffer, ':') || CHECK(parser->buffer, ',')
|| CHECK(parser->buffer, ']') || CHECK(parser->buffer, '}')
|| CHECK(parser->buffer, '%') || CHECK(parser->buffer, '@')
|| CHECK(parser->buffer, '`'))) {
yaml_parser_set_scanner_error(parser, type == YAML_ANCHOR_TOKEN ?
"while scanning an anchor" : "while scanning an alias", start_mark,
"did not find expected alphabetic or numeric character");
goto error;
}
if (type == YAML_ANCHOR_TOKEN) {
ANCHOR_TOKEN_INIT(*token, string.start, start_mark, end_mark);
}
else {
ALIAS_TOKEN_INIT(*token, string.start, start_mark, end_mark);
}
return 1;
error:
STRING_DEL(parser, string);
return 0;
}
static int
yaml_parser_scan_tag(yaml_parser_t *parser, yaml_token_t *token)
{
yaml_char_t *handle = NULL;
yaml_char_t *suffix = NULL;
yaml_mark_t start_mark, end_mark;
start_mark = parser->mark;
if (!CACHE(parser, 2)) goto error;
if (CHECK_AT(parser->buffer, '<', 1))
{
handle = YAML_MALLOC(1);
if (!handle) goto error;
handle[0] = '\0';
SKIP(parser);
SKIP(parser);
if (!yaml_parser_scan_tag_uri(parser, 0, NULL, start_mark, &suffix))
goto error;
if (!CHECK(parser->buffer, '>')) {
yaml_parser_set_scanner_error(parser, "while scanning a tag",
start_mark, "did not find the expected '>'");
goto error;
}
SKIP(parser);
}
else
{
if (!yaml_parser_scan_tag_handle(parser, 0, start_mark, &handle))
goto error;
if (handle[0] == '!' && handle[1] != '\0' && handle[strlen((char *)handle)-1] == '!')
{
if (!yaml_parser_scan_tag_uri(parser, 0, NULL, start_mark, &suffix))
goto error;
}
else
{
if (!yaml_parser_scan_tag_uri(parser, 0, handle, start_mark, &suffix))
goto error;
yaml_free(handle);
handle = YAML_MALLOC(2);
if (!handle) goto error;
handle[0] = '!';
handle[1] = '\0';
if (suffix[0] == '\0') {
yaml_char_t *tmp = handle;
handle = suffix;
suffix = tmp;
}
}
}
if (!CACHE(parser, 1)) goto error;
if (!IS_BLANKZ(parser->buffer)) {
yaml_parser_set_scanner_error(parser, "while scanning a tag",
start_mark, "did not find expected whitespace or line break");
goto error;
}
end_mark = parser->mark;
TAG_TOKEN_INIT(*token, handle, suffix, start_mark, end_mark);
return 1;
error:
yaml_free(handle);
yaml_free(suffix);
return 0;
}
static int
yaml_parser_scan_tag_handle(yaml_parser_t *parser, int directive,
yaml_mark_t start_mark, yaml_char_t **handle)
{
yaml_string_t string = NULL_STRING;
if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;
if (!CACHE(parser, 1)) goto error;
if (!CHECK(parser->buffer, '!')) {
yaml_parser_set_scanner_error(parser, directive ?
"while scanning a tag directive" : "while scanning a tag",
start_mark, "did not find expected '!'");
goto error;
}
if (!READ(parser, string)) goto error;
if (!CACHE(parser, 1)) goto error;
while (IS_ALPHA(parser->buffer))
{
if (!READ(parser, string)) goto error;
if (!CACHE(parser, 1)) goto error;
}
if (CHECK(parser->buffer, '!'))
{
if (!READ(parser, string)) goto error;
}
else
{
if (directive && !(string.start[0] == '!' && string.start[1] == '\0')) {
yaml_parser_set_scanner_error(parser, "while parsing a tag directive",
start_mark, "did not find expected '!'");
goto error;
}
}
*handle = string.start;
return 1;
error:
STRING_DEL(parser, string);
return 0;
}
static int
yaml_parser_scan_tag_uri(yaml_parser_t *parser, int directive,
yaml_char_t *head, yaml_mark_t start_mark, yaml_char_t **uri)
{
size_t length = head ? strlen((char *)head) : 0;
yaml_string_t string = NULL_STRING;
if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;
while ((size_t)(string.end - string.start) <= length) {
if (!yaml_string_extend(&string.start, &string.pointer, &string.end)) {
parser->error = YAML_MEMORY_ERROR;
goto error;
}
}
if (length > 1) {
memcpy(string.start, head+1, length-1);
string.pointer += length-1;
}
if (!CACHE(parser, 1)) goto error;
while (IS_ALPHA(parser->buffer) || CHECK(parser->buffer, ';')
|| CHECK(parser->buffer, '/') || CHECK(parser->buffer, '?')
|| CHECK(parser->buffer, ':') || CHECK(parser->buffer, '@')
|| CHECK(parser->buffer, '&') || CHECK(parser->buffer, '=')
|| CHECK(parser->buffer, '+') || CHECK(parser->buffer, '$')
|| CHECK(parser->buffer, ',') || CHECK(parser->buffer, '.')
|| CHECK(parser->buffer, '!') || CHECK(parser->buffer, '~')
|| CHECK(parser->buffer, '*') || CHECK(parser->buffer, '\'')
|| CHECK(parser->buffer, '(') || CHECK(parser->buffer, ')')
|| CHECK(parser->buffer, '[') || CHECK(parser->buffer, ']')
|| CHECK(parser->buffer, '%'))
{
if (CHECK(parser->buffer, '%')) {
if (!STRING_EXTEND(parser, string))
goto error;
if (!yaml_parser_scan_uri_escapes(parser,
directive, start_mark, &string)) goto error;
}
else {
if (!READ(parser, string)) goto error;
}
length ++;
if (!CACHE(parser, 1)) goto error;
}
if (!length) {
if (!STRING_EXTEND(parser, string))
goto error;
yaml_parser_set_scanner_error(parser, directive ?
"while parsing a %TAG directive" : "while parsing a tag",
start_mark, "did not find expected tag URI");
goto error;
}
*uri = string.start;
return 1;
error:
STRING_DEL(parser, string);
return 0;
}
static int
yaml_parser_scan_uri_escapes(yaml_parser_t *parser, int directive,
yaml_mark_t start_mark, yaml_string_t *string)
{
int width = 0;
do {
unsigned char octet = 0;
if (!CACHE(parser, 3)) return 0;
if (!(CHECK(parser->buffer, '%')
&& IS_HEX_AT(parser->buffer, 1)
&& IS_HEX_AT(parser->buffer, 2))) {
return yaml_parser_set_scanner_error(parser, directive ?
"while parsing a %TAG directive" : "while parsing a tag",
start_mark, "did not find URI escaped octet");
}
octet = (AS_HEX_AT(parser->buffer, 1) << 4) + AS_HEX_AT(parser->buffer, 2);
if (!width)
{
width = (octet & 0x80) == 0x00 ? 1 :
(octet & 0xE0) == 0xC0 ? 2 :
(octet & 0xF0) == 0xE0 ? 3 :
(octet & 0xF8) == 0xF0 ? 4 : 0;
if (!width) {
return yaml_parser_set_scanner_error(parser, directive ?
"while parsing a %TAG directive" : "while parsing a tag",
start_mark, "found an incorrect leading UTF-8 octet");
}
}
else
{
if ((octet & 0xC0) != 0x80) {
return yaml_parser_set_scanner_error(parser, directive ?
"while parsing a %TAG directive" : "while parsing a tag",
start_mark, "found an incorrect trailing UTF-8 octet");
}
}
*(string->pointer++) = octet;
SKIP(parser);
SKIP(parser);
SKIP(parser);
} while (--width);
return 1;
}
static int
yaml_parser_scan_block_scalar(yaml_parser_t *parser, yaml_token_t *token,
int literal)
{
yaml_mark_t start_mark;
yaml_mark_t end_mark;
yaml_string_t string = NULL_STRING;
yaml_string_t leading_break = NULL_STRING;
yaml_string_t trailing_breaks = NULL_STRING;
int chomping = 0;
int increment = 0;
int indent = 0;
int leading_blank = 0;
int trailing_blank = 0;
if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;
if (!STRING_INIT(parser, leading_break, INITIAL_STRING_SIZE)) goto error;
if (!STRING_INIT(parser, trailing_breaks, INITIAL_STRING_SIZE)) goto error;
start_mark = parser->mark;
SKIP(parser);
if (!CACHE(parser, 1)) goto error;
if (CHECK(parser->buffer, '+') || CHECK(parser->buffer, '-'))
{
chomping = CHECK(parser->buffer, '+') ? +1 : -1;
SKIP(parser);
if (!CACHE(parser, 1)) goto error;
if (IS_DIGIT(parser->buffer))
{
if (CHECK(parser->buffer, '0')) {
yaml_parser_set_scanner_error(parser, "while scanning a block scalar",
start_mark, "found an indentation indicator equal to 0");
goto error;
}
increment = AS_DIGIT(parser->buffer);
SKIP(parser);
}
}
else if (IS_DIGIT(parser->buffer))
{
if (CHECK(parser->buffer, '0')) {
yaml_parser_set_scanner_error(parser, "while scanning a block scalar",
start_mark, "found an indentation indicator equal to 0");
goto error;
}
increment = AS_DIGIT(parser->buffer);
SKIP(parser);
if (!CACHE(parser, 1)) goto error;
if (CHECK(parser->buffer, '+') || CHECK(parser->buffer, '-')) {
chomping = CHECK(parser->buffer, '+') ? +1 : -1;
SKIP(parser);
}
}
if (!CACHE(parser, 1)) goto error;
while (IS_BLANK(parser->buffer)) {
SKIP(parser);
if (!CACHE(parser, 1)) goto error;
}
if (CHECK(parser->buffer, '#')) {
while (!IS_BREAKZ(parser->buffer)) {
SKIP(parser);
if (!CACHE(parser, 1)) goto error;
}
}
if (!IS_BREAKZ(parser->buffer)) {
yaml_parser_set_scanner_error(parser, "while scanning a block scalar",
start_mark, "did not find expected comment or line break");
goto error;
}
if (IS_BREAK(parser->buffer)) {
if (!CACHE(parser, 2)) goto error;
SKIP_LINE(parser);
}
end_mark = parser->mark;
if (increment) {
indent = parser->indent >= 0 ? parser->indent+increment : increment;
}
if (!yaml_parser_scan_block_scalar_breaks(parser, &indent, &trailing_breaks,
start_mark, &end_mark)) goto error;
if (!CACHE(parser, 1)) goto error;
while ((int)parser->mark.column == indent && !IS_Z(parser->buffer))
{
trailing_blank = IS_BLANK(parser->buffer);
if (!literal && (*leading_break.start == '\n')
&& !leading_blank && !trailing_blank)
{
if (*trailing_breaks.start == '\0') {
if (!STRING_EXTEND(parser, string)) goto error;
*(string.pointer ++) = ' ';
}
CLEAR(parser, leading_break);
}
else {
if (!JOIN(parser, string, leading_break)) goto error;
CLEAR(parser, leading_break);
}
if (!JOIN(parser, string, trailing_breaks)) goto error;
CLEAR(parser, trailing_breaks);
leading_blank = IS_BLANK(parser->buffer);
while (!IS_BREAKZ(parser->buffer)) {
if (!READ(parser, string)) goto error;
if (!CACHE(parser, 1)) goto error;
}
if (!CACHE(parser, 2)) goto error;
if (!READ_LINE(parser, leading_break)) goto error;
if (!yaml_parser_scan_block_scalar_breaks(parser,
&indent, &trailing_breaks, start_mark, &end_mark)) goto error;
}
if (chomping != -1) {
if (!JOIN(parser, string, leading_break)) goto error;
}
if (chomping == 1) {
if (!JOIN(parser, string, trailing_breaks)) goto error;
}
SCALAR_TOKEN_INIT(*token, string.start, string.pointer-string.start,
literal ? YAML_LITERAL_SCALAR_STYLE : YAML_FOLDED_SCALAR_STYLE,
start_mark, end_mark);
STRING_DEL(parser, leading_break);
STRING_DEL(parser, trailing_breaks);
return 1;
error:
STRING_DEL(parser, string);
STRING_DEL(parser, leading_break);
STRING_DEL(parser, trailing_breaks);
return 0;
}
static int
yaml_parser_scan_block_scalar_breaks(yaml_parser_t *parser,
int *indent, yaml_string_t *breaks,
yaml_mark_t start_mark, yaml_mark_t *end_mark)
{
int max_indent = 0;
*end_mark = parser->mark;
while (1)
{
if (!CACHE(parser, 1)) return 0;
while ((!*indent || (int)parser->mark.column < *indent)
&& IS_SPACE(parser->buffer)) {
SKIP(parser);
if (!CACHE(parser, 1)) return 0;
}
if ((int)parser->mark.column > max_indent)
max_indent = (int)parser->mark.column;
if ((!*indent || (int)parser->mark.column < *indent)
&& IS_TAB(parser->buffer)) {
return yaml_parser_set_scanner_error(parser, "while scanning a block scalar",
start_mark, "found a tab character where an indentation space is expected");
}
if (!IS_BREAK(parser->buffer)) break;
if (!CACHE(parser, 2)) return 0;
if (!READ_LINE(parser, *breaks)) return 0;
*end_mark = parser->mark;
}
if (!*indent) {
*indent = max_indent;
if (*indent < parser->indent + 1)
*indent = parser->indent + 1;
if (*indent < 1)
*indent = 1;
}
return 1;
}
static int
yaml_parser_scan_flow_scalar(yaml_parser_t *parser, yaml_token_t *token,
int single)
{
yaml_mark_t start_mark;
yaml_mark_t end_mark;
yaml_string_t string = NULL_STRING;
yaml_string_t leading_break = NULL_STRING;
yaml_string_t trailing_breaks = NULL_STRING;
yaml_string_t whitespaces = NULL_STRING;
int leading_blanks;
if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;
if (!STRING_INIT(parser, leading_break, INITIAL_STRING_SIZE)) goto error;
if (!STRING_INIT(parser, trailing_breaks, INITIAL_STRING_SIZE)) goto error;
if (!STRING_INIT(parser, whitespaces, INITIAL_STRING_SIZE)) goto error;
start_mark = parser->mark;
SKIP(parser);
while (1)
{
if (!CACHE(parser, 4)) goto error;
if (parser->mark.column == 0 &&
((CHECK_AT(parser->buffer, '-', 0) &&
CHECK_AT(parser->buffer, '-', 1) &&
CHECK_AT(parser->buffer, '-', 2)) ||
(CHECK_AT(parser->buffer, '.', 0) &&
CHECK_AT(parser->buffer, '.', 1) &&
CHECK_AT(parser->buffer, '.', 2))) &&
IS_BLANKZ_AT(parser->buffer, 3))
{
yaml_parser_set_scanner_error(parser, "while scanning a quoted scalar",
start_mark, "found unexpected document indicator");
goto error;
}
if (IS_Z(parser->buffer)) {
yaml_parser_set_scanner_error(parser, "while scanning a quoted scalar",
start_mark, "found unexpected end of stream");
goto error;
}
if (!CACHE(parser, 2)) goto error;
leading_blanks = 0;
while (!IS_BLANKZ(parser->buffer))
{
if (single && CHECK_AT(parser->buffer, '\'', 0)
&& CHECK_AT(parser->buffer, '\'', 1))
{
if (!STRING_EXTEND(parser, string)) goto error;
*(string.pointer++) = '\'';
SKIP(parser);
SKIP(parser);
}
else if (CHECK(parser->buffer, single ? '\'' : '"'))
{
break;
}
else if (!single && CHECK(parser->buffer, '\\')
&& IS_BREAK_AT(parser->buffer, 1))
{
if (!CACHE(parser, 3)) goto error;
SKIP(parser);
SKIP_LINE(parser);
leading_blanks = 1;
break;
}
else if (!single && CHECK(parser->buffer, '\\'))
{
size_t code_length = 0;
if (!STRING_EXTEND(parser, string)) goto error;
switch (parser->buffer.pointer[1])
{
case '0':
*(string.pointer++) = '\0';
break;
case 'a':
*(string.pointer++) = '\x07';
break;
case 'b':
*(string.pointer++) = '\x08';
break;
case 't':
case '\t':
*(string.pointer++) = '\x09';
break;
case 'n':
*(string.pointer++) = '\x0A';
break;
case 'v':
*(string.pointer++) = '\x0B';
break;
case 'f':
*(string.pointer++) = '\x0C';
break;
case 'r':
*(string.pointer++) = '\x0D';
break;
case 'e':
*(string.pointer++) = '\x1B';
break;
case ' ':
*(string.pointer++) = '\x20';
break;
case '"':
*(string.pointer++) = '"';
break;
case '/':
*(string.pointer++) = '/';
break;
case '\\':
*(string.pointer++) = '\\';
break;
case 'N': 
*(string.pointer++) = '\xC2';
*(string.pointer++) = '\x85';
break;
case '_': 
*(string.pointer++) = '\xC2';
*(string.pointer++) = '\xA0';
break;
case 'L': 
*(string.pointer++) = '\xE2';
*(string.pointer++) = '\x80';
*(string.pointer++) = '\xA8';
break;
case 'P': 
*(string.pointer++) = '\xE2';
*(string.pointer++) = '\x80';
*(string.pointer++) = '\xA9';
break;
case 'x':
code_length = 2;
break;
case 'u':
code_length = 4;
break;
case 'U':
code_length = 8;
break;
default:
yaml_parser_set_scanner_error(parser, "while parsing a quoted scalar",
start_mark, "found unknown escape character");
goto error;
}
SKIP(parser);
SKIP(parser);
if (code_length)
{
unsigned int value = 0;
size_t k;
if (!CACHE(parser, code_length)) goto error;
for (k = 0; k < code_length; k ++) {
if (!IS_HEX_AT(parser->buffer, k)) {
yaml_parser_set_scanner_error(parser, "while parsing a quoted scalar",
start_mark, "did not find expected hexdecimal number");
goto error;
}
value = (value << 4) + AS_HEX_AT(parser->buffer, k);
}
if ((value >= 0xD800 && value <= 0xDFFF) || value > 0x10FFFF) {
yaml_parser_set_scanner_error(parser, "while parsing a quoted scalar",
start_mark, "found invalid Unicode character escape code");
goto error;
}
if (value <= 0x7F) {
*(string.pointer++) = value;
}
else if (value <= 0x7FF) {
*(string.pointer++) = 0xC0 + (value >> 6);
*(string.pointer++) = 0x80 + (value & 0x3F);
}
else if (value <= 0xFFFF) {
*(string.pointer++) = 0xE0 + (value >> 12);
*(string.pointer++) = 0x80 + ((value >> 6) & 0x3F);
*(string.pointer++) = 0x80 + (value & 0x3F);
}
else {
*(string.pointer++) = 0xF0 + (value >> 18);
*(string.pointer++) = 0x80 + ((value >> 12) & 0x3F);
*(string.pointer++) = 0x80 + ((value >> 6) & 0x3F);
*(string.pointer++) = 0x80 + (value & 0x3F);
}
for (k = 0; k < code_length; k ++) {
SKIP(parser);
}
}
}
else
{
if (!READ(parser, string)) goto error;
}
if (!CACHE(parser, 2)) goto error;
}
if (!CACHE(parser, 1)) goto error;
if (CHECK(parser->buffer, single ? '\'' : '"'))
break;
if (!CACHE(parser, 1)) goto error;
while (IS_BLANK(parser->buffer) || IS_BREAK(parser->buffer))
{
if (IS_BLANK(parser->buffer))
{
if (!leading_blanks) {
if (!READ(parser, whitespaces)) goto error;
}
else {
SKIP(parser);
}
}
else
{
if (!CACHE(parser, 2)) goto error;
if (!leading_blanks)
{
CLEAR(parser, whitespaces);
if (!READ_LINE(parser, leading_break)) goto error;
leading_blanks = 1;
}
else
{
if (!READ_LINE(parser, trailing_breaks)) goto error;
}
}
if (!CACHE(parser, 1)) goto error;
}
if (leading_blanks)
{
if (leading_break.start[0] == '\n') {
if (trailing_breaks.start[0] == '\0') {
if (!STRING_EXTEND(parser, string)) goto error;
*(string.pointer++) = ' ';
}
else {
if (!JOIN(parser, string, trailing_breaks)) goto error;
CLEAR(parser, trailing_breaks);
}
CLEAR(parser, leading_break);
}
else {
if (!JOIN(parser, string, leading_break)) goto error;
if (!JOIN(parser, string, trailing_breaks)) goto error;
CLEAR(parser, leading_break);
CLEAR(parser, trailing_breaks);
}
}
else
{
if (!JOIN(parser, string, whitespaces)) goto error;
CLEAR(parser, whitespaces);
}
}
SKIP(parser);
end_mark = parser->mark;
SCALAR_TOKEN_INIT(*token, string.start, string.pointer-string.start,
single ? YAML_SINGLE_QUOTED_SCALAR_STYLE : YAML_DOUBLE_QUOTED_SCALAR_STYLE,
start_mark, end_mark);
STRING_DEL(parser, leading_break);
STRING_DEL(parser, trailing_breaks);
STRING_DEL(parser, whitespaces);
return 1;
error:
STRING_DEL(parser, string);
STRING_DEL(parser, leading_break);
STRING_DEL(parser, trailing_breaks);
STRING_DEL(parser, whitespaces);
return 0;
}
static int
yaml_parser_scan_plain_scalar(yaml_parser_t *parser, yaml_token_t *token)
{
yaml_mark_t start_mark;
yaml_mark_t end_mark;
yaml_string_t string = NULL_STRING;
yaml_string_t leading_break = NULL_STRING;
yaml_string_t trailing_breaks = NULL_STRING;
yaml_string_t whitespaces = NULL_STRING;
int leading_blanks = 0;
int indent = parser->indent+1;
if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;
if (!STRING_INIT(parser, leading_break, INITIAL_STRING_SIZE)) goto error;
if (!STRING_INIT(parser, trailing_breaks, INITIAL_STRING_SIZE)) goto error;
if (!STRING_INIT(parser, whitespaces, INITIAL_STRING_SIZE)) goto error;
start_mark = end_mark = parser->mark;
while (1)
{
if (!CACHE(parser, 4)) goto error;
if (parser->mark.column == 0 &&
((CHECK_AT(parser->buffer, '-', 0) &&
CHECK_AT(parser->buffer, '-', 1) &&
CHECK_AT(parser->buffer, '-', 2)) ||
(CHECK_AT(parser->buffer, '.', 0) &&
CHECK_AT(parser->buffer, '.', 1) &&
CHECK_AT(parser->buffer, '.', 2))) &&
IS_BLANKZ_AT(parser->buffer, 3)) break;
if (CHECK(parser->buffer, '#'))
break;
while (!IS_BLANKZ(parser->buffer))
{
if (parser->flow_level
&& CHECK(parser->buffer, ':')
&& !IS_BLANKZ_AT(parser->buffer, 1)) {
yaml_parser_set_scanner_error(parser, "while scanning a plain scalar",
start_mark, "found unexpected ':'");
goto error;
}
if ((CHECK(parser->buffer, ':') && IS_BLANKZ_AT(parser->buffer, 1))
|| (parser->flow_level &&
(CHECK(parser->buffer, ',') || CHECK(parser->buffer, ':')
|| CHECK(parser->buffer, '?') || CHECK(parser->buffer, '[')
|| CHECK(parser->buffer, ']') || CHECK(parser->buffer, '{')
|| CHECK(parser->buffer, '}'))))
break;
if (leading_blanks || whitespaces.start != whitespaces.pointer)
{
if (leading_blanks)
{
if (leading_break.start[0] == '\n') {
if (trailing_breaks.start[0] == '\0') {
if (!STRING_EXTEND(parser, string)) goto error;
*(string.pointer++) = ' ';
}
else {
if (!JOIN(parser, string, trailing_breaks)) goto error;
CLEAR(parser, trailing_breaks);
}
CLEAR(parser, leading_break);
}
else {
if (!JOIN(parser, string, leading_break)) goto error;
if (!JOIN(parser, string, trailing_breaks)) goto error;
CLEAR(parser, leading_break);
CLEAR(parser, trailing_breaks);
}
leading_blanks = 0;
}
else
{
if (!JOIN(parser, string, whitespaces)) goto error;
CLEAR(parser, whitespaces);
}
}
if (!READ(parser, string)) goto error;
end_mark = parser->mark;
if (!CACHE(parser, 2)) goto error;
}
if (!(IS_BLANK(parser->buffer) || IS_BREAK(parser->buffer)))
break;
if (!CACHE(parser, 1)) goto error;
while (IS_BLANK(parser->buffer) || IS_BREAK(parser->buffer))
{
if (IS_BLANK(parser->buffer))
{
if (leading_blanks && (int)parser->mark.column < indent
&& IS_TAB(parser->buffer)) {
yaml_parser_set_scanner_error(parser, "while scanning a plain scalar",
start_mark, "found a tab character that violate indentation");
goto error;
}
if (!leading_blanks) {
if (!READ(parser, whitespaces)) goto error;
}
else {
SKIP(parser);
}
}
else
{
if (!CACHE(parser, 2)) goto error;
if (!leading_blanks)
{
CLEAR(parser, whitespaces);
if (!READ_LINE(parser, leading_break)) goto error;
leading_blanks = 1;
}
else
{
if (!READ_LINE(parser, trailing_breaks)) goto error;
}
}
if (!CACHE(parser, 1)) goto error;
}
if (!parser->flow_level && (int)parser->mark.column < indent)
break;
}
SCALAR_TOKEN_INIT(*token, string.start, string.pointer-string.start,
YAML_PLAIN_SCALAR_STYLE, start_mark, end_mark);
if (leading_blanks) {
parser->simple_key_allowed = 1;
}
STRING_DEL(parser, leading_break);
STRING_DEL(parser, trailing_breaks);
STRING_DEL(parser, whitespaces);
return 1;
error:
STRING_DEL(parser, string);
STRING_DEL(parser, leading_break);
STRING_DEL(parser, trailing_breaks);
STRING_DEL(parser, whitespaces);
return 0;
}
