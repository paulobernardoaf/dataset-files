#include "filter_common.h"
static const struct {
lzma_vli id;
size_t options_size;
bool non_last_ok;
bool last_ok;
bool changes_size;
} features[] = {
#if defined (HAVE_ENCODER_LZMA1) || defined(HAVE_DECODER_LZMA1)
{
.id = LZMA_FILTER_LZMA1,
.options_size = sizeof(lzma_options_lzma),
.non_last_ok = false,
.last_ok = true,
.changes_size = true,
},
#endif
#if defined(HAVE_ENCODER_LZMA2) || defined(HAVE_DECODER_LZMA2)
{
.id = LZMA_FILTER_LZMA2,
.options_size = sizeof(lzma_options_lzma),
.non_last_ok = false,
.last_ok = true,
.changes_size = true,
},
#endif
#if defined(HAVE_ENCODER_X86) || defined(HAVE_DECODER_X86)
{
.id = LZMA_FILTER_X86,
.options_size = sizeof(lzma_options_bcj),
.non_last_ok = true,
.last_ok = false,
.changes_size = false,
},
#endif
#if defined(HAVE_ENCODER_POWERPC) || defined(HAVE_DECODER_POWERPC)
{
.id = LZMA_FILTER_POWERPC,
.options_size = sizeof(lzma_options_bcj),
.non_last_ok = true,
.last_ok = false,
.changes_size = false,
},
#endif
#if defined(HAVE_ENCODER_IA64) || defined(HAVE_DECODER_IA64)
{
.id = LZMA_FILTER_IA64,
.options_size = sizeof(lzma_options_bcj),
.non_last_ok = true,
.last_ok = false,
.changes_size = false,
},
#endif
#if defined(HAVE_ENCODER_ARM) || defined(HAVE_DECODER_ARM)
{
.id = LZMA_FILTER_ARM,
.options_size = sizeof(lzma_options_bcj),
.non_last_ok = true,
.last_ok = false,
.changes_size = false,
},
#endif
#if defined(HAVE_ENCODER_ARMTHUMB) || defined(HAVE_DECODER_ARMTHUMB)
{
.id = LZMA_FILTER_ARMTHUMB,
.options_size = sizeof(lzma_options_bcj),
.non_last_ok = true,
.last_ok = false,
.changes_size = false,
},
#endif
#if defined(HAVE_ENCODER_SPARC) || defined(HAVE_DECODER_SPARC)
{
.id = LZMA_FILTER_SPARC,
.options_size = sizeof(lzma_options_bcj),
.non_last_ok = true,
.last_ok = false,
.changes_size = false,
},
#endif
#if defined(HAVE_ENCODER_DELTA) || defined(HAVE_DECODER_DELTA)
{
.id = LZMA_FILTER_DELTA,
.options_size = sizeof(lzma_options_delta),
.non_last_ok = true,
.last_ok = false,
.changes_size = false,
},
#endif
{
.id = LZMA_VLI_UNKNOWN
}
};
extern LZMA_API(lzma_ret)
lzma_filters_copy(const lzma_filter *src, lzma_filter *dest,
lzma_allocator *allocator)
{
if (src == NULL || dest == NULL)
return LZMA_PROG_ERROR;
lzma_ret ret;
size_t i;
for (i = 0; src[i].id != LZMA_VLI_UNKNOWN; ++i) {
if (i == LZMA_FILTERS_MAX) {
ret = LZMA_OPTIONS_ERROR;
goto error;
}
dest[i].id = src[i].id;
if (src[i].options == NULL) {
dest[i].options = NULL;
} else {
size_t j;
for (j = 0; src[i].id != features[j].id; ++j) {
if (features[j].id == LZMA_VLI_UNKNOWN) {
ret = LZMA_OPTIONS_ERROR;
goto error;
}
}
dest[i].options = lzma_alloc(features[j].options_size,
allocator);
if (dest[i].options == NULL) {
ret = LZMA_MEM_ERROR;
goto error;
}
memcpy(dest[i].options, src[i].options,
features[j].options_size);
}
}
assert(i <= LZMA_FILTERS_MAX + 1);
dest[i].id = LZMA_VLI_UNKNOWN;
dest[i].options = NULL;
return LZMA_OK;
error:
while (i-- > 0) {
lzma_free(dest[i].options, allocator);
dest[i].options = NULL;
}
return ret;
}
static lzma_ret
validate_chain(const lzma_filter *filters, size_t *count)
{
if (filters == NULL || filters[0].id == LZMA_VLI_UNKNOWN)
return LZMA_PROG_ERROR;
size_t changes_size_count = 0;
bool non_last_ok = true;
bool last_ok = false;
size_t i = 0;
do {
size_t j;
for (j = 0; filters[i].id != features[j].id; ++j)
if (features[j].id == LZMA_VLI_UNKNOWN)
return LZMA_OPTIONS_ERROR;
if (!non_last_ok)
return LZMA_OPTIONS_ERROR;
non_last_ok = features[j].non_last_ok;
last_ok = features[j].last_ok;
changes_size_count += features[j].changes_size;
} while (filters[++i].id != LZMA_VLI_UNKNOWN);
if (i > LZMA_FILTERS_MAX || !last_ok || changes_size_count > 3)
return LZMA_OPTIONS_ERROR;
*count = i;
return LZMA_OK;
}
extern lzma_ret
lzma_raw_coder_init(lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter *options,
lzma_filter_find coder_find, bool is_encoder)
{
size_t count;
return_if_error(validate_chain(options, &count));
lzma_filter_info filters[LZMA_FILTERS_MAX + 1];
if (is_encoder) {
for (size_t i = 0; i < count; ++i) {
const size_t j = count - i - 1;
const lzma_filter_coder *const fc
= coder_find(options[i].id);
if (fc == NULL || fc->init == NULL)
return LZMA_OPTIONS_ERROR;
filters[j].id = options[i].id;
filters[j].init = fc->init;
filters[j].options = options[i].options;
}
} else {
for (size_t i = 0; i < count; ++i) {
const lzma_filter_coder *const fc
= coder_find(options[i].id);
if (fc == NULL || fc->init == NULL)
return LZMA_OPTIONS_ERROR;
filters[i].id = options[i].id;
filters[i].init = fc->init;
filters[i].options = options[i].options;
}
}
filters[count].id = LZMA_VLI_UNKNOWN;
filters[count].init = NULL;
const lzma_ret ret = lzma_next_filter_init(next, allocator, filters);
if (ret != LZMA_OK)
lzma_next_end(next, allocator);
return ret;
}
extern uint64_t
lzma_raw_coder_memusage(lzma_filter_find coder_find,
const lzma_filter *filters)
{
{
size_t tmp;
if (validate_chain(filters, &tmp) != LZMA_OK)
return UINT64_MAX;
}
uint64_t total = 0;
size_t i = 0;
do {
const lzma_filter_coder *const fc
= coder_find(filters[i].id);
if (fc == NULL)
return UINT64_MAX; 
if (fc->memusage == NULL) {
total += 1024;
} else {
const uint64_t usage
= fc->memusage(filters[i].options);
if (usage == UINT64_MAX)
return UINT64_MAX; 
total += usage;
}
} while (filters[++i].id != LZMA_VLI_UNKNOWN);
return total + LZMA_MEMUSAGE_BASE;
}
