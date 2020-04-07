static char *vlc_from_EIT (const void *buf, size_t length)
{
if (unlikely(length == 0))
return NULL;
char encbuf[12];
const char *encoding = encbuf;
const char *in = buf;
size_t offset = 1;
unsigned char c = *in;
if (c >= 0x20)
{
offset = 0;
encoding = "ISO_6937";
}
else if ((1 << c) & 0x0EFE) 
{
snprintf (encbuf, sizeof (encbuf), "ISO_8859-%u", 4u + c);
}
else switch (c)
{
case 0x10: 
offset = 3;
if (length < 3 || in[1] != 0x00)
return NULL;
c = in[2];
if ((1 << c) & 0xEFFE) 
snprintf (encbuf, sizeof (encbuf), "ISO_8859-%hhu", c);
else
return NULL;
break;
case 0x11: 
case 0x14: 
encoding = "UCS-2BE";
break;
case 0x12:
encoding = "EUC-KR";
break;
case 0x13: 
encoding = "GB2312";
break;
case 0x15:
encoding = "UTF-8";
break;
#if 0
case 0x1F: 
offset = 2;
#endif
default:
return NULL;
}
in += offset;
length -= offset;
char *out = FromCharset (encoding, in, length);
if (out == NULL)
{ 
out = strndup (in, length);
if (unlikely(out == NULL))
return NULL;
EnsureUTF8 (out);
}
length = strlen(out);
for (char *p = strchr (out, '\xC2'); p; p = strchr (p + 1, '\xC2'))
{
if (p[1] == '\x8A')
memcpy (p, "\r\n", 2);
if (p[1] == '\x86' || p[1] == '\x87') {
const size_t n = p - out;
memmove (p, p+2, length - n);
length -= 2;
out[length] = '\0';
if (length == n)
break;
}
}
for (char *p = strchr (out, '\xEE'); p; p = strchr (p + 1, '\xEE'))
{
if (p[1] != '\x82')
continue;
if (p[2] == '\x8A')
memcpy (p, "\r\r\n", 3); 
if (p[2] == '\x86' || p[2] == '\x87') {
const size_t n = p - out;
memmove (p, p+3, length - n);
length -= 3;
out[length] = '\0';
if (length == n)
break;
}
}
return out;
}
