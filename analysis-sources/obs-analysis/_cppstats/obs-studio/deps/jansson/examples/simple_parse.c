#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
void print_json(json_t *root);
void print_json_aux(json_t *element, int indent);
void print_json_indent(int indent);
const char *json_plural(int count);
void print_json_object(json_t *element, int indent);
void print_json_array(json_t *element, int indent);
void print_json_string(json_t *element, int indent);
void print_json_integer(json_t *element, int indent);
void print_json_real(json_t *element, int indent);
void print_json_true(json_t *element, int indent);
void print_json_false(json_t *element, int indent);
void print_json_null(json_t *element, int indent);
void print_json(json_t *root) {
print_json_aux(root, 0);
}
void print_json_aux(json_t *element, int indent) {
switch (json_typeof(element)) {
case JSON_OBJECT:
print_json_object(element, indent);
break;
case JSON_ARRAY:
print_json_array(element, indent);
break;
case JSON_STRING:
print_json_string(element, indent);
break;
case JSON_INTEGER:
print_json_integer(element, indent);
break;
case JSON_REAL:
print_json_real(element, indent);
break;
case JSON_TRUE:
print_json_true(element, indent);
break;
case JSON_FALSE:
print_json_false(element, indent);
break;
case JSON_NULL:
print_json_null(element, indent);
break;
default:
fprintf(stderr, "unrecognized JSON type %d\n", json_typeof(element));
}
}
void print_json_indent(int indent) {
int i;
for (i = 0; i < indent; i++) { putchar(' '); }
}
const char *json_plural(int count) {
return count == 1 ? "" : "s";
}
void print_json_object(json_t *element, int indent) {
size_t size;
const char *key;
json_t *value;
print_json_indent(indent);
size = json_object_size(element);
printf("JSON Object of %ld pair%s:\n", size, json_plural(size));
json_object_foreach(element, key, value) {
print_json_indent(indent + 2);
printf("JSON Key: \"%s\"\n", key);
print_json_aux(value, indent + 2);
}
}
void print_json_array(json_t *element, int indent) {
size_t i;
size_t size = json_array_size(element);
print_json_indent(indent);
printf("JSON Array of %ld element%s:\n", size, json_plural(size));
for (i = 0; i < size; i++) {
print_json_aux(json_array_get(element, i), indent + 2);
}
}
void print_json_string(json_t *element, int indent) {
print_json_indent(indent);
printf("JSON String: \"%s\"\n", json_string_value(element));
}
void print_json_integer(json_t *element, int indent) {
print_json_indent(indent);
printf("JSON Integer: \"%" JSON_INTEGER_FORMAT "\"\n", json_integer_value(element));
}
void print_json_real(json_t *element, int indent) {
print_json_indent(indent);
printf("JSON Real: %f\n", json_real_value(element));
}
void print_json_true(json_t *element, int indent) {
(void)element;
print_json_indent(indent);
printf("JSON True\n");
}
void print_json_false(json_t *element, int indent) {
(void)element;
print_json_indent(indent);
printf("JSON False\n");
}
void print_json_null(json_t *element, int indent) {
(void)element;
print_json_indent(indent);
printf("JSON Null\n");
}
json_t *load_json(const char *text) {
json_t *root;
json_error_t error;
root = json_loads(text, 0, &error);
if (root) {
return root;
} else {
fprintf(stderr, "json error on line %d: %s\n", error.line, error.text);
return (json_t *)0;
}
}
char *read_line(char *line, int max_chars) {
printf("Type some JSON > ");
fflush(stdout);
return fgets(line, max_chars, stdin);
}
#define MAX_CHARS 4096
int main(int argc, char *argv[]) {
char line[MAX_CHARS];
if (argc != 1) {
fprintf(stderr, "Usage: %s\n", argv[0]);
exit(-1);
}
while (read_line(line, MAX_CHARS) != (char *)NULL) {
json_t *root = load_json(line);
if (root) {
print_json(root);
json_decref(root);
}
}
return 0;
}
