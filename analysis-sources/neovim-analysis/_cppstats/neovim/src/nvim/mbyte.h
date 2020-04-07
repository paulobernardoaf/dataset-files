#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nvim/iconv.h"
#include "nvim/func_attr.h"
#include "nvim/os/os_defs.h" 
#include "nvim/types.h" 
#define MB_BYTE2LEN(b) utf8len_tab[b]
#define MB_BYTE2LEN_CHECK(b) (((b) < 0 || (b) > 255) ? 1 : utf8len_tab[b])
#define MB_MAXCHAR 6
#define ENC_8BIT 0x01
#define ENC_DBCS 0x02
#define ENC_UNICODE 0x04
#define ENC_ENDIAN_B 0x10 
#define ENC_ENDIAN_L 0x20 
#define ENC_2BYTE 0x40 
#define ENC_4BYTE 0x80 
#define ENC_2WORD 0x100 
#define ENC_LATIN1 0x200 
#define ENC_LATIN9 0x400 
#define ENC_MACROMAN 0x800 
#define mb_ptr2len utfc_ptr2len
#define mb_char2len utf_char2len
#define mb_char2cells utf_char2cells
typedef enum {
CONV_NONE = 0,
CONV_TO_UTF8 = 1,
CONV_9_TO_UTF8 = 2,
CONV_TO_LATIN1 = 3,
CONV_TO_LATIN9 = 4,
CONV_ICONV = 5,
} ConvFlags;
#define MBYTE_NONE_CONV { .vc_type = CONV_NONE, .vc_factor = 1, .vc_fail = false, }
typedef struct {
int vc_type; 
int vc_factor; 
#if defined(HAVE_ICONV)
iconv_t vc_fd; 
#endif
bool vc_fail; 
} vimconv_T;
extern const uint8_t utf8len_tab_zero[256];
extern const uint8_t utf8len_tab[256];
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "mbyte.h.generated.h"
#endif
static inline int mb_strcmp_ic(bool ic, const char *s1, const char *s2)
REAL_FATTR_NONNULL_ALL REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT;
static inline int mb_strcmp_ic(bool ic, const char *s1, const char *s2)
{
return (ic ? mb_stricmp(s1, s2) : strcmp(s1, s2));
}
