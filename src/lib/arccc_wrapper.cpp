#include "arccc.h"

struct HArccc : public Arccc { };

extern "C" {
  HArccc* arccc_new(const char* dictionary_file) {
    return (HArccc*) new Arccc(dictionary_file);
  }

  bool arccc_read_grid(HArccc* arccc, const char* grid) {
    arccc->ReadGrid(grid);
    return true;
  }

  bool arccc_read_grid_file(HArccc* arccc, const char* grid_file) {
    arccc->ReadGridFile(grid_file);
    return true;
  }

  void arccc_run(HArccc* arccc) {
    arccc->Run();
  }

  char* arccc_get_grid(HArccc* arccc) {
    return arccc->grid_;
  }
}
