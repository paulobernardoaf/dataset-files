extern VALUE mX509;
ASN1_TIME *ossl_x509_time_adjust(ASN1_TIME *, VALUE);
void Init_ossl_x509(void);
extern VALUE cX509Attr;
extern VALUE eX509AttrError;
VALUE ossl_x509attr_new(X509_ATTRIBUTE *);
X509_ATTRIBUTE *GetX509AttrPtr(VALUE);
void Init_ossl_x509attr(void);
extern VALUE cX509Cert;
extern VALUE eX509CertError;
VALUE ossl_x509_new(X509 *);
X509 *GetX509CertPtr(VALUE);
X509 *DupX509CertPtr(VALUE);
void Init_ossl_x509cert(void);
extern VALUE cX509CRL;
extern VALUE eX509CRLError;
VALUE ossl_x509crl_new(X509_CRL *);
X509_CRL *GetX509CRLPtr(VALUE);
void Init_ossl_x509crl(void);
extern VALUE cX509Ext;
extern VALUE cX509ExtFactory;
extern VALUE eX509ExtError;
VALUE ossl_x509ext_new(X509_EXTENSION *);
X509_EXTENSION *GetX509ExtPtr(VALUE);
void Init_ossl_x509ext(void);
extern VALUE cX509Name;
extern VALUE eX509NameError;
VALUE ossl_x509name_new(X509_NAME *);
X509_NAME *GetX509NamePtr(VALUE);
void Init_ossl_x509name(void);
extern VALUE cX509Req;
extern VALUE eX509ReqError;
X509_REQ *GetX509ReqPtr(VALUE);
void Init_ossl_x509req(void);
extern VALUE cX509Rev;
extern VALUE eX509RevError;
VALUE ossl_x509revoked_new(X509_REVOKED *);
X509_REVOKED *DupX509RevokedPtr(VALUE);
void Init_ossl_x509revoked(void);
extern VALUE cX509Store;
extern VALUE cX509StoreContext;
extern VALUE eX509StoreError;
X509_STORE *GetX509StorePtr(VALUE);
void Init_ossl_x509store(void);
int ossl_verify_cb_call(VALUE, int, X509_STORE_CTX *);
