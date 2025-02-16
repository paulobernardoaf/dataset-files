#include "../libgdbr.h"
#include "r_types_base.h"
#include <r_util.h>
bool gdbr_lock_enter(libgdbr_t *g);
void gdbr_lock_leave(libgdbr_t *g);
int gdbr_connect(libgdbr_t *g, const char *server, int port);
int gdbr_disconnect(libgdbr_t *g);
void gdbr_invalidate_reg_cache(void);
int gdbr_stop_reason(libgdbr_t *g);
int gdbr_check_extended_mode(libgdbr_t *g);
int gdbr_check_vcont(libgdbr_t *g);
int gdbr_send_qRcmd(libgdbr_t *g, const char *cmd, PrintfCallback cb_printf);
int gdbr_attach(libgdbr_t *g, int pid);
int gdbr_detach(libgdbr_t *g);
int gdbr_detach_pid(libgdbr_t *g, int pid);
int gdbr_kill(libgdbr_t *g);
int gdbr_kill_pid(libgdbr_t *g, int pid);
int gdbr_continue(libgdbr_t *g, int pid, int tid, int sig);
int gdbr_step(libgdbr_t *g, int thread_id);
int gdbr_read_registers(libgdbr_t *g);
int gdbr_write_bin_registers(libgdbr_t *g, const char *regs, int len);
int gdbr_write_reg(libgdbr_t *g, const char *name, char *value, int len);
int gdbr_write_register(libgdbr_t *g, int index, char *value, int len);
int gdbr_write_registers(libgdbr_t *g, char *registers);
int gdbr_read_memory(libgdbr_t *g, ut64 address, ut8 *buf, int len);
int gdbr_write_memory(libgdbr_t *g, ut64 address, const uint8_t *data, ut64 len);
int test_command(libgdbr_t *g, const char *command);
int gdbr_set_bp(libgdbr_t *g, ut64 address, const char *conditions, int sizebp);
int gdbr_set_hwbp(libgdbr_t *g, ut64 address, const char *conditions, int sizebp);
int gdbr_set_hww(libgdbr_t *g, ut64 address, const char *conditions, int sizebp);
int gdbr_set_hwr(libgdbr_t *g, ut64 address, const char *conditions, int sizebp);
int gdbr_set_hwa(libgdbr_t *g, ut64 address, const char *conditions, int sizebp);
int gdbr_remove_bp(libgdbr_t *g, ut64 address, int sizebp);
int gdbr_remove_hwbp(libgdbr_t *g, ut64 address, int sizebp);
int gdbr_remove_hww(libgdbr_t *g, ut64 address, int sizebp);
int gdbr_remove_hwr(libgdbr_t *g, ut64 address, int sizebp);
int gdbr_remove_hwa(libgdbr_t *g, ut64 address, int sizebp);
int gdbr_open_file(libgdbr_t *g, const char *filename, int flags, int mode);
int gdbr_read_file(libgdbr_t *g, ut8 *buf, ut64 max_len);
int gdbr_close_file(libgdbr_t *g);
RList* gdbr_threads_list(libgdbr_t *g, int pid);
RList* gdbr_pids_list(libgdbr_t *g, int pid);
char* gdbr_exec_file_read(libgdbr_t *g, int pid);
ut64 gdbr_get_baddr(libgdbr_t *g);
int gdbr_select(libgdbr_t *g, int pid, int tid);
