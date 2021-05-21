#include <iostream>
#include <cstdio>
#include <cstring>
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
 * A null string is encoded as a single at sign in decimal code; since all strings
 * of digits in decimal code are encoded as pairs of letters, the decoder returns
 * a null string.
 * 
 * The two Unicode packings differ only in the encoding of half-fronted Unicode
 * points, so run the text (in UTF-8) through the half-fronting algorithm
 * to get a wide string before encoding.
 * 
 * The various encoding functions return encoded text if the text can be so encoded,
 * else a null string, along with an integer denoting the encoding.
 */

using namespace std;
const string chr24("0123456789 #()*+,-./<=>@");
const string digits("0123456789");

wstring fromutf8(string utf8)
{
  wstring wide;
  string seq;
  wchar_t wch;
  int ch,clen,wclen,i;
  bool err=false;
  while (utf8.length())
  {
    ch=utf8[0]&0xff;
    clen=(ch>0x7f)+(ch>0xbf)+(ch>0xdf)+(ch>0xef)+(ch>0xf7)+(ch>0xfb)+(ch>0xfd);
    if (clen<2)
      clen=1-clen;
    switch (clen)
    {
      case 0: // sequence starts with a continuation byte
      case 7: // sequence starts with 0xfe or 0xff
        wide.clear();
	utf8.clear();
	break;
      default:
	seq=utf8.substr(0,clen);
	utf8.erase(0,clen);
	seq[0]&=(1<<(8-clen))-1;
	for (wch=i=0;i<clen;i++)
	{
	  wch=(wch<<6)|(seq[i]&0x7f);
	  if (i>0 && (seq[i]&0xc0)!=0x80)
	    err=true;
	}
	wide+=wch;
    }
    wclen=(wch>-1)+(wch>0x7f)+(wch>0x7ff)+(wch>0xffff)+(wch>0x1fffff)+(wch>0x3ffffff);
    if (err || wclen!=clen)
    {
      wide.clear();
      utf8.clear();
    }
  }
  return wide;
}

string toutf8(wstring wide)
{
  string utf8,seq;
  wchar_t wch;
  char ch,clen,wclen,i;
  while (wide.length())
  {
    wch=wide[0];
    wclen=(wch>-1)+(wch>0x7f)+(wch>0x7ff)+(wch>0xffff)+(wch>0x1fffff)+(wch>0x3ffffff);
    switch (wclen)
    {
      case 0: // character is negative
        wide.clear();
	utf8.clear();
	break;
      case 1:
	utf8+=wch;
	wide.erase(0,1);
	break;
      default:
	seq="";
	wide.erase(0,1);
	for (i=0;i<wclen;i++)
	{
	  ch=(wch&0x3f)+0x80;
	  if (i==wclen-1)
	    ch=wch|-(1<<(8-wclen));
	  seq=ch+seq;
	  wch>>=6;
	}
	utf8+=seq;
    }
  }
  return utf8;
}

wstring halffront(wstring text)
{
  wstring ftext;
  short lowtable[1024],midtable[2048],hightable[1024];
  int i,l,m,h,lp,mp,hp;
  for (i=0;i<1024;i++)
  {
    lowtable[i]=midtable[i]=hightable[i]=i;
    midtable[i+1024]=i+1024;
  }
  while (text.length())
  {
    l=text[0]&1023;
    m=(text[0]>>10)&2047;
    h=(text[0]>>21)&1023;
    for (lp=0;lowtable[lp]!=l;lp++);
    for (mp=0;midtable[mp]!=m;mp++);
    for (hp=0;hightable[hp]!=h;hp++);
    memmove(lowtable+(lp/2)+1,lowtable+(lp/2),((lp+1)/2)*sizeof(short));
    lowtable[lp/2]=l;
    memmove(midtable+(mp/2)+1,midtable+(mp/2),((mp+1)/2)*sizeof(short));
    midtable[mp/2]=m;
    memmove(hightable+(hp/2)+1,hightable+(hp/2),((hp+1)/2)*sizeof(short));
    hightable[hp/2]=h;
    text.erase(0,1);
    ftext+=(wchar_t)((hp<<21)|(mp<<10)|lp);
  }
  return ftext;
}

wstring unhalffront(wstring ftext)
{
  wstring text;
  short lowtable[1024],midtable[2048],hightable[1024];
  int i,l,m,h,lp,mp,hp;
  for (i=0;i<1024;i++)
  {
    lowtable[i]=midtable[i]=hightable[i]=i;
    midtable[i+1024]=i+1024;
  }
  while (ftext.length())
  {
    lp=ftext[0]&1023;
    mp=(ftext[0]>>10)&2047;
    hp=(ftext[0]>>21)&1023;
    l=lowtable[lp];
    m=midtable[mp];
    h=hightable[hp];
    memmove(lowtable+(lp/2)+1,lowtable+(lp/2),((lp+1)/2)*sizeof(short));
    lowtable[lp/2]=l;
    memmove(midtable+(mp/2)+1,midtable+(mp/2),((mp+1)/2)*sizeof(short));
    midtable[mp/2]=m;
    memmove(hightable+(hp/2)+1,hightable+(hp/2),((hp+1)/2)*sizeof(short));
    hightable[hp/2]=h;
    ftext.erase(0,1);
    text+=(wchar_t)((h<<21)|(m<<10)|l);
  }
  return text;
}

encoded encodeu1(wstring text)
{
  encoded code;
  wstring ch2;
  int n0,n1,i;
  while (text.length())
  {
    ch2=text.substr(0,2);
    n0=1+(ch2[0]>31)+(ch2[0]>1024)+(ch2[0]>32767)+(ch2[0]>1048575)+(ch2[0]>33554431);
    n1=1+(ch2[1]>31)+(ch2[1]>1024)+(ch2[1]>32767)+(ch2[1]>1048575)+(ch2[1]>33554431);
    if (ch2.length()<2)
      n1=0;
    code.codestring+=(n0-1)*6+n1+'@';
    for (i=n0-1;i>=0;i--)
      code.codestring+=((ch2[0]>>(5*i))&31)+'@';
    for (i=n1-1;i>=0;i--)
      code.codestring+=((ch2[1]>>(5*i))&31)+'@';
    text.erase(0,2);
    if (n0>5 || n1>5)
    {
      code.codestring.clear();
      text.clear();
    }
  }
  code.encoding=1;
  return code;
}

wstring decodeu1(string text)
{
  wstring plain;
  wchar_t wch;
  int n0,n1,i;
  bool err=false;
  while (text.length())
  {
    n0=(text[0]&31)/6+1;
    n1=(text[0]&31)%6;
    text.erase(0,1);
    if (n0>5)
      err=true;
    for (i=wch=0;i<n0;i++)
    {
      if (i==0 && n0>1 && text[0]<='@') // no leading zeros except when writing zero
	err=true;
      if (text[0]<'@')
	err=true;
      wch=(wch<<5)|(text[0]&31);
      text.erase(0,1);
    }
    plain+=wch;
    for (i=wch=0;i<n1;i++)
    {
      if (i==0 && n1>1 && text[0]<='@') // no leading zeros except when writing zero
	err=true;
      if (text[0]<'@')
	err=true;
      wch=(wch<<5)|(text[0]&31);
      text.erase(0,1);
    }
    if (n1)
      plain+=wch;
    if (err)
    {
      text.clear();
      plain.clear();
    }
  }
  return plain;
}

encoded encodeu2(wstring text)
{
  encoded code;
  wchar_t wch;
  int n,i;
  while (text.length())
  {
    wch=text[0];
    for (n=1;n<8 && (wch>>(4*n));n++);
    for (i=n-1;i>=0;i--)
      code.codestring+=((wch>>(4*i))&15)+16*(i==0)+'@';
    text.erase(0,1);
  }
  code.encoding=2;
  return code;
}

wstring decodeu2(string text)
{
  wstring plain;
  wchar_t wch=0;
  int n=0;
  bool err=false;
  while (text.length())
  {
    wch=(wch<<4)+(text[0]&15);
    n++;
    if (text[0]&16)
    {
      plain+=wch;
      wch=n=0;
    }
    text.erase(0,1);
    if (n>8)
      err=true;
    if (err)
    {
      text.clear();
      plain.clear();
    }
  }
  return plain;
}

void atspace(string &text)
{
  int i;
  for (i=0;i<text.length();i++)
    if (text[i]==' ' || text[i]=='@')
      text[i]^=(' '^'@');
}

encoded encode32(string text)
{
  encoded code;
  atspace(text);
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

string decode32(string text)
{
  atspace(text);
  return text;
}

encoded encodedecimal(string text)
{
  encoded code;
  string dig3,let2;
  size_t charcode;
  if (text=="")
    code.codestring="@";
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

string decodedecimal(string text)
{
  string plain;
  int charcode;
  while (text.length())
  {
    if (text.length()==1)
      plain="";
    else
    {
      charcode=text[0]*32+text[1]-33*'@';
      if (charcode<1000)
      {
	plain+=charcode/100+'0';
	charcode%=100;
	plain+=charcode/10+'0';
	charcode%=10;
	plain+=charcode+'0';
      }
      else
	plain+=chr24[charcode-1000];
    }
    text.erase(0,2);
  }
  return plain;
}

encoded encodeascii(string text)
{
  encoded code;
  int charcode=0,nbits=0;
  while (text.length())
  {
    charcode<<=7;
    charcode+=text[0]&0x7f;
    if (text[0]&0x80)
    {
      text=code.codestring="";
      nbits=-7;
      charcode=0;
    }
    text.erase(0,1);
    nbits+=7;
    while (nbits>=5)
    {
      code.codestring+=(charcode>>(nbits-5))+'@';
      nbits-=5;
      charcode&=(1<<nbits)-1;
    }
  }
  if (nbits)
    code.codestring+=(charcode<<(5-nbits))+'@';
  code.encoding=7;
  return code;
}

string decodeascii(string text)
{
  string plain;
  int charcode=0,nbits=0;
  while (text.length())
  {
    charcode<<=5;
    charcode+=text[0]&0x1f;
    text.erase(0,1);
    nbits+=5;
    while (nbits>=7)
    {
      plain+=charcode>>(nbits-7);
      nbits-=7;
      charcode&=(1<<nbits)-1;
    }
  }
  return plain;
}

encoded encodebyte(string text)
{
  encoded code;
  int charcode=0,nbits=0;
  while (text.length())
  {
    charcode<<=8;
    charcode+=text[0]&0xff;
    text.erase(0,1);
    nbits+=8;
    while (nbits>=5)
    {
      code.codestring+=(charcode>>(nbits-5))+'@';
      nbits-=5;
      charcode&=(1<<nbits)-1;
    }
  }
  if (nbits)
    code.codestring+=(charcode<<(5-nbits))+'@';
  code.encoding=8;
  return code;
}

string decodebyte(string text)
{
  string plain;
  int charcode=0,nbits=0;
  while (text.length())
  {
    charcode<<=5;
    charcode+=text[0]&0x1f;
    text.erase(0,1);
    nbits+=5;
    while (nbits>=8)
    {
      plain+=charcode>>(nbits-8);
      nbits-=8;
      charcode&=(1<<nbits)-1;
    }
  }
  return plain;
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
  wstring unitext;
  vector<encoded> list;
  encoded enc;
  unitext=halffront(fromutf8(text));
  enc=encodeu1(unitext);
  // At least one encoding produces a nonempty codetext.
  // If text is empty, encodedecimal returns "@"; else encodebyte returns nonempty.
  if (enc.codestring.length())
    list.push_back(enc);
  sort1(list);
  enc=encodeu2(unitext);
  if (enc.codestring.length())
    list.push_back(enc);
  sort1(list);
  enc=encode32(text);
  if (enc.codestring.length())
    list.push_back(enc);
  sort1(list);
  enc=encodeascii(text);
  if (enc.codestring.length())
    list.push_back(enc);
  sort1(list);
  enc=encodebyte(text);
  if (enc.codestring.length())
    list.push_back(enc);
  sort1(list);
  enc=encodedecimal(text);
  if (enc.codestring.length())
    list.push_back(enc);
  sort1(list);
  return list;
}

string decode(encoded ciphertext)
{
  wstring uniplain;
  string plain;
  switch (ciphertext.encoding)
  {
    case 1:
      uniplain=decodeu1(ciphertext.codestring);
      plain=toutf8(unhalffront(uniplain));
      break;
    case 2:
      uniplain=decodeu2(ciphertext.codestring);
      plain=toutf8(unhalffront(uniplain));
      break;
    case 5:
      plain=decode32(ciphertext.codestring);
      break;
    case 7:
      plain=decodeascii(ciphertext.codestring);
      break;
    case 8:
      plain=decodebyte(ciphertext.codestring);
      break;
    case 10:
      plain=decodedecimal(ciphertext.codestring);
      break;
    default:
      cerr<<"unimplemented code "<<ciphertext.encoding<<endl;
  }
  return plain;
}

void dumpenc(vector<encoded> encodedlist)
{
  int i;
  for (i=0;i<encodedlist.size();i++)
    cout<<encodedlist[i].encoding<<" "<<encodedlist[i].codestring<<endl;
}

void testenc1(string text)
{
  vector<encoded> list;
  int i;
  cout<<"Original: "<<text<<endl;
  list=encodedlist(text);
  for (i=0;i<list.size();i++)
    cout<<list[i].encoding<<" "<<list[i].codestring<<" "<<decode(list[i])<<endl;
}

void testenc()
{
  wstring wide;
  int i;
  wide=fromutf8("Πρόπολις");
  for (i=0;i<wide.length();i++)
    printf("%x ",wide[i]);
  cout<<endl<<toutf8(wide)<<endl;
  wide=fromutf8("\300\201"); // overlong encoding
  for (i=0;i<wide.length();i++)
    printf("%x ",wide[i]);
  cout<<endl<<toutf8(wide)<<endl;
  wide=fromutf8("\302\302"); // invalid sequence
  for (i=0;i<wide.length();i++)
    printf("%x ",wide[i]);
  cout<<endl<<toutf8(wide)<<endl;
  wide=fromutf8("\345\202"); // truncated sequence
  for (i=0;i<wide.length();i++)
    printf("%x ",wide[i]);
  cout<<endl<<toutf8(wide)<<endl;
  wide=fromutf8("örült az őrült");
  for (i=0;i<wide.length();i++)
    printf("%x ",wide[i]);
  cout<<endl<<toutf8(wide)<<endl;
  testenc1("");
  testenc1("慮畫搠楥樠湵潧瘠污楳吠敨敳愠敲渠瑯䌠楨敮敳眠牯獤"); // "naku dei jungo valsi These are not Chinese words" converted from utf-16le
  testenc1("PROPOLIS");
  testenc1("propolis");
  testenc1("Πρόπολις");
  testenc1("0588235294117647");
}
