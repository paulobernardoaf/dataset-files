#if !defined(LE_SPECS_H)
#define LE_SPECS_H
#include <r_types.h>

typedef enum {
UNUSED_ENTRY = 0,
ENTRY16,
CALLGATE,
ENTRY32,
FORWARDER,
} LE_entry_bundle_type;

typedef enum {
LE_RT_POINTER = 1, 
LE_RT_BITMAP = 2, 
LE_RT_MENU = 3, 
LE_RT_DIALOG = 4, 
LE_RT_STRING = 5, 
LE_RT_FONTDIR = 6, 
LE_RT_FONT = 7, 
LE_RT_ACCELTABLE = 8, 
LE_RT_RCDATA = 9, 
LE_RT_MESSAGE = 10, 
LE_RT_DLGINCLUDE = 11, 
LE_RT_VKEYTBL = 12, 
LE_RT_KEYTBL = 13, 
LE_RT_CHARTBL = 14, 
LE_RT_DISPLAYINFO = 15, 
LE_RT_FKASHORT = 16, 
LE_RT_FKALONG = 17, 
LE_RT_HELPTABLE = 18, 
LE_RT_HELPSUBTABLE = 19, 
LE_RT_FDDIR = 20, 
LE_RT_FD = 21, 
} LE_resource_type;



#define ENTRY_PARAMETER_TYPING_PRESENT 0x80

typedef struct LE_entry_bundle_header_s {
ut8 count;
ut8 type; 
ut16 objnum; 
} LE_entry_bundle_header;

#define ENTRY_EXPORTED 0x01
#define ENTRY_PARAM_COUNT_MASK 0xF8

typedef R_PACKED (union LE_entry_bundle_entry_u {
R_PACKED (struct {
ut8 flags; 
ut16 offset; 
}) entry_16;
R_PACKED (struct {
ut8 flags; 
ut16 offset; 
ut16 callgate_sel; 
}) callgate;
R_PACKED (struct {
ut8 flags; 
ut32 offset; 
}) entry_32;
R_PACKED (struct {
ut8 flags; 
ut16 import_ord; 
ut32 offset; 
}) forwarder;
}) LE_entry_bundle_entry;


#define F_SOURCE_TYPE_MASK 0xF
#define F_SOURCE_ALIAS 0x10
#define F_SOURCE_LIST 0x20

typedef enum {
BYTEFIXUP,
UNDEFINED1,
SELECTOR16,
POINTER32, 
UNDEFINED2,
OFFSET16,
POINTER48, 
OFFSET32,
SELFOFFSET32,
} LE_fixup_source_type;

#define F_TARGET_TYPE_MASK 0x3
#define F_TARGET_ADDITIVE 0x4
#define F_TARGET_CHAIN 0x8
#define F_TARGET_OFF32 0x10 
#define F_TARGET_ADD32 0x20 
#define F_TARGET_ORD16 0x40 
#define F_TARGET_ORD8 0x80 

typedef enum {
INTERNAL,
IMPORTORD,
IMPORTNAME,
INTERNALENTRY
} LE_fixup_record_type;

typedef struct LE_fixup_record_header_s {
ut8 source;
ut8 target;
} LE_fixup_record_header;

#define O_READABLE 1
#define O_WRITABLE 1 << 1
#define O_EXECUTABLE 1 << 2
#define O_RESOURCE 1 << 3
#define O_DISCARTABLE 1 << 4
#define O_SHARED 1 << 5
#define O_PRELOAD 1 << 6
#define O_INVALID 1 << 7
#define O_ZEROED 1 << 8
#define O_RESIDENT 1 << 9
#define O_CONTIGUOUS O_RESIDENT | O_ZEROED
#define O_LOCKABLE 1 << 10
#define O_RESERVED 1 << 11
#define O_ALIASED 1 << 12
#define O_BIG_BIT 1 << 13
#define O_CODE 1 << 14
#define O_IO_PRIV 1 << 15

typedef struct LE_object_entry_s {
ut32 virtual_size;
ut32 reloc_base_addr;
ut32 flags;
ut32 page_tbl_idx; 
ut32 page_tbl_entries;
ut32 reserved;
} LE_object_entry;

#define P_LEGAL 0
#define P_ITERATED 1
#define P_INVALID 2
#define P_ZEROED 3
#define P_RANGE 4
#define P_COMPRESSED 5

typedef struct LE_object_page_entry_s {
ut32 offset; 
ut16 size;
ut16 flags;
} LE_object_page_entry;

#define M_PP_LIB_INIT 1 << 2
#define M_SYS_DLL 1 << 3 
#define M_INTERNAL_FIXUP 1 << 4
#define M_EXTERNAL_FIXUP 1 << 5
#define M_PM_WINDOWING_INCOMP 1 << 8 
#define M_PM_WINDOWING_COMPAT 1 << 9
#define M_USES_PM_WINDOWING M_PM_WINDOWING_INCOMP | M_PM_WINDOWING_COMPAT
#define M_NOT_LOADABLE 1 << 13
#define M_TYPE_MASK 0x38000
#define M_TYPE_EXE 0
#define M_TYPE_DLL 0x08000
#define M_TYPE_PM_DLL 0x10000
#define M_TYPE_PDD 0x20000 
#define M_TYPE_VDD 0x28000 
#define M_MP_UNSAFE 1 << 19
#define M_PP_LIB_TERM 1 << 30

typedef struct LE_image_header_s { 
ut8 magic[2]; 
ut8 border; 
ut8 worder; 
ut32 level; 
ut16 cpu; 
ut16 os; 
ut32 ver; 
ut32 mflags; 
ut32 mpages; 
ut32 startobj; 
ut32 eip; 
ut32 stackobj; 
ut32 esp; 
ut32 pagesize; 
ut32 pageshift; 
ut32 fixupsize; 
ut32 fixupsum; 
ut32 ldrsize; 
ut32 ldrsum; 
ut32 objtab; 
ut32 objcnt; 
ut32 objmap; 
ut32 itermap; 
ut32 rsrctab; 
ut32 rsrccnt; 
ut32 restab; 
ut32 enttab; 
ut32 dirtab; 
ut32 dircnt; 
ut32 fpagetab; 
ut32 frectab; 
ut32 impmod; 
ut32 impmodcnt; 
ut32 impproc; 
ut32 pagesum; 
ut32 datapage; 
ut32 preload; 
ut32 nrestab; 
ut32 cbnrestab; 
ut32 nressum; 
ut32 autodata; 
ut32 debuginfo; 
ut32 debuglen; 
ut32 instpreload; 
ut32 instdemand; 
ut32 heapsize; 
ut32 stacksize; 
} LE_image_header;
#endif
