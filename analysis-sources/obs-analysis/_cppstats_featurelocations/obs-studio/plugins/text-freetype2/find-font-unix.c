















#include <fontconfig/fontconfig.h>

#include <util/base.h>
#include <util/dstr.h>

#include "find-font.h"
#include "text-freetype2.h"

void free_os_font_list(void) {}

bool load_cached_os_font_list(void)
{
return true;
}

void load_os_font_list(void) {}

const char *get_font_path(const char *family, uint16_t size, const char *style,
uint32_t flags, FT_Long *idx)
{
bool bold = !!(flags & OBS_FONT_BOLD);
bool italic = !!(flags & OBS_FONT_ITALIC);
FcPattern *pattern = FcPatternCreate();
FcPattern *match = NULL;
bool success = false;
FcResult match_result;


static __thread char result[512];

FcPatternAddString(pattern, FC_FAMILY, (const FcChar8 *)family);
FcPatternAddString(pattern, FC_STYLE, (const FcChar8 *)style);
FcPatternAddInteger(pattern, FC_WEIGHT,
bold ? FC_WEIGHT_BOLD : FC_WEIGHT_REGULAR);
FcPatternAddInteger(pattern, FC_SLANT,
italic ? FC_SLANT_ITALIC : FC_SLANT_ROMAN);
FcPatternAddDouble(pattern, FC_SIZE, (double)size);

FcConfigSubstitute(NULL, pattern, FcMatchPattern);
FcDefaultSubstitute(pattern);

match = FcFontMatch(NULL, pattern, &match_result);
if (match) {
FcChar8 *path =
FcPatternFormat(match, (const FcChar8 *)"%{file}");
strncpy(result, (char *)path, 511);
FcStrFree(path);

int fc_index = 0;
FcPatternGetInteger(match, FC_INDEX, 1, &fc_index);
*idx = (FT_Long)fc_index;

FcPatternDestroy(match);
success = true;
} else {
blog(LOG_WARNING, "no matching font for '%s' found", family);
}

FcPatternDestroy(pattern);
return success ? &result[0] : NULL;
}
