/* SPDX-License-Identifier: MIT */
#ifndef EFIKA_IO_H
#define EFIKA_IO_H 1

#include <stdio.h>

#include "efika/core.h"

/*----------------------------------------------------------------------------*/
/*! Public API. */
/*----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
int EFIKA_IO_cluto_load (FILE*, EFIKA_Matrix*);
int EFIKA_IO_cluto_save (FILE*, EFIKA_Matrix const*);
int EFIKA_IO_dimacs_save(FILE*, EFIKA_Matrix const*);
int EFIKA_IO_metis_load (FILE*, EFIKA_Matrix*);
int EFIKA_IO_metis_save (FILE*, EFIKA_Matrix const*);
int EFIKA_IO_mm_load    (FILE*, EFIKA_Matrix*);
int EFIKA_IO_mm_save    (FILE*, EFIKA_Matrix const*);
int EFIKA_IO_snap_load  (FILE*, EFIKA_Matrix*);
int EFIKA_IO_snap_save  (FILE*, EFIKA_Matrix const*);
int EFIKA_IO_ugraph_load(FILE*, EFIKA_Matrix*);
int EFIKA_IO_ugraph_save(FILE*, EFIKA_Matrix const*);
#ifdef __cplusplus
}
#endif

#endif /* EFIKA_IO_H */
