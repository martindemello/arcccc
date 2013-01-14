/* constraint.c - */

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "constraint.h"
#include "lettervar.h"
#include "wordvar.h"

namespace {

void set_letter(struct lettervar *l)
{
  for (int i = 0; i < 256; i++) {
    if (l->letters_allowed[i]) {
      *(l->pos) = i;
      return;
    }
  }
}

}  // namespace


typedef unsigned int uint;

void ConstraintQueue::Drain() {
  while (!queue_.empty()) {
    Constraint* c = queue_.front();
    queue_.pop();
    c->on_queue = false;
  }
}

bool ConstraintQueue::Run() {
  while (!queue_.empty()) {
    Constraint *c = queue_.front();
    queue_.pop();

    assert(c->on_queue);
    c->on_queue = false;

    if (!c->Trigger(*this)) {
      Drain();
      return false;
    }
  }

  return true;
}

void ConstraintQueue::AddConstraint(Constraint* c) {
  assert(c != NULL);

  if (!c->on_queue) {
    queue_.push(c);
    c->on_queue = true;
  }
}

OverlapConstraint::OverlapConstraint(struct wordvar* _w, struct lettervar* _l, int _offset)
  : w(_w), l(_l), offset(_offset) {
  on_queue = false;
}

bool OverlapConstraint::Trigger(ConstraintQueue& queue)
{
  // This function is called only when some entry in
  // l->letters_allowed has changed from true to false.

  // loop through the word values, removing impossible ones
  for (int i = 0; i < w->possible_values->len; i++) {
    char ch = ((char *) g_ptr_array_index(w->possible_values, i))[offset];
    if (l->letters_allowed[(uint) ch] == false) {
      if (queue.WordlistRemoveIndex(w, i) == false) return (false);
      i--;
    } 
  }

  // fail if the word list is now empty
  return (w->possible_values->len > 0);
}

UniquenessConstraint::UniquenessConstraint(struct wordvar* _w, GSList* _other_words)
  : w(_w), other_words(_other_words) {
  on_queue = false;
}

bool UniquenessConstraint::Trigger(ConstraintQueue& queue) {
  GSList *temp;
  char *unique_word = (char*) g_ptr_array_index(w->possible_values, 0);

  // check that constraint should be triggered
  if (w->possible_values->len > 1) return (true);

  for (temp = other_words; temp != NULL; temp = temp->next) {
    struct wordvar *ow = (wordvar*) temp->data;
    GPtrArray *wordlist = ow->possible_values;
    int i;

    for (i = 0; i < wordlist->len; i++) {
      if (g_ptr_array_index(wordlist, i) == unique_word) {
        if (queue.WordlistRemoveIndex(ow, i) == false) return (false);

        // fail if the word list is now empty
        if (wordlist->len == 0) {
#if DEBUG
          printf("died because of unique\n");
#endif
          return (false);
        }
        
        break;
      }
    }
  }

  return (true);
}

bool ConstraintQueue::WordlistRemoveIndex(struct wordvar *w, int index)
{
  GPtrArray *wordlist = w->possible_values;
  char *temp;
  int i;

#if DEBUG
  printf("removing word %s\n", g_ptr_array_index(wordlist, index));
#endif

  // swap pointer to end of list, trim length of list by one.
  temp = (char*) g_ptr_array_index(wordlist, index);
  g_ptr_array_index(wordlist, index) = g_ptr_array_index(wordlist, wordlist->len-1);
  g_ptr_array_index(wordlist, wordlist->len-1) = temp;
  wordlist->len--;

  // loop over characters, decrementing counts in corresponding lettervar
  for (i = 0; i < w->length; i++) {
    if ((-- (w->letter_counts[i][(uint) temp[i]])) == 0) {
      // the support for some letter has been removed, so trigger
      // the constraint in other direction
      OverlapConstraint *c = w->orthogonal_constraints[i];
      if (c) {
        struct lettervar *l = c->l;

        // it's possible that this letter has already been removed
        if (l->letters_allowed[(uint) temp[i]] == false) continue;
        
        l->letters_allowed[(uint) temp[i]] = false;
        l->num_letters_allowed--;
        if (l->num_letters_allowed == 0) {
#if DEBUG
          printf("died for lack of letters\n");
#endif
          return (false);
        }
          
        if (l->num_letters_allowed == 1) set_letter(l);
          
        AddConstraint((Constraint *) c);
      }
    }
  }

  // trigger uniqueness constraint if needed
  if (w->possible_values->len == 1) {
    AddConstraint((Constraint *) w->unique_constraint);
  }

  return (true);
}
