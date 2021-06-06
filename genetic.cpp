/******************************************************/
/*                                                    */
/* genetic.cpp - genetic algorithm                    */
/*                                                    */
/******************************************************/
/* Copyright 2021 Pierre Abbat.
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
/* A genome consists of a single gene, which is a permutation of 32 12-bit
 * patterns with 0x000 held fixed to @. The fitness is the sum of square
 * roots of bit difference counts. A mutation consists of swapping bit patterns.
 * Children are produced by taking some bit patterns from the mother and some
 * from the father in such a way that the child has 32 different bit patterns.
 */
#include "genetic.h"
using namespace std;

LetterMap::LetterMap()
{
  int i;
  fit=NAN;
  for (i=0;i<32;i++)
    bitPatterns[i]=letters[i];
}