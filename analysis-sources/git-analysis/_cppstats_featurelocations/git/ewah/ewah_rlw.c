

















#include "git-compat-util.h"
#include "ewok.h"
#include "ewok_rlw.h"

static inline int next_word(struct rlw_iterator *it)
{
if (it->pointer >= it->size)
return 0;

it->rlw.word = &it->buffer[it->pointer];
it->pointer += rlw_get_literal_words(it->rlw.word) + 1;

it->rlw.literal_words = rlw_get_literal_words(it->rlw.word);
it->rlw.running_len = rlw_get_running_len(it->rlw.word);
it->rlw.running_bit = rlw_get_run_bit(it->rlw.word);
it->rlw.literal_word_offset = 0;

return 1;
}

void rlwit_init(struct rlw_iterator *it, struct ewah_bitmap *from_ewah)
{
it->buffer = from_ewah->buffer;
it->size = from_ewah->buffer_size;
it->pointer = 0;

next_word(it);

it->literal_word_start = rlwit_literal_words(it) +
it->rlw.literal_word_offset;
}

void rlwit_discard_first_words(struct rlw_iterator *it, size_t x)
{
while (x > 0) {
size_t discard;

if (it->rlw.running_len > x) {
it->rlw.running_len -= x;
return;
}

x -= it->rlw.running_len;
it->rlw.running_len = 0;

discard = (x > it->rlw.literal_words) ? it->rlw.literal_words : x;

it->literal_word_start += discard;
it->rlw.literal_words -= discard;
x -= discard;

if (x > 0 || rlwit_word_size(it) == 0) {
if (!next_word(it))
break;

it->literal_word_start =
rlwit_literal_words(it) + it->rlw.literal_word_offset;
}
}
}

size_t rlwit_discharge(
struct rlw_iterator *it, struct ewah_bitmap *out, size_t max, int negate)
{
size_t index = 0;

while (index < max && rlwit_word_size(it) > 0) {
size_t pd, pl = it->rlw.running_len;

if (index + pl > max)
pl = max - index;

ewah_add_empty_words(out, it->rlw.running_bit ^ negate, pl);
index += pl;

pd = it->rlw.literal_words;
if (pd + index > max)
pd = max - index;

ewah_add_dirty_words(out,
it->buffer + it->literal_word_start, pd, negate);

rlwit_discard_first_words(it, pd + pl);
index += pd;
}

return index;
}
