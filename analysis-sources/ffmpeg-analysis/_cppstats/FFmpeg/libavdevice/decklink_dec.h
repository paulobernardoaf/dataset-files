#if defined(__cplusplus)
extern "C" {
#endif
int ff_decklink_read_header(AVFormatContext *avctx);
int ff_decklink_read_packet(AVFormatContext *avctx, AVPacket *pkt);
int ff_decklink_read_close(AVFormatContext *avctx);
int ff_decklink_list_input_devices(AVFormatContext *avctx, struct AVDeviceInfoList *device_list);
#if defined(__cplusplus)
} 
#endif
