typedef struct mux_extradata_builder_t mux_extradata_builder_t;
enum mux_extradata_type_e
{
EXTRADATA_ISOBMFF,
};
mux_extradata_builder_t * mux_extradata_builder_New(vlc_fourcc_t, enum mux_extradata_type_e);
void mux_extradata_builder_Delete(mux_extradata_builder_t *);
void mux_extradata_builder_Feed(mux_extradata_builder_t *, const uint8_t *, size_t);
size_t mux_extradata_builder_Get(mux_extradata_builder_t *, const uint8_t **);
