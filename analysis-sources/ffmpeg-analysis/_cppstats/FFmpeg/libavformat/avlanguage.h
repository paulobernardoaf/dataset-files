#include "libavutil/attributes.h"
#include "libavformat/version.h"
enum AVLangCodespace {
AV_LANG_ISO639_2_BIBL, 
AV_LANG_ISO639_2_TERM, 
AV_LANG_ISO639_1 
};
const char *ff_convert_lang_to(const char *lang, enum AVLangCodespace target_codespace);
#if LIBAVFORMAT_VERSION_MAJOR < 58
attribute_deprecated
const char *av_convert_lang_to(const char *lang, enum AVLangCodespace target_codespace);
#endif
