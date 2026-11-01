#include "SupportLib/MeshFaceReturn.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshFaceReturn(void)
{
	CMFRet	r;
	int		iCorner1;
	int		iCorner2;
	int		iCorner3;
	int		iCorner4;
	int		iCorner5;
	int		iCorner6;
	CMFRet	r1;
	CMFRet	r2;

	AssertSize(8, sizeof(CMFRet));

	r.Pack(17, 5, 6, 7, 3);
	
	AssertInt(1, r.NumFaces());
	r.Unpack1(&iCorner1, &iCorner2, &iCorner3);
	
	AssertInt(5, iCorner1);
	AssertInt(6, iCorner2);
	AssertInt(7, iCorner3);

	AssertInt((int32)17, r.miFirstFace);
	AssertInt(3, r.NewEdges());

	r.Pack(34671, 15, 14, 13, 12, 11, 10, 5);

	AssertInt(2, r.NumFaces());
	r.Unpack1(&iCorner1, &iCorner2, &iCorner3);
	r.Unpack2(&iCorner4, &iCorner5, &iCorner6);

	AssertInt(15, iCorner1);
	AssertInt(14, iCorner2);
	AssertInt(13, iCorner3);
	AssertInt(12, iCorner4);
	AssertInt(11, iCorner5);
	AssertInt(10, iCorner6);

	AssertInt((int32)34671, r.miFirstFace);
	AssertInt(5, r.NewEdges());

	r.PackEmpty();

	AssertInt(0, r.NumFaces());

	r.PackOverflow();

	AssertInt(3, r.NumFaces());

	r1.Pack(17, 15, 14, 13, 3);
	r2.Pack(18, 12, 11, 10, 4);
	r1.Pack2(r2);

	AssertInt(2, r1.NumFaces());
	r1.Unpack1(&iCorner1, &iCorner2, &iCorner3);
	r1.Unpack2(&iCorner4, &iCorner5, &iCorner6);

	AssertInt(15, iCorner1);
	AssertInt(14, iCorner2);
	AssertInt(13, iCorner3);
	AssertInt(12, iCorner4);
	AssertInt(11, iCorner5);
	AssertInt(10, iCorner6);

	AssertInt((int32)17, r1.miFirstFace);
	AssertInt(7, r1.NewEdges());
}

