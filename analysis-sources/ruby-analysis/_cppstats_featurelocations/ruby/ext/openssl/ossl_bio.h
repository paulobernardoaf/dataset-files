








#if !defined(_OSSL_BIO_H_)
#define _OSSL_BIO_H_

BIO *ossl_obj2bio(volatile VALUE *);
VALUE ossl_membio2str(BIO*);

#endif
