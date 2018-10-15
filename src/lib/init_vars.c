/* init_vars.c - */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "wordvar.h"
#include "lettervar.h"

void
init_vars(GSList *words, GSList *letters, GPtrArray *dictionary)
{
  GSList *p;

  init_wordvars(words, dictionary);

  for (p = letters; p != NULL; p = p->next) {
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
