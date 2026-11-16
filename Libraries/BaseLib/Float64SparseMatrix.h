#ifndef __FLOAT64_SPARSE_MATRIX_H__
#define __FLOAT64_SPARSE_MATRIX_H__
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


//A matrix most of whose numbers are zero, held as the others (its entries): each one's row, column and value, added in any
//order.  Two entries at one place add.  Multiplying by it or its transpose looks at each entry once.
//
//It's for linear least squares problems too big to solve directly (SupportLib's UnwrapPlan.txt: LSCM's unknowns are a
//chart's UVs).  Float64 throughout, as a big system's sums lose float32's precision.
class CFloat64SparseMatrix
{
public:
	CArrayLargeInt		maiRows;
	CArrayLargeInt		maiColumns;
	CArrayLargeFloat64	mafValues;
	int					miNumRows;
	int					miNumColumns;

	void	Init(int iNumRows, int iNumColumns);
	void	Kill(void);

	void	Add(int iRow, int iColumn, float64 fValue);

	int		NumRows(void);
	int		NumColumns(void);
	lsize	NumEntries(void);
	float64	Get(int iRow, int iColumn);  //Looks at every entry.

	void	Multiply(float64* pafDest, float64* pafVector);  //A x: x has NumColumns numbers, Dest NumRows.
	void	MultiplyTranspose(float64* pafDest, float64* pafVector);  //A' x: x has NumRows numbers, Dest NumColumns.
};


//Both solvers are conjugate gradients.  pafX is the first guess (zeros will do) and gets the answer.  They stop when the
//residual is no more than fTolerance of where it would be at x = 0, or after iMaxIterations (in exact arithmetic conjugate
//gradients are done within as many iterations as there are unknowns; rounding takes more on a badly conditioned system).
//They return how many iterations they made, or -1 if they didn't get there (or the matrix wasn't as they need it).

//A x = b for A square, symmetric and positive definite.  The residual is |b - A x|.
int		Float64SparseSolve(CFloat64SparseMatrix* pcA, float64* pafX, float64* pafB, float64 fTolerance, int iMaxIterations);

//The x for which |A x - b| is least, for any A whose columns are independent (a least squares fit has more rows than
//columns): the normal equations A'A x = A'b, without making A'A (CGLS: Hestenes and Stiefel's conjugate gradients on them,
//multiplying by A and A' in turn).  The residual is |A'(b - A x)|.
int		Float64SparseLeastSquares(CFloat64SparseMatrix* pcA, float64* pafX, float64* pafB, float64 fTolerance, int iMaxIterations);

//The same answer directly: A'A is made (CFloat64SkylineMatrix), its unknowns numbered by reverse Cuthill-McKee so its envelope is
//narrow, and Cholesky factored.  Exact but for rounding, in about the unknowns times the envelope's width squared; better than
//conjugate gradients where the system is a mesh's (a chart's UVs), as they need about as many iterations as there are
//unknowns.  pafX gets the answer.  False if A's columns aren't independent.
bool	Float64SparseLeastSquaresDirect(CFloat64SparseMatrix* pcA, float64* pafX, float64* pafB);

//Each column's place in an order that keeps columns sharing a row near each other (reverse Cuthill-McKee): from a column of
//least neighbours at the far side of each connected group, breadth first, neighbours fewest first, then reversed.
void	Float64SparseColumnOrder(CArrayLargeInt* paiPlaces, CFloat64SparseMatrix* pcA);


#endif // __FLOAT64_SPARSE_MATRIX_H__

