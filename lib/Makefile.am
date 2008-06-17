# Makefile.am for libdivsufsort

lib_LTLIBRARIES = libdivsufsort.la
libdivsufsort_la_SOURCES = divsufsort.c substringsort.c trsort.c utils.c
libdivsufsort_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@ @LT_NOUNDEF@ -export-symbols libdivsufsort.sym

AM_CFLAGS = @A_CFLAGS@

EXTRA_DIST = libdivsufsort.sym
