#include <iostream>
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
 *  9 reserved for multiple encodings
 * 10 decimal, with 14 punctuation marks
 * 11 decimal in different scripts
 * 12-31 unassigned
 * 
 * The two Unicode packings differ only in the encoding of half-fronted Unicode
 * points, so run the text (in UTF-8) through the half-fronting algorithm
 * to get a wide string before encoding.
 * 
 * The various encoding functions return encoded text if the text can be so encoded,
 * else a null string, along with an integer denoting the encoding.
 */

using namespace std;

encoded encode32(string text)
{
  encoded code;
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

encoded encodedecimal(string text)
{
  encoded code;
  string chr24("0123456789 #()*+,-./<=>@");
  string digits("0123456789");
  string dig3,let2;
  size_t charcode;
  while (text.length())
  {
    dig3=text.substr(0,3);
    if (dig3.length()==3 && dig3.find_first_not_of(digits)==string::npos)
      charcode=100*dig3[0]+10*dig3[1]+dig3[2]-111*'0';
    else
    {
      charcode=chr24.find(dig3[0]);
      if (charcode!=string::npos)
	charcode+=1000;
    }
    if (charcode==string::npos)
      code.codestring=text="";
    else
    {
      let2=charcode/32+'@';
      let2+=charcode%32+'@';
      code.codestring+=let2;
      text.erase(0,(charcode<1000)?3:1);
    }
  }
  code.encoding=10;
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
  list.push_back(encode32(text));
  sort1(list);
  list.push_back(encodedecimal(text));
  sort1(list);
  return list;
}

void dumpenc(vector<encoded> encodedlist)
{
  int i;
  for (i=0;i<encodedlist.size();i++)
    cout<<encodedlist[i].encoding<<" "<<encodedlist[i].codestring<<endl;
}
