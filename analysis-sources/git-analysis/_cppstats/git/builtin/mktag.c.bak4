#include "builtin.h"
#include "tag.h"
#include "replace-object.h"
#include "object-store.h"


















static int verify_object(const struct object_id *oid, const char *expected_type)
{
int ret = -1;
enum object_type type;
unsigned long size;
void *buffer = read_object_file(oid, &type, &size);
const struct object_id *repl = lookup_replace_object(the_repository, oid);

if (buffer) {
if (type == type_from_string(expected_type)) {
ret = check_object_signature(the_repository, repl,
buffer, size,
expected_type);
}
free(buffer);
}
return ret;
}

static int verify_tag(char *buffer, unsigned long size)
{
int typelen;
char type[20];
struct object_id oid;
const char *object, *type_line, *tag_line, *tagger_line, *lb, *rb, *p;
size_t len;

if (size < 84)
return error("wanna fool me ? you obviously got the size wrong !");

buffer[size] = 0;


object = buffer;
if (memcmp(object, "object ", 7))
return error("char%d: does not start with \"object \"", 0);

if (parse_oid_hex(object + 7, &oid, &p))
return error("char%d: could not get SHA1 hash", 7);


type_line = p + 1;
if (memcmp(type_line - 1, "\ntype ", 6))
return error("char%d: could not find \"\\ntype \"", 47);


tag_line = strchr(type_line, '\n');
if (!tag_line)
return error("char%"PRIuMAX": could not find next \"\\n\"",
(uintmax_t) (type_line - buffer));
tag_line++;
if (memcmp(tag_line, "tag ", 4) || tag_line[4] == '\n')
return error("char%"PRIuMAX": no \"tag \" found",
(uintmax_t) (tag_line - buffer));


typelen = tag_line - type_line - strlen("type \n");
if (typelen >= sizeof(type))
return error("char%"PRIuMAX": type too long",
(uintmax_t) (type_line+5 - buffer));

memcpy(type, type_line+5, typelen);
type[typelen] = 0;


if (verify_object(&oid, type))
return error("char%d: could not verify object %s", 7, oid_to_hex(&oid));


tag_line += 4;
for (;;) {
unsigned char c = *tag_line++;
if (c == '\n')
break;
if (c > ' ')
continue;
return error("char%"PRIuMAX": could not verify tag name",
(uintmax_t) (tag_line - buffer));
}


tagger_line = tag_line;

if (memcmp(tagger_line, "tagger ", 7))
return error("char%"PRIuMAX": could not find \"tagger \"",
(uintmax_t) (tagger_line - buffer));







tagger_line += 7;
if (!(lb = strstr(tagger_line, " <")) || !(rb = strstr(lb+2, "> ")) ||
strpbrk(tagger_line, "<>\n") != lb+1 ||
strpbrk(lb+2, "><\n ") != rb)
return error("char%"PRIuMAX": malformed tagger field",
(uintmax_t) (tagger_line - buffer));


if (lb == tagger_line)
return error("char%"PRIuMAX": missing tagger name",
(uintmax_t) (tagger_line - buffer));


tagger_line = rb + 2;
if (!(len = strspn(tagger_line, "0123456789")))
return error("char%"PRIuMAX": missing tag timestamp",
(uintmax_t) (tagger_line - buffer));
tagger_line += len;
if (*tagger_line != ' ')
return error("char%"PRIuMAX": malformed tag timestamp",
(uintmax_t) (tagger_line - buffer));
tagger_line++;


if (!((tagger_line[0] == '+' || tagger_line[0] == '-') &&
strspn(tagger_line+1, "0123456789") == 4 &&
tagger_line[5] == '\n' && atoi(tagger_line+1) <= 1400))
return error("char%"PRIuMAX": malformed tag timezone",
(uintmax_t) (tagger_line - buffer));
tagger_line += 6;


if (*tagger_line != '\n')
return error("char%"PRIuMAX": trailing garbage in tag header",
(uintmax_t) (tagger_line - buffer));


return 0;
}

int cmd_mktag(int argc, const char **argv, const char *prefix)
{
struct strbuf buf = STRBUF_INIT;
struct object_id result;

if (argc != 1)
usage("git mktag");

if (strbuf_read(&buf, 0, 4096) < 0) {
die_errno("could not read from stdin");
}



if (verify_tag(buf.buf, buf.len) < 0)
die("invalid tag signature file");

if (write_object_file(buf.buf, buf.len, tag_type, &result) < 0)
die("unable to write tag file");

strbuf_release(&buf);
printf("%s\n", oid_to_hex(&result));
return 0;
}
