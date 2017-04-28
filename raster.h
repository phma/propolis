#include <string>
#include "hvec.h"
#include "outformat.h"

struct locreg
{
  hvec location;
  int region;
  bool operator==(locreg b)
  {
    return this->location==b.location && this->region==b.region;
  }
};

void initsubsample(int q);
/* q=1: one dot per square
 * q=2: five dots per square
 * q=6: 85 dots per square, the usual
 * q=10: 221 dots per square, the most that makes sense
 */
void rasterdraw(int size,double width,double height,
	    double scale,int dim,int imagetype,std::string filename);
int filletbit(std::complex<double> z);
void checkregbits();
