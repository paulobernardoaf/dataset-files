unsigned int utf8_seqlen(long codepoint);
#if defined(DEFINE_INLINES) || USE_INLINE
INLINE unsigned int utf8_seqlen(long codepoint)
{
if(codepoint < 0x0000080) return 1;
if(codepoint < 0x0000800) return 2;
if(codepoint < 0x0010000) return 3;
if(codepoint < 0x0200000) return 4;
if(codepoint < 0x4000000) return 5;
return 6;
}
#endif
int fill_utf8(long codepoint, char *str);
#if defined(DEFINE_INLINES) || USE_INLINE
INLINE int fill_utf8(long codepoint, char *str)
{
int nbytes = utf8_seqlen(codepoint);
int b = nbytes;
while(b > 1) {
b--;
str[b] = 0x80 | (codepoint & 0x3f);
codepoint >>= 6;
}
switch(nbytes) {
case 1: str[0] = (codepoint & 0x7f); break;
case 2: str[0] = 0xc0 | (codepoint & 0x1f); break;
case 3: str[0] = 0xe0 | (codepoint & 0x0f); break;
case 4: str[0] = 0xf0 | (codepoint & 0x07); break;
case 5: str[0] = 0xf8 | (codepoint & 0x03); break;
case 6: str[0] = 0xfc | (codepoint & 0x01); break;
}
return nbytes;
}
#endif
