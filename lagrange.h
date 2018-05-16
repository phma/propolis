/* Lagrange polynomial interpolation in F31
 */

class int31
{
private:
  int n;
public:
  int31();
  int31(int m);
  int val();
  int31 operator+(int31 b);
  int31 operator-(int31 b);
  int31 operator*(int31 b);
  int31 operator/(int31 b);
  int31& operator+=(int31 b);
  int31& operator-=(int31 b);
  int31& operator*=(int31 b);
};

class poly31
{
private:
  int31 coeff[30]; // By Fermat's little theorem
public:
  poly31();
  int31 operator()(int31 x);
  poly31& operator=(int31 b);
  poly31 operator<<(int n);
  poly31 operator>>(int n);
  poly31 operator+(int31 b);
  poly31 operator+(poly31 &b);
  poly31 operator-(int31 b);
  poly31 operator-(poly31 &b);
  poly31 operator*(int31 b);
  poly31 operator*(poly31 &b);
};
