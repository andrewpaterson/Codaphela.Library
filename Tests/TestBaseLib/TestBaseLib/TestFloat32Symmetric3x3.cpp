#include <math.h>
#include "BaseLib/Float32Symmetric3x3.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// How far each eigenvector is from what the matrix does to it (A v less
// its value times v), the worst of them, and the worst of how far apart
// from a right angle and a unit length they are.
//
//////////////////////////////////////////////////////////////////////////
static void TestFloat32Symmetric3x3Check(SFloat32Symmetric3x3* psMatrix, float32* pafValues, SFloat32Vec3* pasVectors, float32* pfWorstResidual, float32* pfWorstAxes)
{
	SFloat32Vec3	sProduct;
	SFloat32Vec3	sScaled;
	float32			fDot;
	float32			fResidual;
	float32			fOff;
	int				i;
	int				j;

	*pfWorstResidual = 0.0f;
	*pfWorstAxes = 0.0f;
	for (i = 0; i < 3; i++)
	{
		psMatrix->Multiply(&sProduct, &pasVectors[i]);
		sScaled = pasVectors[i] * pafValues[i];
		sProduct -= sScaled;
		fResidual = sProduct.Magnitude();
		if (fResidual > *pfWorstResidual)
		{
			*pfWorstResidual = fResidual;
		}
		for (j = 0; j < 3; j++)
		{
			fDot = Float32Vec3Dot(&pasVectors[i], &pasVectors[j]);
			fOff = fabsf(fDot - ((i == j) ? 1.0f : 0.0f));
			if (fOff > *pfWorstAxes)
			{
				*pfWorstAxes = fOff;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A diagonal matrix is its eigenvalues, largest first, along the axes.
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32Symmetric3x3Diagonal(void)
{
	SFloat32Symmetric3x3	sMatrix;
	float32					afValues[3];
	SFloat32Vec3			asVectors[3];

	sMatrix.Init(1, 0, 0, 3, 0, 2);
	sMatrix.Eigen(afValues, asVectors);
	AssertFloat(3.0f, afValues[0], 6);
	AssertFloat(2.0f, afValues[1], 6);
	AssertFloat(1.0f, afValues[2], 6);
	AssertFloat(1.0f, fabsf(asVectors[0].y), 6);
	AssertFloat(1.0f, fabsf(asVectors[1].z), 6);
	AssertFloat(1.0f, fabsf(asVectors[2].x), 6);

	sMatrix.Init();
	sMatrix.Eigen(afValues, asVectors);
	AssertFloat(0.0f, afValues[0], 6);
	AssertFloat(0.0f, afValues[2], 6);
	AssertFloat(3.0f, asVectors[0].x + asVectors[1].y + asVectors[2].z, 6);
}


//////////////////////////////////////////////////////////////////////////
//
// A full matrix: each vector is turned into itself times its value, they
// are units at right angles, the values add to the trace and multiply to
// the determinant.  Two equal values ((2, 1, 0), (1, 2, 0), (0, 0, 3) has
// 3, 3 and 1) still give vectors at right angles.
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32Symmetric3x3Full(void)
{
	SFloat32Symmetric3x3	sMatrix;
	float32					afValues[3];
	SFloat32Vec3			asVectors[3];
	float32					fResidual;
	float32					fAxes;
	float32					fDeterminant;

	sMatrix.Init(4, 1, 2, 3, 0.5f, 5);
	sMatrix.Eigen(afValues, asVectors);
	TestFloat32Symmetric3x3Check(&sMatrix, afValues, asVectors, &fResidual, &fAxes);
	AssertTrue(fResidual < 0.00001f);
	AssertTrue(fAxes < 0.00001f);
	AssertTrue(afValues[0] >= afValues[1]);
	AssertTrue(afValues[1] >= afValues[2]);
	AssertFloat(12.0f, afValues[0] + afValues[1] + afValues[2], 4);
	fDeterminant = (4 * ((3 * 5) - (0.5f * 0.5f))) - (1 * ((1 * 5) - (0.5f * 2))) + (2 * ((1 * 0.5f) - (3 * 2)));
	AssertFloat(fDeterminant, afValues[0] * afValues[1] * afValues[2], 3);

	sMatrix.Init(2, 1, 0, 2, 0, 3);
	sMatrix.Eigen(afValues, asVectors);
	TestFloat32Symmetric3x3Check(&sMatrix, afValues, asVectors, &fResidual, &fAxes);
	AssertTrue(fResidual < 0.00001f);
	AssertTrue(fAxes < 0.00001f);
	AssertFloat(3.0f, afValues[0], 5);
	AssertFloat(3.0f, afValues[1], 5);
	AssertFloat(1.0f, afValues[2], 5);
}


//////////////////////////////////////////////////////////////////////////
//
// A'A from rows: solved where it has rank, and nothing added along what
// it hasn't.  A value under the cut off of the largest counts as none.
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32Symmetric3x3PseudoInverse(void)
{
	SFloat32Symmetric3x3	sMatrix;
	SFloat32Vec3			sRow;
	SFloat32Vec3			sVector;
	SFloat32Vec3			sResult;
	SFloat32Vec3			sCheck;

	sMatrix.Init(4, 1, 2, 3, 0.5f, 5);
	sVector.Init(1, 2, 3);
	sMatrix.PseudoInverseMultiply(&sResult, &sVector, 0.1f);
	sMatrix.Multiply(&sCheck, &sResult);
	AssertFloat3(sVector, &sCheck, 4);

	sMatrix.Init();
	sRow.Init(0.6f, 0.8f, 0);
	sMatrix.AddOuter(&sRow);
	AssertFloat(0.36f, sMatrix.xx, 6);
	AssertFloat(0.48f, sMatrix.xy, 6);
	AssertFloat(0.64f, sMatrix.yy, 6);
	sMatrix.PseudoInverseMultiply(&sResult, &sVector, 0.1f);
	AssertFloat3(SFloat32Vec3(1.32f, 1.76f, 0.0f), &sResult, 5);

	sRow.Init(0, 0, 1);
	sMatrix.AddOuter(&sRow);
	sMatrix.PseudoInverseMultiply(&sResult, &sVector, 0.1f);
	AssertFloat3(SFloat32Vec3(1.32f, 1.76f, 3.0f), &sResult, 5);

	sMatrix.Init(1, 0, 0, 0.001f, 0, 1);
	sVector.Init(1, 1, 1);
	sMatrix.PseudoInverseMultiply(&sResult, &sVector, 0.01f);
	AssertFloat3(SFloat32Vec3(1.0f, 0.0f, 1.0f), &sResult, 5);
	sMatrix.PseudoInverseMultiply(&sResult, &sVector, 0.0001f);
	AssertFloat3(SFloat32Vec3(1.0f, 1000.0f, 1.0f), &sResult, 2);

	sMatrix.Init();
	sMatrix.PseudoInverseMultiply(&sResult, &sVector, 0.1f);
	AssertFloat3(SFloat32Vec3(0.0f, 0.0f, 0.0f), &sResult, 6);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32Symmetric3x3(void)
{
	BeginTests();

	TestFloat32Symmetric3x3Diagonal();
	TestFloat32Symmetric3x3Full();
	TestFloat32Symmetric3x3PseudoInverse();

	TestStatistics();
}

