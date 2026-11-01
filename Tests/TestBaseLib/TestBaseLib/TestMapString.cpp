#include "BaseLib/MapStringString.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringTemplate(void)
{
	CMapStringTemplate<double>	cMap;
	double						d;
	double*						pd;
	char*						szKey;

	cMap.Init(false);
	cMap.Kill();

	cMap.Init(true);
	cMap.Kill();

	cMap.Init(true, true);
	d = 1.0; cMap.Put("A", &d);
	d = 2.0; cMap.Put("B", &d);
	d = 3.0; cMap.Put("C", &d);

	pd = cMap.Get("B");
	AssertDouble(2.0, *pd, 2);
	szKey = cMap.GetKeyForData(pd);
	AssertString("B", szKey);

	pd = cMap.Get("C");
	AssertDouble(3.0, *pd, 2);
	szKey = cMap.GetKeyForData(pd);
	AssertString("C", szKey);

	pd = cMap.Get("A");
	AssertDouble(1.0, *pd, 2);
	szKey = cMap.GetKeyForData(pd);
	AssertString("A", szKey);

	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringString(void)
{
	CMapStringString	cMap;

	cMap.Init();
	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringInt(void)
{
	CMapStringInt	cMap;
	int				iResult;
	int*			piResult;
	SMapIterator	sIter;
	char*			szKey;

	cMap.Init();

	AssertTrue(cMap.Put("EZ", 15));
	AssertSize(1, cMap.NumElements());
	AssertTrue(cMap.Put("Dock", 17));
	AssertSize(2, cMap.NumElements());

	AssertTrue(cMap.StartIteration(&sIter, (void**)&szKey, NULL, (void**)&piResult, NULL));
	AssertNotNull(szKey);
	AssertNotNull(piResult);
	AssertString("Dock", szKey);
	AssertInt(17, *piResult);

	piResult = cMap.CMapStringTemplate::Get("EZ");
	AssertNotNull(piResult);
	iResult = *piResult;
	AssertInt(15, iResult);

	cMap.Put("KingWin", 13);
	AssertSize(3, cMap.NumElements());

	iResult = *cMap.CMapStringTemplate::Get("EZ");
	AssertInt(15, iResult);
	iResult = *cMap.CMapStringTemplate::Get("KingWin");
	AssertInt(13, iResult);
	iResult = *cMap.CMapStringTemplate::Get("Dock");
	AssertInt(17, iResult);
	AssertNull(cMap.CMapStringTemplate::Get("KWM"));

	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapString(void)
{
	BeginTests();

	TestMapStringTemplate();
	TestMapStringInt();
	TestMapStringString();

	TestStatistics();
}

