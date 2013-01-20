#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "arccc_wrapper.h"

int main(int argc, char *argv[])
{
  if (argc != 4) {
    printf("usage: %s grid1 grid2 dictionary\n", argv[0]);
    exit (-1);
  }

  HArccc* arccc = arccc_new(argv[3]);
  arccc_read_grid_file(arccc, argv[1]);
  arccc_run(arccc);
  printf("%s", arccc_get_grid(arccc));
  
  arccc_read_grid_file(arccc, argv[2]);
  arccc_run(arccc);
  printf("%s", arccc_get_grid(arccc));

  return (0);
}

