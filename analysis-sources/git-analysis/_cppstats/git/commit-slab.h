#include "commit-slab-decl.h"
#include "commit-slab-impl.h"
#define define_commit_slab(slabname, elemtype) declare_commit_slab(slabname, elemtype); implement_static_commit_slab(slabname, elemtype)
