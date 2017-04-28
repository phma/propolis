#include <vector>

bool isedge(hvec z);
bool ismarked(hvec z);
void mark(hvec z);
std::vector<hvec> trace(hvec stpoint);
std::vector<std::vector<hvec> > traceall(int size);
