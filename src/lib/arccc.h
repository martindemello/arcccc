#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "wordvar.h"
#include "lettervar.h"
#include "constraint.h"
#include <iostream>

class Arccc {
  public:
    Arccc(char* dictionary_file);
    void ReadGrid(const char* grid);
    void ReadGridFile(const char* grid_file);
    void Run();

    char* grid_;

  private:
    void FindSolution();
    void Init();

    GSList* words_;
    GSList* letters_;
    GSList* constraints_;
    GPtrArray* dictionary_;
    ConstraintQueue queue_;
    int maxdepth_;
    int total_;
    int count_;
};
