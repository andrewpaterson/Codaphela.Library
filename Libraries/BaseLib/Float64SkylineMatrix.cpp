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
#include <math.h>
#include "Float64SkylineMatrix.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFloat64SkylineMatrix::Init(int* paiFirsts, int iSize)
{
	lsize	uiStart;
	int		i;

	miSize = iSize;
	maiFirsts.Init();
	mauiStarts.Init();
	mafValues.Init();
	uiStart = 0;
	for (i = 0; i < iSize; i++)
	{
		maiFirsts.Add(paiFirsts[i]);
		mauiStarts.Add(uiStart);
		uiStart += (lsize)(i - paiFirsts[i] + 1);
	}
	mauiStarts.Add(uiStart);
	mafValues.Resize(uiStart);
	mafValues.SetArrayValues(0.0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFloat64SkylineMatrix::Kill(void)
{
	mafValues.Kill();
	mauiStarts.Kill();
	maiFirsts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A row's values less its first column, so Row(i)[j] is entry (i, j).
//
//////////////////////////////////////////////////////////////////////////
float64* CFloat64SkylineMatrix::Row(int iRow)
{
	return mafValues.GetData() + mauiStarts.GetValue((lsize)iRow) - maiFirsts.GetValue((lsize)iRow);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFloat64SkylineMatrix::Add(int iRow, int iColumn, float64 fValue)
{
	Row(iRow)[iColumn] += fValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CFloat64SkylineMatrix::Get(int iRow, int iColumn)
{
	int		iSwap;

	if (iColumn > iRow)
	{
		iSwap = iRow;
		iRow = iColumn;
		iColumn = iSwap;
	}
	if (iColumn < maiFirsts.GetValue((lsize)iRow))
	{
		return 0.0;
	}
	return Row(iRow)[iColumn];
}


//////////////////////////////////////////////////////////////////////////
//
// Row by row (Jennings): L(i, j) = (A(i, j) - sum of L(i, k) L(j, k))
// / L(j, j) for k where both rows are, then L(i, i) the root of what's
// left of A(i, i).
//
//////////////////////////////////////////////////////////////////////////
bool CFloat64SkylineMatrix::Factor(void)
{
	float64*	pafRow;
	float64*	pafOther;
	float64		fSum;
	int			iFirst;
	int			iOtherFirst;
	int			i;
	int			j;
	int			k;

	for (i = 0; i < miSize; i++)
	{
		pafRow = Row(i);
		iFirst = maiFirsts.GetValue((lsize)i);
		for (j = iFirst; j < i; j++)
		{
			pafOther = Row(j);
			iOtherFirst = maiFirsts.GetValue((lsize)j);
			fSum = pafRow[j];
			for (k = (iFirst > iOtherFirst) ? iFirst : iOtherFirst; k < j; k++)
			{
				fSum -= pafRow[k] * pafOther[k];
			}
			pafRow[j] = fSum / pafOther[j];
		}
		fSum = pafRow[i];
		for (k = iFirst; k < i; k++)
		{
			fSum -= pafRow[k] * pafRow[k];
		}
		if (fSum <= 0.0)
		{
			return false;
		}
		pafRow[i] = sqrt(fSum);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// L y = b forwards, then L' x = y backwards, each row of L giving its
// column of L'.
//
//////////////////////////////////////////////////////////////////////////
void CFloat64SkylineMatrix::Solve(float64* pafX, float64* pafB)
{
	float64*	pafRow;
	float64		fSum;
	int			iFirst;
	int			i;
	int			k;

	for (i = 0; i < miSize; i++)
	{
		pafRow = Row(i);
		iFirst = maiFirsts.GetValue((lsize)i);
		fSum = pafB[i];
		for (k = iFirst; k < i; k++)
		{
			fSum -= pafRow[k] * pafX[k];
		}
		pafX[i] = fSum / pafRow[i];
	}
	for (i = miSize - 1; i >= 0; i--)
	{
		pafRow = Row(i);
		iFirst = maiFirsts.GetValue((lsize)i);
		pafX[i] /= pafRow[i];
		for (k = iFirst; k < i; k++)
		{
			pafX[k] -= pafRow[k] * pafX[i];
		}
	}
}

