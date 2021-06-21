/******************************************************/
/*                                                    */
/* ecctest.cpp - test error-correcting code           */
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
#include "ecctest.h"
#include "random.h"
/* This test consists of creating a size-33 symbol (3360 data/check letters)
 * containing the number φ-1 in base 32, with the specified redundancy,
 * flipping, setting, or clearing bits at random positions, in a circle at
 * random position, or along a random line, and attempting to decode the
 * erroneous symbol. The fraction of bits, radius of circle, or width of line
 * that makes the symbol undecodable half the time is how much error it
 * can stand.
 */
