#if defined(HAVE_CONFIG_H)
#include "../auto/config.h"
#endif
#if defined(HAVE_ATTRIBUTE_UNUSED)
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif
#if defined(_MSC_VER)
#define inline __inline
#endif
#if !defined(XINCLUDE_H)
#define XINCLUDE_H
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include <string.h>
#include <limits.h>
#include "xmacros.h"
#include "xdiff.h"
#include "xtypes.h"
#include "xutils.h"
#include "xprepare.h"
#include "xdiffi.h"
#include "xemit.h"
#endif 
