

#if !defined(MDMP_PE64_H)
#define MDMP_PE64_H

#define R_BIN_PE64 1

#if defined(MDMP_PE_H)
#undef MDMP_PE_H
#include "mdmp_pe.h"
#else
#include "mdmp_pe.h"
#undef MDMP_PE_H
#endif

#endif 
