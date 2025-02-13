#if !defined(TAR_H)
#define TAR_H

#define TYPEFLAG_AUTO '\0'
#define TYPEFLAG_REG '0'
#define TYPEFLAG_LNK '2'
#define TYPEFLAG_DIR '5'
#define TYPEFLAG_GLOBAL_HEADER 'g'
#define TYPEFLAG_EXT_HEADER 'x'

struct ustar_header {
char name[100]; 
char mode[8]; 
char uid[8]; 
char gid[8]; 
char size[12]; 
char mtime[12]; 
char chksum[8]; 
char typeflag[1]; 
char linkname[100]; 
char magic[6]; 
char version[2]; 
char uname[32]; 
char gname[32]; 
char devmajor[8]; 
char devminor[8]; 
char prefix[155]; 
};

#endif 
