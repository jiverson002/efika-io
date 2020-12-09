/* The original code is public domain -- Will Hartung 4/9/09
 * Modifications, public domain as well, by Antti Haapala, 11/10/17
 * - Switched to getc on 5/23/19
 * Modifications, public domain as well, by Jeremy Iverson 12/09/20 */

#ifdef HAVE_GETLINE
# define _POSIX_C_SOURCE 200809L
# include <stdio.h>
# include <stdint.h>

# include "efika/io/getline.h"

intmax_t IO_getline(char **lineptr, size_t *n, FILE *stream) {
  return (intmax_t)getline(lineptr, n, stream);
}
#else
# include <errno.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdint.h>

# include "efika/io/getline.h"

intmax_t IO_getline(char **lineptr, size_t *n, FILE *stream) {
  intmax_t pos;
  int c;

  if (NULL == lineptr || NULL == stream || NULL == n) {
    errno = EINVAL;
    return -1;
  }

  _lock_file(stream);

  c = _getc_nolock(stream);
  if (EOF == c) {
    pos = -1;
    goto CLEANUP;
  }

  if (NULL == *lineptr) {
    *lineptr = malloc(128);
    if (NULL == *lineptr) {
      pos = -1;
      goto CLEANUP;
    }
    *n = 128;
  }

  pos = 0;
  while(c != EOF) {
    if (pos + 1 >= *n) {
      size_t new_size = *n + (*n >> 2);
      if (new_size < 128) {
        new_size = 128;
      }
      char *new_ptr = realloc(*lineptr, new_size);
      if (NULL == new_ptr) {
        pos = -1;
        goto CLEANUP;
      }
      *n = new_size;
      *lineptr = new_ptr;
    }

    (*lineptr)[pos++] = c;
    if ('\n' == c) {
      break;
    }

    c = _getc_nolock(stream);
  }

  (*lineptr)[pos] = '\0';

CLEANUP:
  _unlock_file(stream);

  return pos;
}
#endif
