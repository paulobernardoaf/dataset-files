#include "libavformat/url.h"
#include "libavformat/avformat.h"
static void test(const char *base, const char *rel)
{
char buf[200], buf2[200];
ff_make_absolute_url(buf, sizeof(buf), base, rel);
printf("%50s %-20s => %s\n", base, rel, buf);
if (base) {
snprintf(buf2, sizeof(buf2), "%s", base);
ff_make_absolute_url(buf2, sizeof(buf2), buf2, rel);
if (strcmp(buf, buf2)) {
printf("In-place handling of %s + %s failed\n", base, rel);
exit(1);
}
}
}
static void test2(const char *url)
{
char proto[64];
char auth[256];
char host[256];
char path[256];
int port=-1;
av_url_split(proto, sizeof(proto), auth, sizeof(auth), host, sizeof(host), &port, path, sizeof(path), url);
printf("%-60s => %-15s %-15s %-15s %5d %s\n", url, proto, auth, host, port, path);
}
int main(void)
{
printf("Testing ff_make_absolute_url:\n");
test(NULL, "baz");
test("/foo/bar", "baz");
test("/foo/bar", "../baz");
test("/foo/bar", "/baz");
test("http://server/foo/", "baz");
test("http://server/foo/bar", "baz");
test("http://server/foo/", "../baz");
test("http://server/foo/bar/123", "../../baz");
test("http://server/foo/bar/123", "/baz");
test("http://server/foo/bar/123", "https://other/url");
test("http://server/foo/bar?param=value/with/slashes", "/baz");
test("http://server/foo/bar?param&otherparam", "?someparam");
test("http://server/foo/bar", "//other/url");
printf("\nTesting av_url_split:\n");
test2("/foo/bar");
test2("http://server/foo/");
test2("http://example.com/foo/bar");
test2("http://user:pass@localhost:8080/foo/bar/123");
test2("http://server/foo/bar?param=value/with/slashes");
test2("https://1l-lh.a.net/i/1LIVE_HDS@179577/master.m3u8");
test2("ftp://u:p%2B%2F2@ftp.pbt.com/ExportHD.mpg");
test2("https://key.dns.com?key_id=2&model_id=12345&&access_key=");
test2("http://example.com#tag");
return 0;
}
