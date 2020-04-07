#if defined(__cplusplus)
extern "C" {
#endif
int ff_decklink_write_header(AVFormatContext *avctx);
int ff_decklink_write_packet(AVFormatContext *avctx, AVPacket *pkt);
int ff_decklink_write_trailer(AVFormatContext *avctx);
int ff_decklink_list_output_devices(AVFormatContext *avctx, struct AVDeviceInfoList *device_list);
#if defined(__cplusplus)
} 
#endif
