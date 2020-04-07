


#include <r_egg.h>

#define DEFAULT_XOR_KEY "0xFF"

static RBuffer *build (REgg *egg) {
RBuffer *buf, *sc;
ut8 aux[32], nkey;
const char *default_key = DEFAULT_XOR_KEY;
char *key = r_egg_option_get (egg, "key");
int i;

if (!key || !*key) {
free (key);
key = strdup (default_key);
eprintf ("XOR key not provided. Using (%s) as the key\n", key);
}
nkey = r_num_math (NULL, key);
if (nkey == 0) {
eprintf ("Invalid key (%s)\n", key);
free (key);
return false;
}
if (nkey != (nkey & 0xff)) {
nkey &= 0xff;
eprintf ("xor key wrapped to (%d)\n", nkey);
}
if (r_buf_size (egg->bin) > 240) { 
eprintf ("shellcode is too long :(\n");
free (key);
return NULL;
}
sc = egg->bin; 
if (!r_buf_size (sc)) {
eprintf ("No shellcode found!\n");
free (key);
return NULL;
}

for (i = 0; i<r_buf_size (sc); i++) {

if ((r_buf_read8_at (sc, i) ^ nkey)==0) {
eprintf ("This xor key generates null bytes. Try again.\n");
free (key);
return NULL;
}
}
buf = r_buf_new ();
sc = r_buf_new ();



r_buf_append_buf (sc, egg->bin);
if (egg->arch == R_SYS_ARCH_X86) {
#define STUBLEN 18
ut8 stub[STUBLEN] =
"\xe8\xff\xff\xff\xff" 
"\xc1" 
"\x5e" 
"\x48\x83\xc6\x0d" 

"\x30\x1e" 
"\x48\xff\xc6" 
"\xe2\xf9"; 

aux[0] = 0x6a; 
aux[1] = r_buf_size (sc);
aux[2] = 0x59; 

aux[3] = 0x6a; 
aux[4] = nkey;
aux[5] = 0x5b; 
r_buf_set_bytes (buf, aux, 6);

r_buf_append_bytes (buf, stub, STUBLEN);

for (i = 0; i<r_buf_size (sc); i++) {
ut8 v = r_buf_read8_at (sc, i) ^ nkey;
r_buf_write_at (sc, i, &v, sizeof (v));
}
r_buf_append_buf (buf, sc);
}
r_buf_free (sc);
free (key);
return buf;
}


REggPlugin r_egg_plugin_xor = {
.name = "xor",
.type = R_EGG_PLUGIN_ENCODER,
.desc = "xor encoder for shellcode",
.build = (void *)build
};

#if 0
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_EGG,
.data = &r_egg_plugin_xor,
.version = R2_VERSION
};
#endif
#endif
