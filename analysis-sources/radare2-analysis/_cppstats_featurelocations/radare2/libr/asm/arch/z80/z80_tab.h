

#if !defined(Z80_TAB_H)
#define Z80_TAB_H

#if defined(NULL)
#undef NULL
#endif
#define NULL 0

enum {
Z80_OP_UNK = 1,
Z80_OP8 = 2,
Z80_OP16 = 4,
Z80_OP24 = 8,
Z80_ARG8 = 16,
Z80_ARG16 = 32,
Z80_ENC0 = 64,
Z80_ENC1 = 128
};

typedef struct{
char* name;
int type;
void *op_moar;
} z80_opcode;

static ut8 z80_fddd_branch_index_res (ut8 hex) {
switch (hex) {
case 0x9:
return 0x0;
case 0x19:
return 0x1;
case 0x21:
case 0x22:
case 0x23:
case 0x24:
case 0x25:
case 0x26:
return hex-0x1f;
case 0x29:
case 0x2a:
case 0x2b:
case 0x2c:
case 0x2d:
case 0x2e:
return hex-0x21;
case 0x34:
case 0x35:
case 0x36:
return hex-0x26;
case 0x39:
return 0x11;
case 0x44:
case 0x45:
case 0x46:
return hex-0x32;
case 0x4c:
case 0x4d:
case 0x4e:
return hex-0x37;
case 0x54: 
case 0x55:
case 0x56:
return hex-0x3c;
case 0x5c:
case 0x5d:
case 0x5e: 
return hex-0x41;
case 0x60:
case 0x61:
case 0x62:
case 0x63:
case 0x64:
case 0x65:
case 0x66:
case 0x67:
case 0x68:
case 0x69:
case 0x6a:
case 0x6b:
case 0x6c:
case 0x6d:
case 0x6e:
case 0x6f:
case 0x70:
case 0x71:
case 0x72:
case 0x73:
case 0x74:
case 0x75:
return hex-0x42;
case 0x77:
return 0x34;
case 0x7c:
case 0x7d:
case 0x7e:
return hex-0x47;
case 0x84:
case 0x85:
case 0x86:
return hex-0x4c;
case 0x8c:
case 0x8d:
case 0x8e:
return hex-0x51;
case 0x94:
case 0x95:
case 0x96:
return hex-0x56;
case 0x9c:
case 0x9d:
case 0x9e:
return hex-0x5b;
case 0xa4:
case 0xa5:
case 0xa6:
return hex-0x60;
case 0xac:
case 0xad:
case 0xae:
return hex-0x65;
case 0xb4:
case 0xb5:
case 0xb6:
return hex-0x6a;
case 0xbc:
case 0xbd:
case 0xbe:
return hex-0x6f;
case 0xcb:
return 0x50;
case 0xe1:
return 0x51;
case 0xe3:
return 0x52;
case 0xe5:
return 0x53;
case 0xe9:
return 0x54;
case 0xf9:
return 0x55;
}
return 0x56;
}

static ut8 z80_ed_branch_index_res (ut8 hex) {
if (hex > 0x3f && 0x4c > hex)
return hex-0x40;
if (hex == 0x4d)
return 0xc;
if (hex > 0x4d && 0x54 > hex)
return hex-0x42;
if (hex > 0x55 && 0x5c > hex)
return hex-0x44;
if (hex > 0x5d && 0x63 > hex)
return hex-0x46;
if (hex > 0x66 && 0x6b > hex)
return hex-0x4a;
if (hex > 0x6e && 0x74 > hex)
return hex-0x4e;
if (hex > 0x77 && 0x7c > hex)
return hex-0x52;
if (hex > 0x9f && 0xa4 > hex)
return hex-0x76;
if (hex > 0xa7 && 0xac > hex)
return hex-0x7a;
if (hex > 0xaf && 0xb4 > hex)
return hex-0x7e;
if (hex > 0xb7 && 0xbc > hex)
return hex-0x82;
return 0x3b;
}

static const char *cb[] = {
"rlc b",
"rlc c",
"rlc d",
"rlc e",
"rlc h",
"rlc l",
"rlc [hl]",
"rlc a",
"rrc b",
"rrc c",
"rrc d",
"rrc e",
"rrc h",
"rrc l",
"rrc [hl]",
"rrc a",
"rl b",
"rl c",
"rl d",
"rl e",
"rl h",
"rl l",
"rl [hl]",
"rl a",
"rr b",
"rr c",
"rr d",
"rr e",
"rr h",
"rr l",
"rr [hl]",
"rr a",
"sla b",
"sla c",
"sla d",
"sla e",
"sla h",
"sla l",
"sla [hl]",
"sla a",
"sra b",
"sra c",
"sra d",
"sra e",
"sra h",
"sra l",
"sra [hl]",
"sra a",
"sll b",
"sll c",
"sll d",
"sll e",
"sll h",
"sll l",
"sll [hl]",
"sll a",
"srl b",
"srl c",
"srl d",
"srl e",
"srl h",
"srl l",
"srl [hl]",
"srl a",
"bit 0, b",
"bit 0, c",
"bit 0, d",
"bit 0, e",
"bit 0, h",
"bit 0, l",
"bit 0, [hl]",
"bit 0, a",
"bit 1, b",
"bit 1, c",
"bit 1, d",
"bit 1, e",
"bit 1, h",
"bit 1, l",
"bit 1, [hl]",
"bit 1, a",
"bit 2, b",
"bit 2, c",
"bit 2, d",
"bit 2, e",
"bit 2, h",
"bit 2, l",
"bit 2, [hl]",
"bit 2, a",
"bit 3, b",
"bit 3, c",
"bit 3, d",
"bit 3, e",
"bit 3, h",
"bit 3, l",
"bit 3, [hl]",
"bit 3, a",
"bit 4, b",
"bit 4, c",
"bit 4, d",
"bit 4, e",
"bit 4, h",
"bit 4, l",
"bit 4, [hl]",
"bit 4, a",
"bit 5, b",
"bit 5, c",
"bit 5, d",
"bit 5, e",
"bit 5, h",
"bit 5, l",
"bit 5, [hl]",
"bit 5, a",
"bit 6, b",
"bit 6, c",
"bit 6, d",
"bit 6, e",
"bit 6, h",
"bit 6, l",
"bit 6, [hl]",
"bit 6, a",
"bit 7, b",
"bit 7, c",
"bit 7, d",
"bit 7, e",
"bit 7, h",
"bit 7, l",
"bit 7, [hl]",
"bit 7, a",
"res 0, b",
"res 0, c",
"res 0, d",
"res 0, e",
"res 0, h",
"res 0, l",
"res 0, [hl]",
"res 0, a",
"res 1, b",
"res 1, c",
"res 1, d",
"res 1, e",
"res 1, h",
"res 1, l",
"res 1, [hl]",
"res 1, a",
"res 2, b",
"res 2, c",
"res 2, d",
"res 2, e",
"res 2, h",
"res 2, l",
"res 2, [hl]",
"res 2, a",
"res 3, b",
"res 3, c",
"res 3, d",
"res 3, e",
"res 3, h",
"res 3, l",
"res 3, [hl]",
"res 3, a",
"res 4, b",
"res 4, c",
"res 4, d",
"res 4, e",
"res 4, h",
"res 4, l",
"res 4, [hl]",
"res 4, a",
"res 5, b",
"res 5, c",
"res 5, d",
"res 5, e",
"res 5, h",
"res 5, l",
"res 5, [hl]",
"res 5, a",
"res 6, b",
"res 6, c",
"res 6, d",
"res 6, e",
"res 6, h",
"res 6, l",
"res 6, [hl]",
"res 6, a",
"res 7, b",
"res 7, c",
"res 7, d",
"res 7, e",
"res 7, h",
"res 7, l",
"res 7, [hl]",
"res 7, a",
"set 0, b",
"set 0, c",
"set 0, d",
"set 0, e",
"set 0, h",
"set 0, l",
"set 0, [hl]",
"set 0, a",
"set 1, b",
"set 1, c",
"set 1, d",
"set 1, e",
"set 1, h",
"set 1, l",
"set 1, [hl]",
"set 1, a",
"set 2, b",
"set 2, c",
"set 2, d",
"set 2, e",
"set 2, h",
"set 2, l",
"set 2, [hl]",
"set 2, a",
"set 3, b",
"set 3, c",
"set 3, d",
"set 3, e",
"set 3, h",
"set 3, l",
"set 3, [hl]",
"set 3, a",
"set 4, b",
"set 4, c",
"set 4, d",
"set 4, e",
"set 4, h",
"set 4, l",
"set 4, [hl]",
"set 4, a",
"set 5, b",
"set 5, c",
"set 5, d",
"set 5, e",
"set 5, h",
"set 5, l",
"set 5, [hl]",
"set 5, a",
"set 6, b",
"set 6, c",
"set 6, d",
"set 6, e",
"set 6, h",
"set 6, l",
"set 6, [hl]",
"set 6, a",
"set 7, b",
"set 7, c",
"set 7, d",
"set 7, e",
"set 7, h",
"set 7, l",
"set 7, [hl]",
"set 7, a",
};

static const char *ddcb[]={
"rlc [ix+0x%02x], b",
"rlc [ix+0x%02x], c",
"rlc [ix+0x%02x], d",
"rlc [ix+0x%02x], e",
"rlc [ix+0x%02x], h",
"rlc [ix+0x%02x], l",
"rlc [ix+0x%02x]",
"rlc [ix+0x%02x], a",
"rrc [ix+0x%02x], b",
"rrc [ix+0x%02x], c",
"rrc [ix+0x%02x], d",
"rrc [ix+0x%02x], e",
"rrc [ix+0x%02x], h",
"rrc [ix+0x%02x], l",
"rrc [ix+0x%02x]",
"rrc [ix+0x%02x], a",
"rl [ix+0x%02x], b",
"rl [ix+0x%02x], c",
"rl [ix+0x%02x], d",
"rl [ix+0x%02x], e",
"rl [ix+0x%02x], h",
"rl [ix+0x%02x], l",
"rl [ix+0x%02x]",
"rl [ix+0x%02x], a",
"rr [ix+0x%02x], b",
"rr [ix+0x%02x], c",
"rr [ix+0x%02x], d",
"rr [ix+0x%02x], e",
"rr [ix+0x%02x], h",
"rr [ix+0x%02x], l",
"rr [ix+0x%02x]",
"rr [ix+0x%02x], a",
"sla [ix+0x%02x], b",
"sla [ix+0x%02x], c",
"sla [ix+0x%02x], d",
"sla [ix+0x%02x], e",
"sla [ix+0x%02x], h",
"sla [ix+0x%02x], l",
"sla [ix+0x%02x]",
"sla [ix+0x%02x], a",
"sra [ix+0x%02x], b",
"sra [ix+0x%02x], c",
"sra [ix+0x%02x], d",
"sra [ix+0x%02x], e",
"sra [ix+0x%02x], h",
"sra [ix+0x%02x], l",
"sra [ix+0x%02x]",
"sra [ix+0x%02x], a",
"sll [ix+0x%02x], b",
"sll [ix+0x%02x], c",
"sll [ix+0x%02x], d",
"sll [ix+0x%02x], e",
"sll [ix+0x%02x], h",
"sll [ix+0x%02x], l",
"sll [ix+0x%02x]",
"sll [ix+0x%02x], a",
"srl [ix+0x%02x], b",
"srl [ix+0x%02x], c",
"srl [ix+0x%02x], d",
"srl [ix+0x%02x], e",
"srl [ix+0x%02x], h",
"srl [ix+0x%02x], l",
"srl [ix+0x%02x]",
"srl [ix+0x%02x], a",
"bit 0, [ix+0x%02x]",
"bit 1, [ix+0x%02x]",
"bit 2, [ix+0x%02x]",
"bit 3, [ix+0x%02x]",
"bit 4, [ix+0x%02x]",
"bit 5, [ix+0x%02x]",
"bit 6, [ix+0x%02x]",
"bit 7, [ix+0x%02x]",
"res 0, [ix+0x%02x], b",
"res 0, [ix+0x%02x], c",
"res 0, [ix+0x%02x], d",
"res 0, [ix+0x%02x], e",
"res 0, [ix+0x%02x], h",
"res 0, [ix+0x%02x], l",
"res 0, [ix+0x%02x]",
"res 0, [ix+0x%02x], a",
"res 1, [ix+0x%02x], b",
"res 1, [ix+0x%02x], c",
"res 1, [ix+0x%02x], d",
"res 1, [ix+0x%02x], e",
"res 1, [ix+0x%02x], h",
"res 1, [ix+0x%02x], l",
"res 1, [ix+0x%02x]",
"res 1, [ix+0x%02x], a",
"res 2, [ix+0x%02x], b",
"res 2, [ix+0x%02x], c",
"res 2, [ix+0x%02x], d",
"res 2, [ix+0x%02x], e",
"res 2, [ix+0x%02x], h",
"res 2, [ix+0x%02x], l",
"res 2, [ix+0x%02x]",
"res 2, [ix+0x%02x], a",
"res 3, [ix+0x%02x], b",
"res 3, [ix+0x%02x], c",
"res 3, [ix+0x%02x], d",
"res 3, [ix+0x%02x], e",
"res 3, [ix+0x%02x], h",
"res 3, [ix+0x%02x], l",
"res 3, [ix+0x%02x]",
"res 3, [ix+0x%02x], a",
"res 4, [ix+0x%02x], b",
"res 4, [ix+0x%02x], c",
"res 4, [ix+0x%02x], d",
"res 4, [ix+0x%02x], e",
"res 4, [ix+0x%02x], h",
"res 4, [ix+0x%02x], l",
"res 4, [ix+0x%02x]",
"res 4, [ix+0x%02x], a",
"res 5, [ix+0x%02x], b",
"res 5, [ix+0x%02x], c",
"res 5, [ix+0x%02x], d",
"res 5, [ix+0x%02x], e",
"res 5, [ix+0x%02x], h",
"res 5, [ix+0x%02x], l",
"res 5, [ix+0x%02x]",
"res 5, [ix+0x%02x], a",
"res 6, [ix+0x%02x], b",
"res 6, [ix+0x%02x], c",
"res 6, [ix+0x%02x], d",
"res 6, [ix+0x%02x], e",
"res 6, [ix+0x%02x], h",
"res 6, [ix+0x%02x], l",
"res 6, [ix+0x%02x]",
"res 6, [ix+0x%02x], a",
"res 7, [ix+0x%02x], b",
"res 7, [ix+0x%02x], c",
"res 7, [ix+0x%02x], d",
"res 7, [ix+0x%02x], e",
"res 7, [ix+0x%02x], h",
"res 7, [ix+0x%02x], l",
"res 7, [ix+0x%02x]",
"res 7, [ix+0x%02x], a",
"set 0, [ix+0x%02x], b",
"set 0, [ix+0x%02x], c",
"set 0, [ix+0x%02x], d",
"set 0, [ix+0x%02x], e",
"set 0, [ix+0x%02x], h",
"set 0, [ix+0x%02x], l",
"set 0, [ix+0x%02x]",
"set 0, [ix+0x%02x], a",
"set 1, [ix+0x%02x], b",
"set 1, [ix+0x%02x], c",
"set 1, [ix+0x%02x], d",
"set 1, [ix+0x%02x], e",
"set 1, [ix+0x%02x], h",
"set 1, [ix+0x%02x], l",
"set 1, [ix+0x%02x]",
"set 1, [ix+0x%02x], a",
"set 2, [ix+0x%02x], b",
"set 2, [ix+0x%02x], c",
"set 2, [ix+0x%02x], d",
"set 2, [ix+0x%02x], e",
"set 2, [ix+0x%02x], h",
"set 2, [ix+0x%02x], l",
"set 2, [ix+0x%02x]",
"set 2, [ix+0x%02x], a",
"set 3, [ix+0x%02x], b",
"set 3, [ix+0x%02x], c",
"set 3, [ix+0x%02x], d",
"set 3, [ix+0x%02x], e",
"set 3, [ix+0x%02x], h",
"set 3, [ix+0x%02x], l",
"set 3, [ix+0x%02x]",
"set 3, [ix+0x%02x], a",
"set 4, [ix+0x%02x], b",
"set 4, [ix+0x%02x], c",
"set 4, [ix+0x%02x], d",
"set 4, [ix+0x%02x], e",
"set 4, [ix+0x%02x], h",
"set 4, [ix+0x%02x], l",
"set 4, [ix+0x%02x]",
"set 4, [ix+0x%02x], a",
"set 5, [ix+0x%02x], b",
"set 5, [ix+0x%02x], c",
"set 5, [ix+0x%02x], d",
"set 5, [ix+0x%02x], e",
"set 5, [ix+0x%02x], h",
"set 5, [ix+0x%02x], l",
"set 5, [ix+0x%02x]",
"set 5, [ix+0x%02x], a",
"set 6, [ix+0x%02x], b",
"set 6, [ix+0x%02x], c",
"set 6, [ix+0x%02x], d",
"set 6, [ix+0x%02x], e",
"set 6, [ix+0x%02x], h",
"set 6, [ix+0x%02x], l",
"set 6, [ix+0x%02x]",
"set 6, [ix+0x%02x], a",
"set 7, [ix+0x%02x], b",
"set 7, [ix+0x%02x], c",
"set 7, [ix+0x%02x], d",
"set 7, [ix+0x%02x], e",
"set 7, [ix+0x%02x], h",
"set 7, [ix+0x%02x], l",
"set 7, [ix+0x%02x]",
"set 7, [ix+0x%02x], a",
"%x\rinvalid", 
};

static const char *fdcb[]={
"rlc b, [iy+0x%02x]",
"rlc c, [iy+0x%02x]",
"rlc d, [iy+0x%02x]",
"rlc e, [iy+0x%02x]",
"rlc h, [iy+0x%02x]",
"rlc l, [iy+0x%02x]",
"rlc [iy+0x%02x]",
"rlc a, [iy+0x%02x]",
"rrc b, [iy+0x%02x]",
"rrc c, [iy+0x%02x]",
"rrc d, [iy+0x%02x]",
"rrc e, [iy+0x%02x]",
"rrc h, [iy+0x%02x]",
"rrc l, [iy+0x%02x]",
"rrc [iy+0x%02x]",
"rrc a, [iy+0x%02x]",
"rl b, [iy+0x%02x]",
"rl c, [iy+0x%02x]",
"rl d, [iy+0x%02x]",
"rl e, [iy+0x%02x]",
"rl h, [iy+0x%02x]",
"rl l, [iy+0x%02x]",
"rl [iy+0x%02x]",
"rl a, [iy+0x%02x]",
"rr b, [iy+0x%02x]",
"rr c, [iy+0x%02x]",
"rr d, [iy+0x%02x]",
"rr e, [iy+0x%02x]",
"rr h, [iy+0x%02x]",
"rr l, [iy+0x%02x]",
"rr [iy+0x%02x]",
"rr a, [iy+0x%02x]",
"sla b, [iy+0x%02x]",
"sla c, [iy+0x%02x]",
"sla d, [iy+0x%02x]",
"sla e, [iy+0x%02x]",
"sla h, [iy+0x%02x]",
"sla l, [iy+0x%02x]",
"sla [iy+0x%02x]",
"sla a, [iy+0x%02x]",
"sra b, [iy+0x%02x]",
"sra c, [iy+0x%02x]",
"sra d, [iy+0x%02x]",
"sra e, [iy+0x%02x]",
"sra h, [iy+0x%02x]",
"sra l, [iy+0x%02x]",
"sra [iy+0x%02x]",
"sra a, [iy+0x%02x]",
"sll b, [iy+0x%02x]",
"sll c, [iy+0x%02x]",
"sll d, [iy+0x%02x]",
"sll e, [iy+0x%02x]",
"sll h, [iy+0x%02x]",
"sll l, [iy+0x%02x]",
"sll [iy+0x%02x]",
"sll a, [iy+0x%02x]",
"srl b, [iy+0x%02x]",
"srl c, [iy+0x%02x]",
"srl d, [iy+0x%02x]",
"srl e, [iy+0x%02x]",
"srl h, [iy+0x%02x]",
"srl l, [iy+0x%02x]",
"srl [iy+0x%02x]",
"srl a, [iy+0x%02x]",
"bit 0, [iy+0x%02x]",
"bit 1, [iy+0x%02x]",
"bit 2, [iy+0x%02x]",
"bit 3, [iy+0x%02x]",
"bit 4, [iy+0x%02x]",
"bit 5, [iy+0x%02x]",
"bit 6, [iy+0x%02x]",
"bit 7, [iy+0x%02x]",
"res 0, [iy+0x%02x], b",
"res 0, [iy+0x%02x], c",
"res 0, [iy+0x%02x], d",
"res 0, [iy+0x%02x], e",
"res 0, [iy+0x%02x], h",
"res 0, [iy+0x%02x], l",
"res 0, [iy+0x%02x]",
"res 0, [iy+0x%02x], a",
"res 1, [iy+0x%02x], b",
"res 1, [iy+0x%02x], c",
"res 1, [iy+0x%02x], d",
"res 1, [iy+0x%02x], e",
"res 1, [iy+0x%02x], h",
"res 1, [iy+0x%02x], l",
"res 1, [iy+0x%02x]",
"res 1, [iy+0x%02x], a",
"res 2, [iy+0x%02x], b",
"res 2, [iy+0x%02x], c",
"res 2, [iy+0x%02x], d",
"res 2, [iy+0x%02x], e",
"res 2, [iy+0x%02x], h",
"res 2, [iy+0x%02x], l",
"res 2, [iy+0x%02x]",
"res 2, [iy+0x%02x], a",
"res 3, [iy+0x%02x], b",
"res 3, [iy+0x%02x], c",
"res 3, [iy+0x%02x], d",
"res 3, [iy+0x%02x], e",
"res 3, [iy+0x%02x], h",
"res 3, [iy+0x%02x], l",
"res 3, [iy+0x%02x]",
"res 3, [iy+0x%02x], a",
"res 4, [iy+0x%02x], b",
"res 4, [iy+0x%02x], c",
"res 4, [iy+0x%02x], d",
"res 4, [iy+0x%02x], e",
"res 4, [iy+0x%02x], h",
"res 4, [iy+0x%02x], l",
"res 4, [iy+0x%02x]",
"res 4, [iy+0x%02x], a",
"res 5, [iy+0x%02x], b",
"res 5, [iy+0x%02x], c",
"res 5, [iy+0x%02x], d",
"res 5, [iy+0x%02x], e",
"res 5, [iy+0x%02x], h",
"res 5, [iy+0x%02x], l",
"res 5, [iy+0x%02x]",
"res 5, [iy+0x%02x], a",
"res 6, [iy+0x%02x], b",
"res 6, [iy+0x%02x], c",
"res 6, [iy+0x%02x], d",
"res 6, [iy+0x%02x], e",
"res 6, [iy+0x%02x], h",
"res 6, [iy+0x%02x], l",
"res 6, [iy+0x%02x]",
"res 6, [iy+0x%02x], a",
"res 7, [iy+0x%02x], b",
"res 7, [iy+0x%02x], c",
"res 7, [iy+0x%02x], d",
"res 7, [iy+0x%02x], e",
"res 7, [iy+0x%02x], h",
"res 7, [iy+0x%02x], l",
"res 7, [iy+0x%02x]",
"res 7, [iy+0x%02x], a",
"set 0, [iy+0x%02x], b",
"set 0, [iy+0x%02x], c",
"set 0, [iy+0x%02x], d",
"set 0, [iy+0x%02x], e",
"set 0, [iy+0x%02x], h",
"set 0, [iy+0x%02x], l",
"set 0, [iy+0x%02x]",
"set 0, [iy+0x%02x], a",
"set 1, [iy+0x%02x], b",
"set 1, [iy+0x%02x], c",
"set 1, [iy+0x%02x], d",
"set 1, [iy+0x%02x], e",
"set 1, [iy+0x%02x], h",
"set 1, [iy+0x%02x], l",
"set 1, [iy+0x%02x]",
"set 1, [iy+0x%02x], a",
"set 2, [iy+0x%02x], b",
"set 2, [iy+0x%02x], c",
"set 2, [iy+0x%02x], d",
"set 2, [iy+0x%02x], e",
"set 2, [iy+0x%02x], h",
"set 2, [iy+0x%02x], l",
"set 2, [iy+0x%02x]",
"set 2, [iy+0x%02x], a",
"set 3, [iy+0x%02x], b",
"set 3, [iy+0x%02x], c",
"set 3, [iy+0x%02x], d",
"set 3, [iy+0x%02x], e",
"set 3, [iy+0x%02x], h",
"set 3, [iy+0x%02x], l",
"set 3, [iy+0x%02x]",
"set 3, [iy+0x%02x], a",
"set 4, [iy+0x%02x], b",
"set 4, [iy+0x%02x], c",
"set 4, [iy+0x%02x], d",
"set 4, [iy+0x%02x], e",
"set 4, [iy+0x%02x], h",
"set 4, [iy+0x%02x], l",
"set 4, [iy+0x%02x]",
"set 4, [iy+0x%02x], a",
"set 5, [iy+0x%02x], b",
"set 5, [iy+0x%02x], c",
"set 5, [iy+0x%02x], d",
"set 5, [iy+0x%02x], e",
"set 5, [iy+0x%02x], h",
"set 5, [iy+0x%02x], l",
"set 5, [iy+0x%02x]",
"set 5, [iy+0x%02x], a",
"set 6, [iy+0x%02x], b",
"set 6, [iy+0x%02x], c",
"set 6, [iy+0x%02x], d",
"set 6, [iy+0x%02x], e",
"set 6, [iy+0x%02x], h",
"set 6, [iy+0x%02x], l",
"set 6, [iy+0x%02x]",
"set 6, [iy+0x%02x], a",
"set 7, [iy+0x%02x], b",
"set 7, [iy+0x%02x], c",
"set 7, [iy+0x%02x], d",
"set 7, [iy+0x%02x], e",
"set 7, [iy+0x%02x], h",
"set 7, [iy+0x%02x], l",
"set 7, [iy+0x%02x]",
"set 7, [iy+0x%02x], a",
"%x\rinvalid", 
};


static z80_opcode dd[] = { 
{"add ix, bc", Z80_OP16 ,NULL},
{"add ix, de", Z80_OP16 ,NULL},
{"ld ix, 0x%04x", Z80_OP16^Z80_ARG16 ,NULL},
{"ld [0x%04x], ix", Z80_OP16^Z80_ARG16 ,NULL},
{"inc ix", Z80_OP16 ,NULL},
{"inc ixh", Z80_OP16 ,NULL},
{"dec ixh", Z80_OP16 ,NULL},
{"ld ixh, 0x%02x", Z80_OP16^Z80_ARG8 ,NULL},
{"add ix, ix", Z80_OP16 ,NULL},
{"ld ix, [0x%04x]", Z80_OP16^Z80_ARG16 ,NULL},
{"dec ix", Z80_OP16 ,NULL},
{"inc ixl", Z80_OP16 ,NULL},
{"dec ixl", Z80_OP16 ,NULL},
{"ld ixl, 0x%02x", Z80_OP16^Z80_ARG8 ,NULL},
{"inc [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"dec [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [ix+0x%02x], 0x%02x",Z80_OP16^Z80_ARG8^Z80_ARG16 ,NULL},
{"add ix, sp", Z80_OP16 ,NULL},
{"ld b, ixh", Z80_OP16 ,NULL},
{"ld b, ixl", Z80_OP16 ,NULL},
{"ld b, [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld c, ixh", Z80_OP16 ,NULL},
{"ld c, ixl", Z80_OP16 ,NULL},
{"ld c, [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld d, ixh", Z80_OP16 ,NULL},
{"ld d, ixl", Z80_OP16 ,NULL},
{"ld d, [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld e, ixh", Z80_OP16 ,NULL},
{"ld e, ixl", Z80_OP16 ,NULL},
{"ld e, [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld ixh, b", Z80_OP16 ,NULL},
{"ld ixh, c", Z80_OP16 ,NULL},
{"ld ixh, d", Z80_OP16 ,NULL},
{"ld ixh, e", Z80_OP16 ,NULL},
{"ld ixh, ixh", Z80_OP16 ,NULL},
{"ld ixh, ixl", Z80_OP16 ,NULL},
{"ld h, [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld ixh, a", Z80_OP16 ,NULL},
{"ld ixl, b", Z80_OP16 ,NULL},
{"ld ixl, c", Z80_OP16 ,NULL},
{"ld ixl, d", Z80_OP16 ,NULL},
{"ld ixl, e", Z80_OP16 ,NULL},
{"ld ixl, ixh", Z80_OP16 ,NULL},
{"ld ixl, ixl", Z80_OP16 ,NULL},
{"ld l, [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld ixl, a", Z80_OP16 ,NULL},
{"ld [ix+0x%02x], b", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [ix+0x%02x], c", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [ix+0x%02x], d", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [ix+0x%02x], e", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [ix+0x%02x], h", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [ix+0x%02x], l", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [ix+0x%02x], a", Z80_OP16^Z80_ARG8 ,NULL},
{"ld a, ixh", Z80_OP16 ,NULL},
{"ld a, ixl", Z80_OP16 ,NULL},
{"ld a, [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"add a, ixh", Z80_OP16 ,NULL},
{"add a, ixl", Z80_OP16 ,NULL},
{"add a, [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"adc a, ixh", Z80_OP16 ,NULL},
{"adc a, ixl", Z80_OP16 ,NULL},
{"adc a, [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"sub a, ixh", Z80_OP16 ,NULL},
{"sub a, ixl", Z80_OP16 ,NULL},
{"sub [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"sbc a, ixh", Z80_OP16 ,NULL},
{"sbc a, ixl", Z80_OP16 ,NULL},
{"sbc a, [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"and ixh", Z80_OP16 ,NULL},
{"and ixl", Z80_OP16 ,NULL},
{"and [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"xor ixh", Z80_OP16 ,NULL},
{"xor ixl", Z80_OP16 ,NULL},
{"xor [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"or ixh", Z80_OP16 ,NULL},
{"or ixl", Z80_OP16 ,NULL},
{"or [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"cp ixh", Z80_OP16 ,NULL},
{"cp ixl", Z80_OP16 ,NULL},
{"cp [ix+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"", Z80_OP24^Z80_ARG8 ,ddcb},
{"pop ix", Z80_OP16 ,NULL},
{"ex [sp], ix", Z80_OP16 ,NULL},
{"push ix", Z80_OP16 ,NULL},
{"jp [ix]", Z80_OP16 ,NULL},
{"ld sp, ix", Z80_OP16 ,NULL},
{"invalid", Z80_OP16 ,NULL}
};

static z80_opcode ed[]={ 
{"in b, [c]", Z80_OP16 ,NULL},
{"out [c], b", Z80_OP16 ,NULL},
{"sbc hl, bc", Z80_OP16 ,NULL},
{"ld [0x%04x], bc", Z80_OP16^Z80_ARG16 ,NULL},
{"neg", Z80_OP16 ,NULL},
{"retn", Z80_OP16 ,NULL},
{"im 0", Z80_OP16 ,NULL},
{"ld i, a", Z80_OP16 ,NULL},
{"in c, [c]", Z80_OP16 ,NULL},
{"out [c], c", Z80_OP16 ,NULL},
{"adc hl, bc", Z80_OP16 ,NULL},
{"ld bc, [0x%04x]", Z80_OP16^Z80_ARG16 ,NULL},
{"reti", Z80_OP16 ,NULL},
{"ld r, a", Z80_OP16 ,NULL},
{"in d, [c]", Z80_OP16 ,NULL},
{"out [c], d", Z80_OP16 ,NULL},
{"sbc hl, de", Z80_OP16 ,NULL},
{"ld [0x%04x], de", Z80_OP16^Z80_ARG16 ,NULL},
{"im 1", Z80_OP16 ,NULL},
{"ld a, i", Z80_OP16 ,NULL},
{"in e, [c]", Z80_OP16 ,NULL},
{"out [c], e", Z80_OP16 ,NULL},
{"adc hl, de", Z80_OP16 ,NULL},
{"ld de, [0x%04x]", Z80_OP16^Z80_ARG16 ,NULL},
{"im 2", Z80_OP16 ,NULL},
{"ld a, r", Z80_OP16 ,NULL},
{"in h, [c]", Z80_OP16 ,NULL},
{"out [c], h", Z80_OP16 ,NULL},
{"sbc hl, hl", Z80_OP16 ,NULL},
{"rrd", Z80_OP16 ,NULL},
{"in l, [c]", Z80_OP16 ,NULL},
{"out [c], l", Z80_OP16 ,NULL},
{"adc hl, hl", Z80_OP16 ,NULL},
{"rld", Z80_OP16 ,NULL},
{"in [c]", Z80_OP16 ,NULL},
{"out [c], 0", Z80_OP16 ,NULL},
{"sbc hl, sp", Z80_OP16 ,NULL},
{"ld [0x%04x], sp", Z80_OP16^Z80_ARG16 ,NULL},
{"in a, [c]", Z80_OP16 ,NULL},
{"out [c], a", Z80_OP16 ,NULL},
{"adc hl, sp", Z80_OP16 ,NULL},
{"ld sp, [0x%04x]", Z80_OP16^Z80_ARG16 ,NULL},
{"ldi", Z80_OP16 ,NULL},
{"cpi", Z80_OP16 ,NULL},
{"ini", Z80_OP16 ,NULL},
{"outi", Z80_OP16 ,NULL},
{"ldd", Z80_OP16 ,NULL},
{"cpd", Z80_OP16 ,NULL},
{"ind", Z80_OP16 ,NULL},
{"outd", Z80_OP16 ,NULL},
{"ldir", Z80_OP16 ,NULL},
{"cpir", Z80_OP16 ,NULL},
{"inir", Z80_OP16 ,NULL},
{"otir", Z80_OP16 ,NULL},
{"lddr", Z80_OP16 ,NULL},
{"cpdr", Z80_OP16 ,NULL},
{"indr", Z80_OP16 ,NULL},
{"otdr", Z80_OP16 ,NULL},
{"invalid", Z80_OP16 ,NULL},
{"invalid", Z80_OP16 ,NULL}
};

static z80_opcode fd[]={ 
{"add iy, bc", Z80_OP16 ,NULL},
{"add iy, de", Z80_OP16 ,NULL},
{"ld iy, 0x%04x", Z80_OP16^Z80_ARG16 ,NULL},
{"ld [0x%04x], iy", Z80_OP16^Z80_ARG16 ,NULL},
{"inc iy", Z80_OP16 ,NULL},
{"inc iyh", Z80_OP16 ,NULL},
{"dec iyh", Z80_OP16 ,NULL},
{"ld iyh, 0x%02x", Z80_OP16^Z80_ARG8 ,NULL},
{"add iy, iy", Z80_OP16 ,NULL},
{"ld iy, [0x%04x]", Z80_OP16^Z80_ARG16 ,NULL},
{"dec iy", Z80_OP16 ,NULL},
{"inc iyl", Z80_OP16 ,NULL},
{"dec iyl", Z80_OP16 ,NULL},
{"ld iyl, 0x%02x", Z80_OP16^Z80_ARG8 ,NULL},
{"inc [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"dec [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [iy+0x%02x], 0x%02x",Z80_OP16^Z80_ARG8^Z80_ARG16 ,NULL},
{"add iy, sp", Z80_OP16 ,NULL},
{"ld b, iyh", Z80_OP16 ,NULL},
{"ld b, iyl", Z80_OP16 ,NULL},
{"ld b, [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld c, iyh", Z80_OP16 ,NULL},
{"ld c, iyl", Z80_OP16 ,NULL},
{"ld c, [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld d, iyh", Z80_OP16 ,NULL},
{"ld d, iyl", Z80_OP16 ,NULL},
{"ld d, [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld e, iyh", Z80_OP16 ,NULL},
{"ld e, iyl", Z80_OP16 ,NULL},
{"ld e, [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld iyh, b", Z80_OP16 ,NULL},
{"ld iyh, c", Z80_OP16 ,NULL},
{"ld iyh, d", Z80_OP16 ,NULL},
{"ld iyh, e", Z80_OP16 ,NULL},
{"ld iyh, iyh", Z80_OP16 ,NULL},
{"ld iyh, iyl", Z80_OP16 ,NULL},
{"ld h, [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld iyh, a", Z80_OP16 ,NULL},
{"ld iyl, b", Z80_OP16 ,NULL},
{"ld iyl, c", Z80_OP16 ,NULL},
{"ld iyl, d", Z80_OP16 ,NULL},
{"ld iyl, e", Z80_OP16 ,NULL},
{"ld iyl, iyh", Z80_OP16 ,NULL},
{"ld iyl, iyl", Z80_OP16 ,NULL},
{"ld l, [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"ld iyl, a", Z80_OP16 ,NULL},
{"ld [iy+0x%02x], b", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [iy+0x%02x], c", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [iy+0x%02x], d", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [iy+0x%02x], e", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [iy+0x%02x], h", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [iy+0x%02x], l", Z80_OP16^Z80_ARG8 ,NULL},
{"ld [iy+0x%02x], a", Z80_OP16^Z80_ARG8 ,NULL},
{"ld a, iyh", Z80_OP16 ,NULL},
{"ld a, iyl", Z80_OP16 ,NULL},
{"ld a, [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"add a, iyh", Z80_OP16 ,NULL},
{"add a, iyl", Z80_OP16 ,NULL},
{"add a, [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"adc a, iyh", Z80_OP16 ,NULL},
{"adc a, iyl", Z80_OP16 ,NULL},
{"adc a, [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"sub iyh", Z80_OP16 ,NULL},
{"sub iyl", Z80_OP16 ,NULL},
{"sub [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"sbc a, iyh", Z80_OP16 ,NULL},
{"sbc a, iyl", Z80_OP16 ,NULL},
{"sbc a, [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"and iyh", Z80_OP16 ,NULL},
{"and iyl", Z80_OP16 ,NULL},
{"and [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"xor iyh", Z80_OP16 ,NULL},
{"xor iyl", Z80_OP16 ,NULL},
{"xor [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"or iyh", Z80_OP16 ,NULL},
{"or iyl", Z80_OP16 ,NULL},
{"or [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"cp iyh", Z80_OP16 ,NULL},
{"cp iyl", Z80_OP16 ,NULL},
{"cp [iy+0x%02x]", Z80_OP16^Z80_ARG8 ,NULL},
{"", Z80_OP24^Z80_ARG8 ,fdcb},
{"pop iy", Z80_OP16 ,NULL},
{"ex [sp], iy", Z80_OP16 ,NULL},
{"push iy", Z80_OP16 ,NULL},
{"jp [iy]", Z80_OP16 ,NULL},
{"ld sp, iy", Z80_OP16 ,NULL},
{"invalid", Z80_OP16 ,NULL}
};

static z80_opcode z80_op[] = {
{"nop", Z80_OP8 ,NULL},
{"ld bc, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"ld [bc], a", Z80_OP8 ,NULL},
{"inc bc", Z80_OP8 ,NULL},
{"inc b", Z80_OP8 ,NULL},
{"dec b", Z80_OP8 ,NULL},
{"ld b, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"rlca", Z80_OP8 ,NULL},
{"ex af, af'", Z80_OP8 ,NULL},
{"add hl, bc", Z80_OP8 ,NULL},
{"ld a, [bc]", Z80_OP8 ,NULL},
{"dec bc", Z80_OP8 ,NULL},
{"inc c", Z80_OP8 ,NULL},
{"dec c", Z80_OP8 ,NULL},
{"ld c, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"rrca", Z80_OP8 ,NULL},
{"djnz 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"ld de, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"ld [de], a", Z80_OP8 ,NULL},
{"inc de", Z80_OP8 ,NULL},
{"inc d", Z80_OP8 ,NULL},
{"dec d", Z80_OP8 ,NULL},
{"ld d, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"rla", Z80_OP8 ,NULL},
{"jr 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"add hl, de", Z80_OP8 ,NULL},
{"ld a, [de]", Z80_OP8 ,NULL},
{"dec de", Z80_OP8 ,NULL},
{"inc e", Z80_OP8 ,NULL},
{"dec e", Z80_OP8 ,NULL},
{"ld e, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"rra", Z80_OP8 ,NULL},
{"jr nz, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"ld hl, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"ld [0x%04x], hl", Z80_OP8^Z80_ARG16 ,NULL},
{"inc hl", Z80_OP8 ,NULL},
{"inc h", Z80_OP8 ,NULL},
{"dec h", Z80_OP8 ,NULL},
{"ld h, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"daa", Z80_OP8 ,NULL},
{"jr z, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"add hl, hl", Z80_OP8 ,NULL},
{"ld hl, [0x%04x]", Z80_OP8^Z80_ARG16 ,NULL},
{"dec hl", Z80_OP8 ,NULL},
{"inc l", Z80_OP8 ,NULL},
{"dec l", Z80_OP8 ,NULL},
{"ld l, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"cpl", Z80_OP8 ,NULL},
{"jr nc, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"ld sp, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"ld [0x%04x], a", Z80_OP8^Z80_ARG16 ,NULL},
{"inc sp", Z80_OP8 ,NULL},
{"inc [hl]", Z80_OP8 ,NULL},
{"dec [hl]", Z80_OP8 ,NULL},
{"ld [hl], 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"scf", Z80_OP8 ,NULL},
{"jr c, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"add hl, sp", Z80_OP8 ,NULL},
{"ld a, [0x%04x]", Z80_OP8^Z80_ARG16 ,NULL},
{"dec sp", Z80_OP8 ,NULL},
{"inc a", Z80_OP8 ,NULL},
{"dec a", Z80_OP8 ,NULL},
{"ld a, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"ccf", Z80_OP8 ,NULL},
{"ld b, b", Z80_OP8 ,NULL},
{"ld b, c", Z80_OP8 ,NULL},
{"ld b, d", Z80_OP8 ,NULL},
{"ld b, e", Z80_OP8 ,NULL},
{"ld b, h", Z80_OP8 ,NULL},
{"ld b, l", Z80_OP8 ,NULL},
{"ld b, [hl]", Z80_OP8 ,NULL},
{"ld b, a", Z80_OP8 ,NULL},
{"ld c, b", Z80_OP8 ,NULL},
{"ld c, c", Z80_OP8 ,NULL},
{"ld c, d", Z80_OP8 ,NULL},
{"ld c, e", Z80_OP8 ,NULL},
{"ld c, h", Z80_OP8 ,NULL},
{"ld c, l", Z80_OP8 ,NULL},
{"ld c, [hl]", Z80_OP8 ,NULL},
{"ld c, a", Z80_OP8 ,NULL},
{"ld d, b", Z80_OP8 ,NULL},
{"ld d, c", Z80_OP8 ,NULL},
{"ld d, d", Z80_OP8 ,NULL},
{"ld d, e", Z80_OP8 ,NULL},
{"ld d, h", Z80_OP8 ,NULL},
{"ld d, l", Z80_OP8 ,NULL},
{"ld d, [hl]", Z80_OP8 ,NULL},
{"ld d, a", Z80_OP8 ,NULL},
{"ld e, b", Z80_OP8 ,NULL},
{"ld e, c", Z80_OP8 ,NULL},
{"ld e, d", Z80_OP8 ,NULL},
{"ld e, e", Z80_OP8 ,NULL},
{"ld e, h", Z80_OP8 ,NULL},
{"ld e, l", Z80_OP8 ,NULL},
{"ld e, [hl]", Z80_OP8 ,NULL},
{"ld e, a", Z80_OP8 ,NULL},
{"ld h, b", Z80_OP8 ,NULL},
{"ld h, c", Z80_OP8 ,NULL},
{"ld h, d", Z80_OP8 ,NULL},
{"ld h, e", Z80_OP8 ,NULL},
{"ld h, h", Z80_OP8 ,NULL},
{"ld h, l", Z80_OP8 ,NULL},
{"ld h, [hl]", Z80_OP8 ,NULL},
{"ld h, a", Z80_OP8 ,NULL},
{"ld l, b", Z80_OP8 ,NULL},
{"ld l, c", Z80_OP8 ,NULL},
{"ld l, d", Z80_OP8 ,NULL},
{"ld l, e", Z80_OP8 ,NULL},
{"ld l, h", Z80_OP8 ,NULL},
{"ld l, l", Z80_OP8 ,NULL},
{"ld l, [hl]", Z80_OP8 ,NULL},
{"ld l, a", Z80_OP8 ,NULL},
{"ld [hl], b", Z80_OP8 ,NULL},
{"ld [hl], c", Z80_OP8 ,NULL},
{"ld [hl], d", Z80_OP8 ,NULL},
{"ld [hl], e", Z80_OP8 ,NULL},
{"ld [hl], h", Z80_OP8 ,NULL},
{"ld [hl], l", Z80_OP8 ,NULL},
{"halt", Z80_OP8 ,NULL},
{"ld [hl], a", Z80_OP8 ,NULL},
{"ld a, b", Z80_OP8 ,NULL},
{"ld a, c", Z80_OP8 ,NULL},
{"ld a, d", Z80_OP8 ,NULL},
{"ld a, e", Z80_OP8 ,NULL},
{"ld a, h", Z80_OP8 ,NULL},
{"ld a, l", Z80_OP8 ,NULL},
{"ld a, [hl]", Z80_OP8 ,NULL},
{"ld a, a", Z80_OP8 ,NULL},
{"add a, b", Z80_OP8 ,NULL},
{"add a, c", Z80_OP8 ,NULL},
{"add a, d", Z80_OP8 ,NULL},
{"add a, e", Z80_OP8 ,NULL},
{"add a, h", Z80_OP8 ,NULL},
{"add a, l", Z80_OP8 ,NULL},
{"add a, [hl]", Z80_OP8 ,NULL},
{"add a, a", Z80_OP8 ,NULL},
{"adc a, b", Z80_OP8 ,NULL},
{"adc a, c", Z80_OP8 ,NULL},
{"adc a, d", Z80_OP8 ,NULL},
{"adc a, e", Z80_OP8 ,NULL},
{"adc a, h", Z80_OP8 ,NULL},
{"adc a, l", Z80_OP8 ,NULL},
{"adc a, [hl]", Z80_OP8 ,NULL},
{"adc a, a", Z80_OP8 ,NULL},
{"sub b", Z80_OP8 ,NULL},
{"sub c", Z80_OP8 ,NULL},
{"sub d", Z80_OP8 ,NULL},
{"sub e", Z80_OP8 ,NULL},
{"sub h", Z80_OP8 ,NULL},
{"sub l", Z80_OP8 ,NULL},
{"sub [hl]", Z80_OP8 ,NULL},
{"sub a", Z80_OP8 ,NULL},
{"sbc a, b", Z80_OP8 ,NULL},
{"sbc a, c", Z80_OP8 ,NULL},
{"sbc a, d", Z80_OP8 ,NULL},
{"sbc a, e", Z80_OP8 ,NULL},
{"sbc a, h", Z80_OP8 ,NULL},
{"sbc a, l", Z80_OP8 ,NULL},
{"sbc a, [hl]", Z80_OP8 ,NULL},
{"sbc a, a", Z80_OP8 ,NULL},
{"and b", Z80_OP8 ,NULL},
{"and c", Z80_OP8 ,NULL},
{"and d", Z80_OP8 ,NULL},
{"and e", Z80_OP8 ,NULL},
{"and h", Z80_OP8 ,NULL},
{"and l", Z80_OP8 ,NULL},
{"and [hl]", Z80_OP8 ,NULL},
{"and a", Z80_OP8 ,NULL},
{"xor b", Z80_OP8 ,NULL},
{"xor c", Z80_OP8 ,NULL},
{"xor d", Z80_OP8 ,NULL},
{"xor e", Z80_OP8 ,NULL},
{"xor h", Z80_OP8 ,NULL},
{"xor l", Z80_OP8 ,NULL},
{"xor [hl]", Z80_OP8 ,NULL},
{"xor a", Z80_OP8 ,NULL},
{"or b", Z80_OP8 ,NULL},
{"or c", Z80_OP8 ,NULL},
{"or d", Z80_OP8 ,NULL},
{"or e", Z80_OP8 ,NULL},
{"or h", Z80_OP8 ,NULL},
{"or l", Z80_OP8 ,NULL},
{"or [hl]", Z80_OP8 ,NULL},
{"or a", Z80_OP8 ,NULL},
{"cp b", Z80_OP8 ,NULL},
{"cp c", Z80_OP8 ,NULL},
{"cp d", Z80_OP8 ,NULL},
{"cp e", Z80_OP8 ,NULL},
{"cp h", Z80_OP8 ,NULL},
{"cp l", Z80_OP8 ,NULL},
{"cp [hl]", Z80_OP8 ,NULL},
{"cp a", Z80_OP8 ,NULL},
{"ret nz", Z80_OP8 ,NULL},
{"pop bc", Z80_OP8 ,NULL},
{"jp nz, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"jp 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"call nz, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"push bc", Z80_OP8 ,NULL},
{"add a, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"rst 0x00", Z80_OP8 ,NULL},
{"ret z", Z80_OP8 ,NULL},
{"ret", Z80_OP8 ,NULL},
{"jp z, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"", Z80_OP16 ,cb},
{"call z, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"call 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"adc a, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"rst 0x08", Z80_OP8 ,NULL},
{"ret nc", Z80_OP8 ,NULL},
{"pop de", Z80_OP8 ,NULL},
{"jp nc, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"out [0x%02x], a", Z80_OP8^Z80_ARG8 ,NULL},
{"call nc, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"push de", Z80_OP8 ,NULL},
{"sub 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"rst 0x10", Z80_OP8 ,NULL},
{"ret c", Z80_OP8 ,NULL},
{"exx", Z80_OP8 ,NULL},
{"jp c, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"in a, [0x%02x]", Z80_OP8^Z80_ARG8 ,NULL},
{"call c, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"", Z80_OP_UNK^Z80_ENC0 ,dd},
{"sbc a, 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"rst 0x18", Z80_OP8 ,NULL},
{"ret po", Z80_OP8 ,NULL},
{"pop hl", Z80_OP8 ,NULL},
{"jp po, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"ex [sp], hl", Z80_OP8 ,NULL},
{"call po, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"push hl", Z80_OP8 ,NULL},
{"and 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"rst 0x20", Z80_OP8 ,NULL},
{"ret pe", Z80_OP8 ,NULL},
{"jp [hl]", Z80_OP8 ,NULL},
{"jp pe, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"ex de, hl", Z80_OP8 ,NULL},
{"call pe, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"", Z80_OP_UNK^Z80_ENC1 ,ed},
{"xor 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"rst 0x28", Z80_OP8 ,NULL},
{"ret p", Z80_OP8 ,NULL},
{"pop af", Z80_OP8 ,NULL},
{"jp p, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"di", Z80_OP8 ,NULL},
{"call p, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"push af", Z80_OP8 ,NULL},
{"or 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"rst 0x30", Z80_OP8 ,NULL},
{"ret m", Z80_OP8 ,NULL},
{"ld sp, hl", Z80_OP8 ,NULL},
{"jp m, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"ei", Z80_OP8 ,NULL},
{"call m, 0x%04x", Z80_OP8^Z80_ARG16 ,NULL},
{"", Z80_OP_UNK^Z80_ENC0 ,fd},
{"cp 0x%02x", Z80_OP8^Z80_ARG8 ,NULL},
{"rst 0x38", Z80_OP8 ,NULL},
};

#endif
