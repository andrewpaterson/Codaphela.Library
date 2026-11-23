#include <stdio.h>
#include "BaseLib/HashMapTemplate.h"
#include "BaseLib/HashSetTemplate.h"
#include "BaseLib/HashMapString.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/MemoryFile.h"
#include "TestLib/Assert.h"


//Every key hashes the same, so every lookup probes from one slot (the last of the first table, so runs wrap).
struct SHashOfCollide
{
	static uint32		Hash(const int32* piKey) { return 15; }
	static HashEquals	Equals(void) { return NULL; }
};


struct STestHashKey
{
	int32	iX;
	int32	iY;
	int32	iZ;
};


//A struct key compared by its fields (by a HashEquals, not its bytes).
struct SHashOfTestKey
{
	static uint32 Hash(const STestHashKey* psKey)
	{
		return HashMix32((uint32)psKey->iX * 31u + (uint32)psKey->iY * 7u + (uint32)psKey->iZ);
	}

	static HashEquals Equals(void)
	{
		return &SHashOfTestKey::KeyEquals;
	}

	static bool KeyEquals(const void* pvKey, const void* pvEntryKey)
	{
		const STestHashKey*	ps1;
		const STestHashKey*	ps2;

		ps1 = (const STestHashKey*)pvKey;
		ps2 = (const STestHashKey*)pvEntryKey;
		return (ps1->iX == ps2->iX) && (ps1->iY == ps2->iY) && (ps1->iZ == ps2->iZ);
	}
};


class CTestHashDataFree : public CDataFree
{
public:
	int		miFreed;

	void FreeData(void* pvData) override
	{
		miFreed++;
	}
};


static uint32 TestHashNext(uint32* puiState)
{
	*puiState = *puiState * 1103515245u + 12345u;
	return *puiState >> 8;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestHashMapPutGet(void)
{
	CHashMapTemplate<int32, int64>	cMap;
	int32							i;
	int32							iKey;
	int64*							plli;
	bool							bAdded;

	cMap.Init();
	AssertSize(0, cMap.NumElements());
	AssertTrue(cMap.IsEmpty());
	iKey = 7;
	AssertNull(cMap.Get(&iKey));
	AssertFalse(cMap.HasKey(&iKey));
	AssertFalse(cMap.Remove(&iKey));

	for (i = 0; i < 3000; i++)
	{
		iKey = i * 3;
		plli = cMap.FindOrAdd(&iKey, &bAdded);
		AssertTrue(bAdded);
		AssertNotNull(plli);
		AssertLong(0, *plli);
		*plli = (int64)i * 1000;
	}
	AssertSize(3000, cMap.NumElements());
	AssertTrue(cMap.IsEmpty() == false);

	for (i = 0; i < 3000; i++)
	{
		iKey = i * 3;
		plli = cMap.Get(&iKey);
		AssertNotNull(plli);
		AssertLong((int64)i * 1000, *plli);
		iKey = i * 3 + 1;
		AssertNull(cMap.Get(&iKey));
	}

	//The order added.
	for (i = 0; i < 3000; i++)
	{
		AssertInt(i * 3, *cMap.GetKey(i));
		AssertLong((int64)i * 1000, *cMap.GetValue(i));
	}

	//Put of a key that's in gives its value.
	iKey = 30;
	plli = cMap.FindOrAdd(&iKey, &bAdded);
	AssertFalse(bAdded);
	AssertLong(10000, *plli);
	AssertSize(3000, cMap.NumElements());
	AssertSize(10, cMap.Find(&iKey));

	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestHashMapRemove(void)
{
	CHashMapTemplate<int32, int32>	cMap;
	int32							i;
	int32							iKey;
	int32							iValue;

	cMap.Init();
	for (i = 0; i < 5; i++)
	{
		iKey = i;
		iValue = i * 10;
		AssertTrue(cMap.Put(&iKey, &iValue));
	}

	//The last entry takes the removed one's index.
	iKey = 1;
	AssertTrue(cMap.Remove(&iKey));
	AssertSize(4, cMap.NumElements());
	AssertInt(0, *cMap.GetKey(0));
	AssertInt(4, *cMap.GetKey(1));
	AssertInt(2, *cMap.GetKey(2));
	AssertInt(3, *cMap.GetKey(3));
	AssertInt(40, *cMap.GetValue(1));
	AssertFalse(cMap.HasKey(&iKey));
	AssertFalse(cMap.Remove(&iKey));

	//The last itself.
	iKey = 3;
	AssertTrue(cMap.Remove(&iKey));
	AssertSize(3, cMap.NumElements());
	AssertInt(2, *cMap.GetKey(2));

	cMap.RemoveAt(0);
	AssertSize(2, cMap.NumElements());
	AssertInt(2, *cMap.GetKey(0));
	AssertInt(4, *cMap.GetKey(1));
	iKey = 4;
	AssertInt(40, *cMap.Get(&iKey));
	iKey = 2;
	AssertInt(20, *cMap.Get(&iKey));

	cMap.RemoveAt(1);
	cMap.RemoveAt(0);
	AssertTrue(cMap.IsEmpty());
	AssertNull(cMap.Get(&iKey));

	iKey = 9;
	iValue = 90;
	AssertTrue(cMap.Put(&iKey, &iValue));
	AssertInt(90, *cMap.Get(&iKey));

	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every key in one probe run, which wraps past the table's end: removing
// from its start, middle and end must leave every other key found.
//
//////////////////////////////////////////////////////////////////////////
void TestHashMapCollisions(void)
{
	CHashMapTemplate<int32, int32, SHashOfCollide>	cMap;
	int32											i;
	int32											j;
	int32											iKey;
	int32											iValue;
	int32											aiRemoved[] = { 3, 0, 6, 5, 1 };
	bool											abRemoved[7];

	cMap.Init();
	for (i = 0; i < 7; i++)
	{
		iKey = i;
		iValue = i + 100;
		AssertTrue(cMap.Put(&iKey, &iValue));
		abRemoved[i] = false;
	}
	AssertSize(7, cMap.NumElements());

	for (j = 0; j < 5; j++)
	{
		iKey = aiRemoved[j];
		AssertTrue(cMap.Remove(&iKey));
		abRemoved[aiRemoved[j]] = true;
		for (i = 0; i < 7; i++)
		{
			iKey = i;
			if (abRemoved[i])
			{
				AssertNull(cMap.Get(&iKey));
			}
			else
			{
				AssertInt(i + 100, *cMap.Get(&iKey));
			}
		}
	}
	AssertSize(2, cMap.NumElements());

	//Put back in, probing the same run again.
	iKey = 3;
	iValue = 103;
	AssertTrue(cMap.Put(&iKey, &iValue));
	AssertInt(103, *cMap.Get(&iKey));
	iKey = 2;
	AssertInt(102, *cMap.Get(&iKey));
	iKey = 4;
	AssertInt(104, *cMap.Get(&iKey));

	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Random puts and removes, checked against a plain array.
//
//////////////////////////////////////////////////////////////////////////
void TestHashMapRandom(void)
{
	CHashMapTemplate<uint32, uint32>	cMap;
	uint32								auiValues[2000];
	uint32								uiState;
	uint32								ui;
	uint32								uiKey;
	uint32								uiValue;
	uint32*								pui;
	size								uiExpected;
	int									iStep;
	bool								bResult;

	cMap.Init();
	memset(auiValues, 0, sizeof(auiValues));
	uiState = 1;
	uiExpected = 0;
	bResult = true;
	for (iStep = 0; iStep < 20000; iStep++)
	{
		uiKey = TestHashNext(&uiState) % 2000;
		if (TestHashNext(&uiState) % 3 == 0)
		{
			if (auiValues[uiKey] != 0)
			{
				uiExpected--;
			}
			bResult &= (cMap.Remove(&uiKey) == (auiValues[uiKey] != 0));
			auiValues[uiKey] = 0;
		}
		else
		{
			uiValue = TestHashNext(&uiState) | 1;
			if (auiValues[uiKey] == 0)
			{
				uiExpected++;
			}
			bResult &= cMap.Put(&uiKey, &uiValue);
			auiValues[uiKey] = uiValue;
		}
	}
	AssertTrue(bResult);
	AssertSize(uiExpected, cMap.NumElements());
	for (ui = 0; ui < 2000; ui++)
	{
		pui = cMap.Get(&ui);
		if (auiValues[ui] == 0)
		{
			bResult &= (pui == NULL);
		}
		else
		{
			bResult &= (pui != NULL) && (*pui == auiValues[ui]);
		}
	}
	AssertTrue(bResult);

	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Put(pKey, pValue) stores a bool value (it isn't FindOrAdd's pbAdded).
//
//////////////////////////////////////////////////////////////////////////
void TestHashMapBoolValues(void)
{
	CHashMapTemplate<int32, bool>	cMap;
	int32							iKey;
	bool							bValue;

	cMap.Init();
	iKey = 1;
	bValue = true;
	AssertTrue(cMap.Put(&iKey, &bValue));
	iKey = 2;
	bValue = false;
	AssertTrue(cMap.Put(&iKey, &bValue));
	iKey = 1;
	AssertTrue(*cMap.Get(&iKey));
	iKey = 2;
	AssertFalse(*cMap.Get(&iKey));
	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint8 gauiTestHashObjects[512 * 16];  //Not on the stack: the W65C816's is in bank 0.


void TestHashMapPointerKeys(void)
{
	CHashMapTemplate<void*, size>	cMap;
	uint8*							auiObjects;
	void*							pv;
	size							i;
	size*							pui;

	auiObjects = gauiTestHashObjects;
	cMap.Init();
	for (i = 0; i < 512; i++)
	{
		pv = &auiObjects[i * 16];
		pui = cMap.Put(&pv);
		*pui = i;
	}
	AssertSize(512, cMap.NumElements());
	for (i = 0; i < 512; i++)
	{
		pv = &auiObjects[i * 16];
		AssertSize(i, *cMap.Get(&pv));
	}
	pv = &auiObjects[8];
	AssertNull(cMap.Get(&pv));
	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestHashMapStructKeys(void)
{
	CHashMapTemplate<STestHashKey, int32, SHashOfTestKey>	cMap;
	STestHashKey											sKey;
	int32													i;
	int32*													pi;

	cMap.Init();
	for (i = 0; i < 300; i++)
	{
		sKey.iX = i;
		sKey.iY = i / 7;
		sKey.iZ = -i;
		pi = cMap.Put(&sKey);
		*pi = i;
	}
	for (i = 0; i < 300; i++)
	{
		sKey.iX = i;
		sKey.iY = i / 7;
		sKey.iZ = -i;
		AssertInt(i, *cMap.Get(&sKey));
		sKey.iZ = i + 1;
		AssertNull(cMap.Get(&sKey));
	}
	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestHashMapDataFree(void)
{
	CHashMapTemplate<int32, int32>	cMap;
	CTestHashDataFree				cFree;
	int32							i;
	int32							iKey;

	cFree.miFreed = 0;
	cMap.Init();
	cMap.SetDataFreeCallback(&cFree);
	for (i = 0; i < 10; i++)
	{
		iKey = i;
		cMap.Put(&iKey);
	}
	iKey = 4;
	cMap.Remove(&iKey);
	AssertInt(1, cFree.miFreed);
	cMap.RemoveAt(0);
	AssertInt(2, cFree.miFreed);
	cMap.Kill();
	AssertInt(10, cFree.miFreed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestHashMapWriteRead(void)
{
	CHashMapTemplate<int32, int64>	cMap;
	CHashMapTemplate<int32, int64>	cMapIn;
	CHashMapTemplate<int32, int32>	cWrong;
	CFileBasic						cFile;
	int32							i;
	int32							iKey;
	int64							lli;

	cMap.Init();
	for (i = 0; i < 100; i++)
	{
		iKey = 99 - i;
		lli = (int64)i * i;
		cMap.Put(&iKey, &lli);
	}
	iKey = 50;
	cMap.Remove(&iKey);

	cFile.Init(MemoryFile());
	cFile.Open(EFM_Write_Create);
	AssertTrue(cMap.Write(&cFile));
	cFile.Close();

	cFile.Open(EFM_Read);
	AssertTrue(cMapIn.Read(&cFile));
	cFile.Close();

	AssertSize(99, cMapIn.NumElements());
	for (i = 0; i < 99; i++)
	{
		AssertInt(*cMap.GetKey(i), *cMapIn.GetKey(i));
		AssertLong(*cMap.GetValue(i), *cMapIn.GetValue(i));
	}
	iKey = 49;
	AssertLong(50 * 50, *cMapIn.Get(&iKey));
	iKey = 50;
	AssertNull(cMapIn.Get(&iKey));

	//A map of other sizes doesn't read it.
	cFile.Open(EFM_Read);
	AssertFalse(cWrong.Read(&cFile));
	cFile.Close();
	cWrong.Kill();

	cFile.Kill();
	cMapIn.Kill();
	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestHashMapLarge(void)
{
	CHashMapTemplate<int32, int32, SHashOf<int32>, SArrayLargeSize>	cMap;
	int32															i;
	int32															iKey;
	int32*															pi;

	cMap.Init();
	for (i = 0; i < 1000; i++)
	{
		iKey = i;
		pi = cMap.Put(&iKey);
		*pi = -i;
	}
	AssertLSize(1000, cMap.NumElements());
	iKey = 999;
	AssertInt(-999, *cMap.Get(&iKey));
	cMap.Kill();
}


struct STestHashBig
{
	uint8	auiBytes[1000];
};


//////////////////////////////////////////////////////////////////////////
//
// Where size is 16 bits a natural map holds at most MAX_SIZE bytes of
// entries: a Put past that (or past the memory free, which on the W65C816
// may come first) fails, and what was in stays.
//
//////////////////////////////////////////////////////////////////////////
void TestHashMapFull(void)
{
#if defined(SIZE_16)
	CHashMapTemplate<int32, STestHashBig>	cMap;
	int32									i;
	int32									iFilled;
	int32									iKey;
	STestHashBig*							ps;

	cMap.Init();
	for (i = 0; i < 100; i++)
	{
		iKey = i;
		ps = cMap.Put(&iKey);
		if (ps == NULL)
		{
			break;
		}
		ps->auiBytes[0] = (uint8)i;
		ps->auiBytes[999] = (uint8)(i + 1);
	}
	AssertTrue(i < 100);
	AssertTrue(i > 0);
	AssertSize((size)i, cMap.NumElements());
	iKey = i;
	AssertNull(cMap.Get(&iKey));
	iFilled = i;
	for (i = 0; i < iFilled; i++)
	{
		iKey = i;
		ps = cMap.Get(&iKey);
		AssertNotNull(ps);
		AssertInt(i, (int32)ps->auiBytes[0]);
		AssertInt(i + 1, (int32)ps->auiBytes[999]);
	}
	cMap.Kill();
#endif // SIZE_16
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestHashSet(void)
{
	CHashSetTemplate<int64>		cSet;
	CHashSetTemplate<SHashText, SHashOfText>	cNames;
	int64						lli;
	int64						i;
	SHashText					sName;
	const char*					szText = "alphabetagamma";

	cSet.Init();
	for (i = 0; i < 1000; i++)
	{
		lli = i * 1000003;
		AssertTrue(cSet.Add(&lli));
	}
	lli = 5 * 1000003;
	AssertFalse(cSet.Add(&lli));
	AssertTrue(cSet.HasKey(&lli));
	AssertLong(lli, *cSet.Get(&lli));
	AssertSize(5, cSet.Find(&lli));
	AssertTrue(cSet.Remove(&lli));
	AssertFalse(cSet.HasKey(&lli));
	AssertSize(999, cSet.NumElements());
	AssertLong(999 * 1000003LL, *cSet.GetKey(5));
	cSet.Kill();

	//Names that aren't NULL terminated, kept where they are.
	cNames.Init();
	sName.Init(szText, 5);
	AssertTrue(cNames.Add(&sName));
	sName.Init(&szText[5], 4);
	AssertTrue(cNames.Add(&sName));
	sName.Init(&szText[9], 5);
	AssertTrue(cNames.Add(&sName));
	sName.Init("beta");
	AssertFalse(cNames.Add(&sName));
	AssertTrue(cNames.HasKey(&sName));
	AssertPointer(&szText[5], cNames.Get(&sName)->sz);
	sName.Init("bet");
	AssertFalse(cNames.HasKey(&sName));
	sName.Init("alphabeta");
	AssertFalse(cNames.HasKey(&sName));
	AssertSize(3, cNames.NumElements());
	cNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestHashMapString(void)
{
	CHashMapString<int32>	cMap;
	char					sz[32];
	int32					i;
	int32*					pi;
	bool					bAdded;
	const char*				szText = "SymbolTable";

	cMap.Init();
	for (i = 0; i < 2000; i++)
	{
		sprintf(sz, "Name%i", i);
		pi = cMap.Put(sz);
		*pi = i;
	}
	AssertSize(2000, cMap.NumElements());

	//The map has its own copies.
	strcpy(sz, "Overwritten");
	for (i = 0; i < 2000; i++)
	{
		sprintf(sz, "Name%i", i);
		AssertInt(i, *cMap.Get(sz));
	}
	AssertString("Name0", cMap.GetKey(0));
	AssertString("Name1999", cMap.GetKey(1999));
	AssertSize(8, cMap.GetKeyLength(1999));
	AssertNull(cMap.Get("Name"));
	AssertNull(cMap.Get("Name2000"));

	//Keys that aren't NULL terminated.
	pi = cMap.FindOrAdd(szText, 6, &bAdded);
	AssertTrue(bAdded);
	*pi = -1;
	AssertInt(-1, *cMap.Get("Symbol"));
	AssertInt(-1, *cMap.Get(szText, 6));
	AssertNull(cMap.Get(szText));
	AssertString("Symbol", cMap.GetKey(2000));
	pi = cMap.FindOrAdd("Symbol", 6, &bAdded);
	AssertFalse(bAdded);
	AssertInt(-1, *pi);

	AssertTrue(cMap.Remove("Name5"));
	AssertFalse(cMap.HasKey("Name5"));
	AssertString("Symbol", cMap.GetKey(5));
	AssertSize(2000, cMap.NumElements());
	AssertTrue(cMap.HasKey("Name6"));

	cMap.ReInit();
	AssertSize(0, cMap.NumElements());
	AssertNull(cMap.Get("Name6"));
	i = 6;
	AssertTrue(cMap.Put("Name6", &i));
	AssertInt(6, *cMap.Get("Name6"));

	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestHashMap(void)
{
	BeginTests();

	TestHashMapPutGet();
	TestHashMapRemove();
	TestHashMapCollisions();
	TestHashMapRandom();
	TestHashMapBoolValues();
	TestHashMapPointerKeys();
	TestHashMapStructKeys();
	TestHashMapDataFree();
	TestHashMapWriteRead();
	TestHashMapLarge();
	TestHashMapFull();
	TestHashSet();
	TestHashMapString();

	TestStatistics();
}

