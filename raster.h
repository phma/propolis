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

void initsubsample();
void rasterdraw(int size,double width,double height,
	    double scale,int dim,int imagetype,string filename);
int filletbit(complex<double> z);
void checkregbits();
