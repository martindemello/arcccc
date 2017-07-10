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

gboolean
revise_word_letter(struct overlap_constraint *c)
{
  // This function is called only when some entry in
  // c->l->letters_allowed has changed from TRUE to FALSE.

  struct wordvar *w = c->w;
  LetterVar *l = c->l;
  gint offset = c->offset;
  gint i;

  // loop through the word values, removing impossible ones
  for (i = 0; i < w->possible_values->len; i++) {
    gchar ch = ((gchar *) g_ptr_array_index(w->possible_values, i))[offset];
    if (lettervar_letter_allowed(l, ch) == FALSE) {
      if (wordlist_remove_index(w, i) == FALSE) return (FALSE);
      i--;
    } 
  }

  // fail if the word list is now empty
  return (w->possible_values->len > 0);
}


gboolean
revise_word_unique(struct uniqueness_constraint *c)
{
  GSList *temp;
  gchar *unique_word = g_ptr_array_index(c->w->possible_values, 0);

  // check that constraint should be triggered
  if (c->w->possible_values->len > 1) return (TRUE);

  for (temp = c->other_words; temp != NULL; temp = temp->next) {
    struct wordvar *ow = temp->data;
    GPtrArray *wordlist = ow->possible_values;
    gint i;

    for (i = 0; i < wordlist->len; i++) {
      if (g_ptr_array_index(wordlist, i) == unique_word) {
        if (wordlist_remove_index(ow, i) == FALSE) return (FALSE);

        // fail if the word list is now empty
        if (wordlist->len == 0) {
#if DEBUG
          printf("died because of unique\n");
#endif
          return (FALSE);
        }
        
        break;
      }
    }
  }

  return (TRUE);
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
