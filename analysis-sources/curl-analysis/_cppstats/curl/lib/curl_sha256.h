#if !defined(CURL_DISABLE_CRYPTO_AUTH)
#define SHA256_DIGEST_LENGTH 32
void Curl_sha256it(unsigned char *outbuffer, const unsigned char *input,
const size_t len);
#endif
