#include <stdio.h>
#include "./lexer.h"
#include "./subtree.h"
#include "./length.h"
#include "./unicode.h"

#define LOG(message, character) if (self->logger.log) { snprintf( self->debug_buffer, TREE_SITTER_SERIALIZATION_BUFFER_SIZE, 32 <= character && character < 127 ? message " character:'%c'" : message " character:%d", character ); self->logger.log( self->logger.payload, TSLogTypeLex, self->debug_buffer ); }
















static const int32_t BYTE_ORDER_MARK = 0xFEFF;

static const TSRange DEFAULT_RANGE = {
.start_point = {
.row = 0,
.column = 0,
},
.end_point = {
.row = UINT32_MAX,
.column = UINT32_MAX,
},
.start_byte = 0,
.end_byte = UINT32_MAX
};




static bool ts_lexer__eof(const TSLexer *_self) {
Lexer *self = (Lexer *)_self;
return self->current_included_range_index == self->included_range_count;
}



static void ts_lexer__clear_chunk(Lexer *self) {
self->chunk = NULL;
self->chunk_size = 0;
self->chunk_start = 0;
}



static void ts_lexer__get_chunk(Lexer *self) {
self->chunk_start = self->current_position.bytes;
self->chunk = self->input.read(
self->input.payload,
self->current_position.bytes,
self->current_position.extent,
&self->chunk_size
);
if (!self->chunk_size) {
self->current_included_range_index = self->included_range_count;
self->chunk = NULL;
}
}




static void ts_lexer__get_lookahead(Lexer *self) {
uint32_t position_in_chunk = self->current_position.bytes - self->chunk_start;
const uint8_t *chunk = (const uint8_t *)self->chunk + position_in_chunk;
uint32_t size = self->chunk_size - position_in_chunk;

if (size == 0) {
self->lookahead_size = 1;
self->data.lookahead = '\0';
return;
}

UnicodeDecodeFunction decode = self->input.encoding == TSInputEncodingUTF8
? ts_decode_utf8
: ts_decode_utf16;

self->lookahead_size = decode(chunk, size, &self->data.lookahead);



if (self->data.lookahead == TS_DECODE_ERROR && size < 4) {
ts_lexer__get_chunk(self);
chunk = (const uint8_t *)self->chunk;
size = self->chunk_size;
self->lookahead_size = decode(chunk, size, &self->data.lookahead);
}

if (self->data.lookahead == TS_DECODE_ERROR) {
self->lookahead_size = 1;
}
}



static void ts_lexer__advance(TSLexer *_self, bool skip) {
Lexer *self = (Lexer *)_self;
if (!self->chunk) return;

if (skip) {
LOG("skip", self->data.lookahead);
} else {
LOG("consume", self->data.lookahead);
}

if (self->lookahead_size) {
self->current_position.bytes += self->lookahead_size;
if (self->data.lookahead == '\n') {
self->current_position.extent.row++;
self->current_position.extent.column = 0;
} else {
self->current_position.extent.column += self->lookahead_size;
}
}

const TSRange *current_range = NULL;
if (self->current_included_range_index < self->included_range_count) {
current_range = &self->included_ranges[self->current_included_range_index];
if (self->current_position.bytes == current_range->end_byte) {
self->current_included_range_index++;
if (self->current_included_range_index < self->included_range_count) {
current_range++;
self->current_position = (Length) {
current_range->start_byte,
current_range->start_point,
};
} else {
current_range = NULL;
}
}
}

if (skip) self->token_start_position = self->current_position;

if (current_range) {
if (self->current_position.bytes >= self->chunk_start + self->chunk_size) {
ts_lexer__get_chunk(self);
}
ts_lexer__get_lookahead(self);
} else {
ts_lexer__clear_chunk(self);
self->data.lookahead = '\0';
self->lookahead_size = 1;
}
}



static void ts_lexer__mark_end(TSLexer *_self) {
Lexer *self = (Lexer *)_self;
if (!ts_lexer__eof(&self->data)) {



TSRange *current_included_range = &self->included_ranges[
self->current_included_range_index
];
if (
self->current_included_range_index > 0 &&
self->current_position.bytes == current_included_range->start_byte
) {
TSRange *previous_included_range = current_included_range - 1;
self->token_end_position = (Length) {
previous_included_range->end_byte,
previous_included_range->end_point,
};
return;
}
}
self->token_end_position = self->current_position;
}

static uint32_t ts_lexer__get_column(TSLexer *_self) {
Lexer *self = (Lexer *)_self;
uint32_t goal_byte = self->current_position.bytes;

self->current_position.bytes -= self->current_position.extent.column;
self->current_position.extent.column = 0;

if (self->current_position.bytes < self->chunk_start) {
ts_lexer__get_chunk(self);
}

uint32_t result = 0;
while (self->current_position.bytes < goal_byte) {
ts_lexer__advance(&self->data, false);
result++;
}

return result;
}




static bool ts_lexer__is_at_included_range_start(const TSLexer *_self) {
const Lexer *self = (const Lexer *)_self;
if (self->current_included_range_index < self->included_range_count) {
TSRange *current_range = &self->included_ranges[self->current_included_range_index];
return self->current_position.bytes == current_range->start_byte;
} else {
return false;
}
}

void ts_lexer_init(Lexer *self) {
*self = (Lexer) {
.data = {



.advance = ts_lexer__advance,
.mark_end = ts_lexer__mark_end,
.get_column = ts_lexer__get_column,
.is_at_included_range_start = ts_lexer__is_at_included_range_start,
.eof = ts_lexer__eof,
.lookahead = 0,
.result_symbol = 0,
},
.chunk = NULL,
.chunk_size = 0,
.chunk_start = 0,
.current_position = {0, {0, 0}},
.logger = {
.payload = NULL,
.log = NULL
},
.included_ranges = NULL,
.included_range_count = 0,
.current_included_range_index = 0,
};
ts_lexer_set_included_ranges(self, NULL, 0);
}

void ts_lexer_delete(Lexer *self) {
ts_free(self->included_ranges);
}

static void ts_lexer_goto(Lexer *self, Length position) {
self->current_position = position;
bool found_included_range = false;


for (unsigned i = 0; i < self->included_range_count; i++) {
TSRange *included_range = &self->included_ranges[i];
if (included_range->end_byte > position.bytes) {
if (included_range->start_byte > position.bytes) {
self->current_position = (Length) {
.bytes = included_range->start_byte,
.extent = included_range->start_point,
};
}

self->current_included_range_index = i;
found_included_range = true;
break;
}
}

if (found_included_range) {


if (self->chunk && (
position.bytes < self->chunk_start ||
position.bytes >= self->chunk_start + self->chunk_size
)) {
ts_lexer__clear_chunk(self);
}

self->lookahead_size = 0;
self->data.lookahead = '\0';
}



else {
self->current_included_range_index = self->included_range_count;
TSRange *last_included_range = &self->included_ranges[self->included_range_count - 1];
self->current_position = (Length) {
.bytes = last_included_range->end_byte,
.extent = last_included_range->end_point,
};
ts_lexer__clear_chunk(self);
self->lookahead_size = 1;
self->data.lookahead = '\0';
}
}

void ts_lexer_set_input(Lexer *self, TSInput input) {
self->input = input;
ts_lexer__clear_chunk(self);
ts_lexer_goto(self, self->current_position);
}



void ts_lexer_reset(Lexer *self, Length position) {
if (position.bytes != self->current_position.bytes) {
ts_lexer_goto(self, position);
}
}

void ts_lexer_start(Lexer *self) {
self->token_start_position = self->current_position;
self->token_end_position = LENGTH_UNDEFINED;
self->data.result_symbol = 0;
if (!ts_lexer__eof(&self->data)) {
if (!self->chunk_size) ts_lexer__get_chunk(self);
if (!self->lookahead_size) ts_lexer__get_lookahead(self);
if (
self->current_position.bytes == 0 &&
self->data.lookahead == BYTE_ORDER_MARK
) ts_lexer__advance(&self->data, true);
}
}

void ts_lexer_finish(Lexer *self, uint32_t *lookahead_end_byte) {
if (length_is_undefined(self->token_end_position)) {
ts_lexer__mark_end(&self->data);
}

uint32_t current_lookahead_end_byte = self->current_position.bytes + 1;





if (self->data.lookahead == TS_DECODE_ERROR) {
current_lookahead_end_byte++;
}

if (current_lookahead_end_byte > *lookahead_end_byte) {
*lookahead_end_byte = current_lookahead_end_byte;
}
}

void ts_lexer_advance_to_end(Lexer *self) {
while (self->chunk) {
ts_lexer__advance(&self->data, false);
}
}

void ts_lexer_mark_end(Lexer *self) {
ts_lexer__mark_end(&self->data);
}

bool ts_lexer_set_included_ranges(
Lexer *self,
const TSRange *ranges,
uint32_t count
) {
if (count == 0 || !ranges) {
ranges = &DEFAULT_RANGE;
count = 1;
} else {
uint32_t previous_byte = 0;
for (unsigned i = 0; i < count; i++) {
const TSRange *range = &ranges[i];
if (
range->start_byte < previous_byte ||
range->end_byte < range->start_byte
) return false;
previous_byte = range->end_byte;
}
}

size_t size = count * sizeof(TSRange);
self->included_ranges = ts_realloc(self->included_ranges, size);
memcpy(self->included_ranges, ranges, size);
self->included_range_count = count;
ts_lexer_goto(self, self->current_position);
return true;
}

TSRange *ts_lexer_included_ranges(const Lexer *self, uint32_t *count) {
*count = self->included_range_count;
return self->included_ranges;
}

#undef LOG
