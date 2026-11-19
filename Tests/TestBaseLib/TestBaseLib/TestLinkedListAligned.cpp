#include "BaseLib/LinkedListBlockAligned.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/DiskFile.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLinkedListBlockAlignedAdd(void)
{
	CLinkedListBlockAligned	cList;
	void*					pv;
	int						k;
	int						iAlignment;
	SLLAlignedNode*			psNode;
	void*					pvAllocatedEnd;
	void*					pvObjectEnd;
	size						iPrevTotalSize;

	cList.Init();

	iPrevTotalSize = 0;
	for (iAlignment = 2; iAlignment <= 48; iAlignment++)
	{
		for (k = 0; k < 5; k++)
		{
			pv = cList.InsertAfterTail(65, iAlignment, 0);
			AssertSize(0, (size)(((ptr)pv) % iAlignment));

			psNode = cList.GetNode(pv);
			pvAllocatedEnd = (void*)((uint8*)psNode->sAligned.pvAlloc + sizeof(SLLAlignedNode) + 65 + iAlignment -1);
			pvObjectEnd = (void*)((uint8*)pv + 65);
			AssertTrue(pvAllocatedEnd >= pvObjectEnd);
			AssertSize(iPrevTotalSize + ((sizeof(SLLAlignedNode) + 65 + iAlignment-1) * (k + 1)), cList.ByteSize());
		}
		iPrevTotalSize += ((sizeof(SLLAlignedNode) + 65 + iAlignment-1) * k);
	}
#ifdef BUILD_64
	AssertSize(30315, iPrevTotalSize);
#else
	AssertSize(25615, iPrevTotalSize);
#endif // BUILD_64

	cList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLinkedListBlockAlignedRemove(void)
{
	CLinkedListBlockAligned		cList;
	void*						pv1;
	void*						pv2;
	void*						pv3;
	void*						pv;

	cList.Init();
	pv1 = cList.InsertAfterTail(1, 4, 0);
	pv2 = cList.InsertAfterTail(2, 4, 0);
	pv3 = cList.InsertAfterTail(3, 4, 0);
	cList.Remove(pv2);

	pv = cList.GetHead();
	AssertPointer(pv1, pv);
	pv = cList.GetNext(pv);
	AssertPointer(pv3, pv);
	pv = cList.GetNext(pv);
	AssertNull(pv);

	cList.Remove(pv1);
	cList.Remove(pv3);

	pv = cList.GetHead();
	AssertNull(pv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLinkedListBlockAlignedGrow(void)
{
	CLinkedListBlockAligned		cList;
	int*						pi;
	void*						pvOld;
	void*						pvNew;

	cList.Init();
	pi = (int*)cList.InsertAfterTail(20, 4, 0);
	pi[0] = 0;
	pi[1] = 1;
	pi[2] = 2;
	pi[3] = 3;
	pi[4] = 4;
	
	pi = (int*)cList.Grow(pi, 24);
	AssertInt(0, pi[0]);
	AssertInt(1, pi[1]);
	AssertInt(2, pi[2]);
	AssertInt(3, pi[3]);
	AssertInt(4, pi[4]);

	pi[5] = 5;

	pi = (int*)cList.Grow(pi, 8);
	AssertInt(0, pi[0]);
	AssertInt(1, pi[1]);

	pi = (int*)cList.Grow(pi, 0);
	AssertNull(pi);
	cList.Kill();

	cList.Init();
	pvOld = cList.InsertAfterTail(1, 4, 0);
	pvNew = cList.Grow(pvOld, 2);
	AssertPointer(pvOld, pvNew);
	pvNew = cList.Grow(pvOld, 3);
	AssertPointer(pvOld, pvNew);
	pvNew = cList.Grow(pvOld, 3);
	AssertPointer(pvOld, pvNew);
	pvNew = cList.Grow(pvOld, 4);
	AssertPointer(pvOld, pvNew);
	pvNew = cList.Grow(pvOld, 8);
	AssertFalse(pvOld == pvNew);
	cList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLinkedListBlockAlignedExhaustive(void)
{
	CLinkedListBlockAligned		cList;
	uint8				pv[1 KB];
	int							iAlignment;
	uint32				uiDataSize;
	void*						pvNew;
	void*						pvData;
	int							i;
	int							iExpectedElements;
	SLLAlignedNode*				psNode;
	int							iResult;
	CDiskFile					cDiskFile;
	CFileBasic					cFile;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "LinkedListBlockAligned";
	char						szFile[] = "Output" _FS_ "LinkedListBlockAligned" _FS_ "Test.DAT";
	bool						bResult;
	char						sz[256];
	void*						pvNext;
	void*						pvPrev;

	memset(pv, 0, 1 KB);
	for (i = 0; i < 256; i++)
	{
		pv[i] = (uint8)i;
	}

	cList.Init();

	iExpectedElements = 0;
	for (iAlignment = 1; iAlignment <= 64; iAlignment++)
	{
		for (uiDataSize = 1; uiDataSize <= 128; uiDataSize++)
		{
			pvNew = cList.Add(uiDataSize, iAlignment);
			memcpy(pvNew, pv, uiDataSize);

			iExpectedElements++;
		}

		AssertSize(iExpectedElements, cList.NumElements());
	}

	AssertSize(8192, cList.NumElements());

	uiDataSize = 1;

	pvData = cList.GetHead();
	for (i = 0; i < 8192; i++)
	{
		iAlignment = (i + 1) / 128 + 1;
		uiDataSize = (i + 1) % 128 + 1;
		pvData = cList.GetNext(pvData);
		psNode = cList.GetNode(pvData);

		if (i != 8191)
		{
			if (iAlignment != psNode->sAligned.iAlignment)
			{
				AssertInt(iAlignment, psNode->sAligned.iAlignment);
			}

			iResult = (int)(((ptr)pvData) % iAlignment);
			if (iResult != 0)
			{
				AssertInt(0, iResult);
			}

			if (uiDataSize != psNode->sAligned.uiSize)
			{
				AssertSize(uiDataSize, psNode->sAligned.uiSize);
			}

			iResult = memcmp(pv, pvData, uiDataSize);
			if (iResult != 0)
			{
				AssertMemory(pv, pvData, uiDataSize);
			}

			if (uiDataSize == 64)
			{
				Pass();
			}
		}
	}
	AssertNull(pvData);
	AssertNull(psNode);


	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);

	cDiskFile.Init(szFile);
	cFile.Init(&cDiskFile);

	AssertTrue(cFile.Open(EFM_Write_Create));
	AssertTrue(cFile.WriteString("StartMagic"));
	bResult = cList.Write(&cFile);
	AssertTrue(bResult);
	AssertTrue(cFile.WriteString("EndMagic"));
	AssertTrue(cDiskFile.Close());
	cFile.Kill();
	cDiskFile.Kill();
	cList.Kill();

	cDiskFile.Init(szFile);
	cFile.Init(&cDiskFile);

	AssertTrue(cFile.Open(EFM_ReadWrite));
	AssertTrue(cFile.ReadString(sz, 256));
	AssertString("StartMagic", sz);

	cList.Init();
	bResult = cList.Read(&cFile);
	AssertTrue(bResult);
	AssertTrue(cFile.ReadString(sz, 256));
	AssertString("EndMagic", sz);
	AssertTrue(cDiskFile.Close());
	cFile.Kill();
	cDiskFile.Kill();

	AssertSize(8192, cList.NumElements());

	uiDataSize = 1;

	pvData = cList.GetHead();
	for (i = 0; i < 8192; i++)
	{
		iAlignment = (i + 1) / 128 + 1;
		uiDataSize = (i + 1) % 128 + 1;
		pvData = cList.GetNext(pvData);
		psNode = cList.GetNode(pvData);

		if (i != 8191)
		{
			if (iAlignment != psNode->sAligned.iAlignment)
			{
				AssertInt(iAlignment, psNode->sAligned.iAlignment);
			}

			iResult = (int)(((ptr)pvData) % iAlignment);
			if (iResult != 0)
			{
				AssertInt(0, iResult);
			}

			if (uiDataSize != psNode->sAligned.uiSize)
			{
				AssertSize(uiDataSize, psNode->sAligned.uiSize);
			}

			iResult = memcmp(pv, pvData, uiDataSize);
			if (iResult != 0)
			{
				AssertMemory(pv, pvData, uiDataSize);
			}

			if (uiDataSize == 64)
			{
				Pass();
			}
		}
	}
	AssertNull(pvData);
	AssertNull(psNode);

	pvData = cList.GetHead();
	while (pvData)
	{
		pvNext = cList.GetNext(pvData);
		cList.Remove(pvData);
		pvData = cList.GetNext(pvNext);
	}

	AssertSize(4096, cList.NumElements());

	pvData = cList.GetTail();
	while (pvData)
	{
		pvPrev = cList.GetPrev(pvData);
		cList.Remove(pvData);
		pvData = pvPrev;
	}

	AssertSize(0, cList.NumElements());

	cList.Kill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLinkedListBlockAligned(void)
{
	BeginTests();
	MemoryInit();

	TestLinkedListBlockAlignedAdd();
	TestLinkedListBlockAlignedRemove();
	TestLinkedListBlockAlignedGrow();
	TestLinkedListBlockAlignedExhaustive();

	MemoryKill();
	TestStatistics();
}

