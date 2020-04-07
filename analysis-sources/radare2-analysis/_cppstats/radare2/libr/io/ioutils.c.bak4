

#include <r_io.h>
#include <r_util.h>
#include <r_types.h>
#include "io_private.h"



R_API bool r_io_addr_is_mapped(RIO *io, ut64 vaddr) {
r_return_val_if_fail (io, false);
return (io->va && r_io_map_get (io, vaddr));
}





R_API bool r_io_is_valid_offset(RIO* io, ut64 offset, int hasperm) {
r_return_val_if_fail (io, false);
if (io->va) {
if (!hasperm) {

RIOMap* map = r_io_map_get (io, offset);
return map? map->perm & R_PERM_R: false;
}
RIOMap* map = r_io_map_get (io, offset);
return map? (map->perm & hasperm) == hasperm: false;
}
if (!io->desc) {
return false;
}
if (offset > r_io_desc_size (io->desc)) {
return false;
}
return ((io->desc->perm & hasperm) == hasperm);
}


R_API bool r_io_read_i(RIO* io, ut64 addr, ut64 *val, int size, bool endian) {
ut8 buf[8];
r_return_val_if_fail (io && val, false);
size = R_DIM (size, 1, 8);
if (!r_io_read_at (io, addr, buf, size)) {
return false;
}

*val = r_read_ble (buf, endian, size * 8);
return true;
}


R_API bool r_io_write_i(RIO* io, ut64 addr, ut64 *val, int size, bool endian) {
ut8 buf[8];
r_return_val_if_fail (io && val, false);
size = R_DIM (size, 1, 8);

r_write_ble (buf, *val, endian, size * 8);
return r_io_write_at (io, addr, buf, size) == size;
}
