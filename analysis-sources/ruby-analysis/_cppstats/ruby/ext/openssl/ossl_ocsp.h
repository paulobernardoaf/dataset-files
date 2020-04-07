#if !defined(OPENSSL_NO_OCSP)
extern VALUE mOCSP;
extern VALUE cOCSPReq;
extern VALUE cOCSPRes;
extern VALUE cOCSPBasicRes;
#endif
void Init_ossl_ocsp(void);
