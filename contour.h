#include <vector>

bool isedge(hvec z);
bool ismarked(hvec z);
void mark(hvec z);
vector<hvec> trace(hvec stpoint);
vector<vector<hvec> > traceall(int size);
