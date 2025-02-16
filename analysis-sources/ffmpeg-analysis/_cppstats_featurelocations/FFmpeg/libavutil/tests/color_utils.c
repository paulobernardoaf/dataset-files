



















#include "libavutil/color_utils.c"

int main(int argc, char *argv[])
{
int i, j;
static const double test_data[] = {
-0.1, -0.018053968510807, -0.01, -0.00449, 0.0, 0.00316227760, 0.005,
0.009, 0.015, 0.1, 1.0, 52.37, 125.098765, 1999.11123, 6945.443,
15123.4567, 19845.88923, 98678.4231, 99999.899998
};

for(i = 0; i < AVCOL_TRC_NB; i++) {
avpriv_trc_function func = avpriv_get_trc_function_from_trc(i);
for(j = 0; j < FF_ARRAY_ELEMS(test_data); j++) {
if(func != NULL) {
double result = func(test_data[j]);
printf("AVColorTransferCharacteristic=%d calling func(%f) expected=%f\n",
i, test_data[j], result);
}
}
}

}
