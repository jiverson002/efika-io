/* SPDX-License-Identifier: MIT */
#ifndef EFIKA_IO_GETLINE_H
#define EFIKA_IO_GETLINE_H 1

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/
/*! IO routines. */
/*----------------------------------------------------------------------------*/
#define IO_getline efika_IO_getline

/*----------------------------------------------------------------------------*/
/*! Private API. */
/*----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

intmax_t IO_getline(char **lineptr, size_t *n, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif /* EFIKA_CORE_BLAS_H */
