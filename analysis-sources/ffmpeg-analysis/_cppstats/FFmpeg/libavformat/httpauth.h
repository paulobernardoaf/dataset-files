typedef enum HTTPAuthType {
HTTP_AUTH_NONE = 0, 
HTTP_AUTH_BASIC, 
HTTP_AUTH_DIGEST, 
} HTTPAuthType;
typedef struct DigestParams {
char nonce[300]; 
char algorithm[10]; 
char qop[30]; 
char opaque[300]; 
char stale[10]; 
int nc; 
} DigestParams;
typedef struct HTTPAuthState {
int auth_type;
char realm[200];
DigestParams digest_params;
int stale;
} HTTPAuthState;
void ff_http_auth_handle_header(HTTPAuthState *state, const char *key,
const char *value);
char *ff_http_auth_create_response(HTTPAuthState *state, const char *auth,
const char *path, const char *method);
