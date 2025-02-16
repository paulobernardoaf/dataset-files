#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <libsecret/secret.h>
struct credential {
char *protocol;
char *host;
unsigned short port;
char *path;
char *username;
char *password;
};
#define CREDENTIAL_INIT { NULL, NULL, 0, NULL, NULL, NULL }
typedef int (*credential_op_cb)(struct credential *);
struct credential_operation {
char *name;
credential_op_cb op;
};
#define CREDENTIAL_OP_END { NULL, NULL }
static char *make_label(struct credential *c)
{
if (c->port)
return g_strdup_printf("Git: %s://%s:%hu/%s",
c->protocol, c->host, c->port, c->path ? c->path : "");
else
return g_strdup_printf("Git: %s://%s/%s",
c->protocol, c->host, c->path ? c->path : "");
}
static GHashTable *make_attr_list(struct credential *c)
{
GHashTable *al = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
if (c->username)
g_hash_table_insert(al, "user", g_strdup(c->username));
if (c->protocol)
g_hash_table_insert(al, "protocol", g_strdup(c->protocol));
if (c->host)
g_hash_table_insert(al, "server", g_strdup(c->host));
if (c->port)
g_hash_table_insert(al, "port", g_strdup_printf("%hu", c->port));
if (c->path)
g_hash_table_insert(al, "object", g_strdup(c->path));
return al;
}
static int keyring_get(struct credential *c)
{
SecretService *service = NULL;
GHashTable *attributes = NULL;
GError *error = NULL;
GList *items = NULL;
if (!c->protocol || !(c->host || c->path))
return EXIT_FAILURE;
service = secret_service_get_sync(0, NULL, &error);
if (error != NULL) {
g_critical("could not connect to Secret Service: %s", error->message);
g_error_free(error);
return EXIT_FAILURE;
}
attributes = make_attr_list(c);
items = secret_service_search_sync(service,
SECRET_SCHEMA_COMPAT_NETWORK,
attributes,
SECRET_SEARCH_LOAD_SECRETS | SECRET_SEARCH_UNLOCK,
NULL,
&error);
g_hash_table_unref(attributes);
if (error != NULL) {
g_critical("lookup failed: %s", error->message);
g_error_free(error);
return EXIT_FAILURE;
}
if (items != NULL) {
SecretItem *item;
SecretValue *secret;
const char *s;
item = items->data;
secret = secret_item_get_secret(item);
attributes = secret_item_get_attributes(item);
s = g_hash_table_lookup(attributes, "user");
if (s) {
g_free(c->username);
c->username = g_strdup(s);
}
s = secret_value_get_text(secret);
if (s) {
g_free(c->password);
c->password = g_strdup(s);
}
g_hash_table_unref(attributes);
secret_value_unref(secret);
g_list_free_full(items, g_object_unref);
}
return EXIT_SUCCESS;
}
static int keyring_store(struct credential *c)
{
char *label = NULL;
GHashTable *attributes = NULL;
GError *error = NULL;
if (!c->protocol || !(c->host || c->path) ||
!c->username || !c->password)
return EXIT_FAILURE;
label = make_label(c);
attributes = make_attr_list(c);
secret_password_storev_sync(SECRET_SCHEMA_COMPAT_NETWORK,
attributes,
NULL,
label,
c->password,
NULL,
&error);
g_free(label);
g_hash_table_unref(attributes);
if (error != NULL) {
g_critical("store failed: %s", error->message);
g_error_free(error);
return EXIT_FAILURE;
}
return EXIT_SUCCESS;
}
static int keyring_erase(struct credential *c)
{
GHashTable *attributes = NULL;
GError *error = NULL;
if (!c->protocol && !c->host && !c->path && !c->username)
return EXIT_FAILURE;
attributes = make_attr_list(c);
secret_password_clearv_sync(SECRET_SCHEMA_COMPAT_NETWORK,
attributes,
NULL,
&error);
g_hash_table_unref(attributes);
if (error != NULL) {
g_critical("erase failed: %s", error->message);
g_error_free(error);
return EXIT_FAILURE;
}
return EXIT_SUCCESS;
}
static struct credential_operation const credential_helper_ops[] = {
{ "get", keyring_get },
{ "store", keyring_store },
{ "erase", keyring_erase },
CREDENTIAL_OP_END
};
static void credential_init(struct credential *c)
{
memset(c, 0, sizeof(*c));
}
static void credential_clear(struct credential *c)
{
g_free(c->protocol);
g_free(c->host);
g_free(c->path);
g_free(c->username);
g_free(c->password);
credential_init(c);
}
static int credential_read(struct credential *c)
{
char *buf;
size_t line_len;
char *key;
char *value;
key = buf = g_malloc(1024);
while (fgets(buf, 1024, stdin)) {
line_len = strlen(buf);
if (line_len && buf[line_len-1] == '\n')
buf[--line_len] = '\0';
if (!line_len)
break;
value = strchr(buf, '=');
if (!value) {
g_warning("invalid credential line: %s", key);
g_free(buf);
return -1;
}
*value++ = '\0';
if (!strcmp(key, "protocol")) {
g_free(c->protocol);
c->protocol = g_strdup(value);
} else if (!strcmp(key, "host")) {
g_free(c->host);
c->host = g_strdup(value);
value = strrchr(c->host, ':');
if (value) {
*value++ = '\0';
c->port = atoi(value);
}
} else if (!strcmp(key, "path")) {
g_free(c->path);
c->path = g_strdup(value);
} else if (!strcmp(key, "username")) {
g_free(c->username);
c->username = g_strdup(value);
} else if (!strcmp(key, "password")) {
g_free(c->password);
c->password = g_strdup(value);
while (*value)
*value++ = '\0';
}
}
g_free(buf);
return 0;
}
static void credential_write_item(FILE *fp, const char *key, const char *value)
{
if (!value)
return;
fprintf(fp, "%s=%s\n", key, value);
}
static void credential_write(const struct credential *c)
{
credential_write_item(stdout, "username", c->username);
credential_write_item(stdout, "password", c->password);
}
static void usage(const char *name)
{
struct credential_operation const *try_op = credential_helper_ops;
const char *basename = strrchr(name, '/');
basename = (basename) ? basename + 1 : name;
fprintf(stderr, "usage: %s <", basename);
while (try_op->name) {
fprintf(stderr, "%s", (try_op++)->name);
if (try_op->name)
fprintf(stderr, "%s", "|");
}
fprintf(stderr, "%s", ">\n");
}
int main(int argc, char *argv[])
{
int ret = EXIT_SUCCESS;
struct credential_operation const *try_op = credential_helper_ops;
struct credential cred = CREDENTIAL_INIT;
if (!argv[1]) {
usage(argv[0]);
exit(EXIT_FAILURE);
}
g_set_application_name("Git Credential Helper");
while (try_op->name && strcmp(argv[1], try_op->name))
try_op++;
if (!try_op->name || !try_op->op)
goto out;
ret = credential_read(&cred);
if (ret)
goto out;
ret = (*try_op->op)(&cred);
credential_write(&cred);
out:
credential_clear(&cred);
return ret;
}
