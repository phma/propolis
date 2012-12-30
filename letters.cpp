/* Alphabet of 32 letters for hexagonal code
 *     * *
 *    * * *
 *   * * * *
 *    * * *
 * Each letter is a 12-bit code for a 5-bit plaintext. They are read
 * with bit 11 at the top and bit 0 at the bottom right. Rotating a
 * codeword 120° and flipping all bits do consistent things to plaintext,
 * but neither rotating nor changing two bits (the Hamimng distance is 2)
 * does anything related to a Reed-Solomon code.
 * 
 * Codes for single 1 bits are:
 *  00001   00010   00100   01000   10000
 *   0 0     1 1     1 1     1 0     0 1
 *  0 0 0   1 1 1   0 0 0   1 0 0   0 0 1
 * 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 1 1
 *  1 1 1   0 0 0   0 0 0   1 0 0   0 0 1
 * Rotating a code does this:
 *  00100   01010   10001
 *   1 1     0 0     0 0
 *  0 0 0   0 0 0   0 0 0
 * 0 0 0 0 0 0 0 1 1 0 0 0
 *  0 0 0   0 0 1   1 0 0
 */
#include <cstdio>
#include <cstring>
#include "letters.h"
#include "hvec.h"

BIT16 letters[38]={
0x000, // 00000  00 000 0000 000
0x007, // 00001  00 000 0000 111
0xf80, // 00010  11 111 0000 000
0x01f, // 00011  00 000 0011 111
0xc00, // 00100  11 000 0000 000
0x09b, // 00101  00 001 0011 011
0xf40, // 00110  11 110 1000 000
0xd99, // 00111  11 011 0011 001
0xa64, // 01000  10 100 1100 100
0x067, // 01001  00 000 1100 111
0x009, // 01010  00 000 0001 001
0x277, // 01011  00 100 1110 111
0x488, // 01100  01 001 0001 000
0x5bf, // 01101  01 011 0111 111
0xfbb, // 01110  11 111 0111 011
0xb66, // 01111  10 110 1100 110
0x499, // 10000  01 001 0011 001
0x044, // 10001  00 000 1000 100
0xa40, // 10010  10 100 1000 000
0xb77, // 10011  10 110 1110 111
0xd88, // 10100  11 011 0001 000
0xff6, // 10101  11 111 1110 110
0xf98, // 10110  11 111 0011 000
0x59b, // 10111  01 011 0011 011
0x266, // 11000  00 100 1100 110
0x0bf, // 11001  00 001 0111 111
0xf64, // 11010  11 110 1100 100
0x3ff, // 11011  00 111 1111 111
0xfe0, // 11100  11 111 1100 000
0x07f, // 11101  00 000 1111 111
0xff8, // 11110  11 111 1111 000
0xfff, // 11111  11 111 1111 111
/*
// Side borders. The corner borders are 04, 12, 11, 01, 0a, 0c.
0xe40, //        11 100 1000 000
0xa44, //        10 100 1000 100
0x047, //        00 000 1000 111
0x00f, //        00 000 0001 111
0x489, //        01 001 0001 001
0xc88};//        11 001 0001 000
*/
// Side borders. The corner borders are 02, 1a, 18, 1d, 05, 07.
0xfe4, //        11 111 1100 100
0xf66, //        11 110 1100 110
0x27f, //        00 100 1111 111
0x0ff, //        00 001 1111 111
0xd9b, //        11 011 0011 011
0xf99};//        11 111 0011 001
BIT16 invletters[4096];
/* Inverse letter table format:
 * 1xxxxxyyyyyzzzzz a bit pattern that could be any of three letters
 * 010000xxxxxyyyyy a bit pattern that could be any of two letters
 * 00100000000xxxxx a bit pattern that differs by 1 bit from one valid letter
 * 00010000000xxxxx a bit pattern of a valid letter
 * 01100000dddddddd an undecodable bit pattern indicating a framing error
 *                  dddddddd is in the size-8 hexagon, 0 through 216
 * 0000000000000000 an undecodable bit pattern that counts as erasure in RS
 */ 

int bitcount(int n)
{n=((n&0xaaaaaaaa)>>1)+(n&0x55555555);
 n=((n&0xcccccccc)>>2)+(n&0x33333333);
 n=((n&0xf0f0f0f0)>>4)+(n&0x0f0f0f0f);
 n=((n&0xff00ff00)>>8)+(n&0x00ff00ff);
 n=((n&0xffff0000)>>16)+(n&0x0000ffff);
 return n;
 }

void degauss()
{int i,j,tmp,cont;
 BIT16 basis[32];
 for (i=0;i<32;i++)
     basis[i]=letters[i];
 do
   {cont=0;
    for (i=0;i<32;i++)
        {for (j=0;j<32;j++)
             if (i!=j)
                {tmp=basis[i]^basis[j];
	         //printf("tmp=%d basis[%d]=%d basis[%d]=%d\n",tmp,i,basis[i],j,basis[j]);
                 if (bitcount(tmp)<bitcount(basis[i]) || bitcount(tmp)<bitcount(basis[j]))
                    {if (bitcount(basis[i])<bitcount(basis[j]))
                        basis[j]=tmp;
		     else
   		        basis[i]=tmp;
		     cont=1;
		     }
	         if ((i<j)^(basis[i]<basis[j]))
   	            {tmp=basis[i];
	             basis[i]=basis[j];
	             basis[j]=tmp;
	             }
	         }
         }
    } while (cont);
 for (j=0;j<32;j++)
     printf("%2d %03x\n",j,basis[j]);
 }

void fillinvletters()
{int i,j,k,l,inv[4096],il,in;
 memset(inv,0,sizeof(inv));
 hvec disp;
 for (i=0;i<32;i++)
     {inv[letters[i]]=i+32;
      for (j=0;j<12;j++)
          {il=letters[i]^(1<<j);
           in=i+64;
           while (in&inv[il])
              in<<=8;
	   inv[il]|=in;
	   }
      }
 for (i=0;i<4096;i++)
     {if (inv[i]&0x400000)
         invletters[i]=((inv[i]&0x1f0000)>>6)|((inv[i]&0x1f00)>>3)|(inv[i]&0x1f)|0x8000;
      else if (inv[i]&0x4000)
        invletters[i]=((inv[i]&0x1f00)>>3)|(inv[i]&0x1f)|0x4000;
      else if (inv[i]&0x40)
        invletters[i]=(inv[i]&0x1f)|0x2000;
      else if (inv[i]&0x20)
        invletters[i]=(inv[i]&0x1f)|0x1000;
      if (invletters[i])
         printf("%03x: %d%c%c%c\n",i,(invletters[i]>>15)&1,((invletters[i]>>10)&31)+64,((invletters[i]>>5)&31)+64,((invletters[i]>>0)&31)+64);
      }
 for (i=0;i<32;i++)
     {hletters[0]=i;
      for (j=0;j<32;j++)
          {hletters[1]=hletters[-1]=j;
           for (k=0;k<32;k++)
               {hletters[hvec(0,1)]=hletters[hvec(0,-1)]=k;
                for (l=0;l<32;l++)
                    {hletters[hvec(1,1)]=hletters[hvec(-1,-1)]=l;
                     for (disp=start(8);disp.cont(8);disp.inc(8))
                         ;//TODO: read the displaced letter and put it into the table
		     }
		}
	   }
      }
 }
       