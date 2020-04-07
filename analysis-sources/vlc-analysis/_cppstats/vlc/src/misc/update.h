#include <stdatomic.h>
#include <vlc_update.h>
enum 
{
SIGNATURE_PACKET = 0x02,
PUBLIC_KEY_PACKET = 0x06,
USER_ID_PACKET = 0x0d
};
enum 
{
BINARY_SIGNATURE = 0x00,
TEXT_SIGNATURE = 0x01,
GENERIC_KEY_SIGNATURE = 0x10, 
PERSONA_KEY_SIGNATURE = 0x11, 
CASUAL_KEY_SIGNATURE = 0x12, 
POSITIVE_KEY_SIGNATURE = 0x13 
};
enum 
{
ISSUER_SUBPACKET = 0x10
};
struct public_key_packet_t
{
uint8_t version; 
uint8_t timestamp[4]; 
uint8_t algo; 
union {
struct {
uint8_t p[2+3072/8];
uint8_t q[2+256/8];
uint8_t g[2+3072/8];
uint8_t y[2+3072/8];
} dsa ;
struct {
uint8_t n[2+4096/8];
uint8_t e[2+4096/8];
} rsa;
} sig;
};
struct signature_packet_t
{
uint8_t version; 
uint8_t type;
uint8_t public_key_algo; 
uint8_t digest_algo;
uint8_t hash_verification[2];
uint8_t issuer_longid[8];
union 
{
struct
{
uint8_t hashed_data_len[2]; 
uint8_t *hashed_data; 
uint8_t unhashed_data_len[2]; 
uint8_t *unhashed_data; 
} v4;
struct
{
uint8_t hashed_data_len; 
uint8_t timestamp[4]; 
} v3;
} specific;
union {
struct {
uint8_t r[2+256/8];
uint8_t s[2+256/8];
} dsa;
struct {
uint8_t s[2+4096/8];
} rsa;
} algo_specific;
};
typedef struct public_key_packet_t public_key_packet_t;
typedef struct signature_packet_t signature_packet_t;
struct public_key_t
{
uint8_t longid[8]; 
uint8_t *psz_username; 
public_key_packet_t key; 
signature_packet_t sig; 
};
typedef struct public_key_t public_key_t;
typedef struct
{
struct vlc_object_t obj;
vlc_thread_t thread;
atomic_bool aborted;
update_t *p_update;
char *psz_destdir;
} update_download_thread_t;
typedef struct
{
vlc_thread_t thread;
update_t *p_update;
void (*pf_callback)( void *, bool );
void *p_data;
} update_check_thread_t;
struct update_t
{
libvlc_int_t *p_libvlc;
vlc_mutex_t lock;
struct update_release_t release; 
public_key_t *p_pkey;
update_download_thread_t *p_download;
update_check_thread_t *p_check;
};
public_key_t *
download_key(
vlc_object_t *p_this, const uint8_t *p_longid,
const uint8_t *p_signature_issuer );
int
parse_public_key(
const uint8_t *p_key_data, size_t i_key_len, public_key_t *p_key,
const uint8_t *p_sig_issuer );
int
verify_signature(signature_packet_t *sign, public_key_packet_t *p_key,
uint8_t *p_hash );
int
download_signature(
vlc_object_t *p_this, signature_packet_t *p_sig, const char *psz_url );
uint8_t *
hash_from_text(
const char *psz_text, signature_packet_t *p_sig );
uint8_t *
hash_from_file(
const char *psz_file, signature_packet_t *p_sig );
uint8_t *
hash_from_public_key( public_key_t *p_pkey );
