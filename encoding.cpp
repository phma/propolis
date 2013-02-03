#include "encoding.h"

/*  0 Test pattern, do not decode. Not all test patterns use metadata.
 *  1 Unicode packing mode 1
 *  2 Unicode packing mode 2
 *  3 unassigned, possibly octal
 *  4 unassigned, possibly hex
 *  5 letters @ to _ (i.e. no encoding)
 *  6 base-64 code?
 *  7 ASCII
 *  8 bytes
 *  9 unassigned
 * 10 decimal, with 14 punctuation marks
 * 11-31 unassigned
 * 
 * The two Unicode packings differ only in the encoding of half-fronted Unicode
 * points, so run the text (int UTF-8) through the half-fronting algorithm
 * to get a wide string before encoding.
 */
