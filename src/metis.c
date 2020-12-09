/* SPDX-License-Identifier: MIT */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "efika/core.h"
#include "efika/io.h"

#include "efika/core/gc.h"
#include "efika/core/pp.h"
#include "efika/io/rename.h"

/*----------------------------------------------------------------------------*/
/*! Shim to allow fclose to be registered. */
/*----------------------------------------------------------------------------*/
static inline void
vfclose(FILE * file)
{
  (void)fclose(file);
}

/*----------------------------------------------------------------------------*/
/*! Get next non-comment line from a file. */
/*----------------------------------------------------------------------------*/
static inline ssize_t
getline_nc(char ** const lineptr, size_t * const n, FILE * const istream)
{
  ssize_t ret;

  /* skip comment lines */
  do {
    ret = getline(lineptr, n, istream);
  } while (0 < ret && '%' == *lineptr[0]);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*! Function to read a metis file. */
/*----------------------------------------------------------------------------*/
EFIKA_EXPORT int
IO_metis_load(char const * const filename, Matrix * const M)
{
  /* ...garbage collected function... */
  GC_func_init();

  int fmt = 0;
  size_t n = 0;
  ind_t i, j;
  ind_t nr, nnz, nnnz, ncon = 0;
  char * tok, * line = NULL;

  /* validate input */
  if (!pp_all(filename, M))
    return -1;

  /* register line with the garbage collector */
  GC_register(&line);

  /* open input file */
  FILE * istream = fopen(filename, "r");
  GC_assert(istream);
  GC_register_free(vfclose, istream);

  /* get first non-comment line in file */
  GC_assert(0 < getline_nc(&line, &n, istream));

  switch (sscanf(line, PRIind" "PRIind" %d "PRIind"\n", &nr, &nnz, &fmt, &ncon)) {
    case 4:
    /* validate */
    if (0 == ncon)
      GC_return -1;
    if ( 10 != fmt &&  11 != fmt && 110 != fmt && 111 != fmt)
      GC_return -1;
    /* fall through */

    case 3:
    /* validate */
    if (  0 != fmt &&   1 != fmt &&  10 != fmt &&  11 != fmt && 100 != fmt &&
        101 != fmt && 110 != fmt && 111 != fmt)
      GC_return -1;
    /* fall through */

    case 2:
    nnz *= 2;
    break;

    default:
    GC_return -1;
  }

  ind_t * const ia = GC_malloc((nr + 1) * sizeof(*ia));
  ind_t * const ja = GC_malloc(nnz * sizeof(*ja));
  val_t *a = NULL;
  val_t *vwgt = NULL;
  ind_t *vsiz = NULL;
  if (has_adjwgt(fmt))
    a = GC_malloc(nnz * sizeof(*a));
  if (has_vtxwgt(fmt))
    vwgt = GC_malloc(ncon * nr * sizeof(*vwgt));
  if (has_vtxsiz(fmt))
    vsiz = GC_malloc(nr * sizeof(*vsiz));

  ia[0] = 0;
  for (nnnz = 0, i = 0; i < nr; i++) {
    GC_assert(0 < getline_nc(&line, &n, istream));

    tok = strtok(line, " \t\n");
    if (has_vtxsiz(fmt)) {
      GC_assert(NULL != tok);

      vsiz[i] = strtoi(tok, NULL);
      tok = strtok(NULL, " \t\n");
    }

    if (has_vtxwgt(fmt)) {
      for (j = 0; j < ncon; j++) {
        GC_assert(NULL != tok);

        vwgt[i * ncon + j] = strtov(tok, NULL);
        tok = strtok(NULL, " \t\n");
      }
    }

    while (NULL != tok) {
      ja[nnnz] = strtoi(tok, NULL) - 1;
      tok = strtok(NULL, " \t\n");

      if (has_adjwgt(fmt)) {
        GC_assert(NULL != tok);

        a[nnnz] = strtov(tok, NULL);
        tok = strtok(NULL, " \t\n");
      }

      if (++nnnz == nnz && NULL != tok)
        GC_return -1;
    }
    ia[i+1] = nnnz;
  }
  GC_assert(nnnz == nnz);

  while (!feof(istream) && 0 < getline(&line, &n, istream))
    GC_assert('%' == line[0]);

  M->fmt   = fmt;
  /*M->diag  = 0;*/
  /*M->sort  = NONE;*/
  M->symm  = 1;
  M->nr    = nr;
  M->nc    = nr;
  M->nnz   = nnz;
  M->ia    = ia;
  M->ja    = ja;
  M->a     = a;
  M->ncon  = ncon;
  M->vsiz  = vsiz;
  M->vwgt  = vwgt;

  GC_free(istream);
  GC_free(line);

  return 0;
}

/*----------------------------------------------------------------------------*/
/*! Function to write a metis file. */
/*----------------------------------------------------------------------------*/
EFIKA_EXPORT int
IO_metis_save(char const * const filename, Matrix const * const M)
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
  ind_t const nr   = M->nr;
  ind_t const nnz  = M->nnz;
  ind_t const ncon = M->ncon;
  ind_t const * const ia   = M->ia;
  ind_t const * const ja   = M->ja;
  val_t const * const a    = M->a;
  val_t const * const vwgt = M->vwgt;
  ind_t const * const vsiz = M->vsiz;

  fprintf(ostream, PRIind" "PRIind, nr, nnz/2);
  if (fmt > 0)
    fprintf(ostream, " %03d "PRIind, fmt, ncon);
  fprintf(ostream, "\n");

  for (ind_t i = 0; i < nr; i++) {
    if (has_vtxsiz(fmt))
      fprintf(ostream, PRIind" ", vsiz[i]);

    if (has_vtxwgt(fmt))
      for (ind_t j = 0; j < ncon; j++)
        fprintf(ostream, PRIval" ", vwgt[i * ncon + j]);

    for (ind_t j = ia[i]; j < ia[i + 1]; j++) {
      fprintf(ostream, PRIind" ", ja[j]+1);
      if (has_adjwgt(fmt))
        fprintf(ostream, PRIval" ", a[j]);
    }
    fprintf(ostream, "\n");
  }

  /* ... */
  fclose(ostream);

  return 0;
}
