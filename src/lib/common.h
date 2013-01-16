// common.h - 

GPtrArray *read_words(char *filename);
gchar *read_grid(const char *grid, GSList **wordlist, GSList **letterlist, GSList **constraintlist);

#define MAX_GRID 128
