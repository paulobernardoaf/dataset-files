#include "mpeg.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static size_t _find_emulation_prevention_byte(const uint8_t* data, size_t size)
{
size_t offset = 2;
while (offset < size) {
if (0 == data[offset]) {
offset += 1;
} else if (3 != data[offset]) {
offset += 3;
} else if (0 != data[offset - 1]) {
offset += 2;
} else if (0 != data[offset - 2]) {
offset += 1;
} else {
return offset;
}
}
return size;
}
static size_t _copy_to_rbsp(uint8_t* destData, size_t destSize, const uint8_t* sorcData, size_t sorcSize)
{
size_t toCopy, totlSize = 0;
for (;;) {
if (destSize >= sorcSize) {
return 0;
}
toCopy = _find_emulation_prevention_byte(sorcData, destSize);
memcpy(destData, sorcData, toCopy);
totlSize += toCopy;
destData += toCopy;
destSize -= toCopy;
if (0 == destSize) {
return totlSize;
}
totlSize += 1;
sorcData += toCopy + 1;
sorcSize -= toCopy + 1;
}
return 0;
}
static inline size_t _find_emulated(uint8_t* data, size_t size)
{
size_t offset = 2;
while (offset < size) {
if (3 < data[offset]) {
offset += 3;
} else if (0 != data[offset - 1]) {
offset += 2;
} else if (0 != data[offset - 2]) {
offset += 1;
} else {
return offset;
}
}
return size;
}
size_t _copy_from_rbsp(uint8_t* data, uint8_t* payloadData, size_t payloadSize)
{
size_t total = 0;
while (payloadSize) {
size_t bytes = _find_emulated(payloadData, payloadSize);
if (bytes > payloadSize) {
return 0;
}
memcpy(data, payloadData, bytes);
if (bytes == payloadSize) {
return total + bytes;
}
data[bytes] = 3; 
data += bytes + 1;
total += bytes + 1;
payloadData += bytes;
payloadSize -= bytes;
}
return total;
}
sei_message_t* sei_message_next(sei_message_t* msg) { return ((struct _sei_message_t*)msg)->next; }
sei_msgtype_t sei_message_type(sei_message_t* msg) { return ((struct _sei_message_t*)msg)->type; }
size_t sei_message_size(sei_message_t* msg) { return ((struct _sei_message_t*)msg)->size; }
uint8_t* sei_message_data(sei_message_t* msg) { return ((uint8_t*)msg) + sizeof(struct _sei_message_t); }
void sei_message_free(sei_message_t* msg)
{
if (msg) {
free(msg);
}
}
sei_message_t* sei_message_new(sei_msgtype_t type, uint8_t* data, size_t size)
{
struct _sei_message_t* msg = (struct _sei_message_t*)malloc(sizeof(struct _sei_message_t) + size);
msg->next = 0;
msg->type = type;
msg->size = size;
if (data) {
memcpy(sei_message_data(msg), data, size);
} else {
memset(sei_message_data(msg), 0, size);
}
return (sei_message_t*)msg;
}
void sei_init(sei_t* sei, double timestamp)
{
sei->head = 0;
sei->tail = 0;
sei->timestamp = timestamp;
}
void sei_message_append(sei_t* sei, sei_message_t* msg)
{
if (0 == sei->head) {
sei->head = msg;
sei->tail = msg;
} else {
sei->tail->next = msg;
sei->tail = msg;
}
}
void sei_cat(sei_t* to, sei_t* from, int itu_t_t35)
{
if (!to || !from) {
return;
}
sei_message_t* msg = NULL;
for (msg = sei_message_head(from); msg; msg = sei_message_next(msg)) {
if (itu_t_t35 || sei_type_user_data_registered_itu_t_t35 != msg->type) {
sei_message_append(to, sei_message_copy(msg));
}
}
}
void sei_free(sei_t* sei)
{
sei_message_t* tail;
while (sei->head) {
tail = sei->head->next;
free(sei->head);
sei->head = tail;
}
sei_init(sei, 0);
}
void sei_dump(sei_t* sei)
{
fprintf(stderr, "SEI %p\n", sei);
sei_dump_messages(sei->head, sei->timestamp);
}
void sei_dump_messages(sei_message_t* head, double timestamp)
{
cea708_t cea708;
sei_message_t* msg;
cea708_init(&cea708, timestamp);
for (msg = head; msg; msg = sei_message_next(msg)) {
uint8_t* data = sei_message_data(msg);
size_t size = sei_message_size(msg);
fprintf(stderr, "-- Message %p\n-- Message Type: %d\n-- Message Size: %d\n", data, sei_message_type(msg), (int)size);
while (size) {
fprintf(stderr, "%02X ", *data);
++data;
--size;
}
fprintf(stderr, "\n");
if (sei_type_user_data_registered_itu_t_t35 == sei_message_type(msg)) {
if (LIBCAPTION_OK != cea708_parse_h262(sei_message_data(msg), sei_message_size(msg), &cea708)) {
fprintf(stderr, "cea708_parse error\n");
} else {
cea708_dump(&cea708);
}
}
}
}
size_t sei_render_size(sei_t* sei)
{
if (!sei || !sei->head) {
return 0;
}
size_t size = 2; 
sei_message_t* msg;
for (msg = sei_message_head(sei); msg; msg = sei_message_next(msg)) {
size += 1 + (msg->type / 255);
size += 1 + (msg->size / 255);
size += 1 + (msg->size * 4 / 3);
}
return size;
}
size_t sei_render(sei_t* sei, uint8_t* data)
{
if (!sei || !sei->head) {
return 0;
}
size_t escaped_size, size = 2; 
sei_message_t* msg;
(*data) = 6;
++data;
for (msg = sei_message_head(sei); msg; msg = sei_message_next(msg)) {
int payloadType = sei_message_type(msg);
int payloadSize = (int)sei_message_size(msg);
uint8_t* payloadData = sei_message_data(msg);
while (255 <= payloadType) {
(*data) = 255;
++data;
++size;
payloadType -= 255;
}
(*data) = payloadType;
++data;
++size;
while (255 <= payloadSize) {
(*data) = 255;
++data;
++size;
payloadSize -= 255;
}
(*data) = payloadSize;
++data;
++size;
if (0 >= (escaped_size = _copy_from_rbsp(data, payloadData, payloadSize))) {
return 0;
}
data += escaped_size;
size += escaped_size;
}
(*data) = 0x80;
return size;
}
uint8_t* sei_render_alloc(sei_t* sei, size_t* size)
{
size_t aloc = sei_render_size(sei);
uint8_t* data = malloc(aloc);
(*size) = sei_render(sei, data);
return data;
}
libcaption_stauts_t sei_parse(sei_t* sei, const uint8_t* data, size_t size, double timestamp)
{
sei_init(sei, timestamp);
int ret = 0;
while (1 < size) {
size_t payloadType = 0;
size_t payloadSize = 0;
while (0 < size && 255 == (*data)) {
payloadType += 255;
++data, --size;
}
if (0 == size) {
return LIBCAPTION_ERROR;
}
payloadType += (*data);
++data, --size;
while (0 < size && 255 == (*data)) {
payloadSize += 255;
++data, --size;
}
if (0 == size) {
return LIBCAPTION_ERROR;
}
payloadSize += (*data);
++data, --size;
if (payloadSize) {
sei_message_t* msg = sei_message_new((sei_msgtype_t)payloadType, 0, payloadSize);
uint8_t* payloadData = sei_message_data(msg);
size_t bytes = _copy_to_rbsp(payloadData, payloadSize, data, size);
sei_message_append(sei, msg);
if (bytes < payloadSize) {
return LIBCAPTION_ERROR;
}
data += bytes;
size -= bytes;
++ret;
}
}
return LIBCAPTION_OK;
}
libcaption_stauts_t sei_to_caption_frame(sei_t* sei, caption_frame_t* frame)
{
cea708_t cea708;
sei_message_t* msg;
libcaption_stauts_t status = LIBCAPTION_OK;
cea708_init(&cea708, frame->timestamp);
for (msg = sei_message_head(sei); msg; msg = sei_message_next(msg)) {
if (sei_type_user_data_registered_itu_t_t35 == sei_message_type(msg)) {
cea708_parse_h264(sei_message_data(msg), sei_message_size(msg), &cea708);
status = libcaption_status_update(status, cea708_to_caption_frame(frame, &cea708));
}
}
if (LIBCAPTION_READY == status) {
frame->timestamp = sei->timestamp;
}
return status;
}
#define DEFAULT_CHANNEL 0
void sei_append_708(sei_t* sei, cea708_t* cea708)
{
sei_message_t* msg = sei_message_new(sei_type_user_data_registered_itu_t_t35, 0, CEA608_MAX_SIZE);
msg->size = cea708_render(cea708, sei_message_data(msg), sei_message_size(msg));
sei_message_append(sei, msg);
cea708_init(cea708, sei->timestamp); 
}
void sei_encode_eia608(sei_t* sei, cea708_t* cea708, uint16_t cc_data)
{
if (31 == cea708->user_data.cc_count) {
sei_append_708(sei, cea708);
}
if (0 == cea708->user_data.cc_count) { 
cea708_add_cc_data(cea708, 1, cc_type_ntsc_cc_field_1, eia608_control_command(eia608_control_resume_caption_loading, DEFAULT_CHANNEL));
cea708_add_cc_data(cea708, 1, cc_type_ntsc_cc_field_1, eia608_control_command(eia608_control_resume_caption_loading, DEFAULT_CHANNEL));
}
if (0 == cc_data) { 
sei_encode_eia608(sei, cea708, eia608_control_command(eia608_control_end_of_caption, DEFAULT_CHANNEL));
sei_encode_eia608(sei, cea708, eia608_control_command(eia608_control_end_of_caption, DEFAULT_CHANNEL));
sei_append_708(sei, cea708);
return;
}
cea708_add_cc_data(cea708, 1, cc_type_ntsc_cc_field_1, cc_data);
}
libcaption_stauts_t sei_from_caption_frame(sei_t* sei, caption_frame_t* frame)
{
int r, c;
int unl, prev_unl;
cea708_t cea708;
const char* data;
uint16_t prev_cc_data;
eia608_style_t styl, prev_styl;
sei_init(sei, frame->timestamp);
cea708_init(&cea708, frame->timestamp); 
cea708_add_cc_data(&cea708, 1, cc_type_ntsc_cc_field_1, eia608_control_command(eia608_control_erase_non_displayed_memory, DEFAULT_CHANNEL));
cea708_add_cc_data(&cea708, 1, cc_type_ntsc_cc_field_1, eia608_control_command(eia608_control_resume_caption_loading, DEFAULT_CHANNEL));
for (r = 0; r < SCREEN_ROWS; ++r) {
prev_unl = 0, prev_styl = eia608_style_white;
for (c = 0; c < SCREEN_COLS && 0 == *caption_frame_read_char(frame, r, c, &styl, &unl); ++c) {
}
if (SCREEN_COLS == c) {
continue;
}
if (0 < c || (0 == unl && eia608_style_white == styl)) {
int tab = c % 4;
sei_encode_eia608(sei, &cea708, eia608_row_column_pramble(r, c, DEFAULT_CHANNEL, 0));
if (tab) {
sei_encode_eia608(sei, &cea708, eia608_tab(tab, DEFAULT_CHANNEL));
}
} else {
sei_encode_eia608(sei, &cea708, eia608_row_style_pramble(r, DEFAULT_CHANNEL, styl, unl));
prev_unl = unl, prev_styl = styl;
}
for (prev_cc_data = 0, data = caption_frame_read_char(frame, r, c, 0, 0);
(*data) && c < SCREEN_COLS; ++c, data = caption_frame_read_char(frame, r, c, &styl, &unl)) {
uint16_t cc_data = eia608_from_utf8_1(data, DEFAULT_CHANNEL);
if (unl != prev_unl || styl != prev_styl) {
sei_encode_eia608(sei, &cea708, eia608_midrow_change(DEFAULT_CHANNEL, styl, unl));
prev_unl = unl, prev_styl = styl;
}
if (!cc_data) {
} else if (eia608_is_basicna(prev_cc_data)) {
if (eia608_is_basicna(cc_data)) {
sei_encode_eia608(sei, &cea708, eia608_from_basicna(prev_cc_data, cc_data));
} else if (eia608_is_westeu(cc_data)) {
sei_encode_eia608(sei, &cea708, eia608_from_basicna(prev_cc_data, eia608_from_utf8_1(EIA608_CHAR_SPACE, DEFAULT_CHANNEL)));
sei_encode_eia608(sei, &cea708, cc_data);
} else {
sei_encode_eia608(sei, &cea708, prev_cc_data);
sei_encode_eia608(sei, &cea708, cc_data);
}
prev_cc_data = 0; 
} else if (eia608_is_westeu(cc_data)) {
sei_encode_eia608(sei, &cea708, eia608_from_utf8_1(EIA608_CHAR_SPACE, DEFAULT_CHANNEL));
sei_encode_eia608(sei, &cea708, cc_data);
} else if (eia608_is_basicna(cc_data)) {
prev_cc_data = cc_data;
} else {
sei_encode_eia608(sei, &cea708, cc_data);
}
if (eia608_is_specialna(cc_data)) {
sei_encode_eia608(sei, &cea708, eia608_control_command(eia608_control_resume_caption_loading, DEFAULT_CHANNEL));
}
}
if (0 != prev_cc_data) {
sei_encode_eia608(sei, &cea708, prev_cc_data);
}
}
sei_encode_eia608(sei, &cea708, 0); 
sei->timestamp = frame->timestamp; 
return LIBCAPTION_OK;
}
libcaption_stauts_t sei_from_scc(sei_t* sei, const scc_t* scc)
{
unsigned int i;
cea708_t cea708;
cea708_init(&cea708, sei->timestamp); 
for (i = 0; i < scc->cc_size; ++i) {
if (31 == cea708.user_data.cc_count) {
sei_append_708(sei, &cea708);
}
cea708_add_cc_data(&cea708, 1, cc_type_ntsc_cc_field_1, scc->cc_data[i]);
}
if (0 != cea708.user_data.cc_count) {
sei_append_708(sei, &cea708);
}
return LIBCAPTION_OK;
}
libcaption_stauts_t sei_from_caption_clear(sei_t* sei)
{
cea708_t cea708;
cea708_init(&cea708, sei->timestamp); 
cea708_add_cc_data(&cea708, 1, cc_type_ntsc_cc_field_1, eia608_control_command(eia608_control_end_of_caption, DEFAULT_CHANNEL));
cea708_add_cc_data(&cea708, 1, cc_type_ntsc_cc_field_1, eia608_control_command(eia608_control_end_of_caption, DEFAULT_CHANNEL));
cea708_add_cc_data(&cea708, 1, cc_type_ntsc_cc_field_1, eia608_control_command(eia608_control_erase_non_displayed_memory, DEFAULT_CHANNEL));
cea708_add_cc_data(&cea708, 1, cc_type_ntsc_cc_field_1, eia608_control_command(eia608_control_erase_non_displayed_memory, DEFAULT_CHANNEL));
cea708_add_cc_data(&cea708, 1, cc_type_ntsc_cc_field_1, eia608_control_command(eia608_control_erase_display_memory, DEFAULT_CHANNEL));
cea708_add_cc_data(&cea708, 1, cc_type_ntsc_cc_field_1, eia608_control_command(eia608_control_erase_display_memory, DEFAULT_CHANNEL));
sei_append_708(sei, &cea708);
return LIBCAPTION_OK;
}
void mpeg_bitstream_init(mpeg_bitstream_t* packet)
{
packet->dts = 0;
packet->cts = 0;
packet->size = 0;
packet->front = 0;
packet->latent = 0;
packet->status = LIBCAPTION_OK;
}
uint8_t mpeg_bitstream_packet_type(mpeg_bitstream_t* packet, unsigned stream_type)
{
if (4 > packet->size) {
return 0;
}
switch (stream_type) {
case STREAM_TYPE_H262:
return packet->data[3];
case STREAM_TYPE_H264:
return packet->data[3] & 0x1F;
case STREAM_TYPE_H265:
return (packet->data[3] >> 1) & 0x3F;
default:
return 0;
}
}
static size_t find_start_code(const uint8_t* data, size_t size)
{
uint32_t start_code = 0xffffffff;
for (size_t i = 1; i < size; ++i) {
start_code = (start_code << 8) | data[i];
if (0x00000100 == (start_code & 0xffffff00)) {
return i - 3;
}
}
return 0;
}
cea708_t* _mpeg_bitstream_cea708_at(mpeg_bitstream_t* packet, size_t pos) { return &packet->cea708[(packet->front + pos) % MAX_REFRENCE_FRAMES]; }
cea708_t* _mpeg_bitstream_cea708_front(mpeg_bitstream_t* packet) { return _mpeg_bitstream_cea708_at(packet, 0); }
cea708_t* _mpeg_bitstream_cea708_back(mpeg_bitstream_t* packet) { return _mpeg_bitstream_cea708_at(packet, packet->latent - 1); }
cea708_t* _mpeg_bitstream_cea708_emplace_back(mpeg_bitstream_t* packet, double timestamp)
{
++packet->latent;
cea708_t* cea708 = _mpeg_bitstream_cea708_back(packet);
cea708_init(cea708, timestamp);
return cea708;
}
void _mpeg_bitstream_cea708_sort(mpeg_bitstream_t* packet)
{
again:
for (size_t i = 1; i < packet->latent; ++i) {
cea708_t c;
cea708_t* a = _mpeg_bitstream_cea708_at(packet, i - 1);
cea708_t* b = _mpeg_bitstream_cea708_at(packet, i);
if (a->timestamp > b->timestamp) {
memcpy(&c, a, sizeof(cea708_t));
memcpy(a, b, sizeof(cea708_t));
memcpy(b, &c, sizeof(cea708_t));
goto again;
}
}
}
size_t mpeg_bitstream_flush(mpeg_bitstream_t* packet, caption_frame_t* frame)
{
if (packet->latent) {
cea708_t* cea708 = _mpeg_bitstream_cea708_front(packet);
packet->status = libcaption_status_update(LIBCAPTION_OK, cea708_to_caption_frame(frame, cea708));
packet->front = (packet->front + 1) % MAX_REFRENCE_FRAMES;
--packet->latent;
}
return packet->latent;
}
void _mpeg_bitstream_cea708_sort_flush(mpeg_bitstream_t* packet, caption_frame_t* frame, double dts)
{
_mpeg_bitstream_cea708_sort(packet);
while (packet->latent && packet->status == LIBCAPTION_OK && _mpeg_bitstream_cea708_front(packet)->timestamp < dts) {
mpeg_bitstream_flush(packet, frame);
}
}
size_t mpeg_bitstream_parse(mpeg_bitstream_t* packet, caption_frame_t* frame, const uint8_t* data, size_t size, unsigned stream_type, double dts, double cts)
{
if (MAX_NALU_SIZE <= packet->size) {
packet->status = LIBCAPTION_ERROR;
return 0;
}
if (MAX_NALU_SIZE <= packet->size + size) {
size = MAX_NALU_SIZE - packet->size;
}
sei_t sei;
size_t header_size, scpos;
packet->status = LIBCAPTION_OK;
memcpy(&packet->data[packet->size], data, size);
packet->size += size;
while (packet->status == LIBCAPTION_OK && 0 < (scpos = find_start_code(&packet->data[0], packet->size))) {
switch (mpeg_bitstream_packet_type(packet, stream_type)) {
default:
break;
case H262_SEI_PACKET:
header_size = 4;
if (STREAM_TYPE_H262 == stream_type && scpos > header_size) {
cea708_t* cea708 = _mpeg_bitstream_cea708_emplace_back(packet, dts + cts);
packet->status = libcaption_status_update(packet->status, cea708_parse_h262(&packet->data[header_size], scpos - header_size, cea708));
_mpeg_bitstream_cea708_sort_flush(packet, frame, dts);
}
break;
case H264_SEI_PACKET:
case H265_SEI_PACKET:
header_size = STREAM_TYPE_H264 == stream_type ? 4 : STREAM_TYPE_H265 == stream_type ? 5 : 0;
if (header_size && scpos > header_size) {
packet->status = libcaption_status_update(packet->status, sei_parse(&sei, &packet->data[header_size], scpos - header_size, dts + cts));
for (sei_message_t* msg = sei_message_head(&sei); msg; msg = sei_message_next(msg)) {
if (sei_type_user_data_registered_itu_t_t35 == sei_message_type(msg)) {
cea708_t* cea708 = _mpeg_bitstream_cea708_emplace_back(packet, dts + cts);
packet->status = libcaption_status_update(packet->status, cea708_parse_h264(sei_message_data(msg), sei_message_size(msg), cea708));
_mpeg_bitstream_cea708_sort_flush(packet, frame, dts);
}
}
}
break;
}
packet->size -= scpos;
memmove(&packet->data[0], &packet->data[scpos], packet->size);
}
return size;
}
