int64_t Ogg_GetKeyframeGranule ( const logical_stream_t *, int64_t i_granule );
bool Ogg_IsKeyFrame ( const logical_stream_t *, const ogg_packet * );
vlc_tick_t Ogg_GranuleToTime( const logical_stream_t *, int64_t i_granule,
bool b_packetstart, bool b_pts );
vlc_tick_t Ogg_SampleToTime( const logical_stream_t *, int64_t i_sample,
bool b_packetstart );
bool Ogg_GranuleIsValid( const logical_stream_t *, int64_t i_granule );
