#include "BaseLib/FreeList.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertIteration(CFreeList* pcFreeList, void* pvFirst, ...)
{
	va_list				vaMarker;
	void*				pv;
	size				iCount;
	SFreeListIterator	sIter;
	void*				pvTest;

	iCount = 0;
	pv = pvFirst;

	pvTest = pcFreeList->StartIteration(&sIter);

	va_start(vaMarker, pvFirst);
	while ((pv != NULL) || (pvTest != NULL))
	{
		if (pv != pvTest)
		{
			AssertPointer(pv, pvTest);
			return;
		}

		pvTest = pcFreeList->Iterate(&sIter);
		iCount++;
		pv = va_arg(vaMarker, void*);
	}
	va_end(vaMarker);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeListSizeof(void)
{
#ifdef BUILD_64
	AssertSize(72, sizeof(CFreeList));
	AssertSize(40, sizeof(SFNode));
#elif defined(SIZE_16)
	AssertSize(36, sizeof(CFreeList));
	AssertSize(24, sizeof(SFNode));
#else
	AssertSize(40, sizeof(CFreeList));
	AssertSize(24, sizeof(SFNode));
#endif // BUILD_64
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeListAllocation(void)
{
	CFreeList	cFreeList;
	void*		pv1;
	void*		pv2;
	void*		pv3;
	void*		pv4;
	void*		pv5;
	void*		pv6;
	void*		pv7;
	void*		pv8;
	void*		pv9;
	SFNode*		psNode0;
	SFNode*		psNode1;
	SFNode*		psNode2;

	cFreeList.Init(7, 1);
	AssertSize(0, cFreeList.GetNumAllocatedChunks());
	AssertSize(0, cFreeList.NumElements());

	pv1 = cFreeList.Add();
	AssertSize(1, cFreeList.GetNumAllocatedChunks());
	AssertSize(1, cFreeList.NumElements());

	pv2 = cFreeList.Add();
	pv3 = cFreeList.Add();
	pv4 = cFreeList.Add();
	pv5 = cFreeList.Add();
	AssertSize(1, cFreeList.GetNumAllocatedChunks());
	AssertSize(5, cFreeList.NumElements());
	AssertIteration(&cFreeList, pv1, pv2, pv3, pv4, pv5, NULL);

	cFreeList.Remove(pv2);
	AssertSize(1, cFreeList.GetNumAllocatedChunks());
	AssertSize(4, cFreeList.NumElements());
	psNode0 = cFreeList.GetNode(0);
	pv2 = cFreeList.GetElement(psNode0, 1);
	AssertNull(pv2);
	AssertIteration(&cFreeList, pv1, pv3, pv4, pv5, NULL);

	cFreeList.Remove(pv1);
	AssertSize(1, cFreeList.GetNumAllocatedChunks());
	AssertSize(3, cFreeList.NumElements());
	psNode1 = cFreeList.GetNode(0);
	AssertPointer(psNode1, psNode0);
	AssertIteration(&cFreeList, pv3, pv4, pv5, NULL);

	pv6 = cFreeList.Add();
	AssertSize(1, cFreeList.GetNumAllocatedChunks());
	AssertSize(4, cFreeList.NumElements());
	AssertIteration(&cFreeList, pv6, pv3, pv4, pv5, NULL);

	pv7 = cFreeList.Add();
	pv8 = cFreeList.Add();
	pv9 = cFreeList.Add();
	pv1 = cFreeList.Add();
	AssertSize(1, cFreeList.GetNumAllocatedChunks());
	AssertSize(8, cFreeList.NumElements());
	AssertIteration(&cFreeList, pv6, pv7, pv3, pv4, pv5, pv8, pv9, pv1, NULL);

	pv2 = cFreeList.Add();
	AssertSize(2, cFreeList.GetNumAllocatedChunks());
	AssertSize(9, cFreeList.NumElements());
	psNode1 = cFreeList.GetNode(1);
	AssertFalse(psNode0 == psNode1);  //Make sure the "freed" node is reused.
	AssertIteration(&cFreeList, pv6, pv7, pv3, pv4, pv5, pv8, pv9, pv1, pv2, NULL);

	cFreeList.Remove(pv2);
	AssertSize(1, cFreeList.GetNumAllocatedChunks());
	AssertSize(8, cFreeList.NumElements());
	AssertIteration(&cFreeList, pv6, pv7, pv3, pv4, pv5, pv8, pv9, pv1, NULL);

	pv2 = cFreeList.Add();
	AssertSize(2, cFreeList.GetNumAllocatedChunks());
	AssertSize(9, cFreeList.NumElements());
	psNode2 = cFreeList.GetNode(1);
	AssertPointer(psNode1, psNode2);  //Make sure the "freed" node is reused.
	AssertIteration(&cFreeList, pv6, pv7, pv3, pv4, pv5, pv8, pv9, pv1, pv2, NULL);

	cFreeList.Kill();
}


typedef CArrayTemplate<CFreeList>	CArrayFreeList;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeListAlignment(void)
{
	CArrayFreeList	cArray;
	char			iAlignment;
	CFreeList*		pcFreeList;
	void*			pv;
	size			k;
	CFreeList		cFreeList;
	SFNode*			psNode;
	size			i;

	cArray.Init();

	for (iAlignment = 2; iAlignment <= 48; iAlignment++)
	{
		pcFreeList = cArray.Add();
		pcFreeList->Init(iAlignment, iAlignment);
		AssertSize((size)iAlignment, pcFreeList->GetElementStride());
		AssertSize((size)iAlignment, pcFreeList->GetElementSize());

		for (k = 0; k < 9; k++)
		{
			pv = pcFreeList->Add();
			AssertInt((int32)0, (int32)((ptr)pv % iAlignment));
			psNode = pcFreeList->FindNode(pv, false);
			AssertTrue(psNode->iOffset < iAlignment);
		}
	}

	for (i = 0; i < cArray.NumElements(); i++)
	{
		pcFreeList = cArray.Get(i);
		pcFreeList->Kill();
	}

	cArray.Kill();

	cFreeList.Init(10, 16);
	AssertSize(16, cFreeList.GetElementStride());
	AssertSize(10, cFreeList.GetElementSize());
	cFreeList.Kill();

	cFreeList.Init(20, 16);
	AssertSize(32, cFreeList.GetElementStride());
	AssertSize(20, cFreeList.GetElementSize());
	cFreeList.Kill();

	cFreeList.Init(1, 4);
	AssertSize(4, cFreeList.GetElementStride());
	cFreeList.Kill();

	cFreeList.Init(2, 4);
	AssertSize(4, cFreeList.GetElementStride());
	cFreeList.Kill();

	cFreeList.Init(3, 4);
	AssertSize(4, cFreeList.GetElementStride());
	cFreeList.Kill();

	cFreeList.Init(4, 4);
	AssertSize(4, cFreeList.GetElementStride());
	cFreeList.Kill();

	cFreeList.Init(5, 4);
	AssertSize(8, cFreeList.GetElementStride());
	cFreeList.Kill();

	cFreeList.Init(6, 4);
	AssertSize(8, cFreeList.GetElementStride());
	cFreeList.Kill();

	cFreeList.Init(7, 4);
	AssertSize(8, cFreeList.GetElementStride());
	cFreeList.Kill();

	cFreeList.Init(8, 4);
	AssertSize(8, cFreeList.GetElementStride());
	cFreeList.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeListOffsetAlignment(void)
{
	CArrayFreeList	cArray;
	uint16			iElementSize;
	char			iOffset;
	CFreeList*		pcFreeList;
	void*			pv;
	size			i;
	char			iAlignment;

	cArray.Init();

	for (iAlignment = 2; iAlignment <= 6; iAlignment++)
	{
		for (iElementSize = 2; iElementSize <= 8; iElementSize++)
		{
			for (iOffset = -1; iOffset <= 7; iOffset++)
			{
				pcFreeList = cArray.Add();
				pcFreeList->Init(iElementSize, iAlignment, iOffset);

				for (i = 0; i < 5; i++)
				{
					pv = pcFreeList->Add();
					if (iOffset >= 0)
					{
						AssertInt(iOffset % iAlignment, (int)((ptr)pv % iAlignment));
					}
					else
					{
						AssertInt(iAlignment - (-iOffset % iAlignment), (int)((ptr)pv % iAlignment));
					}
				}
			}
		}
	}

	for (i = 0; i < cArray.NumElements(); i++)
	{
		pcFreeList = cArray.Get(i);
		pcFreeList->Kill();
	}

	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeListIteration()
{
	CFreeList			cFreeList;
	int					*pi;
	size				ui;
	SFreeListIterator	sIter;

	cFreeList.Init(sizeof(int));
	
	for (ui = 0; ui < 10000; ui++)
	{
		pi = (int*)cFreeList.Add();
		*pi = (int)ui;
	}

	ui = 0;
	pi = (int*)cFreeList.StartIteration(&sIter);
	
	while (pi)
	{
		if (*pi != (int)ui)
		{
			AssertSize((size)*pi, ui);
			break;
		}
		ui++;

		pi = (int*)cFreeList.Iterate(&sIter);
	}
	AssertTrue(true);

	AssertSize(10000, cFreeList.NumElements());
	AssertSize(46, cFreeList.NumNodes())

	cFreeList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeListRemoveDuringIteration1()
{
	CFreeList			cFreeList;
	int*				pi;
	size				ui;
	SFreeListIterator	sIter;

	cFreeList.Init(sizeof(int));

	for (ui = 0; ui < 500; ui++)
	{
		pi = (int*)cFreeList.Add();
		*pi = (int)ui;
	}

	ui = 0;
	pi = (int*)cFreeList.StartIteration(&sIter);

	while (pi)
	{
		AssertSize(500 - ui, cFreeList.NumElements());
		cFreeList.RemoveDuringIteration(&sIter);
		ui++;

		pi = (int*)cFreeList.Iterate(&sIter);
	}
	AssertFalse(cFreeList.HasElements());

	cFreeList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeListRemoveDuringIteration2()
{
	CFreeList			cFreeList;
	int*				pi;
	size				ui;
	SFreeListIterator	sIter;

	cFreeList.Init(sizeof(int));

	for (ui = 0; ui < 500; ui++)
	{
		pi = (int*)cFreeList.Add();
		*pi = (int)ui;
	}

	pi = (int*)cFreeList.StartIteration(&sIter);
	ui = 1;
	pi = (int*)cFreeList.Iterate(&sIter);
	AssertInt(1, *pi);

	while (pi)
	{
		AssertSize(501 - ui, cFreeList.NumElements());
		cFreeList.RemoveDuringIteration(&sIter);
		ui++;

		if (ui == 500)
		{
			break;
		}

		pi = (int*)cFreeList.Iterate(&sIter);
	}
	AssertSize(1, cFreeList.NumElements());
	AssertInt(498, *pi);

	ui = 499;
	pi = (int*)cFreeList.StartIteration(&sIter);
	while (pi)
	{
		AssertSize((size)*pi, ui);
		ui++;
		pi = (int*)cFreeList.Iterate(&sIter);
	}

	cFreeList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeListRemoveDuringIteration3()
{
	CFreeList			cFreeList;
	int* pi;
	size				ui;
	SFreeListIterator	sIter;

	cFreeList.Init(sizeof(int));

	for (ui = 0; ui < 500; ui++)
	{
		pi = (int*)cFreeList.Add();
		*pi = (int)ui;
	}

	ui = 0;
	pi = (int*)cFreeList.StartIteration(&sIter);
	for (ui = 0; ui < 7; ui++)
	{
		pi = (int*)cFreeList.Iterate(&sIter);
	}
	AssertInt(7, *pi);

	while (pi)
	{
		AssertSize(507 - ui, cFreeList.NumElements());
		cFreeList.RemoveDuringIteration(&sIter);
		ui++;

		if (ui == 500)
		{
			break;
		}

		pi = (int*)cFreeList.Iterate(&sIter);
	}
	AssertSize(7, cFreeList.NumElements());
	AssertInt(492, *pi);

	ui = 493;
	pi = (int*)cFreeList.StartIteration(&sIter);
	while (pi)
	{
		AssertSize((size)*pi, ui);
		ui++;
		pi = (int*)cFreeList.Iterate(&sIter);
	}

	cFreeList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeListRemoveDuringIteration4()
{
	CFreeList			cFreeList;
	int*				pi;
	size				ui;
	SFreeListIterator	sIter;

	cFreeList.Init(sizeof(int));

	for (ui = 0; ui < 10; ui++)
	{
		pi = (int*)cFreeList.Add();
		*pi = (int)ui;
	}

	ui = 0;
	pi = (int*)cFreeList.StartIteration(&sIter);

	while (pi)
	{
		AssertSize(500 - ui, cFreeList.NumElements());
		cFreeList.RemoveDuringIteration(&sIter);
		ui++;

	}
	AssertFalse(cFreeList.HasElements());

	cFreeList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeList(void)
{
	BeginTests();

	TestFreeListSizeof();
	TestFreeListAllocation();
	TestFreeListAlignment();
	TestFreeListOffsetAlignment();
	TestFreeListIteration();
	TestFreeListRemoveDuringIteration1();
	TestFreeListRemoveDuringIteration2();
	TestFreeListRemoveDuringIteration3();

	TestStatistics();
}

