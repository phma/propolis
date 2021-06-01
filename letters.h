#ifndef LETTERS_H
#define LETTERS_H
#include <array>
#include <map>
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
void drawletter(int letter,hvec place,harray<char> &canvas=hbits);
void fillinvletters();
void readinvletters();
void writeinvletters();
void checkinvletters();
void testroundframe();
void testrotate();
void debugframingerror();
void writeAmbig();
void fillLetters(int perm,int negs,int splay,int twist);
void findLetterAssignment();

struct InvLetterTask
{
  harray<char> hletters;
  int i,j,k,l;
};

struct InvLetterResult
/* Used by threads to return the result of all framings of
 * one combination of four letters.
 */
{
  std::map<int,sixvec> torus;
  int suminv;
  int i,j,k,l;
};

class litteron // blend of "littera" and "neuron"
{
private:
  std::array<unsigned char,32> letterprob;
  std::array<signed char,5> softbits;
public:
  void setprob(std::array<signed char,12> seen,int index);
  void setprob(short seen,int index);
  void propagate(std::array<signed char,5> belief);
  signed char operator[](int n);
};

InvLetterResult shiftFrame(harray<char> hletters,int i,int j,int k,int l);

#endif
