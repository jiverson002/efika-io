/* SPDX-License-Identifier: MIT */
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "efika/core.h"
#include "efika/io.h"

#include "efika/core/gc.h"
#include "efika/core/pp.h"
#include "efika/io/getline.h"
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
static inline intmax_t
getline_nc(char ** const lineptr, size_t * const n, FILE * const istream)
{
  intptr_t ret;

  /* skip comment lines */
  do {
    ret = IO_getline(lineptr, n, istream);
  } while (0 < ret && '%' == *lineptr[0]);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*! Function to read a cluto file. */
/*----------------------------------------------------------------------------*/
EFIKA_EXPORT int
IO_cluto_load(char const * const filename, Matrix * const M)
{
  /* ...garbage collected function... */
  GC_func_init();

  size_t n = 0;
  ind_t nr, nc, nnz;
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

  /* get first non-comment line in file */
  GC_assert(0 < getline_nc(&line, &n, istream));

  /* read the file header */
  GC_assert(3 == sscanf(line, PRIind" "PRIind" "PRIind"\n", &nr, &nc, &nnz));

  /* allocate memory for /M/ */
  ind_t * const ia = GC_malloc((nr + 1) * sizeof(*ia));
  ind_t * const ja = GC_malloc(nnz * sizeof(*ja));
  val_t * const a  = GC_malloc(nnz * sizeof(*a));

  /* read the sparse matrix file */
  ind_t i = 0, j = 0;
  ia[0] = 0;
  while (0 < getline_nc(&line, &n, istream)) {
    char *head = line, *tail;

    /* parse the line */
    while (1) {
      /* parse the index and its corresponding value */
      ind_t const ind = strtoi(head, &tail);
      val_t const val = strtov(tail, &head);

      /* no conversion took place, which means the end of the line or an error
       * occurred */
      if (head == tail) {
        GC_assert(ERANGE != errno);
        break;
      }

      /* insist that not all non-zeros have already been read */
      GC_assert(j < nnz);

      /* insist that index is valid */
      GC_assert(0 < ind && ind <= nc);

      /* record parsed values */
      ja[j]  = ind - 1;
      a[j++] = val;
    }

    /* insist that not all rows have already been read */
    GC_assert(i <= nr);

    /* record the current number of non-zeros */
    ia[++i] = j;
  }

  /* insist that correct number of values were read */
  GC_assert(i == nr);
  GC_assert(j == nnz);

  /* record relevant info in /M/ */
  /*M->fmt  = 0;*/
  /*M->diag = 0;*/
  M->sort   = NONE;
  /*M->symm = 0;*/
  M->nr     = nr;
  M->nc     = nc;
  M->nnz    = nnz;
  /*M->ncon = 0;*/
  M->ia     = ia;
  M->ja     = ja;
  M->a      = a;

  GC_free(istream);
  GC_free(line);

  return 0;
}

/*----------------------------------------------------------------------------*/
/*! Function to write a cluto file. */
/*----------------------------------------------------------------------------*/
EFIKA_EXPORT int
IO_cluto_save(char const * const filename, Matrix const * const M)
{
  /* validate input */
  if (!pp_all(filename, M))
    return -1;

  /* open output file */
  FILE * ostream = fopen(filename, "w");
  if (!ostream)
    return -1;

  /* unpack /M/ */
  ind_t const nr  = M->nr;
  ind_t const nc  = M->nc;
  ind_t const nnz = M->nnz;
  ind_t const * const ia = M->ia;
  ind_t const * const ja = M->ja;
  val_t const * const a  = M->a;

  fprintf(ostream, PRIind" "PRIind" "PRIind"\n", nr, nc, nnz);

  for (ind_t i = 0; i < nr; i++) {
    for (ind_t j = ia[i]; j < ia[i + 1]; j++)
      fprintf(ostream, PRIind" "PRIval" ", ja[j]+1, a[j]);
    fprintf(ostream, "\n");
  }

  /* ... */
  fclose(ostream);

  return 0;
}
