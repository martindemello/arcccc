#pragma once

#include <queue>

struct wordvar;
struct lettervar;
class ConstraintQueue;

typedef enum {
  VALUE_CHANGE,
  VALUE_INSTANTIATION,
} trigger_type;

class Constraint {
  public:
    virtual bool Trigger(ConstraintQueue& queue) = 0;
    bool on_queue;
    int data;
};

// word to letter constraint
class OverlapConstraint : public Constraint {
  public:
    OverlapConstraint(struct wordvar *w, struct lettervar *l, int offset);
    struct wordvar* w;
    struct lettervar* l;
    int offset; // offset of letter into word
    bool Trigger(ConstraintQueue& queue);
};

// uniqueness constraint
class UniquenessConstraint : public Constraint {
  public:
    UniquenessConstraint(struct wordvar *w, GSList *other_words);
    struct wordvar *w;
    GSList *other_words;
    bool Trigger(ConstraintQueue& queue);
};

class ConstraintQueue {
  public:
    bool Run();
    void AddConstraint(Constraint* c);
    bool WordlistRemoveIndex(struct wordvar *w, int index);

  private:
    void Drain();
    std::queue<Constraint*> queue_;
};
