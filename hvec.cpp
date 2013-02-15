/* Hexagonal vector (Eisenstein or Euler integers) and array of bytes subscripted by hexagonal vector
 */

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include "hvec.h"
#include "ps.h"

//Page size for storing arrays subscripted by hvec
/* A page looks like this:
 *       * * * * * *
 *      * * * * * * *
 *     * * * * * * * *
 *    * * * * * * * * *
 *   * * * * * * * * * *
 *  * * * * * * * * * * *
 *   * * * * * * * * * *
 *    * * * * * * * * *
 *     * * * * * * * *
 *      * * * * * * *
 *       * * * * * *
 */
const hvec PAGEMOD(PAGERAD+1,2*PAGERAD+1);
const hvec LETTERMOD(-2,-4);
int debughvec;

int hvec::numx,hvec::numy,hvec::denx=0,hvec::deny=0,hvec::quox,hvec::quoy,hvec::remx,hvec::remy;

unsigned long _norm(int x,int y)
{return sqr(x)+sqr(y)-x*y;
 }

hvec::hvec(complex<double> z)
{
  double norm0,norm1;
  y=lrint(z.imag()/M_SQRT_3_4);
  x=lrint(z.real()+y*0.5);
  norm0=::norm(z-(complex<double> (*this)));
  y++;
  norm1=::norm(z-(complex<double> (*this)));
  if (norm1>norm0)
    y--;
  else
    norm0=norm1;
  y--,x--;
  norm1=::norm(z-(complex<double> (*this)));
  if (norm1>norm0)
    y++,x++;
  else
    norm0=norm1;
  /*x++;
  norm1=::norm(z-(complex<double> (*this)));
  if (norm1>norm0)
    x--;
  else
    norm0=norm1;*/
  y--;
  norm1=::norm(z-(complex<double> (*this)));
  if (norm1>norm0)
    y++;
  else
    norm0=norm1;
  y++,x++;
  norm1=::norm(z-(complex<double> (*this)));
  if (norm1>norm0)
    y--,x--;
  else
    norm0=norm1;
  /*x--;
  norm1=::norm(z-(complex<double> (*this)));
  if (norm1>norm0)
    x++;
  else
    norm0=norm1;*/
}

void hvec::divmod(hvec b)
/* Division and remainder, done together to save time
 * 1     denx       deny
 * 1+ω   denx-deny  denx
 * ω     -deny      denx-deny
 * -1    -denx      -deny
 * -1-ω  deny-denx  -denx
 * -ω    deny       deny-denx
 */
{int cont;
 if (this->x!=numx || this->y!=numy || b.x!=denx || b.y!=deny)
    {int nrm,nrm1;
     numx=this->x;
     numy=this->y;
     denx=b.x;
     deny=b.y;
     nrm=b.norm();
     if (debughvec)
        printf("%d+%dω/%d+%dω\n",numx,numy,denx,deny);
     // Do a rough division.
     quox=round((numx*denx+numy*deny-numx*deny)/(double)nrm);
     quoy=round((numy*denx-numx*deny)/(double)nrm);
     remx=numx-denx*quox+deny*quoy;
     remy=numy-denx*quoy-deny*quox+deny*quoy;
     // Adjust division so that remainder has least norm.
     // Ties are broken by < or <= for a symmetrical, but eccentric,
     // shape when dividing by LETTERMOD.
     do {cont=false; // FIXME this loop may need to be optimized
         nrm=_norm(remx,remy);
         nrm1=_norm(remx+denx-deny,remy+denx);
         if (debughvec)
            printf("quo=%d+%dω rem=%d+%dω nrm=%d nrm1=%d\n",quox,quoy,remx,remy,nrm,nrm1);
         if (nrm1<nrm)
            {remx=remx+denx-deny;
             remy=remy+denx;
             quox--;
	     quoy--;
	     cont-=13;
	     }
         nrm=_norm(remx,remy);
         nrm1=_norm(remx+deny,remy+deny-denx);
         if (debughvec)
            printf("quo=%d+%dω rem=%d+%dω nrm=%d nrm1=%d\n",quox,quoy,remx,remy,nrm,nrm1);
         if (nrm1<nrm)
            {remx=remx+deny;
             remy=remy+deny-denx;
             quoy++;
	     cont+=8;
	     }
         nrm=_norm(remx,remy);
         nrm1=_norm(remx-denx,remy-deny);
         if (debughvec)
            printf("quo=%d+%dω rem=%d+%dω nrm=%d nrm1=%d\n",quox,quoy,remx,remy,nrm,nrm1);
         if (nrm1<nrm)
            {remx=remx-denx;
             remy=remy-deny;
             quox++;
	     cont+=5;
	     }
         nrm=_norm(remx,remy);
         nrm1=_norm(remx+deny-denx,remy-denx);
         if (debughvec)
            printf("quo=%d+%dω rem=%d+%dω nrm=%d nrm1=%d\n",quox,quoy,remx,remy,nrm,nrm1);
         if (nrm1<nrm)
            {remx=remx+deny-denx;
             remy=remy-denx;
             quox++;
	     quoy++;
	     cont+=13;
	     }
         nrm=_norm(remx,remy);
         nrm1=_norm(remx-deny,remy+denx-deny);
         if (debughvec)
            printf("quo=%d+%dω rem=%d+%dω nrm=%d nrm1=%d\n",quox,quoy,remx,remy,nrm,nrm1);
         if (nrm1<nrm)
            {remx=remx-deny;
             remy=remy+denx-deny;
             quoy--;
	     cont-=8;
	     }
         nrm=_norm(remx,remy);
         nrm1=_norm(remx+denx,remy+deny);
         if (debughvec)
            printf("quo=%d+%dω rem=%d+%dω nrm=%d nrm1=%d\n",quox,quoy,remx,remy,nrm,nrm1);
         if (nrm1<nrm)
            {remx=remx+denx;
             remy=remy+deny;
             quox--;
	     cont-=5;
	     }
	 if (debughvec)
            printf("loop\n");
	 } while (0);
     nrm=_norm(remx,remy);
     nrm1=_norm(remx+denx,remy+deny);
     if (debughvec)
        printf("quo=%d+%dω rem=%d+%dω nrm=%d nrm1=%d\n",quox,quoy,remx,remy,nrm,nrm1);
     if (nrm1<=nrm)
        {remx=remx+denx;
         remy=remy+deny;
         quox--;
         }
     if (debughvec)
        printf("quo=%d+%dω rem=%d+%dω \n",quox,quoy,remx,remy);
     nrm=_norm(remx,remy);
     nrm1=_norm(remx-deny+denx,remy+denx);
     if (debughvec)
        printf("quo=%d+%dω rem=%d+%dω nrm=%d nrm1=%d\n",quox,quoy,remx,remy,nrm,nrm1);
     if (nrm1<=nrm)
        {remx=remx-deny+denx;
         remy=remy+denx;
         quox--;
	 quoy--;
	 }
     nrm=_norm(remx,remy);
     nrm1=_norm(remx+deny,remy-denx+deny);
     if (debughvec)
        printf("quo=%d+%dω rem=%d+%dω nrm=%d nrm1=%d\n",quox,quoy,remx,remy,nrm,nrm1);
     if (nrm1<=nrm)
        {remx=remx+deny;
         remy=remy-denx+deny;
         quoy++;
	 }
     }
 }

hvec hvec::operator+(hvec b)
{return hvec(this->x+b.x,this->y+b.y);
 }

hvec& hvec::operator+=(hvec b)
{this->x+=b.x,this->y+=b.y;
 return *this;
 }

hvec hvec::operator-()
{return hvec(-this->x,-this->y);
 }

hvec hvec::operator-(hvec b)
{return hvec(this->x-b.x,this->y-b.y);
 }

bool operator<(const hvec a,const hvec b)
// These numbers are complex, so there is no consistent < operator on them.
// This operator is used only to give some order to the map.
{if (a.y!=b.y)
    return a.y<b.y;
 else
    return a.x<b.x;
 }

hvec hvec::operator*(hvec b)
{return hvec(x*b.x-y*b.y,x*b.y+y*b.x-y*b.y);
 }

hvec& hvec::operator*=(hvec b)
{int tmp;
 tmp=x*b.x-y*b.y;
 y=x*b.y+y*b.x-y*b.y;
 x=tmp;
 return *this;
 }

hvec hvec::operator/(hvec b)
{if (b==0)
    throw(domain_error("Divide by zero Eisenstein integer"));
 divmod(b);
 return hvec(quox,quoy);
 }

hvec hvec::operator%(hvec b)
{if (b==0)
    return (*this); // Dividing by zero is an error, but modding by zero is not.
 else
    {divmod(b);
     return hvec(remx,remy);
     }
 }

bool hvec::operator==(hvec b)
{return this->x==b.x && this->y==b.y;
 }

bool hvec::operator!=(hvec b)
{return this->x!=b.x || this->y!=b.y;
 }

unsigned long hvec::norm()
{return sqr(this->x)+sqr(this->y)-this->x*this->y;
 }

int hvec::pageinx()
// Index to a byte within a page. Meaningful only if the number
// is a remainder of division by PAGEMOD.
{if (y>0)
    return (y-PAGERAD)*(y-3*PAGERAD-3)/2+x;
 else
    return x+PAGESIZE-(y+PAGERAD)*(y+3*PAGERAD+3)/2-1;
 }

// Iteration: start, inc, cont. Iterates over a hexagon.

hvec start(int n)
{if (n<0)
    throw(out_of_range("hvec start: n<0"));
 return hvec(-n,-n);
 }

void hvec::inc(int n)
{if (n<0)
    throw(out_of_range("hvec::inc: n<0"));
 x++;
 if (y<0)
    if (x-y>n)
       {y++;
        x=-n;
        }
    else;
 else
    if (x>n)
       {y++;
        x=y-n;
        }
 }

bool hvec::cont(int n)
{return y<=n;
 }

int hvec::letterinx()
{switch (y)
   {case 1:
    return 11-x;
    case 0:
    return 8-x;
    case -1:
    return 4-x;
    case -2:
    return -x;
    default:
    return 32768;
    }
 }

void testcomplex()
{
  complex<double> z=8191,r(0.8,0.6),z2,diff;
  int i;
  hvec h;
  psopen("testcomplex.ps");
  psprolog();
  startpage();
  for (i=0;i<32768;i++)
  {
    z*=r;
    h=z;
    z2=h;
    diff=z-z2;
    //cout<<diff<<endl;
    plotpoint(diff.real()*100,diff.imag()*100);
  }
  h=hvec(0,1);
  z=h;
  cout<<z<<endl;
  endpage();
  pstrailer();
  psclose();
}
