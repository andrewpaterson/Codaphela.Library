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
#include "Float64SparseMatrix.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFloat64SparseMatrix::Init(int iNumRows, int iNumColumns)
{
	maiRows.Init();
	maiColumns.Init();
	mafValues.Init();
	miNumRows = iNumRows;
	miNumColumns = iNumColumns;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFloat64SparseMatrix::Kill(void)
{
	mafValues.Kill();
	maiColumns.Kill();
	maiRows.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFloat64SparseMatrix::Add(int iRow, int iColumn, float64 fValue)
{
	maiRows.Add(iRow);
	maiColumns.Add(iColumn);
	mafValues.Add(fValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CFloat64SparseMatrix::NumRows(void)
{
	return miNumRows;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CFloat64SparseMatrix::NumColumns(void)
{
	return miNumColumns;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CFloat64SparseMatrix::NumEntries(void)
{
	return mafValues.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CFloat64SparseMatrix::Get(int iRow, int iColumn)
{
	lsize	i;
	lsize	uiNumEntries;
	float64	fValue;

	fValue = 0.0;
	uiNumEntries = NumEntries();
	for (i = 0; i < uiNumEntries; i++)
	{
		if ((maiRows.GetValue(i) == iRow) && (maiColumns.GetValue(i) == iColumn))
		{
			fValue += mafValues.GetValue(i);
		}
	}
	return fValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFloat64SparseMatrix::Multiply(float64* pafDest, float64* pafVector)
{
	int*		paiRows;
	int*		paiColumns;
	float64*	pafValues;
	lsize		i;
	lsize		uiNumEntries;
	int			iRow;

	for (iRow = 0; iRow < miNumRows; iRow++)
	{
		pafDest[iRow] = 0.0;
	}
	paiRows = maiRows.GetData();
	paiColumns = maiColumns.GetData();
	pafValues = mafValues.GetData();
	uiNumEntries = NumEntries();
	for (i = 0; i < uiNumEntries; i++)
	{
		pafDest[paiRows[i]] += pafValues[i] * pafVector[paiColumns[i]];
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFloat64SparseMatrix::MultiplyTranspose(float64* pafDest, float64* pafVector)
{
	int*		paiRows;
	int*		paiColumns;
	float64*	pafValues;
	lsize		i;
	lsize		uiNumEntries;
	int			iColumn;

	for (iColumn = 0; iColumn < miNumColumns; iColumn++)
	{
		pafDest[iColumn] = 0.0;
	}
	paiRows = maiRows.GetData();
	paiColumns = maiColumns.GetData();
	pafValues = mafValues.GetData();
	uiNumEntries = NumEntries();
	for (i = 0; i < uiNumEntries; i++)
	{
		pafDest[paiColumns[i]] += pafValues[i] * pafVector[paiRows[i]];
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static float64 Float64SparseDot(float64* pafA, float64* pafB, int iLength)
{
	float64	fSum;
	int		i;

	fSum = 0.0;
	for (i = 0; i < iLength; i++)
	{
		fSum += pafA[i] * pafB[i];
	}
	return fSum;
}


//////////////////////////////////////////////////////////////////////////
//
// Dest += fScale * Vector.
//
//////////////////////////////////////////////////////////////////////////
static void Float64SparseAddScaled(float64* pafDest, float64 fScale, float64* pafVector, int iLength)
{
	int		i;

	for (i = 0; i < iLength; i++)
	{
		pafDest[i] += fScale * pafVector[i];
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The next search direction: Direction = Residual + fScale * Direction.
//
//////////////////////////////////////////////////////////////////////////
static void Float64SparseNextDirection(float64* pafDirection, float64* pafResidual, float64 fScale, int iLength)
{
	int		i;

	for (i = 0; i < iLength; i++)
	{
		pafDirection[i] = pafResidual[i] + (fScale * pafDirection[i]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The arrays a solve works in, each iLength numbers, returned in turn.
//
//////////////////////////////////////////////////////////////////////////
static float64* Float64SparseVector(CArrayLargeFloat64* pafVectors, int iLength, int iVector)
{
	return &pafVectors->GetData()[(lsize)iLength * (lsize)iVector];
}


//////////////////////////////////////////////////////////////////////////
//
// r = b - A x, then x moves along directions each conjugate to (A
// orthogonal to) those before: x += alpha p, r -= alpha A p, p = r + beta
// p.
//
//////////////////////////////////////////////////////////////////////////
int Float64SparseSolve(CFloat64SparseMatrix* pcA, float64* pafX, float64* pafB, float64 fTolerance, int iMaxIterations)
{
	CArrayLargeFloat64	afVectors;
	float64*			pafResidual;
	float64*			pafDirection;
	float64*			pafProduct;
	float64				fResidual;
	float64				fLimit;
	float64				fCurve;
	float64				fAlpha;
	float64				fLast;
	int					iLength;
	int					iIteration;
	int					i;

	iLength = pcA->NumColumns();
	afVectors.Init();
	afVectors.Resize((lsize)iLength * 3);
	pafResidual = Float64SparseVector(&afVectors, iLength, 0);
	pafDirection = Float64SparseVector(&afVectors, iLength, 1);
	pafProduct = Float64SparseVector(&afVectors, iLength, 2);

	pcA->Multiply(pafResidual, pafX);
	for (i = 0; i < iLength; i++)
	{
		pafResidual[i] = pafB[i] - pafResidual[i];
		pafDirection[i] = pafResidual[i];
	}
	fResidual = Float64SparseDot(pafResidual, pafResidual, iLength);
	fLimit = fTolerance * fTolerance * Float64SparseDot(pafB, pafB, iLength);

	for (iIteration = 0; iIteration <= iMaxIterations; iIteration++)
	{
		if (fResidual <= fLimit)
		{
			afVectors.Kill();
			return iIteration;
		}
		if (iIteration == iMaxIterations)
		{
			break;
		}

		pcA->Multiply(pafProduct, pafDirection);
		fCurve = Float64SparseDot(pafDirection, pafProduct, iLength);
		if (fCurve <= 0.0)
		{
			break;
		}
		fAlpha = fResidual / fCurve;
		Float64SparseAddScaled(pafX, fAlpha, pafDirection, iLength);
		Float64SparseAddScaled(pafResidual, -fAlpha, pafProduct, iLength);

		fLast = fResidual;
		fResidual = Float64SparseDot(pafResidual, pafResidual, iLength);
		Float64SparseNextDirection(pafDirection, pafResidual, fResidual / fLast, iLength);
	}
	afVectors.Kill();
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// As Float64SparseSolve on A'A x = A'b, keeping r = b - A x (a row's
// length) and s = A' r (a column's): x += alpha p, r -= alpha A p,
// s = A' r, p = s + beta p.
//
//////////////////////////////////////////////////////////////////////////
int Float64SparseLeastSquares(CFloat64SparseMatrix* pcA, float64* pafX, float64* pafB, float64 fTolerance, int iMaxIterations)
{
	CArrayLargeFloat64	afColumns;
	CArrayLargeFloat64	afRows;
	float64*			pafNormal;
	float64*			pafDirection;
	float64*			pafResidual;
	float64*			pafProduct;
	float64				fNormal;
	float64				fLimit;
	float64				fCurve;
	float64				fAlpha;
	float64				fLast;
	int					iNumColumns;
	int					iNumRows;
	int					iIteration;
	int					i;

	iNumColumns = pcA->NumColumns();
	iNumRows = pcA->NumRows();
	afColumns.Init();
	afColumns.Resize((lsize)iNumColumns * 2);
	afRows.Init();
	afRows.Resize((lsize)iNumRows * 2);
	pafNormal = Float64SparseVector(&afColumns, iNumColumns, 0);
	pafDirection = Float64SparseVector(&afColumns, iNumColumns, 1);
	pafResidual = Float64SparseVector(&afRows, iNumRows, 0);
	pafProduct = Float64SparseVector(&afRows, iNumRows, 1);

	pcA->MultiplyTranspose(pafNormal, pafB);
	fLimit = fTolerance * fTolerance * Float64SparseDot(pafNormal, pafNormal, iNumColumns);

	pcA->Multiply(pafResidual, pafX);
	for (i = 0; i < iNumRows; i++)
	{
		pafResidual[i] = pafB[i] - pafResidual[i];
	}
	pcA->MultiplyTranspose(pafNormal, pafResidual);
	for (i = 0; i < iNumColumns; i++)
	{
		pafDirection[i] = pafNormal[i];
	}
	fNormal = Float64SparseDot(pafNormal, pafNormal, iNumColumns);

	for (iIteration = 0; iIteration <= iMaxIterations; iIteration++)
	{
		if (fNormal <= fLimit)
		{
			afRows.Kill();
			afColumns.Kill();
			return iIteration;
		}
		if (iIteration == iMaxIterations)
		{
			break;
		}

		pcA->Multiply(pafProduct, pafDirection);
		fCurve = Float64SparseDot(pafProduct, pafProduct, iNumRows);
		if (fCurve <= 0.0)
		{
			break;
		}
		fAlpha = fNormal / fCurve;
		Float64SparseAddScaled(pafX, fAlpha, pafDirection, iNumColumns);
		Float64SparseAddScaled(pafResidual, -fAlpha, pafProduct, iNumRows);
		pcA->MultiplyTranspose(pafNormal, pafResidual);

		fLast = fNormal;
		fNormal = Float64SparseDot(pafNormal, pafNormal, iNumColumns);
		Float64SparseNextDirection(pafDirection, pafNormal, fNormal / fLast, iNumColumns);
	}
	afRows.Kill();
	afColumns.Kill();
	return -1;
}

