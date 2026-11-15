#ifndef __TEST_GYROID_H__
#define __TEST_GYROID_H__
#include <math.h>
#include "SupportLib/Shape.h"


//Schoen's gyroid, sin x cos y + sin y cos z + sin z cos x, at mfScale: a surface that winds through every cell of a grid
//whatever its size, so it meets each of a cube's cases, the ones with two inside corners across a face from each other too.
//Not a distance, and endless, so the tests cut it with a sphere.
class CTestGyroid : public CShape
{
CONSTRUCTABLE(CTestGyroid);
DESTRUCTABLE(CTestGyroid);
public:
	float32	mfScale;

	void Init(float32 fScale)
	{
		PreInit();
		CShape::Init();
		mfScale = fScale;
		PostInit();
	}

	void Class(void) override
	{
		CShape::Class();
		U_Float32(mfScale);
	}

	float32 Distance(SFloat32Vec3* psPosition) override
	{
		float32	x;
		float32	y;
		float32	z;

		x = psPosition->x * mfScale;
		y = psPosition->y * mfScale;
		z = psPosition->z * mfScale;
		return (sinf(x) * cosf(y)) + (sinf(y) * cosf(z)) + (sinf(z) * cosf(x));
	}

	void GetBounds(SFloat32Bounds* psDest) override
	{
		SFloat32Vec3	sLargest;

		sLargest.Init(FLOAT32_BOUNDS_LARGEST, FLOAT32_BOUNDS_LARGEST, FLOAT32_BOUNDS_LARGEST);
		psDest->sMaximum = sLargest;
		psDest->sMinimum = -sLargest;
	}

	bool IsExact(void) override
	{
		return false;
	}
};


#endif // __TEST_GYROID_H__

