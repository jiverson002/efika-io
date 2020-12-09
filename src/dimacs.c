/* SPDX-License-Identifier: MIT */
#include <stdio.h>

#include "efika/core.h"
#include "efika/io.h"

#include "efika/core/pp.h"
#include "efika/io/rename.h"

/*----------------------------------------------------------------------------*/
/*! Function to write a dimacs file. */
/*----------------------------------------------------------------------------*/
EFIKA_EXPORT int
IO_dimacs_save(char const * const filename, Matrix const * const M)
{
  /* validate input */
  if (!pp_all(filename, M))
    return -1;

  /* open output file */
  FILE * ostream = fopen(filename, "w");
  if (!ostream)
    return -1;

  /* unpack /M/ */
  int   const fmt = M->fmt;
  ind_t const nr  = M->nr;
  ind_t const nnz = M->nnz;
  ind_t const * const ia = M->ia;
  ind_t const * const ja = M->ja;
  val_t const * const a  = M->a;

  fprintf(ostream, "p %s "PRIind" "PRIind"\n",
    has_adjwgt(fmt) ? "sp" : "edge", nr, nnz);

  for (ind_t i = 0; i < nr; i++) {
    for (ind_t j = ia[i]; j < ia[i + 1]; j++) {
      if (has_adjwgt(fmt))
        fprintf(ostream, "a "PRIind" "PRIind" "PRIval"\n", i+1, ja[j]+1, a[j]);
      else
        fprintf(ostream, "e "PRIind" "PRIind"\n", i+1, ja[j]+1);
    }
  }

  /* ... */
  fclose(ostream);

  return 0;
}
