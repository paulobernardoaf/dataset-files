#if !defined(R_PROTOBUF_H)
#define R_PROTOBUF_H

#if defined(__cplusplus)
extern "C" {
#endif

R_API char *r_protobuf_decode(const ut8* buffer, const ut64 size, bool debug);

#if defined(__cplusplus)
}
#endif

#endif 
