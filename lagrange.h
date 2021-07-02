/******************************************************/
/*                                                    */
/* lagrange.h - Lagrange polynomial interpolation     */
/*                                                    */
/******************************************************/
/* Copyright 2018 Pierre Abbat.
 * This file is part of Propolis.
 * 
 * The Propolis program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Propolis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License and Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Propolis. If not, see <http://www.gnu.org/licenses/>.
 */
/* Lagrange polynomial interpolation in F31
 */
#include <vector>

class int31
{
private:
  int n;
public:
  int31();
  int31(int m);
  int val();
  char locase();
  int31 operator+(int31 b);
  int31 operator-(int31 b);
  int31 operator*(int31 b);
  int31 operator/(int31 b);
  int31& operator+=(int31 b);
  int31& operator-=(int31 b);
  int31& operator*=(int31 b);
  bool operator==(int31 b);
  friend class poly31;
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
  poly31& operator+=(int31 b);
  poly31 operator+(poly31 &b);
  poly31 operator+=(const poly31 &b);
  poly31 operator-(int31 b);
  poly31 operator-(poly31 &b);
  poly31 operator*(int31 b);
  poly31& operator*=(int31 b);
  poly31 operator*(poly31 &b);
  poly31& operator*=(const poly31 &b);
  poly31 operator/(int31 b);
  poly31& operator/=(int31 b);
};

poly31 interceptor(int31 x);
poly31 impulse(int bitmask,int31 x);
void putMetadataCheck(std::vector<int31> &metadata);
