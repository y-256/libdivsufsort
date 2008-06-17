/*
 * mksary.c for libdivsufsort
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
#include <time.h>
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif


int
main(int argc, const char *argv[]) {
  FILE *ifp, *ofp;
  sauchar_t *T;
  saidx_t *SA;
  saidx_t n;
  clock_t start, finish;
#if HAVE_SYS_STAT_H
  struct stat s;
#endif

  /* Check argument. */
  if(argc != 3) {
    fprintf(stderr,
      "mksary, a simple suffix array builder, version %s.\n"
      , divsufsort_version());
    fprintf(stderr,
      "usage: %s srcFILE dstSA\n\n"
      , argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Get a file's status information. */
#if HAVE_SYS_STAT_H
  if(stat(argv[1], &s) != 0) {
    fprintf(stderr, "%s: Cannot stat file `%s': ", argv[0], argv[1]);
    perror(NULL);
    exit(EXIT_FAILURE);
  }
  n = s.st_size;
#endif

  /* Open a file for reading. */
  if((ifp = fopen(argv[1], "rb")) == NULL) {
    fprintf(stderr, "%s: Cannot open file `%s': ", argv[0], argv[1]);
    perror(NULL);
    exit(EXIT_FAILURE);
  }

  /* Open a file for writing. */
  if((ofp = fopen(argv[2], "wb")) == NULL) {
    fprintf(stderr, "%s: Cannot open file `%s': ", argv[0], argv[2]);
    perror(NULL);
    exit(EXIT_FAILURE);
  }

#if !HAVE_SYS_STAT_H
  fseek(ifp, 0, SEEK_END);
  n = ftell(ifp);
  rewind(ifp);
#endif

  /* Allocate n+4(n+1) bytes of memory. */
  if(((T = malloc(n * sizeof(sauchar_t))) == NULL) ||
     ((SA = malloc((n + 1) * sizeof(saidx_t))) == NULL)) {
    fprintf(stderr, "%s: Cannot allocate memory.\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Read n bytes of data. */
  if(fread(T, sizeof(sauchar_t), n, ifp) != n) {
    fprintf(stderr, "%s: %s `%s': ",
      argv[0],
      (ferror(ifp) || !feof(ifp)) ? "Cannot read from" : "Unexpected EOF in",
      argv[1]);
    perror(NULL);
    exit(EXIT_FAILURE);
  }
  fclose(ifp);

  /* Construct the suffix array. */
  fprintf(stderr, "%s: %d bytes ... ", argv[1], (int)n);
  start = clock();
  divsufsort(T, SA, n);
  finish = clock();
  fprintf(stderr, "%.4f sec\n",
    (double)(finish - start) / (double)CLOCKS_PER_SEC);

  /* Write the suffix array. */
  if(fwrite(SA, sizeof(saidx_t), n + 1, ofp) != (n + 1)) {
    fprintf(stderr, "%s: Cannot write to `%s': ", argv[0], argv[2]);
    perror(NULL);
    exit(EXIT_FAILURE);
  }
  fclose(ofp);

  /* Deallocate memory. */
  free(SA);
  free(T);

  return 0;
}
