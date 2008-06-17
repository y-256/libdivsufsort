/*
 * divsufsort.c for libdivsufsort
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


/*- Private Functions -*/

/* Sorts suffixes of type B*. */
static
saidx_t
_sort_typeBstar(const sauchar_t *T, saidx_t *SA,
                saidx_t *bucket_A, saidx_t *bucket_B,
                saidx_t n) {
  saidx_t *PAb, *ISAb, *buf;
  saidx_t i, j, k, t, m, bufsize;
  saint_t c0, c1;

  /* Initialize bucket arrays. */
  for(i = 0; i < BUCKET_A_SIZE; ++i) { bucket_A[i] = 0; }
  for(i = 0; i < BUCKET_B_SIZE; ++i) { bucket_B[i] = 0; }

  /* Count the number of occurrences of the first one or two characters of each
     type A, B and B* suffix. Moreover, store the beginning position of all
     type B* suffixes into the array SA. */
  for(i = n - 1, m = n; 0 <= i;) {
    /* type A suffix. */
    do { ++BUCKET_A(T[i]); } while((0 <= --i) && (T[i] >= T[i + 1]));
    if(0 <= i) {
      /* type B* suffix. */
      ++BUCKET_BSTAR(T[i], T[i + 1]);
      SA[--m] = i;
      /* type B suffix. */
      for(--i; (0 <= i) && (T[i] <= T[i + 1]); --i) {
        ++BUCKET_B(T[i], T[i + 1]);
      }
    }
  }
  m = n - m;
/*
note:
  A type B* suffix is lexicographically smaller than a type B suffix that
  begins with the same first two characters.
*/

  /* Calculate the index of start/end point of each bucket. */
  for(c0 = 0, i = 0, j = 0; c0 < ALPHABET_SIZE; ++c0) {
    t = i + BUCKET_A(c0);
    BUCKET_A(c0) = i + j; /* start point */
    i = t + BUCKET_B(c0, c0);
    for(c1 = c0 + 1; c1 < ALPHABET_SIZE; ++c1) {
      j += BUCKET_BSTAR(c0, c1);
      BUCKET_BSTAR(c0, c1) = j; /* end point */
      i += BUCKET_B(c0, c1);
    }
  }

  if(0 < m) {
    /* Sort the type B* suffixes by their first two characters. */
    PAb = SA + n - m; ISAb = SA + m;
    PAb[m] = n - 2; /* for sentinel. */
    for(i = m - 2; 0 <= i; --i) {
      t = PAb[i], c0 = T[t], c1 = T[t + 1];
      SA[--BUCKET_BSTAR(c0, c1)] = i;
    }
    t = PAb[m - 1], c0 = T[t], c1 = T[t + 1];
    SA[--BUCKET_BSTAR(c0, c1)] = m - 1;

    /* Sort the type B* substrings using sssort. */
    buf = SA + m, bufsize = n - (2 * m);
    if(bufsize <= LOCALMERGE_BUFFERSIZE) {
      if((buf = malloc(LOCALMERGE_BUFFERSIZE * sizeof(saidx_t))) == NULL) {
        return -1;
      }
      bufsize = LOCALMERGE_BUFFERSIZE;
    }
    for(c0 = ALPHABET_SIZE - 1, j = m; 0 < j; --c0) {
      for(c1 = ALPHABET_SIZE - 1; c0 < c1; j = i, --c1) {
        i = BUCKET_BSTAR(c0, c1);
        if(1 < (j - i)) {
          substringsort(T, PAb, SA + i, SA + j,
                        buf, bufsize, 2, *(SA + i) == (m - 1));
        }
      }
    }
    if(bufsize == LOCALMERGE_BUFFERSIZE) { free(buf); }

    /* Compute ranks of type B* substrings. */
    for(i = m - 1; 0 <= i; --i) {
      if(0 <= SA[i]) {
        j = i;
        do { ISAb[SA[i]] = i; } while((0 <= --i) && (0 <= SA[i]));
        SA[i + 1] = i - j;
        if(i <= 0) { break; }
      }
      j = i;
      do { ISAb[SA[i] = ~SA[i]] = j; } while(SA[--i] < 0);
      ISAb[SA[i]] = j;
    }

    /* Construct the inverse suffix array of type B* suffixes using trsort. */
    trsort(ISAb, SA, m, 1);

    /* Set the sorted order of tyoe B* suffixes. */
    for(i = n - 1, j = m; 0 <= i;) {
      for(--i; (0 <= i) && (T[i] >= T[i + 1]); --i) { }
      if(0 <= i) {
        SA[ISAb[--j]] = i;
        for(--i; (0 <= i) && (T[i] <= T[i + 1]); --i) { }
      }
    }

    /* Calculate the index of start/end point of each bucket. */
    for(c0 = ALPHABET_SIZE - 1, i = n, k = m - 1; 0 <= c0; --c0) {
      for(c1 = ALPHABET_SIZE - 1; c0 < c1; --c1) {
        t = i - BUCKET_B(c0, c1);
        BUCKET_B(c0, c1) = i + 1; /* end point */

        /* Move all type B* suffixes to the correct position. */
        for(i = t, j = BUCKET_BSTAR(c0, c1);
            j <= k;
            --i, --k) { SA[i] = SA[k]; }
      }
      t = i - BUCKET_B(c0, c0);
      BUCKET_B(c0, c0) = i + 1; /* end point */
      if(c0 < (ALPHABET_SIZE - 1)) {
        BUCKET_BSTAR(c0, c0 + 1) = t + 1; /* start point */
      }
      i = BUCKET_A(c0);
    }
  }

  return m;
}

/* Constructs the suffix array by using the sorted order of type B* suffixes. */
static
void
_construct_SA(const sauchar_t *T, saidx_t *SA,
              saidx_t *bucket_A, saidx_t *bucket_B,
              saidx_t n, saidx_t m) {
  saidx_t *i, *j, *t;
  saidx_t s;
  saint_t c0, c1, c2;

  /** An implementation version of MSufSort3's second stage. **/

  if(0 < m) {
    /* Construct the sorted order of type B suffixes by using
       the sorted order of type B* suffixes. */
    for(c1 = ALPHABET_SIZE - 2; 0 <= c1; --c1) {
      /* Scan the suffix array from right to left. */
      for(i = SA + BUCKET_BSTAR(c1, c1 + 1),
          j = SA + BUCKET_A(c1 + 1), t = NULL, c2 = -1;
          i <= j;
          --j) {
        if(0 <= (s = *j)) {
          if((0 <= --s) && ((c0 = T[s]) <= c1)) {
            *j = ~(s + 1);
            if((0 < s) && (T[s - 1] > c0)) { s = ~s; }
            if(c2 == c0) { *--t = s; }
            else {
              if(0 <= c2) { BUCKET_B(c2, c1) = t - SA; }
              *(t = SA + BUCKET_B(c2 = c0, c1) - 1) = s;
            }
          }
        } else {
          *j = ~s;
        }
      }
    }
  }

  /* Construct the suffix array by using
     the sorted order of type B suffixes. */
  SA[0] = n;
  *(t = SA + BUCKET_A(c2 = T[n - 1]) + 1) = n - 1;
  /* Scan the suffix array from left to right. */
  for(i = SA + 1, j = SA + n; i <= j; ++i) {
    if(0 <= (s = *i)) {
      if((0 <= --s) && ((c0 = T[s]) >= T[s + 1])) {
        if((0 < s) && (T[s - 1] < c0)) { s = ~s; }
        if(c0 == c2) { *++t = s; }
        else {
          BUCKET_A(c2) = t - SA;
          *(t = SA + BUCKET_A(c2 = c0) + 1) = s;
        }
      }
    } else {
      *i = ~s;
    }
  }
}

/* Constructs the burrows-wheeler transformed string directly
   by using the sorted order of type B* suffixes. */
static
saidx_t
_construct_BWT(const sauchar_t *T, saidx_t *SA,
               saidx_t *bucket_A, saidx_t *bucket_B,
               saidx_t n, saidx_t m) {
  saidx_t *i, *j, *t, *orig;
  saidx_t s;
  saint_t c0, c1, c2;

  /** An implementation version of MSufSort3's semidirect BWT. **/

  if(0 < m) {
    /* Construct the sorted order of type B suffixes by using
       the sorted order of type B* suffixes. */
    for(c1 = ALPHABET_SIZE - 2; 0 <= c1; --c1) {
      /* Scan the suffix array from right to left. */
      for(i = SA + BUCKET_BSTAR(c1, c1 + 1),
          j = SA + BUCKET_A(c1 + 1), t = NULL, c2 = -1;
          i <= j;
          --j) {
        if(0 <= (s = *j)) {
          if((0 <= --s) && ((c0 = T[s]) <= c1)) {
            *j = ~((saidx_t)c0);
            if((0 < s) && (T[s - 1] > c0)) { s = ~s; }
            if(c0 == c2) { *--t = s; }
            else {
              if(0 <= c2) { BUCKET_B(c2, c1) = t - SA; }
              *(t = SA + BUCKET_B(c2 = c0, c1) - 1) = s;
            }
          }
        } else {
          *j = ~s;
        }
      }
    }
  }

  /* Construct the BWTed string by using
     the sorted order of type B suffixes. */
  c0 = T[s = n - 1];
  SA[0] = c0;
  if(T[s - 1] < c0) { s = ~((saidx_t)T[s - 1]); }
  *(t = SA + BUCKET_A(c2 = c0) + 1) = s;
  /* Scan the suffix array from left to right. */
  for(i = SA + 1, j = SA + n, orig = SA - 1; i <= j; ++i) {
    if(0 <= (s = *i)) {
      if((0 <= --s) && ((c0 = T[s]) >= T[s + 1])) {
        *i = c0;
        if((0 < s) && (T[s - 1] < c0)) { s = ~((saidx_t)T[s - 1]); }
        if(c0 == c2) { *++t = s; }
        else {
          BUCKET_A(c2) = t - SA;
          *(t = SA + BUCKET_A(c2 = c0) + 1) = s;
        }
      } else if(s < 0) { orig = i; }
    } else {
      *i = ~s;
    }
  }

  return orig - SA;
}


/*---------------------------------------------------------------------------*/

/*- Function -*/

saint_t
divsufsort(const sauchar_t *T, saidx_t *SA, saidx_t n) {
  saidx_t *bucket_A, *bucket_B;
  saidx_t m;
  saint_t err = 0;

  /* Check arguments. */
  if((T == NULL) || (SA == NULL) || (n < 0)) { return -1; }
  else if(n == 0) { SA[0] = 0; return 0; }
  else if(n == 1) { SA[0] = 1; SA[1] = 0; return 0; }

  bucket_A = malloc(BUCKET_A_SIZE * sizeof(saidx_t));
  bucket_B = malloc(BUCKET_B_SIZE * sizeof(saidx_t));

  /* Suffixsort. */
  if((bucket_A != NULL) && (bucket_B != NULL)) {
    m = _sort_typeBstar(T, SA, bucket_A, bucket_B, n);
    if(0 <= m) {
      _construct_SA(T, SA, bucket_A, bucket_B, n, m);
    } else {
      err = -3;
    }
  } else {
    err = -2;
  }

  free(bucket_B);
  free(bucket_A);

  return err;
}

saidx_t
divbwt(const sauchar_t *T, sauchar_t *U, saidx_t *A, saidx_t n) {
  saidx_t *B;
  saidx_t *bucket_A, *bucket_B;
  saidx_t m, pidx = -1, i;
  saint_t err = 0;

  /* Check arguments. */
  if((T == NULL) || (U == NULL) || (n < 0)) { return -1; }
  else if(n <= 1) { if(n == 1) { U[0] = T[0]; } return n; }

  if((B = A) == NULL) { B = malloc((n + 1) * sizeof(saidx_t)); }
  bucket_A = malloc(BUCKET_A_SIZE * sizeof(saidx_t));
  bucket_B = malloc(BUCKET_B_SIZE * sizeof(saidx_t));

  /* Burrows-Wheeler Transform. */
  if((B != NULL) && (bucket_A != NULL) && (bucket_B != NULL)) {
    m = _sort_typeBstar(T, B, bucket_A, bucket_B, n);
    if(0 <= m) {
      pidx = _construct_BWT(T, B, bucket_A, bucket_B, n, m);

      /* Copy to output string. */
      for(i = 0; i < pidx; ++i) { U[i] = B[i]; }
      for(; i < n; ++i) { U[i] = B[i + 1]; }
    } else {
      err = -3;
    }
  } else {
    err = -2;
  }

  free(bucket_B);
  free(bucket_A);
  if(A == NULL) { free(B); }

  return pidx;
}


const char *
divsufsort_version(void) {
  return PACKAGE_VERSION;
}
