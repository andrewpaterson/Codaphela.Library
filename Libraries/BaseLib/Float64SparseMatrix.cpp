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
int CFloat64SparseMatrix::NumRows(void) const
{
	return miNumRows;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CFloat64SparseMatrix::NumColumns(void) const
{
	return miNumColumns;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CFloat64SparseMatrix::NumEntries(void) const
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

//////////////////////////////////////////////////////////////////////////
//
// The entries row by row: paiStarts[r] to paiStarts[r + 1] in paiEntries
// are row r's.
//
//////////////////////////////////////////////////////////////////////////
static void Float64SparseRows(CFloat64SparseMatrix* pcA, CArrayLargeInt* paiStarts, CArrayLargeInt* paiEntries)
{
	CArrayLargeInt	aiFilled;
	lsize			uiEntry;
	int				iRow;

	paiStarts->Resize((lsize)pcA->NumRows() + 1);
	paiStarts->SetArrayValues(0);
	for (uiEntry = 0; uiEntry < pcA->NumEntries(); uiEntry++)
	{
		iRow = pcA->maiRows.GetValue(uiEntry);
		paiStarts->SetValue((lsize)iRow + 1, paiStarts->GetValue((lsize)iRow + 1) + 1);
	}
	for (iRow = 0; iRow < pcA->NumRows(); iRow++)
	{
		paiStarts->SetValue((lsize)iRow + 1, paiStarts->GetValue((lsize)iRow + 1) + paiStarts->GetValue((lsize)iRow));
	}
	aiFilled.Init();
	aiFilled.Resize((lsize)pcA->NumRows());
	aiFilled.SetArrayValues(0);
	paiEntries->Resize(pcA->NumEntries());
	for (uiEntry = 0; uiEntry < pcA->NumEntries(); uiEntry++)
	{
		iRow = pcA->maiRows.GetValue(uiEntry);
		paiEntries->SetValue((lsize)(paiStarts->GetValue((lsize)iRow) + aiFilled.GetValue((lsize)iRow)), (int)uiEntry);
		aiFilled.SetValue((lsize)iRow, aiFilled.GetValue((lsize)iRow) + 1);
	}
	aiFilled.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each column's neighbours (the other columns in its rows), each once:
// paiStarts[c] to paiStarts[c + 1] in paiNeighbours.
//
//////////////////////////////////////////////////////////////////////////
static void Float64SparseNeighbours(CFloat64SparseMatrix* pcA, CArrayLargeInt* paiStarts, CArrayLargeInt* paiNeighbours)
{
	CArrayLargeInt	aiRowStarts;
	CArrayLargeInt	aiEntries;
	CArrayLargeInt	aiPairs;  //Column, neighbour.
	CArrayLargeInt	aiMark;
	CArrayLargeInt	aiPlaces;  //Where each column's next neighbour goes; once filled, where the next column's start.
	CArrayLargeInt	aiAll;
	int				iRow;
	int				iColumn;
	int				iOther;
	int				i;
	int				j;
	lsize			uiPair;

	aiRowStarts.Init();
	aiEntries.Init();
	Float64SparseRows(pcA, &aiRowStarts, &aiEntries);
	aiPairs.Init();
	for (iRow = 0; iRow < pcA->NumRows(); iRow++)
	{
		for (i = aiRowStarts.GetValue((lsize)iRow); i < aiRowStarts.GetValue((lsize)iRow + 1); i++)
		{
			iColumn = pcA->maiColumns.GetValue((lsize)aiEntries.GetValue((lsize)i));
			for (j = aiRowStarts.GetValue((lsize)iRow); j < aiRowStarts.GetValue((lsize)iRow + 1); j++)
			{
				iOther = pcA->maiColumns.GetValue((lsize)aiEntries.GetValue((lsize)j));
				if (iOther != iColumn)
				{
					aiPairs.Add(iColumn);
					aiPairs.Add(iOther);
				}
			}
		}
	}
	aiEntries.Kill();
	aiRowStarts.Kill();

	//Counted into place, then each column's repeats taken out by marking the neighbours it has.
	aiPlaces.Init();
	aiPlaces.Resize((lsize)pcA->NumColumns() + 1);
	aiPlaces.SetArrayValues(0);
	for (uiPair = 0; uiPair < aiPairs.NumElements(); uiPair += 2)
	{
		iColumn = aiPairs.GetValue(uiPair);
		aiPlaces.SetValue((lsize)iColumn + 1, aiPlaces.GetValue((lsize)iColumn + 1) + 1);
	}
	for (iColumn = 0; iColumn < pcA->NumColumns(); iColumn++)
	{
		aiPlaces.SetValue((lsize)iColumn + 1, aiPlaces.GetValue((lsize)iColumn + 1) + aiPlaces.GetValue((lsize)iColumn));
	}
	aiAll.Init();
	aiAll.Resize(aiPairs.NumElements() / 2);
	for (uiPair = 0; uiPair < aiPairs.NumElements(); uiPair += 2)
	{
		iColumn = aiPairs.GetValue(uiPair);
		aiAll.SetValue((lsize)aiPlaces.GetValue((lsize)iColumn), aiPairs.GetValue(uiPair + 1));
		aiPlaces.SetValue((lsize)iColumn, aiPlaces.GetValue((lsize)iColumn) + 1);
	}

	paiStarts->Resize((lsize)pcA->NumColumns() + 1);
	paiNeighbours->ReInit();
	aiMark.Init();
	aiMark.Resize((lsize)pcA->NumColumns());
	aiMark.SetArrayValues(-1);
	i = 0;
	for (iColumn = 0; iColumn < pcA->NumColumns(); iColumn++)
	{
		paiStarts->SetValue((lsize)iColumn, (int)paiNeighbours->NumElements());
		for (; i < aiPlaces.GetValue((lsize)iColumn); i++)
		{
			iOther = aiAll.GetValue((lsize)i);
			if (aiMark.GetValue((lsize)iOther) != iColumn)
			{
				aiMark.SetValue((lsize)iOther, iColumn);
				paiNeighbours->Add(iOther);
			}
		}
	}
	paiStarts->SetValue((lsize)pcA->NumColumns(), (int)paiNeighbours->NumElements());
	aiAll.Kill();
	aiPlaces.Kill();
	aiMark.Kill();
	aiPairs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A column's number of neighbours.
//
//////////////////////////////////////////////////////////////////////////
static int Float64SparseDegree(CArrayLargeInt* paiStarts, int iColumn)
{
	return paiStarts->GetValue((lsize)iColumn + 1) - paiStarts->GetValue((lsize)iColumn);
}


//////////////////////////////////////////////////////////////////////////
//
// Breadth first from iStart, each column marked iMark in paiReached as
// it's added to paiOrder, a column's new neighbours added fewest
// neighbours first.  Returns the last column reached, at the far side.
//
//////////////////////////////////////////////////////////////////////////
static int Float64SparseBreadthFirst(CArrayLargeInt* paiOrder, CArrayLargeInt* paiReached, int iMark, int iStart, CArrayLargeInt* paiStarts, CArrayLargeInt* paiNeighbours)
{
	lsize	uiHead;
	lsize	uiAdded;
	lsize	j;
	int		iColumn;
	int		iNeighbour;
	int		iEarlier;
	int		i;

	uiHead = paiOrder->NumElements();
	paiOrder->Add(iStart);
	paiReached->SetValue((lsize)iStart, iMark);
	iColumn = iStart;
	for (; uiHead < paiOrder->NumElements(); uiHead++)
	{
		iColumn = paiOrder->GetValue(uiHead);
		uiAdded = paiOrder->NumElements();
		for (i = paiStarts->GetValue((lsize)iColumn); i < paiStarts->GetValue((lsize)iColumn + 1); i++)
		{
			iNeighbour = paiNeighbours->GetValue((lsize)i);
			if (paiReached->GetValue((lsize)iNeighbour) != iMark)
			{
				paiReached->SetValue((lsize)iNeighbour, iMark);
				paiOrder->Add(iNeighbour);
				for (j = paiOrder->NumElements() - 1; j > uiAdded; j--)
				{
					iEarlier = paiOrder->GetValue(j - 1);
					if (Float64SparseDegree(paiStarts, iEarlier) <= Float64SparseDegree(paiStarts, iNeighbour))
					{
						break;
					}
					paiOrder->SetValue(j, iEarlier);
					paiOrder->SetValue(j - 1, iNeighbour);
				}
			}
		}
	}
	return iColumn;
}


//////////////////////////////////////////////////////////////////////////
//
// For each connected group: from its column of fewest neighbours, a
// breadth first walk finds a column at its far side, the walk from there
// is the group's order (Cuthill-McKee); all reversed at the end.
//
//////////////////////////////////////////////////////////////////////////
void Float64SparseColumnOrder(CArrayLargeInt* paiPlaces, CFloat64SparseMatrix* pcA)
{
	CArrayLargeInt	aiStarts;
	CArrayLargeInt	aiNeighbours;
	CArrayLargeInt	aiOrder;
	CArrayLargeInt	aiReached;  //1 once a trial walk reaches a column, 2 once it's placed.
	lsize			uiFirst;
	int				iNumColumns;
	int				iColumn;
	int				iStart;
	int				iDegree;
	int				iLeast;

	iNumColumns = pcA->NumColumns();
	aiStarts.Init();
	aiNeighbours.Init();
	Float64SparseNeighbours(pcA, &aiStarts, &aiNeighbours);
	aiOrder.Init();
	aiReached.Init();
	aiReached.Resize((lsize)iNumColumns);
	aiReached.SetArrayValues(0);
	while ((int)aiOrder.NumElements() < iNumColumns)
	{
		iStart = -1;
		iLeast = 0;
		for (iColumn = 0; iColumn < iNumColumns; iColumn++)
		{
			iDegree = Float64SparseDegree(&aiStarts, iColumn);
			if ((aiReached.GetValue((lsize)iColumn) != 2) && ((iStart < 0) || (iDegree < iLeast)))
			{
				iStart = iColumn;
				iLeast = iDegree;
			}
		}

		//A trial walk to the far side, taken back off; marks made by it are 1, so they don't stop the real walk (2).
		uiFirst = aiOrder.NumElements();
		iStart = Float64SparseBreadthFirst(&aiOrder, &aiReached, 1, iStart, &aiStarts, &aiNeighbours);
		aiOrder.Resize(uiFirst);
		Float64SparseBreadthFirst(&aiOrder, &aiReached, 2, iStart, &aiStarts, &aiNeighbours);
	}

	paiPlaces->Resize((lsize)iNumColumns);
	for (iColumn = 0; iColumn < iNumColumns; iColumn++)
	{
		paiPlaces->SetValue((lsize)aiOrder.GetValue((lsize)iColumn), iNumColumns - 1 - iColumn);
	}
	aiReached.Kill();
	aiOrder.Kill();
	aiNeighbours.Kill();
	aiStarts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A'A is the sum over A's rows of each row's outer product with itself,
// each row's entries merged first, so a column given twice in it adds.
//
//////////////////////////////////////////////////////////////////////////
bool CFloat64SparseLeastSquares::Init(CFloat64SparseMatrix* pcA)
{
	CArrayLargeInt	aiRowStarts;
	CArrayLargeInt	aiEntries;
	CArrayLargeInt	aiFirsts;
	CArrayLargeInt	aiRowPlaces;
	CArrayLargeFloat64	afRowValues;
	lsize			uiEntry;
	int				iNumColumns;
	int				iRow;
	int				iPlace;
	int				iLeast;
	int				i;
	int				j;

	mpcA = pcA;
	iNumColumns = pcA->NumColumns();
	maiPlaces.Init();
	Float64SparseColumnOrder(&maiPlaces, pcA);
	aiRowStarts.Init();
	aiEntries.Init();
	Float64SparseRows(pcA, &aiRowStarts, &aiEntries);

	//Each place's first: the least place in any row it's in.
	aiFirsts.Init();
	aiFirsts.Resize((lsize)iNumColumns);
	for (i = 0; i < iNumColumns; i++)
	{
		aiFirsts.SetValue((lsize)i, i);
	}
	for (iRow = 0; iRow < pcA->NumRows(); iRow++)
	{
		iLeast = iNumColumns;
		for (i = aiRowStarts.GetValue((lsize)iRow); i < aiRowStarts.GetValue((lsize)iRow + 1); i++)
		{
			iPlace = maiPlaces.GetValue((lsize)pcA->maiColumns.GetValue((lsize)aiEntries.GetValue((lsize)i)));
			iLeast = (iPlace < iLeast) ? iPlace : iLeast;
		}
		for (i = aiRowStarts.GetValue((lsize)iRow); i < aiRowStarts.GetValue((lsize)iRow + 1); i++)
		{
			iPlace = maiPlaces.GetValue((lsize)pcA->maiColumns.GetValue((lsize)aiEntries.GetValue((lsize)i)));
			if (iLeast < aiFirsts.GetValue((lsize)iPlace))
			{
				aiFirsts.SetValue((lsize)iPlace, iLeast);
			}
		}
	}

	mcNormal.Init(aiFirsts.GetData(), iNumColumns);
	aiRowPlaces.Init();
	afRowValues.Init();
	for (iRow = 0; iRow < pcA->NumRows(); iRow++)
	{
		aiRowPlaces.ReInit();
		afRowValues.ReInit();
		for (i = aiRowStarts.GetValue((lsize)iRow); i < aiRowStarts.GetValue((lsize)iRow + 1); i++)
		{
			uiEntry = (lsize)aiEntries.GetValue((lsize)i);
			iPlace = maiPlaces.GetValue((lsize)pcA->maiColumns.GetValue(uiEntry));
			for (j = 0; (j < (int)aiRowPlaces.NumElements()) && (aiRowPlaces.GetValue((lsize)j) != iPlace); j++)
			{
			}
			if (j == (int)aiRowPlaces.NumElements())
			{
				aiRowPlaces.Add(iPlace);
				afRowValues.Add(0.0);
			}
			afRowValues.SetValue((lsize)j, afRowValues.GetValue((lsize)j) + pcA->mafValues.GetValue(uiEntry));
		}
		for (i = 0; i < (int)aiRowPlaces.NumElements(); i++)
		{
			iPlace = aiRowPlaces.GetValue((lsize)i);
			for (j = 0; j < (int)aiRowPlaces.NumElements(); j++)
			{
				if (aiRowPlaces.GetValue((lsize)j) <= iPlace)
				{
					mcNormal.Add(iPlace, aiRowPlaces.GetValue((lsize)j), afRowValues.GetValue((lsize)i) * afRowValues.GetValue((lsize)j));
				}
			}
		}
	}
	afRowValues.Kill();
	aiRowPlaces.Kill();
	aiFirsts.Kill();
	aiEntries.Kill();
	aiRowStarts.Kill();

	mafRight.Init();
	mafRight.Resize((lsize)iNumColumns);
	mbFactored = mcNormal.Factor();
	return mbFactored;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFloat64SparseLeastSquares::Kill(void)
{
	mafRight.Kill();
	mcNormal.Kill();
	maiPlaces.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A'b, put in the factored order, solved, and put back.
//
//////////////////////////////////////////////////////////////////////////
void CFloat64SparseLeastSquares::Solve(float64* pafX, float64* pafB)
{
	float64*	pafRight;
	int			i;

	mpcA->MultiplyTranspose(pafX, pafB);
	pafRight = mafRight.GetData();
	for (i = 0; i < mpcA->NumColumns(); i++)
	{
		pafRight[maiPlaces.GetValue((lsize)i)] = pafX[i];
	}
	mcNormal.Solve(pafRight, pafRight);
	for (i = 0; i < mpcA->NumColumns(); i++)
	{
		pafX[i] = pafRight[maiPlaces.GetValue((lsize)i)];
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool Float64SparseLeastSquaresDirect(CFloat64SparseMatrix* pcA, float64* pafX, float64* pafB)
{
	CFloat64SparseLeastSquares	cSolver;
	bool						bSolved;

	bSolved = cSolver.Init(pcA);
	if (bSolved)
	{
		cSolver.Solve(pafX, pafB);
	}
	cSolver.Kill();
	return bSolved;
}

