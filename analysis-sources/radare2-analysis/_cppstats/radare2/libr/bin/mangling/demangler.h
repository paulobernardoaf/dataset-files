#include "demangler_types.h"
EDemanglerErr create_demangler(SDemangler **demangler);
EDemanglerErr init_demangler(SDemangler *demangler, char *sym);
void free_demangler(SDemangler *demangler);
