#include <X11/extensions/XvMC.h>
#include "libavutil/attributes.h"
#include "version.h"
#include "avcodec.h"
#define AV_XVMC_ID 0x1DC711C0 
struct attribute_deprecated xvmc_pix_fmt {
int xvmc_id;
short* data_blocks;
XvMCMacroBlock* mv_blocks;
int allocated_mv_blocks;
int allocated_data_blocks;
int idct;
int unsigned_intra;
XvMCSurface* p_surface;
XvMCSurface* p_past_surface;
XvMCSurface* p_future_surface;
unsigned int picture_structure;
unsigned int flags;
int start_mv_blocks_num;
int filled_mv_blocks_num;
int next_free_data_block_num;
};
