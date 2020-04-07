



















#if defined(__ANDROID__) || defined(_WIN32)
#define CRYPTFILE

struct crypt
{
void * p_ctx;
size_t (*pf_encrypt)(vlc_keystore *, void *, const uint8_t *, size_t, uint8_t **);
size_t (*pf_decrypt)(vlc_keystore *, void *, const uint8_t *, size_t, uint8_t **);
void (*pf_clean)(vlc_keystore *, void *);
};

int CryptInit(vlc_keystore *, struct crypt *);

#endif
