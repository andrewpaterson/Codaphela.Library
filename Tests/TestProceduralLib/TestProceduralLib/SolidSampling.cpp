#include "SolidSampling.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSolidSampleAt(Ptr<CSolidTexture> pTexture, float32 x, float32 y, float32 z, SSolidSample* psSample)
{
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sNormal;

	sPoint.Init(x, y, z);
	sNormal.Init(0, 0, 1);
	pTexture->Sample(&sPoint, &sNormal, 0.0f, psSample);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 TestSolidValueAt(Ptr<CSolidTexture> pTexture, float32 x, float32 y, float32 z)
{
	SSolidSample	sSample;

	TestSolidSampleAt(pTexture, x, y, z, &sSample);
	return sSample.Value();
}

