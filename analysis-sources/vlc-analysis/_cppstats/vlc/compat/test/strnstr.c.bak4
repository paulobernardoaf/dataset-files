





















#include "config.h"

#include <stdbool.h>
#undef NDEBUG
#include <assert.h>
#include <string.h>

const char* haystack = "Lorem ipsum dolor sit amet";

static void test( const char* haystack, const char* needle, size_t len, bool res )
{
if ( len == 0 )
len = strlen( haystack );
char* str = strnstr( haystack, needle, len );
assert( res == ( str != NULL ) );
}

int main(void)
{
test( haystack, "Lorem", 0, true );
test( haystack, "Sea Otters", 0, false );
test( haystack, "", 0, true );
test( haystack, "Lorem ipsum dolor sit amet, but bigger", 0, false );
test( haystack, haystack, 0, true );
test( haystack, "amet", 0, true );
test( haystack, "dolor", 5, false );
}
