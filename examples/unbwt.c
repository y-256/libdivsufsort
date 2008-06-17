/*
 * unbwt.c for libdivsufsort
 * Copyright (c) 2003-2008 Yuta Mori All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <divsufsort.h>
#include <stdio.h>
#if HAVE_STDLIB_H
# include <stdlib.h>
#endif
#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#include <time.h>


int
main(int argc, const char *argv[]) {
  sauchar_t *T;
  saidx_t *A, m, n, blocksize, idx;
  saint_t err;
  clock_t start, finish;

  /* Check argument. */
  if(argc != 1) {
    fprintf(stderr,
      "unbwt, an inverse burrows-wheeler transform program, version %s.\n"
      , divsufsort_version());
    fprintf(stderr,
      "usage: %s < STDIN > STDOUT\n\n"
      , argv[0]);
    return 0;
  }

  /* Read the blocksize from stdin. */
  if(fread(&blocksize, sizeof(saidx_t), 1, stdin) != 1) {
    fprintf(stderr, "%s: %s `stdin': ",
      argv[0],
      (ferror(stdin) || !feof(stdin)) ?
        "Cannot read from" : "Unexpected EOF in");
    perror(NULL);
    exit(EXIT_FAILURE);
  }

  /* Allocate blocksize+4(blocksize+1) bytes of memory. */
  if(((T = malloc(blocksize * sizeof(sauchar_t))) == NULL) ||
     ((A = malloc((blocksize + 1) * sizeof(saidx_t))) == NULL)) {
    fprintf(stderr, "%s: Cannot allocate memory.\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "UnBWT (blocksize %d) ... ", (int)blocksize);
  start = clock();
  for(n = 0; fread(&idx,sizeof(saidx_t),1,stdin)!=0; n += m) {
    /* Read blocksize bytes of data. */
    if((m = fread(T, sizeof(sauchar_t), blocksize, stdin)) == 0) {
      fprintf(stderr, "%s: Unexpected EOF in `stdin': ", argv[0]);
      perror(NULL);
      exit(EXIT_FAILURE);
    }

    /* Inverse Burrows-Wheeler Transform. */
    if((err = inverse_bw_transform(T, T, A, m, idx)) != 0) {
      fprintf(stderr, "%s (inverse_bw_transform): %s.\n",
        argv[0],
        (err == -1) ? "Invalid arguments" : "Cannot allocate memory");
      exit(EXIT_FAILURE);
    }

    /* Write m bytes of data. */
    if(fwrite(T, sizeof(sauchar_t), m, stdout) != m) {
      fprintf(stderr, "%s: Cannot write to `stdout': ", argv[0]);
      perror(NULL);
      exit(EXIT_FAILURE);
    }
  }
  if(ferror(stdin)) {
    fprintf(stderr, "%s: Cannot read from `stdin': ", argv[0]);
    perror(NULL);
    exit(EXIT_FAILURE);
  }
  finish = clock();
  fprintf(stderr, "%d bytes: %.4f sec\n",
    (int)n, (double)(finish - start) / (double)CLOCKS_PER_SEC);

  /* Deallocate memory. */
  free(T);
  free(A);

  return 0;
}
