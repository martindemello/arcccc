#pragma once

struct lettervar;
class OverlapConstraint;
class UniquenessConstraint;

struct wordvar {
  GPtrArray *possible_values;
  gint length; // length of this word
  struct lettervar **letters;
  gint **letter_counts; // dimensions are [length][256] (pointers into lettervars)
  GPtrArray *stack; // for backtracking
  OverlapConstraint **orthogonal_constraints;
  UniquenessConstraint *unique_constraint;
  GString *name;
};
