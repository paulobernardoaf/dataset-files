#include "avformat.h"
#include "metadata.h"
int64_t ff_vorbiscomment_length(AVDictionary *m, const char *vendor_string,
AVChapter **chapters, unsigned int nb_chapters);
int ff_vorbiscomment_write(uint8_t **p, AVDictionary **m,
const char *vendor_string,
AVChapter **chapters, unsigned int nb_chapters);
extern const AVMetadataConv ff_vorbiscomment_metadata_conv[];
