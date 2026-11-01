#include "BaseLib/MemoryFile.h"
#include "BaseLib/TreeTemplate.h"
#include "TestLib/Assert.h"


struct STestTreeTemplateItem
{
	int	mi1;
	int	mi2;

	void Init(int i1, int i2);
	bool IsOkay(int i1, int i2);
};


void STestTreeTemplateItem::Init(int i1, int i2)
{
	mi1 = i1;
	mi2 = i2;
}


bool STestTreeTemplateItem::IsOkay(int i1, int i2)
{
	return (mi1 == i1) && (mi2 == i2);
}



typedef CTreeTemplate<STestTreeTemplateItem> CTestTree;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTreeTemplateInsert(void)
{
	CTestTree				cTree;
	STestTreeTemplateItem*	pcNode0;
	STestTreeTemplateItem*	pcNode00;
	STestTreeTemplateItem*	pcNode01;
	STestTreeTemplateItem*	pcNode02;
	STestTreeTemplateItem*	pcNode000;
	STestTreeTemplateItem*	pcTest;

	cTree.Init();
	pcNode0 = cTree.InsertRoot();
	pcNode0->Init(672, 7645);

	pcNode00 = cTree.InsertOnRightOfChildren(pcNode0);
	pcNode00->Init(720, 6);

	pcNode000 = cTree.InsertOnRightOfChildren(pcNode00);
	pcNode000->Init(325, 28274);

	pcNode01 = cTree.InsertOnRightOfChildren(pcNode0);
	pcNode01->Init(8, 12);

	pcNode02 = cTree.InsertOnRightOfChildren(pcNode0);
	pcNode02->Init(99, 7);

	AssertSize(5, cTree.NumElements());
	pcTest = cTree.GetRoot();
	AssertPointer(pcNode0, pcTest);
	AssertTrue(pcTest->IsOkay(672, 7645));
	pcTest = cTree.GetUp(pcNode0);
	AssertPointer(pcNode00, pcTest);
	AssertTrue(pcTest->IsOkay(720, 6));
	pcTest = cTree.GetUp(pcNode00);
	AssertPointer(pcNode000, pcTest);
	pcTest = cTree.GetRight(pcNode00);
	AssertPointer(pcNode01, pcTest);
	pcTest = cTree.GetRight(pcNode01);
	AssertPointer(pcNode02, pcTest);
	pcTest = cTree.GetRight(pcNode02);
	AssertNull(pcTest);
	AssertNull(cTree.GetUp(pcNode01));
	AssertNull(cTree.GetUp(pcNode02));
	AssertPointer(pcNode0, cTree.GetDown(pcNode00));
	AssertPointer(pcNode0, cTree.GetDown(pcNode01));
	AssertPointer(pcNode0, cTree.GetDown(pcNode02));

	cTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ClearPath(size* paiPath, size uiSize)
{
	size	i;

	for (i = 0; i < uiSize; i++)
	{
		paiPath[i] = ARRAY_ELEMENT_NOT_FOUND;
	}
}
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTreeTemplateGetPathTo(void)
{
	CTestTree				cTree;
	STestTreeTemplateItem*	pcNode0;
	STestTreeTemplateItem*	pcNode00;
	STestTreeTemplateItem*	pcNode01;
	STestTreeTemplateItem*	pcNode02;
	STestTreeTemplateItem*	pcNode000;
	STestTreeTemplateItem*	pcNode010;
	size					uiNumElements;
	size					uiNumElementsByCount;
	size					aiPath[10];

	cTree.Init();
	pcNode0 = cTree.InsertRoot();
	pcNode0->Init(672, 7645);

	pcNode00 = cTree.InsertOnRightOfChildren(pcNode0);
	pcNode00->Init(720, 6);

	pcNode000 = cTree.InsertOnRightOfChildren(pcNode00);
	pcNode000->Init(325, 28274);

	pcNode01 = cTree.InsertOnRightOfChildren(pcNode0);
	pcNode01->Init(8, 12);

	pcNode010 = cTree.InsertOnRightOfChildren(pcNode01);
	pcNode010->Init(64, 12753);

	pcNode02 = cTree.InsertOnRightOfChildren(pcNode0);
	pcNode02->Init(99, 7);

	uiNumElements = cTree.NumElements();
	uiNumElementsByCount = cTree.CountElements();
	AssertSize(uiNumElements, uiNumElementsByCount);

	ClearPath(aiPath, 10);
	cTree.GetPathTo(aiPath, pcNode0);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, aiPath[0]);

	ClearPath(aiPath, 10);
	cTree.GetPathTo(aiPath, pcNode01);
	AssertSize(1, aiPath[0]);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, aiPath[1]);

	ClearPath(aiPath, 10);
	cTree.GetPathTo(aiPath, pcNode010);
	AssertSize(1, aiPath[0]);
	AssertSize(0, aiPath[1]);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, aiPath[2]);

	ClearPath(aiPath, 10);
	cTree.GetPathTo(aiPath, pcNode00);
	AssertSize(0, aiPath[0]);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, aiPath[1]);

	ClearPath(aiPath, 10);
	cTree.GetPathTo(aiPath, pcNode000);
	AssertSize(0, aiPath[0]);
	AssertSize(0, aiPath[1]);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, aiPath[2]);

	ClearPath(aiPath, 10);
	cTree.GetPathTo(aiPath, pcNode02);
	AssertSize(2, aiPath[0]);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, aiPath[1]);

	cTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTreeTemplateFileHelper(void)
{
	CTestTree				cTree;
	STestTreeTemplateItem*	psRoot;
	STestTreeTemplateItem*	psChild1;
	STestTreeTemplateItem*	psChild2;
	CMemoryFile*			pcMemoryFile;
	CFileBasic				cFileBasic;
	bool					bResult;

	cTree.Init();

	psRoot = cTree.InsertRoot();
	psRoot->mi1 = 5; psRoot->mi2 = 7;

	psChild1 = cTree.InsertOnRightOfChildren(psRoot);
	psChild1->mi1 = 1; psChild1->mi2 = 2;

	psChild2 = cTree.InsertOnRightOfChildren(psRoot);
	psChild2->mi1 = 3; psChild2->mi2 = 9;

	pcMemoryFile = MemoryFile();
	pcMemoryFile->mbBasicFileMustFree = false;

	cFileBasic.Init(pcMemoryFile);
	cFileBasic.Open(EFM_Write_Create);
	bResult = cTree.WriteTreeTemplate(&cFileBasic);
	AssertTrue(bResult);
	cFileBasic.Close();
	cFileBasic.Kill();

	cTree.Kill();

	cFileBasic.Init(pcMemoryFile);
	cFileBasic.Open(EFM_Read);
	bResult = cTree.ReadTreeTemplate(&cFileBasic);
	AssertTrue(bResult);
	cFileBasic.Close();
	cFileBasic.Kill();

	pcMemoryFile->Kill();
	free(pcMemoryFile);

	psRoot = cTree.GetRoot();
	AssertInt(5, psRoot->mi1);
	AssertInt(7, psRoot->mi2);

	psChild1 = cTree.GetUp(psRoot);
	AssertInt(1, psChild1->mi1);
	AssertInt(2, psChild1->mi2);

	psChild2 = cTree.GetRight(psChild1);
	AssertInt(3, psChild2->mi1);
	AssertInt(9, psChild2->mi2);

	cTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTreeTemplate(void)
{
	BeginTests();

	MemoryInit();

	TestTreeTemplateInsert();
	TestTreeTemplateGetPathTo();
	TestTreeTemplateFileHelper();

	MemoryKill();

	TestStatistics();
}

