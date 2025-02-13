#include <ft2build.h>
#include "ft2_err.h"
#define FT_NOERRORDEF_( sym, num, str ) num,
#define FT_ERRORDEF_( sym, num, str ) num,
static const unsigned short ft2_errorindex[] =
{
#include FT_ERROR_DEFINITIONS_H
};
#undef FT_NOERRORDEF_
#undef FT_ERRORDEF_
#define FT_NOERRORDEF_( sym, num, str ) str,
#define FT_ERRORDEF_( sym, num, str ) str,
static const char *ft2_errorstrings[] =
{
#include FT_ERROR_DEFINITIONS_H
};
#undef FT_NOERRORDEF_
#undef FT_ERRORDEF_
enum { ft2_num_errors = sizeof(ft2_errorindex)/sizeof(*ft2_errorindex) };
const char *ft2_strerror(unsigned err)
{
unsigned i;
for( i=0; i<ft2_num_errors; ++i )
if( err==ft2_errorindex[i] )
break;
return i<ft2_num_errors ? ft2_errorstrings[i] :
"An error freetype2 neglected to specify";
}
