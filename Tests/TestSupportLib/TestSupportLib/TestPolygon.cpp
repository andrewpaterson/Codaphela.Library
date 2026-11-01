#include "SupportLib/Polygon.h"
#include "SupportLib/Triangle.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPolygonAddTriangle(void)
{
	CPolygon	cPolygon;
	CTriangle	cTriangle1;
	CTriangle	cTriangle2;
	SFloat32Vec3		sPos1;
	SFloat32Vec3		sPos2;
	SFloat32Vec3		sPos3;
	SFloat32Vec3		sPos4;
	SFloat32Vec3		sNormal;

	sNormal.Init(0.0f, 1.0f, 0.0f);
	sPos1.Init(25.00, -12.50, 50.00);
	sPos2.Init(25.00, -12.50, -50.00);
	sPos3.Init(-25.00, -12.50, 50.00);
	sPos4.Init(-25.00, -12.50, -50.00);

	cTriangle1.Init(&sPos1, &sPos2, &sPos3, &sNormal);
	cTriangle1.Set();

	cTriangle2.Init(&sPos2, &sPos4, &sPos3, &sNormal);
	cTriangle2.Set();

	cPolygon.Init();
	cPolygon.AddTriangle(&cTriangle1);
	cPolygon.AddTriangle(&cTriangle2);

	AssertSize(4, cPolygon.mapsPositions.NumElements());
	AssertPointer(&sPos1, *((SFloat32Vec3**)cPolygon.mapsPositions.Get(0)));
	AssertPointer(&sPos2, *((SFloat32Vec3**)cPolygon.mapsPositions.Get(1)));
	AssertPointer(&sPos4, *((SFloat32Vec3**)cPolygon.mapsPositions.Get(2)));
	AssertPointer(&sPos3, *((SFloat32Vec3**)cPolygon.mapsPositions.Get(3)));

	cPolygon.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPolygonFindIndiciesFacing(void)
{
	CPolygon	cPolygon;
	SFloat32Vec3	sPos[6];
	SFloat32Vec3		sNormal;
	SFloat32Vec3	sTest;
	float		fDeg;
	float		fLen;
	CArrayInt	cIndicies;
	//CChars		sz;
	//int			i;
	//int			iIndex;

	sPos[0].Init(-17.3f, 30.0f, 0.0f);
	sPos[1].Init( 17.3f, 30.0f, 0.0f);
	sPos[2].Init( 34.6f,  0.0f, 0.0f);
	sPos[3].Init( 17.3f,-30.0f, 0.0f);
	sPos[4].Init(-17.3f,-30.0f, 0.0f);
	sPos[5].Init(-34.6f,  0.0f, 0.0f);

	cPolygon.Init(&sNormal);
	cPolygon.AddPositions(&sPos[0], &sPos[1], &sPos[2]);
	cPolygon.AddPositions(&sPos[3], &sPos[4], &sPos[5]);

	fLen = 170.0f;
	for (fDeg = 0.0f; fDeg < 360.0f; fDeg += 5.0f)
	{
		sTest.Init(sinf(Deg2Rad(fDeg)) * fLen, cosf(Deg2Rad(fDeg)) * fLen, 0.0f);

		cIndicies.Init();
		cPolygon.FindIndicesOfVisibleHalfSpaces(&sTest, &cIndicies);

		//sz.Init();
		//for (i = 0; i < cIndicies.miUsedElements; i+=2)
		//{
		//	iIndex = cIndicies.GetValue(i);
		//	sz.Append(iIndex);
		//	sz.Append(" -> ");
		//	iIndex = cIndicies.GetValue(i+1);
		//	sz.Append(iIndex);
		//	sz.Append(", ");
		//}
		//sz.Append("\n");
		//OutputDebugString(sz.Text());
		//sz.Kill();

		cIndicies.Kill();
	}

	cPolygon.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPolygon(void)
{
	BeginTests();

	TestPolygonFindIndiciesFacing();
	TestPolygonAddTriangle();

	TestStatistics();
}

