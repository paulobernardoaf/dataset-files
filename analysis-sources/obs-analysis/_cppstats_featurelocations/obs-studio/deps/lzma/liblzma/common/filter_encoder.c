











#include "filter_encoder.h"
#include "filter_common.h"
#include "lzma_encoder.h"
#include "lzma2_encoder.h"
#include "simple_encoder.h"
#include "delta_encoder.h"


typedef struct {

lzma_vli id;



lzma_init_function init;



uint64_t (*memusage)(const void *options);





lzma_vli (*chunk_size)(const void *options);




lzma_ret (*props_size_get)(uint32_t *size, const void *options);
uint32_t props_size_fixed;







lzma_ret (*props_encode)(const void *options, uint8_t *out);

} lzma_filter_encoder;


static const lzma_filter_encoder encoders[] = {
#if defined(HAVE_ENCODER_LZMA1)
{
.id = LZMA_FILTER_LZMA1,
.init = &lzma_lzma_encoder_init,
.memusage = &lzma_lzma_encoder_memusage,
.chunk_size = NULL, 
.props_size_get = NULL,
.props_size_fixed = 5,
.props_encode = &lzma_lzma_props_encode,
},
#endif
#if defined(HAVE_ENCODER_LZMA2)
{
.id = LZMA_FILTER_LZMA2,
.init = &lzma_lzma2_encoder_init,
.memusage = &lzma_lzma2_encoder_memusage,
.chunk_size = NULL, 
.props_size_get = NULL,
.props_size_fixed = 1,
.props_encode = &lzma_lzma2_props_encode,
},
#endif
#if defined(HAVE_ENCODER_X86)
{
.id = LZMA_FILTER_X86,
.init = &lzma_simple_x86_encoder_init,
.memusage = NULL,
.chunk_size = NULL,
.props_size_get = &lzma_simple_props_size,
.props_encode = &lzma_simple_props_encode,
},
#endif
#if defined(HAVE_ENCODER_POWERPC)
{
.id = LZMA_FILTER_POWERPC,
.init = &lzma_simple_powerpc_encoder_init,
.memusage = NULL,
.chunk_size = NULL,
.props_size_get = &lzma_simple_props_size,
.props_encode = &lzma_simple_props_encode,
},
#endif
#if defined(HAVE_ENCODER_IA64)
{
.id = LZMA_FILTER_IA64,
.init = &lzma_simple_ia64_encoder_init,
.memusage = NULL,
.chunk_size = NULL,
.props_size_get = &lzma_simple_props_size,
.props_encode = &lzma_simple_props_encode,
},
#endif
#if defined(HAVE_ENCODER_ARM)
{
.id = LZMA_FILTER_ARM,
.init = &lzma_simple_arm_encoder_init,
.memusage = NULL,
.chunk_size = NULL,
.props_size_get = &lzma_simple_props_size,
.props_encode = &lzma_simple_props_encode,
},
#endif
#if defined(HAVE_ENCODER_ARMTHUMB)
{
.id = LZMA_FILTER_ARMTHUMB,
.init = &lzma_simple_armthumb_encoder_init,
.memusage = NULL,
.chunk_size = NULL,
.props_size_get = &lzma_simple_props_size,
.props_encode = &lzma_simple_props_encode,
},
#endif
#if defined(HAVE_ENCODER_SPARC)
{
.id = LZMA_FILTER_SPARC,
.init = &lzma_simple_sparc_encoder_init,
.memusage = NULL,
.chunk_size = NULL,
.props_size_get = &lzma_simple_props_size,
.props_encode = &lzma_simple_props_encode,
},
#endif
#if defined(HAVE_ENCODER_DELTA)
{
.id = LZMA_FILTER_DELTA,
.init = &lzma_delta_encoder_init,
.memusage = &lzma_delta_coder_memusage,
.chunk_size = NULL,
.props_size_get = NULL,
.props_size_fixed = 1,
.props_encode = &lzma_delta_props_encode,
},
#endif
};


static const lzma_filter_encoder *
encoder_find(lzma_vli id)
{
for (size_t i = 0; i < ARRAY_SIZE(encoders); ++i)
if (encoders[i].id == id)
return encoders + i;

return NULL;
}


extern LZMA_API(lzma_bool)
lzma_filter_encoder_is_supported(lzma_vli id)
{
return encoder_find(id) != NULL;
}


extern LZMA_API(lzma_ret)
lzma_filters_update(lzma_stream *strm, const lzma_filter *filters)
{
if (strm->internal->next.update == NULL)
return LZMA_PROG_ERROR;


if (lzma_raw_encoder_memusage(filters) == UINT64_MAX)
return LZMA_OPTIONS_ERROR;



size_t count = 1;
while (filters[count].id != LZMA_VLI_UNKNOWN)
++count;

lzma_filter reversed_filters[LZMA_FILTERS_MAX + 1];
for (size_t i = 0; i < count; ++i)
reversed_filters[count - i - 1] = filters[i];

reversed_filters[count].id = LZMA_VLI_UNKNOWN;

return strm->internal->next.update(strm->internal->next.coder,
strm->allocator, filters, reversed_filters);
}


extern lzma_ret
lzma_raw_encoder_init(lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter *options)
{
return lzma_raw_coder_init(next, allocator,
options, (lzma_filter_find)(&encoder_find), true);
}


extern LZMA_API(lzma_ret)
lzma_raw_encoder(lzma_stream *strm, const lzma_filter *options)
{
lzma_next_strm_init(lzma_raw_coder_init, strm, options,
(lzma_filter_find)(&encoder_find), true);

strm->internal->supported_actions[LZMA_RUN] = true;
strm->internal->supported_actions[LZMA_SYNC_FLUSH] = true;
strm->internal->supported_actions[LZMA_FINISH] = true;

return LZMA_OK;
}


extern LZMA_API(uint64_t)
lzma_raw_encoder_memusage(const lzma_filter *filters)
{
return lzma_raw_coder_memusage(
(lzma_filter_find)(&encoder_find), filters);
}



























extern LZMA_API(lzma_ret)
lzma_properties_size(uint32_t *size, const lzma_filter *filter)
{
const lzma_filter_encoder *const fe = encoder_find(filter->id);
if (fe == NULL) {




return filter->id <= LZMA_VLI_MAX
? LZMA_OPTIONS_ERROR : LZMA_PROG_ERROR;
}

if (fe->props_size_get == NULL) {

*size = fe->props_size_fixed;
return LZMA_OK;
}

return fe->props_size_get(size, filter->options);
}


extern LZMA_API(lzma_ret)
lzma_properties_encode(const lzma_filter *filter, uint8_t *props)
{
const lzma_filter_encoder *const fe = encoder_find(filter->id);
if (fe == NULL)
return LZMA_PROG_ERROR;

if (fe->props_encode == NULL)
return LZMA_OK;

return fe->props_encode(filter->options, props);
}
