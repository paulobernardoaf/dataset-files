#if !defined(NE_SPECS_H)
#define NE_SPECS_H

enum {
LOBYTE = 0,
SEL_16 = 2,
POI_32 = 3,
OFF_16 = 5,
POI_48 = 11,
OFF_32 = 13
};

enum {
INTERNAL_REF = 0,
IMPORTED_ORD = 1,
IMPORTED_NAME = 2,
OSFIXUP = 3,
ADDITIVE = 4
};

typedef struct _RELOC {
ut8 type;
ut8 flags;
ut16 offset;
union {
ut16 index;
struct { 
ut8 segnum;
ut8 zero;
ut16 segoff;
};
struct { 
ut16 ignore;
ut16 entry_ordinal;
};
struct { 
ut16 align1;
ut16 func_ord;
};
struct { 
ut16 align2;
ut16 name_off;
};
};
} NE_image_reloc_item;

enum {
IS_DATA = 1,
IS_MOVEABLE = 0x10,
IS_SHAREABLE = 0x20,
IS_PRELOAD = 0x40,
RELOCINFO = 0x100,
IS_RX = 0x1000
};

enum {
NOAUTODATA = 0,
SINGLEDATA = 1,
LINKERROR = 0x2000,
LIBRARY = 0x8000
};

typedef struct _SEGMENT {
ut16 offset; 
ut16 length; 
ut16 flags; 
ut16 minAllocSz; 
} NE_image_segment_entry;

typedef struct _NAMEINFO {
ut16 rnOffset;
ut16 rnLength;
ut16 rnFlags;
ut16 rnID;
ut16 rnHandle;
ut16 rnUsage;
} NE_image_nameinfo_entry;

typedef struct _TYPEINFO {
ut16 rtTypeID;
ut16 rtResourceCount;
ut32 rtReserved;
NE_image_nameinfo_entry rtNameInfo[];
} NE_image_typeinfo_entry;

typedef struct {
char sig[2]; 
ut8 MajLinkerVersion; 
ut8 MinLinkerVersion; 
ut16 EntryTableOffset; 
ut16 EntryTableLength; 
ut32 FileLoadCRC; 
ut8 ProgFlags; 
ut8 ApplFlags; 
ut8 AutoDataSegIndex; 
ut16 InitHeapSize; 
ut16 InitStackSize; 
ut16 ipEntryPoint; 
ut16 csEntryPoint; 
ut32 InitStack; 
ut16 SegCount; 
ut16 ModRefs; 
ut16 NoResNamesTabSiz; 
ut16 SegTableOffset; 
ut16 ResTableOffset; 
ut16 ResidNamTable; 
ut16 ModRefTable; 
ut16 ImportNameTable; 
ut32 OffStartNonResTab; 
ut16 MovEntryCount; 
ut16 FileAlnSzShftCnt; 
ut16 nResTabEntries; 
ut8 targOS; 
ut8 OS2EXEFlags; 
ut16 retThunkOffset; 
ut16 segrefthunksoff; 
ut16 mincodeswap; 
ut8 expctwinver[2]; 
} NE_image_header;

#endif