




















#include "blockdsp_mips.h"

#if HAVE_MSA
static av_cold void blockdsp_init_msa(BlockDSPContext *c)
{
c->clear_block = ff_clear_block_msa;
c->clear_blocks = ff_clear_blocks_msa;

c->fill_block_tab[0] = ff_fill_block16_msa;
c->fill_block_tab[1] = ff_fill_block8_msa;
}
#endif 

#if HAVE_MMI
static av_cold void blockdsp_init_mmi(BlockDSPContext *c)
{
c->clear_block = ff_clear_block_mmi;
c->clear_blocks = ff_clear_blocks_mmi;

c->fill_block_tab[0] = ff_fill_block16_mmi;
c->fill_block_tab[1] = ff_fill_block8_mmi;
}
#endif 

void ff_blockdsp_init_mips(BlockDSPContext *c)
{
#if HAVE_MMI
blockdsp_init_mmi(c);
#endif 
#if HAVE_MSA
blockdsp_init_msa(c);
#endif 
}
