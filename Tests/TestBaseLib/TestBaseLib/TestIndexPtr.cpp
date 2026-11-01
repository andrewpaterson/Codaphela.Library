#include "BaseLib/GlobalMemory.h"
#include "BaseLib/LogString.h"
#include "BaseLib/LocalMallocatorConstructors.h"
#include "BaseLib/DataOrderers.h"
#include "BaseLib/IndexPtr.h"
#include "BaseLib/ArrayBit.h"
#include "TestLib/Assert.h"


struct SIndexTemplateKey
{
	double	f;
	char	c;
};


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void TestIndexPtrGet(void)
{
	CIndexPtr		cIndex;
	void*			pvv1;
	void*			pvv2;
	void*			pvv3;
	void*			pvv4;
	void*			pvv5;
	void*			pvv6;
	int64			uiK1;
	int64			uiK2;
	int64			uiK3;
	int64			uiK4;
	int64			uiK5;
	int64			uiK6;
	size			iKeySize;
	void*			pvValue;

	pvv1 = (void*)0x00723764;
	pvv2 = (void*)0x002ff462;
	pvv3 = (void*)0x00198201;
	pvv4 = (void*)0x0019abcd;
	pvv5 = (void*)0x002ff983;
	pvv6 = (void*)0x00ffffff;
	uiK1 = 1;
	uiK2 = 2;
	uiK3 = 3;
	uiK4 = 4;
	uiK5 = 5;
	uiK6 = 6;

	iKeySize = sizeof(int64);
	cIndex.Init();
	cIndex.Put((uint8*)&uiK1, iKeySize, pvv1);
	cIndex.Put((uint8*)&uiK2, iKeySize, pvv2);
	cIndex.Put((uint8*)&uiK3, iKeySize, pvv3);
	cIndex.Put((uint8*)&uiK4, iKeySize, pvv4);
	cIndex.Put((uint8*)&uiK5, iKeySize, pvv5);
	cIndex.Put((uint8*)&uiK6, iKeySize, pvv6);
	AssertSize(6, cIndex.NumElements());

	pvValue = cIndex.GetPtr((uint8*)&uiK1, iKeySize);
	AssertPointer(pvv1, pvValue);
	pvValue = cIndex.GetPtr((uint8*)&uiK2, iKeySize);
	AssertPointer(pvv2, pvValue);
	pvValue = cIndex.GetPtr((uint8*)&uiK3, iKeySize);
	AssertPointer(pvv3, pvValue);
	pvValue = cIndex.GetPtr((uint8*)&uiK4, iKeySize);
	AssertPointer(pvv4, pvValue);
	pvValue = cIndex.GetPtr((uint8*)&uiK5, iKeySize);
	AssertPointer(pvv5, pvValue);
	pvValue = cIndex.GetPtr((uint8*)&uiK6, iKeySize);
	AssertPointer(pvv6, pvValue);

	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void TestIndexPtrIterate(void)
{
	CIndexPtr						cIndex;
	size							ui;
	int64							iKey;
	char*							szValue;
	bool							bValid;
	char*							szMemory;
	SIndexTreeMemoryUnsafeIterator  sIter1;
	SIndexTreeMemoryIterator		sIter2;
	char**							pszResult;
	size							uiKeyLength;
	bool							bExists;
	char							szExpected[32];
	char*							pszData;
	CArrayBit						aBit;

	memset(szExpected, 0, 6);
	cIndex.Init();

	szMemory = (char*)malloc((lsize)12000 * 6);
	bValid = true;
	for (ui = 0; ui < 12000; ui++)
	{
		iKey = (int64)ui * 0x39e7;
		szValue = (char*)RemapSinglePointer(szMemory, (lsize)ui * 6);
		SizeToString(szValue, 6, ui);
		bValid &= cIndex.Put((uint8*)&iKey, sizeof(int64), szValue);
		if (!bValid)
		{
			AssertTrue(bValid);
		}
	}
	AssertTrue(bValid);

	aBit.Init();
	aBit.GrowTo(12000);
	aBit.Zero();
	bExists = cIndex.StartIteration(&sIter1, &iKey, &uiKeyLength, sizeof(int64), (void**)&pszResult);
	ui = 0;
	bValid = true;
	while (bExists)
	{
		iKey = iKey / 0x39e7;
		aBit.Set((size)iKey, true);
		IntToString(szExpected, 6, (int32)iKey);
		bValid &= (StringCompare(szExpected, *pszResult) == 0);
		if (!bValid)
		{
			AssertTrue(bValid);
		}
		bExists = cIndex.Iterate(&sIter1, &iKey, &uiKeyLength, sizeof(int64), (void**)&pszResult);
		ui++;
	}
	AssertSize(12000, ui);
	AssertTrue(bValid);

	bValid = true;
	for (ui = 0; ui < 12000; ui++)
	{
		bValid &= aBit.Get(ui);
		if (!bValid)
		{
			AssertTrue(bValid);
		}
	}
	AssertTrue(bValid);
	aBit.Kill();

	aBit.Init();
	aBit.GrowTo(12000);
	aBit.Zero();
	bExists = cIndex.StartIteration(&sIter2, &iKey, &uiKeyLength, sizeof(int64), (void**)&pszData);
	ui = 0;
	bValid = true;
	while (bExists)
	{
		iKey = iKey / 0x39e7;
		aBit.Set((size)iKey, true);
		IntToString(szExpected, 6, (int32)iKey);
		bValid &= (StringCompare(szExpected, pszData) == 0);
		if (!bValid)
		{
			AssertTrue(bValid);
		}
		bExists = cIndex.Iterate(&sIter2, &iKey, &uiKeyLength, sizeof(int64), (void**)&pszData);
		ui++;
	}
	AssertSize(12000, ui);
	AssertTrue(bValid);

	bValid = true;
	for (ui = 0; ui < 12000; ui++)
	{
		bValid &= aBit.Get(ui);
		if (!bValid)
		{
			AssertTrue(bValid);
		}
	}
	AssertTrue(bValid);
	aBit.Kill();

	free(szMemory);
	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexPtr(void)
{
	BeginTests();
	MemoryInit();
	LocalMallocatorsInit(&gcConstructors, &gcMallocators);
	DataOrderersInit(&gcConstructors, &gcDataOrderers);

	TestIndexPtrGet();
	TestIndexPtrIterate();

	DataOrderersKill();
	MemoryKill();
	TestStatistics();
}

