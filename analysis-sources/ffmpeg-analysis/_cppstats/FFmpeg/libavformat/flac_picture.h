#include "avformat.h"
#define RETURN_ERROR(code) do { ret = (code); goto fail; } while (0)
int ff_flac_parse_picture(AVFormatContext *s, uint8_t *buf, int buf_size);
