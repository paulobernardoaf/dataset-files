#include "common.h"
#include "integer.h"
#include "avassert.h"
static const AVInteger zero_i;
AVInteger av_add_i(AVInteger a, AVInteger b){
int i, carry=0;
for(i=0; i<AV_INTEGER_SIZE; i++){
carry= (carry>>16) + a.v[i] + b.v[i];
a.v[i]= carry;
}
return a;
}
AVInteger av_sub_i(AVInteger a, AVInteger b){
int i, carry=0;
for(i=0; i<AV_INTEGER_SIZE; i++){
carry= (carry>>16) + a.v[i] - b.v[i];
a.v[i]= carry;
}
return a;
}
int av_log2_i(AVInteger a){
int i;
for(i=AV_INTEGER_SIZE-1; i>=0; i--){
if(a.v[i])
return av_log2_16bit(a.v[i]) + 16*i;
}
return -1;
}
AVInteger av_mul_i(AVInteger a, AVInteger b){
AVInteger out;
int i, j;
int na= (av_log2_i(a)+16) >> 4;
int nb= (av_log2_i(b)+16) >> 4;
memset(&out, 0, sizeof(out));
for(i=0; i<na; i++){
unsigned int carry=0;
if(a.v[i])
for(j=i; j<AV_INTEGER_SIZE && j-i<=nb; j++){
carry= (carry>>16) + out.v[j] + a.v[i]*(unsigned)b.v[j-i];
out.v[j]= carry;
}
}
return out;
}
int av_cmp_i(AVInteger a, AVInteger b){
int i;
int v= (int16_t)a.v[AV_INTEGER_SIZE-1] - (int16_t)b.v[AV_INTEGER_SIZE-1];
if(v) return (v>>16)|1;
for(i=AV_INTEGER_SIZE-2; i>=0; i--){
int v= a.v[i] - b.v[i];
if(v) return (v>>16)|1;
}
return 0;
}
AVInteger av_shr_i(AVInteger a, int s){
AVInteger out;
int i;
for(i=0; i<AV_INTEGER_SIZE; i++){
unsigned int index= i + (s>>4);
unsigned int v=0;
if(index+1<AV_INTEGER_SIZE) v = a.v[index+1]<<16;
if(index <AV_INTEGER_SIZE) v+= a.v[index ];
out.v[i]= v >> (s&15);
}
return out;
}
AVInteger av_mod_i(AVInteger *quot, AVInteger a, AVInteger b){
int i= av_log2_i(a) - av_log2_i(b);
AVInteger quot_temp;
if(!quot) quot = &quot_temp;
if ((int16_t)a.v[AV_INTEGER_SIZE-1] < 0) {
a = av_mod_i(quot, av_sub_i(zero_i, a), b);
*quot = av_sub_i(zero_i, *quot);
return av_sub_i(zero_i, a);
}
av_assert2((int16_t)a.v[AV_INTEGER_SIZE-1] >= 0 && (int16_t)b.v[AV_INTEGER_SIZE-1] >= 0);
av_assert2(av_log2_i(b)>=0);
if(i > 0)
b= av_shr_i(b, -i);
memset(quot, 0, sizeof(AVInteger));
while(i-- >= 0){
*quot= av_shr_i(*quot, -1);
if(av_cmp_i(a, b) >= 0){
a= av_sub_i(a, b);
quot->v[0] += 1;
}
b= av_shr_i(b, 1);
}
return a;
}
AVInteger av_div_i(AVInteger a, AVInteger b){
AVInteger quot;
av_mod_i(&quot, a, b);
return quot;
}
AVInteger av_int2i(int64_t a){
AVInteger out;
int i;
for(i=0; i<AV_INTEGER_SIZE; i++){
out.v[i]= a;
a>>=16;
}
return out;
}
int64_t av_i2int(AVInteger a){
int i;
int64_t out=(int8_t)a.v[AV_INTEGER_SIZE-1];
for(i= AV_INTEGER_SIZE-2; i>=0; i--){
out = (out<<16) + a.v[i];
}
return out;
}
