#include "curl_setup.h"
#include "timeval.h"
void Curl_speedinit(struct Curl_easy *data);
CURLcode Curl_speedcheck(struct Curl_easy *data,
struct curltime now);
