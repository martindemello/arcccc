// constraint.h -

struct constraint;
struct wordvar;
struct lettervar;

typedef enum {
  VALUE_CHANGE,
  VALUE_INSTANTIATION,
} trigger_type;

// the general form of a constraint
struct constraint {
  gboolean on_queue;
  gint data;
};

// word to letter constraint
struct overlap_constraint {
  gboolean on_queue;
  struct wordvar *w;
  struct lettervar *l;
  int offset; // offset of letter into word
};

// uniqueness constraint
struct uniqueness_constraint {
  gboolean on_queue;
  struct wordvar *w;
  GSList *other_words;
};

// Rust exports

struct Constraint {
  unsigned char tag;
  void* constraint;
};

unsigned char get_tag(struct Constraint* c);

struct UniquenessConstraint {
  unsigned char tag;
  struct uniqueness_constraint* constraint;
};

typedef struct UniquenessConstraint UniquenessConstraint;

struct OverlapConstraint {
  unsigned char tag;
  struct overlap_constraint* constraint;
};

typedef struct OverlapConstraint OverlapConstraint;

struct OverlapConstraint *make_overlap_constraint(struct wordvar *w,
                                                  struct lettervar *l,
                                                  gint offset);

struct UniquenessConstraint *make_uniqueness_constraint(struct wordvar *w,
                                                        GSList *other_words);
