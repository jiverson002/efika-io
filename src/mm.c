/* SPDX-License-Identifier: MIT */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "efika/core/gc.h"
#include "efika/core/pp.h"
#include "efika/io/rename.h"
#include "efika/io.h"

/*----------------------------------------------------------------------------*/
/*! Function to read a matrix market file. */
/*----------------------------------------------------------------------------*/
extern int
IO_mm_load(FILE * const istream, Matrix * const M)
{
  /*==========================================================================*/
  GC_func_init();
  /*==========================================================================*/

  int fmt = 0, symm = 0;
  size_t n = 0;
  ind_t i;
  ind_t u, v, nr = 0, nc = 0, nnz = 0, nnnz = 0;
  val_t w;
  char * line = NULL, * tok = NULL;

  /* validate input */
  if (!pp_all(istream, M))
    return -1;

  /* register line with the garbage collector */
  GC_register(&line);

  /* read header line */
  GC_assert(0 < getline(&line, &n, istream));
  GC_assert(NULL != (tok = strtok(line," \t")));
  GC_assert(0 == memcmp(tok, "%%MatrixMarket", 15));
  GC_assert(NULL != (tok = strtok(NULL," \t")));
  GC_assert(0 == memcmp(tok, "matrix", 7));
  GC_assert(NULL != (tok = strtok(NULL," \t")));
  GC_assert(0 == memcmp(tok, "coordinate", 11));
  GC_assert(NULL != (tok = strtok(NULL," ")));
  if (0 == memcmp(tok, "real", 5))
    fmt = 1;
  else
    GC_assert(0 == memcmp(tok, "pattern", 8));
  GC_assert(NULL != (tok = strtok(NULL," \t\n")));
  if (0 == memcmp(tok, "symmetric", 10))
    symm = 1;
  else
    GC_assert(0 == memcmp(tok, "general", 8));

  /* skip comment lines */
  while (0 < getline(&line, &n, istream) && '%' == line[0]);

  /* read size line */
  GC_assert(3 == sscanf(line, IND_T" "IND_T" "IND_T"\n", &nr, &nc, &nnz));

  if (1 == symm) {
    GC_assert(nr == nc);
    nnz *= 2;
  }

  ind_t * const tmp = GC_calloc(nr + 1, sizeof(*tmp));

  while (0 < getline(&line, &n, istream)) {
    if ('%' == line[0])
      continue;

    if (has_adjwgt(fmt))
      GC_assert(3 == sscanf(line, IND_T" "IND_T" "VAL_T"\n", &u, &v, &w));
    else
      GC_assert(2 == sscanf(line, IND_T" "IND_T"\n", &u, &v));

    GC_assert(0 != u && 0 != v);
    GC_assert(u <= nr && v <= nc);
    GC_assert(1 != symm || u >= v);

    tmp[u]++;
    if (1 == symm)
      tmp[v]++;
    nnnz += (ind_t)(1+symm);
  }

  GC_assert(nnz == nnnz);

  ind_t * const ia = GC_malloc((nr + 1) * sizeof(*ia));
  ind_t * const ja = GC_malloc(nnz * sizeof(*ja));
  val_t *a = NULL;
  if (has_adjwgt(fmt))
    a = GC_malloc(nnz * sizeof(*a));

  ia[0] = 0;
  for (i = 1; i <= nr; i++) {
    tmp[i] += tmp[i-1];
    ia[i] = tmp[i];
  }

  rewind(istream);

  /* read header line */
  GC_assert(0 < getline(&line, &n, istream));

  /* skip comment lines */
  while (0 < getline(&line, &n, istream) && '%' == line[0]);
  /* skip size line */

  while (0 < getline(&line, &n, istream)) {
    if ('%' == line[0])
      continue;

    if (has_adjwgt(fmt))
      GC_assert(3 == sscanf(line, IND_T" "IND_T" "VAL_T"\n", &u, &v, &w));
    else
      GC_assert(2 == sscanf(line, IND_T" "IND_T"\n", &u, &v));

    if (has_adjwgt(fmt))
      a[tmp[u-1]] = w;
    ja[tmp[u-1]++] = v-1;
    if (1 == symm) {
      if (has_adjwgt(fmt))
        a[tmp[v-1]] = w;
      ja[tmp[v-1]++] = u-1;
    }
  }

  while (!feof(istream) && 0 < getline(&line, &n, istream))
    GC_assert('%' == line[0]);

  M->fmt   = fmt;
  /*M->diag  = 0;*/
  /*M->sort  = NONE;*/
  M->symm  = symm;
  M->nr    = nr;
  M->nc    = nc;
  M->nnz   = nnz;
  /*M->ncon  = 0;*/
  M->ia    = ia;
  M->ja    = ja;
  M->a     = a;

  GC_free(line);

  return 0;
}

/*----------------------------------------------------------------------------*/
/*! Function to write a matrix market file. */
/*----------------------------------------------------------------------------*/
extern int
IO_mm_save(FILE * const ostream, Matrix const * const M)
{
  /* validate input */
  if (!pp_all(ostream, M))
    return -1;

  /* unpack /M/ */
  int   const fmt  = M->fmt;
  int   const symm = M->symm;
  ind_t const nr  = M->nr;
  ind_t const nnz = M->nnz;
  ind_t const * const ia = M->ia;
  ind_t const * const ja = M->ja;
  val_t const * const a  = M->a;

  fprintf(ostream, "%%%%MatrixMarket matrix coordinate %s %s\n",
    1 == fmt ? "pattern" : " real", 1 == symm ? "symmetric" : "general");

  fprintf(ostream, IND_T" "IND_T" "IND_T"\n", nr, nr, nnz/(ind_t)(1 + symm));

  for (ind_t i = 0; i < nr; i++) {
    for (ind_t j = ia[i]; j < ia[i + 1]; j++) {
      if ((0 == symm) || (1 == symm && i > ja[j])) {
        if (has_adjwgt(fmt))
          fprintf(ostream, IND_T" "IND_T" "VAL_T"\n", i + 1, ja[j] + 1, a[j]);
        else
          fprintf(ostream, IND_T" "IND_T"\n", i + 1, ja[j] + 1);
      }
    }
  }

  return 0;
}
