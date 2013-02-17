#include <string>
#include "outformat.h"

#define FMT_PNM 0
#define FMT_PNG 1
#define FMT_JPEG 2

void initsubsample();
void rasterdraw(int size,double width,double height,
	    double scale,int dim,int imagetype,string filename);
