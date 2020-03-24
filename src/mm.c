/* SPDX-License-Identifier: MIT */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>

#include "../common.h"
#include "../insist.h"

/*----------------------------------------------------------------------------*/
/*! Function to read a matrix market file. */
/*----------------------------------------------------------------------------*/
extern int
IO_mm_load(FILE * const istream, Matrix * const m)
{
  int ret=-1, fmt=0, symm=0;
  size_t n=0;
  ind_t i;
  ind_t u, v, nr=0, nc=0, nnz=0, nnnz=0;
  val_t w;
  char * line=NULL, * tok=NULL;
  ind_t * ia=NULL, * ja=NULL, * tmp=NULL;
  val_t * a=NULL;

  insist(NULL != istream);
  insist(NULL != m);

  /* read header line */
  insist(0 < getline(&line, &n, istream));
  insist(NULL != (tok=strtok(line," \t")));
  insist(0 == memcmp(tok, "%%MatrixMarket", 15));
  insist(NULL != (tok=strtok(NULL," \t")));
  insist(0 == memcmp(tok, "matrix", 7));
  insist(NULL != (tok=strtok(NULL," \t")));
  insist(0 == memcmp(tok, "coordinate", 11));
  insist(NULL != (tok=strtok(NULL," ")));
  if (0 == memcmp(tok, "real", 5))
    fmt = 1;
  else
    insist(0 == memcmp(tok, "pattern", 8));
  insist(NULL != (tok=strtok(NULL," \t\n")));
  if (0 == memcmp(tok, "symmetric", 10))
    symm = 1;
  else
    insist(0 == memcmp(tok, "general", 8));

  /* skip comment lines */
  while (0 < getline(&line, &n, istream) && '%' == line[0]);

  /* read size line */
  insist(3 == sscanf(line, IND_T" "IND_T" "IND_T"\n", &nr, &nc, &nnz));

  if (1 == symm) {
    insist(nr == nc);
    nnz *= 2;
  }

  insist(NULL != (tmp=calloc(nr+1, sizeof(*tmp))));

  while (0 < getline(&line, &n, istream)) {
    if ('%' == line[0])
      continue;

    if (has_adjwgt(fmt))
      insist(3 == sscanf(line, IND_T" "IND_T" "VAL_T"\n", &u, &v, &w));
    else
      insist(2 == sscanf(line, IND_T" "IND_T"\n", &u, &v));

    insist(0 != u && 0 != v);
    insist(u <= nr && v <= nc);
    insist(1 != symm || u >= v);

    tmp[u]++;
    if (1 == symm)
      tmp[v]++;
    nnnz += (ind_t)(1+symm);
  }

  insist(nnz == nnnz);

  insist(NULL != (ia=malloc((nr+1)*sizeof(*ia))));
  insist(NULL != (ja=malloc(nnz*sizeof(*ja))));
  if (has_adjwgt(fmt))
    insist(NULL != (a=malloc(nnz*sizeof(*a))));

  ia[0] = 0;
  for (i=1; i<=nr; ++i) {
    tmp[i] += tmp[i-1];
    ia[i] = tmp[i];
  }

  rewind(istream);

  /* read header line */
  insist(0 < getline(&line, &n, istream));

  /* skip comment lines */
  while (0 < getline(&line, &n, istream) && '%' == line[0]);
  /* skip size line */

  while (0 < getline(&line, &n, istream)) {
    if ('%' == line[0])
      continue;

    if (has_adjwgt(fmt))
      insist(3 == sscanf(line, IND_T" "IND_T" "VAL_T"\n", &u, &v, &w));
    else
      insist(2 == sscanf(line, IND_T" "IND_T"\n", &u, &v));

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
    insist('%' == line[0]);

  m->fmt   = fmt;
  /*m->diag  = 0;*/
  /*m->sort  = NONE;*/
  m->symm  = symm;
  m->nr    = nr;
  m->nc    = nc;
  m->nnz   = nnz;
  /*m->ncon  = 0;*/
  m->ia    = ia;
  m->ja    = ja;
  m->a     = a;

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
/*! Function to write a matrix market file. */
/*----------------------------------------------------------------------------*/
extern int
IO_mm_save(FILE * const ostream, Matrix const * const m)
{
  int fmt, symm, ret=-1;
  ind_t i, j;
  ind_t nr, nnz;
  ind_t const * ia, * ja;
  val_t const * a;

  insist(NULL != ostream);
  insist(NULL != m);

  fmt  = m->fmt;
  symm = m->symm;
  nr   = m->nr;
  nnz  = m->nnz;
  ia   = m->ia;
  ja   = m->ja;
  a    = m->a;

  fprintf(ostream, "%%%%MatrixMarket matrix coordinate %s %s\n",
    1 == fmt ? "pattern" : " real", 1 == symm ? "symmetric" : "general");

  fprintf(ostream, IND_T" "IND_T" "IND_T"\n", nr, nr, nnz/(ind_t)(1+symm));

  for (i=0; i<nr; ++i) {
    for (j=ia[i]; j<ia[i+1]; ++j) {
      if ((0 == symm) || (1 == symm && i > ja[j])) {
        if (has_adjwgt(fmt))
          fprintf(ostream, IND_T" "IND_T" "VAL_T"\n", i+1, ja[j]+1, a[j]);
        else
          fprintf(ostream, IND_T" "IND_T"\n", i+1, ja[j]+1);
      }
    }
  }

  ret = 0;

  CLEANUP:
  return ret;
}
