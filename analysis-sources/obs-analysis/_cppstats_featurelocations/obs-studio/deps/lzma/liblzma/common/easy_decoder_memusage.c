











#include "easy_preset.h"


extern LZMA_API(uint64_t)
lzma_easy_decoder_memusage(uint32_t preset)
{
lzma_options_easy opt_easy;
if (lzma_easy_preset(&opt_easy, preset))
return UINT32_MAX;

return lzma_raw_decoder_memusage(opt_easy.filters);
}
