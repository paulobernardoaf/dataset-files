




























#include "avcodec.h"
#include "idctdsp.h"
#include "mpegvideo.h"
#include "msmpeg4.h"
#include "libavutil/x86/asm.h"
#include "h263.h"
#include "mpeg4video.h"
#include "msmpeg4data.h"
#include "mpegvideodata.h"
#include "vc1data.h"
#include "libavutil/imgutils.h"











static av_cold void init_h263_dc_for_msmpeg4(void)
{
int level, uni_code, uni_len;

if(ff_v2_dc_chroma_table[255 + 256][1])
return;

for(level=-256; level<256; level++){
int size, v, l;

size = 0;
v = abs(level);
while (v) {
v >>= 1;
size++;
}

if (level < 0)
l= (-level) ^ ((1 << size) - 1);
else
l= level;


uni_code= ff_mpeg4_DCtab_lum[size][0];
uni_len = ff_mpeg4_DCtab_lum[size][1];
uni_code ^= (1<<uni_len)-1; 

if (size > 0) {
uni_code<<=size; uni_code|=l;
uni_len+=size;
if (size > 8){
uni_code<<=1; uni_code|=1;
uni_len++;
}
}
ff_v2_dc_lum_table[level + 256][0] = uni_code;
ff_v2_dc_lum_table[level + 256][1] = uni_len;


uni_code= ff_mpeg4_DCtab_chrom[size][0];
uni_len = ff_mpeg4_DCtab_chrom[size][1];
uni_code ^= (1<<uni_len)-1; 

if (size > 0) {
uni_code<<=size; uni_code|=l;
uni_len+=size;
if (size > 8){
uni_code<<=1; uni_code|=1;
uni_len++;
}
}
ff_v2_dc_chroma_table[level + 256][0] = uni_code;
ff_v2_dc_chroma_table[level + 256][1] = uni_len;

}
}

av_cold void ff_msmpeg4_common_init(MpegEncContext *s)
{
switch(s->msmpeg4_version){
case 1:
case 2:
s->y_dc_scale_table=
s->c_dc_scale_table= ff_mpeg1_dc_scale_table;
break;
case 3:
if(s->workaround_bugs){
s->y_dc_scale_table= ff_old_ff_y_dc_scale_table;
s->c_dc_scale_table= ff_wmv1_c_dc_scale_table;
} else{
s->y_dc_scale_table= ff_mpeg4_y_dc_scale_table;
s->c_dc_scale_table= ff_mpeg4_c_dc_scale_table;
}
break;
case 4:
case 5:
s->y_dc_scale_table= ff_wmv1_y_dc_scale_table;
s->c_dc_scale_table= ff_wmv1_c_dc_scale_table;
break;
#if CONFIG_VC1_DECODER
case 6:
s->y_dc_scale_table= ff_wmv3_dc_scale_table;
s->c_dc_scale_table= ff_wmv3_dc_scale_table;
break;
#endif

}


if(s->msmpeg4_version>=4){
ff_init_scantable(s->idsp.idct_permutation, &s->intra_scantable, ff_wmv1_scantable[1]);
ff_init_scantable(s->idsp.idct_permutation, &s->intra_h_scantable, ff_wmv1_scantable[2]);
ff_init_scantable(s->idsp.idct_permutation, &s->intra_v_scantable, ff_wmv1_scantable[3]);
ff_init_scantable(s->idsp.idct_permutation, &s->inter_scantable, ff_wmv1_scantable[0]);
}


init_h263_dc_for_msmpeg4();
}


int ff_msmpeg4_coded_block_pred(MpegEncContext * s, int n, uint8_t **coded_block_ptr)
{
int xy, wrap, pred, a, b, c;

xy = s->block_index[n];
wrap = s->b8_stride;




a = s->coded_block[xy - 1 ];
b = s->coded_block[xy - 1 - wrap];
c = s->coded_block[xy - wrap];

if (b == c) {
pred = a;
} else {
pred = c;
}


*coded_block_ptr = &s->coded_block[xy];

return pred;
}

static int get_dc(uint8_t *src, int stride, int scale, int block_size)
{
int y;
int sum=0;
for(y=0; y<block_size; y++){
int x;
for(x=0; x<block_size; x++){
sum+=src[x + y*stride];
}
}
return FASTDIV((sum + (scale>>1)), scale);
}


int ff_msmpeg4_pred_dc(MpegEncContext *s, int n,
int16_t **dc_val_ptr, int *dir_ptr)
{
int a, b, c, wrap, pred, scale;
int16_t *dc_val;


if (n < 4) {
scale = s->y_dc_scale;
} else {
scale = s->c_dc_scale;
}

wrap = s->block_wrap[n];
dc_val= s->dc_val[0] + s->block_index[n];




a = dc_val[ - 1];
b = dc_val[ - 1 - wrap];
c = dc_val[ - wrap];

if(s->first_slice_line && (n&2)==0 && s->msmpeg4_version<4){
b=c=1024;
}





#if ARCH_X86 && HAVE_7REGS && HAVE_EBX_AVAILABLE
__asm__ volatile(
"movl %3, %%eax \n\t"
"shrl $1, %%eax \n\t"
"addl %%eax, %2 \n\t"
"addl %%eax, %1 \n\t"
"addl %0, %%eax \n\t"
"imull %4 \n\t"
"movl %%edx, %0 \n\t"
"movl %1, %%eax \n\t"
"imull %4 \n\t"
"movl %%edx, %1 \n\t"
"movl %2, %%eax \n\t"
"imull %4 \n\t"
"movl %%edx, %2 \n\t"
: "+b" (a), "+c" (b), "+D" (c)
: "g" (scale), "S" (ff_inverse[scale])
: "%eax", "%edx"
);
#else

if (scale == 8) {
a = (a + (8 >> 1)) / 8;
b = (b + (8 >> 1)) / 8;
c = (c + (8 >> 1)) / 8;
} else {
a = FASTDIV((a + (scale >> 1)), scale);
b = FASTDIV((b + (scale >> 1)), scale);
c = FASTDIV((c + (scale >> 1)), scale);
}
#endif


if(s->msmpeg4_version>3){
if(s->inter_intra_pred){
uint8_t *dest;
int wrap;

if(n==1){
pred=a;
*dir_ptr = 0;
}else if(n==2){
pred=c;
*dir_ptr = 1;
}else if(n==3){
if (abs(a - b) < abs(b - c)) {
pred = c;
*dir_ptr = 1;
} else {
pred = a;
*dir_ptr = 0;
}
}else{
int bs = 8 >> s->avctx->lowres;
if(n<4){
wrap= s->linesize;
dest= s->current_picture.f->data[0] + (((n >> 1) + 2*s->mb_y) * bs* wrap ) + ((n & 1) + 2*s->mb_x) * bs;
}else{
wrap= s->uvlinesize;
dest= s->current_picture.f->data[n - 3] + (s->mb_y * bs * wrap) + s->mb_x * bs;
}
if(s->mb_x==0) a= (1024 + (scale>>1))/scale;
else a= get_dc(dest-bs, wrap, scale*8>>(2*s->avctx->lowres), bs);
if(s->mb_y==0) c= (1024 + (scale>>1))/scale;
else c= get_dc(dest-bs*wrap, wrap, scale*8>>(2*s->avctx->lowres), bs);

if (s->h263_aic_dir==0) {
pred= a;
*dir_ptr = 0;
}else if (s->h263_aic_dir==1) {
if(n==0){
pred= c;
*dir_ptr = 1;
}else{
pred= a;
*dir_ptr = 0;
}
}else if (s->h263_aic_dir==2) {
if(n==0){
pred= a;
*dir_ptr = 0;
}else{
pred= c;
*dir_ptr = 1;
}
} else {
pred= c;
*dir_ptr = 1;
}
}
}else{
if (abs(a - b) < abs(b - c)) {
pred = c;
*dir_ptr = 1;
} else {
pred = a;
*dir_ptr = 0;
}
}
}else{
if (abs(a - b) <= abs(b - c)) {
pred = c;
*dir_ptr = 1;
} else {
pred = a;
*dir_ptr = 0;
}
}


*dc_val_ptr = &dc_val[0];
return pred;
}

