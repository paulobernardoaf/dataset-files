#include <stddef.h>
#include <stdint.h>
#include "libavutil/buffer.h"
enum {
JPEG_MARKER_SOF0 = 0xc0,
JPEG_MARKER_SOF1 = 0xc1,
JPEG_MARKER_SOF2 = 0xc2,
JPEG_MARKER_SOF3 = 0xc3,
JPEG_MARKER_DHT = 0xc4,
JPEG_MARKER_SOI = 0xd8,
JPEG_MARKER_EOI = 0xd9,
JPEG_MARKER_SOS = 0xda,
JPEG_MARKER_DQT = 0xdb,
JPEG_MARKER_APPN = 0xe0,
JPEG_MARKER_JPGN = 0xf0,
JPEG_MARKER_COM = 0xfe,
};
enum {
JPEG_MAX_COMPONENTS = 255,
JPEG_MAX_HEIGHT = 65535,
JPEG_MAX_WIDTH = 65535,
};
typedef struct JPEGRawFrameHeader {
uint16_t Lf;
uint8_t P;
uint16_t Y;
uint16_t X;
uint16_t Nf;
uint8_t C [JPEG_MAX_COMPONENTS];
uint8_t H [JPEG_MAX_COMPONENTS];
uint8_t V [JPEG_MAX_COMPONENTS];
uint8_t Tq[JPEG_MAX_COMPONENTS];
} JPEGRawFrameHeader;
typedef struct JPEGRawScanHeader {
uint16_t Ls;
uint8_t Ns;
uint8_t Cs[JPEG_MAX_COMPONENTS];
uint8_t Td[JPEG_MAX_COMPONENTS];
uint8_t Ta[JPEG_MAX_COMPONENTS];
uint8_t Ss;
uint8_t Se;
uint8_t Ah;
uint8_t Al;
} JPEGRawScanHeader;
typedef struct JPEGRawScan {
JPEGRawScanHeader header;
uint8_t *data;
size_t data_size;
AVBufferRef *data_ref;
} JPEGRawScan;
typedef struct JPEGRawQuantisationTable {
uint8_t Pq;
uint8_t Tq;
uint16_t Q[64];
} JPEGRawQuantisationTable;
typedef struct JPEGRawQuantisationTableSpecification {
uint16_t Lq;
JPEGRawQuantisationTable table[4];
} JPEGRawQuantisationTableSpecification;
typedef struct JPEGRawHuffmanTable {
uint8_t Tc;
uint8_t Th;
uint8_t L[16];
uint8_t V[224];
} JPEGRawHuffmanTable;
typedef struct JPEGRawHuffmanTableSpecification {
uint16_t Lh;
JPEGRawHuffmanTable table[8];
} JPEGRawHuffmanTableSpecification;
typedef struct JPEGRawApplicationData {
uint16_t Lp;
uint8_t *Ap;
AVBufferRef *Ap_ref;
} JPEGRawApplicationData;
typedef struct JPEGRawComment {
uint16_t Lc;
uint8_t *Cm;
AVBufferRef *Cm_ref;
} JPEGRawComment;
