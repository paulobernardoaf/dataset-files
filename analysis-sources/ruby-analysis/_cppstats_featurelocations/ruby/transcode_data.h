










#include "ruby/ruby.h"

#if !defined(RUBY_TRANSCODE_DATA_H)
#define RUBY_TRANSCODE_DATA_H 1

RUBY_SYMBOL_EXPORT_BEGIN

#define WORDINDEX_SHIFT_BITS 2
#define WORDINDEX2INFO(widx) ((widx) << WORDINDEX_SHIFT_BITS)
#define INFO2WORDINDEX(info) ((info) >> WORDINDEX_SHIFT_BITS)
#define BYTE_LOOKUP_BASE(bl) ((bl)[0])
#define BYTE_LOOKUP_INFO(bl) ((bl)[1])

#define PType (unsigned int)

#define NOMAP (PType 0x01) 
#define ONEbt (0x02) 
#define TWObt (0x03) 
#define THREEbt (0x05) 
#define FOURbt (0x06) 
#define INVALID (PType 0x07) 
#define UNDEF (PType 0x09) 
#define ZERObt (PType 0x0A) 
#define FUNii (PType 0x0B) 
#define FUNsi (PType 0x0D) 
#define FUNio (PType 0x0E) 
#define FUNso (PType 0x0F) 
#define STR1 (PType 0x11) 
#define GB4bt (PType 0x12) 
#define FUNsio (PType 0x13) 

#define STR1_LENGTH(byte_addr) (unsigned int)(*(byte_addr) + 4)
#define STR1_BYTEINDEX(w) ((w) >> 6)
#define makeSTR1(bi) (((bi) << 6) | STR1)
#define makeSTR1LEN(len) ((len)-4)

#define o1(b1) (PType((((unsigned char)(b1))<<8)|ONEbt))
#define o2(b1,b2) (PType((((unsigned char)(b1))<<8)|(((unsigned char)(b2))<<16)|TWObt))


#define o3(b1,b2,b3) (PType(((((unsigned char)(b1))<<8)|(((unsigned char)(b2))<<16)|(((unsigned int)(unsigned char)(b3))<<24)|THREEbt)&0xffffffffU))




#define o4(b0,b1,b2,b3) (PType(((((unsigned char)(b1))<<8)|(((unsigned char)(b2))<<16)|(((unsigned int)(unsigned char)(b3))<<24)|((((unsigned char)(b0))&0x07)<<5)|FOURbt)&0xffffffffU))





#define g4(b0,b1,b2,b3) (PType(((((unsigned char)(b0))<<8)|(((unsigned char)(b2))<<16)|((((unsigned char)(b1))&0x0f)<<24)|((((unsigned int)(unsigned char)(b3))&0x0f)<<28)|GB4bt)&0xffffffffU))





#define funsio(diff) (PType((((unsigned int)(diff))<<8)|FUNsio))

#define getBT1(a) ((unsigned char)((a)>> 8))
#define getBT2(a) ((unsigned char)((a)>>16))
#define getBT3(a) ((unsigned char)((a)>>24))
#define getBT0(a) (((unsigned char)((a)>> 5)&0x07)|0xF0) 

#define getGB4bt0(a) ((unsigned char)((a)>> 8))
#define getGB4bt1(a) (((unsigned char)((a)>>24)&0x0F)|0x30)
#define getGB4bt2(a) ((unsigned char)((a)>>16))
#define getGB4bt3(a) (((unsigned char)((a)>>28)&0x0F)|0x30)

#define o2FUNii(b1,b2) (PType((((unsigned char)(b1))<<8)|(((unsigned char)(b2))<<16)|FUNii))


#define ONETRAIL 
#define TWOTRAIL 
#define THREETRAIL 

typedef enum {
asciicompat_converter, 
asciicompat_decoder, 
asciicompat_encoder 

} rb_transcoder_asciicompat_type_t;

typedef struct rb_transcoder rb_transcoder;


struct rb_transcoder {
const char *src_encoding;
const char *dst_encoding;
unsigned int conv_tree_start;
const unsigned char *byte_array;
unsigned int byte_array_length;
const unsigned int *word_array;
unsigned int word_array_length;
int word_size;
int input_unit_length;
int max_input;
int max_output;
rb_transcoder_asciicompat_type_t asciicompat_type;
size_t state_size;
int (*state_init_func)(void*); 
int (*state_fini_func)(void*); 
VALUE (*func_ii)(void*, VALUE); 
VALUE (*func_si)(void*, const unsigned char*, size_t); 
ssize_t (*func_io)(void*, VALUE, const unsigned char*, size_t); 
ssize_t (*func_so)(void*, const unsigned char*, size_t, unsigned char*, size_t); 
ssize_t (*finish_func)(void*, unsigned char*, size_t); 
ssize_t (*resetsize_func)(void*); 
ssize_t (*resetstate_func)(void*, unsigned char*, size_t); 
ssize_t (*func_sio)(void*, const unsigned char*, size_t, VALUE, unsigned char*, size_t); 
};

void rb_declare_transcoder(const char *enc1, const char *enc2, const char *lib);
void rb_register_transcoder(const rb_transcoder *);





#if defined(EXTSTATIC) && EXTSTATIC
#define TRANS_INIT(name) void Init_trans_ ##name(void)
#else
#define TRANS_INIT(name) void Init_ ##name(void)
#endif

RUBY_SYMBOL_EXPORT_END

#endif 
