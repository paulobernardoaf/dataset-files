












#include "lzma2_encoder.h"
#include "lzma_encoder_private.h"
#include "fastpos.h"






static inline void
literal_matched(lzma_range_encoder *rc, probability *subcoder,
uint32_t match_byte, uint32_t symbol)
{
uint32_t offset = 0x100;
symbol += UINT32_C(1) << 8;

do {
match_byte <<= 1;
const uint32_t match_bit = match_byte & offset;
const uint32_t subcoder_index
= offset + match_bit + (symbol >> 8);
const uint32_t bit = (symbol >> 7) & 1;
rc_bit(rc, &subcoder[subcoder_index], bit);

symbol <<= 1;
offset &= ~(match_byte ^ symbol);

} while (symbol < (UINT32_C(1) << 16));
}


static inline void
literal(lzma_coder *coder, lzma_mf *mf, uint32_t position)
{

const uint8_t cur_byte = mf->buffer[
mf->read_pos - mf->read_ahead];
probability *subcoder = literal_subcoder(coder->literal,
coder->literal_context_bits, coder->literal_pos_mask,
position, mf->buffer[mf->read_pos - mf->read_ahead - 1]);

if (is_literal_state(coder->state)) {


rc_bittree(&coder->rc, subcoder, 8, cur_byte);
} else {



const uint8_t match_byte = mf->buffer[
mf->read_pos - coder->reps[0] - 1
- mf->read_ahead];
literal_matched(&coder->rc, subcoder, match_byte, cur_byte);
}

update_literal(coder->state);
}






static void
length_update_prices(lzma_length_encoder *lc, const uint32_t pos_state)
{
const uint32_t table_size = lc->table_size;
lc->counters[pos_state] = table_size;

const uint32_t a0 = rc_bit_0_price(lc->choice);
const uint32_t a1 = rc_bit_1_price(lc->choice);
const uint32_t b0 = a1 + rc_bit_0_price(lc->choice2);
const uint32_t b1 = a1 + rc_bit_1_price(lc->choice2);
uint32_t *const prices = lc->prices[pos_state];

uint32_t i;
for (i = 0; i < table_size && i < LEN_LOW_SYMBOLS; ++i)
prices[i] = a0 + rc_bittree_price(lc->low[pos_state],
LEN_LOW_BITS, i);

for (; i < table_size && i < LEN_LOW_SYMBOLS + LEN_MID_SYMBOLS; ++i)
prices[i] = b0 + rc_bittree_price(lc->mid[pos_state],
LEN_MID_BITS, i - LEN_LOW_SYMBOLS);

for (; i < table_size; ++i)
prices[i] = b1 + rc_bittree_price(lc->high, LEN_HIGH_BITS,
i - LEN_LOW_SYMBOLS - LEN_MID_SYMBOLS);

return;
}


static inline void
length(lzma_range_encoder *rc, lzma_length_encoder *lc,
const uint32_t pos_state, uint32_t len, const bool fast_mode)
{
assert(len <= MATCH_LEN_MAX);
len -= MATCH_LEN_MIN;

if (len < LEN_LOW_SYMBOLS) {
rc_bit(rc, &lc->choice, 0);
rc_bittree(rc, lc->low[pos_state], LEN_LOW_BITS, len);
} else {
rc_bit(rc, &lc->choice, 1);
len -= LEN_LOW_SYMBOLS;

if (len < LEN_MID_SYMBOLS) {
rc_bit(rc, &lc->choice2, 0);
rc_bittree(rc, lc->mid[pos_state], LEN_MID_BITS, len);
} else {
rc_bit(rc, &lc->choice2, 1);
len -= LEN_MID_SYMBOLS;
rc_bittree(rc, lc->high, LEN_HIGH_BITS, len);
}
}



if (!fast_mode)
if (--lc->counters[pos_state] == 0)
length_update_prices(lc, pos_state);
}






static inline void
match(lzma_coder *coder, const uint32_t pos_state,
const uint32_t distance, const uint32_t len)
{
update_match(coder->state);

length(&coder->rc, &coder->match_len_encoder, pos_state, len,
coder->fast_mode);

const uint32_t pos_slot = get_pos_slot(distance);
const uint32_t len_to_pos_state = get_len_to_pos_state(len);
rc_bittree(&coder->rc, coder->pos_slot[len_to_pos_state],
POS_SLOT_BITS, pos_slot);

if (pos_slot >= START_POS_MODEL_INDEX) {
const uint32_t footer_bits = (pos_slot >> 1) - 1;
const uint32_t base = (2 | (pos_slot & 1)) << footer_bits;
const uint32_t pos_reduced = distance - base;

if (pos_slot < END_POS_MODEL_INDEX) {


rc_bittree_reverse(&coder->rc,
coder->pos_special + base - pos_slot - 1,
footer_bits, pos_reduced);
} else {
rc_direct(&coder->rc, pos_reduced >> ALIGN_BITS,
footer_bits - ALIGN_BITS);
rc_bittree_reverse(
&coder->rc, coder->pos_align,
ALIGN_BITS, pos_reduced & ALIGN_MASK);
++coder->align_price_count;
}
}

coder->reps[3] = coder->reps[2];
coder->reps[2] = coder->reps[1];
coder->reps[1] = coder->reps[0];
coder->reps[0] = distance;
++coder->match_price_count;
}






static inline void
rep_match(lzma_coder *coder, const uint32_t pos_state,
const uint32_t rep, const uint32_t len)
{
if (rep == 0) {
rc_bit(&coder->rc, &coder->is_rep0[coder->state], 0);
rc_bit(&coder->rc,
&coder->is_rep0_long[coder->state][pos_state],
len != 1);
} else {
const uint32_t distance = coder->reps[rep];
rc_bit(&coder->rc, &coder->is_rep0[coder->state], 1);

if (rep == 1) {
rc_bit(&coder->rc, &coder->is_rep1[coder->state], 0);
} else {
rc_bit(&coder->rc, &coder->is_rep1[coder->state], 1);
rc_bit(&coder->rc, &coder->is_rep2[coder->state],
rep - 2);

if (rep == 3)
coder->reps[3] = coder->reps[2];

coder->reps[2] = coder->reps[1];
}

coder->reps[1] = coder->reps[0];
coder->reps[0] = distance;
}

if (len == 1) {
update_short_rep(coder->state);
} else {
length(&coder->rc, &coder->rep_len_encoder, pos_state, len,
coder->fast_mode);
update_long_rep(coder->state);
}
}






static void
encode_symbol(lzma_coder *coder, lzma_mf *mf,
uint32_t back, uint32_t len, uint32_t position)
{
const uint32_t pos_state = position & coder->pos_mask;

if (back == UINT32_MAX) {

assert(len == 1);
rc_bit(&coder->rc,
&coder->is_match[coder->state][pos_state], 0);
literal(coder, mf, position);
} else {

rc_bit(&coder->rc,
&coder->is_match[coder->state][pos_state], 1);

if (back < REP_DISTANCES) {


rc_bit(&coder->rc, &coder->is_rep[coder->state], 1);
rep_match(coder, pos_state, back, len);
} else {

rc_bit(&coder->rc, &coder->is_rep[coder->state], 0);
match(coder, pos_state, back - REP_DISTANCES, len);
}
}

assert(mf->read_ahead >= len);
mf->read_ahead -= len;
}


static bool
encode_init(lzma_coder *coder, lzma_mf *mf)
{
assert(mf_position(mf) == 0);

if (mf->read_pos == mf->read_limit) {
if (mf->action == LZMA_RUN)
return false; 


assert(mf->write_pos == mf->read_pos);
assert(mf->action == LZMA_FINISH);
} else {


mf_skip(mf, 1);
mf->read_ahead = 0;
rc_bit(&coder->rc, &coder->is_match[0][0], 0);
rc_bittree(&coder->rc, coder->literal[0], 8, mf->buffer[0]);
}


coder->is_initialized = true;

return true;
}


static void
encode_eopm(lzma_coder *coder, uint32_t position)
{
const uint32_t pos_state = position & coder->pos_mask;
rc_bit(&coder->rc, &coder->is_match[coder->state][pos_state], 1);
rc_bit(&coder->rc, &coder->is_rep[coder->state], 0);
match(coder, pos_state, UINT32_MAX, MATCH_LEN_MIN);
}





#define LOOP_INPUT_MAX (OPTS + 1)


extern lzma_ret
lzma_lzma_encode(lzma_coder *restrict coder, lzma_mf *restrict mf,
uint8_t *restrict out, size_t *restrict out_pos,
size_t out_size, uint32_t limit)
{

if (!coder->is_initialized && !encode_init(coder, mf))
return LZMA_OK;


uint32_t position = mf_position(mf);

while (true) {






if (rc_encode(&coder->rc, out, out_pos, out_size)) {
assert(limit == UINT32_MAX);
return LZMA_OK;
}




if (limit != UINT32_MAX
&& (mf->read_pos - mf->read_ahead >= limit
|| *out_pos + rc_pending(&coder->rc)
>= LZMA2_CHUNK_MAX
- LOOP_INPUT_MAX))
break;


if (mf->read_pos >= mf->read_limit) {
if (mf->action == LZMA_RUN)
return LZMA_OK;

if (mf->read_ahead == 0)
break;
}









uint32_t len;
uint32_t back;

if (coder->fast_mode)
lzma_lzma_optimum_fast(coder, mf, &back, &len);
else
lzma_lzma_optimum_normal(
coder, mf, &back, &len, position);

encode_symbol(coder, mf, back, len, position);

position += len;
}

if (!coder->is_flushed) {
coder->is_flushed = true;



if (limit == UINT32_MAX)
encode_eopm(coder, position);


rc_flush(&coder->rc);





if (rc_encode(&coder->rc, out, out_pos, out_size)) {
assert(limit == UINT32_MAX);
return LZMA_OK;
}
}


coder->is_flushed = false;

return LZMA_STREAM_END;
}


static lzma_ret
lzma_encode(lzma_coder *restrict coder, lzma_mf *restrict mf,
uint8_t *restrict out, size_t *restrict out_pos,
size_t out_size)
{

if (unlikely(mf->action == LZMA_SYNC_FLUSH))
return LZMA_OPTIONS_ERROR;

return lzma_lzma_encode(coder, mf, out, out_pos, out_size, UINT32_MAX);
}






static bool
is_options_valid(const lzma_options_lzma *options)
{


return is_lclppb_valid(options)
&& options->nice_len >= MATCH_LEN_MIN
&& options->nice_len <= MATCH_LEN_MAX
&& (options->mode == LZMA_MODE_FAST
|| options->mode == LZMA_MODE_NORMAL);
}


static void
set_lz_options(lzma_lz_options *lz_options, const lzma_options_lzma *options)
{


lz_options->before_size = OPTS;
lz_options->dict_size = options->dict_size;
lz_options->after_size = LOOP_INPUT_MAX;
lz_options->match_len_max = MATCH_LEN_MAX;
lz_options->nice_len = options->nice_len;
lz_options->match_finder = options->mf;
lz_options->depth = options->depth;
lz_options->preset_dict = options->preset_dict;
lz_options->preset_dict_size = options->preset_dict_size;
return;
}


static void
length_encoder_reset(lzma_length_encoder *lencoder,
const uint32_t num_pos_states, const bool fast_mode)
{
bit_reset(lencoder->choice);
bit_reset(lencoder->choice2);

for (size_t pos_state = 0; pos_state < num_pos_states; ++pos_state) {
bittree_reset(lencoder->low[pos_state], LEN_LOW_BITS);
bittree_reset(lencoder->mid[pos_state], LEN_MID_BITS);
}

bittree_reset(lencoder->high, LEN_HIGH_BITS);

if (!fast_mode)
for (size_t pos_state = 0; pos_state < num_pos_states;
++pos_state)
length_update_prices(lencoder, pos_state);

return;
}


extern lzma_ret
lzma_lzma_encoder_reset(lzma_coder *coder, const lzma_options_lzma *options)
{
if (!is_options_valid(options))
return LZMA_OPTIONS_ERROR;

coder->pos_mask = (1U << options->pb) - 1;
coder->literal_context_bits = options->lc;
coder->literal_pos_mask = (1U << options->lp) - 1;


rc_reset(&coder->rc);


coder->state = STATE_LIT_LIT;
for (size_t i = 0; i < REP_DISTANCES; ++i)
coder->reps[i] = 0;

literal_init(coder->literal, options->lc, options->lp);


for (size_t i = 0; i < STATES; ++i) {
for (size_t j = 0; j <= coder->pos_mask; ++j) {
bit_reset(coder->is_match[i][j]);
bit_reset(coder->is_rep0_long[i][j]);
}

bit_reset(coder->is_rep[i]);
bit_reset(coder->is_rep0[i]);
bit_reset(coder->is_rep1[i]);
bit_reset(coder->is_rep2[i]);
}

for (size_t i = 0; i < FULL_DISTANCES - END_POS_MODEL_INDEX; ++i)
bit_reset(coder->pos_special[i]);


for (size_t i = 0; i < LEN_TO_POS_STATES; ++i)
bittree_reset(coder->pos_slot[i], POS_SLOT_BITS);

bittree_reset(coder->pos_align, ALIGN_BITS);


length_encoder_reset(&coder->match_len_encoder,
1U << options->pb, coder->fast_mode);

length_encoder_reset(&coder->rep_len_encoder,
1U << options->pb, coder->fast_mode);














coder->match_price_count = UINT32_MAX / 2;
coder->align_price_count = UINT32_MAX / 2;

coder->opts_end_index = 0;
coder->opts_current_index = 0;

return LZMA_OK;
}


extern lzma_ret
lzma_lzma_encoder_create(lzma_coder **coder_ptr, lzma_allocator *allocator,
const lzma_options_lzma *options, lzma_lz_options *lz_options)
{

if (*coder_ptr == NULL) {
*coder_ptr = lzma_alloc(sizeof(lzma_coder), allocator);
if (*coder_ptr == NULL)
return LZMA_MEM_ERROR;
}

lzma_coder *coder = *coder_ptr;





switch (options->mode) {
case LZMA_MODE_FAST:
coder->fast_mode = true;
break;

case LZMA_MODE_NORMAL: {
coder->fast_mode = false;



uint32_t log_size = 0;
while ((UINT32_C(1) << log_size) < options->dict_size)
++log_size;

coder->dist_table_size = log_size * 2;


coder->match_len_encoder.table_size
= options->nice_len + 1 - MATCH_LEN_MIN;
coder->rep_len_encoder.table_size
= options->nice_len + 1 - MATCH_LEN_MIN;
break;
}

default:
return LZMA_OPTIONS_ERROR;
}





coder->is_initialized = options->preset_dict != NULL
&& options->preset_dict_size > 0;
coder->is_flushed = false;

set_lz_options(lz_options, options);

return lzma_lzma_encoder_reset(coder, options);
}


static lzma_ret
lzma_encoder_init(lzma_lz_encoder *lz, lzma_allocator *allocator,
const void *options, lzma_lz_options *lz_options)
{
lz->code = &lzma_encode;
return lzma_lzma_encoder_create(
&lz->coder, allocator, options, lz_options);
}


extern lzma_ret
lzma_lzma_encoder_init(lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter_info *filters)
{
return lzma_lz_encoder_init(
next, allocator, filters, &lzma_encoder_init);
}


extern uint64_t
lzma_lzma_encoder_memusage(const void *options)
{
if (!is_options_valid(options))
return UINT64_MAX;

lzma_lz_options lz_options;
set_lz_options(&lz_options, options);

const uint64_t lz_memusage = lzma_lz_encoder_memusage(&lz_options);
if (lz_memusage == UINT64_MAX)
return UINT64_MAX;

return (uint64_t)(sizeof(lzma_coder)) + lz_memusage;
}


extern bool
lzma_lzma_lclppb_encode(const lzma_options_lzma *options, uint8_t *byte)
{
if (!is_lclppb_valid(options))
return true;

*byte = (options->pb * 5 + options->lp) * 9 + options->lc;
assert(*byte <= (4 * 5 + 4) * 9 + 8);

return false;
}


#if defined(HAVE_ENCODER_LZMA1)
extern lzma_ret
lzma_lzma_props_encode(const void *options, uint8_t *out)
{
const lzma_options_lzma *const opt = options;

if (lzma_lzma_lclppb_encode(opt, out))
return LZMA_PROG_ERROR;

unaligned_write32le(out + 1, opt->dict_size);

return LZMA_OK;
}
#endif


extern LZMA_API(lzma_bool)
lzma_mode_is_supported(lzma_mode mode)
{
return mode == LZMA_MODE_FAST || mode == LZMA_MODE_NORMAL;
}
