struct dex_header {
uint8_t magic[8];
uint32_t checksum;
uint8_t signature[20];
uint32_t size;
uint32_t header_size;
uint32_t endian;
uint32_t linksection_size;
uint32_t linksection_offset;
uint32_t map_offset;
uint32_t strings_size;
uint32_t strings_offset;
uint32_t types_size;
uint32_t types_offset;
uint32_t prototypes_size;
uint32_t prototypes_offset;
uint32_t fields_size;
uint32_t fields_offset;
uint32_t method_size;
uint32_t method_offset;
uint32_t class_size;
uint32_t class_offset;
uint32_t data_size;
uint32_t data_offset;
};
