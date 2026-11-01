#include "BaseLib/ListTemplate.h"
#include "BaseLib/SystemAllocator.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/MapStringString.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"


struct STestListTemplateItem
{
	size	mi1;
	size	mi2;

	void Init(size i1, size i2)
	{
		this->mi1 = i1;
		this->mi2 = i2;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestListTemplateMethods(void)
{
	CListTemplate<STestListTemplateItem>	cList;
	STestListTemplateItem*					psItem1;
	STestListTemplateItem					sItem2;
	STestListTemplateItem*					psItem2;
	STestListTemplateItem*					psItem3;
	STestListTemplateItem*					psItem4;
	STestListTemplateItem*					psItem5;
	STestListTemplateItem					sItem6;
	size									iIndex;
	STestListTemplateItem					sItem;

	cList.Init(16);
	cList.Kill();

	cList.Init(16, 7);
	cList.Kill();

	cList.Init();
	psItem1 = cList.Add();
	psItem1->Init(111, 8);

	sItem2.Init(222, 16);
	cList.Add(&sItem2);
	AssertSize(2, cList.NumElements());

	psItem3 = cList.AddGetIndex(&iIndex);
	psItem3->Init(333, 32);
	AssertSize(2, iIndex);
	AssertSize(3, cList.NumElements());
	AssertSize(3, cList.NumElementsFromFreeList());
	AssertSize(1, cList.NumBlocks());

	psItem2 = cList.Get(1);
	AssertSize(sItem2.mi1, psItem2->mi1);
	AssertSize(sItem2.mi2, psItem2->mi2);
	AssertPointer(psItem1, cList.Get(0));

	AssertNull(cList.SafeGet(-1));
	AssertNull(cList.SafeGet(3));

	AssertSize(0, cList.GetIndex(psItem1));
	AssertSize(1, cList.GetIndex(psItem2));
	AssertSize(2, cList.GetIndex(psItem3));

	AssertPointer(psItem3, cList.Tail());

	psItem4 = cList.InsertAt(1);
	psItem4->Init(444, 64);
	AssertSize(4, cList.NumElements());
	AssertSize(4, cList.NumElementsFromFreeList());
	AssertSize(0, cList.GetIndex(psItem1));
	AssertSize(1, cList.GetIndex(psItem4));
	AssertSize(2, cList.GetIndex(psItem2));
	AssertSize(3, cList.GetIndex(psItem3));

	cList.Pop(&sItem);
	AssertSize(333, sItem.mi1);
	AssertSize(32, sItem.mi2);
	AssertSize(3, cList.NumElements());
	AssertSize(3, cList.NumElementsFromFreeList());

	psItem5 = cList.Push();
	psItem5->Init(1, 5);

	sItem6.Init(1, 6);
	cList.Push(&sItem6);
	AssertSize(5, cList.NumElements());
	AssertSize(5, cList.NumElementsFromFreeList());

	AssertTrue(cList.Contains(psItem1));
	AssertFalse(cList.Contains(NULL));

	sItem.Init(12, 17);
	AssertTrue(cList.Set(0, &sItem));
	AssertSize(5, cList.NumElements());
	AssertSize(5, cList.NumElementsFromFreeList());
	AssertSize(12, cList.Get(0)->mi1);
	AssertSize(17, cList.Get(0)->mi2);
	AssertPointer(psItem1, cList.Get(0));

	AssertFalse(cList.Set(-1, &sItem));
	AssertFalse(cList.Set(5, &sItem));

	cList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestListTemplate(void)
{
	BeginTests();
	MemoryInit();

	TestListTemplateMethods();

	MemoryKill();
	TestStatistics();
}

