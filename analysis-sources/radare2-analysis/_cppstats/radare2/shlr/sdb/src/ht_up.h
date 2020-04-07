#define HT_TYPE 2
#include "ht_inc.h"
SDB_API HtName_(Ht)* Ht_(new0)(void);
SDB_API HtName_(Ht)* Ht_(new)(HT_(DupValue) valdup, HT_(KvFreeFunc) pair_free, HT_(CalcSizeV) valueSize);
SDB_API HtName_(Ht)* Ht_(new_size)(ut32 initial_size, HT_(DupValue) valdup, HT_(KvFreeFunc) pair_free, HT_(CalcSizeV) valueSize);
#undef HT_TYPE
