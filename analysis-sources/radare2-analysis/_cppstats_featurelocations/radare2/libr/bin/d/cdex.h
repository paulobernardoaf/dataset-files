struct cdex_header {
uint8_t magic[8];
uint32_t checksum;
uint8_t signature[20]; 
uint32_t fileSize;
uint32_t headerSize;
uint32_t endianTag;
uint32_t linkSize;
uint32_t linkOff;
uint32_t mapOff;
uint32_t stringIdsSize;
uint32_t stringIdsOff;
uint32_t typeIdsSize;
uint32_t typeIdsOff;
uint32_t protoIdsSize;
uint32_t protoIdsOff;
uint32_t fieldIdsSize;
uint32_t fieldIdsOff;
uint32_t methodIdsSize;
uint32_t methodIdsOff;
uint32_t classDefsSize;
uint32_t classDefsOff;
uint32_t dataSize;
uint32_t dataOff;
uint32_t featureFlags;
uint32_t debugInfoOffsetsPos;
uint32_t debugInfoOffsetsTableOffset;
uint32_t debugInfoBase;
uint32_t ownedDataBegin;
uint32_t ownedDataEnd;
};
