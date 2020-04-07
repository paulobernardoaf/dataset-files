



















#include <stdio.h>
#include <string.h>
#include <math.h>
#include "libavfilter/dnn/dnn_backend_native_layer_maximum.h"

#define EPSON 0.00001

static int test(void)
{
DnnLayerMaximumParams params;
DnnOperand operands[2];
int32_t input_indexes[1];
float input[1*1*2*3] = {
-3, 2.5, 2, -2.1, 7.8, 100
};
float *output;

params.val.y = 2.3;

operands[0].data = input;
operands[0].dims[0] = 1;
operands[0].dims[1] = 1;
operands[0].dims[2] = 2;
operands[0].dims[3] = 3;
operands[1].data = NULL;

input_indexes[0] = 0;
dnn_execute_layer_maximum(operands, input_indexes, 1, &params);

output = operands[1].data;
for (int i = 0; i < sizeof(input) / sizeof(float); i++) {
float expected_output = input[i] > params.val.y ? input[i] : params.val.y;
if (fabs(output[i] - expected_output) > EPSON) {
printf("at index %d, output: %f, expected_output: %f\n", i, output[i], expected_output);
av_freep(&output);
return 1;
}
}

av_freep(&output);
return 0;

}

int main(int argc, char **argv)
{
if (test())
return 1;

return 0;
}
