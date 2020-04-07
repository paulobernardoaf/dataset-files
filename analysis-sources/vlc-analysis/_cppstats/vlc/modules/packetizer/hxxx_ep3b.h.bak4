



















#include <vlc_bits.h>

static inline uint8_t *hxxx_ep3b_to_rbsp( uint8_t *p, uint8_t *end, unsigned *pi_prev, size_t i_count )
{
for( size_t i=0; i<i_count; i++ )
{
if( ++p >= end )
return p;

*pi_prev = (*pi_prev << 1) | (!*p);

if( *p == 0x03 &&
( p + 1 ) != end ) 
{
if( (*pi_prev & 0x06) == 0x06 )
{
++p;
*pi_prev = ((*pi_prev >> 1) << 1) | (!*p);
}
}
}
return p;
}

#if 0

static inline uint8_t * hxxx_ep3b_to_rbsp(const uint8_t *p_src, size_t i_src, size_t *pi_ret)
{
uint8_t *p_dst;
if(!p_src || !(p_dst = malloc(i_src)))
return NULL;

size_t j = 0;
for (size_t i = 0; i < i_src; i++) {
if (i < i_src - 3 &&
p_src[i] == 0 && p_src[i+1] == 0 && p_src[i+2] == 3) {
p_dst[j++] = 0;
p_dst[j++] = 0;
i += 2;
continue;
}
p_dst[j++] = p_src[i];
}
*pi_ret = j;
return p_dst;
}
#endif


struct hxxx_bsfw_ep3b_ctx_s
{
unsigned i_prev;
size_t i_bytepos;
size_t i_bytesize;
};

static void hxxx_bsfw_ep3b_ctx_init( struct hxxx_bsfw_ep3b_ctx_s *ctx )
{
ctx->i_prev = 0;
ctx->i_bytepos = 0;
ctx->i_bytesize = 0;
}

static size_t hxxx_ep3b_total_size( const uint8_t *p, const uint8_t *p_end )
{

unsigned i_prev = 0;
size_t i = 0;
while( p < p_end )
{
uint8_t *n = hxxx_ep3b_to_rbsp( (uint8_t *)p, (uint8_t *)p_end, &i_prev, 1 );
if( n > p )
++i;
p = n;
}
return i;
}

static size_t hxxx_bsfw_byte_forward_ep3b( bs_t *s, size_t i_count )
{
struct hxxx_bsfw_ep3b_ctx_s *ctx = (struct hxxx_bsfw_ep3b_ctx_s *) s->p_priv;
if( s->p == NULL )
{
ctx->i_bytesize = hxxx_ep3b_total_size( s->p_start, s->p_end );
s->p = s->p_start;
ctx->i_bytepos = 1;
return 1;
}

if( s->p >= s->p_end )
return 0;

s->p = hxxx_ep3b_to_rbsp( s->p, s->p_end, &ctx->i_prev, i_count );
ctx->i_bytepos += i_count;
return i_count;
}

static size_t hxxx_bsfw_byte_pos_ep3b( const bs_t *s )
{
struct hxxx_bsfw_ep3b_ctx_s *ctx = (struct hxxx_bsfw_ep3b_ctx_s *) s->p_priv;
return ctx->i_bytepos;
}

static size_t hxxx_bsfw_byte_remain_ep3b( const bs_t *s )
{
struct hxxx_bsfw_ep3b_ctx_s *ctx = (struct hxxx_bsfw_ep3b_ctx_s *) s->p_priv;
if( ctx->i_bytesize == 0 && s->p_start != s->p_end )
ctx->i_bytesize = hxxx_ep3b_total_size( s->p_start, s->p_end );
return (ctx->i_bytesize > ctx->i_bytepos) ? ctx->i_bytesize - ctx->i_bytepos : 0;
}

static const bs_byte_callbacks_t hxxx_bsfw_ep3b_callbacks =
{
hxxx_bsfw_byte_forward_ep3b,
hxxx_bsfw_byte_pos_ep3b,
hxxx_bsfw_byte_remain_ep3b,
};
