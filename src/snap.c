/* SPDX-License-Identifier: MIT */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "efika/core/gc.h"
#include "efika/core/pp.h"
#include "efika/io/export.h"
#include "efika/io/rename.h"
#include "efika/io.h"

#define INIT_TMPSIZE 1024

/*----------------------------------------------------------------------------*/
/*! Shim to allow fclose to be registered. */
/*----------------------------------------------------------------------------*/
static inline void
vfclose(FILE * file)
{
  (void)fclose(file);
}

/*----------------------------------------------------------------------------*/
/*! Function to read a snap file. */
/*----------------------------------------------------------------------------*/
EFIKA_IO_EXPORT int
IO_snap_load(char const * const filename, Matrix * const M)
{
  /* ...garbage collected function... */
  GC_func_init();

  size_t n = 0;
  ind_t tmpsz = INIT_TMPSIZE;
  ind_t nr = 0, nnz = 0;
  char *line = NULL;

  /* validate input */
  if (!pp_all(filename, M))
    return -1;

  /* register line with the garbage collector */
  GC_register(&line);

  /* open input file */
  FILE * istream = fopen(filename, "r");
  GC_assert(istream);
  GC_register_free(vfclose, istream);

  ind_t *tmp = GC_calloc((tmpsz + 1), sizeof(*tmp));

  while (0 < getline(&line, &n, istream)) {
    if ('#' == line[0])
      continue;

    ind_t u, v;
    val_t w;

    GC_assert(2 <= sscanf(line, PRIind" "PRIind" "PRIval"\n", &u, &v, &w));
    GC_assert(0 != u || 0 != v);

    if (u > nr)
      nr = u;
    if (v > nr)
      nr = v;

    ind_t const otmpsz = tmpsz;
    while (nr > tmpsz)
      tmpsz *= 2;
    if (tmpsz > otmpsz) {
      tmp = GC_realloc(tmp, (tmpsz + 1) * sizeof(*tmp));
      memset(tmp+otmpsz, 0, (tmpsz + 1 - otmpsz) * sizeof(*tmp));
    }

    tmp[u]++;
    nnz++;
  }

  ind_t * const ia = GC_malloc((nr + 1) * sizeof(ind_t));
  ind_t * const ja = GC_malloc(nnz * sizeof(ind_t));
  val_t *a = NULL;

  ia[0] = 0;
  for (ind_t i = 1; i <= nr; i++) {
    tmp[i] += tmp[i-1];
    ia[i] = tmp[i];
  }

  rewind(istream);

  while (0 < getline(&line, &n, istream)) {
    if ('#' == line[0])
      continue;

    ind_t u, v;
    val_t w;

    switch (sscanf(line, PRIind" "PRIind" "PRIval"\n", &u, &v, &w)) {
      case 3:
      if (NULL == a)
        a = GC_malloc(nnz * sizeof(*a));
      a[tmp[u-1]] = w;
      /* fall through */

      case 2:
      ja[tmp[u - 1]++] = v - 1;
      break;

      default:
      GC_return -1;
    }
  }

  while (!feof(istream) && 0 < getline(&line, &n, istream))
    GC_assert('#' == line[0]);

  /*M->fmt   = 0;*/
  /*M->diag  = 0;*/
  /*M->sort  = NONE;*/
  /*M->symm  = 0;*/
  M->nr    = nr;
  M->nc    = nr;
  M->nnz   = nnz;
  /*M->ncon  = 0;*/
  M->ia    = ia;
  M->ja    = ja;
  M->a     = a;

  GC_free(istream);
  GC_free(line);

  return 0;
}

/*----------------------------------------------------------------------------*/
/*! Function to write a snap file. */
/*----------------------------------------------------------------------------*/
EFIKA_IO_EXPORT int
IO_snap_save(char const * const filename, Matrix const * const M)
{
  /* validate input */
  if (!pp_all(filename, M))
    return -1;

  /* open output file */
  FILE * ostream = fopen(filename, "w");
  if (!ostream)
    return -1;

  /* unpack /M/ */
  ind_t const nr = M->nr;
  ind_t const * const ia = M->ia;
  ind_t const * const ja = M->ja;
  val_t const * const a  = M->a;

  for (ind_t i = 0; i < nr; i++) {
    for (ind_t j = ia[i]; j < ia[i + 1]; j++) {
      fprintf(ostream, PRIind" "PRIind, i + 1, ja[j] + 1);
      if (NULL != a)
        fprintf(ostream, " "PRIval, a[j]);
      fprintf(ostream, "\n");
    }
  }

  /* ... */
  fclose(ostream);

  return 0;
}
