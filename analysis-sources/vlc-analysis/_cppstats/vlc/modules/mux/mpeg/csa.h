typedef struct csa_t csa_t;
#define csa_New __csa_New
#define csa_Delete __csa_Delete
#define csa_SetCW __csa_SetCW
#define csa_UseKey __csa_UseKey
#define csa_Decrypt __csa_decrypt
#define csa_Encrypt __csa_encrypt
csa_t *csa_New( void );
void csa_Delete( csa_t * );
int csa_SetCW( vlc_object_t *p_caller, csa_t *c, char *psz_ck, bool odd );
int csa_UseKey( vlc_object_t *p_caller, csa_t *, bool use_odd );
void csa_Decrypt( csa_t *, uint8_t *pkt, int i_pkt_size );
void csa_Encrypt( csa_t *, uint8_t *pkt, int i_pkt_size );
