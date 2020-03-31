/* SPDX-License-Identifier: MIT */
#include <stdio.h>

#include "efika/core/pp.h"
#include "efika/io/export.h"
#include "efika/io/rename.h"
#include "efika/io.h"

/*----------------------------------------------------------------------------*/
/*! Function to write a dimacs file. */
/*----------------------------------------------------------------------------*/
EFIKA_IO_EXPORT int
IO_dimacs_save(FILE * const ostream, Matrix const * const M)
{
  /* validate input */
  if (!pp_all(ostream, M))
    return -1;

  /* unpack /M/ */
  int   const fmt = M->fmt;
  ind_t const nr  = M->nr;
  ind_t const nnz = M->nnz;
  ind_t const * const ia = M->ia;
  ind_t const * const ja = M->ja;
  val_t const * const a  = M->a;

  fprintf(ostream, "p %s "IND_T" "IND_T"\n",
    has_adjwgt(fmt) ? "sp" : "edge", nr, nnz);

  for (ind_t i = 0; i < nr; i++) {
    for (ind_t j = ia[i]; j < ia[i + 1]; j++) {
      if (has_adjwgt(fmt))
        fprintf(ostream, "a "IND_T" "IND_T" "VAL_T"\n", i+1, ja[j]+1, a[j]);
      else
        fprintf(ostream, "e "IND_T" "IND_T"\n", i+1, ja[j]+1);
    }
  }

  return 0;
}
