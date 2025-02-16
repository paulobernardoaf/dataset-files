#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#define PARAM1_DEFAULT 4.0
#define PARAM2_DEFAULT 3.0
#define PARAM3_DEFAULT 6.0
struct vf_priv_s {
int Coefs[4][512*16];
unsigned int *Line;
unsigned short *Frame[3];
};
static unsigned int LowPassMul(unsigned int PrevMul, unsigned int CurrMul, int* Coef){
int dMul= PrevMul-CurrMul;
unsigned int d=((dMul+0x10007FF)>>12);
return CurrMul + Coef[d];
}
static void deNoiseTemporal(
unsigned char *Frame, 
unsigned char *FrameDest, 
unsigned short *FrameAnt,
int W, int H, int sStride, int dStride,
int *Temporal)
{
unsigned int PixelDst;
for (long Y = 0; Y < H; Y++){
for (long X = 0; X < W; X++){
PixelDst = LowPassMul(FrameAnt[X]<<8, Frame[X]<<16, Temporal);
FrameAnt[X] = ((PixelDst+0x1000007F)>>8);
FrameDest[X]= ((PixelDst+0x10007FFF)>>16);
}
Frame += sStride;
FrameDest += dStride;
FrameAnt += W;
}
}
static void deNoiseSpacial(
unsigned char *Frame, 
unsigned char *FrameDest, 
unsigned int *LineAnt, 
int W, int H, int sStride, int dStride,
int *Horizontal, int *Vertical)
{
long sLineOffs = 0, dLineOffs = 0;
unsigned int PixelAnt;
unsigned int PixelDst;
PixelDst = LineAnt[0] = PixelAnt = Frame[0]<<16;
FrameDest[0]= ((PixelDst+0x10007FFF)>>16);
for (long X = 1; X < W; X++){
PixelDst = LineAnt[X] = LowPassMul(PixelAnt, Frame[X]<<16, Horizontal);
FrameDest[X]= ((PixelDst+0x10007FFF)>>16);
}
for (long Y = 1; Y < H; Y++){
sLineOffs += sStride, dLineOffs += dStride;
PixelAnt = Frame[sLineOffs]<<16;
PixelDst = LineAnt[0] = LowPassMul(LineAnt[0], PixelAnt, Vertical);
FrameDest[dLineOffs]= ((PixelDst+0x10007FFF)>>16);
for (long X = 1; X < W; X++){
PixelAnt = LowPassMul(PixelAnt, Frame[sLineOffs+X]<<16, Horizontal);
PixelDst = LineAnt[X] = LowPassMul(LineAnt[X], PixelAnt, Vertical);
FrameDest[dLineOffs+X]= ((PixelDst+0x10007FFF)>>16);
}
}
}
static void deNoise(unsigned char *Frame, 
unsigned char *FrameDest, 
unsigned int *LineAnt, 
unsigned short **FrameAntPtr,
int W, int H, int sStride, int dStride,
int *Horizontal, int *Vertical, int *Temporal)
{
long sLineOffs = 0, dLineOffs = 0;
unsigned int PixelAnt;
unsigned int PixelDst;
unsigned short* FrameAnt=(*FrameAntPtr);
if(!FrameAnt){
(*FrameAntPtr)=FrameAnt=malloc(W*H*sizeof(unsigned short));
if(!FrameAnt)
return;
for (long Y = 0; Y < H; Y++){
unsigned short* dst=&FrameAnt[Y*W];
unsigned char* src=Frame+Y*sStride;
for (long X = 0; X < W; X++) dst[X]=src[X]<<8;
}
}
if(!Horizontal[0] && !Vertical[0]){
deNoiseTemporal(Frame, FrameDest, FrameAnt,
W, H, sStride, dStride, Temporal);
return;
}
if(!Temporal[0]){
deNoiseSpacial(Frame, FrameDest, LineAnt,
W, H, sStride, dStride, Horizontal, Vertical);
return;
}
LineAnt[0] = PixelAnt = Frame[0]<<16;
PixelDst = LowPassMul(FrameAnt[0]<<8, PixelAnt, Temporal);
FrameAnt[0] = ((PixelDst+0x1000007F)>>8);
FrameDest[0]= ((PixelDst+0x10007FFF)>>16);
for (long X = 1; X < W; X++){
LineAnt[X] = PixelAnt = LowPassMul(PixelAnt, Frame[X]<<16, Horizontal);
PixelDst = LowPassMul(FrameAnt[X]<<8, PixelAnt, Temporal);
FrameAnt[X] = ((PixelDst+0x1000007F)>>8);
FrameDest[X]= ((PixelDst+0x10007FFF)>>16);
}
for (long Y = 1; Y < H; Y++){
unsigned short* LinePrev=&FrameAnt[Y*W];
sLineOffs += sStride, dLineOffs += dStride;
PixelAnt = Frame[sLineOffs]<<16;
LineAnt[0] = LowPassMul(LineAnt[0], PixelAnt, Vertical);
PixelDst = LowPassMul(LinePrev[0]<<8, LineAnt[0], Temporal);
LinePrev[0] = ((PixelDst+0x1000007F)>>8);
FrameDest[dLineOffs]= ((PixelDst+0x10007FFF)>>16);
for (long X = 1; X < W; X++){
PixelAnt = LowPassMul(PixelAnt, Frame[sLineOffs+X]<<16, Horizontal);
LineAnt[X] = LowPassMul(LineAnt[X], PixelAnt, Vertical);
PixelDst = LowPassMul(LinePrev[X]<<8, LineAnt[X], Temporal);
LinePrev[X] = ((PixelDst+0x1000007F)>>8);
FrameDest[dLineOffs+X]= ((PixelDst+0x10007FFF)>>16);
}
}
}
static void PrecalcCoefs(int *Ct, double Dist25)
{
double Gamma, Simil, C;
Gamma = log(0.25) / log(1.0 - Dist25/255.0 - 0.00001);
for (int i = -255*16; i <= 255*16; i++)
{
Simil = 1.0 - abs(i) / (16*255.0);
C = pow(Simil, Gamma) * 65536.0 * (double)i / 16.0;
Ct[16*256+i] = (C<0) ? (C-0.5) : (C+0.5);
}
Ct[0] = (Dist25 != 0);
}
