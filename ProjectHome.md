The libdivsufsort project provides a fast, lightweight, and robust C API library to construct the [suffix array](http://en.wikipedia.org/wiki/Suffix_array) and the [Burrows-Wheeler transformed string](http://en.wikipedia.org/wiki/Burrows-Wheeler_transform) for any input string of a constant-size alphabet.

## News: ##
  * **2008/08/23**: libdivsufsort-2.0.0 is now available.
  * **2008/06/09**: The project has been moved to Google Code.

## Current Features: ##
  * Constructs the suffix array in O(n log n) time and O(1) extra working space.
  * Constructs the Burrows-Wheeler transformed string directly.
  * Inverse BW-transform.
  * Fast suffix array checker.
  * Simple string search.

## Planned Features: ##
  * Lightweight BWT and Inverse BWT. (3n? bytes)
  * Regular expression search...?