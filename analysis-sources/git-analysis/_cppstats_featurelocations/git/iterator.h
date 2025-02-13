#if !defined(ITERATOR_H)
#define ITERATOR_H









#define ITER_OK 0




#define ITER_DONE -1





#define ITER_ERROR -2






enum iterator_selection {

ITER_SELECT_DONE = ITER_DONE,


ITER_SELECT_ERROR = ITER_ERROR,







ITER_CURRENT_SELECTION_MASK = 0x01,


ITER_CURRENT_SELECTION_0 = 0x00,


ITER_CURRENT_SELECTION_1 = 0x01,


ITER_YIELD_CURRENT = 0x02,


ITER_SKIP_SECONDARY = 0x04,







ITER_SELECT_0 = ITER_CURRENT_SELECTION_0 | ITER_YIELD_CURRENT,


ITER_SELECT_0_SKIP_1 = ITER_SELECT_0 | ITER_SKIP_SECONDARY,


ITER_SKIP_0 = ITER_CURRENT_SELECTION_1 | ITER_SKIP_SECONDARY,


ITER_SELECT_1 = ITER_CURRENT_SELECTION_1 | ITER_YIELD_CURRENT,


ITER_SELECT_1_SKIP_0 = ITER_SELECT_1 | ITER_SKIP_SECONDARY,


ITER_SKIP_1 = ITER_CURRENT_SELECTION_0 | ITER_SKIP_SECONDARY
};

#endif 
