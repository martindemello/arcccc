#include <stdbool.h>

typedef struct Queue Queue;
typedef struct Constraint Constraint;

Queue* queue_new();

bool run_constraint_queue(Queue* queue);

Queue* add_constraint_to_queue(Queue* queue, Constraint* cptr);
