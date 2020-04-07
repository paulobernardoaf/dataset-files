



















#include "libavformat/network.h"

static void test(const char *pattern, const char *host)
{
int res = ff_http_match_no_proxy(pattern, host);
printf("The pattern \"%s\" %s the hostname %s\n",
pattern ? pattern : "(null)", res ? "matches" : "does not match",
host);
}

int main(void)
{
test(NULL, "domain.com");
test("example.com domain.com", "domain.com");
test("example.com other.com", "domain.com");
test("example.com,domain.com", "domain.com");
test("example.com,domain.com", "otherdomain.com");
test("example.com, *.domain.com", "sub.domain.com");
test("example.com, *.domain.com", "domain.com");
test("example.com, .domain.com", "domain.com");
test("*", "domain.com");
return 0;
}
