struct repository;
struct argv_array;
struct packet_reader;
int ls_refs(struct repository *r, struct argv_array *keys,
struct packet_reader *request);
