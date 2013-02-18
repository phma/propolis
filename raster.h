#include <string>
#include "hvec.h"
#include "outformat.h"

#define FMT_PNM 0
#define FMT_PNG 1
#define FMT_JPEG 2

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
