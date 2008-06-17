/*
 * utils.c for libdivsufsort
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

#include "divsufsort_private.h"


/*- Private Function -*/

/* Binary search for inverse bwt. */
static
saidx_t
_binarysearch(const saidx_t *A, saidx_t len, saidx_t val) {
  saidx_t half, m;
  for(m = 0, half = len >> 1;
      0 < len;
      len = half, half >>= 1) {
    if(A[m + half] < val) {
      m += half + 1;
      half -= ((len & 1) == 0);
    }
  }
  return m;
}


/*- Functions -*/

/* Burrows-Wheeler transform. */
saint_t
bw_transform(const sauchar_t *T, sauchar_t *U, saidx_t *SA,
             saidx_t n, saidx_t *idx) {
  saidx_t *A, i, p;
  saint_t c;

  /* Check arguments. */
  if((T == NULL) || (U == NULL) || (n < 0) || (idx == NULL)) { return -1; }
  if(n <= 1) {
    *idx = n;
    return 0;
  }

  if((A = SA) == NULL) {
    i = divbwt(T, U, NULL, n);
    if(0 <= i) { *idx = i; i = 0; }
    return (saint_t)i;
  }

  /* BW transform. */
  if(T == U) {
    for(i = 0; 0 <= (p = A[i] - 1); ++i) {
      c = T[i];
      U[i] = (i <= p) ? T[p] : A[p];
      A[i] = c;
    }
    *idx = i;
    for( ; i < n; ++i) {
      p = A[i + 1] - 1;
      c = T[i];
      U[i] = (i <= p) ? T[p] : A[p];
      A[i] = c;
    }
  } else {
    for(i = 0; A[i] != 0; ++i) { U[i] = T[A[i] - 1]; }
    *idx = i;
    for( ; i < n; ++i)         { U[i] = T[A[i + 1] - 1]; }
  }

  if(SA == NULL) {
    /* Deallocate memory. */
    free(A);
  }

  return 0;
}

/* Inverse Burrows-Wheeler transform. */
saint_t
inverse_bw_transform(const sauchar_t *T, sauchar_t *U, saidx_t *A,
                     saidx_t n, saidx_t idx) {
  saidx_t C[ALPHABET_SIZE];
  saidx_t D[ALPHABET_SIZE];
  saidx_t *B;
  saidx_t i, k, t, sum;

  /* Check arguments. */
  if((T == NULL) || (U == NULL) || (n < 0) || (idx < 0) ||
     (n < idx) || ((0 < n) && (idx == 0))) {
    return -1;
  }
  if(n <= 1) { return 0; }

  if((B = A) == NULL) {
    /* Allocate (n+1)*sizeof(saidx_t) bytes of memory. */
    if((B = malloc((n + 1) * sizeof(saidx_t))) == NULL) { return -2; }
  }

  /* Inverse BW transform. */
  for(i = 0; i < ALPHABET_SIZE; ++i) { C[i] = 0; }
  for(i = 0; i < n; ++i) { ++C[T[i]]; }
  for(i = 0, sum = 0; i < ALPHABET_SIZE; ++i) {
    t = C[i];
    C[i] = sum;
    sum += t;
  }
  B[0] = idx;
  for(i = 0; i < idx; ++i) { B[++C[T[i]]] = i; }
  for( ; i < n; ++i)       { B[++C[T[i]]] = i + 1; }
  for(i = 0, k = 0, t = 0; i < ALPHABET_SIZE; ++i) {
    if(t != C[i]) {
      D[k] = i;
      t = C[k] = C[i];
      ++k;
    }
  }
  for(i = 0, t = 0; i < n; ++i) {
    U[i] = D[_binarysearch(C, k, t = B[t])];
  }

  if(A == NULL) {
    /* Deallocate memory. */
    free(B);
  }

  return 0;
}

/* Checks the suffix array SA of the string T. */
saint_t
sufcheck(const sauchar_t *T, const saidx_t *SA,
         saidx_t n, saint_t verbose) {
  saidx_t C[ALPHABET_SIZE];
  saidx_t i = 0, p, t = 0;
  saint_t c;
  saint_t err = 0;

  if(1 <= verbose) { fprintf(stderr, "sufchecker: "); }

  /* Check arguments. */
  if((T == NULL) || (SA == NULL) || (n < 0)) { err = -1; }

  /* ranges. */
  if(err == 0) {
    for(i = 0; i <= n; ++i) {
      if((SA[i] < 0) || (n < SA[i])) {
        err = -2;
        break;
      }
    }
  }

  /* first characters. */
  if(err == 0) {
    for(i = 1; i < n; ++i) {
      if(T[SA[i]] > T[SA[i + 1]]) {
        err = -3;
        break;
      }
    }
  }

  /* suffixes. */
  if(err == 0) {
    for(i = 0; i < ALPHABET_SIZE; ++i) { C[i] = 0; }
    for(i = 0; i < n; ++i) { ++C[T[i]]; }
    for(i = 0, p = 1; i < ALPHABET_SIZE; ++i) {
      t = C[i];
      C[i] = p;
      p += t;
    }

    for(i = 0; i <= n; ++i) {
      p = SA[i];
      if(0 < p) {
        c = T[--p];
        t = C[c];
      } else {
        p = n;
        c = -1;
        t = 0;
      }
      if(p != SA[t]) {
        err = -4;
        break;
      }
      if(0 <= c) {
        ++C[c];
        if((n < C[c]) || (T[SA[C[c]]] != c)) { C[c] = -1; }
      }
    }
  }

  if(1 <= verbose) {
    if(err == 0) {
      fprintf(stderr, "Done.\n");
    } else if(verbose == 1) {
      fprintf(stderr, "Error.\n");
    } else if(err == -1) {
      fprintf(stderr, "Invalid arguments.\n");
    } else if(err == -2) {
      fprintf(stderr, "Out of the range [0,%d].\n  SA[%d]=%d\n",
        (int)n, (int)i, (int)SA[i]);
    } else if(err == -3) {
      fprintf(stderr, "Suffixes in wrong order.\n"
                      "  T[SA[%d]=%d]=%d > T[SA[%d]=%d]=%d\n",
        (int)i, (int)SA[i], (int)T[SA[i]],
        (int)i + 1, (int)SA[i + 1], (int)T[SA[i + 1]]);
    } else if(err == -4) {
      fprintf(stderr, "Suffix in wrong position.\n");
      if(0 <= t) { fprintf(stderr, "  SA[%d]=%d or\n", (int)t, (int)SA[t]); }
      fprintf(stderr, "  SA[%d]=%d\n", (int)i, (int)SA[i]);
    }
  }

  return err;
}


static
int
_compare(const sauchar_t *T, saidx_t Tsize,
         const sauchar_t *P, saidx_t Psize,
         saidx_t suf, saidx_t *match) {
  saidx_t i, j;
  saint_t r;
  for(i = suf + *match, j = *match, r = 0;
      (i < Tsize) && (j < Psize) && ((r = T[i] - P[j]) == 0); ++i, ++j) { }
  *match = j;
  return (r == 0) ? -(j != Psize) : r;
}

/* Search for the pattern P in the string T. */
saidx_t
sa_search(const sauchar_t *T, saidx_t Tsize,
          const sauchar_t *P, saidx_t Psize,
          const saidx_t *SA, saidx_t SAsize,
          saidx_t *idx) {
  saidx_t size, lsize, rsize, half;
  saidx_t match, lmatch, rmatch;
  saidx_t llmatch, lrmatch, rlmatch, rrmatch;
  saidx_t i, j, k;
  saint_t r;

  if(idx != NULL) { *idx = -1; }
  if((T == NULL) || (P == NULL) || (SA == NULL) ||
     (Tsize < 0) || (Psize < 0) || (SAsize < 0)) { return -1; }
  if((Tsize == 0) || (SAsize == 0)) { return 0; }
  if(Psize == 0) { if(idx != NULL) { *idx = 0; } return SAsize; }

  for(i = j = k = 0, lmatch = rmatch = 0, size = SAsize, half = size >> 1;
      0 < size;
      size = half, half >>= 1) {
    match = MIN(lmatch, rmatch);
    r = _compare(T, Tsize, P, Psize, SA[i + half], &match);
    if(r < 0) {
      i += half + 1;
      half -= (size & 1) ^ 1;
      lmatch = match;
    } else if(r > 0) {
      rmatch = match;
    } else {
      lsize = half, j = i, rsize = size - half - 1, k = i + half + 1;

      /* left part */
      for(llmatch = lmatch, lrmatch = match, half = lsize >> 1;
          0 < lsize;
          lsize = half, half >>= 1) {
        lmatch = MIN(llmatch, lrmatch);
        r = _compare(T, Tsize, P, Psize, SA[j + half], &lmatch);
        if(r < 0) {
          j += half + 1;
          half -= (lsize & 1) ^ 1;
          llmatch = lmatch;
        } else {
          lrmatch = lmatch;
        }
      }

      /* right part */
      for(rlmatch = match, rrmatch = rmatch, half = rsize >> 1;
          0 < rsize;
          rsize = half, half >>= 1) {
        rmatch = MIN(rlmatch, rrmatch);
        r = _compare(T, Tsize, P, Psize, SA[k + half], &rmatch);
        if(r <= 0) {
          k += half + 1;
          half -= (rsize & 1) ^ 1;
          rlmatch = rmatch;
        } else {
          rrmatch = rmatch;
        }
      }

      break;
    }
  }

  if(idx != NULL) { *idx = (0 < (k - j)) ? j : i; }
  return k - j;
}

/* Search for the character c in the string T. */
saidx_t
sa_simplesearch(const sauchar_t *T, saidx_t Tsize,
                const saidx_t *SA, saidx_t SAsize,
                saint_t c, saidx_t *idx) {
  saidx_t size, lsize, rsize, half;
  saidx_t i, j, k, p;
  saint_t r;

  if(idx != NULL) { *idx = -1; }
  if((T == NULL) || (SA == NULL) || (Tsize < 0) || (SAsize < 0)) { return -1; }
  if((Tsize == 0) || (SAsize == 0)) { return 0; }

  for(i = j = k = 0, size = SAsize, half = size >> 1;
      0 < size;
      size = half, half >>= 1) {
    p = SA[i + half];
    r = (p < Tsize) ? T[p] - c : -1;
    if(r < 0) {
      i += half + 1;
      half -= (size & 1) ^ 1;
    } else if(r == 0) {
      lsize = half, j = i, rsize = size - half - 1, k = i + half + 1;

      /* left part */
      for(half = lsize >> 1;
          0 < lsize;
          lsize = half, half >>= 1) {
        p = SA[j + half];
        r = (p < Tsize) ? T[p] - c : -1;
        if(r < 0) {
          j += half + 1;
          half -= (lsize & 1) ^ 1;
        }
      }

      /* right part */
      for(half = rsize >> 1;
          0 < rsize;
          rsize = half, half >>= 1) {
        p = SA[k + half];
        r = (p < Tsize) ? T[p] - c : -1;
        if(r <= 0) {
          k += half + 1;
          half -= (rsize & 1) ^ 1;
        }
      }

      break;
    }
  }

  if(idx != NULL) { *idx = (0 < (k - j)) ? j : i; }
  return k - j;
}
