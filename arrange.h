#include <string>
#include <vector>
#include "hvec.h"
#include "hamming.h"

int ndataletters(int n);
void listsizes();
int findsize(int n,double redundancy);
void testfindsize();

class row
/* A row consists of three parts:
 * unwritten zeros, stored as spaces;
 * written data letters, stored as letters from @ to _;
 * written check letters, stored as letters from ` to delete.
 * When printing a row for debugging, a question mark is substituted for a delete.
 * This causes no confusion, as the only unwritten value (0x20-0x3f) is a space.
 */
{
public:
  char data[32]; // 31 letters and a null
  row();
  void setunwritten(int n);
  int getunwritten();
  void setndata(int n);
  int getndata();
  void printdelete();
  void restoredelete();
  void whiten(int rownum);
  void shuffle(int rownum);
  void scramble(int rownum);
  void unwhiten(int rownum);
  void unshuffle(int rownum);
  void unscramble(int rownum);
  void encode();
};

class codematrix
{
private:
  std::vector<row> rows;
  int size,nletters,nrows,ndata,leftover;
public:
  void setsize(int sz);
  int getsize();
  bool setndata(int nd,bool marrel=false); // returns true if successful
  void setdata(std::string str,int encoding); // str should consist of letters @ through _; encoding should be 1-31
  void scramble();
  void unscramble();
  void encode();
  void dump();
  void arrange(harray<char> &hletters);
};

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
};

extern codematrix thematrix;
extern CodeMatrix theMatrix;
void testbitctrot();
void testshuffle();
void testwhiten();
void testCheckLetters();
