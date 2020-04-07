











#if defined(WORDS_BIGENDIAN)
#define A(x) ((x) >> 24)
#define B(x) (((x) >> 16) & 0xFF)
#define C(x) (((x) >> 8) & 0xFF)
#define D(x) ((x) & 0xFF)

#define S8(x) ((x) << 8)
#define S32(x) ((x) << 32)

#else
#define A(x) ((x) & 0xFF)
#define B(x) (((x) >> 8) & 0xFF)
#define C(x) (((x) >> 16) & 0xFF)
#define D(x) ((x) >> 24)

#define S8(x) ((x) >> 8)
#define S32(x) ((x) >> 32)
#endif
