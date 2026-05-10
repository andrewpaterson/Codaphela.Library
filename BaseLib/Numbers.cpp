#include "Float32Vec2.h"
#include "Float64Vec2.h"
#include "Numbers.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float32 Deg2Dot(float32 fDegrees)
{
	SFloat32Vec2	s1;
	SFloat32Vec2	s2;
	float32			fRad;

	s1.Init(1.0f, 0.0f);
	fRad = Deg2Rad(fDegrees);
	s2.Init(cosf(fRad), sinf(fRad));

	return Float32Vec2Dot(&s1, &s2);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float64 Deg2Dot(float64 fDegrees)
{
	SFloat64Vec2	s1;
	SFloat64Vec2	s2;
	float64			fRad;

	s1.Init(1.0f, 0.0f);
	fRad = Deg2Rad(fDegrees);
	s2.Init(cos(fRad), sin(fRad));

	return Float64Vec2Dot(&s1, &s2);
}

