
















































































#if !defined(EASINGS_H)
#define EASINGS_H

#define EASINGS_STATIC_INLINE 

#if defined(EASINGS_STATIC_INLINE)
#define EASEDEF static inline
#else
#define EASEDEF extern
#endif

#include <math.h> 

#if !defined(PI)
#define PI 3.14159265358979323846f 
#endif

#if defined(__cplusplus)
extern "C" { 
#endif


EASEDEF float EaseLinearNone(float t, float b, float c, float d) { return (c*t/d + b); }
EASEDEF float EaseLinearIn(float t, float b, float c, float d) { return (c*t/d + b); }
EASEDEF float EaseLinearOut(float t, float b, float c, float d) { return (c*t/d + b); }
EASEDEF float EaseLinearInOut(float t,float b, float c, float d) { return (c*t/d + b); }


EASEDEF float EaseSineIn(float t, float b, float c, float d) { return (-c*cos(t/d*(PI/2)) + c + b); }
EASEDEF float EaseSineOut(float t, float b, float c, float d) { return (c*sin(t/d*(PI/2)) + b); }
EASEDEF float EaseSineInOut(float t, float b, float c, float d) { return (-c/2*(cos(PI*t/d) - 1) + b); }


EASEDEF float EaseCircIn(float t, float b, float c, float d) { t /= d; return (-c*(sqrt(1 - t*t) - 1) + b); }
EASEDEF float EaseCircOut(float t, float b, float c, float d) { t = t/d - 1; return (c*sqrt(1 - t*t) + b); }
EASEDEF float EaseCircInOut(float t, float b, float c, float d) 
{
if ((t/=d/2) < 1) return (-c/2*(sqrt(1 - t*t) - 1) + b);
t -= 2; return (c/2*(sqrt(1 - t*t) + 1) + b);
}


EASEDEF float EaseCubicIn(float t, float b, float c, float d) { t /= d; return (c*t*t*t + b); }
EASEDEF float EaseCubicOut(float t, float b, float c, float d) { t = t/d-1; return (c*(t*t*t + 1) + b); }
EASEDEF float EaseCubicInOut(float t, float b, float c, float d) 
{ 
if ((t/=d/2) < 1) return (c/2*t*t*t + b);
t -= 2; return (c/2*(t*t*t + 2) + b);
}


EASEDEF float EaseQuadIn(float t, float b, float c, float d) { t /= d; return (c*t*t + b); }
EASEDEF float EaseQuadOut(float t, float b, float c, float d) { t /= d; return (-c*t*(t - 2) + b); }
EASEDEF float EaseQuadInOut(float t, float b, float c, float d) 
{
if ((t/=d/2) < 1) return (((c/2)*(t*t)) + b);
t--; return (-c/2*(((t - 2)*t) - 1) + b);
}


EASEDEF float EaseExpoIn(float t, float b, float c, float d) { return (t == 0) ? b : (c*pow(2, 10*(t/d - 1)) + b); }
EASEDEF float EaseExpoOut(float t, float b, float c, float d) { return (t == d) ? (b + c) : (c*(-pow(2, -10*t/d) + 1) + b); }
EASEDEF float EaseExpoInOut(float t, float b, float c, float d) 
{
if (t == 0) return b;
if (t == d) return (b + c);
if ((t/=d/2) < 1) return (c/2*pow(2, 10*(t - 1)) + b);

return (c/2*(-pow(2, -10*--t) + 2) + b);
}


EASEDEF float EaseBackIn(float t, float b, float c, float d) 
{
float s = 1.70158f;
float postFix = t/=d;
return (c*(postFix)*t*((s + 1)*t - s) + b);
}

EASEDEF float EaseBackOut(float t, float b, float c, float d)
{ 
float s = 1.70158f;
t = t/d - 1;
return (c*(t*t*((s + 1)*t + s) + 1) + b);
}

EASEDEF float EaseBackInOut(float t, float b, float c, float d)
{
float s = 1.70158f;
if ((t/=d/2) < 1) 
{
s *= 1.525f;
return (c/2*(t*t*((s + 1)*t - s)) + b);
}

float postFix = t-=2;
s *= 1.525f;
return (c/2*((postFix)*t*((s + 1)*t + s) + 2) + b);
}


EASEDEF float EaseBounceOut(float t, float b, float c, float d) 
{
if ((t/=d) < (1/2.75f)) 
{
return (c*(7.5625f*t*t) + b);
} 
else if (t < (2/2.75f)) 
{
float postFix = t-=(1.5f/2.75f);
return (c*(7.5625f*(postFix)*t + 0.75f) + b);
} 
else if (t < (2.5/2.75)) 
{
float postFix = t-=(2.25f/2.75f);
return (c*(7.5625f*(postFix)*t + 0.9375f) + b);
} 
else 
{
float postFix = t-=(2.625f/2.75f);
return (c*(7.5625f*(postFix)*t + 0.984375f) + b);
}
}

EASEDEF float EaseBounceIn(float t, float b, float c, float d) { return (c - EaseBounceOut(d-t, 0, c, d) + b); }
EASEDEF float EaseBounceInOut(float t, float b, float c, float d) 
{
if (t < d/2) return (EaseBounceIn(t*2, 0, c, d)*0.5f + b);
else return (EaseBounceOut(t*2-d, 0, c, d)*0.5f + c*0.5f + b);
}


EASEDEF float EaseElasticIn(float t, float b, float c, float d) 
{
if (t == 0) return b;
if ((t/=d) == 1) return (b + c);

float p = d*0.3f;
float a = c; 
float s = p/4;
float postFix = a*pow(2, 10*(t-=1));

return (-(postFix*sin((t*d-s)*(2*PI)/p )) + b);
}

EASEDEF float EaseElasticOut(float t, float b, float c, float d)
{
if (t == 0) return b;
if ((t/=d) == 1) return (b + c);

float p = d*0.3f;
float a = c; 
float s = p/4;

return (a*pow(2,-10*t)*sin((t*d-s)*(2*PI)/p) + c + b); 
}

EASEDEF float EaseElasticInOut(float t, float b, float c, float d)
{
if (t == 0) return b;
if ((t/=d/2) == 2) return (b + c);

float p = d*(0.3f*1.5f);
float a = c;
float s = p/4;

if (t < 1) 
{
float postFix = a*pow(2, 10*(t-=1));
return -0.5f*(postFix*sin((t*d-s)*(2*PI)/p)) + b;
}

float postFix = a*pow(2, -10*(t-=1));

return (postFix*sin((t*d-s)*(2*PI)/p)*0.5f + c + b);
}

#if defined(__cplusplus)
}
#endif

#endif 
