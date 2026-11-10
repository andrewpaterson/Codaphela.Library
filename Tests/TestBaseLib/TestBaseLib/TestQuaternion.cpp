#include <math.h>
#include "BaseLib/Float4x4.h"
#include "BaseLib/Float32Vec3.h"
#include "BaseLib/Quaternion.h"
#include "TestLib/Assert.h"


#define TEST_QUATERNION_PI		3.14159265358979f


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertTestQuaternionMatrix(SFloat4x4* psExpected, SFloat4x4* psActual)
{
	AssertFloat(psExpected->x.x, psActual->x.x, 4);
	AssertFloat(psExpected->x.y, psActual->x.y, 4);
	AssertFloat(psExpected->x.z, psActual->x.z, 4);
	AssertFloat(psExpected->y.x, psActual->y.x, 4);
	AssertFloat(psExpected->y.y, psActual->y.y, 4);
	AssertFloat(psExpected->y.z, psActual->y.z, 4);
	AssertFloat(psExpected->z.x, psActual->z.x, 4);
	AssertFloat(psExpected->z.y, psActual->z.y, 4);
	AssertFloat(psExpected->z.z, psActual->z.z, 4);
	AssertFloat(psExpected->pos.x, psActual->pos.x, 4);
	AssertFloat(psExpected->pos.y, psActual->pos.y, 4);
	AssertFloat(psExpected->pos.z, psActual->pos.z, 4);
	AssertFloat(psExpected->pos.w, psActual->pos.w, 4);
}


//////////////////////////////////////////////////////////////////////////
//
// A turn about z is (0, 0, sin(a / 2), cos(a / 2)), and goes to the same
// matrix as Float4x4RotationZ and back again.
//
//////////////////////////////////////////////////////////////////////////
void TestQuaternionRotation(void)
{
	SQuaternion	sRotation;
	SQuaternion	sBack;
	SFloat4x4	sExpected;
	SFloat4x4	sMatrix;
	float		fAngle;

	fAngle = TEST_QUATERNION_PI / 3.0f;
	sRotation = SQuaternion(0.0f, 0.0f, sinf(fAngle / 2.0f), cosf(fAngle / 2.0f));
	Float4x4RotationZ(&sExpected, fAngle);
	Float4x4RotationQuaternion(&sMatrix, &sRotation);
	AssertTestQuaternionMatrix(&sExpected, &sMatrix);

	QuaternionRotationMatrix(&sBack, &sMatrix);
	AssertFloat(1.0f, fabsf(QuaternionDot(&sRotation, &sBack)), 4);

	//A half turn about each axis, where w is 0, comes back too.
	Float4x4RotationX(&sMatrix, TEST_QUATERNION_PI);
	QuaternionRotationMatrix(&sBack, &sMatrix);
	AssertFloat(1.0f, fabsf(sBack.x), 4);
	Float4x4RotationY(&sMatrix, TEST_QUATERNION_PI);
	QuaternionRotationMatrix(&sBack, &sMatrix);
	AssertFloat(1.0f, fabsf(sBack.y), 4);
	Float4x4RotationZ(&sMatrix, TEST_QUATERNION_PI);
	QuaternionRotationMatrix(&sBack, &sMatrix);
	AssertFloat(1.0f, fabsf(sBack.z), 4);
}


//////////////////////////////////////////////////////////////////////////
//
// From no turn to a quarter turn about z, slerp is the turn that far along,
// and goes the shorter way when the second is given as its negative.
//
//////////////////////////////////////////////////////////////////////////
void TestQuaternionSlerp(void)
{
	SQuaternion	sFrom;
	SQuaternion	sTo;
	SQuaternion	sNegativeTo;
	SQuaternion	sHalf;
	float		fAngle;

	fAngle = TEST_QUATERNION_PI / 2.0f;
	sFrom = SQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
	sTo = SQuaternion(0.0f, 0.0f, sinf(fAngle / 2.0f), cosf(fAngle / 2.0f));

	QuaternionSlerp(&sHalf, &sFrom, &sTo, 0.5f);
	AssertFloat(sinf(fAngle / 4.0f), sHalf.z, 4);
	AssertFloat(cosf(fAngle / 4.0f), sHalf.w, 4);

	QuaternionSlerp(&sHalf, &sFrom, &sTo, 0.0f);
	AssertFloat(1.0f, sHalf.w, 4);
	QuaternionSlerp(&sHalf, &sFrom, &sTo, 1.0f);
	AssertFloat(sTo.z, sHalf.z, 4);

	sNegativeTo = -sTo;
	QuaternionSlerp(&sHalf, &sFrom, &sNegativeTo, 0.5f);
	AssertFloat(sinf(fAngle / 4.0f), sHalf.z, 4);
	AssertFloat(cosf(fAngle / 4.0f), sHalf.w, 4);

	//Almost the same, they're blended straight.
	sTo = SQuaternion(0.0f, 0.0f, 0.001f, 1.0f);
	QuaternionSlerp(&sHalf, &sFrom, &sTo, 0.5f);
	AssertFloat(0.0005f, sHalf.z, 5);
	AssertFloat(1.0f, QuaternionDot(&sHalf, &sHalf), 4);
}


//////////////////////////////////////////////////////////////////////////
//
// A matrix scaled along each axis, turned and moved splits into its parts
// and goes back together, a mirrored one with a negative z scale.
//
//////////////////////////////////////////////////////////////////////////
void TestQuaternionDecompose(void)
{
	SFloat4x4		sRotation;
	SFloat4x4		sMatrix;
	SFloat4x4		sBack;
	SFloat32Vec3	sScale;
	SFloat32Vec3	sPosition;
	SQuaternion		sTurn;

	Float4x4RotationY(&sRotation, 0.7f);
	sMatrix = sRotation;
	sMatrix.x.x *= 2.0f;	sMatrix.x.y *= 2.0f;	sMatrix.x.z *= 2.0f;
	sMatrix.y.x *= 3.0f;	sMatrix.y.y *= 3.0f;	sMatrix.y.z *= 3.0f;
	sMatrix.z.x *= 0.5f;	sMatrix.z.y *= 0.5f;	sMatrix.z.z *= 0.5f;
	sMatrix.pos.Init(1.0f, -2.0f, 5.0f, 1.0f);

	Float4x4Decompose(&sScale, &sTurn, &sPosition, &sMatrix);
	AssertFloat(2.0f, sScale.x, 4);
	AssertFloat(3.0f, sScale.y, 4);
	AssertFloat(0.5f, sScale.z, 4);
	AssertFloat(sinf(0.35f), sTurn.y, 4);
	AssertFloat(cosf(0.35f), sTurn.w, 4);
	AssertFloat(-2.0f, sPosition.y, 4);
	Float4x4Compose(&sBack, &sScale, &sTurn, &sPosition);
	AssertTestQuaternionMatrix(&sMatrix, &sBack);

	//Mirrored in z.
	sMatrix.z.x = -sMatrix.z.x;	sMatrix.z.y = -sMatrix.z.y;	sMatrix.z.z = -sMatrix.z.z;
	Float4x4Decompose(&sScale, &sTurn, &sPosition, &sMatrix);
	AssertFloat(-0.5f, sScale.z, 4);
	AssertFloat(sinf(0.35f), sTurn.y, 4);
	Float4x4Compose(&sBack, &sScale, &sTurn, &sPosition);
	AssertTestQuaternionMatrix(&sMatrix, &sBack);

	//The identity has no scale, turn or position to speak of.
	sMatrix.Identity();
	Float4x4Decompose(&sScale, &sTurn, &sPosition, &sMatrix);
	AssertFloat(1.0f, sScale.x, 4);
	AssertFloat(1.0f, sTurn.w, 4);
	AssertFloat(0.0f, sPosition.x, 4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestQuaternion(void)
{
	BeginTests();

	TestQuaternionRotation();
	TestQuaternionSlerp();
	TestQuaternionDecompose();

	TestStatistics();
}
