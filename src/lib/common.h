// common.h - 

class Constraint;
class ConstraintQueue;

GPtrArray *read_words(char *filename);
gchar *read_grid(char *filename, GSList **wordlist, GSList **letterlist, GSList **constraintlist);
void init_vars(GSList *words, GSList *letters, GPtrArray *dictionary);
void find_solution(GSList *words, GSList *letters, gchar *grid, gint depth, ConstraintQueue& queue);

#define MAX_GRID 128

