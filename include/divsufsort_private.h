/*
 * divsufsort_private.h for libdivsufsort
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

#ifndef _DIVSUFSORT_PRIVATE_H
#define _DIVSUFSORT_PRIVATE_H 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <assert.h>
#include <stdio.h>
#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
# if HAVE_MEMORY_H
#  include <memory.h>
# endif
#endif
#if HAVE_STDDEF_H
# include <stddef.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif
#include "divsufsort.h"


/*- Constants -*/
#if !defined(UINT8_MAX)
# define UINT8_MAX (255)
#endif /* UINT8_MAX */
#if defined(ALPHABET_SIZE) && (ALPHABET_SIZE < 1)
# undef ALPHABET_SIZE
#endif
#if !defined(ALPHABET_SIZE)
# define ALPHABET_SIZE (UINT8_MAX + 1)
#endif
#if defined(STACK_SIZE) && (STACK_SIZE < 32)
# undef STACK_SIZE
#endif
#if !defined(STACK_SIZE)
# define STACK_SIZE (64)
#endif
#if defined(LOCALMERGE_BUFFERSIZE) && (LOCALMERGE_BUFFERSIZE < 32)
# undef LOCALMERGE_BUFFERSIZE
#endif
#if !defined(LOCALMERGE_BUFFERSIZE)
# define LOCALMERGE_BUFFERSIZE (256)
#endif
/* for divsufsort.c */
#define BUCKET_A_SIZE (ALPHABET_SIZE)
#define BUCKET_B_SIZE (ALPHABET_SIZE * ALPHABET_SIZE)
/* for sssort.c */
#define SS_INSERTIONSORT_THRESHOLD (8)
#define SS_BLOCKSIZE (1024)
/* for trsort.c */
#define LS_INSERTIONSORT_THRESHOLD (8)
#define TR_INSERTIONSORT_THRESHOLD (8)


/*- Macros -*/
#ifndef SWAP
# define SWAP(a,b) do { t = (a); (a) = (b); (b) = t; } while(0)
#endif /* SWAP */
#ifndef MIN
# define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif /* MIN */
#ifndef MAX
# define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif /* MAX */
#define STACK_PUSH3(_a, _b, _c)\
  do {\
    assert(ssize < STACK_SIZE);\
    stack[ssize].a = (_a), stack[ssize].b = (_b),\
    stack[ssize++].c = (_c);\
  } while(0)
#define STACK_PUSH(_a, _b, _c, _d)\
  do {\
    assert(ssize < STACK_SIZE);\
    stack[ssize].a = (_a), stack[ssize].b = (_b),\
    stack[ssize].c = (_c), stack[ssize++].d = (_d);\
  } while(0)
#define STACK_POP3(_a, _b, _c)\
  do {\
    assert(0 <= ssize);\
    if(ssize == 0) { return; }\
    (_a) = stack[--ssize].a, (_b) = stack[ssize].b,\
    (_c) = stack[ssize].c;\
  } while(0)
#define STACK_POP(_a, _b, _c, _d)\
  do {\
    assert(0 <= ssize);\
    if(ssize == 0) { return; }\
    (_a) = stack[--ssize].a, (_b) = stack[ssize].b,\
    (_c) = stack[ssize].c, (_d) = stack[ssize].d;\
  } while(0)
/* for divsufsort.c */
#define BUCKET_A(c0) bucket_A[(c0)]
#if ALPHABET_SIZE == 256
#define BUCKET_B(c0, c1) (bucket_B[((c1) << 8) | (c0)])
#define BUCKET_BSTAR(c0, c1) (bucket_B[((c0) << 8) | (c1)])
#else
#define BUCKET_B(c0, c1) (bucket_B[(c1) * ALPHABET_SIZE + (c0)])
#define BUCKET_BSTAR(c0, c1) (bucket_B[(c0) * ALPHABET_SIZE + (c1)])
#endif


/*- Private Prototypes -*/
/* substringsort.c */
void
substringsort(const sauchar_t *Td, const saidx_t *PA,
              saidx_t *first, saidx_t *last,
              saidx_t *buf, saidx_t bufsize,
              saidx_t depth, saint_t lastsuffix);
/* trsort.c */
void
trsort(saidx_t *ISA, saidx_t *SA, saidx_t n, saidx_t depth);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _DIVSUFSORT_PRIVATE_H_ */
