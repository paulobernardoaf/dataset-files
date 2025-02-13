struct strbuf;
#define GPG_VERIFY_VERBOSE 1
#define GPG_VERIFY_RAW 2
#define GPG_VERIFY_OMIT_STATUS 4
enum signature_trust_level {
TRUST_UNDEFINED,
TRUST_NEVER,
TRUST_MARGINAL,
TRUST_FULLY,
TRUST_ULTIMATE,
};
struct signature_check {
char *payload;
char *gpg_output;
char *gpg_status;
char result;
char *signer;
char *key;
char *fingerprint;
char *primary_key_fingerprint;
enum signature_trust_level trust_level;
};
void signature_check_clear(struct signature_check *sigc);
size_t parse_signature(const char *buf, size_t size);
int sign_buffer(struct strbuf *buffer, struct strbuf *signature,
const char *signing_key);
int git_gpg_config(const char *, const char *, void *);
void set_signing_key(const char *);
const char *get_signing_key(void);
int check_signature(const char *payload, size_t plen,
const char *signature, size_t slen,
struct signature_check *sigc);
void print_signature_buffer(const struct signature_check *sigc,
unsigned flags);
