#include "tree_sitter/api.h"
#include "./alloc.h"
#include "./array.h"
#include "./bits.h"
#include "./language.h"
#include "./point.h"
#include "./tree_cursor.h"
#include "./unicode.h"
#include <wctype.h>





typedef struct {
const char *input;
const char *end;
int32_t next;
uint8_t next_size;
} Stream;

#define MAX_STEP_CAPTURE_COUNT 4
















typedef struct {
TSSymbol symbol;
TSFieldId field;
uint16_t capture_ids[MAX_STEP_CAPTURE_COUNT];
uint16_t depth: 13;
bool contains_captures: 1;
bool is_immediate: 1;
bool is_last: 1;
} QueryStep;







typedef struct {
uint32_t offset;
uint32_t length;
} Slice;




typedef struct {
Array(char) characters;
Array(Slice) slices;
} SymbolTable;









typedef struct {
uint16_t step_index;
uint16_t pattern_index;
} PatternEntry;







typedef struct {
uint16_t start_depth;
uint16_t pattern_index;
uint16_t step_index;
uint16_t capture_count;
uint16_t capture_list_id;
uint16_t consumed_capture_count;
uint32_t id;
} QueryState;







typedef struct {
Array(TSQueryCapture) list;
uint32_t usage_map;
} CaptureListPool;






struct TSQuery {
SymbolTable captures;
SymbolTable predicate_values;
Array(QueryStep) steps;
Array(PatternEntry) pattern_map;
Array(TSQueryPredicateStep) predicate_steps;
Array(Slice) predicates_by_pattern;
Array(uint32_t) start_bytes_by_pattern;
const TSLanguage *language;
uint16_t max_capture_count;
uint16_t wildcard_root_pattern_count;
TSSymbol *symbol_map;
};




struct TSQueryCursor {
const TSQuery *query;
TSTreeCursor cursor;
Array(QueryState) states;
Array(QueryState) finished_states;
CaptureListPool capture_list_pool;
uint32_t depth;
uint32_t start_byte;
uint32_t end_byte;
uint32_t next_state_id;
TSPoint start_point;
TSPoint end_point;
bool ascending;
};

static const TSQueryError PARENT_DONE = -1;
static const uint8_t PATTERN_DONE_MARKER = UINT8_MAX;
static const uint16_t NONE = UINT16_MAX;
static const TSSymbol WILDCARD_SYMBOL = 0;
static const TSSymbol NAMED_WILDCARD_SYMBOL = UINT16_MAX - 1;
static const uint16_t MAX_STATE_COUNT = 32;


#define LOG(...)






static bool stream_advance(Stream *self) {
self->input += self->next_size;
if (self->input < self->end) {
uint32_t size = ts_decode_utf8(
(const uint8_t *)self->input,
self->end - self->input,
&self->next
);
if (size > 0) {
self->next_size = size;
return true;
}
} else {
self->next_size = 0;
self->next = '\0';
}
return false;
}



static void stream_reset(Stream *self, const char *input) {
self->input = input;
self->next_size = 0;
stream_advance(self);
}

static Stream stream_new(const char *string, uint32_t length) {
Stream self = {
.next = 0,
.input = string,
.end = string + length,
};
stream_advance(&self);
return self;
}

static void stream_skip_whitespace(Stream *stream) {
for (;;) {
if (iswspace(stream->next)) {
stream_advance(stream);
} else if (stream->next == ';') {

stream_advance(stream);
while (stream->next && stream->next != '\n') {
if (!stream_advance(stream)) break;
}
} else {
break;
}
}
}

static bool stream_is_ident_start(Stream *stream) {
return iswalnum(stream->next) || stream->next == '_' || stream->next == '-';
}

static void stream_scan_identifier(Stream *stream) {
do {
stream_advance(stream);
} while (
iswalnum(stream->next) ||
stream->next == '_' ||
stream->next == '-' ||
stream->next == '.' ||
stream->next == '?' ||
stream->next == '!'
);
}





static CaptureListPool capture_list_pool_new(void) {
return (CaptureListPool) {
.list = array_new(),
.usage_map = UINT32_MAX,
};
}

static void capture_list_pool_reset(CaptureListPool *self, uint16_t list_size) {
self->usage_map = UINT32_MAX;
uint32_t total_size = MAX_STATE_COUNT * list_size;
array_reserve(&self->list, total_size);
self->list.size = total_size;
}

static void capture_list_pool_delete(CaptureListPool *self) {
array_delete(&self->list);
}

static TSQueryCapture *capture_list_pool_get(CaptureListPool *self, uint16_t id) {
return &self->list.contents[id * (self->list.size / MAX_STATE_COUNT)];
}

static bool capture_list_pool_is_empty(const CaptureListPool *self) {
return self->usage_map == 0;
}

static uint16_t capture_list_pool_acquire(CaptureListPool *self) {




uint16_t id = count_leading_zeros(self->usage_map);
if (id == 32) return NONE;
self->usage_map &= ~bitmask_for_index(id);
return id;
}

static void capture_list_pool_release(CaptureListPool *self, uint16_t id) {
self->usage_map |= bitmask_for_index(id);
}





static SymbolTable symbol_table_new(void) {
return (SymbolTable) {
.characters = array_new(),
.slices = array_new(),
};
}

static void symbol_table_delete(SymbolTable *self) {
array_delete(&self->characters);
array_delete(&self->slices);
}

static int symbol_table_id_for_name(
const SymbolTable *self,
const char *name,
uint32_t length
) {
for (unsigned i = 0; i < self->slices.size; i++) {
Slice slice = self->slices.contents[i];
if (
slice.length == length &&
!strncmp(&self->characters.contents[slice.offset], name, length)
) return i;
}
return -1;
}

static const char *symbol_table_name_for_id(
const SymbolTable *self,
uint16_t id,
uint32_t *length
) {
Slice slice = self->slices.contents[id];
*length = slice.length;
return &self->characters.contents[slice.offset];
}

static uint16_t symbol_table_insert_name(
SymbolTable *self,
const char *name,
uint32_t length
) {
int id = symbol_table_id_for_name(self, name, length);
if (id >= 0) return (uint16_t)id;
Slice slice = {
.offset = self->characters.size,
.length = length,
};
array_grow_by(&self->characters, length + 1);
memcpy(&self->characters.contents[slice.offset], name, length);
self->characters.contents[self->characters.size - 1] = 0;
array_push(&self->slices, slice);
return self->slices.size - 1;
}





static QueryStep query_step__new(
TSSymbol symbol,
uint16_t depth,
bool is_immediate
) {
return (QueryStep) {
.symbol = symbol,
.depth = depth,
.field = 0,
.capture_ids = {NONE, NONE, NONE, NONE},
.contains_captures = false,
.is_immediate = is_immediate,
};
}

static void query_step__add_capture(QueryStep *self, uint16_t capture_id) {
for (unsigned i = 0; i < MAX_STEP_CAPTURE_COUNT; i++) {
if (self->capture_ids[i] == NONE) {
self->capture_ids[i] = capture_id;
break;
}
}
}

static void query_step__remove_capture(QueryStep *self, uint16_t capture_id) {
for (unsigned i = 0; i < MAX_STEP_CAPTURE_COUNT; i++) {
if (self->capture_ids[i] == capture_id) {
self->capture_ids[i] = NONE;
while (i + 1 < MAX_STEP_CAPTURE_COUNT) {
if (self->capture_ids[i + 1] == NONE) break;
self->capture_ids[i] = self->capture_ids[i + 1];
self->capture_ids[i + 1] = NONE;
i++;
}
break;
}
}
}



















static inline bool ts_query__pattern_map_search(
const TSQuery *self,
TSSymbol needle,
uint32_t *result
) {
uint32_t base_index = self->wildcard_root_pattern_count;
uint32_t size = self->pattern_map.size - base_index;
if (size == 0) {
*result = base_index;
return false;
}
while (size > 1) {
uint32_t half_size = size / 2;
uint32_t mid_index = base_index + half_size;
TSSymbol mid_symbol = self->steps.contents[
self->pattern_map.contents[mid_index].step_index
].symbol;
if (needle > mid_symbol) base_index = mid_index;
size -= half_size;
}

TSSymbol symbol = self->steps.contents[
self->pattern_map.contents[base_index].step_index
].symbol;

if (needle > symbol) {
base_index++;
if (base_index < self->pattern_map.size) {
symbol = self->steps.contents[
self->pattern_map.contents[base_index].step_index
].symbol;
}
}

*result = base_index;
return needle == symbol;
}



static inline void ts_query__pattern_map_insert(
TSQuery *self,
TSSymbol symbol,
uint32_t start_step_index
) {
uint32_t index;
ts_query__pattern_map_search(self, symbol, &index);
array_insert(&self->pattern_map, index, ((PatternEntry) {
.step_index = start_step_index,
.pattern_index = self->pattern_map.size,
}));
}

static void ts_query__finalize_steps(TSQuery *self) {
for (unsigned i = 0; i < self->steps.size; i++) {
QueryStep *step = &self->steps.contents[i];
uint32_t depth = step->depth;
if (step->capture_ids[0] != NONE) {
step->contains_captures = true;
} else {
step->contains_captures = false;
for (unsigned j = i + 1; j < self->steps.size; j++) {
QueryStep *s = &self->steps.contents[j];
if (s->depth == PATTERN_DONE_MARKER || s->depth <= depth) break;
if (s->capture_ids[0] != NONE) step->contains_captures = true;
}
}
}
}







static TSQueryError ts_query__parse_predicate(
TSQuery *self,
Stream *stream
) {
if (stream->next == ')') return PARENT_DONE;
if (stream->next != '(') return TSQueryErrorSyntax;
stream_advance(stream);
stream_skip_whitespace(stream);

unsigned step_count = 0;
for (;;) {
if (stream->next == ')') {
stream_advance(stream);
stream_skip_whitespace(stream);
array_back(&self->predicates_by_pattern)->length++;
array_push(&self->predicate_steps, ((TSQueryPredicateStep) {
.type = TSQueryPredicateStepTypeDone,
.value_id = 0,
}));
break;
}


else if (stream->next == '@') {
stream_advance(stream);


if (!stream_is_ident_start(stream)) return TSQueryErrorSyntax;
const char *capture_name = stream->input;
stream_scan_identifier(stream);
uint32_t length = stream->input - capture_name;


int capture_id = symbol_table_id_for_name(
&self->captures,
capture_name,
length
);
if (capture_id == -1) {
stream_reset(stream, capture_name);
return TSQueryErrorCapture;
}

array_back(&self->predicates_by_pattern)->length++;
array_push(&self->predicate_steps, ((TSQueryPredicateStep) {
.type = TSQueryPredicateStepTypeCapture,
.value_id = capture_id,
}));
}


else if (stream->next == '"') {
stream_advance(stream);


const char *string_content = stream->input;
while (stream->next != '"') {
if (stream->next == '\n' || !stream_advance(stream)) {
stream_reset(stream, string_content - 1);
return TSQueryErrorSyntax;
}
}
uint32_t length = stream->input - string_content;


uint16_t id = symbol_table_insert_name(
&self->predicate_values,
string_content,
length
);
array_back(&self->predicates_by_pattern)->length++;
array_push(&self->predicate_steps, ((TSQueryPredicateStep) {
.type = TSQueryPredicateStepTypeString,
.value_id = id,
}));

if (stream->next != '"') return TSQueryErrorSyntax;
stream_advance(stream);
}


else if (stream_is_ident_start(stream)) {
const char *symbol_start = stream->input;
stream_scan_identifier(stream);
uint32_t length = stream->input - symbol_start;
uint16_t id = symbol_table_insert_name(
&self->predicate_values,
symbol_start,
length
);
array_back(&self->predicates_by_pattern)->length++;
array_push(&self->predicate_steps, ((TSQueryPredicateStep) {
.type = TSQueryPredicateStepTypeString,
.value_id = id,
}));
}

else {
return TSQueryErrorSyntax;
}

step_count++;
stream_skip_whitespace(stream);
}

return 0;
}




static TSQueryError ts_query__parse_pattern(
TSQuery *self,
Stream *stream,
uint32_t depth,
uint32_t *capture_count,
bool is_immediate
) {
uint16_t starting_step_index = self->steps.size;

if (stream->next == 0) return TSQueryErrorSyntax;


if (stream->next == ')') {
return PARENT_DONE;
}


else if (stream->next == '(') {
stream_advance(stream);
stream_skip_whitespace(stream);



if (stream->next == '(' && depth == 0) {
TSQueryError e = ts_query__parse_pattern(self, stream, 0, capture_count, is_immediate);
if (e) return e;


stream_skip_whitespace(stream);
for (;;) {
TSQueryError e = ts_query__parse_predicate(self, stream);
if (e == PARENT_DONE) {
stream_advance(stream);
stream_skip_whitespace(stream);
return 0;
} else if (e) {
return e;
}
}
}

TSSymbol symbol;


if (stream->next == '*') {
symbol = NAMED_WILDCARD_SYMBOL;
stream_advance(stream);
}


else if (stream_is_ident_start(stream)) {
const char *node_name = stream->input;
stream_scan_identifier(stream);
uint32_t length = stream->input - node_name;
symbol = ts_language_symbol_for_name(
self->language,
node_name,
length,
true
);
if (!symbol) {
stream_reset(stream, node_name);
return TSQueryErrorNodeType;
}
} else {
return TSQueryErrorSyntax;
}


array_push(&self->steps, query_step__new(symbol, depth, is_immediate));


stream_skip_whitespace(stream);
bool child_is_immediate = false;
uint16_t child_start_step_index = self->steps.size;
for (;;) {
if (stream->next == '.') {
child_is_immediate = true;
stream_advance(stream);
stream_skip_whitespace(stream);
}

TSQueryError e = ts_query__parse_pattern(
self,
stream,
depth + 1,
capture_count,
child_is_immediate
);
if (e == PARENT_DONE) {
if (child_is_immediate) {
self->steps.contents[child_start_step_index].is_last = true;
}
stream_advance(stream);
break;
} else if (e) {
return e;
}

child_is_immediate = false;
}
}


else if (stream->next == '"') {
stream_advance(stream);


const char *string_content = stream->input;
while (stream->next != '"') {
if (!stream_advance(stream)) {
stream_reset(stream, string_content - 1);
return TSQueryErrorSyntax;
}
}
uint32_t length = stream->input - string_content;


TSSymbol symbol = ts_language_symbol_for_name(
self->language,
string_content,
length,
false
);
if (!symbol) {
stream_reset(stream, string_content);
return TSQueryErrorNodeType;
}
array_push(&self->steps, query_step__new(symbol, depth, is_immediate));

if (stream->next != '"') return TSQueryErrorSyntax;
stream_advance(stream);
}


else if (stream_is_ident_start(stream)) {

const char *field_name = stream->input;
stream_scan_identifier(stream);
uint32_t length = stream->input - field_name;
stream_skip_whitespace(stream);

if (stream->next != ':') {
stream_reset(stream, field_name);
return TSQueryErrorSyntax;
}
stream_advance(stream);
stream_skip_whitespace(stream);


uint32_t step_index = self->steps.size;
TSQueryError e = ts_query__parse_pattern(
self,
stream,
depth,
capture_count,
is_immediate
);
if (e == PARENT_DONE) return TSQueryErrorSyntax;
if (e) return e;


TSFieldId field_id = ts_language_field_id_for_name(
self->language,
field_name,
length
);
if (!field_id) {
stream->input = field_name;
return TSQueryErrorField;
}
self->steps.contents[step_index].field = field_id;
}


else if (stream->next == '*') {
stream_advance(stream);
stream_skip_whitespace(stream);


array_push(&self->steps, query_step__new(WILDCARD_SYMBOL, depth, is_immediate));
}

else {
return TSQueryErrorSyntax;
}

stream_skip_whitespace(stream);


while (stream->next == '@') {
stream_advance(stream);


if (!stream_is_ident_start(stream)) return TSQueryErrorSyntax;
const char *capture_name = stream->input;
stream_scan_identifier(stream);
uint32_t length = stream->input - capture_name;


uint16_t capture_id = symbol_table_insert_name(
&self->captures,
capture_name,
length
);
QueryStep *step = &self->steps.contents[starting_step_index];
query_step__add_capture(step, capture_id);
(*capture_count)++;

stream_skip_whitespace(stream);
}

return 0;
}

TSQuery *ts_query_new(
const TSLanguage *language,
const char *source,
uint32_t source_len,
uint32_t *error_offset,
TSQueryError *error_type
) {
TSSymbol *symbol_map;
if (ts_language_version(language) >= TREE_SITTER_LANGUAGE_VERSION_WITH_SYMBOL_DEDUPING) {
symbol_map = NULL;
} else {




uint32_t symbol_count = ts_language_symbol_count(language);
symbol_map = ts_malloc(sizeof(TSSymbol) * symbol_count);
for (unsigned i = 0; i < symbol_count; i++) {
const char *name = ts_language_symbol_name(language, i);
const TSSymbolType symbol_type = ts_language_symbol_type(language, i);

symbol_map[i] = i;

for (unsigned j = 0; j < i; j++) {
if (ts_language_symbol_type(language, j) == symbol_type) {
if (!strcmp(name, ts_language_symbol_name(language, j))) {
symbol_map[i] = j;
break;
}
}
}
}
}

TSQuery *self = ts_malloc(sizeof(TSQuery));
*self = (TSQuery) {
.steps = array_new(),
.pattern_map = array_new(),
.captures = symbol_table_new(),
.predicate_values = symbol_table_new(),
.predicate_steps = array_new(),
.predicates_by_pattern = array_new(),
.symbol_map = symbol_map,
.wildcard_root_pattern_count = 0,
.max_capture_count = 0,
.language = language,
};


Stream stream = stream_new(source, source_len);
stream_skip_whitespace(&stream);
uint32_t start_step_index;
while (stream.input < stream.end) {
start_step_index = self->steps.size;
uint32_t capture_count = 0;
array_push(&self->start_bytes_by_pattern, stream.input - source);
array_push(&self->predicates_by_pattern, ((Slice) {
.offset = self->predicate_steps.size,
.length = 0,
}));
*error_type = ts_query__parse_pattern(self, &stream, 0, &capture_count, false);
array_push(&self->steps, query_step__new(0, PATTERN_DONE_MARKER, false));



if (*error_type) {
*error_offset = stream.input - source;
ts_query_delete(self);
return NULL;
}


ts_query__pattern_map_insert(
self,
self->steps.contents[start_step_index].symbol,
start_step_index
);
if (self->steps.contents[start_step_index].symbol == WILDCARD_SYMBOL) {
self->wildcard_root_pattern_count++;
}




if (capture_count > self->max_capture_count) {
self->max_capture_count = capture_count;
}
}

ts_query__finalize_steps(self);
return self;
}

void ts_query_delete(TSQuery *self) {
if (self) {
array_delete(&self->steps);
array_delete(&self->pattern_map);
array_delete(&self->predicate_steps);
array_delete(&self->predicates_by_pattern);
array_delete(&self->start_bytes_by_pattern);
symbol_table_delete(&self->captures);
symbol_table_delete(&self->predicate_values);
ts_free(self->symbol_map);
ts_free(self);
}
}

uint32_t ts_query_pattern_count(const TSQuery *self) {
return self->predicates_by_pattern.size;
}

uint32_t ts_query_capture_count(const TSQuery *self) {
return self->captures.slices.size;
}

uint32_t ts_query_string_count(const TSQuery *self) {
return self->predicate_values.slices.size;
}

const char *ts_query_capture_name_for_id(
const TSQuery *self,
uint32_t index,
uint32_t *length
) {
return symbol_table_name_for_id(&self->captures, index, length);
}

const char *ts_query_string_value_for_id(
const TSQuery *self,
uint32_t index,
uint32_t *length
) {
return symbol_table_name_for_id(&self->predicate_values, index, length);
}

const TSQueryPredicateStep *ts_query_predicates_for_pattern(
const TSQuery *self,
uint32_t pattern_index,
uint32_t *step_count
) {
Slice slice = self->predicates_by_pattern.contents[pattern_index];
*step_count = slice.length;
return &self->predicate_steps.contents[slice.offset];
}

uint32_t ts_query_start_byte_for_pattern(
const TSQuery *self,
uint32_t pattern_index
) {
return self->start_bytes_by_pattern.contents[pattern_index];
}

void ts_query_disable_capture(
TSQuery *self,
const char *name,
uint32_t length
) {


int id = symbol_table_id_for_name(&self->captures, name, length);
if (id != -1) {
for (unsigned i = 0; i < self->steps.size; i++) {
QueryStep *step = &self->steps.contents[i];
query_step__remove_capture(step, id);
}
ts_query__finalize_steps(self);
}
}

void ts_query_disable_pattern(
TSQuery *self,
uint32_t pattern_index
) {


for (unsigned i = 0; i < self->pattern_map.size; i++) {
PatternEntry *pattern = &self->pattern_map.contents[i];
if (pattern->pattern_index == pattern_index) {
array_erase(&self->pattern_map, i);
i--;
}
}
}





TSQueryCursor *ts_query_cursor_new() {
TSQueryCursor *self = ts_malloc(sizeof(TSQueryCursor));
*self = (TSQueryCursor) {
.ascending = false,
.states = array_new(),
.finished_states = array_new(),
.capture_list_pool = capture_list_pool_new(),
.start_byte = 0,
.end_byte = UINT32_MAX,
.start_point = {0, 0},
.end_point = POINT_MAX,
};
array_reserve(&self->states, MAX_STATE_COUNT);
array_reserve(&self->finished_states, MAX_STATE_COUNT);
return self;
}

void ts_query_cursor_delete(TSQueryCursor *self) {
array_delete(&self->states);
array_delete(&self->finished_states);
ts_tree_cursor_delete(&self->cursor);
capture_list_pool_delete(&self->capture_list_pool);
ts_free(self);
}

void ts_query_cursor_exec(
TSQueryCursor *self,
const TSQuery *query,
TSNode node
) {
array_clear(&self->states);
array_clear(&self->finished_states);
ts_tree_cursor_reset(&self->cursor, node);
capture_list_pool_reset(&self->capture_list_pool, query->max_capture_count);
self->next_state_id = 0;
self->depth = 0;
self->ascending = false;
self->query = query;
}

void ts_query_cursor_set_byte_range(
TSQueryCursor *self,
uint32_t start_byte,
uint32_t end_byte
) {
if (end_byte == 0) {
start_byte = 0;
end_byte = UINT32_MAX;
}
self->start_byte = start_byte;
self->end_byte = end_byte;
}

void ts_query_cursor_set_point_range(
TSQueryCursor *self,
TSPoint start_point,
TSPoint end_point
) {
if (end_point.row == 0 && end_point.column == 0) {
start_point = POINT_ZERO;
end_point = POINT_MAX;
}
self->start_point = start_point;
self->end_point = end_point;
}



static bool ts_query_cursor__first_in_progress_capture(
TSQueryCursor *self,
uint32_t *state_index,
uint32_t *byte_offset,
uint32_t *pattern_index
) {
bool result = false;
for (unsigned i = 0; i < self->states.size; i++) {
const QueryState *state = &self->states.contents[i];
if (state->capture_count > 0) {
const TSQueryCapture *captures = capture_list_pool_get(
&self->capture_list_pool,
state->capture_list_id
);
uint32_t capture_byte = ts_node_start_byte(captures[0].node);
if (
!result ||
capture_byte < *byte_offset ||
(
capture_byte == *byte_offset &&
state->pattern_index < *pattern_index
)
) {
result = true;
*state_index = i;
*byte_offset = capture_byte;
*pattern_index = state->pattern_index;
}
}
}
return result;
}

static bool ts_query__cursor_add_state(
TSQueryCursor *self,
const PatternEntry *pattern
) {
uint32_t list_id = capture_list_pool_acquire(&self->capture_list_pool);




if (list_id == NONE) {
uint32_t state_index, byte_offset, pattern_index;
if (ts_query_cursor__first_in_progress_capture(
self,
&state_index,
&byte_offset,
&pattern_index
)) {
LOG(
" abandon state. index:%u, pattern:%u, offset:%u.\n",
state_index, pattern_index, byte_offset
);
list_id = self->states.contents[state_index].capture_list_id;
array_erase(&self->states, state_index);
} else {
LOG(" too many finished states.\n");
return false;
}
}

LOG(" start state. pattern:%u\n", pattern->pattern_index);
array_push(&self->states, ((QueryState) {
.capture_list_id = list_id,
.step_index = pattern->step_index,
.pattern_index = pattern->pattern_index,
.start_depth = self->depth,
.capture_count = 0,
.consumed_capture_count = 0,
}));
return true;
}

static QueryState *ts_query__cursor_copy_state(
TSQueryCursor *self,
const QueryState *state
) {
uint32_t new_list_id = capture_list_pool_acquire(&self->capture_list_pool);
if (new_list_id == NONE) return NULL;
array_push(&self->states, *state);
QueryState *new_state = array_back(&self->states);
new_state->capture_list_id = new_list_id;
TSQueryCapture *old_captures = capture_list_pool_get(
&self->capture_list_pool,
state->capture_list_id
);
TSQueryCapture *new_captures = capture_list_pool_get(
&self->capture_list_pool,
new_list_id
);
memcpy(new_captures, old_captures, state->capture_count * sizeof(TSQueryCapture));
return new_state;
}





static inline bool ts_query_cursor__advance(TSQueryCursor *self) {
do {
if (self->ascending) {
LOG("leave node. type:%s\n", ts_node_type(ts_tree_cursor_current_node(&self->cursor)));



uint32_t deleted_count = 0;
for (unsigned i = 0, n = self->states.size; i < n; i++) {
QueryState *state = &self->states.contents[i];
QueryStep *step = &self->query->steps.contents[state->step_index];

if ((uint32_t)state->start_depth + (uint32_t)step->depth > self->depth) {
LOG(
" failed to match. pattern:%u, step:%u\n",
state->pattern_index,
state->step_index
);

capture_list_pool_release(
&self->capture_list_pool,
state->capture_list_id
);
deleted_count++;
} else if (deleted_count > 0) {
self->states.contents[i - deleted_count] = *state;
}
}

self->states.size -= deleted_count;

if (ts_tree_cursor_goto_next_sibling(&self->cursor)) {
self->ascending = false;
} else if (ts_tree_cursor_goto_parent(&self->cursor)) {
self->depth--;
} else {
return self->finished_states.size > 0;
}
} else {
bool has_later_siblings;
bool can_have_later_siblings_with_this_field;
TSFieldId field_id = ts_tree_cursor_current_status(
&self->cursor,
&has_later_siblings,
&can_have_later_siblings_with_this_field
);
TSNode node = ts_tree_cursor_current_node(&self->cursor);
TSSymbol symbol = ts_node_symbol(node);
bool is_named = ts_node_is_named(node);
if (symbol != ts_builtin_sym_error && self->query->symbol_map) {
symbol = self->query->symbol_map[symbol];
}



if (
ts_node_end_byte(node) <= self->start_byte ||
point_lte(ts_node_end_point(node), self->start_point)
) {
if (!ts_tree_cursor_goto_next_sibling(&self->cursor)) {
self->ascending = true;
}
continue;
}


if (
self->end_byte <= ts_node_start_byte(node) ||
point_lte(self->end_point, ts_node_start_point(node))
) return false;

LOG(
"enter node. "
"type:%s, field:%s, row:%u state_count:%u, "
"finished_state_count:%u, has_later_siblings:%d, "
"can_have_later_siblings_with_this_field:%d\n",
ts_node_type(node),
ts_language_field_name_for_id(self->query->language, field_id),
ts_node_start_point(node).row,
self->states.size,
self->finished_states.size,
has_later_siblings,
can_have_later_siblings_with_this_field
);


for (unsigned i = 0; i < self->query->wildcard_root_pattern_count; i++) {
PatternEntry *pattern = &self->query->pattern_map.contents[i];
QueryStep *step = &self->query->steps.contents[pattern->step_index];



if (step->field && field_id != step->field) continue;
if (!ts_query__cursor_add_state(self, pattern)) break;
}


unsigned i;
if (ts_query__pattern_map_search(self->query, symbol, &i)) {
PatternEntry *pattern = &self->query->pattern_map.contents[i];
QueryStep *step = &self->query->steps.contents[pattern->step_index];
do {


if (step->field && field_id != step->field) continue;
if (!ts_query__cursor_add_state(self, pattern)) break;


i++;
if (i == self->query->pattern_map.size) break;
pattern = &self->query->pattern_map.contents[i];
step = &self->query->steps.contents[pattern->step_index];
} while (step->symbol == symbol);
}


for (unsigned i = 0, n = self->states.size; i < n; i++) {
QueryState *state = &self->states.contents[i];
QueryStep *step = &self->query->steps.contents[state->step_index];



if ((uint32_t)state->start_depth + (uint32_t)step->depth != self->depth) continue;




bool node_does_match =
step->symbol == symbol ||
step->symbol == WILDCARD_SYMBOL ||
(step->symbol == NAMED_WILDCARD_SYMBOL && is_named);
bool later_sibling_can_match = has_later_siblings;
if (step->is_immediate && is_named) {
later_sibling_can_match = false;
}
if (step->is_last && has_later_siblings) {
node_does_match = false;
}
if (step->field) {
if (step->field == field_id) {
if (!can_have_later_siblings_with_this_field) {
later_sibling_can_match = false;
}
} else {
node_does_match = false;
}
}

if (!node_does_match) {
if (!later_sibling_can_match) {
LOG(
" discard state. pattern:%u, step:%u\n",
state->pattern_index,
state->step_index
);
capture_list_pool_release(
&self->capture_list_pool,
state->capture_list_id
);
array_erase(&self->states, i);
i--;
n--;
}
continue;
}








QueryState *next_state = state;
if (
step->depth > 0 &&
step->contains_captures &&
later_sibling_can_match
) {
QueryState *copy = ts_query__cursor_copy_state(self, state);
if (copy) {
LOG(
" split state. pattern:%u, step:%u\n",
copy->pattern_index,
copy->step_index
);
next_state = copy;
} else {
LOG(" canot split state.\n");
}
}

LOG(
" advance state. pattern:%u, step:%u\n",
next_state->pattern_index,
next_state->step_index
);



for (unsigned j = 0; j < MAX_STEP_CAPTURE_COUNT; j++) {
uint16_t capture_id = step->capture_ids[j];
if (step->capture_ids[j] == NONE) break;
LOG(
" capture node. pattern:%u, capture_id:%u\n",
next_state->pattern_index,
capture_id
);
TSQueryCapture *capture_list = capture_list_pool_get(
&self->capture_list_pool,
next_state->capture_list_id
);
capture_list[next_state->capture_count++] = (TSQueryCapture) {
node,
capture_id
};
}



next_state->step_index++;
QueryStep *next_step = step + 1;
if (next_step->depth == PATTERN_DONE_MARKER) {
LOG(" finish pattern %u\n", next_state->pattern_index);

next_state->id = self->next_state_id++;
array_push(&self->finished_states, *next_state);
if (next_state == state) {
array_erase(&self->states, i);
i--;
n--;
} else {
self->states.size--;
}
}
}


if (ts_tree_cursor_goto_first_child(&self->cursor)) {
self->depth++;
} else {
self->ascending = true;
}
}
} while (self->finished_states.size == 0);

return true;
}

bool ts_query_cursor_next_match(
TSQueryCursor *self,
TSQueryMatch *match
) {
if (self->finished_states.size == 0) {
if (!ts_query_cursor__advance(self)) {
return false;
}
}

QueryState *state = &self->finished_states.contents[0];
match->id = state->id;
match->pattern_index = state->pattern_index;
match->capture_count = state->capture_count;
match->captures = capture_list_pool_get(
&self->capture_list_pool,
state->capture_list_id
);
capture_list_pool_release(&self->capture_list_pool, state->capture_list_id);
array_erase(&self->finished_states, 0);
return true;
}

void ts_query_cursor_remove_match(
TSQueryCursor *self,
uint32_t match_id
) {
for (unsigned i = 0; i < self->finished_states.size; i++) {
const QueryState *state = &self->finished_states.contents[i];
if (state->id == match_id) {
capture_list_pool_release(
&self->capture_list_pool,
state->capture_list_id
);
array_erase(&self->finished_states, i);
return;
}
}
}

bool ts_query_cursor_next_capture(
TSQueryCursor *self,
TSQueryMatch *match,
uint32_t *capture_index
) {
for (;;) {




if (self->finished_states.size > 0) {



uint32_t first_unfinished_capture_byte = UINT32_MAX;
uint32_t first_unfinished_pattern_index = UINT32_MAX;
uint32_t first_unfinished_state_index;
ts_query_cursor__first_in_progress_capture(
self,
&first_unfinished_state_index,
&first_unfinished_capture_byte,
&first_unfinished_pattern_index
);


int first_finished_state_index = -1;
uint32_t first_finished_capture_byte = first_unfinished_capture_byte;
uint32_t first_finished_pattern_index = first_unfinished_pattern_index;
for (unsigned i = 0; i < self->finished_states.size; i++) {
const QueryState *state = &self->finished_states.contents[i];
if (state->capture_count > state->consumed_capture_count) {
const TSQueryCapture *captures = capture_list_pool_get(
&self->capture_list_pool,
state->capture_list_id
);
uint32_t capture_byte = ts_node_start_byte(
captures[state->consumed_capture_count].node
);
if (
capture_byte < first_finished_capture_byte ||
(
capture_byte == first_finished_capture_byte &&
state->pattern_index < first_finished_pattern_index
)
) {
first_finished_state_index = i;
first_finished_capture_byte = capture_byte;
first_finished_pattern_index = state->pattern_index;
}
} else {
capture_list_pool_release(
&self->capture_list_pool,
state->capture_list_id
);
array_erase(&self->finished_states, i);
i--;
}
}




if (first_finished_state_index != -1) {
QueryState *state = &self->finished_states.contents[
first_finished_state_index
];
match->id = state->id;
match->pattern_index = state->pattern_index;
match->capture_count = state->capture_count;
match->captures = capture_list_pool_get(
&self->capture_list_pool,
state->capture_list_id
);
*capture_index = state->consumed_capture_count;
state->consumed_capture_count++;
return true;
}

if (capture_list_pool_is_empty(&self->capture_list_pool)) {
LOG(
" abandon state. index:%u, pattern:%u, offset:%u.\n",
first_unfinished_state_index,
first_unfinished_pattern_index,
first_unfinished_capture_byte
);
capture_list_pool_release(
&self->capture_list_pool,
self->states.contents[first_unfinished_state_index].capture_list_id
);
array_erase(&self->states, first_unfinished_state_index);
}
}



if (!ts_query_cursor__advance(self)) return false;
}
}

#undef LOG
