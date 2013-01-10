#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "wordvar.h"
#include "lettervar.h"
#include "constraint.h"

class Arccc {
  public:
    Arccc(char* dictionary_file); 
    ReadGrid(char* grid_file); 
    Run();

    char* grid_;

  private:
    void FindSolution(int depth);
    void Init();
    
    GSList* words_;
    GSList* letters_;
    GSList* constraints_;
    GPtrArray* dictionary_;
    ConstraintQueue queue_;
    int maxdepth_;
    int total;
    int count;
};
