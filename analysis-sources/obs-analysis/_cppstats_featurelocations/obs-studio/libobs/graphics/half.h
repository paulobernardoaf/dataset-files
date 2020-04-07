







































#pragma once

#include "math-defs.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct half {
uint16_t u;
};


static struct half half_from_float(float f)
{
uint32_t Result;

uint32_t IValue;
memcpy(&IValue, &f, sizeof(IValue));
uint32_t Sign = (IValue & 0x80000000U) >> 16U;
IValue = IValue & 0x7FFFFFFFU; 

if (IValue > 0x477FE000U) {

if (((IValue & 0x7F800000) == 0x7F800000) &&
((IValue & 0x7FFFFF) != 0)) {
Result = 0x7FFF; 
} else {
Result = 0x7C00U; 
}
} else if (!IValue) {
Result = 0;
} else {
if (IValue < 0x38800000U) {


uint32_t Shift = 113U - (IValue >> 23U);
IValue = (0x800000U | (IValue & 0x7FFFFFU)) >> Shift;
} else {

IValue += 0xC8000000U;
}

Result = ((IValue + 0x0FFFU + ((IValue >> 13U) & 1U)) >> 13U) &
0x7FFFU;
}

struct half h;
h.u = (uint16_t)(Result | Sign);
return h;
}

static struct half half_from_bits(uint16_t u)
{
struct half h;
h.u = u;
return h;
}

#if defined(__cplusplus)
}
#endif
