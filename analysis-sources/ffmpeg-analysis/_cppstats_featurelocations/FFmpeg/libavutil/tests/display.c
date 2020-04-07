



















#include "libavutil/display.c"

static void print_matrix(int32_t matrix[9])
{
int i, j;

for (i = 0; i < 3; ++i) {
for (j = 0; j < 3 - 1; ++j)
printf("%d ", matrix[i*3 + j]);

printf("%d\n", matrix[i*3 + j]);
}
}

int main(void)
{
int32_t matrix[9];


av_display_rotation_set(matrix, 90);
print_matrix(matrix);
printf("degrees: %f\n", av_display_rotation_get(matrix));


av_display_rotation_set(matrix, -45);
print_matrix(matrix);
printf("degrees: %f\n", av_display_rotation_get(matrix));


av_display_matrix_flip(matrix, 1, 0);
print_matrix(matrix);
printf("degrees: %f\n", av_display_rotation_get(matrix));


av_display_matrix_flip(matrix, 0, 1);
print_matrix(matrix);
printf("degrees: %f\n", av_display_rotation_get(matrix));

return 0;

}
