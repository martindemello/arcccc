/* test.c - */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "wordvar.h"
#include "lettervar.h"
#include "constraint.h"

gint total = 0;

int main(int argc, char *argv[])
{
  GSList *words, *letters, *constraints, *ll;
  GPtrArray *dictionary;
  gchar *grid;
  struct wordvar *w;
  Queue* queue = (Queue *) queue_new();

  if (argc != 3) {
    printf("usage: %s grid dictionary\n", argv[0]);
    exit (-1);
  }
  
  words = letters = constraints = NULL;

  grid = read_grid(argv[1], &words, &letters, &constraints);
  dictionary = read_words(argv[2]);
  init_vars(words, letters, dictionary);

  printf("%s\n", grid);

  w = words->data;
  printf("Initial %d\n", w->possible_values->len);

  for (ll = constraints; ll != NULL; ll = ll->next) {
    struct constraint *c = ll->data;
    put_constraint_on_queue(queue, (void *) c);
  }

  run_constraints(queue);

  printf("First %d\n", w->possible_values->len);

  for (ll = constraints; ll != NULL; ll = ll->next) {
    struct constraint *c = ll->data;
    put_constraint_on_queue(queue, (void *) c);
  }


  total = 0;
  find_solution(queue, words, letters, grid, 0);
  printf("total %d\n", total);


  return (0);
}

