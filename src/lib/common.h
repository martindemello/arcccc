// common.h - 

// defined in rust
struct Queue;
typedef struct Queue Queue;
struct Dictionary;
typedef struct Dictionary Dictionary;

Dictionary *read_words(char *filename);
gchar *read_grid(char *filename, GSList **wordlist, GSList **letterlist, GSList **constraintlist);
gboolean run_constraints(Queue* queue);
void put_constraint_on_queue(Queue* queue, void* c);
void find_solution(Queue* queue, GSList *words, GSList *letters, gchar *grid, gint depth);

#define MAX_GRID 128
