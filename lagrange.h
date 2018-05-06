/* Lagrange polynomial interpolation in F31
 */

#include <vector>

class int31
{
private:
  int n;
public:
  int31(int m);
  int val();
  int31 operator+(int31 b);
  int31 operator-(int31 b);
  int31 operator*(int31 b);
  int31 operator/(int31 b);
};
