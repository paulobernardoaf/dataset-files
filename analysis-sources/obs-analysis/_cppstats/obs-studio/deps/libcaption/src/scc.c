#include "scc.h"
#include "utf8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static scc_t* scc_relloc(scc_t* scc, unsigned int cc_count)
{
if (0 == scc || scc->cc_aloc < cc_count) {
scc = (scc_t*)realloc(scc, sizeof(scc_t) + ((cc_count * 15 / 10) * sizeof(uint16_t)));
scc->cc_aloc = cc_count;
}
return scc;
}
scc_t* scc_new(int cc_count)
{
scc_t* scc = scc_relloc(0, cc_count);
scc->timestamp = 0.0;
scc->cc_size = 0;
return scc;
}
scc_t* scc_free(scc_t* scc)
{
free(scc);
return NULL;
}
double scc_time_to_timestamp(int hh, int mm, int ss, int ff)
{
return (hh * 3600.0) + (mm * 60.0) + ss + (ff / 29.97);
}
size_t scc_to_608(scc_t** scc, const utf8_char_t* data)
{
size_t llen, size = 0;
int v1 = 0, v2 = 0, hh = 0, mm = 0, ss = 0, ff = 0, cc_data = 0;
if (0 == data) {
return 0;
}
if ((*scc)) {
(*scc)->cc_size = 0;
}
if (2 == sscanf(data, "Scenarist_SCC V%1d.%1d", &v1, &v2)) {
data += 18, size += 18;
if (1 != v1 || 0 != v2) {
return 0;
}
}
for (;;) {
llen = utf8_line_length(data);
if (0 == llen || 0 != utf8_trimmed_length(data, llen)) {
break;
}
data += llen;
size += llen;
}
if (4 == sscanf(data, "%2d:%2d:%2d%*1[:;]%2d", &hh, &mm, &ss, &ff)) {
data += 12, size += 12;
llen = utf8_line_length(data);
llen = utf8_trimmed_length(data, llen);
unsigned int max_cc_count = 1 + ((unsigned int)llen / 5);
(*scc) = scc_relloc((*scc), max_cc_count * 15 / 10);
(*scc)->timestamp = scc_time_to_timestamp(hh, mm, ss, ff);
(*scc)->cc_size = 0;
while ((*scc)->cc_size < max_cc_count && 1 == sscanf(data, "%04x", &cc_data)) {
(*scc)->cc_data[(*scc)->cc_size] = (uint16_t)cc_data;
(*scc)->cc_size += 1;
data += 5, size += 5;
}
}
return size;
}
