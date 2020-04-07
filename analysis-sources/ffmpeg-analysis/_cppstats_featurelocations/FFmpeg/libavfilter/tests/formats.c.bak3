



















#include "libavfilter/formats.c"

#undef printf

int main(void)
{
const int64_t *cl;
char buf[512];
int i;
const char *teststrings[] ={
"blah",
"1",
"2",
"-1",
"60",
"65",
"1c",
"2c",
"-1c",
"60c",
"65c",
"2C",
"60C",
"65C",
"5.1",
"stereo",
"1+1+1+1",
"1c+1c+1c+1c",
"2c+1c",
"0x3",
};

for (cl = avfilter_all_channel_layouts; *cl != -1; cl++) {
av_get_channel_layout_string(buf, sizeof(buf), -1, *cl);
printf("%s\n", buf);
}

for ( i = 0; i<FF_ARRAY_ELEMS(teststrings); i++) {
int64_t layout = -1;
int count = -1;
int ret;
ret = ff_parse_channel_layout(&layout, &count, teststrings[i], NULL);

printf ("%d = ff_parse_channel_layout(%016"PRIX64", %2d, %s);\n", ret ? -1 : 0, layout, count, teststrings[i]);
}

return 0;
}
