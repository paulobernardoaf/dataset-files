




















#if !defined(AVCODEC_MPC7DATA_H)
#define AVCODEC_MPC7DATA_H

#include <stdint.h>

static const int8_t mpc7_idx30[] = { -1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1};
static const int8_t mpc7_idx31[] = { -1,-1,-1, 0, 0, 0, 1, 1, 1,-1,-1,-1, 0, 0, 0, 1, 1, 1,-1,-1,-1, 0, 0, 0, 1, 1, 1};
static const int8_t mpc7_idx32[] = { -1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1};
static const int8_t mpc7_idx50[] = { -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2};
static const int8_t mpc7_idx51[] = { -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2};

#define MPC7_SCFI_SIZE 4
#define MPC7_SCFI_BITS 3
static const uint8_t mpc7_scfi[MPC7_SCFI_SIZE * 2] = {
0x2, 3, 0x1, 1, 0x3, 3, 0x0, 2
};

#define MPC7_DSCF_SIZE 16
#define MPC7_DSCF_BITS 6
static const uint8_t mpc7_dscf[MPC7_DSCF_SIZE * 2] = {
0x20, 6, 0x04, 5, 0x11, 5, 0x1E, 5, 0x0D, 4, 0x00, 3, 0x03, 3, 0x09, 4,
0x05, 3, 0x02, 3, 0x0E, 4, 0x03, 4, 0x1F, 5, 0x05, 5, 0x21, 6, 0x0C, 4
};

#define MPC7_HDR_SIZE 10
#define MPC7_HDR_BITS 9
static const uint8_t mpc7_hdr[MPC7_HDR_SIZE * 2] = {
0x5C, 8, 0x2F, 7, 0x0A, 5, 0x04, 4, 0x00, 2,
0x01, 1, 0x03, 3, 0x16, 6, 0xBB, 9, 0xBA, 9
};

#define MPC7_QUANT_VLC_TABLES 7
static const uint8_t mpc7_quant_vlc_sizes[MPC7_QUANT_VLC_TABLES * 2] = {
27, 25, 7, 9, 15, 31, 63
};

static const uint8_t mpc7_quant_vlc_off[MPC7_QUANT_VLC_TABLES] = {
0, 0, 3, 4, 7, 15, 31
};

static const uint16_t mpc7_quant_vlc[MPC7_QUANT_VLC_TABLES][2][64 * 2] = {
{
{
0x0036, 6, 0x0009, 5, 0x0020, 6, 0x0005, 5, 0x000A, 4, 0x0007, 5,
0x0034, 6, 0x0000, 5, 0x0023, 6, 0x000A, 5, 0x0006, 4, 0x0004, 5,
0x000B, 4, 0x0007, 3, 0x000C, 4, 0x0003, 5, 0x0007, 4, 0x000B, 5,
0x0022, 6, 0x0001, 5, 0x0035, 6, 0x0006, 5, 0x0009, 4, 0x0002, 5,
0x0021, 6, 0x0008, 5, 0x0037, 6
},
{
0x0067, 8, 0x003E, 7, 0x00E1, 9, 0x0037, 7, 0x0003, 4, 0x0034, 7,
0x0065, 8, 0x003C, 7, 0x00E3, 9, 0x0018, 6, 0x0000, 4, 0x003D, 7,
0x0004, 4, 0x0001, 1, 0x0005, 4, 0x003F, 7, 0x0001, 4, 0x003B, 7,
0x00E2, 9, 0x0039, 7, 0x0064, 8, 0x0035, 7, 0x0002, 4, 0x0036, 7,
0x00E0, 9, 0x003A, 7, 0x0066, 8
}
},
{
{
0x0059, 7, 0x002F, 6, 0x000F, 5, 0x0000, 5, 0x005B, 7, 0x0004, 5,
0x0006, 4, 0x000D, 4, 0x0004, 4, 0x0005, 5, 0x0014, 5, 0x000C, 4,
0x0004, 3, 0x000F, 4, 0x000E, 5, 0x0003, 5, 0x0003, 4, 0x000E, 4,
0x0005, 4, 0x0001, 5, 0x005A, 7, 0x0002, 5, 0x0015, 5, 0x002E, 6,
0x0058, 7
},
{
0x0399, 10, 0x0071, 7, 0x0033, 6, 0x00E7, 8, 0x039A, 10, 0x0068, 7,
0x001E, 5, 0x0000, 3, 0x001D, 5, 0x0069, 7, 0x0032, 6, 0x0001, 3,
0x0002, 2, 0x0003, 3, 0x0031, 6, 0x006B, 7, 0x001B, 5, 0x0002, 3,
0x001F, 5, 0x0070, 7, 0x0398, 10, 0x006A, 7, 0x0030, 6, 0x0072, 7,
0x039B, 10
}
},
{
{
0x000C, 4, 0x0004, 3, 0x0000, 2, 0x0001, 2, 0x0007, 3, 0x0005, 3, 0x000D, 4
},
{
0x0004, 5, 0x0003, 4, 0x0002, 2, 0x0003, 2, 0x0001, 2, 0x0000, 3, 0x0005, 5
}
},
{
{
0x0005, 4, 0x0000, 3, 0x0004, 3, 0x0006, 3, 0x0007, 3, 0x0005, 3, 0x0003, 3, 0x0001, 3, 0x0004, 4
},
{
0x0009, 5, 0x000C, 4, 0x0003, 3, 0x0000, 2, 0x0002, 2, 0x0007, 3, 0x000D, 4, 0x0005, 4, 0x0008, 5
}
},
{
{
0x0039, 6, 0x0017, 5, 0x0008, 4, 0x000A, 4, 0x000D, 4, 0x0000, 3,
0x0002, 3, 0x0003, 3, 0x0001, 3, 0x000F, 4, 0x000C, 4, 0x0009, 4,
0x001D, 5, 0x0016, 5, 0x0038, 6,
},
{
0x00E5, 8, 0x0038, 6, 0x0007, 5, 0x0002, 4, 0x0000, 3, 0x0003, 3,
0x0005, 3, 0x0006, 3, 0x0004, 3, 0x0002, 3, 0x000F, 4, 0x001D, 5,
0x0006, 5, 0x0073, 7, 0x00E4, 8,
},
},
{
{
0x0041, 7, 0x0006, 6, 0x002C, 6, 0x002D, 6, 0x003B, 6, 0x000D, 5,
0x0011, 5, 0x0013, 5, 0x0017, 5, 0x0015, 5, 0x001A, 5, 0x001E, 5,
0x0000, 4, 0x0002, 4, 0x0005, 4, 0x0007, 4, 0x0003, 4, 0x0004, 4,
0x001F, 5, 0x001C, 5, 0x0019, 5, 0x001B, 5, 0x0018, 5, 0x0014, 5,
0x0012, 5, 0x000C, 5, 0x0002, 5, 0x003A, 6, 0x0021, 6, 0x0007, 6,
0x0040, 7
},
{
0x1948, 13, 0x194A, 13, 0x0328, 10, 0x0195, 9, 0x00CB, 8, 0x0066, 7,
0x0031, 6, 0x0009, 5, 0x000F, 5, 0x001F, 5, 0x0002, 4, 0x0006, 4,
0x0008, 4, 0x000B, 4, 0x000D, 4, 0x0000, 3, 0x000E, 4, 0x000A, 4,
0x0009, 4, 0x0005, 4, 0x0003, 4, 0x001E, 5, 0x000E, 5, 0x0008, 5,
0x0030, 6, 0x0067, 7, 0x00C9, 8, 0x00C8, 8, 0x0653, 11, 0x1949, 13,
0x194B, 13
}
},
{
{
0x0067, 8, 0x0099, 8, 0x00B5, 8, 0x00E9, 8, 0x0040, 7, 0x0041, 7,
0x004D, 7, 0x0051, 7, 0x005B, 7, 0x0071, 7, 0x0070, 7, 0x0018, 6,
0x001D, 6, 0x0023, 6, 0x0025, 6, 0x0029, 6, 0x002C, 6, 0x002E, 6,
0x0033, 6, 0x0031, 6, 0x0036, 6, 0x0037, 6, 0x0039, 6, 0x003C, 6,
0x0000, 5, 0x0002, 5, 0x000A, 5, 0x0005, 5, 0x0009, 5, 0x0006, 5,
0x000D, 5, 0x0007, 5, 0x000B, 5, 0x000F, 5, 0x0008, 5, 0x0004, 5,
0x0003, 5, 0x0001, 5, 0x003F, 6, 0x003E, 6, 0x003D, 6, 0x0035, 6,
0x003B, 6, 0x0034, 6, 0x0030, 6, 0x002F, 6, 0x002B, 6, 0x002A, 6,
0x0027, 6, 0x0024, 6, 0x0021, 6, 0x001C, 6, 0x0075, 7, 0x0065, 7,
0x0064, 7, 0x0050, 7, 0x0045, 7, 0x0044, 7, 0x0032, 7, 0x00E8, 8,
0x00B4, 8, 0x0098, 8, 0x0066, 8
},
{
0x37A4, 14, 0x37AD, 14, 0x37A6, 14, 0x37AE, 14, 0x0DEA, 12, 0x02F0, 10,
0x02F1, 10, 0x00A0, 9, 0x00A2, 9, 0x01BC, 9, 0x007A, 8, 0x00DF, 8,
0x003C, 7, 0x0049, 7, 0x006E, 7, 0x000E, 6, 0x0018, 6, 0x0019, 6,
0x0022, 6, 0x0025, 6, 0x0036, 6, 0x0003, 5, 0x0009, 5, 0x000B, 5,
0x0010, 5, 0x0013, 5, 0x0015, 5, 0x0018, 5, 0x001A, 5, 0x001D, 5,
0x001F, 5, 0x0002, 4, 0x0000, 4, 0x001E, 5, 0x001C, 5, 0x0019, 5,
0x0016, 5, 0x0014, 5, 0x000E, 5, 0x000D, 5, 0x0008, 5, 0x0006, 5,
0x0002, 5, 0x002E, 6, 0x0023, 6, 0x001F, 6, 0x0015, 6, 0x000F, 6,
0x005F, 7, 0x0048, 7, 0x0029, 7, 0x00BD, 8, 0x007B, 8, 0x0179, 9,
0x00A1, 9, 0x037B, 10, 0x0147, 10, 0x0146, 10, 0x0DE8, 12, 0x37AF, 14,
0x37A7, 14, 0x37AC, 14, 0x37A5, 14
}
}
};

#endif 
