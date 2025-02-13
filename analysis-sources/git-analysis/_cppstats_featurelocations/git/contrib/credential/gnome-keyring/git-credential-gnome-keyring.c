























#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gnome-keyring.h>

#if defined(GNOME_KEYRING_DEFAULT)



#include <gnome-keyring-memory.h>

#else










#define GNOME_KEYRING_DEFAULT NULL







#define GNOME_KEYRING_RESULT_NO_MATCH GNOME_KEYRING_RESULT_DENIED

#define gnome_keyring_memory_alloc g_malloc
#define gnome_keyring_memory_free gnome_keyring_free_password
#define gnome_keyring_memory_strdup g_strdup

static const char *gnome_keyring_result_to_message(GnomeKeyringResult result)
{
switch (result) {
case GNOME_KEYRING_RESULT_OK:
return "OK";
case GNOME_KEYRING_RESULT_DENIED:
return "Denied";
case GNOME_KEYRING_RESULT_NO_KEYRING_DAEMON:
return "No Keyring Daemon";
case GNOME_KEYRING_RESULT_ALREADY_UNLOCKED:
return "Already UnLocked";
case GNOME_KEYRING_RESULT_NO_SUCH_KEYRING:
return "No Such Keyring";
case GNOME_KEYRING_RESULT_BAD_ARGUMENTS:
return "Bad Arguments";
case GNOME_KEYRING_RESULT_IO_ERROR:
return "IO Error";
case GNOME_KEYRING_RESULT_CANCELLED:
return "Cancelled";
case GNOME_KEYRING_RESULT_ALREADY_EXISTS:
return "Already Exists";
default:
return "Unknown Error";
}
}






#if GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION < 8

static void gnome_keyring_done_cb(GnomeKeyringResult result, gpointer user_data)
{
gpointer *data = (gpointer *)user_data;
int *done = (int *)data[0];
GnomeKeyringResult *r = (GnomeKeyringResult *)data[1];

*r = result;
*done = 1;
}

static void wait_for_request_completion(int *done)
{
GMainContext *mc = g_main_context_default();
while (!*done)
g_main_context_iteration(mc, TRUE);
}

static GnomeKeyringResult gnome_keyring_item_delete_sync(const char *keyring, guint32 id)
{
int done = 0;
GnomeKeyringResult result;
gpointer data[] = { &done, &result };

gnome_keyring_item_delete(keyring, id, gnome_keyring_done_cb, data,
NULL);

wait_for_request_completion(&done);

return result;
}

#endif
#endif




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




static char *keyring_object(struct credential *c)
{
if (!c->path)
return NULL;

if (c->port)
return g_strdup_printf("%s:%hd/%s", c->host, c->port, c->path);

return g_strdup_printf("%s/%s", c->host, c->path);
}

static int keyring_get(struct credential *c)
{
char *object = NULL;
GList *entries;
GnomeKeyringNetworkPasswordData *password_data;
GnomeKeyringResult result;

if (!c->protocol || !(c->host || c->path))
return EXIT_FAILURE;

object = keyring_object(c);

result = gnome_keyring_find_network_password_sync(
c->username,
NULL ,
c->host,
object,
c->protocol,
NULL ,
c->port,
&entries);

g_free(object);

if (result == GNOME_KEYRING_RESULT_NO_MATCH)
return EXIT_SUCCESS;

if (result == GNOME_KEYRING_RESULT_CANCELLED)
return EXIT_SUCCESS;

if (result != GNOME_KEYRING_RESULT_OK) {
g_critical("%s", gnome_keyring_result_to_message(result));
return EXIT_FAILURE;
}


password_data = (GnomeKeyringNetworkPasswordData *)entries->data;

gnome_keyring_memory_free(c->password);
c->password = gnome_keyring_memory_strdup(password_data->password);

if (!c->username)
c->username = g_strdup(password_data->user);

gnome_keyring_network_password_list_free(entries);

return EXIT_SUCCESS;
}


static int keyring_store(struct credential *c)
{
guint32 item_id;
char *object = NULL;
GnomeKeyringResult result;








if (!c->protocol || !(c->host || c->path) ||
!c->username || !c->password)
return EXIT_FAILURE;

object = keyring_object(c);

result = gnome_keyring_set_network_password_sync(
GNOME_KEYRING_DEFAULT,
c->username,
NULL ,
c->host,
object,
c->protocol,
NULL ,
c->port,
c->password,
&item_id);

g_free(object);

if (result != GNOME_KEYRING_RESULT_OK &&
result != GNOME_KEYRING_RESULT_CANCELLED) {
g_critical("%s", gnome_keyring_result_to_message(result));
return EXIT_FAILURE;
}

return EXIT_SUCCESS;
}

static int keyring_erase(struct credential *c)
{
char *object = NULL;
GList *entries;
GnomeKeyringNetworkPasswordData *password_data;
GnomeKeyringResult result;









if (!c->protocol && !c->host && !c->path && !c->username)
return EXIT_FAILURE;

object = keyring_object(c);

result = gnome_keyring_find_network_password_sync(
c->username,
NULL ,
c->host,
object,
c->protocol,
NULL ,
c->port,
&entries);

g_free(object);

if (result == GNOME_KEYRING_RESULT_NO_MATCH)
return EXIT_SUCCESS;

if (result == GNOME_KEYRING_RESULT_CANCELLED)
return EXIT_SUCCESS;

if (result != GNOME_KEYRING_RESULT_OK) {
g_critical("%s", gnome_keyring_result_to_message(result));
return EXIT_FAILURE;
}


password_data = (GnomeKeyringNetworkPasswordData *)entries->data;

result = gnome_keyring_item_delete_sync(
password_data->keyring, password_data->item_id);

gnome_keyring_network_password_list_free(entries);

if (result != GNOME_KEYRING_RESULT_OK) {
g_critical("%s", gnome_keyring_result_to_message(result));
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
gnome_keyring_memory_free(c->password);

credential_init(c);
}

static int credential_read(struct credential *c)
{
char *buf;
size_t line_len;
char *key;
char *value;

key = buf = gnome_keyring_memory_alloc(1024);

while (fgets(buf, 1024, stdin)) {
line_len = strlen(buf);

if (line_len && buf[line_len-1] == '\n')
buf[--line_len] = '\0';

if (!line_len)
break;

value = strchr(buf, '=');
if (!value) {
g_warning("invalid credential line: %s", key);
gnome_keyring_memory_free(buf);
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
gnome_keyring_memory_free(c->password);
c->password = gnome_keyring_memory_strdup(value);
while (*value)
*value++ = '\0';
}





}

gnome_keyring_memory_free(buf);

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
