extern VALUE cCipher;
extern VALUE eCipherError;
const EVP_CIPHER *ossl_evp_get_cipherbyname(VALUE);
VALUE ossl_cipher_new(const EVP_CIPHER *);
void Init_ossl_cipher(void);
