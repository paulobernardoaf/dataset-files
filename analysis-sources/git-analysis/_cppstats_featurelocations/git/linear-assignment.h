#if !defined(LINEAR_ASSIGNMENT_H)
#define LINEAR_ASSIGNMENT_H













void compute_assignment(int column_count, int row_count, int *cost,
int *column2row, int *row2column);


#define COST_MAX (1<<16)

#endif
