#include <string>
#include <vector>
#include "hvec.h"
#include "hamming.h"

int ndataletters(int n);
void listsizes();
int findsize(int n,double redundancy);
void testfindsize();

class CodeMatrix
{
private:
  std::vector<Hamming> hammingBlocks;
  std::vector<int> hammingSizes;
  std::vector<char> metadata; // 6 or 7 letters, depending on size
  std::vector<char> data; // rearranged by criss-crossing
  int size,nLetters,nData,nDataCheck;
  /* Consider a size-3 symbol holding "1.618033988749894848204" in decimal
   * encoding. This is encoded as "_I_ZSJAA^\WM[^ZPFL". There must be a
   * check-count letter, so this takes at least 19 letters. The next space
   * available within size 3 is 20, so there is one check-padding letter.
   * The letters are arranged in the Hamming blocks as follows:
   * cc_cI_ZcSJAA^ cc\cWM[c^ZPF ccLc** where the first * is check-padding
   * and the second is check-count.
   * size=3; nLetters=31; nData=18; nDataCheck=20;
   */
public:
  int getSize();
  int getNLetters();
  int getNData();
  int getNDataCheck();
  double getRedundancy();
  int findSize(int n,double redundancy);
  void setDataCheck(std::string str,int encoding); // str should consist of letters @ through _; encoding should be 1-31
  void setData(std::string str,int encoding);
  void dump();
  void arrange(harray<char> &hletters);
};

extern CodeMatrix theMatrix;
void testbitctrot();
void testshuffle();
void testwhiten();
void testCheckLetters();
