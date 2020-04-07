








#if !defined(_OSSL_BN_H_)
#define _OSSL_BN_H_

extern VALUE cBN;
extern VALUE eBNError;

extern BN_CTX *ossl_bn_ctx;

#define GetBNPtr(obj) ossl_bn_value_ptr(&(obj))

VALUE ossl_bn_new(const BIGNUM *);
BIGNUM *ossl_bn_value_ptr(volatile VALUE *);
void Init_ossl_bn(void);


#endif 
