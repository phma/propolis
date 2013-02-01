#include <string>
#include <vector>
#include "hvec.h"

using namespace std;

int ndataletters(int n);
void listsizes();
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
  vector<row> rows;
  int size,nletters,nrows,ndata,leftover;
public:
  void setsize(int sz);
  int getsize();
  void setndata(int nd);
  void setdata(string str,int encoding); // str should consist of letters @ through _; encoding should be 1-31
  void scramble();
  void unscramble();
  void encode();
  void dump();
  void arrange(harray<char> &hletters);
};

extern codematrix thematrix;
void testbitctrot();
void testshuffle();
void testwhiten();
