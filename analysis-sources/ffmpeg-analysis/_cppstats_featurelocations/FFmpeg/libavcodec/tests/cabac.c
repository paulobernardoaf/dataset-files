



















#include "libavcodec/cabac.c"

#define SIZE 10240

#include "libavutil/lfg.h"
#include "libavcodec/avcodec.h"

static inline void put_cabac_bit(CABACContext *c, int b){
put_bits(&c->pb, 1, b);
for(;c->outstanding_count; c->outstanding_count--){
put_bits(&c->pb, 1, 1-b);
}
}

static inline void renorm_cabac_encoder(CABACContext *c){
while(c->range < 0x100){

if(c->low<0x100){
put_cabac_bit(c, 0);
}else if(c->low<0x200){
c->outstanding_count++;
c->low -= 0x100;
}else{
put_cabac_bit(c, 1);
c->low -= 0x200;
}

c->range+= c->range;
c->low += c->low;
}
}

static void put_cabac(CABACContext *c, uint8_t * const state, int bit){
int RangeLPS= ff_h264_lps_range[2*(c->range&0xC0) + *state];

if(bit == ((*state)&1)){
c->range -= RangeLPS;
*state = ff_h264_mlps_state[128 + *state];
}else{
c->low += c->range - RangeLPS;
c->range = RangeLPS;
*state= ff_h264_mlps_state[127 - *state];
}

renorm_cabac_encoder(c);
}




static void put_cabac_bypass(CABACContext *c, int bit){
c->low += c->low;

if(bit){
c->low += c->range;
}

if(c->low<0x200){
put_cabac_bit(c, 0);
}else if(c->low<0x400){
c->outstanding_count++;
c->low -= 0x200;
}else{
put_cabac_bit(c, 1);
c->low -= 0x400;
}
}





static int put_cabac_terminate(CABACContext *c, int bit){
c->range -= 2;

if(!bit){
renorm_cabac_encoder(c);
}else{
c->low += c->range;
c->range= 2;

renorm_cabac_encoder(c);

av_assert0(c->low <= 0x1FF);
put_cabac_bit(c, c->low>>9);
put_bits(&c->pb, 2, ((c->low>>7)&3)|1);

flush_put_bits(&c->pb); 
}

return (put_bits_count(&c->pb)+7)>>3;
}

int main(void){
CABACContext c;
uint8_t b[9*SIZE];
uint8_t r[9*SIZE];
int i, ret = 0;
uint8_t state[10]= {0};
AVLFG prng;

av_lfg_init(&prng, 1);
ff_init_cabac_encoder(&c, b, SIZE);

for(i=0; i<SIZE; i++){
if(2*i<SIZE) r[i] = av_lfg_get(&prng) % 7;
else r[i] = (i>>8)&1;
}

for(i=0; i<SIZE; i++){
put_cabac_bypass(&c, r[i]&1);
}

for(i=0; i<SIZE; i++){
put_cabac(&c, state, r[i]&1);
}

i= put_cabac_terminate(&c, 1);
b[i++] = av_lfg_get(&prng);
b[i ] = av_lfg_get(&prng);

ff_init_cabac_decoder(&c, b, SIZE);

memset(state, 0, sizeof(state));

for(i=0; i<SIZE; i++){
if( (r[i]&1) != get_cabac_bypass(&c) ) {
av_log(NULL, AV_LOG_ERROR, "CABAC bypass failure at %d\n", i);
ret = 1;
}
}

for(i=0; i<SIZE; i++){
if( (r[i]&1) != get_cabac_noinline(&c, state) ) {
av_log(NULL, AV_LOG_ERROR, "CABAC failure at %d\n", i);
ret = 1;
}
}
if(!get_cabac_terminate(&c)) {
av_log(NULL, AV_LOG_ERROR, "where's the Terminator?\n");
ret = 1;
}

return ret;
}
