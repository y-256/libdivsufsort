/*
 * sasearch.c for libdivsufsort
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
#include <string.h>
#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif


#define SA_SORT_LEXICOGRAPHICALORDER (1 << 0)
#define SA_PRINT_OFFSET (1 << 1)
#define SA_PRINT_FILENAME (1 << 2)
#define SA_HEX_MODE (1 << 3)

typedef struct _searchoption_t searchoption_t;
struct _searchoption_t {
  const char *fname;
  saidx_t maxcount;
  saidx_t blen, alen;
  unsigned int flags;
  void(*func)(const sauchar_t *T, saidx_t Tsize,
              const sauchar_t *P, saidx_t Psize,
              const saidx_t *SA, saidx_t SAsize,
              saidx_t left, saidx_t size, searchoption_t *opt);
};

static
void
_print_suffix(const sauchar_t *T, saidx_t Tsize, saidx_t Psize, saidx_t pos,
              const searchoption_t *option) {
  saidx_t i;
  saidx_t a, b, c, d;

  a = (option->blen < pos) ? pos - option->blen : 0;
  b = pos;
  c = pos + Psize;
  d = ((c + option->alen) < Tsize) ? c + option->alen : Tsize;

  if(option->flags & SA_PRINT_FILENAME) { printf("%s:", option->fname); }
  if(option->flags & SA_PRINT_OFFSET) { printf("%d:", a); }

  if(option->flags & SA_HEX_MODE) {
    for(i = a; i < (d - 1); ++i) {
      printf("%02x ", T[i]);
    }
    printf("%02x\n", T[d - 1]);
  } else {
    for(i = a; i < d; ++i) {
      switch(T[i]) {
        case '\n': printf("[\\n]"); break;
        case '\r': printf("[\\r]"); break;
        default: printf("%c", T[i]);
      }
    }
    printf("\n");
  }
}

static
void
_onlyprint_count(const sauchar_t *T, saidx_t Tsize,
                 const sauchar_t *P, saidx_t Psize,
                 const saidx_t *SA, saidx_t SAsize,
                 saidx_t left, saidx_t size, searchoption_t *option) {
  if(0 < size) {
    if(option->flags & SA_PRINT_FILENAME) { printf("%s:", option->fname); }
    printf("%d\n", size);
  }
}

static
int
_idx_cmp(const void *p1, const void *p2) {
  saidx_t i1 = *((saidx_t *)p1), i2 = *((saidx_t *)p2);
  if(i1 < i2) { return -1; }
  if(i1 > i2) { return  1; }
  return 0;
/*  return i1 - i2; */
}

static
void
_print_suffixes(const sauchar_t *T, saidx_t Tsize,
                const sauchar_t *P, saidx_t Psize,
                const saidx_t *SA, saidx_t SAsize,
                saidx_t left, saidx_t size, searchoption_t *option) {
  saidx_t *ary;
  saidx_t i;

  if(option->flags & SA_SORT_LEXICOGRAPHICALORDER) {
    for(i = 0; i < size; ++i) {
      _print_suffix(T, Tsize, Psize, SA[left + i], option);
    }
  } else {
    ary = malloc(size * sizeof(saidx_t));
    memcpy(ary, SA + left, size * sizeof(saidx_t));
    qsort(ary, size, sizeof(saidx_t), _idx_cmp);
    for(i = 0; i < size; ++i) {
      _print_suffix(T, Tsize, Psize, ary[i], option);
    }
    free(ary);
  }
}

static
void
_search_file(const sauchar_t *T, saidx_t Tsize,
             const sauchar_t *P, saidx_t Psize,
             const saidx_t *SA, saidx_t SAsize,
             searchoption_t *option) {
  saidx_t size, left;
  size = sa_search(T, Tsize, P, Psize, SA, SAsize, &left);
  if(0 <= option->maxcount) {
    if(option->maxcount == 0) { return; }
    if(option->maxcount < size) { size = option->maxcount; }
  }
  option->func(T, Tsize, P, Psize, SA, SAsize, left, size, option);
}

static
void
_print_version(const char *pname) {
  fprintf(stderr,
"%s, a SA-based full-text search tool, version %s\n\n",
    pname, divsufsort_version());
}

static
void
_print_usage(const char *pname, int status) {
  _print_version(pname);
  fprintf(stderr,
"usage: %s [OPTION]... PATTERN FILE SAFILE\n"
"\nOutput control:\n"
"  -m NUM         stop after NUM matches\n"
"  -b             print the byte offset\n"
"  -H             print the filename\n"
"  -c             only print a count of matches\n"
"  -S             sort in lexicographical order\n"
"\nContext control:\n"
"  -B NUM         print NUM characters of leading context\n"
"  -A NUM         print NUM characters of trailing context\n"
"  -C NUM         print NUM characters of output context\n"
"\nMiscellaneous:\n"
"  -h             print this message\n"
"  -v             display version number\n"
"\n", pname);
  exit(status);
}

static
void
_print_tryhelp(const char *pname, int status) {
  fprintf(stderr, "Try `%s --help' for more information.\n", pname);
  exit(status);
}

static
void
_print_version_and_license(const char *pname, int status) {
  _print_version(pname);
  fprintf(stderr,
    "   Copyright (c) 2003-2007 Yuta Mori All Rights Reserved.\n"
    "\n"
    "   Permission is hereby granted, free of charge, to any person\n"
    "   obtaining a copy of this software and associated documentation\n"
    "   files (the \"Software\"), to deal in the Software without\n"
    "   restriction, including without limitation the rights to use,\n"
    "   copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
    "   copies of the Software, and to permit persons to whom the\n"
    "   Software is furnished to do so, subject to the following\n"
    "   conditions:\n"
    "\n"
    "   The above copyright notice and this permission notice shall be\n"
    "   included in all copies or substantial portions of the Software.\n"
    "\n"
    "   THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
    "   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES\n"
    "   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\n"
    "   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT\n"
    "   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,\n"
    "   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n"
    "   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR\n"
    "   OTHER DEALINGS IN THE SOFTWARE.\n");
  exit(status);
}


int
main(int argc, const char *argv[]) {
  int i;
  searchoption_t option;
  sauchar_t *P;
  saidx_t Psize;

  if(argc <= 1) { _print_usage(argv[0], EXIT_SUCCESS); }

  option.maxcount = -1;
  option.fname = NULL;
  option.flags = 0;
  option.func = _print_suffixes;
  option.alen = option.blen = 10;

  for(i = 1; i < argc; ++i) {
    if(argv[i][0] != '-') { break; }

    if((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
      _print_usage(argv[0], EXIT_SUCCESS);
    }
    if((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--version") == 0)) {
      _print_version_and_license(argv[0], EXIT_SUCCESS);
    }

    if(strcmp(argv[i],"-m")==0) {
      if((i + 1) == argc) {
        fprintf(stderr,"%s: option requires an argument -- %s\n", argv[0], argv[i]);
        _print_tryhelp(argv[0], EXIT_FAILURE);
      }
      option.maxcount = atoi(argv[++i]);
    } else if(strcmp(argv[i],"-b")==0) {
      option.flags |= SA_PRINT_OFFSET;
    } else if(strcmp(argv[i],"-H")==0) {
      option.flags |= SA_PRINT_FILENAME;
    } else if(strcmp(argv[i],"-c")==0) {
      option.func = _onlyprint_count;
    } else if(strcmp(argv[i],"-S")==0) {
      option.flags |= SA_SORT_LEXICOGRAPHICALORDER;
    } else if(strcmp(argv[i],"-B")==0) {
      if((i + 1) == argc) {
        fprintf(stderr,"%s: option requires an argument -- %s\n", argv[0], argv[i]);
        _print_tryhelp(argv[0], EXIT_FAILURE);
      }
      option.blen = atoi(argv[++i]);
    } else if(strcmp(argv[i],"-A")==0) {
      if((i + 1) == argc) {
        fprintf(stderr,"%s: option requires an argument -- %s\n", argv[0], argv[i]);
        _print_tryhelp(argv[0], EXIT_FAILURE);
      }
      option.alen = atoi(argv[++i]);
    } else if(strcmp(argv[i],"-C")==0) {
      if((i + 1) == argc) {
        fprintf(stderr,"%s: option requires an argument -- %s\n", argv[0], argv[i]);
        _print_tryhelp(argv[0], EXIT_FAILURE);
      }
      option.alen = option.blen = atoi(argv[++i]);

    } else if(strcmp(argv[i],"--hex")==0) {
      option.flags |= SA_HEX_MODE;

    } else {
      fprintf(stderr,"%s: invalid option -- %s\n", argv[0], argv[i]);
      _print_tryhelp(argv[0], EXIT_FAILURE);
    }
  }

  if(i == argc) { return 0; }

  P = (sauchar_t *)argv[i];
  Psize = (saidx_t)strlen(argv[i]);

  if(option.flags & SA_HEX_MODE) {
    sauchar_t *newP = malloc(Psize / 2 * sizeof(sauchar_t));
    saidx_t j, k;
    unsigned char c, t;
    if(newP == NULL) {
      fprintf(stderr, "%s: Cannot allocate memory.\n", argv[0]);
      exit(EXIT_FAILURE);
    }
    for(j = 0, k = 0, c = 0; j < Psize; ++j) {
      if((('0' <= P[j]) && (P[j] <= '9')) || (('a' <= P[j]) && (P[j] <= 'f'))) {
        t = (('0' <= P[j]) && (P[j] <= '9')) ? P[j] - '0' : (P[j] - 'a' + 10);
        c = (c << 4) | t;
        if(k & 1) { newP[k / 2] = c; }
        k += 1;
      }
    }
    Psize = k / 2;
    P = newP;
  }

  for(i += 1; (i + 1) < argc; i += 2) {
    FILE *fp;
    sauchar_t *T;
    saidx_t *SA;
    saidx_t size;

#if HAVE_SYS_STAT_H
    struct stat s;
    if(stat(argv[i], &s) != 0) {
      fprintf(stderr, "%s: Cannot stat file `%s': ", argv[0], argv[i]);
      perror(NULL);
      exit(EXIT_FAILURE);
    }
    size = s.st_size;
#endif

    option.fname = argv[i];
    /* Open a file for reading. */
    if((fp = fopen(argv[i], "rb")) == NULL) {
      fprintf(stderr, "%s: Cannot open file `%s': ", argv[0], argv[i]);
      perror(NULL);
      exit(EXIT_FAILURE);
    }
#if !HAVE_SYS_STAT_H
    if(fseek(fp, 0, SEEK_END) != 0) {
      fprintf(stderr, "%s: Cannot fseek on `%s': ", argv[0], argv[i]);
      perror(NULL);
      exit(EXIT_FAILURE);
    }
    if((size = ftell(fp)) == -1) {
      fprintf(stderr, "%s: Cannot ftell on `%s': ", argv[0], argv[i]);
      perror(NULL);
      exit(EXIT_FAILURE);
    }
    rewind(fp);
#endif

    /* Allocate n+4(n+1) bytes of memory. */
    if(((T = malloc(size * sizeof(sauchar_t))) == NULL) ||
       ((SA = malloc((size + 1) * sizeof(saidx_t))) == NULL)) {
      fprintf(stderr, "%s: Cannot allocate memory.\n", argv[0]);
      exit(EXIT_FAILURE);
    }

    /* Read n * sizeof(sauchar_t) bytes of data. */
    if(fread(T, sizeof(sauchar_t), size, fp) != size) {
      fprintf(stderr, "%s: %s `%s': ",
        argv[0],
        (ferror(fp) || !feof(fp)) ? "Cannot read from" : "Unexpected EOF in",
        argv[i]);
      perror(NULL);
      exit(EXIT_FAILURE);
    }
    fclose(fp);

    /* Open a SA file for reading. */
    if((fp = fopen(argv[i + 1], "rb")) == NULL) {
      fprintf(stderr, "%s: Cannot open file `%s': ", argv[0], argv[i + 1]);
      perror(NULL);
      exit(EXIT_FAILURE);
    }
    /* Read (n + 1) * sizeof(saidx_t) bytes of data. */
    if(fread(SA, sizeof(saidx_t), size + 1, fp) != (size + 1)) {
      fprintf(stderr, "%s: %s `%s': ",
        argv[0],
        (ferror(fp) || !feof(fp)) ? "Cannot read from" : "Unexpected EOF in",
        argv[i + 1]);
      perror(NULL);
      exit(EXIT_FAILURE);
    }
    fclose(fp);

    _search_file(T, size, P, Psize, SA, size + 1, &option);

    free(T);
    free(SA);
  }

  if(option.flags & SA_HEX_MODE) {
    free(P);
  }

  return 0;
}
