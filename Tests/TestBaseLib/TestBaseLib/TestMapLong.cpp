#include "BaseLib/GlobalMemory.h"
#include "BaseLib/MapLongLong.h"
#include "TestLib/Assert.h"


class CTestMapLongLong : public CMapLongLong
{
public:
	void InsertHoldingIntoSorted(void)
	{
		return CMapLongLong::InsertHoldingIntoSorted();
	}


	bool GetInSorted(int iIndex, void** ppvKey, void** ppvData)
	{
		return CMapLongLong::GetInSorted(iIndex, ppvKey, ppvData);
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapLongTemplate(void)
{
	CMapLongTemplate<double>	cMap;

	double	d;
	double	e;
	int64		i;

	cMap.Init(true);

	d = 1.0; cMap.Put((int64)d, &d);
	d = 2.0; cMap.Put((int64)d, &d);
	d = 3.0; cMap.Put((int64)d, &d);
	d = 4.0; cMap.Put((int64)d, &d);
	d = -1.0; cMap.Put((int64)d, &d);
	d = -2.0; cMap.Put((int64)d, &d);
	d = -3.0; cMap.Put((int64)d, &d);
	d = -4.0; cMap.Put((int64)d, &d);
	d = 0.0; cMap.Put((int64)d, &d);
	AssertLong(9, cMap.NumElements());

	AssertNull(cMap.Get(-5));
	e = -4.0; d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	e = -3.0; d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	e = -2.0; d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	e = -1.0; d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	e = 0.0; d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	e = 1.0; d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	e = 2.0; d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	e = 3.0; d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	e = 4.0; d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	e = 1.0; d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	e = 1.0; d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	AssertNull(cMap.Get(5));

	i = -4;
	AssertTrue(cMap.Remove(i));
	AssertLong(8, cMap.NumElements());
	AssertNull(cMap.Get(i));
	e = -3.0;
	d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	AssertFalse(cMap.Remove(i));

	i = -3;
	AssertTrue(cMap.Remove(i));
	AssertLong(7, cMap.NumElements());
	AssertNull(cMap.Get(i));
	e = -2.0;
	d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	AssertFalse(cMap.Remove(i));

	i = 3;
	AssertTrue(cMap.Remove(i));
	AssertLong(6, cMap.NumElements());
	AssertNull(cMap.Get(i));
	e = 4.0;
	d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	AssertFalse(cMap.Remove(i));

	i = 0;
	AssertTrue(cMap.Remove(i));
	AssertLong(5, cMap.NumElements());
	AssertNull(cMap.Get(i));
	e = 4.0;
	d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	i = 0;
	AssertFalse(cMap.Remove(i));

	i = 1;
	AssertTrue(cMap.Remove(i));
	AssertLong(4, cMap.NumElements());
	AssertNull(cMap.Get(i));
	e = -2.0;
	d = *cMap.Get((int64)e);
	AssertDouble(e, d, 2);
	AssertFalse(cMap.Remove(i));

	AssertTrue(cMap.Remove(-2));
	AssertTrue(cMap.Remove(-1));
	AssertTrue(cMap.Remove(4));
	AssertLong(1, cMap.NumElements());
	AssertTrue(cMap.Remove(2));
	AssertLong(0, cMap.NumElements());
	AssertFalse(cMap.Remove(2));

	cMap.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapLongLong(void)
{
	CTestMapLongLong	cMap;
	int64				iResult;
	int64*				piKey;
	int64*				piValue;

	cMap.Init(true);

	cMap.Put(5, 15);
	cMap.Put(7, 17);

	iResult = *cMap.Get(5);
	AssertLong(15, iResult);

	cMap.Put(3, 13);
	AssertLong(3, cMap.NumElements());

	iResult = *cMap.Get(5);
	AssertLong(15, iResult);
	iResult = *cMap.Get(3);
	AssertLong(13, iResult);
	iResult = *cMap.Get(7);
	AssertLong(17, iResult);
	AssertNull(cMap.Get(1));

	AssertLong(0, cMap.GetArray()->GetSortedSize());
	AssertLong(3, cMap.GetArray()->GetHoldingSize());
	AssertLong(8, (*(SMNode**)cMap.GetArray()->GetInHolding(0, 0))->iKeySize);

	cMap.InsertHoldingIntoSorted();
	cMap.GetInSorted(0, (void**)&piKey, (void**)&piValue);
	AssertLong(3, *piKey);
	AssertLong(13, *piValue);

	cMap.GetInSorted(1, (void**)&piKey, (void**)&piValue);
	AssertLong(5, *piKey);
	AssertLong(15, *piValue);

	cMap.GetInSorted(2, (void**)&piKey, (void**)&piValue);
	AssertLong(7, *piKey);
	AssertLong(17, *piValue);

	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapLong(void)
{
	BeginTests();
	MemoryInit();

	TestMapLongTemplate();
	TestMapLongLong();

	MemoryKill();
	TestStatistics();
}

