VALUE asn1time_to_time(const ASN1_TIME *);
void ossl_time_split(VALUE, time_t *, int *);
VALUE asn1str_to_str(const ASN1_STRING *);
VALUE asn1integer_to_num(const ASN1_INTEGER *);
ASN1_INTEGER *num_to_asn1integer(VALUE, ASN1_INTEGER *);
extern VALUE mASN1;
extern VALUE eASN1Error;
extern VALUE cASN1Data;
extern VALUE cASN1Primitive;
extern VALUE cASN1Constructive;
extern VALUE cASN1Boolean; 
extern VALUE cASN1Integer, cASN1Enumerated; 
extern VALUE cASN1BitString; 
extern VALUE cASN1OctetString, cASN1UTF8String; 
extern VALUE cASN1NumericString, cASN1PrintableString;
extern VALUE cASN1T61String, cASN1VideotexString;
extern VALUE cASN1IA5String, cASN1GraphicString;
extern VALUE cASN1ISO64String, cASN1GeneralString;
extern VALUE cASN1UniversalString, cASN1BMPString;
extern VALUE cASN1Null; 
extern VALUE cASN1ObjectId; 
extern VALUE cASN1UTCTime, cASN1GeneralizedTime; 
extern VALUE cASN1Sequence, cASN1Set; 
ASN1_TYPE *ossl_asn1_get_asn1type(VALUE);
void Init_ossl_asn1(void);
