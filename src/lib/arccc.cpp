#include "arccc.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdlib.h>
#include <string>

namespace {

void push_state(GSList *words, GSList *letters)
{
  GSList *p;

  for (p = words; p != NULL; p = p->next) {
    struct wordvar *w = (wordvar*) p->data;
    g_ptr_array_add(w->stack, (void *) w->possible_values->len);
  }

  for (p = letters; p != NULL; p = p->next) {
    struct lettervar *l = (lettervar*) p->data;
    g_array_append_vals(l->stack, l, 1);
  }
}

void pop_state(GSList *words, GSList *letters)
{
  GSList *p;

  for (p = words; p != NULL; p = p->next) {
    struct wordvar *w = (wordvar*) p->data;
    w->possible_values->len = (guint) g_ptr_array_index(w->stack, w->stack->len-1);
    g_ptr_array_set_size(w->stack, w->stack->len-1);
  }

  for (p = letters; p != NULL; p = p->next) {
    struct lettervar *l = (lettervar*) p->data;
    *l = g_array_index(l->stack, struct lettervar, l->stack->len-1);
    g_array_set_size(l->stack, l->stack->len-1);
  }
}

std::string slurp(const char* filename) {
  std::ostringstream os;
  std::ifstream file(filename);
  os << file.rdbuf();
  file.close();
  return os.str();
}

struct Frame {
  int min;
  int depth;
  int i;
  int return_pt;
  struct lettervar* next_to_try;
  char gridsnap[MAX_GRID*MAX_GRID];
  gboolean letters_to_try[256];

  Frame() {
    min = 0;
    return_pt = 0;
    depth = 0;
    i = 0;
    next_to_try = NULL;
  }
};

}  // namespace


Arccc::Arccc(char* dictionary_file) :
  maxdepth_(0), total_(0), count_(0) {
  printf("dictionary %s\n", dictionary_file);
  dictionary_ = read_words(dictionary_file);
}

void Arccc::ReadGridFile(const char* grid_file) {
  std::string grid = slurp(grid_file);
  ReadGrid(grid.c_str());
}

void Arccc::ReadGrid(const char* grid) {
  grid_ = read_grid(grid, &words_, &letters_, &constraints_);
  Init();
  printf("%s\n", grid_);
}

void Arccc::Init() {
  GSList *p;

  for (p = words_; p != NULL; p = p->next) {
    struct wordvar *w = (wordvar*) p->data;
    unsigned int i;

    w->possible_values = g_ptr_array_new();

    for (i = 0; i < dictionary_->len; i++) {
      char *dword = (char*) g_ptr_array_index(dictionary_, i);
      gint j;

      // check that the lengths match
      if (strlen(dword) != w->length) continue;

      // check that the word matches the constraints
      for (j = 0; j < w->length; j++) {
        if (w->letters[j]->letters_allowed[(guint) dword[j]] != true) break;
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
    struct lettervar *l = (lettervar*) p->data;
    gint i, val = 0;

    l->num_letters_allowed = 0;
    // update allowed letters
    for (i = 0; i < 256; i++) {
      if ((l->letter_counts[0][i] > 0) && (l->letter_counts[1][i] > 0)) {
        l->letters_allowed[i] = true;
        l->num_letters_allowed++;
        val = i;
      } else {
        l->letters_allowed[i] = false;
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

void Arccc::Run() {
  GSList* ll;

  for (ll = constraints_; ll != NULL; ll = ll->next) {
    Constraint *c = (Constraint*) ll->data;
    queue_.AddConstraint(c);
  }

  queue_.Run();

  for (ll = constraints_; ll != NULL; ll = ll->next) {
    Constraint *c = (Constraint*) ll->data;
    queue_.AddConstraint(c);
  }

  FindSolution();
}

typedef std::shared_ptr<Frame> FramePtr;

void Arccc::FindSolution()
{
  FramePtr f, frame;
  Frame* nf;
  std::vector<FramePtr> stack;
  f.reset(new Frame);
  stack.push_back(f);

  while(true) {
top:
    frame = stack.back();
    count_++;

    if (frame->depth >= maxdepth_) {
      maxdepth_ = frame->depth;
    }

    push_state(words_, letters_);
    strcpy(frame->gridsnap, grid_);

    if (queue_.Run()) {
      frame->min = 257;
      frame->next_to_try = NULL;

      // find the most constrained (but still not set) letter
      for (GSList* ll = letters_; ll != NULL; ll = ll->next) {
        struct lettervar *l = (lettervar*) ll->data;

        if (l->num_letters_allowed == 1) continue;

        if (l->num_letters_allowed < frame->min) {
          frame->min = l->num_letters_allowed;
          frame->next_to_try = l;
        }
      }

      if (frame->next_to_try == NULL) {
        total_++;
        printf("depth %d (%d, %d)\n%s\n\n", frame->depth, count_, total_, grid_);
        pop_state(words_, letters_);
        strcpy(grid_, frame->gridsnap);
        return;
      }

      memcpy(frame->letters_to_try, frame->next_to_try->letters_allowed, sizeof (frame->letters_to_try));
      memset(frame->next_to_try->letters_allowed, 0, sizeof (frame->letters_to_try));

      for (frame->i = 0; frame->i < 256; frame->i++) {
        if (frame->letters_to_try[frame->i]) {
          frame->next_to_try->letters_allowed[frame->i] = true;
          frame->next_to_try->num_letters_allowed = 1;

          if (frame->depth == 0) {
            *(frame->next_to_try->pos) = frame->i - 'a' + 'A';
          } else {
            *(frame->next_to_try->pos) = frame->i;
          }

          queue_.AddConstraint((Constraint*) frame->next_to_try->constraints[0]);
          queue_.AddConstraint((Constraint*) frame->next_to_try->constraints[1]);

          // In place of the recursive call
          //   FindSolution(frame->depth + 1);
          // we allocate a new frame, push it onto the stack, and jump back to the start.
          // Setting return_pt = 1 causes the function to jump back to recur: instead of returning.
          nf = new Frame();
          nf->depth = frame->depth + 1;
          nf->return_pt = 1;
          f.reset(nf);
          stack.push_back(f);
          goto top;
recur:
          frame->next_to_try->letters_allowed[frame->i] = false;
        }
      }
    }

    pop_state(words_, letters_);
    strcpy(grid_, frame->gridsnap);

    // This is where the recursive function would return; instead we pop the
    // current frame off the stack, reset the frame pointer to the new stack
    // top, and jump back to just after the old recursive call.
    //
    // We needn't check for stack.empty() before calling pop_back(), since the
    // first frame pushed onto the stack has return_pt = 0, and will therefore
    // exit the function.
    if (frame->return_pt == 1) {
      stack.pop_back();
      frame = stack.back();
      goto recur;
    } else {
      return;
    }
  }
}
