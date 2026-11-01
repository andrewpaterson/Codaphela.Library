#include "BaseLib/IndexStringTemplate.h"
#include "BaseLib/IndexStringInt.h"
#include "BaseLib/IndexStringPrimitive.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexStringTemplate(void)
{
	CIndexStringTemplate<double>	cIndex;
	double							d;
	double*							pd;
	char							szA[] = "A";
	bool							bResult;

	cIndex.Init();
	cIndex.Kill();

	cIndex.Init();
	d = 1.0; bResult = cIndex.Put("A", &d);
	d = 2.0; bResult &= cIndex.Put("B", &d);
	d = 3.0; bResult &= cIndex.Put("C", &d);
	AssertTrue(bResult);

	pd = cIndex.Get("B");
	AssertDouble(2.0, *pd, 2);

	pd = cIndex.Get("C");
	AssertDouble(3.0, *pd, 2);

	pd = cIndex.Get("A");
	AssertDouble(1.0, *pd, 2);

	pd = cIndex.Get(szA, szA);
	AssertDouble(1.0, *pd, 2);

	pd = cIndex.Put(szA, szA);
	AssertNotNull(pd);
	*pd = 55.0;
	AssertDouble(55.0, *pd, 2);

	pd = cIndex.Get("A");
	AssertDouble(55.0, *pd, 2);

	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexStringInt(void)
{
	CIndexStringInt					cIndex;
	int								iResult;
	SIndexTreeMemoryUnsafeIterator	sIter;
	size							uiKeySize;
	char							szDestKey[32];

	cIndex.Init();

	AssertTrue(cIndex.Put("EZ", 15));
	AssertSize(1, cIndex.NumElements());
	AssertTrue(cIndex.Put("Dock", 17));
	AssertSize(2, cIndex.NumElements());

	AssertTrue(cIndex.StartIteration(&sIter, &iResult, szDestKey, &uiKeySize, 32));
	AssertString("Dock", szDestKey);
	AssertInt(17, iResult);

	iResult = cIndex.Get("EZ");
	AssertTrue(iResult != -1);
	AssertInt(15, iResult);

	cIndex.Put("KingWin", 13);
	AssertSize(3, cIndex.NumElements());

	iResult = cIndex.Get("EZ");
	AssertInt(15, iResult);
	iResult = cIndex.Get("KingWin");
	AssertInt(13, iResult);
	iResult = cIndex.Get("Dock");
	AssertInt(17, iResult);
	iResult = cIndex.Get("KWM");
	AssertInt(-1, iResult);

	cIndex.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexStringPrimitive(void)
{
	CIndexStringPrimitive<int64>	cIndex;
	char							szBall[] = "Ball";
	SIndexTreeMemoryUnsafeIterator	sIter;
	int64							lli;
	char							szKey[32];
	size							iKeySize;
	char							szGrip[] = "Grip";

	cIndex.Init();
	AssertTrue(cIndex.Put(szBall, 0x298CC45731D2LL, &szBall[3]));

	AssertTrue(cIndex.StartIteration(&sIter, &lli, szKey, &iKeySize, 32));
	AssertLong(0x298CC45731D2LL, lli);
	AssertString("Ball", szKey);

	AssertFalse(cIndex.Iterate(&sIter, &lli, szKey, &iKeySize, 32));

	AssertTrue(cIndex.Put(szGrip, 0xC7689F79B2498LL));

	AssertTrue(cIndex.StartIteration(&sIter, &lli, szKey, &iKeySize, 32));
	AssertLong(0x298CC45731D2LL, lli);
	AssertString("Ball", szKey);

	AssertTrue(cIndex.Iterate(&sIter, &lli, szKey, &iKeySize, 32));
	AssertLong(0xC7689F79B2498LL, lli);
	AssertString("Grip", szKey);

	AssertFalse(cIndex.Iterate(&sIter, &lli, szKey, &iKeySize, 32));

	lli = cIndex.Get(&szGrip[0], -1, &szGrip[2]);
	AssertLong(-1, lli);
	lli = cIndex.Get(&szGrip[0], -1, &szGrip[3]);
	AssertLong(0xC7689F79B2498LL, lli);
	lli = cIndex.Get(&szGrip[0], -1);
	AssertLong(0xC7689F79B2498LL, lli);

	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexString(void)
{
	BeginTests();

	TestIndexStringTemplate();
	TestIndexStringPrimitive();
	TestIndexStringInt();

	TestStatistics();
}

