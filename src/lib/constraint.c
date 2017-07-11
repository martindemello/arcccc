/* constraint.c - */

#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "constraint.h"
#include "lettervar.h"
#include "wordvar.h"

static void *queue = NULL;

gboolean
run_constraints(void)
{
  return run_constraint_queue(queue);
}

void
put_constraint_on_queue(void* c)
{
  g_assert(c != NULL);
  if (queue == NULL) {
    queue = (void *) queue_new();
  }
  queue = (void *) add_constraint_to_queue(queue, c);
}

// g_ptr_array_index is not mapped in rust's glib_sys crate.
gchar* wordlist_ptr_to_index(GPtrArray* wordlist, int index) {
   return (gchar *) g_ptr_array_index(wordlist, index);
}

// swap pointer to end of list, trim length of list by one.
gchar* wordlist_swap_index_with_end(GPtrArray* wordlist, int index) {
  gchar *temp = g_ptr_array_index(wordlist, index);
  g_ptr_array_index(wordlist, index) =
    g_ptr_array_index(wordlist, wordlist->len-1);
  g_ptr_array_index(wordlist, wordlist->len-1) = temp;
  wordlist->len--;
  return temp;
}

unsigned char get_tag(struct Constraint* c) {
  return c->tag;
}
