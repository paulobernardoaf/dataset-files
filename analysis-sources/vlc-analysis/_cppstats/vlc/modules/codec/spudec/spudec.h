typedef struct
{
bool b_packetizer;
bool b_disabletrans;
vlc_tick_t i_pts;
unsigned int i_spu_size;
unsigned int i_rle_size;
unsigned int i_spu;
block_t *p_block;
uint8_t buffer[65536];
} decoder_sys_t;
#define SPU_CHUNK_SIZE 0x200
#define SPU_CMD_FORCE_DISPLAY 0x00
#define SPU_CMD_START_DISPLAY 0x01
#define SPU_CMD_STOP_DISPLAY 0x02
#define SPU_CMD_SET_PALETTE 0x03
#define SPU_CMD_SET_ALPHACHANNEL 0x04
#define SPU_CMD_SET_COORDINATES 0x05
#define SPU_CMD_SET_OFFSETS 0x06
#define SPU_CMD_SET_COLCON 0x07
#define SPU_CMD_END 0xff
void ParsePacket( decoder_t *, void(*pf_queue)(decoder_t *, subpicture_t *) );
