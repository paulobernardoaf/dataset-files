#if !defined(RAYMATH_STANDALONE)
#include "raylib.h" 
#endif
#if defined(RAYMATH_IMPLEMENTATION) && defined(RAYMATH_HEADER_ONLY)
#error "Specifying both RAYMATH_IMPLEMENTATION and RAYMATH_HEADER_ONLY is contradictory"
#endif
#if defined(RAYMATH_IMPLEMENTATION)
#if defined(_WIN32) && defined(BUILD_LIBTYPE_SHARED)
#define RMDEF __declspec(dllexport) extern inline 
#elif defined(_WIN32) && defined(USE_LIBTYPE_SHARED)
#define RMDEF __declspec(dllimport) 
#else
#define RMDEF extern inline 
#endif
#elif defined(RAYMATH_HEADER_ONLY)
#define RMDEF static inline 
#else
#if defined(__TINYC__)
#define RMDEF static inline 
#else
#define RMDEF inline 
#endif
#endif
#if !defined(PI)
#define PI 3.14159265358979323846
#endif
#if !defined(DEG2RAD)
#define DEG2RAD (PI/180.0f)
#endif
#if !defined(RAD2DEG)
#define RAD2DEG (180.0f/PI)
#endif
#if !defined(MatrixToFloat)
#define MatrixToFloat(mat) (MatrixToFloatV(mat).v)
#endif
#if !defined(Vector3ToFloat)
#define Vector3ToFloat(vec) (Vector3ToFloatV(vec).v)
#endif
#if defined(RAYMATH_STANDALONE)
typedef struct Vector2 {
float x;
float y;
} Vector2;
typedef struct Vector3 {
float x;
float y;
float z;
} Vector3;
typedef struct Quaternion {
float x;
float y;
float z;
float w;
} Quaternion;
typedef struct Matrix {
float m0, m4, m8, m12;
float m1, m5, m9, m13;
float m2, m6, m10, m14;
float m3, m7, m11, m15;
} Matrix;
#endif
typedef struct float3 { float v[3]; } float3;
typedef struct float16 { float v[16]; } float16;
#include <math.h> 
RMDEF float Clamp(float value, float min, float max)
{
const float res = value < min ? min : value;
return res > max ? max : res;
}
RMDEF float Lerp(float start, float end, float amount)
{
return start + amount*(end - start);
}
RMDEF Vector2 Vector2Zero(void)
{
Vector2 result = { 0.0f, 0.0f };
return result;
}
RMDEF Vector2 Vector2One(void)
{
Vector2 result = { 1.0f, 1.0f };
return result;
}
RMDEF Vector2 Vector2Add(Vector2 v1, Vector2 v2)
{
Vector2 result = { v1.x + v2.x, v1.y + v2.y };
return result;
}
RMDEF Vector2 Vector2Subtract(Vector2 v1, Vector2 v2)
{
Vector2 result = { v1.x - v2.x, v1.y - v2.y };
return result;
}
RMDEF float Vector2Length(Vector2 v)
{
float result = sqrtf((v.x*v.x) + (v.y*v.y));
return result;
}
RMDEF float Vector2DotProduct(Vector2 v1, Vector2 v2)
{
float result = (v1.x*v2.x + v1.y*v2.y);
return result;
}
RMDEF float Vector2Distance(Vector2 v1, Vector2 v2)
{
float result = sqrtf((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y));
return result;
}
RMDEF float Vector2Angle(Vector2 v1, Vector2 v2)
{
float result = atan2f(v2.y - v1.y, v2.x - v1.x)*(180.0f/PI);
if (result < 0) result += 360.0f;
return result;
}
RMDEF Vector2 Vector2Scale(Vector2 v, float scale)
{
Vector2 result = { v.x*scale, v.y*scale };
return result;
}
RMDEF Vector2 Vector2MultiplyV(Vector2 v1, Vector2 v2)
{
Vector2 result = { v1.x*v2.x, v1.y*v2.y };
return result;
}
RMDEF Vector2 Vector2Negate(Vector2 v)
{
Vector2 result = { -v.x, -v.y };
return result;
}
RMDEF Vector2 Vector2Divide(Vector2 v, float div)
{
Vector2 result = { v.x/div, v.y/div };
return result;
}
RMDEF Vector2 Vector2DivideV(Vector2 v1, Vector2 v2)
{
Vector2 result = { v1.x/v2.x, v1.y/v2.y };
return result;
}
RMDEF Vector2 Vector2Normalize(Vector2 v)
{
Vector2 result = Vector2Divide(v, Vector2Length(v));
return result;
}
RMDEF Vector2 Vector2Lerp(Vector2 v1, Vector2 v2, float amount)
{
Vector2 result = { 0 };
result.x = v1.x + amount*(v2.x - v1.x);
result.y = v1.y + amount*(v2.y - v1.y);
return result;
}
RMDEF Vector2 Vector2Rotate(Vector2 v, float degs)
{
float rads = degs*DEG2RAD;
Vector2 result = {v.x * cosf(rads) - v.y * sinf(rads) , v.x * sinf(rads) + v.y * cosf(rads) };
return result;
}
RMDEF Vector3 Vector3Zero(void)
{
Vector3 result = { 0.0f, 0.0f, 0.0f };
return result;
}
RMDEF Vector3 Vector3One(void)
{
Vector3 result = { 1.0f, 1.0f, 1.0f };
return result;
}
RMDEF Vector3 Vector3Add(Vector3 v1, Vector3 v2)
{
Vector3 result = { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
return result;
}
RMDEF Vector3 Vector3Subtract(Vector3 v1, Vector3 v2)
{
Vector3 result = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
return result;
}
RMDEF Vector3 Vector3Scale(Vector3 v, float scalar)
{
Vector3 result = { v.x*scalar, v.y*scalar, v.z*scalar };
return result;
}
RMDEF Vector3 Vector3Multiply(Vector3 v1, Vector3 v2)
{
Vector3 result = { v1.x*v2.x, v1.y*v2.y, v1.z*v2.z };
return result;
}
RMDEF Vector3 Vector3CrossProduct(Vector3 v1, Vector3 v2)
{
Vector3 result = { v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x };
return result;
}
RMDEF Vector3 Vector3Perpendicular(Vector3 v)
{
Vector3 result = { 0 };
float min = (float) fabs(v.x);
Vector3 cardinalAxis = {1.0f, 0.0f, 0.0f};
if (fabs(v.y) < min)
{
min = (float) fabs(v.y);
Vector3 tmp = {0.0f, 1.0f, 0.0f};
cardinalAxis = tmp;
}
if (fabs(v.z) < min)
{
Vector3 tmp = {0.0f, 0.0f, 1.0f};
cardinalAxis = tmp;
}
result = Vector3CrossProduct(v, cardinalAxis);
return result;
}
RMDEF float Vector3Length(const Vector3 v)
{
float result = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
return result;
}
RMDEF float Vector3DotProduct(Vector3 v1, Vector3 v2)
{
float result = (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
return result;
}
RMDEF float Vector3Distance(Vector3 v1, Vector3 v2)
{
float dx = v2.x - v1.x;
float dy = v2.y - v1.y;
float dz = v2.z - v1.z;
float result = sqrtf(dx*dx + dy*dy + dz*dz);
return result;
}
RMDEF Vector3 Vector3Negate(Vector3 v)
{
Vector3 result = { -v.x, -v.y, -v.z };
return result;
}
RMDEF Vector3 Vector3Divide(Vector3 v, float div)
{
Vector3 result = { v.x / div, v.y / div, v.z / div };
return result;
}
RMDEF Vector3 Vector3DivideV(Vector3 v1, Vector3 v2)
{
Vector3 result = { v1.x/v2.x, v1.y/v2.y, v1.z/v2.z };
return result;
}
RMDEF Vector3 Vector3Normalize(Vector3 v)
{
Vector3 result = v;
float length, ilength;
length = Vector3Length(v);
if (length == 0.0f) length = 1.0f;
ilength = 1.0f/length;
result.x *= ilength;
result.y *= ilength;
result.z *= ilength;
return result;
}
RMDEF void Vector3OrthoNormalize(Vector3 *v1, Vector3 *v2)
{
*v1 = Vector3Normalize(*v1);
Vector3 vn = Vector3CrossProduct(*v1, *v2);
vn = Vector3Normalize(vn);
*v2 = Vector3CrossProduct(vn, *v1);
}
RMDEF Vector3 Vector3Transform(Vector3 v, Matrix mat)
{
Vector3 result = { 0 };
float x = v.x;
float y = v.y;
float z = v.z;
result.x = mat.m0*x + mat.m4*y + mat.m8*z + mat.m12;
result.y = mat.m1*x + mat.m5*y + mat.m9*z + mat.m13;
result.z = mat.m2*x + mat.m6*y + mat.m10*z + mat.m14;
return result;
}
RMDEF Vector3 Vector3RotateByQuaternion(Vector3 v, Quaternion q)
{
Vector3 result = { 0 };
result.x = v.x*(q.x*q.x + q.w*q.w - q.y*q.y - q.z*q.z) + v.y*(2*q.x*q.y - 2*q.w*q.z) + v.z*(2*q.x*q.z + 2*q.w*q.y);
result.y = v.x*(2*q.w*q.z + 2*q.x*q.y) + v.y*(q.w*q.w - q.x*q.x + q.y*q.y - q.z*q.z) + v.z*(-2*q.w*q.x + 2*q.y*q.z);
result.z = v.x*(-2*q.w*q.y + 2*q.x*q.z) + v.y*(2*q.w*q.x + 2*q.y*q.z)+ v.z*(q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z);
return result;
}
RMDEF Vector3 Vector3Lerp(Vector3 v1, Vector3 v2, float amount)
{
Vector3 result = { 0 };
result.x = v1.x + amount*(v2.x - v1.x);
result.y = v1.y + amount*(v2.y - v1.y);
result.z = v1.z + amount*(v2.z - v1.z);
return result;
}
RMDEF Vector3 Vector3Reflect(Vector3 v, Vector3 normal)
{
Vector3 result = { 0 };
float dotProduct = Vector3DotProduct(v, normal);
result.x = v.x - (2.0f*normal.x)*dotProduct;
result.y = v.y - (2.0f*normal.y)*dotProduct;
result.z = v.z - (2.0f*normal.z)*dotProduct;
return result;
}
RMDEF Vector3 Vector3Min(Vector3 v1, Vector3 v2)
{
Vector3 result = { 0 };
result.x = fminf(v1.x, v2.x);
result.y = fminf(v1.y, v2.y);
result.z = fminf(v1.z, v2.z);
return result;
}
RMDEF Vector3 Vector3Max(Vector3 v1, Vector3 v2)
{
Vector3 result = { 0 };
result.x = fmaxf(v1.x, v2.x);
result.y = fmaxf(v1.y, v2.y);
result.z = fmaxf(v1.z, v2.z);
return result;
}
RMDEF Vector3 Vector3Barycenter(Vector3 p, Vector3 a, Vector3 b, Vector3 c)
{
Vector3 v0 = Vector3Subtract(b, a);
Vector3 v1 = Vector3Subtract(c, a);
Vector3 v2 = Vector3Subtract(p, a);
float d00 = Vector3DotProduct(v0, v0);
float d01 = Vector3DotProduct(v0, v1);
float d11 = Vector3DotProduct(v1, v1);
float d20 = Vector3DotProduct(v2, v0);
float d21 = Vector3DotProduct(v2, v1);
float denom = d00*d11 - d01*d01;
Vector3 result = { 0 };
result.y = (d11*d20 - d01*d21)/denom;
result.z = (d00*d21 - d01*d20)/denom;
result.x = 1.0f - (result.z + result.y);
return result;
}
RMDEF float3 Vector3ToFloatV(Vector3 v)
{
float3 buffer = { 0 };
buffer.v[0] = v.x;
buffer.v[1] = v.y;
buffer.v[2] = v.z;
return buffer;
}
RMDEF float MatrixDeterminant(Matrix mat)
{
float a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
float a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
float a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;
float a30 = mat.m12, a31 = mat.m13, a32 = mat.m14, a33 = mat.m15;
float result = a30*a21*a12*a03 - a20*a31*a12*a03 - a30*a11*a22*a03 + a10*a31*a22*a03 +
a20*a11*a32*a03 - a10*a21*a32*a03 - a30*a21*a02*a13 + a20*a31*a02*a13 +
a30*a01*a22*a13 - a00*a31*a22*a13 - a20*a01*a32*a13 + a00*a21*a32*a13 +
a30*a11*a02*a23 - a10*a31*a02*a23 - a30*a01*a12*a23 + a00*a31*a12*a23 +
a10*a01*a32*a23 - a00*a11*a32*a23 - a20*a11*a02*a33 + a10*a21*a02*a33 +
a20*a01*a12*a33 - a00*a21*a12*a33 - a10*a01*a22*a33 + a00*a11*a22*a33;
return result;
}
RMDEF float MatrixTrace(Matrix mat)
{
float result = (mat.m0 + mat.m5 + mat.m10 + mat.m15);
return result;
}
RMDEF Matrix MatrixTranspose(Matrix mat)
{
Matrix result = { 0 };
result.m0 = mat.m0;
result.m1 = mat.m4;
result.m2 = mat.m8;
result.m3 = mat.m12;
result.m4 = mat.m1;
result.m5 = mat.m5;
result.m6 = mat.m9;
result.m7 = mat.m13;
result.m8 = mat.m2;
result.m9 = mat.m6;
result.m10 = mat.m10;
result.m11 = mat.m14;
result.m12 = mat.m3;
result.m13 = mat.m7;
result.m14 = mat.m11;
result.m15 = mat.m15;
return result;
}
RMDEF Matrix MatrixInvert(Matrix mat)
{
Matrix result = { 0 };
float a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
float a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
float a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;
float a30 = mat.m12, a31 = mat.m13, a32 = mat.m14, a33 = mat.m15;
float b00 = a00*a11 - a01*a10;
float b01 = a00*a12 - a02*a10;
float b02 = a00*a13 - a03*a10;
float b03 = a01*a12 - a02*a11;
float b04 = a01*a13 - a03*a11;
float b05 = a02*a13 - a03*a12;
float b06 = a20*a31 - a21*a30;
float b07 = a20*a32 - a22*a30;
float b08 = a20*a33 - a23*a30;
float b09 = a21*a32 - a22*a31;
float b10 = a21*a33 - a23*a31;
float b11 = a22*a33 - a23*a32;
float invDet = 1.0f/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);
result.m0 = (a11*b11 - a12*b10 + a13*b09)*invDet;
result.m1 = (-a01*b11 + a02*b10 - a03*b09)*invDet;
result.m2 = (a31*b05 - a32*b04 + a33*b03)*invDet;
result.m3 = (-a21*b05 + a22*b04 - a23*b03)*invDet;
result.m4 = (-a10*b11 + a12*b08 - a13*b07)*invDet;
result.m5 = (a00*b11 - a02*b08 + a03*b07)*invDet;
result.m6 = (-a30*b05 + a32*b02 - a33*b01)*invDet;
result.m7 = (a20*b05 - a22*b02 + a23*b01)*invDet;
result.m8 = (a10*b10 - a11*b08 + a13*b06)*invDet;
result.m9 = (-a00*b10 + a01*b08 - a03*b06)*invDet;
result.m10 = (a30*b04 - a31*b02 + a33*b00)*invDet;
result.m11 = (-a20*b04 + a21*b02 - a23*b00)*invDet;
result.m12 = (-a10*b09 + a11*b07 - a12*b06)*invDet;
result.m13 = (a00*b09 - a01*b07 + a02*b06)*invDet;
result.m14 = (-a30*b03 + a31*b01 - a32*b00)*invDet;
result.m15 = (a20*b03 - a21*b01 + a22*b00)*invDet;
return result;
}
RMDEF Matrix MatrixNormalize(Matrix mat)
{
Matrix result = { 0 };
float det = MatrixDeterminant(mat);
result.m0 = mat.m0/det;
result.m1 = mat.m1/det;
result.m2 = mat.m2/det;
result.m3 = mat.m3/det;
result.m4 = mat.m4/det;
result.m5 = mat.m5/det;
result.m6 = mat.m6/det;
result.m7 = mat.m7/det;
result.m8 = mat.m8/det;
result.m9 = mat.m9/det;
result.m10 = mat.m10/det;
result.m11 = mat.m11/det;
result.m12 = mat.m12/det;
result.m13 = mat.m13/det;
result.m14 = mat.m14/det;
result.m15 = mat.m15/det;
return result;
}
RMDEF Matrix MatrixIdentity(void)
{
Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
0.0f, 1.0f, 0.0f, 0.0f,
0.0f, 0.0f, 1.0f, 0.0f,
0.0f, 0.0f, 0.0f, 1.0f };
return result;
}
RMDEF Matrix MatrixAdd(Matrix left, Matrix right)
{
Matrix result = MatrixIdentity();
result.m0 = left.m0 + right.m0;
result.m1 = left.m1 + right.m1;
result.m2 = left.m2 + right.m2;
result.m3 = left.m3 + right.m3;
result.m4 = left.m4 + right.m4;
result.m5 = left.m5 + right.m5;
result.m6 = left.m6 + right.m6;
result.m7 = left.m7 + right.m7;
result.m8 = left.m8 + right.m8;
result.m9 = left.m9 + right.m9;
result.m10 = left.m10 + right.m10;
result.m11 = left.m11 + right.m11;
result.m12 = left.m12 + right.m12;
result.m13 = left.m13 + right.m13;
result.m14 = left.m14 + right.m14;
result.m15 = left.m15 + right.m15;
return result;
}
RMDEF Matrix MatrixSubtract(Matrix left, Matrix right)
{
Matrix result = MatrixIdentity();
result.m0 = left.m0 - right.m0;
result.m1 = left.m1 - right.m1;
result.m2 = left.m2 - right.m2;
result.m3 = left.m3 - right.m3;
result.m4 = left.m4 - right.m4;
result.m5 = left.m5 - right.m5;
result.m6 = left.m6 - right.m6;
result.m7 = left.m7 - right.m7;
result.m8 = left.m8 - right.m8;
result.m9 = left.m9 - right.m9;
result.m10 = left.m10 - right.m10;
result.m11 = left.m11 - right.m11;
result.m12 = left.m12 - right.m12;
result.m13 = left.m13 - right.m13;
result.m14 = left.m14 - right.m14;
result.m15 = left.m15 - right.m15;
return result;
}
RMDEF Matrix MatrixTranslate(float x, float y, float z)
{
Matrix result = { 1.0f, 0.0f, 0.0f, x,
0.0f, 1.0f, 0.0f, y,
0.0f, 0.0f, 1.0f, z,
0.0f, 0.0f, 0.0f, 1.0f };
return result;
}
RMDEF Matrix MatrixRotate(Vector3 axis, float angle)
{
Matrix result = { 0 };
float x = axis.x, y = axis.y, z = axis.z;
float length = sqrtf(x*x + y*y + z*z);
if ((length != 1.0f) && (length != 0.0f))
{
length = 1.0f/length;
x *= length;
y *= length;
z *= length;
}
float sinres = sinf(angle);
float cosres = cosf(angle);
float t = 1.0f - cosres;
result.m0 = x*x*t + cosres;
result.m1 = y*x*t + z*sinres;
result.m2 = z*x*t - y*sinres;
result.m3 = 0.0f;
result.m4 = x*y*t - z*sinres;
result.m5 = y*y*t + cosres;
result.m6 = z*y*t + x*sinres;
result.m7 = 0.0f;
result.m8 = x*z*t + y*sinres;
result.m9 = y*z*t - x*sinres;
result.m10 = z*z*t + cosres;
result.m11 = 0.0f;
result.m12 = 0.0f;
result.m13 = 0.0f;
result.m14 = 0.0f;
result.m15 = 1.0f;
return result;
}
RMDEF Matrix MatrixRotateXYZ(Vector3 ang)
{
Matrix result = MatrixIdentity();
float cosz = cosf(-ang.z);
float sinz = sinf(-ang.z);
float cosy = cosf(-ang.y);
float siny = sinf(-ang.y);
float cosx = cosf(-ang.x);
float sinx = sinf(-ang.x);
result.m0 = cosz * cosy;
result.m4 = (cosz * siny * sinx) - (sinz * cosx);
result.m8 = (cosz * siny * cosx) + (sinz * sinx);
result.m1 = sinz * cosy;
result.m5 = (sinz * siny * sinx) + (cosz * cosx);
result.m9 = (sinz * siny * cosx) - (cosz * sinx);
result.m2 = -siny;
result.m6 = cosy * sinx;
result.m10= cosy * cosx;
return result;
}
RMDEF Matrix MatrixRotateX(float angle)
{
Matrix result = MatrixIdentity();
float cosres = cosf(angle);
float sinres = sinf(angle);
result.m5 = cosres;
result.m6 = -sinres;
result.m9 = sinres;
result.m10 = cosres;
return result;
}
RMDEF Matrix MatrixRotateY(float angle)
{
Matrix result = MatrixIdentity();
float cosres = cosf(angle);
float sinres = sinf(angle);
result.m0 = cosres;
result.m2 = sinres;
result.m8 = -sinres;
result.m10 = cosres;
return result;
}
RMDEF Matrix MatrixRotateZ(float angle)
{
Matrix result = MatrixIdentity();
float cosres = cosf(angle);
float sinres = sinf(angle);
result.m0 = cosres;
result.m1 = -sinres;
result.m4 = sinres;
result.m5 = cosres;
return result;
}
RMDEF Matrix MatrixScale(float x, float y, float z)
{
Matrix result = { x, 0.0f, 0.0f, 0.0f,
0.0f, y, 0.0f, 0.0f,
0.0f, 0.0f, z, 0.0f,
0.0f, 0.0f, 0.0f, 1.0f };
return result;
}
RMDEF Matrix MatrixMultiply(Matrix left, Matrix right)
{
Matrix result = { 0 };
result.m0 = left.m0*right.m0 + left.m1*right.m4 + left.m2*right.m8 + left.m3*right.m12;
result.m1 = left.m0*right.m1 + left.m1*right.m5 + left.m2*right.m9 + left.m3*right.m13;
result.m2 = left.m0*right.m2 + left.m1*right.m6 + left.m2*right.m10 + left.m3*right.m14;
result.m3 = left.m0*right.m3 + left.m1*right.m7 + left.m2*right.m11 + left.m3*right.m15;
result.m4 = left.m4*right.m0 + left.m5*right.m4 + left.m6*right.m8 + left.m7*right.m12;
result.m5 = left.m4*right.m1 + left.m5*right.m5 + left.m6*right.m9 + left.m7*right.m13;
result.m6 = left.m4*right.m2 + left.m5*right.m6 + left.m6*right.m10 + left.m7*right.m14;
result.m7 = left.m4*right.m3 + left.m5*right.m7 + left.m6*right.m11 + left.m7*right.m15;
result.m8 = left.m8*right.m0 + left.m9*right.m4 + left.m10*right.m8 + left.m11*right.m12;
result.m9 = left.m8*right.m1 + left.m9*right.m5 + left.m10*right.m9 + left.m11*right.m13;
result.m10 = left.m8*right.m2 + left.m9*right.m6 + left.m10*right.m10 + left.m11*right.m14;
result.m11 = left.m8*right.m3 + left.m9*right.m7 + left.m10*right.m11 + left.m11*right.m15;
result.m12 = left.m12*right.m0 + left.m13*right.m4 + left.m14*right.m8 + left.m15*right.m12;
result.m13 = left.m12*right.m1 + left.m13*right.m5 + left.m14*right.m9 + left.m15*right.m13;
result.m14 = left.m12*right.m2 + left.m13*right.m6 + left.m14*right.m10 + left.m15*right.m14;
result.m15 = left.m12*right.m3 + left.m13*right.m7 + left.m14*right.m11 + left.m15*right.m15;
return result;
}
RMDEF Matrix MatrixFrustum(double left, double right, double bottom, double top, double near, double far)
{
Matrix result = { 0 };
float rl = (float)(right - left);
float tb = (float)(top - bottom);
float fn = (float)(far - near);
result.m0 = ((float) near*2.0f)/rl;
result.m1 = 0.0f;
result.m2 = 0.0f;
result.m3 = 0.0f;
result.m4 = 0.0f;
result.m5 = ((float) near*2.0f)/tb;
result.m6 = 0.0f;
result.m7 = 0.0f;
result.m8 = ((float)right + (float)left)/rl;
result.m9 = ((float)top + (float)bottom)/tb;
result.m10 = -((float)far + (float)near)/fn;
result.m11 = -1.0f;
result.m12 = 0.0f;
result.m13 = 0.0f;
result.m14 = -((float)far*(float)near*2.0f)/fn;
result.m15 = 0.0f;
return result;
}
RMDEF Matrix MatrixPerspective(double fovy, double aspect, double near, double far)
{
double top = near*tan(fovy*0.5);
double right = top*aspect;
Matrix result = MatrixFrustum(-right, right, -top, top, near, far);
return result;
}
RMDEF Matrix MatrixOrtho(double left, double right, double bottom, double top, double near, double far)
{
Matrix result = { 0 };
float rl = (float)(right - left);
float tb = (float)(top - bottom);
float fn = (float)(far - near);
result.m0 = 2.0f/rl;
result.m1 = 0.0f;
result.m2 = 0.0f;
result.m3 = 0.0f;
result.m4 = 0.0f;
result.m5 = 2.0f/tb;
result.m6 = 0.0f;
result.m7 = 0.0f;
result.m8 = 0.0f;
result.m9 = 0.0f;
result.m10 = -2.0f/fn;
result.m11 = 0.0f;
result.m12 = -((float)left + (float)right)/rl;
result.m13 = -((float)top + (float)bottom)/tb;
result.m14 = -((float)far + (float)near)/fn;
result.m15 = 1.0f;
return result;
}
RMDEF Matrix MatrixLookAt(Vector3 eye, Vector3 target, Vector3 up)
{
Matrix result = { 0 };
Vector3 z = Vector3Subtract(eye, target);
z = Vector3Normalize(z);
Vector3 x = Vector3CrossProduct(up, z);
x = Vector3Normalize(x);
Vector3 y = Vector3CrossProduct(z, x);
y = Vector3Normalize(y);
result.m0 = x.x;
result.m1 = x.y;
result.m2 = x.z;
result.m3 = 0.0f;
result.m4 = y.x;
result.m5 = y.y;
result.m6 = y.z;
result.m7 = 0.0f;
result.m8 = z.x;
result.m9 = z.y;
result.m10 = z.z;
result.m11 = 0.0f;
result.m12 = eye.x;
result.m13 = eye.y;
result.m14 = eye.z;
result.m15 = 1.0f;
result = MatrixInvert(result);
return result;
}
RMDEF float16 MatrixToFloatV(Matrix mat)
{
float16 buffer = { 0 };
buffer.v[0] = mat.m0;
buffer.v[1] = mat.m1;
buffer.v[2] = mat.m2;
buffer.v[3] = mat.m3;
buffer.v[4] = mat.m4;
buffer.v[5] = mat.m5;
buffer.v[6] = mat.m6;
buffer.v[7] = mat.m7;
buffer.v[8] = mat.m8;
buffer.v[9] = mat.m9;
buffer.v[10] = mat.m10;
buffer.v[11] = mat.m11;
buffer.v[12] = mat.m12;
buffer.v[13] = mat.m13;
buffer.v[14] = mat.m14;
buffer.v[15] = mat.m15;
return buffer;
}
RMDEF Quaternion QuaternionIdentity(void)
{
Quaternion result = { 0.0f, 0.0f, 0.0f, 1.0f };
return result;
}
RMDEF float QuaternionLength(Quaternion q)
{
float result = (float)sqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
return result;
}
RMDEF Quaternion QuaternionNormalize(Quaternion q)
{
Quaternion result = { 0 };
float length, ilength;
length = QuaternionLength(q);
if (length == 0.0f) length = 1.0f;
ilength = 1.0f/length;
result.x = q.x*ilength;
result.y = q.y*ilength;
result.z = q.z*ilength;
result.w = q.w*ilength;
return result;
}
RMDEF Quaternion QuaternionInvert(Quaternion q)
{
Quaternion result = q;
float length = QuaternionLength(q);
float lengthSq = length*length;
if (lengthSq != 0.0)
{
float i = 1.0f/lengthSq;
result.x *= -i;
result.y *= -i;
result.z *= -i;
result.w *= i;
}
return result;
}
RMDEF Quaternion QuaternionMultiply(Quaternion q1, Quaternion q2)
{
Quaternion result = { 0 };
float qax = q1.x, qay = q1.y, qaz = q1.z, qaw = q1.w;
float qbx = q2.x, qby = q2.y, qbz = q2.z, qbw = q2.w;
result.x = qax*qbw + qaw*qbx + qay*qbz - qaz*qby;
result.y = qay*qbw + qaw*qby + qaz*qbx - qax*qbz;
result.z = qaz*qbw + qaw*qbz + qax*qby - qay*qbx;
result.w = qaw*qbw - qax*qbx - qay*qby - qaz*qbz;
return result;
}
RMDEF Quaternion QuaternionLerp(Quaternion q1, Quaternion q2, float amount)
{
Quaternion result = { 0 };
result.x = q1.x + amount*(q2.x - q1.x);
result.y = q1.y + amount*(q2.y - q1.y);
result.z = q1.z + amount*(q2.z - q1.z);
result.w = q1.w + amount*(q2.w - q1.w);
return result;
}
RMDEF Quaternion QuaternionNlerp(Quaternion q1, Quaternion q2, float amount)
{
Quaternion result = QuaternionLerp(q1, q2, amount);
result = QuaternionNormalize(result);
return result;
}
RMDEF Quaternion QuaternionSlerp(Quaternion q1, Quaternion q2, float amount)
{
Quaternion result = { 0 };
float cosHalfTheta = q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;
if (fabs(cosHalfTheta) >= 1.0f) result = q1;
else if (cosHalfTheta > 0.95f) result = QuaternionNlerp(q1, q2, amount);
else
{
float halfTheta = acosf(cosHalfTheta);
float sinHalfTheta = sqrtf(1.0f - cosHalfTheta*cosHalfTheta);
if (fabs(sinHalfTheta) < 0.001f)
{
result.x = (q1.x*0.5f + q2.x*0.5f);
result.y = (q1.y*0.5f + q2.y*0.5f);
result.z = (q1.z*0.5f + q2.z*0.5f);
result.w = (q1.w*0.5f + q2.w*0.5f);
}
else
{
float ratioA = sinf((1 - amount)*halfTheta)/sinHalfTheta;
float ratioB = sinf(amount*halfTheta)/sinHalfTheta;
result.x = (q1.x*ratioA + q2.x*ratioB);
result.y = (q1.y*ratioA + q2.y*ratioB);
result.z = (q1.z*ratioA + q2.z*ratioB);
result.w = (q1.w*ratioA + q2.w*ratioB);
}
}
return result;
}
RMDEF Quaternion QuaternionFromVector3ToVector3(Vector3 from, Vector3 to)
{
Quaternion result = { 0 };
float cos2Theta = Vector3DotProduct(from, to);
Vector3 cross = Vector3CrossProduct(from, to);
result.x = cross.x;
result.y = cross.y;
result.z = cross.y;
result.w = 1.0f + cos2Theta; 
result = QuaternionNormalize(result);
return result;
}
RMDEF Quaternion QuaternionFromMatrix(Matrix mat)
{
Quaternion result = { 0 };
float trace = MatrixTrace(mat);
if (trace > 0.0f)
{
float s = sqrtf(trace + 1)*2.0f;
float invS = 1.0f/s;
result.w = s*0.25f;
result.x = (mat.m6 - mat.m9)*invS;
result.y = (mat.m8 - mat.m2)*invS;
result.z = (mat.m1 - mat.m4)*invS;
}
else
{
float m00 = mat.m0, m11 = mat.m5, m22 = mat.m10;
if (m00 > m11 && m00 > m22)
{
float s = (float)sqrt(1.0f + m00 - m11 - m22)*2.0f;
float invS = 1.0f/s;
result.w = (mat.m6 - mat.m9)*invS;
result.x = s*0.25f;
result.y = (mat.m4 + mat.m1)*invS;
result.z = (mat.m8 + mat.m2)*invS;
}
else if (m11 > m22)
{
float s = sqrtf(1.0f + m11 - m00 - m22)*2.0f;
float invS = 1.0f/s;
result.w = (mat.m8 - mat.m2)*invS;
result.x = (mat.m4 + mat.m1)*invS;
result.y = s*0.25f;
result.z = (mat.m9 + mat.m6)*invS;
}
else
{
float s = sqrtf(1.0f + m22 - m00 - m11)*2.0f;
float invS = 1.0f/s;
result.w = (mat.m1 - mat.m4)*invS;
result.x = (mat.m8 + mat.m2)*invS;
result.y = (mat.m9 + mat.m6)*invS;
result.z = s*0.25f;
}
}
return result;
}
RMDEF Matrix QuaternionToMatrix(Quaternion q)
{
Matrix result = { 0 };
float x = q.x, y = q.y, z = q.z, w = q.w;
float x2 = x + x;
float y2 = y + y;
float z2 = z + z;
float length = QuaternionLength(q);
float lengthSquared = length*length;
float xx = x*x2/lengthSquared;
float xy = x*y2/lengthSquared;
float xz = x*z2/lengthSquared;
float yy = y*y2/lengthSquared;
float yz = y*z2/lengthSquared;
float zz = z*z2/lengthSquared;
float wx = w*x2/lengthSquared;
float wy = w*y2/lengthSquared;
float wz = w*z2/lengthSquared;
result.m0 = 1.0f - (yy + zz);
result.m1 = xy - wz;
result.m2 = xz + wy;
result.m3 = 0.0f;
result.m4 = xy + wz;
result.m5 = 1.0f - (xx + zz);
result.m6 = yz - wx;
result.m7 = 0.0f;
result.m8 = xz - wy;
result.m9 = yz + wx;
result.m10 = 1.0f - (xx + yy);
result.m11 = 0.0f;
result.m12 = 0.0f;
result.m13 = 0.0f;
result.m14 = 0.0f;
result.m15 = 1.0f;
return result;
}
RMDEF Quaternion QuaternionFromAxisAngle(Vector3 axis, float angle)
{
Quaternion result = { 0.0f, 0.0f, 0.0f, 1.0f };
if (Vector3Length(axis) != 0.0f)
angle *= 0.5f;
axis = Vector3Normalize(axis);
float sinres = sinf(angle);
float cosres = cosf(angle);
result.x = axis.x*sinres;
result.y = axis.y*sinres;
result.z = axis.z*sinres;
result.w = cosres;
result = QuaternionNormalize(result);
return result;
}
RMDEF void QuaternionToAxisAngle(Quaternion q, Vector3 *outAxis, float *outAngle)
{
if (fabs(q.w) > 1.0f) q = QuaternionNormalize(q);
Vector3 resAxis = { 0.0f, 0.0f, 0.0f };
float resAngle = 2.0f*acosf(q.w);
float den = sqrtf(1.0f - q.w*q.w);
if (den > 0.0001f)
{
resAxis.x = q.x/den;
resAxis.y = q.y/den;
resAxis.z = q.z/den;
}
else
{
resAxis.x = 1.0f;
}
*outAxis = resAxis;
*outAngle = resAngle;
}
RMDEF Quaternion QuaternionFromEuler(float roll, float pitch, float yaw)
{
Quaternion q = { 0 };
float x0 = cosf(roll*0.5f);
float x1 = sinf(roll*0.5f);
float y0 = cosf(pitch*0.5f);
float y1 = sinf(pitch*0.5f);
float z0 = cosf(yaw*0.5f);
float z1 = sinf(yaw*0.5f);
q.x = x1*y0*z0 - x0*y1*z1;
q.y = x0*y1*z0 + x1*y0*z1;
q.z = x0*y0*z1 - x1*y1*z0;
q.w = x0*y0*z0 + x1*y1*z1;
return q;
}
RMDEF Vector3 QuaternionToEuler(Quaternion q)
{
Vector3 result = { 0 };
float x0 = 2.0f*(q.w*q.x + q.y*q.z);
float x1 = 1.0f - 2.0f*(q.x*q.x + q.y*q.y);
result.x = atan2f(x0, x1)*RAD2DEG;
float y0 = 2.0f*(q.w*q.y - q.z*q.x);
y0 = y0 > 1.0f ? 1.0f : y0;
y0 = y0 < -1.0f ? -1.0f : y0;
result.y = asinf(y0)*RAD2DEG;
float z0 = 2.0f*(q.w*q.z + q.x*q.y);
float z1 = 1.0f - 2.0f*(q.y*q.y + q.z*q.z);
result.z = atan2f(z0, z1)*RAD2DEG;
return result;
}
RMDEF Quaternion QuaternionTransform(Quaternion q, Matrix mat)
{
Quaternion result = { 0 };
result.x = mat.m0*q.x + mat.m4*q.y + mat.m8*q.z + mat.m12*q.w;
result.y = mat.m1*q.x + mat.m5*q.y + mat.m9*q.z + mat.m13*q.w;
result.z = mat.m2*q.x + mat.m6*q.y + mat.m10*q.z + mat.m14*q.w;
result.w = mat.m3*q.x + mat.m7*q.y + mat.m11*q.z + mat.m15*q.w;
return result;
}
