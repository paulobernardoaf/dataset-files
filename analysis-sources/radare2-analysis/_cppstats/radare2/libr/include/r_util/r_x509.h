#if defined(__cplusplus)
extern "C" {
#endif
typedef struct r_x509_validity_t {
RASN1String *notBefore;
RASN1String *notAfter;
} RX509Validity;
typedef struct r_x509_name_t {
ut32 length;
RASN1String **oids;
RASN1String **names;
} RX509Name;
typedef struct r_x509_algorithmidentifier_t {
RASN1String *algorithm; 
RASN1String *parameters; 
} RX509AlgorithmIdentifier;
typedef struct r_x509_authoritykeyidentifier_t {
RASN1Binary *keyIdentifier;
RX509Name authorityCertIssuer;
RASN1Binary *authorityCertSerialNumber;
} RX509AuthorityKeyIdentifier;
typedef struct r_x509_subjectpublickeyinfo_t {
RX509AlgorithmIdentifier algorithm;
RASN1Binary *subjectPublicKey; 
RASN1Binary *subjectPublicKeyExponent;
RASN1Binary *subjectPublicKeyModule;
} RX509SubjectPublicKeyInfo;
typedef struct r_x509_extension_t {
RASN1String *extnID; 
bool critical;
RASN1Binary *extnValue; 
} RX509Extension;
typedef struct r_x509_extensions_t {
ut32 length;
RX509Extension **extensions;
} RX509Extensions;
typedef struct r_x509_tbscertificate_t {
ut32 version; 
RASN1String *serialNumber; 
RX509AlgorithmIdentifier signature;
RX509Name issuer;
RX509Validity validity;
RX509Name subject;
RX509SubjectPublicKeyInfo subjectPublicKeyInfo;
RASN1Binary *issuerUniqueID; 
RASN1Binary *subjectUniqueID; 
RX509Extensions extensions;
} RX509TBSCertificate;
typedef struct r_x509_certificate_t {
RX509TBSCertificate tbsCertificate;
RX509AlgorithmIdentifier algorithmIdentifier;
RASN1Binary *signature; 
} RX509Certificate;
typedef struct r_x509_crlentry {
RASN1Binary *userCertificate; 
RASN1String *revocationDate; 
} RX509CRLEntry;
typedef struct r_x509_certificaterevocationlist {
RX509AlgorithmIdentifier signature;
RX509Name issuer;
RASN1String *lastUpdate; 
RASN1String *nextUpdate; 
ut32 length;
RX509CRLEntry **revokedCertificates;
} RX509CertificateRevocationList;
R_API RX509CertificateRevocationList* r_x509_parse_crl(RASN1Object *object);
R_API char *r_x509_crl_to_string(RX509CertificateRevocationList *crl, const char* pad);
R_API void r_x509_crl_json(PJ* pj, RX509CertificateRevocationList *crl);
R_API RX509Certificate *r_x509_parse_certificate(RASN1Object *object);
R_API RX509Certificate *r_x509_parse_certificate2(const ut8 *buffer, ut32 length);
R_API void r_x509_free_certificate(RX509Certificate* certificate);
R_API char *r_x509_certificate_to_string(RX509Certificate* certificate, const char* pad);
R_API void r_x509_certificate_json(PJ* pj, RX509Certificate *certificate);
R_API void r_x509_certificate_dump(RX509Certificate* cert, const char* pad, RStrBuf *sb);
#if defined(__cplusplus)
}
#endif
