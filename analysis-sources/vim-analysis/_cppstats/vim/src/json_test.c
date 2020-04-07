#undef NDEBUG
#include <assert.h>
#define NO_VIM_MAIN
#include "main.c"
#include "json.c"
#if defined(FEAT_EVAL)
static void
test_decode_find_end(void)
{
js_read_T reader;
reader.js_fill = NULL;
reader.js_used = 0;
reader.js_buf = (char_u *)"\"hello\"";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)" \"hello\" ";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)"\"hello";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"123";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)"-";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"false";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)"f";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"fa";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"fal";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"fals";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"true";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)"t";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"tr";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"tru";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"null";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)"n";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"nu";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"nul";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"{\"a\":123}";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)"{\"a\":123";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"{\"a\":";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"{\"a\"";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"{\"a";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"{\"";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"{";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)" { \"a\" : 123 } ";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)" { \"a\" : 123 ";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)" { \"a\" : ";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)" { \"a\" ";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)" { \"a ";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)" { ";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)" { a : 123 } ";
assert(json_find_end(&reader, JSON_JS) == OK);
reader.js_buf = (char_u *)" { a : ";
assert(json_find_end(&reader, JSON_JS) == MAYBE);
reader.js_buf = (char_u *)"[\"a\",123]";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)"[\"a\",123";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"[\"a\",";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"[\"a\"";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"[\"a";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"[\"";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)"[";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)" [ \"a\" , 123 ] ";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)" [ \"a\" , 123 ";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)" [ \"a\" , ";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)" [ \"a\" ";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)" [ \"a ";
assert(json_find_end(&reader, 0) == MAYBE);
reader.js_buf = (char_u *)" [ ";
assert(json_find_end(&reader, 0) == MAYBE);
}
static int
fill_from_cookie(js_read_T *reader)
{
reader->js_buf = reader->js_cookie;
return TRUE;
}
static void
test_fill_called_on_find_end(void)
{
js_read_T reader;
reader.js_fill = fill_from_cookie;
reader.js_used = 0;
reader.js_buf = (char_u *)" [ \"a\" , 123 ";
reader.js_cookie = " [ \"a\" , 123 ] ";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)" [ \"a\" , ";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)" [ \"a\" ";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)" [ \"a";
assert(json_find_end(&reader, 0) == OK);
reader.js_buf = (char_u *)" [ ";
assert(json_find_end(&reader, 0) == OK);
}
static void
test_fill_called_on_string(void)
{
js_read_T reader;
reader.js_fill = fill_from_cookie;
reader.js_used = 0;
reader.js_buf = (char_u *)" \"foo";
reader.js_end = reader.js_buf + STRLEN(reader.js_buf);
reader.js_cookie = " \"foobar\" ";
assert(json_decode_string(&reader, NULL, '"') == OK);
}
#endif
int
main(void)
{
#if defined(FEAT_EVAL)
test_decode_find_end();
test_fill_called_on_find_end();
test_fill_called_on_string();
#endif
return 0;
}
