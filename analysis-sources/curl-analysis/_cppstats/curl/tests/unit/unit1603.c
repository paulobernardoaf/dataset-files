#include "curlcheck.h"
#define ENABLE_CURLX_PRINTF
#include "curlx.h"
#include "hash.h"
#include "memdebug.h" 
static struct curl_hash hash_static;
static const int slots = 3;
static void mydtor(void *p)
{
(void)p; 
}
static CURLcode unit_setup(void)
{
return Curl_hash_init(&hash_static, slots, Curl_hash_str,
Curl_str_key_compare, mydtor);
}
static void unit_stop(void)
{
Curl_hash_destroy(&hash_static);
}
UNITTEST_START
char key1[] = "key1";
char key2[] = "key2b";
char key3[] = "key3";
char key4[] = "key4";
char notakey[] = "notakey";
char *nodep;
int rc;
if(Curl_hash_str(key1, strlen(key1), slots) != 1 ||
Curl_hash_str(key2, strlen(key2), slots) != 0 ||
Curl_hash_str(key3, strlen(key3), slots) != 2 ||
Curl_hash_str(key4, strlen(key4), slots) != 1)
fprintf(stderr, "Warning: hashes are not computed as expected on this "
"architecture; test coverage will be less comprehensive\n");
nodep = Curl_hash_add(&hash_static, &key1, strlen(key1), &key1);
fail_unless(nodep, "insertion into hash failed");
nodep = Curl_hash_pick(&hash_static, &key1, strlen(key1));
fail_unless(nodep == key1, "hash retrieval failed");
nodep = Curl_hash_add(&hash_static, &key2, strlen(key2), &key2);
fail_unless(nodep, "insertion into hash failed");
nodep = Curl_hash_pick(&hash_static, &key2, strlen(key2));
fail_unless(nodep == key2, "hash retrieval failed");
nodep = Curl_hash_add(&hash_static, &key3, strlen(key3), &key3);
fail_unless(nodep, "insertion into hash failed");
nodep = Curl_hash_pick(&hash_static, &key3, strlen(key3));
fail_unless(nodep == key3, "hash retrieval failed");
nodep = Curl_hash_add(&hash_static, &key4, strlen(key4), &key4);
fail_unless(nodep, "insertion into hash failed");
nodep = Curl_hash_pick(&hash_static, &key4, strlen(key4));
fail_unless(nodep == key4, "hash retrieval failed");
nodep = Curl_hash_pick(&hash_static, &key1, strlen(key1));
fail_unless(nodep == key1, "hash retrieval failed");
nodep = Curl_hash_pick(&hash_static, &key2, strlen(key2));
fail_unless(nodep == key2, "hash retrieval failed");
nodep = Curl_hash_pick(&hash_static, &key3, strlen(key3));
fail_unless(nodep == key3, "hash retrieval failed");
nodep = Curl_hash_pick(&hash_static, &key4, strlen(key4));
fail_unless(nodep == key4, "hash retrieval failed");
rc = Curl_hash_delete(&hash_static, &key4, strlen(key4));
fail_unless(rc == 0, "hash delete failed");
nodep = Curl_hash_pick(&hash_static, &key1, strlen(key1));
fail_unless(nodep == key1, "hash retrieval failed");
nodep = Curl_hash_pick(&hash_static, &key4, strlen(key4));
fail_unless(!nodep, "hash retrieval should have failed");
nodep = Curl_hash_add(&hash_static, &key4, strlen(key4), &key4);
fail_unless(nodep, "insertion into hash failed");
nodep = Curl_hash_pick(&hash_static, &key4, strlen(key4));
fail_unless(nodep == key4, "hash retrieval failed");
rc = Curl_hash_delete(&hash_static, &key1, strlen(key1));
fail_unless(rc == 0, "hash delete failed");
nodep = Curl_hash_pick(&hash_static, &key1, strlen(key1));
fail_unless(!nodep, "hash retrieval should have failed");
nodep = Curl_hash_pick(&hash_static, &key4, strlen(key4));
fail_unless(nodep == key4, "hash retrieval failed");
rc = Curl_hash_delete(&hash_static, &key4, strlen(key4));
fail_unless(rc == 0, "hash delete failed");
nodep = Curl_hash_pick(&hash_static, &key1, strlen(key1));
fail_unless(!nodep, "hash retrieval should have failed");
nodep = Curl_hash_pick(&hash_static, &key4, strlen(key4));
fail_unless(!nodep, "hash retrieval should have failed");
rc = Curl_hash_delete(&hash_static, &key4, strlen(key4));
fail_unless(rc, "hash delete should have failed");
nodep = Curl_hash_add(&hash_static, &key1, strlen(key1), &notakey);
fail_unless(nodep, "insertion into hash failed");
nodep = Curl_hash_pick(&hash_static, &key1, strlen(key1));
fail_unless(nodep == notakey, "hash retrieval failed");
nodep = Curl_hash_pick(&hash_static, &key2, strlen(key2));
fail_unless(nodep == key2, "hash retrieval failed");
nodep = Curl_hash_pick(&hash_static, &key3, strlen(key3));
fail_unless(nodep == key3, "hash retrieval failed");
Curl_hash_clean(&hash_static);
UNITTEST_STOP
