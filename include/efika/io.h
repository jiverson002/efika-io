/* SPDX-License-Identifier: MIT */
#ifndef EFIKA_IO_H
#define EFIKA_IO_H 1

#include "efika/core.h"

/*----------------------------------------------------------------------------*/
/*! Public API. */
/*----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
int EFIKA_IO_cluto_load (char const*, EFIKA_Matrix*);
int EFIKA_IO_cluto_save (char const*, EFIKA_Matrix const*);
int EFIKA_IO_dimacs_save(char const*, EFIKA_Matrix const*);
int EFIKA_IO_metis_load (char const*, EFIKA_Matrix*);
int EFIKA_IO_metis_save (char const*, EFIKA_Matrix const*);
int EFIKA_IO_mm_load    (char const*, EFIKA_Matrix*);
int EFIKA_IO_mm_save    (char const*, EFIKA_Matrix const*);
int EFIKA_IO_snap_load  (char const*, EFIKA_Matrix*);
int EFIKA_IO_snap_save  (char const*, EFIKA_Matrix const*);
#ifdef __cplusplus
}
#endif

#endif /* EFIKA_IO_H */
