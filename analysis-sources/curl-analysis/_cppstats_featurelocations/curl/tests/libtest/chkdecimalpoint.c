





















#include "curl_printf.h"

#include <string.h>
#include <locale.h>

#define TOTAL_STR_LEN 4

int main(void)
{
char zero[TOTAL_STR_LEN] = {'\0'};
int chars;

setlocale(LC_NUMERIC, "");
chars = msnprintf(zero, TOTAL_STR_LEN, "%.1f", 0.0);
if((chars == (TOTAL_STR_LEN - 1)) && (strcmp(zero, "0.0") == 0))
return 0;
else
return 1;
}
