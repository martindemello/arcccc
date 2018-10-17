// lettervar.h - 

typedef struct OverlapConstraint OverlapConstraint;


// constraint between a letter square and one word in that position
struct lettervar {
  gint letter_counts[2][256]; // support for each letter from the across=0 or down=1 words
  gboolean letters_allowed[256]; // letters that can appear in this word
  gint num_letters_allowed;
  struct OverlapConstraint *constraints[2];
  GArray *stack; // for backtracking
  GString *name;
  gchar *pos;
};

typedef struct lettervar LetterVar;

LetterVar* make_lettervar(char chr, char* p);
    
void init_lettervars(GSList* letters);

void lettervar_set_name(LetterVar* lptr, GString* aw, GString* dw, int row, int col);
    
void lettervar_set_constraints(LetterVar* lptr, OverlapConstraint* oca, OverlapConstraint* ocd);
    
void set_letter(LetterVar* lptr);

gboolean lettervar_letter_allowed(LetterVar* lptr, char i);

int lettervar_set_letter_allowed(LetterVar* lptr, char i, int t);

int lettervar_num_letters_allowed(LetterVar* lptr);
