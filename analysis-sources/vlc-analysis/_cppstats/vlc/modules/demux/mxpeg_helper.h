static uint8_t find_jpeg_marker(int *position, const uint8_t *data, int size)
{
for (int i = *position; i + 1 < size; i++) {
if (data[i] != 0xff)
continue;
if (data[i + 1] != 0xff) {
*position = i + 2;
return data[i + 1];
}
}
return 0xff;
}
static bool IsMxpeg(stream_t *s)
{
const uint8_t *header;
int size = vlc_stream_Peek(s, &header, 256);
int position = 0;
if (find_jpeg_marker(&position, header, size) != 0xd8 || position > size-2)
return false;
if (find_jpeg_marker(&position, header, position + 2) != 0xe0)
return false;
if (position + 2 > size)
return false;
uint32_t header_size = GetWBE(&header[position]);
position += header_size;
if (position + 6 > size)
{
size = position + 6;
if( vlc_stream_Peek (s, &header, size) < size )
return false;
}
if ( !(header[position] == 0xFF && header[position+1] == 0xFE) )
return false;
position += 2;
header_size = GetWBE (&header[position]);
if (!memcmp (&header[position+2], "MXF\0", 4) )
return true;
size = position + header_size + 8; 
if (vlc_stream_Peek(s, &header, size ) < size)
return false;
position += header_size;
if ( !(header[position] == 0xFF && header[position+1] == 0xFE) )
return false;
position += 4;
if (memcmp (&header[position], "MXF\0", 4) )
return false;
return true;
}
