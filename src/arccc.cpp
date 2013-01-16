/* test.c - */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "wordvar.h"
#include "lettervar.h"
#include "constraint.h"
#include "arccc.h"

int main(int argc, char *argv[])
{
  if (argc != 3) {
    printf("usage: %s grid dictionary\n", argv[0]);
    exit (-1);
  }

  Arccc arccc(argv[2]);
  arccc.ReadGridFile(argv[1]);
  arccc.Run();

  return (0);
}

