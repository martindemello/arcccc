#include "arccc.h"
#include "arccc_wrapper.h"

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

  bool arccc_run(HArccc* arccc) {
    return arccc->Run();
  }

  char* arccc_get_grid(HArccc* arccc) {
    return arccc->grid_;
  }
}
