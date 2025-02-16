
























#include <curl/curl.h>
#include "config-os400.h" 

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <qadrt.h>
#include <errno.h>

#if defined(HAVE_ZLIB_H)
#include <zlib.h>
#endif

#if defined(USE_GSKIT)
#include <gskssl.h>
#include <qsoasync.h>
#endif

#if defined(HAVE_GSSAPI)
#include <gssapi.h>
#endif

#if !defined(CURL_DISABLE_LDAP)
#include <ldap.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>

#include "os400sys.h"









#pragma convert(0) 


#define MIN_BYTE_GAIN 1024 

typedef struct {
unsigned long size; 
char * buf; 
} buffer_t;


static char * buffer_undef(localkey_t key, long size);
static char * buffer_threaded(localkey_t key, long size);
static char * buffer_unthreaded(localkey_t key, long size);

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_key_t thdkey;
static buffer_t * locbufs;

char * (* Curl_thread_buffer)(localkey_t key, long size) = buffer_undef;


static void
thdbufdestroy(void * private)

{
if(private) {
buffer_t * p = (buffer_t *) private;
localkey_t i;

for(i = (localkey_t) 0; i < LK_LAST; i++) {
free(p->buf);
p++;
}

free(private);
}
}


static void
terminate(void)

{
if(Curl_thread_buffer == buffer_threaded) {
locbufs = pthread_getspecific(thdkey);
pthread_setspecific(thdkey, (void *) NULL);
pthread_key_delete(thdkey);
}

if(Curl_thread_buffer != buffer_undef) {
thdbufdestroy((void *) locbufs);
locbufs = (buffer_t *) NULL;
}

Curl_thread_buffer = buffer_undef;
}


static char *
get_buffer(buffer_t * buf, long size)

{
char * cp;




if(size < 0)
return buf->buf;

if(!buf->buf) {
buf->buf = malloc(size);
if(buf->buf)
buf->size = size;

return buf->buf;
}

if((unsigned long) size <= buf->size) {



if(buf->size - size < MIN_BYTE_GAIN)
return buf->buf;
}



cp = realloc(buf->buf, size);
if(cp) {
buf->buf = cp;
buf->size = size;
}
else if(size <= buf->size)
cp = buf->buf;

return cp;
}


static char *
buffer_unthreaded(localkey_t key, long size)

{
return get_buffer(locbufs + key, size);
}


static char *
buffer_threaded(localkey_t key, long size)

{
buffer_t * bufs;





bufs = (buffer_t *) pthread_getspecific(thdkey);

if(!bufs) {
if(size < 0)
return (char *) NULL; 



bufs = calloc((size_t) LK_LAST, sizeof(*bufs));
if(!bufs)
return (char *) NULL;

if(pthread_setspecific(thdkey, (void *) bufs)) {
free(bufs);
return (char *) NULL;
}
}

return get_buffer(bufs + key, size);
}


static char *
buffer_undef(localkey_t key, long size)

{




pthread_mutex_lock(&mutex);



if(Curl_thread_buffer == buffer_undef) { 
if(!pthread_key_create(&thdkey, thdbufdestroy))
Curl_thread_buffer = buffer_threaded;
else if(!(locbufs = calloc((size_t) LK_LAST, sizeof(*locbufs)))) {
pthread_mutex_unlock(&mutex);
return (char *) NULL;
}
else
Curl_thread_buffer = buffer_unthreaded;

atexit(terminate);
}

pthread_mutex_unlock(&mutex);
return Curl_thread_buffer(key, size);
}


static char *
set_thread_string(localkey_t key, const char * s)

{
int i;
char * cp;

if(!s)
return (char *) NULL;

i = strlen(s) + 1;
cp = Curl_thread_buffer(key, MAX_CONV_EXPANSION * i + 1);

if(cp) {
i = QadrtConvertE2A(cp, s, MAX_CONV_EXPANSION * i, i);
cp[i] = '\0';
}

return cp;
}


int
Curl_getnameinfo_a(const struct sockaddr * sa, curl_socklen_t salen,
char * nodename, curl_socklen_t nodenamelen,
char * servname, curl_socklen_t servnamelen,
int flags)

{
char *enodename = NULL;
char *eservname = NULL;
int status;

if(nodename && nodenamelen) {
enodename = malloc(nodenamelen);
if(!enodename)
return EAI_MEMORY;
}

if(servname && servnamelen) {
eservname = malloc(servnamelen);
if(!eservname) {
free(enodename);
return EAI_MEMORY;
}
}

status = getnameinfo(sa, salen, enodename, nodenamelen,
eservname, servnamelen, flags);

if(!status) {
int i;
if(enodename) {
i = QadrtConvertE2A(nodename, enodename,
nodenamelen - 1, strlen(enodename));
nodename[i] = '\0';
}

if(eservname) {
i = QadrtConvertE2A(servname, eservname,
servnamelen - 1, strlen(eservname));
servname[i] = '\0';
}
}

free(enodename);
free(eservname);
return status;
}


int
Curl_getaddrinfo_a(const char * nodename, const char * servname,
const struct addrinfo * hints,
struct addrinfo * * res)

{
char * enodename;
char * eservname;
int status;
int i;

enodename = (char *) NULL;
eservname = (char *) NULL;

if(nodename) {
i = strlen(nodename);

enodename = malloc(i + 1);
if(!enodename)
return EAI_MEMORY;

i = QadrtConvertA2E(enodename, nodename, i, i);
enodename[i] = '\0';
}

if(servname) {
i = strlen(servname);

eservname = malloc(i + 1);
if(!eservname) {
free(enodename);
return EAI_MEMORY;
}

QadrtConvertA2E(eservname, servname, i, i);
eservname[i] = '\0';
}

status = getaddrinfo(enodename, eservname, hints, res);
free(enodename);
free(eservname);
return status;
}


#if defined(USE_GSKIT)













struct gskstrlist {
struct gskstrlist * next;
const char * ebcdicstr;
const char * asciistr;
};

struct Curl_gsk_descriptor {
gsk_handle h;
struct gskstrlist * strlist;
};


int
Curl_gsk_environment_open(gsk_handle * my_env_handle)

{
struct Curl_gsk_descriptor * p;
int rc;

if(!my_env_handle)
return GSK_OS400_ERROR_INVALID_POINTER;
p = (struct Curl_gsk_descriptor *) malloc(sizeof(*p));
if(!p)
return GSK_INSUFFICIENT_STORAGE;
p->strlist = (struct gskstrlist *) NULL;
rc = gsk_environment_open(&p->h);
if(rc != GSK_OK)
free(p);
else
*my_env_handle = (gsk_handle) p;
return rc;
}


int
Curl_gsk_secure_soc_open(gsk_handle my_env_handle,
gsk_handle * my_session_handle)

{
struct Curl_gsk_descriptor * p;
gsk_handle h;
int rc;

if(!my_env_handle)
return GSK_INVALID_HANDLE;
if(!my_session_handle)
return GSK_OS400_ERROR_INVALID_POINTER;
h = ((struct Curl_gsk_descriptor *) my_env_handle)->h;
p = (struct Curl_gsk_descriptor *) malloc(sizeof(*p));
if(!p)
return GSK_INSUFFICIENT_STORAGE;
p->strlist = (struct gskstrlist *) NULL;
rc = gsk_secure_soc_open(h, &p->h);
if(rc != GSK_OK)
free(p);
else
*my_session_handle = (gsk_handle) p;
return rc;
}


static void
gsk_free_handle(struct Curl_gsk_descriptor * p)

{
struct gskstrlist * q;

while((q = p->strlist)) {
p->strlist = q;
free((void *) q->asciistr);
free(q);
}
free(p);
}


int
Curl_gsk_environment_close(gsk_handle * my_env_handle)

{
struct Curl_gsk_descriptor * p;
int rc;

if(!my_env_handle)
return GSK_OS400_ERROR_INVALID_POINTER;
if(!*my_env_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) *my_env_handle;
rc = gsk_environment_close(&p->h);
if(rc == GSK_OK) {
gsk_free_handle(p);
*my_env_handle = (gsk_handle) NULL;
}
return rc;
}


int
Curl_gsk_secure_soc_close(gsk_handle * my_session_handle)

{
struct Curl_gsk_descriptor * p;
int rc;

if(!my_session_handle)
return GSK_OS400_ERROR_INVALID_POINTER;
if(!*my_session_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) *my_session_handle;
rc = gsk_secure_soc_close(&p->h);
if(rc == GSK_OK) {
gsk_free_handle(p);
*my_session_handle = (gsk_handle) NULL;
}
return rc;
}


int
Curl_gsk_environment_init(gsk_handle my_env_handle)

{
struct Curl_gsk_descriptor * p;

if(!my_env_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) my_env_handle;
return gsk_environment_init(p->h);
}


int
Curl_gsk_secure_soc_init(gsk_handle my_session_handle)

{
struct Curl_gsk_descriptor * p;

if(!my_session_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) my_session_handle;
return gsk_secure_soc_init(p->h);
}


int
Curl_gsk_attribute_set_buffer_a(gsk_handle my_gsk_handle, GSK_BUF_ID bufID,
const char * buffer, int bufSize)

{
struct Curl_gsk_descriptor * p;
char * ebcdicbuf;
int rc;

if(!my_gsk_handle)
return GSK_INVALID_HANDLE;
if(!buffer)
return GSK_OS400_ERROR_INVALID_POINTER;
if(bufSize < 0)
return GSK_ATTRIBUTE_INVALID_LENGTH;
p = (struct Curl_gsk_descriptor *) my_gsk_handle;
if(!bufSize)
bufSize = strlen(buffer);
ebcdicbuf = malloc(bufSize + 1);
if(!ebcdicbuf)
return GSK_INSUFFICIENT_STORAGE;
QadrtConvertA2E(ebcdicbuf, buffer, bufSize, bufSize);
ebcdicbuf[bufSize] = '\0';
rc = gsk_attribute_set_buffer(p->h, bufID, ebcdicbuf, bufSize);
free(ebcdicbuf);
return rc;
}


int
Curl_gsk_attribute_set_enum(gsk_handle my_gsk_handle, GSK_ENUM_ID enumID,
GSK_ENUM_VALUE enumValue)

{
struct Curl_gsk_descriptor * p;

if(!my_gsk_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) my_gsk_handle;
return gsk_attribute_set_enum(p->h, enumID, enumValue);
}


int
Curl_gsk_attribute_set_numeric_value(gsk_handle my_gsk_handle,
GSK_NUM_ID numID, int numValue)

{
struct Curl_gsk_descriptor * p;

if(!my_gsk_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) my_gsk_handle;
return gsk_attribute_set_numeric_value(p->h, numID, numValue);
}


int
Curl_gsk_attribute_set_callback(gsk_handle my_gsk_handle,
GSK_CALLBACK_ID callBackID,
void * callBackAreaPtr)

{
struct Curl_gsk_descriptor * p;

if(!my_gsk_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) my_gsk_handle;
return gsk_attribute_set_callback(p->h, callBackID, callBackAreaPtr);
}


static int
cachestring(struct Curl_gsk_descriptor * p,
const char * ebcdicbuf, int bufsize, const char * * buffer)

{
int rc;
char * asciibuf;
struct gskstrlist * sp;

for(sp = p->strlist; sp; sp = sp->next)
if(sp->ebcdicstr == ebcdicbuf)
break;
if(!sp) {
sp = (struct gskstrlist *) malloc(sizeof(*sp));
if(!sp)
return GSK_INSUFFICIENT_STORAGE;
asciibuf = malloc(bufsize + 1);
if(!asciibuf) {
free(sp);
return GSK_INSUFFICIENT_STORAGE;
}
QadrtConvertE2A(asciibuf, ebcdicbuf, bufsize, bufsize);
asciibuf[bufsize] = '\0';
sp->ebcdicstr = ebcdicbuf;
sp->asciistr = asciibuf;
sp->next = p->strlist;
p->strlist = sp;
}
*buffer = sp->asciistr;
return GSK_OK;
}


int
Curl_gsk_attribute_get_buffer_a(gsk_handle my_gsk_handle, GSK_BUF_ID bufID,
const char * * buffer, int * bufSize)

{
struct Curl_gsk_descriptor * p;
int rc;
const char * mybuf;
int mylen;

if(!my_gsk_handle)
return GSK_INVALID_HANDLE;
if(!buffer || !bufSize)
return GSK_OS400_ERROR_INVALID_POINTER;
p = (struct Curl_gsk_descriptor *) my_gsk_handle;
rc = gsk_attribute_get_buffer(p->h, bufID, &mybuf, &mylen);
if(rc != GSK_OK)
return rc;
rc = cachestring(p, mybuf, mylen, buffer);
if(rc == GSK_OK)
*bufSize = mylen;
return rc;
}


int
Curl_gsk_attribute_get_enum(gsk_handle my_gsk_handle, GSK_ENUM_ID enumID,
GSK_ENUM_VALUE * enumValue)

{
struct Curl_gsk_descriptor * p;

if(!my_gsk_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) my_gsk_handle;
return gsk_attribute_get_enum(p->h, enumID, enumValue);
}


int
Curl_gsk_attribute_get_numeric_value(gsk_handle my_gsk_handle,
GSK_NUM_ID numID, int * numValue)

{
struct Curl_gsk_descriptor * p;

if(!my_gsk_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) my_gsk_handle;
return gsk_attribute_get_numeric_value(p->h, numID, numValue);
}


int
Curl_gsk_attribute_get_cert_info(gsk_handle my_gsk_handle,
GSK_CERT_ID certID,
const gsk_cert_data_elem * * certDataElem,
int * certDataElementCount)

{
struct Curl_gsk_descriptor * p;

if(!my_gsk_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) my_gsk_handle;

return gsk_attribute_get_cert_info(p->h, certID,
certDataElem, certDataElementCount);
}


int
Curl_gsk_secure_soc_misc(gsk_handle my_session_handle, GSK_MISC_ID miscID)

{
struct Curl_gsk_descriptor * p;

if(!my_session_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) my_session_handle;
return gsk_secure_soc_misc(p->h, miscID);
}


int
Curl_gsk_secure_soc_read(gsk_handle my_session_handle, char * readBuffer,
int readBufSize, int * amtRead)

{
struct Curl_gsk_descriptor * p;

if(!my_session_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) my_session_handle;
return gsk_secure_soc_read(p->h, readBuffer, readBufSize, amtRead);
}


int
Curl_gsk_secure_soc_write(gsk_handle my_session_handle, char * writeBuffer,
int writeBufSize, int * amtWritten)

{
struct Curl_gsk_descriptor * p;

if(!my_session_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) my_session_handle;
return gsk_secure_soc_write(p->h, writeBuffer, writeBufSize, amtWritten);
}


const char *
Curl_gsk_strerror_a(int gsk_return_value)

{
return set_thread_string(LK_GSK_ERROR, gsk_strerror(gsk_return_value));
}

int
Curl_gsk_secure_soc_startInit(gsk_handle my_session_handle,
int IOCompletionPort,
Qso_OverlappedIO_t * communicationsArea)

{
struct Curl_gsk_descriptor * p;

if(!my_session_handle)
return GSK_INVALID_HANDLE;
p = (struct Curl_gsk_descriptor *) my_session_handle;
return gsk_secure_soc_startInit(p->h, IOCompletionPort, communicationsArea);
}

#endif 



#if defined(HAVE_GSSAPI)



static int
Curl_gss_convert_in_place(OM_uint32 * minor_status, gss_buffer_t buf)

{
unsigned int i = buf->length;




if(i) {
char *t = malloc(i);
if(!t) {
gss_release_buffer(minor_status, buf);

if(minor_status)
*minor_status = ENOMEM;

return -1;
}

QadrtConvertE2A(t, buf->value, i, i);
memcpy(buf->value, t, i);
free(t);
}

return 0;
}


OM_uint32
Curl_gss_import_name_a(OM_uint32 * minor_status, gss_buffer_t in_name,
gss_OID in_name_type, gss_name_t * out_name)

{
int rc;
unsigned int i;
gss_buffer_desc in;

if(!in_name || !in_name->value || !in_name->length)
return gss_import_name(minor_status, in_name, in_name_type, out_name);

memcpy((char *) &in, (char *) in_name, sizeof(in));
i = in.length;

in.value = malloc(i + 1);
if(!in.value) {
if(minor_status)
*minor_status = ENOMEM;

return GSS_S_FAILURE;
}

QadrtConvertA2E(in.value, in_name->value, i, i);
((char *) in.value)[i] = '\0';
rc = gss_import_name(minor_status, &in, in_name_type, out_name);
free(in.value);
return rc;
}


OM_uint32
Curl_gss_display_status_a(OM_uint32 * minor_status, OM_uint32 status_value,
int status_type, gss_OID mech_type,
gss_msg_ctx_t * message_context, gss_buffer_t status_string)

{
int rc;

rc = gss_display_status(minor_status, status_value, status_type,
mech_type, message_context, status_string);

if(rc != GSS_S_COMPLETE || !status_string ||
!status_string->length || !status_string->value)
return rc;





if(Curl_gss_convert_in_place(minor_status, status_string))
return GSS_S_FAILURE;

return rc;
}


OM_uint32
Curl_gss_init_sec_context_a(OM_uint32 * minor_status,
gss_cred_id_t cred_handle,
gss_ctx_id_t * context_handle,
gss_name_t target_name, gss_OID mech_type,
gss_flags_t req_flags, OM_uint32 time_req,
gss_channel_bindings_t input_chan_bindings,
gss_buffer_t input_token,
gss_OID * actual_mech_type,
gss_buffer_t output_token, gss_flags_t * ret_flags,
OM_uint32 * time_rec)

{
int rc;
gss_buffer_desc in;
gss_buffer_t inp;

in.value = NULL;
inp = input_token;

if(inp) {
if(inp->length && inp->value) {
unsigned int i = inp->length;

in.value = malloc(i + 1);
if(!in.value) {
if(minor_status)
*minor_status = ENOMEM;

return GSS_S_FAILURE;
}

QadrtConvertA2E(in.value, input_token->value, i, i);
((char *) in.value)[i] = '\0';
in.length = i;
inp = &in;
}
}

rc = gss_init_sec_context(minor_status, cred_handle, context_handle,
target_name, mech_type, req_flags, time_req,
input_chan_bindings, inp, actual_mech_type,
output_token, ret_flags, time_rec);
free(in.value);

if(rc != GSS_S_COMPLETE || !output_token ||
!output_token->length || !output_token->value)
return rc;





if(Curl_gss_convert_in_place(minor_status, output_token))
return GSS_S_FAILURE;

return rc;
}


OM_uint32
Curl_gss_delete_sec_context_a(OM_uint32 * minor_status,
gss_ctx_id_t * context_handle,
gss_buffer_t output_token)

{
int rc;

rc = gss_delete_sec_context(minor_status, context_handle, output_token);

if(rc != GSS_S_COMPLETE || !output_token ||
!output_token->length || !output_token->value)
return rc;





if(Curl_gss_convert_in_place(minor_status, output_token))
return GSS_S_FAILURE;

return rc;
}

#endif 


#if !defined(CURL_DISABLE_LDAP)



void *
Curl_ldap_init_a(char * host, int port)

{
unsigned int i;
char * ehost;
void * result;

if(!host)
return (void *) ldap_init(host, port);

i = strlen(host);

ehost = malloc(i + 1);
if(!ehost)
return (void *) NULL;

QadrtConvertA2E(ehost, host, i, i);
ehost[i] = '\0';
result = (void *) ldap_init(ehost, port);
free(ehost);
return result;
}


int
Curl_ldap_simple_bind_s_a(void * ld, char * dn, char * passwd)

{
int i;
char * edn;
char * epasswd;

edn = (char *) NULL;
epasswd = (char *) NULL;

if(dn) {
i = strlen(dn);

edn = malloc(i + 1);
if(!edn)
return LDAP_NO_MEMORY;

QadrtConvertA2E(edn, dn, i, i);
edn[i] = '\0';
}

if(passwd) {
i = strlen(passwd);

epasswd = malloc(i + 1);
if(!epasswd) {
free(edn);
return LDAP_NO_MEMORY;
}

QadrtConvertA2E(epasswd, passwd, i, i);
epasswd[i] = '\0';
}

i = ldap_simple_bind_s(ld, edn, epasswd);
free(epasswd);
free(edn);
return i;
}


int
Curl_ldap_search_s_a(void * ld, char * base, int scope, char * filter,
char * * attrs, int attrsonly, LDAPMessage * * res)

{
int i;
int j;
char * ebase;
char * efilter;
char * * eattrs;
int status;

ebase = (char *) NULL;
efilter = (char *) NULL;
eattrs = (char * *) NULL;
status = LDAP_SUCCESS;

if(base) {
i = strlen(base);

ebase = malloc(i + 1);
if(!ebase)
status = LDAP_NO_MEMORY;
else {
QadrtConvertA2E(ebase, base, i, i);
ebase[i] = '\0';
}
}

if(filter && status == LDAP_SUCCESS) {
i = strlen(filter);

efilter = malloc(i + 1);
if(!efilter)
status = LDAP_NO_MEMORY;
else {
QadrtConvertA2E(efilter, filter, i, i);
efilter[i] = '\0';
}
}

if(attrs && status == LDAP_SUCCESS) {
for(i = 0; attrs[i++];)
;

eattrs = calloc(i, sizeof(*eattrs));
if(!eattrs)
status = LDAP_NO_MEMORY;
else {
for(j = 0; attrs[j]; j++) {
i = strlen(attrs[j]);

eattrs[j] = malloc(i + 1);
if(!eattrs[j]) {
status = LDAP_NO_MEMORY;
break;
}

QadrtConvertA2E(eattrs[j], attrs[j], i, i);
eattrs[j][i] = '\0';
}
}
}

if(status == LDAP_SUCCESS)
status = ldap_search_s(ld, ebase? ebase: "", scope,
efilter? efilter: "(objectclass=*)",
eattrs, attrsonly, res);

if(eattrs) {
for(j = 0; eattrs[j]; j++)
free(eattrs[j]);

free(eattrs);
}

free(efilter);
free(ebase);
return status;
}


struct berval * *
Curl_ldap_get_values_len_a(void * ld, LDAPMessage * entry, const char * attr)

{
char * cp;
struct berval * * result;

cp = (char *) NULL;

if(attr) {
int i = strlen(attr);

cp = malloc(i + 1);
if(!cp) {
ldap_set_lderrno(ld, LDAP_NO_MEMORY, NULL,
ldap_err2string(LDAP_NO_MEMORY));
return (struct berval * *) NULL;
}

QadrtConvertA2E(cp, attr, i, i);
cp[i] = '\0';
}

result = ldap_get_values_len(ld, entry, cp);
free(cp);




return result;
}


char *
Curl_ldap_err2string_a(int error)

{
return set_thread_string(LK_LDAP_ERROR, ldap_err2string(error));
}


char *
Curl_ldap_get_dn_a(void * ld, LDAPMessage * entry)

{
int i;
char * cp;
char * cp2;

cp = ldap_get_dn(ld, entry);

if(!cp)
return cp;

i = strlen(cp);

cp2 = malloc(i + 1);
if(!cp2)
return cp2;

QadrtConvertE2A(cp2, cp, i, i);
cp2[i] = '\0';





strcpy(cp, cp2);
free(cp2);
return cp;
}


char *
Curl_ldap_first_attribute_a(void * ld,
LDAPMessage * entry, BerElement * * berptr)

{
int i;
char * cp;
char * cp2;

cp = ldap_first_attribute(ld, entry, berptr);

if(!cp)
return cp;

i = strlen(cp);

cp2 = malloc(i + 1);
if(!cp2)
return cp2;

QadrtConvertE2A(cp2, cp, i, i);
cp2[i] = '\0';





strcpy(cp, cp2);
free(cp2);
return cp;
}


char *
Curl_ldap_next_attribute_a(void * ld,
LDAPMessage * entry, BerElement * berptr)

{
int i;
char * cp;
char * cp2;

cp = ldap_next_attribute(ld, entry, berptr);

if(!cp)
return cp;

i = strlen(cp);

cp2 = malloc(i + 1);
if(!cp2)
return cp2;

QadrtConvertE2A(cp2, cp, i, i);
cp2[i] = '\0';





strcpy(cp, cp2);
free(cp2);
return cp;
}

#endif 


static int
sockaddr2ebcdic(struct sockaddr_storage *dstaddr,
const struct sockaddr *srcaddr, int srclen)
{
const struct sockaddr_un *srcu;
struct sockaddr_un *dstu;
unsigned int i;
unsigned int dstsize;



if(!srcaddr || srclen < offsetof(struct sockaddr, sa_family) +
sizeof(srcaddr->sa_family) || srclen > sizeof(*dstaddr)) {
errno = EINVAL;
return -1;
}

memcpy((char *) dstaddr, (char *) srcaddr, srclen);

switch (srcaddr->sa_family) {

case AF_UNIX:
srcu = (const struct sockaddr_un *) srcaddr;
dstu = (struct sockaddr_un *) dstaddr;
dstsize = sizeof(*dstaddr) - offsetof(struct sockaddr_un, sun_path);
srclen -= offsetof(struct sockaddr_un, sun_path);
i = QadrtConvertA2E(dstu->sun_path, srcu->sun_path, dstsize - 1, srclen);
dstu->sun_path[i] = '\0';
srclen = i + offsetof(struct sockaddr_un, sun_path);
}

return srclen;
}


static int
sockaddr2ascii(struct sockaddr *dstaddr, int dstlen,
const struct sockaddr_storage *srcaddr, int srclen)
{
const struct sockaddr_un *srcu;
struct sockaddr_un *dstu;
unsigned int dstsize;



if(!srclen)
return 0;
if(srclen > dstlen)
srclen = dstlen;
if(!srcaddr || srclen < 0) {
errno = EINVAL;
return -1;
}

memcpy((char *) dstaddr, (char *) srcaddr, srclen);

if(srclen >= offsetof(struct sockaddr_storage, ss_family) +
sizeof(srcaddr->ss_family)) {
switch (srcaddr->ss_family) {

case AF_UNIX:
srcu = (const struct sockaddr_un *) srcaddr;
dstu = (struct sockaddr_un *) dstaddr;
dstsize = dstlen - offsetof(struct sockaddr_un, sun_path);
srclen -= offsetof(struct sockaddr_un, sun_path);
if(dstsize > 0 && srclen > 0) {
srclen = QadrtConvertE2A(dstu->sun_path, srcu->sun_path,
dstsize - 1, srclen);
dstu->sun_path[srclen] = '\0';
}
srclen += offsetof(struct sockaddr_un, sun_path);
}
}

return srclen;
}


int
Curl_os400_connect(int sd, struct sockaddr * destaddr, int addrlen)
{
int i;
struct sockaddr_storage laddr;

i = sockaddr2ebcdic(&laddr, destaddr, addrlen);

if(i < 0)
return -1;

return connect(sd, (struct sockaddr *) &laddr, i);
}


int
Curl_os400_bind(int sd, struct sockaddr * localaddr, int addrlen)
{
int i;
struct sockaddr_storage laddr;

i = sockaddr2ebcdic(&laddr, localaddr, addrlen);

if(i < 0)
return -1;

return bind(sd, (struct sockaddr *) &laddr, i);
}


int
Curl_os400_sendto(int sd, char * buffer, int buflen, int flags,
struct sockaddr * dstaddr, int addrlen)
{
int i;
struct sockaddr_storage laddr;

i = sockaddr2ebcdic(&laddr, dstaddr, addrlen);

if(i < 0)
return -1;

return sendto(sd, buffer, buflen, flags, (struct sockaddr *) &laddr, i);
}


int
Curl_os400_recvfrom(int sd, char * buffer, int buflen, int flags,
struct sockaddr * fromaddr, int * addrlen)
{
int rcvlen;
struct sockaddr_storage laddr;
int laddrlen = sizeof(laddr);

if(!fromaddr || !addrlen || *addrlen <= 0)
return recvfrom(sd, buffer, buflen, flags, fromaddr, addrlen);

laddr.ss_family = AF_UNSPEC; 
rcvlen = recvfrom(sd, buffer, buflen, flags,
(struct sockaddr *) &laddr, &laddrlen);

if(rcvlen < 0)
return rcvlen;

if(laddr.ss_family == AF_UNSPEC)
laddrlen = 0;
else {
laddrlen = sockaddr2ascii(fromaddr, *addrlen, &laddr, laddrlen);
if(laddrlen < 0)
return laddrlen;
}
*addrlen = laddrlen;
return rcvlen;
}


int
Curl_os400_getpeername(int sd, struct sockaddr *addr, int *addrlen)
{
struct sockaddr_storage laddr;
int laddrlen = sizeof(laddr);
int retcode = getpeername(sd, (struct sockaddr *) &laddr, &laddrlen);

if(!retcode) {
laddrlen = sockaddr2ascii(addr, *addrlen, &laddr, laddrlen);
if(laddrlen < 0)
return laddrlen;
*addrlen = laddrlen;
}

return retcode;
}


int
Curl_os400_getsockname(int sd, struct sockaddr *addr, int *addrlen)
{
struct sockaddr_storage laddr;
int laddrlen = sizeof(laddr);
int retcode = getsockname(sd, (struct sockaddr *) &laddr, &laddrlen);

if(!retcode) {
laddrlen = sockaddr2ascii(addr, *addrlen, &laddr, laddrlen);
if(laddrlen < 0)
return laddrlen;
*addrlen = laddrlen;
}

return retcode;
}


#if defined(HAVE_LIBZ)
const char *
Curl_os400_zlibVersion(void)

{
return set_thread_string(LK_ZLIB_VERSION, zlibVersion());
}


int
Curl_os400_inflateInit_(z_streamp strm, const char * version, int stream_size)

{
z_const char * msgb4 = strm->msg;
int ret;

ret = inflateInit(strm);

if(strm->msg != msgb4)
strm->msg = set_thread_string(LK_ZLIB_MSG, strm->msg);

return ret;
}


int
Curl_os400_inflateInit2_(z_streamp strm, int windowBits,
const char * version, int stream_size)

{
z_const char * msgb4 = strm->msg;
int ret;

ret = inflateInit2(strm, windowBits);

if(strm->msg != msgb4)
strm->msg = set_thread_string(LK_ZLIB_MSG, strm->msg);

return ret;
}


int
Curl_os400_inflate(z_streamp strm, int flush)

{
z_const char * msgb4 = strm->msg;
int ret;

ret = inflate(strm, flush);

if(strm->msg != msgb4)
strm->msg = set_thread_string(LK_ZLIB_MSG, strm->msg);

return ret;
}


int
Curl_os400_inflateEnd(z_streamp strm)

{
z_const char * msgb4 = strm->msg;
int ret;

ret = inflateEnd(strm);

if(strm->msg != msgb4)
strm->msg = set_thread_string(LK_ZLIB_MSG, strm->msg);

return ret;
}

#endif
