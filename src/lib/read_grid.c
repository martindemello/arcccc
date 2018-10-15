/* read_grid.c - */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "constraint.h"
#include "wordvar.h"
#include "lettervar.h"

gchar *
read_grid(char *filename, GSList **wordlist, GSList **letterlist, GSList **constraintlist)
{
  FILE *fp;
  gchar grid[MAX_GRID][MAX_GRID];
  gchar *gridstr, *p;
  gpointer awgrid[MAX_GRID][MAX_GRID], dwgrid[MAX_GRID][MAX_GRID];
  int row, col, maxrow;
  int wcount = 0;
  GSList *l;
  struct OverlapConstraint *oca;
  struct OverlapConstraint *ocd;

  fp = fopen(filename, "r");
  g_assert(fp != NULL);

  *wordlist = *letterlist = *constraintlist = NULL;

  p = gridstr = g_malloc((MAX_GRID+1) * (MAX_GRID+1));

  row = 0;
  while(fgets(grid[row++], MAX_GRID, fp) != NULL);
  memset(grid[row], 0, MAX_GRID);
  maxrow = row;

  for (row = 0; row < maxrow; row++) {
    for (col = 0; col < MAX_GRID; col++) {
      if (grid[row][col] == '\n') {
        grid[row][col] = '\0';
      }
      awgrid[row][col] = NULL;
      dwgrid[row][col] = NULL;
    }
  }

  /* scan out words, storing them in awgrid and dwgrid */
  for (row = 0; row < maxrow; row++) {
    for (col = 0; grid[row][col] != '\0'; col++) {
      LetterVar *l;
      WordVar *aw, *dw;
      int wcountcur;

      /* at a block? */
      if (grid[row][col] == '#') {
        *(p++) = '#';
        continue;
      }

      // create new lettervar
      *p = grid[row][col];
      l = make_lettervar(grid[row][col], p++);
      *letterlist = g_slist_prepend(*letterlist, l);

      // necessary to not increment twice on simultaneous new across&down
      wcountcur = wcount;

      /* extend across word or start new one */
      if ((col > 0) && (awgrid[row][col-1] != NULL)) {
        aw = awgrid[row][col-1];
        aw->length++;
        if (aw->length >= 4) {
          aw->letters = g_realloc(aw->letters, (aw->length+1) * sizeof (LetterVar *));
          aw->letter_counts = g_realloc(aw->letter_counts, (aw->length+1) * sizeof (gint *));
          aw->orthogonal_constraints = g_realloc(aw->orthogonal_constraints, 
                                                 (aw->length+1) * sizeof (struct OverlapConstraint *));
        }
      } else {
        aw = g_malloc0(sizeof (struct wordvar));
        *wordlist = g_slist_prepend(*wordlist, (gpointer) aw);
        wcount++;
        aw->name = g_string_new("");
        g_string_sprintf(aw->name, "%d across", wcount);
        aw->stack = g_ptr_array_new();
        // allocate space for 4 letters by default
        aw->letters = g_malloc(4 * sizeof (LetterVar *));
        aw->letter_counts = g_malloc(4 * sizeof (gint *));
        aw->orthogonal_constraints = g_malloc(4 * sizeof (struct OverlapConstraint *));
      }
      awgrid[row][col] = aw;
      

      /* extend down word or start new one */
      if ((row > 0) && (dwgrid[row-1][col] != NULL)) {
        dw = dwgrid[row-1][col];
        dw->length++;
        if (dw->length >= 4) {
          dw->letters = g_realloc(dw->letters, (dw->length+1) * sizeof (LetterVar *));
          dw->letter_counts = g_realloc(dw->letter_counts, (dw->length+1) * sizeof(gint *));
          dw->orthogonal_constraints = g_realloc(dw->orthogonal_constraints, 
                                                 (dw->length+1) * sizeof (struct OverlapConstraint *));
        }
      } else {
        dw = g_malloc0(sizeof (struct wordvar));
        *wordlist = g_slist_prepend(*wordlist, dw);
        wcount = wcountcur + 1;
        dw->name = g_string_new("");
        g_string_sprintf(dw->name, "%d down", wcount);
        dw->stack = g_ptr_array_new();
        // allocate space for 4 letters by default
        dw->letters = g_malloc(4 * sizeof (LetterVar *));
        dw->letter_counts = g_malloc(4 * sizeof(gint *));
        dw->orthogonal_constraints = g_malloc(4 * sizeof (struct OverlapConstraint *));
      }
      dwgrid[row][col] = dw;

      // useful
      lettervar_set_name(l, aw->name, dw->name, row, col);

      /* create overlap constraints */
      oca = make_overlap_constraint(aw, l, aw->length);
      ocd = make_overlap_constraint(dw, l, dw->length);
      *constraintlist = g_slist_prepend(*constraintlist, oca);
      *constraintlist = g_slist_prepend(*constraintlist, ocd);

      /* link letters and constraints */
      lettervar_set_constraints(l, oca, ocd);
      
      /* connect across and down */
      aw->orthogonal_constraints[aw->length] = ocd;
      dw->orthogonal_constraints[dw->length] = oca;

      /* connect words to letters */
      aw->letters[aw->length] = l;
      dw->letters[dw->length] = l;

      /* set up letter_counts crosspointers */
      aw->letter_counts[aw->length] = &(l->letter_counts[0][0]);
      dw->letter_counts[dw->length] = &(l->letter_counts[1][0]);
    }

    *(p++) = '\n';
  }      

  /* word lengths were not incremented for final character */
  for (l = *wordlist; l != NULL; l = l->next) {
    struct wordvar *w = l->data;
    w->length++;
  }

  /* set up uniqueness constraints */
  for (l = *wordlist; l != NULL; l = l->next) {
    GSList *l2, *samelen;
    struct wordvar *w1 = l->data;
    struct UniquenessConstraint *uc;

    samelen = NULL;

    for (l2 = *wordlist; l2 != NULL; l2 = l2->next) {
      struct wordvar *w2 = l2->data;
      if (w1 == w2) continue;
      if (w1->length != w2->length) continue;

      samelen = g_slist_prepend(samelen, w2);
    }

    uc = make_uniqueness_constraint(w1, samelen);
    w1->unique_constraint = uc;
    *constraintlist = g_slist_prepend(*constraintlist, uc);
  }

  fclose(fp);
  return (gridstr);
}
