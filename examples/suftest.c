/*
 * suftest.c for libdivsufsort
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
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif


int
main(int argc, const char *argv[]) {
  FILE *fp;
  sauchar_t *T;
  saidx_t *SA;
  saidx_t n;
  clock_t start, finish;
#if HAVE_SYS_STAT_H
  struct stat s;
#endif

  /* Check argument. */
  if((argc != 2) ||
     (strcmp(argv[1], "-h") == 0) ||
     (strcmp(argv[1], "--help") == 0)) {
    fprintf(stderr,
      "suftest, a suffixsort tester, version %s.\n"
      , divsufsort_version());
    fprintf(stderr,
      "usage: %s FILE\n\n"
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
  if((fp = fopen(argv[1], "rb")) == NULL) {
    fprintf(stderr, "%s: Cannot open file `%s': ", argv[0], argv[1]);
    perror(NULL);
    exit(EXIT_FAILURE);
  }

#if !HAVE_SYS_STAT_H
  fseek(fp, 0, SEEK_END);
  n = ftell(fp);
  rewind(fp);
#endif

  /* Allocate n+4(n+1) bytes of memory. */
  if(((T = malloc(n * sizeof(sauchar_t))) == NULL) ||
     ((SA = malloc((n + 1) * sizeof(saidx_t))) == NULL)) {
    fprintf(stderr, "%s: Cannot allocate memory.\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Read n bytes of data. */
  if(fread(T, sizeof(sauchar_t), n, fp) != n) {
    fprintf(stderr, "%s: %s `%s': ",
      argv[0],
      (ferror(fp) || !feof(fp)) ? "Cannot read from" : "Unexpected EOF in",
      argv[1]);
    perror(NULL);
    exit(EXIT_FAILURE);
  }
  fclose(fp);

  /* Construct the suffix array. */
  fprintf(stderr, "%s: %d bytes ... ", argv[1], (int)n);
  start = clock();
  divsufsort(T, SA, n);
  finish = clock();
  fprintf(stderr, "%.4f sec\n",
    (double)(finish - start) / (double)CLOCKS_PER_SEC);

  /* Check the suffix array. */
  if(sufcheck(T, SA, n, 3) != 0) {
    exit(EXIT_FAILURE);
  }

  /* Deallocate memory. */
  free(SA);
  free(T);

  return 0;
}
