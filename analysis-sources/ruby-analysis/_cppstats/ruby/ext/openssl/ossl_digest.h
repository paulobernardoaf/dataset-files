extern VALUE cDigest;
extern VALUE eDigestError;
const EVP_MD *ossl_evp_get_digestbyname(VALUE);
VALUE ossl_digest_new(const EVP_MD *);
void Init_ossl_digest(void);
