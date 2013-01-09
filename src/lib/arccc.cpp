#include "arccc.h"

#include <string>
#include <cstring>
#include <stdlib.h>

namespace {

void push_state(GSList *words, GSList *letters)
{
  GSList *p;
  GSList *newwords = NULL, *newletters = NULL;
  
  for (p = words; p != NULL; p = p->next) {
    struct wordvar *w = p->data;
    g_ptr_array_add(w->stack, (void *) w->possible_values->len);
  }
  
  for (p = letters; p != NULL; p = p->next) {
    struct lettervar *l = p->data;
    g_array_append_vals(l->stack, l, 1);
  }
}

void pop_state(GSList *words, GSList *letters)
{
  GSList *p;

  for (p = words; p != NULL; p = p->next) {
    struct wordvar *w = p->data;
    w->possible_values->len = (guint) g_ptr_array_index(w->stack, w->stack->len-1);
    g_ptr_array_set_size(w->stack, w->stack->len-1);
  }

  for (p = letters; p != NULL; p = p->next) {
    struct lettervar *l = p->data;
    *l = g_array_index(l->stack, struct lettervar, l->stack->len-1);
    g_array_set_size(l->stack, l->stack->len-1);
  }
}

}  // namespace


Arccc::Arccc(char* dictionary_file) {
  printf("dictionary %s\n", dictionary_file);
  dictionary_ = read_words(dictionary_file);
}

Arccc::ReadGrid(char* grid_file) {
  grid_ = read_grid(grid_file, &words_, &letters_, &constraints_);
  Init();
  printf("%s\n", grid_);
}

void Arccc::Init() {
  GSList *p;

  for (p = words_; p != NULL; p = p->next) {
    struct wordvar *w = p->data;
    gint i;

    w->possible_values = g_ptr_array_new();

    for (i = 0; i < dictionary_->len; i++) {
      gchar *dword = g_ptr_array_index(dictionary_, i);
      gint j;

      // check that the lengths match
      if (strlen(dword) != w->length) continue;

      // check that the word matches the constraints
      for (j = 0; j < w->length; j++) {
        if (w->letters[j]->letters_allowed[(guint) dword[j]] != TRUE) break;
      }
      if (j < w->length) continue;
      
      // add this word to the possible values
      g_ptr_array_add(w->possible_values, dword);
      for (j = 0; j < w->length; j++) {
        w->letter_counts[j][(guint) dword[j]]++;
      }
    }

    if (w->possible_values->len == 0) {
      printf("Die: No words for %s.\n", w->name->str);
      exit(-1);
    }
  }

  for (p = letters_; p != NULL; p = p->next) {
    struct lettervar *l = p->data;
    gint i, val = 0;
    
    l->num_letters_allowed = 0;
    // update allowed letters
    for (i = 0; i < 256; i++) {
      if ((l->letter_counts[0][i] > 0) && (l->letter_counts[1][i] > 0)) {
        l->letters_allowed[i] = TRUE;
        l->num_letters_allowed++;
        val = i;
      } else {
        l->letters_allowed[i] = FALSE;
      }
    }

    if (l->num_letters_allowed == 0) {
      printf("Die: No letters for %s.\n", l->name->str);
      exit(-1);
    } 
    if (l->num_letters_allowed == 1) {
      *(l->pos) = val;
    }
  }
}

Arccc::Run() {
  GSList* ll;

  struct wordvar* w = words_->data;
  printf("Initial %d\n", w->possible_values->len);

  for (ll = constraints_; ll != NULL; ll = ll->next) {
    Constraint *c = ll->data;
    queue_.AddConstraint(c);
  }

  queue_.Run();

  printf("First %d\n", w->possible_values->len);

  for (ll = constraints_; ll != NULL; ll = ll->next) {
    Constraint *c = ll->data;
    queue_.AddConstraint(c);
  }


  total = 0;
  FindSolution(words_, letters_, grid_, 0, queue_);
  printf("total %d\n", total);
}

void Arccc::FindSolution(GSList *words, GSList *letters, gchar *grid, gint depth, ConstraintQueue& queue)
{
  GSList *ll;
  gchar gridsnap[MAX_GRID*MAX_GRID];
  static gint count = 0;
  static gint maxdepth = 0;

  count++;
    
  if (depth >= maxdepth) {
    printf("depth %d (%d, %d)\n%s\n\n", maxdepth = depth, count, total, grid);
  }

  push_state(words, letters);
  strcpy(gridsnap, grid);
  
  if (queue.Run()) {
    gint min = 257;
    struct lettervar *next_to_try = NULL;
    gboolean letters_to_try[256];
    gint i;
    
    // find the most constrained (but still not set) letter
    for (ll = letters; ll != NULL; ll = ll->next) {
      struct lettervar *l = ll->data;
      
      if (l->num_letters_allowed == 1) continue;

      if (l->num_letters_allowed < min) {
        min = l->num_letters_allowed;
        next_to_try = l;
      }
    }


    if (next_to_try == NULL) {
      total++;
      printf("depth %d (%d, %d)\n%s\n\n", depth, count, total, grid);
      pop_state(words, letters);
      strcpy(grid, gridsnap);
      return;
    }

    memcpy(letters_to_try, next_to_try->letters_allowed, sizeof (letters_to_try));
    memset(next_to_try->letters_allowed, 0, sizeof (letters_to_try));

    
    for (i = 0; i < 256; i++) {
      if (letters_to_try[i]) {
        next_to_try->letters_allowed[i] = TRUE;
        next_to_try->num_letters_allowed = 1;
        
        if (depth == 0) {
          *(next_to_try->pos) = i - 'a' + 'A';
        } else {
          *(next_to_try->pos) = i;
        }
        
        queue.AddConstraint((Constraint*) next_to_try->constraints[0]);
        queue.AddConstraint((Constraint*) next_to_try->constraints[1]);
        FindSolution(words, letters, grid, depth + 1, queue);

        next_to_try->letters_allowed[i] = FALSE;
      }
    }
  }

  pop_state(words, letters);
  strcpy(grid, gridsnap);
}
