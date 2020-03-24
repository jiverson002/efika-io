/* SPDX-License-Identifier: MIT */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>

#include "../common.h"
#include "../insist.h"

#define INIT_TMPSIZE 1024

/*----------------------------------------------------------------------------*/
/*! Function to read a snap file. */
/*----------------------------------------------------------------------------*/
extern int
IO_snap_load(FILE * const istream, Matrix * const M)
{
  int ret=-1;
  size_t n=0;
  ind_t i, otmpsz, tmpsz=INIT_TMPSIZE;
  ind_t u, v, nr=0, nnz=0;
  val_t w;
  char * line=NULL;
  ind_t * ia=NULL, * ja=NULL, * tmp=NULL;
  val_t * a=NULL;

  insist(NULL != istream);
  insist(NULL != M);

  insist(NULL != (tmp=(ind_t *) calloc((tmpsz+1), sizeof(ind_t))));

  while (0 < getline(&line, &n, istream)) {
    if ('#' == line[0])
      continue;

    insist(2 <= sscanf(line, IND_T" "IND_T" "VAL_T"\n", &u, &v, &w));
    insist(0 != u || 0 != v);

    if (u > nr)
      nr = u;
    if (v > nr)
      nr = v;

    otmpsz = tmpsz;
    while (nr > tmpsz)
      tmpsz *= 2;
    if (tmpsz > otmpsz) {
      insist(NULL != (tmp=(ind_t *)realloc(tmp, (tmpsz+1)*sizeof(ind_t))));
      memset(tmp+otmpsz, 0, (tmpsz+1-otmpsz)*sizeof(ind_t));
    }

    tmp[u]++;
    nnz++;
  }

  insist(NULL != (ia=(ind_t *) malloc((nr+1)*sizeof(ind_t))));
  insist(NULL != (ja=(ind_t *) malloc(nnz*sizeof(ind_t))));

  ia[0] = 0;
  for (i=1; i<=nr; ++i) {
    tmp[i] += tmp[i-1];
    ia[i] = tmp[i];
  }

  rewind(istream);

  while (0 < getline(&line, &n, istream)) {
    if ('#' == line[0])
      continue;

    switch (sscanf(line, IND_T" "IND_T" "VAL_T"\n", &u, &v, &w)) {
      case 3:
      if (NULL == a)
        insist(NULL != (a=(val_t *) malloc(nnz*sizeof(val_t))));
      a[tmp[u-1]] = w;
      /* fall through */

      case 2:
      ja[tmp[u-1]++] = v-1;
      break;
      
      default:
      goto CLEANUP;
    }
  }

  while (!feof(istream) && 0 < getline(&line, &n, istream))
    insist('#' == line[0]);

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

  ret = 0;

  CLEANUP:
  if (-1 == ret) {
    free(ia);
    free(ja);
    free(a);
  }
  free(tmp);
  free(line);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*! Function to write a snap file. */
/*----------------------------------------------------------------------------*/
extern int
IO_snap_save(FILE * const ostream, Matrix const * const M)
{
  /* validate input */
  if (!pp_all(ostream, M))
    return -1;

  /* unpack /M/ */
  ind_t const nr = M->nr;
  ind_t const * const ia = M->ia;
  ind_t const * const ja = M->ja;
  val_t const * const a  = M->a;

  foreach (ind_t i in range(nr)) {
    foreach (ind_t j in range(ia[i], ia[i+1])) {
      fprintf(ostream, IND_T" "IND_T, i+1, ja[j]+1);
      if (NULL != a) {
        fprintf(ostream, " "VAL_T, a[j]);
      }
      fprintf(ostream, "\n");
    }
  }

  return 0;
}
