/*
 * substringsort.c for libdivsufsort
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

/* Compares two suffixes. */
static INLINE
saint_t
_compare(const sauchar_t *T,
         const saidx_t *p1, const saidx_t *p2,
         saidx_t depth) {
  const sauchar_t *U1, *U2, *U1n, *U2n;

  for(U1 = T + depth + *p1,
      U2 = T + depth + *p2,
      U1n = T + *(p1 + 1) + 2,
      U2n = T + *(p2 + 1) + 2;
      (U1 < U1n) && (U2 < U2n) && (*U1 == *U2);
      ++U1, ++U2) {
  }

  return U1 < U1n ?
        (U2 < U2n ? *U1 - *U2 : 1) :
        (U2 < U2n ? -1 : 0);
}


/*---------------------------------------------------------------------------*/

/* Insertionsort for small size groups */
static
void
_insertionsort(const sauchar_t *T, const saidx_t *PA,
               saidx_t *first, saidx_t *last, saidx_t depth) {
  saidx_t *i, *j;
  saidx_t t;
  saint_t r;

  for(i = last - 2; first <= i; --i) {
    for(t = *i, j = i + 1; 0 < (r = _compare(T, PA + t, PA + *j, depth));) {
      do { *(j - 1) = *j; } while((++j < last) && (*j < 0));
      if(last <= j) { break; }
    }
    if(r == 0) { *j = ~*j; }
    *(j - 1) = t;
  }
}


/*---------------------------------------------------------------------------*/

static INLINE
void
_fixdown(const sauchar_t *Td, const saidx_t *PA,
         saidx_t *SA, saidx_t i, saidx_t size) {
  saidx_t j, k;
  saidx_t v;
  saint_t c, d, e;

  for(v = SA[i], c = Td[PA[v]]; (j = 2 * i + 1) < size; SA[i] = SA[k], i = k) {
    d = Td[PA[SA[k = j++]]];
    if(d < (e = Td[PA[SA[j]]])) { k = j; d = e; }
    if(d <= c) { break; }
  }
  SA[i] = v;
}

/* Simple top-down heapsort. */
static
void
_heapsort(const sauchar_t *Td, const saidx_t *PA, saidx_t *SA, saidx_t size) {
  saidx_t i, m;
  saidx_t t;

  m = size;
  if((size % 2) == 0) {
    m--;
    if(Td[PA[SA[m / 2]]] < Td[PA[SA[m]]]) { SWAP(SA[m], SA[m / 2]); }
  }

  for(i = m / 2 - 1; 0 <= i; --i) { _fixdown(Td, PA, SA, i, m); }

  if((size % 2) == 0) {
    SWAP(SA[0], SA[m]);
    _fixdown(Td, PA, SA, 0, m);
  }

  for(i = m - 1; 0 < i; --i) {
    t = SA[0];
    SA[0] = SA[i];
    _fixdown(Td, PA, SA, 0, i);
    SA[i] = t;
  }
}


/*---------------------------------------------------------------------------*/

/* Returns the median of three elements. */
static INLINE
saidx_t *
_median3(const sauchar_t *Td, const saidx_t *PA,
         saidx_t *v1, saidx_t *v2, saidx_t *v3) {
  saidx_t *t;
  if(Td[PA[*v1]] > Td[PA[*v2]]) { SWAP(v1, v2); }
  if(Td[PA[*v2]] > Td[PA[*v3]]) {
    if(Td[PA[*v1]] > Td[PA[*v3]]) { return v1; }
    else { return v3; }
  }
  return v2;
}

/* Returns the median of five elements. */
static INLINE
saidx_t *
_median5(const sauchar_t *Td, const saidx_t *PA,
         saidx_t *v1, saidx_t *v2, saidx_t *v3, saidx_t *v4, saidx_t *v5) {
  saidx_t *t;
  if(Td[PA[*v2]] > Td[PA[*v3]]) { SWAP(v2, v3); }
  if(Td[PA[*v4]] > Td[PA[*v5]]) { SWAP(v4, v5); }
  if(Td[PA[*v2]] > Td[PA[*v4]]) { SWAP(v2, v4); SWAP(v3, v5); }
  if(Td[PA[*v1]] > Td[PA[*v3]]) { SWAP(v1, v3); }
  if(Td[PA[*v1]] > Td[PA[*v4]]) { SWAP(v1, v4); SWAP(v3, v5); }
  if(Td[PA[*v3]] > Td[PA[*v4]]) { return v4; }
  return v3;
}

/* Returns the pivot element. */
static INLINE
saidx_t *
_pivot(const sauchar_t *Td, const saidx_t *PA, saidx_t *first, saidx_t *last) {
  saidx_t *middle;
  saidx_t t;

  t = last - first;
  middle = first + t / 2;

  if(t <= 512) {
    if(t <= 32) {
      return _median3(Td, PA, first, middle, last - 1);
    } else {
      t >>= 2;
      return _median5(Td, PA,
                      first, first + t, middle,
                      last - 1 - t, last - 1);
    }
  }
  t >>= 3;
  return _median3(Td, PA,
           _median3(Td, PA, first, first + t, first + (t << 1)),
           _median3(Td, PA, middle - t, middle, middle + t),
           _median3(Td, PA, last - 1 - (t << 1), last - 1 - t, last - 1));
}


/*---------------------------------------------------------------------------*/

static INLINE
saidx_t
_lg(saidx_t n) {
static const int log2table[256]= {
 -1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};
  /* for 16 bits */
  return ((n & 0xff00) != 0) ?
          8 + log2table[(n >> 8) & 0xff] :
          log2table[n & 0xff];

  /* for 32 bits */
/*
  return (n & 0xffff0000) ?
          ((n & 0xff000000) ?
            24 + log2table[(n >> 24) & 0xff] :
            16 + log2table[(n >> 16) & 0xff]) :
          ((n & 0x0000ff00) ?
             8 + log2table[(n >>  8) & 0xff] :
             0 + log2table[(n >>  0) & 0xff]);
*/
}

/* Binary partition for substrings. */
static INLINE
saidx_t *
_substring_partition(const saidx_t *PA,
                     saidx_t *first, saidx_t *last, saidx_t depth) {
  saidx_t *a, *b;
  saidx_t t;
  for(a = first - 1, b = last;;) {
    for(; (++a < b) && ((PA[*a] + depth) >= (PA[*a + 1] + 1));) { *a = ~*a; }
    for(; (a < --b) && ((PA[*b] + depth) <  (PA[*b + 1] + 1));) { }
    if(b <= a) { break; }
    t = ~*b;
    *b = *a;
    *a = t;
  }
  if(first < a) { *first = ~*first; }
  return a;
}

/* Multikey introsort for medium size groups. */
static
void
_multikey_introsort(const sauchar_t *T, const saidx_t *PA,
                    saidx_t *first, saidx_t *last,
                    saidx_t depth) {
  struct { saidx_t *a, *b, c, d; } stack[STACK_SIZE];
  const sauchar_t *Td;
  saidx_t *a, *b, *c, *d, *e, *f;
  saidx_t s, t;
  saidx_t ssize;
  saidx_t limit;
  saint_t v, x = 0;

  for(ssize = 0, limit = _lg(last - first);;) {

    if((last - first) <= SS_INSERTIONSORT_THRESHOLD) {
      if(1 < (last - first)) { _insertionsort(T, PA, first, last, depth); }
      STACK_POP(first, last, depth, limit);
      continue;
    }

    Td = T + depth;
    if(limit-- == 0) { _heapsort(Td, PA, first, last - first); }
    if(limit < 0) {
      for(a = first + 1, v = Td[PA[*first]]; a < last; ++a) {
        if((x = Td[PA[*a]]) != v) {
          if(1 < (a - first)) { break; }
          v = x;
          first = a;
        }
      }
      if(Td[PA[*first] - 1] < v) {
        first = _substring_partition(PA, first, a, depth);
      }
      if((a - first) <= (last - a)) {
        if(1 < (a - first)) {
          STACK_PUSH(a, last, depth, -1);
          last = a, depth += 1, limit = _lg(a - first);
        } else {
          first = a, limit = -1;
        }
      } else {
        if(1 < (last - a)) {
          STACK_PUSH(first, a, depth + 1, _lg(a - first));
          first = a, limit = -1;
        } else {
          last = a, depth += 1, limit = _lg(a - first);
        }
      }
      continue;
    }

    /* choose pivot */
    a = _pivot(Td, PA, first, last);
    v = Td[PA[*a]];
    SWAP(*first, *a);

    /* partition */
    for(b = first; (++b < last) && ((x = Td[PA[*b]]) == v);) { }
    if(((a = b) < last) && (x < v)) {
      for(; (++b < last) && ((x = Td[PA[*b]]) <= v);) {
        if(x == v) { SWAP(*b, *a); ++a; }
      }
    }
    for(c = last; (b < --c) && ((x = Td[PA[*c]]) == v);) { }
    if((b < (d = c)) && (x > v)) {
      for(; (b < --c) && ((x = Td[PA[*c]]) >= v);) {
        if(x == v) { SWAP(*c, *d); --d; }
      }
    }
    for(; b < c;) {
      SWAP(*b, *c);
      for(; (++b < c) && ((x = Td[PA[*b]]) <= v);) {
        if(x == v) { SWAP(*b, *a); ++a; }
      }
      for(; (b < --c) && ((x = Td[PA[*c]]) >= v);) {
        if(x == v) { SWAP(*c, *d); --d; }
      }
    }

    if(a <= d) {
      c = b - 1;

      if((s = a - first) > (t = b - a)) { s = t; }
      for(e = first, f = b - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
      if((s = d - c) > (t = last - d - 1)) { s = t; }
      for(e = b, f = last - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }

      a = first + (b - a), c = last - (d - c);
      b = (v <= Td[PA[*a] - 1]) ? a : _substring_partition(PA, a, c, depth);

      if((a - first) <= (last - c)) {
        if((last - c) <= (c - b)) {
          STACK_PUSH(b, c, depth + 1, _lg(c - b));
          STACK_PUSH(c, last, depth, limit);
          last = a;
        } else if((a - first) <= (c - b)) {
          STACK_PUSH(c, last, depth, limit);
          STACK_PUSH(b, c, depth + 1, _lg(c - b));
          last = a;
        } else {
          STACK_PUSH(c, last, depth, limit);
          STACK_PUSH(first, a, depth, limit);
          first = b, last = c, depth += 1, limit = _lg(c - b);
        }
      } else {
        if((a - first) <= (c - b)) {
          STACK_PUSH(b, c, depth + 1, _lg(c - b));
          STACK_PUSH(first, a, depth, limit);
          first = c;
        } else if((last - c) <= (c - b)) {
          STACK_PUSH(first, a, depth, limit);
          STACK_PUSH(b, c, depth + 1, _lg(c - b));
          first = c;
        } else {
          STACK_PUSH(first, a, depth, limit);
          STACK_PUSH(c, last, depth, limit);
          first = b, last = c, depth += 1, limit = _lg(c - b);
        }
      }
    } else {
      limit += 1;
      if(Td[PA[*first] - 1] < v) {
        first = _substring_partition(PA, first, last, depth);
        limit = _lg(last - first);
      }
      depth += 1;
    }
  }
}


/*---------------------------------------------------------------------------*/

/* Block swapping */
static INLINE
void
_block_swap(saidx_t *first1, saidx_t *first2, saidx_t size) {
  saidx_t *a, *b;
  saidx_t i, t;
  for(i = size, a = first1, b = first2; 0 < i; --i, ++a, ++b) {
    SWAP(*a, *b);
  }
}

/* Merge-forward with internal buffer. */
static
void
_merge_forward(const sauchar_t *T, const saidx_t *PA,
               saidx_t *buf, saidx_t *first, saidx_t *middle, saidx_t *last,
               saidx_t depth) {
  saidx_t *bufend;
  saidx_t *i, *j, *k;
  saidx_t t;
  saint_t r;

  bufend = buf + (middle - first) - 1;
  _block_swap(buf, first, middle - first);

  for(t = *first, i = first, j = buf, k = middle;;) {
    r = _compare(T, PA + *j, PA + *k, depth);
    if(r < 0) {
      do {
        *i++ = *j;
        if(bufend <= j) { *j = t; return; }
        *j++ = *i;
      } while(*j < 0);
    } else if(r > 0) {
      do {
        *i++ = *k, *k++ = *i;
        if(last <= k) {
          while(j < bufend) { *i++ = *j, *j++ = *i; }
          *i = *j, *j = t;
          return;
        }
      } while(*k < 0);
    } else {
      *k = ~*k;
      do {
        *i++ = *j;
        if(bufend <= j) { *j = t; return; }
        *j++ = *i;
      } while(*j < 0);

      do {
        *i++ = *k, *k++ = *i;
        if(last <= k) {
          while(j < bufend) { *i++ = *j, *j++ = *i; }
          *i = *j, *j = t;
          return;
        }
      } while(*k < 0);
    }
  }
}

/* Merge-backward with internal buffer. */
static
void
_merge_backward(const sauchar_t *T, const saidx_t *PA, saidx_t *buf,
                saidx_t *first, saidx_t *middle, saidx_t *last,
                saidx_t depth) {
  const saidx_t *p1, *p2;
  saidx_t *bufend;
  saidx_t *i, *j, *k;
  saidx_t t;
  saint_t r;
  saint_t x;

  bufend = buf + (last - middle);
  _block_swap(buf, middle, last - middle);

  x = 0;
  if(*(bufend - 1) < 0) { x |=  1; p1 = PA + ~*(bufend - 1); }
  else                  {          p1 = PA +  *(bufend - 1); }
  if(*(middle - 1) < 0) { x |=  2; p2 = PA + ~*(middle - 1); }
  else                  {          p2 = PA +  *(middle - 1); }
  for(t = *(last - 1), i = last - 1, j = bufend - 1, k = middle - 1;;) {

    r = _compare(T, p1, p2, depth);
    if(r > 0) {
      if(x & 1) { do { *i-- = *j; *j-- = *i; } while(*j < 0); x ^= 1; }
      *i-- = *j;
      if(j <= buf) { *j = t; return; }
      *j-- = *i;

      if(*j < 0) { x |=  1; p1 = PA + ~*j; }
      else       {          p1 = PA +  *j; }
    } else if(r < 0) {
      if(x & 2) { do { *i-- = *k; *k-- = *i; } while(*k < 0); x ^= 2; }
      *i-- = *k; *k-- = *i;
      if(k < first) {
        while(buf < j) { *i-- = *j; *j-- = *i; }
        *i = *j, *j = t;
        return;
      }

      if(*k < 0) { x |=  2; p2 = PA + ~*k; }
      else       {          p2 = PA +  *k; }
    } else {
      if(x & 1) { do { *i-- = *j; *j-- = *i; } while(*j < 0); x ^= 1; }
      *i-- = ~*j;
      if(j <= buf) { *j = t; return; }
      *j-- = *i;

      if(x & 2) { do { *i-- = *k; *k-- = *i; } while(*k < 0); x ^= 2; }
      *i-- = *k; *k-- = *i;
      if(k < first) {
        while(buf < j) { *i-- = *j; *j-- = *i; }
        *i = *j, *j = t;
        return;
      }

      if(*j < 0) { x |=  1; p1 = PA + ~*j; }
      else       {          p1 = PA +  *j; }
      if(*k < 0) { x |=  2; p2 = PA + ~*k; }
      else       {          p2 = PA +  *k; }
    }
  }
}

/* Faster merge (based on divide and conquer technique). */
static
void
_merge(const sauchar_t *T, const saidx_t *PA,
       saidx_t *first, saidx_t *middle, saidx_t *last,
       saidx_t *buf, saidx_t bufsize,
       saidx_t depth) {
#define GETIDX(a) ((0 <= (a)) ? (a) : (~(a)))
#define MERGE_CHECK_EQUAL(a)\
  do {\
    if((0 <= *(a)) &&\
       (_compare(T, PA + GETIDX(*((a) - 1)), PA + *(a), depth) == 0)) {\
      *(a) = ~*(a);\
    }\
  } while(0)
  struct { saidx_t *a, *b, *c; int d; } stack[STACK_SIZE];
  saidx_t *i, *j;
  saidx_t m, len, half;
  saidx_t ssize;
  int check, next;

  for(check = 0, ssize = 0;;) {

    if((last - middle) <= bufsize) {
      if((first < middle) && (middle < last)) {
        _merge_backward(T, PA, buf, first, middle, last, depth);
      }
      if(check & 1) { MERGE_CHECK_EQUAL(first); }
      if(check & 2) { MERGE_CHECK_EQUAL(last); }
      STACK_POP(first, middle, last, check);
      continue;
    }

    if((middle - first) <= bufsize) {
      if(first < middle) {
        _merge_forward(T, PA, buf, first, middle, last, depth);
      }
      if(check & 1) { MERGE_CHECK_EQUAL(first); }
      if(check & 2) { MERGE_CHECK_EQUAL(last); }
      STACK_POP(first, middle, last, check);
      continue;
    }

    for(m = 0, len = MIN(middle - first, last - middle), half = len >> 1;
        0 < len;
        len = half, half >>= 1) {
      if(_compare(T, PA + GETIDX(*(middle + m + half)),
                     PA + GETIDX(*(middle - m - half - 1)), depth) < 0) {
        m += half + 1;
        half -= (len & 1) ^ 1;
      }
    }

    if(0 < m) {
      _block_swap(middle - m, middle, m);
      i = j = middle, next = 0;
      if((middle + m) < last) {
        if(*(middle + m) < 0) {
          for(; *(i - 1) < 0; --i) { }
          *(middle + m) = ~*(middle + m);
        }
        for(j = middle; *j < 0; ++j) { }
        next = 1;
      }
      if((i - first) <= (last - j)) {
        STACK_PUSH(j, middle + m, last, (check &  2) | (next & 1));
        middle -= m, last = i, check = (check & 1);
      } else {
        if((i == middle) && (middle == j)) { next <<= 1; }
        STACK_PUSH(first, middle - m, i, (check & 1) | (next & 2));
        first = j, middle += m, check = (check & 2) | (next & 1);
      }
    } else {
      if(check & 1) { MERGE_CHECK_EQUAL(first); }
      MERGE_CHECK_EQUAL(middle);
      if(check & 2) { MERGE_CHECK_EQUAL(last); }
      STACK_POP(first, middle, last, check);
    }
  }
}


/*---------------------------------------------------------------------------*/

/*- Function -*/

/* Substring sort */
void
substringsort(const sauchar_t *T, const saidx_t *PA,
              saidx_t *first, saidx_t *last,
              saidx_t *buf, saidx_t bufsize,
              saidx_t depth, saint_t lastsuffix) {
  saidx_t *a, *b;
  saidx_t *curbuf;
  saidx_t i, j, k;
  saidx_t curbufsize;

  if(lastsuffix != 0) { ++first; }
  for(a = first, i = 0; (a + SS_BLOCKSIZE) < last; a += SS_BLOCKSIZE, ++i) {
    _multikey_introsort(T, PA, a, a + SS_BLOCKSIZE, depth);
    curbuf = a + SS_BLOCKSIZE;
    curbufsize = last - (a + SS_BLOCKSIZE);
    if(curbufsize <= bufsize) { curbufsize = bufsize, curbuf = buf; }
    for(b = a, k = SS_BLOCKSIZE, j = i; j & 1; b -= k, k <<= 1, j >>= 1) {
      _merge(T, PA, b - k, b, b + k, curbuf, curbufsize, depth);
    }
  }
  _multikey_introsort(T, PA, a, last, depth);
  for(k = SS_BLOCKSIZE; i != 0; k <<= 1, i >>= 1) {
    if(i & 1) {
      _merge(T, PA, a - k, a, last, buf, bufsize, depth);
      a -= k;
    }
  }

  if(lastsuffix != 0) {
    /* Insert last type B* suffix. */
    for(a = first, i = *(first - 1);
        (a < last) && ((*a < 0) || (0 < _compare(T, PA + i, PA + *a, depth)));
        ++a) {
      *(a - 1) = *a;
    }
    *(a - 1) = i;
  }
}
