



















#if !defined(AVFORMAT_RTPPROTO_H)
#define AVFORMAT_RTPPROTO_H

#include "url.h"

int ff_rtp_set_remote_url(URLContext *h, const char *uri);

int ff_rtp_get_local_rtp_port(URLContext *h);

#endif 
