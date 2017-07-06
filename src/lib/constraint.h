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


struct overlap_constraint *new_overlap_constraint(struct wordvar *w,
                                                  struct lettervar *l,
                                                  gint offset);

struct uniqueness_constraint *new_uniqueness_constraint(struct wordvar *w,
                                                        GSList *other_words);

void set_on_queue_false(struct constraint* c);
void set_on_queue_true(struct constraint* c);
gboolean get_on_queue(struct constraint* c);
gboolean trigger_constraint(struct constraint *c);
gboolean revise_word_letter(struct overlap_constraint *c);
gboolean revise_word_unique(struct uniqueness_constraint *c);

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
