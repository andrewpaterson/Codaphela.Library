#include "BaseLib/Float64SparseMatrix.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// The worst difference between two vectors.
//
//////////////////////////////////////////////////////////////////////////
static float64 TestFloat64SparseWorst(float64* pafA, float64* pafB, int iLength)
{
	float64	fWorst;
	float64	fDifference;
	int		i;

	fWorst = 0.0;
	for (i = 0; i < iLength; i++)
	{
		fDifference = pafA[i] - pafB[i];
		if (fDifference < 0.0)
		{
			fDifference = -fDifference;
		}
		if (fDifference > fWorst)
		{
			fWorst = fDifference;
		}
	}
	return fWorst;
}


//////////////////////////////////////////////////////////////////////////
//
// The next of a run of numbers from -1 to 1 that look random (a linear
// congruential generator's top bits), the same on every target.
//
//////////////////////////////////////////////////////////////////////////
static float64 TestFloat64SparseNext(uint32* puiSeed)
{
	*puiSeed = (*puiSeed * 1664525) + 1013904223;
	return ((float64)(*puiSeed >> 8) / 8388608.0) - 1.0;
}


//////////////////////////////////////////////////////////////////////////
//
//     1  0  2  0
//     0  3  0 -1
//     4  0  0  5
//
// with its 2 made of 1.5 and 0.5 (entries at one place add), times
// (1, 2, 3, 4) and its transpose times (1, 2, 3).
//
//////////////////////////////////////////////////////////////////////////
void TestFloat64SparseMatrixMultiply(void)
{
	CFloat64SparseMatrix	cMatrix;
	float64					afX[4] = { 1, 2, 3, 4 };
	float64					afY[3] = { 1, 2, 3 };
	float64					afRows[3];
	float64					afColumns[4];

	cMatrix.Init(3, 4);
	cMatrix.Add(2, 3, 5);
	cMatrix.Add(0, 0, 1);
	cMatrix.Add(0, 2, 1.5);
	cMatrix.Add(1, 1, 3);
	cMatrix.Add(2, 0, 4);
	cMatrix.Add(1, 3, -1);
	cMatrix.Add(0, 2, 0.5);

	AssertInt(3, cMatrix.NumRows());
	AssertInt(4, cMatrix.NumColumns());
	AssertLSize(7, cMatrix.NumEntries());
	AssertDouble(2.0, cMatrix.Get(0, 2), 10);
	AssertDouble(-1.0, cMatrix.Get(1, 3), 10);
	AssertDouble(0.0, cMatrix.Get(1, 0), 10);

	cMatrix.Multiply(afRows, afX);
	AssertDouble(7.0, afRows[0], 10);
	AssertDouble(2.0, afRows[1], 10);
	AssertDouble(24.0, afRows[2], 10);

	cMatrix.MultiplyTranspose(afColumns, afY);
	AssertDouble(13.0, afColumns[0], 10);
	AssertDouble(6.0, afColumns[1], 10);
	AssertDouble(2.0, afColumns[2], 10);
	AssertDouble(13.0, afColumns[3], 10);

	cMatrix.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 3 by 3 symmetric positive definite system, done in three iterations
// as conjugate gradients are; started at its answer, in none.  A long
// one (the second differences of 100 numbers, as a rod's heat, plus a
// little of each number so it's well posed) to its answer.  A zero b is
// a zero x at once.  A matrix that isn't positive definite fails.
//
//////////////////////////////////////////////////////////////////////////
void TestFloat64SparseMatrixSolve(void)
{
	CFloat64SparseMatrix	cMatrix;
	float64					afAnswer[100];
	float64					afB[100];
	float64					afX[100];
	int						i;
	int						iIterations;

	cMatrix.Init(3, 3);
	cMatrix.Add(0, 0, 4);
	cMatrix.Add(0, 1, 1);
	cMatrix.Add(1, 0, 1);
	cMatrix.Add(1, 1, 3);
	cMatrix.Add(1, 2, 1);
	cMatrix.Add(2, 1, 1);
	cMatrix.Add(2, 2, 2);
	afB[0] = 2;
	afB[1] = -2;
	afB[2] = 4;
	for (i = 0; i < 3; i++)
	{
		afX[i] = 0;
	}
	iIterations = Float64SparseSolve(&cMatrix, afX, afB, 1e-12, 10);
	AssertTrue((iIterations > 0) && (iIterations <= 3));
	AssertDouble(1.0, afX[0], 10);
	AssertDouble(-2.0, afX[1], 10);
	AssertDouble(3.0, afX[2], 10);
	AssertInt(0, Float64SparseSolve(&cMatrix, afX, afB, 1e-12, 10));

	for (i = 0; i < 3; i++)
	{
		afB[i] = 0;
		afX[i] = 0;
	}
	AssertInt(0, Float64SparseSolve(&cMatrix, afX, afB, 1e-12, 10));
	AssertDouble(0.0, afX[1], 10);
	cMatrix.Kill();

	cMatrix.Init(100, 100);
	for (i = 0; i < 100; i++)
	{
		cMatrix.Add(i, i, 2.01);
		if (i > 0)
		{
			cMatrix.Add(i, i - 1, -1);
		}
		if (i < 99)
		{
			cMatrix.Add(i, i + 1, -1);
		}
		afAnswer[i] = (i % 7) - (0.01 * i);
		afX[i] = 0;
	}
	cMatrix.Multiply(afB, afAnswer);
	iIterations = Float64SparseSolve(&cMatrix, afX, afB, 1e-12, 400);
	AssertTrue(iIterations > 0);
	AssertTrue(TestFloat64SparseWorst(afX, afAnswer, 100) < 1e-8);
	cMatrix.Kill();

	cMatrix.Init(2, 2);
	cMatrix.Add(0, 0, -1);
	cMatrix.Add(1, 1, -2);
	afB[0] = 1;
	afB[1] = 1;
	afX[0] = 0;
	afX[1] = 0;
	AssertInt(-1, Float64SparseSolve(&cMatrix, afX, afB, 1e-12, 10));
	cMatrix.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A line a + b t through (0, 1), (1, 3), (2, 4), (3, 8) and (4, 9) by
// least squares: the normal equations
//
//     5 10   a   25
//    10 30   b = 71
//
// make a 0.8 and b 2.1, in two iterations.  Then 300 equations in 60
// unknowns, each three of them that look random (and every unknown
// alone once, so they're independent), b made from an answer: the
// answer back.
//
//////////////////////////////////////////////////////////////////////////
void TestFloat64SparseMatrixLeastSquares(void)
{
	CFloat64SparseMatrix	cMatrix;
	float64					afAnswer[60];
	float64					afB[300];
	float64					afX[60];
	int						i;
	int						j;
	int						iIterations;
	uint32					uiSeed;

	cMatrix.Init(5, 2);
	for (i = 0; i < 5; i++)
	{
		cMatrix.Add(i, 0, 1);
		cMatrix.Add(i, 1, i);
	}
	afB[0] = 1;
	afB[1] = 3;
	afB[2] = 4;
	afB[3] = 8;
	afB[4] = 9;
	afX[0] = 0;
	afX[1] = 0;
	iIterations = Float64SparseLeastSquares(&cMatrix, afX, afB, 1e-12, 10);
	AssertTrue((iIterations > 0) && (iIterations <= 2));
	AssertDouble(0.8, afX[0], 10);
	AssertDouble(2.1, afX[1], 10);
	cMatrix.Kill();

	uiSeed = 1;
	cMatrix.Init(300, 60);
	for (i = 0; i < 60; i++)
	{
		cMatrix.Add(i, i, 1);
		afAnswer[i] = TestFloat64SparseNext(&uiSeed) * 10.0;
		afX[i] = 0;
	}
	for (i = 60; i < 300; i++)
	{
		for (j = 0; j < 3; j++)
		{
			cMatrix.Add(i, (int)((TestFloat64SparseNext(&uiSeed) + 1.0) * 29.99), TestFloat64SparseNext(&uiSeed));
		}
	}
	cMatrix.Multiply(afB, afAnswer);
	iIterations = Float64SparseLeastSquares(&cMatrix, afX, afB, 1e-12, 240);
	AssertTrue(iIterations > 0);
	AssertTrue(TestFloat64SparseWorst(afX, afAnswer, 60) < 1e-8);
	cMatrix.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat64SparseMatrix(void)
{
	BeginTests();

	TestFloat64SparseMatrixMultiply();
	TestFloat64SparseMatrixSolve();
	TestFloat64SparseMatrixLeastSquares();

	TestStatistics();
}

