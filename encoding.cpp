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
 * 
 * The various encoding functions return encoded text if the text can be so encoded,
 * else a null string, along with an integer denoting the encoding.
 */

using namespace std;

encoded code32(string text)
{
  encoded code;
  int c;
  while (text.length())
  {
    if (text[0]>='@' && text[0]<='_')
    {
      code.codestring+=text[0];
      text.erase(0,1);
    }
    else
      code.codestring=text="";
  }
  code.encoding=5;
  return code;
}

void sort1(vector<encoded> &list)
{
  int i;
  i=list.size()-2;
  while (i>=0 && list[i].codestring.length()>list[i+1].codestring.length())
  {
    swap(list[i],list[i+1]);
    i--;
  }
}

vector<encoded> encodedlist(string text)
{
  vector<encoded> list;
  list.push_back(code32(text));
  sort1(list);
  return list;
}
