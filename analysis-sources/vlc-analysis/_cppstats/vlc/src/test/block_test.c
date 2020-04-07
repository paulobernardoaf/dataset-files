#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#undef NDEBUG
#include <assert.h>
#include <vlc_common.h>
#include <vlc_block.h>
static const char text[] =
"This is a test!\n"
"This file can be deleted safely!\n";
static void test_block_File(bool write)
{
FILE *stream;
int res;
stream = fopen ("testfile.txt", "wb+e");
assert (stream != NULL);
res = fputs (text, stream);
assert (res != EOF);
res = fflush (stream);
assert (res != EOF);
block_t *block = block_File(fileno(stream), write);
fclose (stream);
assert (block != NULL);
assert (block->i_buffer == strlen (text));
assert (!memcmp (block->p_buffer, text, block->i_buffer));
if (write)
memset(block->p_buffer, 'A', block->i_buffer);
block_Release (block);
remove ("testfile.txt");
}
static void test_block (void)
{
block_t *block = block_Alloc (sizeof (text));
assert (block != NULL);
memcpy (block->p_buffer, text, sizeof (text));
block = block_Realloc (block, 0, sizeof (text));
assert (block != NULL);
assert (block->i_buffer == sizeof (text));
assert (!memcmp (block->p_buffer, text, sizeof (text)));
block = block_Realloc (block, 200, sizeof (text) + 200);
assert (block != NULL);
assert (block->i_buffer == 200 + sizeof (text) + 200);
assert (!memcmp (block->p_buffer + 200, text, sizeof (text)));
block = block_Realloc (block, -200, sizeof (text) + 200);
assert (block != NULL);
assert (block->i_buffer == sizeof (text));
assert (!memcmp (block->p_buffer, text, sizeof (text)));
block_Release (block);
}
int main (void)
{
test_block_File(false);
test_block_File(true);
test_block ();
return 0;
}
