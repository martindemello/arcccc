/* constraint.c - */

#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "constraint.h"
#include "lettervar.h"
#include "wordvar.h"

static gboolean wordlist_remove_index(struct wordvar *w, int index);
static void set_letter(struct lettervar *l);

static void *queue = NULL;

gboolean
run_constraints(void)
{
  return run_constraint_queue(queue);
}

void set_on_queue_false(struct constraint* c) {
  c->on_queue = FALSE;
}

void set_on_queue_true(struct constraint* c) {
  c->on_queue = TRUE;
}

gboolean get_on_queue(struct constraint* c) {
  return c->on_queue;
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
trigger_constraint(struct constraint *c)
{
  // trigger the constraint, and return its success value
  return ((c->func)(c));
}


gboolean
revise_word_letter(struct overlap_constraint *c)
{
  // This function is called only when some entry in
  // c->l->letters_allowed has changed from TRUE to FALSE.

  struct wordvar *w = c->w;
  struct lettervar *l = c->l;
  gint offset = c->offset;
  gint i;

  // loop through the word values, removing impossible ones
  for (i = 0; i < w->possible_values->len; i++) {
    gchar ch = ((gchar *) g_ptr_array_index(w->possible_values, i))[offset];
    if (l->letters_allowed[(guint) ch] == FALSE) {
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

static gboolean
wordlist_remove_index(struct wordvar *w, int index)
{
  GPtrArray *wordlist = w->possible_values;
  gchar *temp;
  gint i;

#if DEBUG
  printf("removing word %s\n", g_ptr_array_index(wordlist, index));
#endif

  // swap pointer to end of list, trim length of list by one.
  temp = g_ptr_array_index(wordlist, index);
  g_ptr_array_index(wordlist, index) = g_ptr_array_index(wordlist, wordlist->len-1);
  g_ptr_array_index(wordlist, wordlist->len-1) = temp;
  wordlist->len--;

  // loop over characters, decrementing counts in corresponding lettervar
  for (i = 0; i < w->length; i++) {
    if ((-- (w->letter_counts[i][(guint) temp[i]])) == 0) {
      // the support for some letter has been removed, so trigger
      // the constraint in other direction
      OverlapConstraint* oc = w->orthogonal_constraints[i];
      struct overlap_constraint *c = oc->constraint;
      if (c) {
        struct lettervar *l = c->l;

        // it's possible that this letter has already been removed
        if (l->letters_allowed[(guint) temp[i]] == FALSE) continue;
        
        l->letters_allowed[(guint) temp[i]] = FALSE;
        l->num_letters_allowed--;
        if (l->num_letters_allowed == 0) {
#if DEBUG
          printf("died for lack of letters\n");
#endif
          return (FALSE);
        }
          
        if (l->num_letters_allowed == 1) set_letter(l);
          
        put_constraint_on_queue((void *) oc);
      }
    }
  }

  // trigger uniqueness constraint if needed
  if (w->possible_values->len == 1) {
    put_constraint_on_queue((void *) w->unique_constraint);
  }

  return (TRUE);
}

struct overlap_constraint *new_overlap_constraint(struct wordvar *w, 
                                                  struct lettervar *l,
                                                  gint offset)
{
  struct overlap_constraint *c;
  
  c = g_malloc(sizeof (struct overlap_constraint));

  c->func = (constraint_function) revise_word_letter;
  c->on_queue = FALSE;
  c->w = w;
  c->l = l;
  c->offset = offset;

  return (c);
}

struct uniqueness_constraint *new_uniqueness_constraint(struct wordvar *w,
                                                        GSList *other_words)
{
  struct uniqueness_constraint *c;
  
  c = g_malloc(sizeof (struct uniqueness_constraint));

  c->func = (constraint_function) revise_word_unique;
  c->on_queue = FALSE;
  c->w = w;
  c->other_words = other_words;

  return (c);
}

static void
set_letter(struct lettervar *l)
{
  gint i;

  for (i = 0; i < 256; i++) 
    if (l->letters_allowed[i]) {
      *(l->pos) = i;
      return;
    }
}

unsigned char get_tag(struct Constraint* c) {
  return c->tag;
}
