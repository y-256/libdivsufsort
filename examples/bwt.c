/*
 * bwt.c for libdivsufsort
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


static
saidx_t
_str2size(const char *str) {
  saidx_t s[3];
  saidx_t t;
  int i, c;
  for(i = 0, t = s[0] = s[1] = s[2] = 0; (c = str[i]) != '\0'; ++i) {
    if(('0' <= c) && (c <= '9')) {
      t = (t * 10) + (c - '0');
    } else {
      switch(c) {
      case 'm':
      case 'M':
        s[0] += t << 20;
        break;
      case 'k':
      case 'K':
        s[1] += t << 10;
        break;
      case 'b':
      case 'B':
        s[2] += t;
        break;
      }
      t = 0;
    }
  }
  return s[0] + s[1] + s[2];
}

int
main(int argc, const char *argv[]) {
  sauchar_t *T;
  saidx_t *SA;
  saidx_t m, n, blocksize, idx;
  clock_t start,finish;

  /* Check argument. */
  if(((argc != 1) && (argc != 2)) ||
     ((argc == 2) && (strcmp(argv[1], "-h") == 0)) ||
     ((argc == 2) && (strcmp(argv[1], "--help") == 0))) {
    fprintf(stderr,
      "bwt, a burrows-wheeler transform program, version %s.\n"
      , divsufsort_version());
    fprintf(stderr,
      "usage: %s [BLOCKSIZE] < STDIN > STDOUT\n\n"
      , argv[0]);
    exit(EXIT_FAILURE);
  }
  blocksize = (argc == 2) ? _str2size(argv[1]) : 0;
  if(blocksize <= 0) {
    fseek(stdin, 0, SEEK_END);
    blocksize = ftell(stdin);
    if(blocksize < 0) { blocksize = BUFSIZ; }
    rewind(stdin);
  }

  /* Allocate 5blocksize bytes of memory. */
  if(((T = malloc(blocksize * sizeof(sauchar_t))) == NULL) ||
     ((SA = malloc(blocksize * sizeof(saidx_t))) == NULL)) {
    fprintf(stderr, "%s: Cannot allocate memory.\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Write the blocksize. */
  if(fwrite(&blocksize, sizeof(saidx_t), 1, stdout) != 1) {
    fprintf(stderr, "%s: Cannot write to `stdout': ", argv[0]);
    perror(NULL);
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "  BWT (blocksize %d) ... ", (int)blocksize);
  start=clock();
  for(n = 0; 0 < (m = fread(T, sizeof(sauchar_t), blocksize, stdin)); n += m) {
    /* Burrows-Wheeler Transform. */
    idx = divbwt(T, T, SA, m);
    if(idx < 0) {
      fprintf(stderr, "%s (bw_transform): %s.\n",
        argv[0],
        (idx == -1) ? "Invalid arguments" : "Cannot allocate memory");
      exit(EXIT_FAILURE);
    }

    /* Write the bwted data. */
    if((fwrite(&idx, sizeof(saidx_t), 1, stdout) != 1) ||
       (fwrite(T, sizeof(sauchar_t), m, stdout) != m)) {
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
  free(SA);

  return 0;
}
