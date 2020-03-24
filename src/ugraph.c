/* SPDX-License-Identifier: MIT */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>

#include "../common.h"
#include "../insist.h"

/*----------------------------------------------------------------------------*/
/*! Function to read a ugraph file. */
/*----------------------------------------------------------------------------*/
extern int
IO_ugraph_load(FILE * const istream, Matrix * const m)
{
  int ret=-1, fmt=0;
  size_t n=0;
  ind_t i, j, v, tmp;
  ind_t nr, nnz, nnnz, ncon=0;
  char * tok, * line=NULL;
  ind_t * tia=NULL, * ia=NULL, * ja=NULL, * vsiz=NULL;
  val_t * a=NULL, * vwgt=NULL;

  insist(NULL != istream);
  insist(NULL != m);

  do {
    insist(0 < getline(&line, &n, istream));
  } while ('%' == line[0]);

  switch (sscanf(line, IND_T" "IND_T" %d "IND_T"\n", &nr, &nnz, &fmt, &ncon))
  {
    case 4:
    /* validate */
    if (0 == ncon) {
      //gkl_pwarn("ncon cannot equal 0\n");
      goto CLEANUP;
    }
    if ( 10 != fmt &&  11 != fmt && 110 != fmt && 111 != fmt) {
      //gkl_pwarn("invalid format value (%d) with non-zero ncon\n", fmt);
      goto CLEANUP;
    }
    /* fall through */

    case 3:
    /* validate */
    if (  0 != fmt &&   1 != fmt &&  10 != fmt &&  11 != fmt && 100 != fmt &&
        101 != fmt && 110 != fmt && 111 != fmt) {
      //gkl_pwarn("invalid format value (%d)\n", fmt);
      goto CLEANUP;
    }
    /* fall through */

    case 2:
    nnz *= 2;
    break;

    default:
    //gkl_pwarn("too many entries on header line\n");
    goto CLEANUP;
  }

  insist(NULL != (tia=(ind_t *) calloc((nr+1), sizeof(ind_t))));
  insist(NULL != (ia=(ind_t *) malloc((nr+1)*sizeof(ind_t))));
  insist(NULL != (ja=(ind_t *) malloc(nnz*sizeof(ind_t))));
  if (has_adjwgt(fmt))
    insist(NULL != (a=(val_t *) malloc(nnz*sizeof(val_t))));
  if (has_vtxwgt(fmt))
    insist(NULL != (vwgt=(val_t *) malloc(ncon*nr*sizeof(val_t))));
  if (has_vtxsiz(fmt))
    insist(NULL != (vsiz=(ind_t *) malloc(nr*sizeof(ind_t))));

  //gkl_print("   nr="IND_T", nc="IND_T", nnz="IND_T"\n", nr, nr, nnz);
  //gkl_print("   fmt=%d, symm=%d, sort=%d\n\n", fmt, 1, 0);

  for (nnnz=0,i=0; i<nr; ++i) {
    insist(0 < getline(&line, &n, istream));

    if ('%' == line[0]) {
      i--;
      continue;
    }

    tok = strtok(line, " \t\n");
    if (has_vtxsiz(fmt)) {
      insist(NULL != tok);

      vsiz[i] = strtoi(tok, NULL);
      tok = strtok(NULL, " \t\n");
    }

    if (has_vtxwgt(fmt)) {
      for (j=0; j<ncon; ++j) {
        insist(NULL != tok);

        vwgt[i*ncon+j] = strtov(tok, NULL);
        tok = strtok(NULL, " \t\n");
      }
    }

    tmp = tia[i];
    tia[i] = nnnz;
    nnnz += tmp;

    while (NULL != tok) {
      v = strtoi(tok, NULL)-1;

      if (v < i) {
        //gkl_pwarn("non-zero specified from lower triangle\n");
        goto CLEANUP;
      }

      ja[nnnz] = v;
      tia[v]++;

      if (has_adjwgt(fmt)) {
        insist(NULL != tok);

        a[nnnz] = strtov(tok, NULL);
        tok = strtok(NULL, " \t\n");
      }

      tok = strtok(NULL, " \t\n");
      if (++nnnz == nnz && NULL != tok) {
        //gkl_pwarn("too many edges specified in file\n");
        goto CLEANUP;
      }
    }
  }
  tia[i] = nnnz;

  insist(nnnz == nnz);

  while (!feof(istream) && 0 < getline(&line, &n, istream))
    insist('%' == line[0]);

  memcpy(ia, tia, (nr+1)*sizeof(ind_t));

  for (i=0; i<nr; ++i) {
    for (j=tia[i]; j<tia[i+1]; ++j) {
      if (ja[j] > i) {
        if (has_adjwgt(fmt))
          a[tia[ja[j]]] = a[j];
        ja[tia[ja[j]]++] = i;
      }
    }
  }

  m->fmt   = fmt;
  /*m->diag  = 0;*/
  /*m->sort  = NONE;*/
  m->symm  = 1;
  m->nr    = nr;
  m->nc    = nr;
  m->nnz   = nnz;
  m->ia    = ia;
  m->ja    = ja;
  m->a     = a;
  m->ncon  = ncon;
  m->vsiz  = vsiz;
  m->vwgt  = vwgt;

  ret = 0;

  CLEANUP:
  if (-1 == ret) {
    free(ia);
    free(ja);
    free(a);
    free(vsiz);
    free(vwgt);
  }
  free(line);
  free(tia);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*! Function to write a ugraph file. */
/*----------------------------------------------------------------------------*/
extern int
IO_ugraph_save(FILE * const ostream, Matrix const * const m)
{
  int fmt, ret=-1;
  ind_t i, j;
  ind_t nr, nnz, ncon;
  ind_t const * ia, * ja, * vsiz;
  val_t const * a, * vwgt;

  insist(NULL != ostream);
  insist(NULL != m);

  if (1 != m->symm)
    return -1;

  fmt  = m->fmt;
  nr   = m->nr;
  nnz  = m->nnz;
  ncon = m->ncon;
  ia   = m->ia;
  ja   = m->ja;
  a    = m->a;
  vwgt = m->vwgt;
  vsiz = m->vsiz;

  fprintf(ostream, IND_T" "IND_T, nr, nnz/2);
  if (fmt > 0)
    fprintf(ostream, " %03d "IND_T, fmt, ncon);
  fprintf(ostream, "\n");

  for (i=0; i<nr; ++i) {
    if (has_vtxsiz(fmt))
      fprintf(ostream, IND_T" ", vsiz[i]);

    if (has_vtxwgt(fmt))
      for (j=0; j<ncon; ++j)
        fprintf(ostream, VAL_T" ", vwgt[i*ncon+j]);

    for (j=ia[i]; j<ia[i+1]; ++j) {
      if (i < ja[j]) {
        fprintf(ostream, IND_T" ", ja[j]+1);
        if (has_adjwgt(fmt))
          fprintf(ostream, VAL_T" ", a[j]);
      }
    }
    fprintf(ostream, "\n");
  }

  ret = 0;

  CLEANUP:
  return ret;
}
