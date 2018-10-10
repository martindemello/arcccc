// wordvar.h - 

struct lettervar;

struct wordvar {
  GPtrArray *possible_values;
  gint length; // length of this word
  struct lettervar **letters;
  gint **letter_counts; // dimensions are [length][256] (pointers into lettervars)
  GPtrArray *stack; // for backtracking
  struct OverlapConstraint **orthogonal_constraints;
  struct UniquenessConstraint *unique_constraint;
  GString *name;
};

void init_wordvars(GSList* words, GPtrArray* dictionary);
