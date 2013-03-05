#include "propolis.h"
#include "hvec.h"
extern BIT16 letters[38];
/* The extra six letters are for borders.
 *     11212112
 *    22      20
 *   22        20
 *  01          04
 *   23        25
 *    23      25
 *     0a24240c
 */
extern int debugletters;
extern BIT16 invletters[4096];
extern const hvec twelve[];
#define FRAMERAD 25
#define FRAMESIZE (FRAMERAD*(FRAMERAD+1)*3+1)
// Possibly FRAMERAD should be 18 (almost 1 frame in each tiniest region) or 25 (1951, a little less than 2048).
#define SLIVER_CENTROID 0.447545911917060126823164181486
// See calculation in hvec.cpp

void degauss();
void drawletter(int letter,hvec place);
void fillinvletters();
void readinvletters();
void writeinvletters();
void checkinvletters();
void testroundframe();
void testrotate();
void debugframingerror();
