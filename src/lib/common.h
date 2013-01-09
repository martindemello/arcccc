// common.h - 

GPtrArray *read_words(char *filename);
gchar *read_grid(char *filename, GSList **wordlist, GSList **letterlist, GSList **constraintlist);

#define MAX_GRID 128

