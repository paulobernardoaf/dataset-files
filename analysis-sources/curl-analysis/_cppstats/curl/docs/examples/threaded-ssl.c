#define USE_OPENSSL 
#include <stdio.h>
#include <pthread.h>
#include <curl/curl.h>
#define NUMT 4
static pthread_mutex_t *lockarray;
#if defined(USE_OPENSSL)
#include <openssl/crypto.h>
static void lock_callback(int mode, int type, char *file, int line)
{
(void)file;
(void)line;
if(mode & CRYPTO_LOCK) {
pthread_mutex_lock(&(lockarray[type]));
}
else {
pthread_mutex_unlock(&(lockarray[type]));
}
}
static unsigned long thread_id(void)
{
unsigned long ret;
ret = (unsigned long)pthread_self();
return ret;
}
static void init_locks(void)
{
int i;
lockarray = (pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() *
sizeof(pthread_mutex_t));
for(i = 0; i<CRYPTO_num_locks(); i++) {
pthread_mutex_init(&(lockarray[i]), NULL);
}
CRYPTO_set_id_callback((unsigned long (*)())thread_id);
CRYPTO_set_locking_callback((void (*)())lock_callback);
}
static void kill_locks(void)
{
int i;
CRYPTO_set_locking_callback(NULL);
for(i = 0; i<CRYPTO_num_locks(); i++)
pthread_mutex_destroy(&(lockarray[i]));
OPENSSL_free(lockarray);
}
#endif
#if defined(USE_GNUTLS)
#include <gcrypt.h>
#include <errno.h>
GCRY_THREAD_OPTION_PTHREAD_IMPL;
void init_locks(void)
{
gcry_control(GCRYCTL_SET_THREAD_CBS);
}
#define kill_locks()
#endif
const char * const urls[]= {
"https://www.example.com/",
"https://www2.example.com/",
"https://www3.example.com/",
"https://www4.example.com/",
};
static void *pull_one_url(void *url)
{
CURL *curl;
curl = curl_easy_init();
curl_easy_setopt(curl, CURLOPT_URL, url);
curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
curl_easy_perform(curl); 
curl_easy_cleanup(curl);
return NULL;
}
int main(int argc, char **argv)
{
pthread_t tid[NUMT];
int i;
(void)argc; 
(void)argv;
curl_global_init(CURL_GLOBAL_ALL);
init_locks();
for(i = 0; i< NUMT; i++) {
int error = pthread_create(&tid[i],
NULL, 
pull_one_url,
(void *)urls[i]);
if(0 != error)
fprintf(stderr, "Couldn't run thread number %d, errno %d\n", i, error);
else
fprintf(stderr, "Thread %d, gets %s\n", i, urls[i]);
}
for(i = 0; i< NUMT; i++) {
pthread_join(tid[i], NULL);
fprintf(stderr, "Thread %d terminated\n", i);
}
kill_locks();
return 0;
}
