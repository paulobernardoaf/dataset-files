struct hpack_decoder;
struct hpack_decoder *hpack_decode_init(size_t header_table_size);
void hpack_decode_destroy(struct hpack_decoder *);
int hpack_decode(struct hpack_decoder *dec, const uint8_t *data,
size_t length, char *headers[][2], unsigned max);
size_t hpack_encode_hdr_neverindex(uint8_t *restrict buf, size_t size,
const char *name, const char *value);
size_t hpack_encode(uint8_t *restrict buf, size_t size,
const char *const headers[][2], unsigned count);
