#if (defined(USE_ELF) || defined(HAVE_MACH_O_LOADER_H))
void
rb_dump_backtrace_with_lines(int num_traces, void **traces);
#endif 
