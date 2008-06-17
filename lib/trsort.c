/*
 * trsort.c for libdivsufsort
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

static inline
void
_fixdown(const saidx_t *ISAd, saidx_t *SA, saidx_t i, saidx_t size) {
  saidx_t j, k;
  saidx_t v;
  saidx_t c, d, e;

  for(v = SA[i], c = ISAd[v]; (j = 2 * i + 1) < size; SA[i] = SA[k], i = k) {
    d = ISAd[SA[k = j++]];
    if(d < (e = ISAd[SA[j]])) { k = j; d = e; }
    if(d <= c) { break; }
  }
  SA[i] = v;
}

/* Simple top-down heapsort. */
static
void
_heapsort(const saidx_t *ISAd, saidx_t *SA, saidx_t size) {
  saidx_t i, m;
  saidx_t t;

  m = size;
  if((size % 2) == 0) {
    m--;
    if(ISAd[SA[m / 2]] < ISAd[SA[m]]) {
      SWAP(SA[m], SA[m / 2]);
    }
  }

  for(i = m / 2 - 1; 0 <= i; --i) {
    _fixdown(ISAd, SA, i, m);
  }

  if((size % 2) == 0) {
    SWAP(SA[0], SA[m]);
    _fixdown(ISAd, SA, 0, m);
  }

  for(i = m - 1; 0 < i; --i) {
    t = SA[0];
    SA[0] = SA[i];
    _fixdown(ISAd, SA, 0, i);
    SA[i] = t;
  }
}

/* Simple insertionsort for small size groups. */
static
void
_insertionsort(const saidx_t *ISAd, saidx_t *first, saidx_t *last) {
  saidx_t *a, *b;
  saidx_t t, r;

  for(a = first + 1; a < last; ++a) {
    for(t = *a, b = a - 1; 0 > (r = ISAd[t] - ISAd[*b]);) {
      do { *(b + 1) = *b; } while((first <= --b) && (*b < 0));
      if(b < first) { break; }
    }
    if(r == 0) { *b = ~*b; }
    *(b + 1) = t;
  }
}

static inline
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
/*
  return ((n & 0xff00) != 0) ?
          8 + log2table[(n >> 8) & 0xff] :
          log2table[n & 0xff];
*/
  /* for 32 bits */
  return (n & 0xffff0000) ?
          ((n & 0xff000000) ?
            24 + log2table[(n >> 24) & 0xff] :
            16 + log2table[(n >> 16) & 0xff]) :
          ((n & 0x0000ff00) ?
             8 + log2table[(n >>  8) & 0xff] :
             0 + log2table[(n >>  0) & 0xff]);
}


/*---------------------------------------------------------------------------*/

/* Returns the median of three elements. */
static inline
saidx_t *
_median3(const saidx_t *ISAd, saidx_t *v1, saidx_t *v2, saidx_t *v3) {
  saidx_t *t;
  if(ISAd[*v1] > ISAd[*v2]) { SWAP(v1, v2); }
  if(ISAd[*v2] > ISAd[*v3]) {
    if(ISAd[*v1] > ISAd[*v3]) { return v1; }
    else { return v3; }
  }
  return v2;
}

/* Returns the median of five elements. */
static inline
saidx_t *
_median5(const saidx_t *ISAd,
         saidx_t *v1, saidx_t *v2, saidx_t *v3, saidx_t *v4, saidx_t *v5) {
  saidx_t *t;
  if(ISAd[*v2] > ISAd[*v3]) { SWAP(v2, v3); }
  if(ISAd[*v4] > ISAd[*v5]) { SWAP(v4, v5); }
  if(ISAd[*v2] > ISAd[*v4]) { SWAP(v2, v4); SWAP(v3, v5); }
  if(ISAd[*v1] > ISAd[*v3]) { SWAP(v1, v3); }
  if(ISAd[*v1] > ISAd[*v4]) { SWAP(v1, v4); SWAP(v3, v5); }
  if(ISAd[*v3] > ISAd[*v4]) { return v4; }
  return v3;
}

/* Returns the pivot element. */
static inline
saidx_t *
_pivot(const saidx_t *ISAd, saidx_t *first, saidx_t *last) {
  saidx_t *middle;
  saidx_t t;

  t = last - first;
  middle = first + t / 2;

  if(t <= 512) {
    if(t <= 32) {
      return _median3(ISAd, first, middle, last - 1);
    } else {
      t >>= 2;
      return _median5(ISAd,
                      first, first + t,
                      middle,
                      last - 1 - t, last - 1);
    }
  }
  t >>= 3;
  return _median3(ISAd,
           _median3(ISAd, first, first + t, first + (t << 1)),
           _median3(ISAd, middle - t, middle, middle + t),
           _median3(ISAd, last - 1 - (t << 1), last - 1 - t, last - 1));
}


/*---------------------------------------------------------------------------*/

/* Update group numbers. */
static
void
_ls_updategroup(saidx_t *ISA, const saidx_t *SA,
                saidx_t *first, saidx_t *last) {
  saidx_t *a, *b;
  saidx_t t;

  for(a = first; a < last; ++a) {
    if(0 <= *a) {
      b = a;
      do { ISA[*a] = a - SA; } while((++a < last) && (0 <= *a));
      *b = b - a;
      if(last <= a) { break; }
    }
    b = a;
    do { *a = ~*a; } while(*++a < 0);
    t = a - SA;
    do { ISA[*b] = t; } while(++b <= a);
  }
}

/* Introspective sort. */
static
void
_ls_introsort(saidx_t *ISA, saidx_t *ISAd, const saidx_t *SA,
              saidx_t *first, saidx_t *last) {
  struct { saidx_t *a, *b, c; } stack[STACK_SIZE];
  saidx_t *a, *b, *c, *d, *e, *f;
  saidx_t s, t;
  saidx_t limit;
  saidx_t v, x = 0;
  int ssize;

  for(ssize = 0, limit = _lg(last - first);;) {

    if((last - first) <= LS_INSERTIONSORT_THRESHOLD) {
      if(1 < (last - first)) {
        _insertionsort(ISAd, first, last);
        _ls_updategroup(ISA, SA, first, last);
      } else if((last - first) == 1) { *first = -1; }
      STACK_POP3(first, last, limit);
      continue;
    }

    if(limit-- == 0) {
      _heapsort(ISAd, first, last - first);
      for(a = last - 1; first < a; a = b) {
        for(x = ISAd[*a], b = a - 1; (first <= b) && (ISAd[*b] == x); --b) { *b = ~*b; }
      }
      _ls_updategroup(ISA, SA, first, last);
      STACK_POP3(first, last, limit);
      continue;
    }

    /* choose pivot */
    a = _pivot(ISAd, first, last);
    SWAP(*first, *a);
    v = ISAd[*first];

    /* Two-stage double-index controlled ternary partition */
    for(b = first; (++b < last) && ((x = ISAd[*b]) == v);) { }
    if(((a = b) < last) && (x < v)) {
      for(; (++b < last) && ((x = ISAd[*b]) <= v);) {
        if(x == v) { SWAP(*b, *a); ++a; }
      }
    }
    for(c = last; (b < --c) && ((x = ISAd[*c]) == v);) { }
    if((b < (d = c)) && (x > v)) {
      for(; (b < --c) && ((x = ISAd[*c]) >= v);) {
        if(x == v) { SWAP(*c, *d); --d; }
      }
    }
    for(; b < c;) {
      SWAP(*b, *c);
      for(; (++b < c) && ((x = ISAd[*b]) <= v);) {
        if(x == v) { SWAP(*b, *a); ++a; }
      }
      for(; (b < --c) && ((x = ISAd[*c]) >= v);) {
        if(x == v) { SWAP(*c, *d); --d; }
      }
    }

    if(a <= d) {
      c = b - 1;

      if((s = a - first) > (t = b - a)) { s = t; }
      for(e = first, f = b - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
      if((s = d - c) > (t = last - d - 1)) { s = t; }
      for(e = b, f = last - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }

      a = first + (b - a), b = last - (d - c);

      /* update ranks */
      for(c = first, v = a - SA - 1; c < a; ++c) { ISA[*c] = v; }
      if(b < last) { for(c = a, v = b - SA - 1; c < b; ++c) { ISA[*c] = v; } }
      if((b - a) == 1) { *a = - 1; }

      if((a - first) <= (last - b)) {
        if(first < a) {
          STACK_PUSH3(b, last, limit);
          last = a;
        } else {
          first = b;
        }
      } else {
        if(b < last) {
          STACK_PUSH3(first, a, limit);
          first = b;
        } else {
          last = a;
        }
      }
    } else {
      STACK_POP3(first, last, limit);
    }
  }
}


/*---------------------------------------------------------------------------*/

/* Larsson-Sadakane sort */
static
void
_lssort(saidx_t *ISA, saidx_t *SA, saidx_t n, saidx_t depth) {
  saidx_t *ISAd;
  saidx_t *first, *last;
  saidx_t t, skip;

  for(ISAd = ISA + depth; -n < *SA; ISAd += (ISAd - ISA)) {
    first = SA;
    skip = 0;
    do {
      if((t = *first) < 0) { first -= t; skip += t; }
      else {
        if(skip != 0) { *(first + skip) = skip; skip = 0; }
        last = SA + ISA[t] + 1;
        _ls_introsort(ISA, ISAd, SA, first, last);
        first = last;
      }
    } while(first < (SA + n));
    if(skip != 0) { *(first + skip) = skip; }
  }
}


/*---------------------------------------------------------------------------*/

static
void
_tr_partition(const saidx_t *ISAd, const saidx_t *SA,
              saidx_t *first, saidx_t *last,
              saidx_t **pa, saidx_t **pb, saidx_t v) {
  saidx_t *a, *b, *c, *d, *e, *f;
  saidx_t t, s;
  saidx_t x = 0;

  for(b = first - 1; (++b < last) && ((x = ISAd[*b]) == v);) { }
  if(((a = b) < last) && (x < v)) {
    for(; (++b < last) && ((x = ISAd[*b]) <= v);) {
      if(x == v) { SWAP(*b, *a); ++a; }
    }
  }
  for(c = last; (b < --c) && ((x = ISAd[*c]) == v);) { }
  if((b < (d = c)) && (x > v)) {
    for(; (b < --c) && ((x = ISAd[*c]) >= v);) {
      if(x == v) { SWAP(*c, *d); --d; }
    }
  }
  for(; b < c;) {
    SWAP(*b, *c);
    for(; (++b < c) && ((x = ISAd[*b]) <= v);) {
      if(x == v) { SWAP(*b, *a); ++a; }
    }
    for(; (b < --c) && ((x = ISAd[*c]) >= v);) {
      if(x == v) { SWAP(*c, *d); --d; }
    }
  }

  if(a <= d) {
    c = b - 1;
    if((s = a - first) > (t = b - a)) { s = t; }
    for(e = first, f = b - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
    if((s = d - c) > (t = last - d - 1)) { s = t; }
    for(e = b, f = last - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
    first += (b - a), last -= (d - c);
  }
  *pa = first, *pb = last;
}

static
void
_tr_copy(saidx_t *ISA, const saidx_t *SA,
         saidx_t *first, saidx_t *a, saidx_t *b, saidx_t *last,
         saidx_t depth) {
  /* sort suffixes of middle partition
     by using sorted order of suffixes of left and right partition. */
  saidx_t *c, *d, *e;
  saidx_t s, v;

  v = b - SA - 1;
  for(c = first, d = a - 1; c <= d; ++c) {
    if((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *++d = s;
      ISA[s] = d - SA;
    }
  }
  for(c = last - 1, e = d + 1, d = b; e < d; --c) {
    if((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *--d = s;
      ISA[s] = d - SA;
    }
  }
}

/* Multikey introsort. */
static
void
_tr_introsort(saidx_t *ISA, const saidx_t *ISAd,
              saidx_t *SA, saidx_t *first, saidx_t *last,
              saidx_t *budget, saidx_t *chance, saidx_t size) {
#define UPDATE_BUDGET(n)\
  {\
    (*budget) -= (n);\
    if(*budget <= 0) {\
      if(--(*chance) == 0) { break; }\
      (*budget) += size;\
    }\
  }
  struct { const saidx_t *a; saidx_t *b, *c, d; }stack[STACK_SIZE];
  saidx_t *a, *b, *c, *d, *e, *f;
  saidx_t s, t;
  saidx_t v, x = 0;
  saidx_t limit, next;
  int ssize;

  for(ssize = 0, limit = _lg(last - first);;) {

    if(limit < 0) {
      if(limit == -1) {
        /* tandem repeat partition */
        UPDATE_BUDGET(last - first);
        _tr_partition(ISAd - 1, SA, first, last, &a, &b, last - SA - 1);

        /* update ranks */
        if(a < last) {
          for(c = first, v = a - SA - 1; c < a; ++c) { ISA[*c] = v; }
        }
        if(b < last) {
          for(c = a, v = b - SA - 1; c < b; ++c) { ISA[*c] = v; }
        }

        /* push */
        STACK_PUSH(NULL, a, b, 0);
        STACK_PUSH(ISAd - 1, first, last, -2);
        if((a - first) <= (last - b)) {
          if(1 < (a - first)) {
            STACK_PUSH(ISAd, b, last, _lg(last - b));
            last = a, limit = _lg(a - first);
          } else if(1 < (last - b)) {
            first = b, limit = _lg(last - b);
          } else {
            STACK_POP(ISAd, first, last, limit);
          }
        } else {
          if(1 < (last - b)) {
            STACK_PUSH(ISAd, first, a, _lg(a - first));
            first = b, limit = _lg(last - b);
          } else if(1 < (a - first)) {
            last = a, limit = _lg(a - first);
          } else {
            STACK_POP(ISAd, first, last, limit);
          }
        }
      } else if(limit == -2) {
        /* tandem repeat copy */
        a = stack[--ssize].b, b = stack[ssize].c;
        _tr_copy(ISA, SA, first, a, b, last, ISAd - ISA);
        STACK_POP(ISAd, first, last, limit);
      } else {
        /* sorted partition */
        if(0 <= *first) {
          a = first;
          do { ISA[*a] = a - SA; } while((++a < last) && (0 <= *a));
          first = a;
        }
        if(first < last) {
          a = first; do { *a = ~*a; } while(*++a < 0);
          next = (ISA[*a] != ISAd[*a]) ? _lg(a - first + 1) : -1;
          if(++a < last) { for(b = first, v = a - SA - 1; b < a; ++b) { ISA[*b] = v; } }

          /* push */
          if((a - first) <= (last - a)) {
            STACK_PUSH(ISAd, a, last, -3);
            ISAd += 1, last = a, limit = next;
          } else {
            if(1 < (last - a)) {
              STACK_PUSH(ISAd + 1, first, a, next);
              first = a, limit = -3;
            } else {
              ISAd += 1, last = a, limit = next;
            }
          }
        } else {
          STACK_POP(ISAd, first, last, limit);
        }
      }
      continue;
    }

    if((last - first) <= TR_INSERTIONSORT_THRESHOLD) {
      UPDATE_BUDGET(last - first);
//      UPDATE_BUDGET((last - first) * (last - first) / 2);
      _insertionsort(ISAd, first, last);
      limit = -3;
      continue;
    }

    if(limit-- == 0) {
      UPDATE_BUDGET(last - first);
//      UPDATE_BUDGET((last - first) * _lg(last - first));
      _heapsort(ISAd, first, last - first);
      for(a = last - 1; first < a; a = b) {
        for(x = ISAd[*a], b = a - 1; (first <= b) && (ISAd[*b] == x); --b) { *b = ~*b; }
      }
      limit = -3;
      continue;
    }

    UPDATE_BUDGET(last - first);

    /* choose pivot */
    a = _pivot(ISAd, first, last);
    SWAP(*first, *a);
    v = ISAd[*first];

    /* partition */
    for(b = first; (++b < last) && ((x = ISAd[*b]) == v);) { }
    if(((a = b) < last) && (x < v)) {
      for(; (++b < last) && ((x = ISAd[*b]) <= v);) {
        if(x == v) { SWAP(*b, *a); ++a; }
      }
    }
    for(c = last; (b < --c) && ((x = ISAd[*c]) == v);) { }
    if((b < (d = c)) && (x > v)) {
      for(; (b < --c) && ((x = ISAd[*c]) >= v);) {
        if(x == v) { SWAP(*c, *d); --d; }
      }
    }
    for(; b < c;) {
      SWAP(*b, *c);
      for(; (++b < c) && ((x = ISAd[*b]) <= v);) {
        if(x == v) { SWAP(*b, *a); ++a; }
      }
      for(; (b < --c) && ((x = ISAd[*c]) >= v);) {
        if(x == v) { SWAP(*c, *d); --d; }
      }
    }

    if(a <= d) {
      c = b - 1;

      if((s = a - first) > (t = b - a)) { s = t; }
      for(e = first, f = b - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
      if((s = d - c) > (t = last - d - 1)) { s = t; }
      for(e = b, f = last - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }

      a = first + (b - a), b = last - (d - c);
      next = (ISA[*a] != v) ? _lg(b - a) : -1;

      /* update ranks */
      for(c = first, v = a - SA - 1; c < a; ++c) { ISA[*c] = v; }
      if(b < last) { for(c = a, v = b - SA - 1; c < b; ++c) { ISA[*c] = v; } }

      /* push */
      if((a - first) <= (last - b)) {
        if((last - b) <= (b - a)) {
          if(1 < (a - first)) {
            STACK_PUSH(ISAd + 1, a, b, next);
            STACK_PUSH(ISAd, b, last, limit);
            last = a;
          } else if(1 < (last - b)) {
            STACK_PUSH(ISAd + 1, a, b, next);
            first = b;
          } else if(1 < (b - a)) {
            ISAd += 1, first = a, last = b, limit = next;
          } else {
            STACK_POP(ISAd, first, last, limit);
          }
        } else if((a - first) <= (b - a)) {
          if(1 < (a - first)) {
            STACK_PUSH(ISAd, b, last, limit);
            STACK_PUSH(ISAd + 1, a, b, next);
            last = a;
          } else if(1 < (b - a)) {
            STACK_PUSH(ISAd, b, last, limit);
            ISAd += 1, first = a, last = b, limit = next;
          } else {
            first = b;
          }
        } else {
          if(1 < (b - a)) {
            STACK_PUSH(ISAd, b, last, limit);
            STACK_PUSH(ISAd, first, a, limit);
            ISAd += 1, first = a, last = b, limit = next;
          } else {
            STACK_PUSH(ISAd, b, last, limit);
            last = a;
          }
        }
      } else {
        if((a - first) <= (b - a)) {
          if(1 < (last - b)) {
            STACK_PUSH(ISAd + 1, a, b, next);
            STACK_PUSH(ISAd, first, a, limit);
            first = b;
          } else if(1 < (a - first)) {
            STACK_PUSH(ISAd + 1, a, b, next);
            last = a;
          } else if(1 < (b - a)) {
            ISAd += 1, first = a, last = b, limit = next;
          } else {
            STACK_POP(ISAd, first, last, limit);
          }
        } else if((last - b) <= (b - a)) {
          if(1 < (last - b)) {
            STACK_PUSH(ISAd, first, a, limit);
            STACK_PUSH(ISAd + 1, a, b, next);
            first = b;
          } else if(1 < (b - a)) {
            STACK_PUSH(ISAd, first, a, limit);
            ISAd += 1, first = a, last = b, limit = next;
          } else {
            last = a;
          }
        } else {
          if(1 < (b - a)) {
            STACK_PUSH(ISAd, first, a, limit);
            STACK_PUSH(ISAd, b, last, limit);
            ISAd += 1, first = a, last = b, limit = next;
          } else {
            STACK_PUSH(ISAd, first, a, limit);
            first = b;
          }
        }
      }
    } else {
      limit += 1, ISAd += 1;
    }
  }

  for(s = 0; s < ssize; ++s) {
    if(stack[s].d == -3) {
      _ls_updategroup(ISA, SA, stack[s].b, stack[s].c);
    }
  }
}


/*---------------------------------------------------------------------------*/

/*- Function -*/

/* Tandem repeat sort */
void
trsort(saidx_t *ISA, saidx_t *SA, saidx_t n, saidx_t depth) {
  saidx_t *first, *last;
  saidx_t t, skip;
  saidx_t budget;
  saidx_t chance;

  if(-n < *SA) {
    first = SA;
    skip = 0;
    budget = n;
/*    chance = _lg(n); */
/*    chance = _lg(n) / 2 + 1; */
    chance = _lg(n) * 2 / 3 + 1;
    do {
      if((t = *first) < 0) { first -= t; skip += t; }
      else {
        skip = 0;
        last = SA + ISA[t] + 1;
        if(1 < (last - first)) {
          _tr_introsort(ISA, ISA + depth, SA, first, last, &budget, &chance, n);
          if(chance == 0) {
            /* Switch to Larsson-Sadakane sorting algorithm. */
            if(SA < first) { *SA = -(first - SA); }
            _lssort(ISA, SA, n, depth);
            break;
          }
        }
        first = last;
      }
    } while(first < (SA + n));
  }
}
