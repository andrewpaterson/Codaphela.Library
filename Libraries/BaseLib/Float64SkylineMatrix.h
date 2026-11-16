#ifndef __FLOAT64_SKYLINE_MATRIX_H__
#define __FLOAT64_SKYLINE_MATRIX_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "ArrayInt.h"
#include "ArrayFloat64.h"


//A symmetric matrix held as its lower triangle's envelope (its skyline): each row from its first column that may not be zero
//to the diagonal, the rows end to end.  Cholesky factoring (A = L L') fills nothing outside the envelope, so a matrix whose
//entries keep near its diagonal (its unknowns numbered so, as reverse Cuthill-McKee does) factors in about its size times its
//envelope's width squared, and then solves exactly.
class CFloat64SkylineMatrix
{
public:
	CArrayLargeInt								maiFirsts;  //Each row's first column.
	CArrayLargeTemplatePrimitive<lsize>			mauiStarts;  //Each row's first value in mafValues, and one more after the last.
	CArrayLargeFloat64							mafValues;
	int											miSize;

	void	Init(int* paiFirsts, int iSize);  //All zero.
	void	Kill(void);

	void	Add(int iRow, int iColumn, float64 fValue);  //In the lower triangle (iColumn no more than iRow) and the envelope.
	float64	Get(int iRow, int iColumn);  //Either triangle; zero outside the envelope.

	bool	Factor(void);  //Into L, in place.  False if the matrix isn't positive definite.
	void	Solve(float64* pafX, float64* pafB);  //Once factored: L L' x = b.  pafX can be pafB.

protected:
	float64*	Row(int iRow);  //Indexed by column, valid from the row's first column to its diagonal.
};


#endif // __FLOAT64_SKYLINE_MATRIX_H__

