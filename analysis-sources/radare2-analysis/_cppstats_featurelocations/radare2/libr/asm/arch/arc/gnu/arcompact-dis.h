



















#if !defined(ARCOMPACT_DIS_H)
#define ARCOMPACT_DIS_H


void arc_print_disassembler_options (FILE *stream);

struct arcDisState
arcAnalyzeInstr(bfd_vma address,
disassemble_info* info);

int ARCompact_decodeInstr (bfd_vma address, disassemble_info* info);


#endif 
