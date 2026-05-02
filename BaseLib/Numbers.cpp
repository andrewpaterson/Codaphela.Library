#include "Float32Vec2.h"
#include "Numbers.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float Deg2Dot(float fDegrees)
{
	SFloat32Vec2		s1;
	SFloat32Vec2		s2;
	float		fRad;

	s1.Init(1.0f, 0.0f);
	fRad = Deg2Rad(fDegrees);
	s2.Init(cosf(fRad), sinf(fRad));

	return Float2Dot(&s1, &s2);
}

