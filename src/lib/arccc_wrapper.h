// Opaque handle to an Arccc instance
struct HArccc;
typedef struct HArccc HArccc;

HArccc* arccc_new(const char* dictionary_file);

bool arccc_read_grid(HArccc* arccc, const char* grid);

bool arccc_read_grid_file(HArccc* arccc, const char* grid_file);

bool arccc_run(HArccc* arccc);

char* arccc_get_grid(HArccc* arccc);
