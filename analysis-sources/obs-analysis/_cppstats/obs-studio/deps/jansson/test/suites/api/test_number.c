#include <math.h>
#include <jansson.h>
#include "util.h"
#if defined(INFINITY)
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning (disable: 4756)
#endif
static void test_inifity()
{
json_t *real = json_real(INFINITY);
if (real != NULL)
fail("could construct a real from Inf");
real = json_real(1.0);
if (json_real_set(real, INFINITY) != -1)
fail("could set a real to Inf");
if (json_real_value(real) != 1.0)
fail("real value changed unexpectedly");
json_decref(real);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
}
#endif 
static void run_tests()
{
json_t *integer, *real;
json_int_t i;
double d;
integer = json_integer(5);
real = json_real(100.1);
if(!integer)
fail("unable to create integer");
if(!real)
fail("unable to create real");
i = json_integer_value(integer);
if(i != 5)
fail("wrong integer value");
d = json_real_value(real);
if(d != 100.1)
fail("wrong real value");
d = json_number_value(integer);
if(d != 5.0)
fail("wrong number value");
d = json_number_value(real);
if(d != 100.1)
fail("wrong number value");
json_decref(integer);
json_decref(real);
#if defined(NAN)
real = json_real(NAN);
if(real != NULL)
fail("could construct a real from NaN");
real = json_real(1.0);
if(json_real_set(real, NAN) != -1)
fail("could set a real to NaN");
if(json_real_value(real) != 1.0)
fail("real value changed unexpectedly");
json_decref(real);
#endif
#if defined(INFINITY)
test_inifity();
#endif
}
