#if !defined(VARINT_H)
#define VARINT_H

int encode_varint(uintmax_t, unsigned char *);
uintmax_t decode_varint(const unsigned char **);

#endif 
