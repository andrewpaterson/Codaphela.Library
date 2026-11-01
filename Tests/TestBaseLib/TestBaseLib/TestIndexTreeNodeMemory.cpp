#include "BaseLib/Logger.h"
#include "BaseLib/IndexTreeMemory.h"
#include "TestLib/Assert.h"
#include "TestIndexTreeObject.h"
#include "TestIndexTreeMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeNodeMemoryInit(void)
{
	CIndexTreeNodeMemory*	pcNode;
	CIndexTreeNodeMemory	pcChildNode;
	CIndexTreeNodeMemory*	pcResult;
	CTestIndexTreeMemory	cTree;
	SLogConfig				sLogConfig;
	
	cTree.FakeInit(IKR_No);


	pcNode = (CIndexTreeNodeMemory*)malloc(sizeof(CIndexTreeNodeMemory) + sizeof(CIndexTreeNodeMemory*));  //Node.  No object.  One child.
	pcNode->Init(&cTree, NULL, 0);
	AssertTrue(pcNode->IsEmpty());
	AssertFalse(pcNode->HasNodes());
	AssertFalse(pcNode->HasData());
	AssertTrue(pcNode->ValidateNodesEmpty());

	pcNode->Contain(33);
	AssertFalse(pcNode->IsEmpty());
	AssertTrue(pcNode->HasNodes());
	AssertFalse(pcNode->HasData());

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(pcNode->ValidateNodesEmpty());
	gcLogger.SetConfig(&sLogConfig);

	AssertSize(33, pcNode->GetFirstIndex());
	AssertSize(33, pcNode->GetLastIndex());

	pcNode->Set(33, &pcChildNode);
	AssertFalse(pcNode->IsEmpty());
	AssertTrue(pcNode->HasNodes());
	AssertFalse(pcNode->HasData());
	AssertSize(1, pcNode->NumValidIndexes());
	AssertTrue(pcNode->ValidateNodesEmpty());

	pcResult = pcNode->Get(33);
	AssertPointer(&pcChildNode, pcResult);

	AssertFalse(pcNode->ContainsIndex(32));
	AssertTrue(pcNode->ContainsIndex(33));
	AssertFalse(pcNode->ContainsIndex(34));

	free(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* TestIndexTreeNodeMemoryMalloc(void)
{
	size					iSize;
	CIndexTreeNodeMemory*	pcNode;

	iSize = sizeof(CIndexTreeNodeMemory) + (MAX_UINT8 - 1) * sizeof(CIndexTreeNodeMemory*);
	pcNode = (CIndexTreeNodeMemory*)malloc(iSize);
	memset(pcNode, 0xfc, iSize);  //memset uses the low byte of 0xfffefdfc.
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeNodeMemoryFree(CIndexTreeNodeMemory* pcNode)
{
	free(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeNodeMemoryContain(void)
{
	CIndexTreeNodeMemory*	pcNode1;
	CIndexTreeNodeMemory*	pcNode2;
	CIndexTreeNodeMemory*	pcNode3;
	CIndexTreeNodeMemory*	pcNode4;
	CIndexTreeNodeMemory*	pcNode5;
	CIndexTreeNodeMemory*	pcNode6;
	CTestIndexTreeMemory	cTree;

	cTree.FakeInit(IKR_No);

	pcNode1 = TestIndexTreeNodeMemoryMalloc();
	pcNode2 = TestIndexTreeNodeMemoryMalloc();
	pcNode3 = TestIndexTreeNodeMemoryMalloc();
	pcNode4 = TestIndexTreeNodeMemoryMalloc();
	pcNode5 = TestIndexTreeNodeMemoryMalloc();
	pcNode6 = TestIndexTreeNodeMemoryMalloc();

	pcNode1->Init(&cTree, NULL, 0);
	pcNode1->Contain(8);
	AssertSize(8, pcNode1->GetFirstIndex());
	AssertSize(1, pcNode1->NumIndexes());
	AssertSize(8, pcNode1->GetLastIndex());
	AssertTrue(pcNode1->ContainsIndex(8));
	AssertNull(pcNode1->Get(8));
	pcNode1->Set(8, pcNode2);
	AssertNotNull(pcNode1->Get(8));

	pcNode1->Contain(9);
	AssertSize(8, pcNode1->GetFirstIndex());
	AssertSize(2, pcNode1->NumIndexes());
	AssertSize(9, pcNode1->GetLastIndex());
	AssertNull(pcNode1->Get(9));
	pcNode1->Set(9, pcNode3);
	AssertNotNull(pcNode1->Get(9));
	
	pcNode1->Contain(7);
	AssertSize(7, pcNode1->GetFirstIndex());
	AssertSize(3, pcNode1->NumIndexes());
	AssertSize(9, pcNode1->GetLastIndex());
	AssertNull(pcNode1->Get(7));
	pcNode1->Set(7, pcNode4);
	AssertNotNull(pcNode1->Get(7));

	pcNode1->Contain(2);
	AssertSize(2, pcNode1->GetFirstIndex());
	AssertSize(8, pcNode1->NumIndexes());
	AssertSize(9, pcNode1->GetLastIndex());
	AssertNull(pcNode1->Get(2));
	AssertNull(pcNode1->Get(3));
	AssertNull(pcNode1->Get(4));
	AssertNull(pcNode1->Get(5));
	AssertNull(pcNode1->Get(6));
	AssertNotNull(pcNode1->Get(7));
	pcNode1->Set(2, pcNode5);
	AssertNotNull(pcNode1->Get(2));
	AssertNull(pcNode1->Get(3));
	AssertNull(pcNode1->Get(4));
	AssertNull(pcNode1->Get(5));
	AssertNull(pcNode1->Get(6));
	AssertNotNull(pcNode1->Get(7));

	pcNode1->Contain(14);
	AssertSize(2, pcNode1->GetFirstIndex());
	AssertSize(13, pcNode1->NumIndexes());
	AssertSize(14, pcNode1->GetLastIndex());
	AssertNotNull(pcNode1->Get(9));
	AssertNull(pcNode1->Get(10));
	AssertNull(pcNode1->Get(11));
	AssertNull(pcNode1->Get(12));
	AssertNull(pcNode1->Get(13));
	AssertNull(pcNode1->Get(14));
	pcNode1->Set(14, pcNode6);
	AssertNotNull(pcNode1->Get(9));
	AssertNull(pcNode1->Get(10));
	AssertNull(pcNode1->Get(11));
	AssertNull(pcNode1->Get(12));
	AssertNull(pcNode1->Get(13));
	AssertNotNull(pcNode1->Get(14));

	AssertNull(pcNode1->Get(1));
	pcNode1->Set(1, pcNode5);
	AssertNull(pcNode1->Get(1));
	AssertNull(pcNode1->Get(15));
	pcNode1->Set(15, pcNode5);
	AssertNull(pcNode1->Get(15));

	AssertFalse(pcNode1->ContainsIndex(1));
	AssertTrue(pcNode1->ContainsIndex(2));
	AssertTrue(pcNode1->ContainsIndex(14));
	AssertFalse(pcNode1->ContainsIndex(15));

	AssertPointer(pcNode5, pcNode1->GetNode(0));
	AssertNull(pcNode1->GetNode(1));
	AssertNull(pcNode1->GetNode(2));
	AssertNull(pcNode1->GetNode(3));
	AssertNull(pcNode1->GetNode(4));
	AssertPointer(pcNode4, pcNode1->GetNode(5));
	AssertPointer(pcNode2, pcNode1->GetNode(6));
	AssertPointer(pcNode3, pcNode1->GetNode(7));
	AssertNull(pcNode1->GetNode(8));
	AssertNull(pcNode1->GetNode(9));
	AssertNull(pcNode1->GetNode(10));
	AssertNull(pcNode1->GetNode(11));
	AssertPointer(pcNode6, pcNode1->GetNode(12));

	AssertPointer(pcNode5, pcNode1->Get(2));
	AssertNull(pcNode1->Get(3));
	AssertNull(pcNode1->Get(4));
	AssertNull(pcNode1->Get(5));
	AssertNull(pcNode1->Get(6));
	AssertPointer(pcNode4, pcNode1->Get(7));
	AssertPointer(pcNode2, pcNode1->Get(8));
	AssertPointer(pcNode3, pcNode1->Get(9));
	AssertNull(pcNode1->Get(10));
	AssertNull(pcNode1->Get(11));
	AssertNull(pcNode1->Get(12));
	AssertNull(pcNode1->Get(13));
	AssertPointer(pcNode6, pcNode1->Get(14));

	TestIndexTreeNodeMemoryFree(pcNode6);
	TestIndexTreeNodeMemoryFree(pcNode5);
	TestIndexTreeNodeMemoryFree(pcNode4);
	TestIndexTreeNodeMemoryFree(pcNode3);
	TestIndexTreeNodeMemoryFree(pcNode2);
	TestIndexTreeNodeMemoryFree(pcNode1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeNodeMemoryUncontain(void)
{
	CIndexTreeNodeMemory*	pcNode1;
	CTestIndexTreeMemory	cTree;

	cTree.FakeInit(IKR_No);

	pcNode1 = TestIndexTreeNodeMemoryMalloc();

	pcNode1->Init(&cTree, NULL, 0);
	AssertFalse(pcNode1->HasNodes());
	AssertNull(pcNode1->GetNodesMemory());

	pcNode1->Contain(8);
	AssertTrue(pcNode1->HasNodes());
	AssertFalse(pcNode1->IsEmpty());
	AssertNotNull(pcNode1->GetNodesMemory());
	AssertSize(8, pcNode1->GetFirstIndex());
	AssertSize(1, pcNode1->NumIndexes());
	AssertSize(8, pcNode1->GetLastIndex());
	AssertTrue(pcNode1->HasNodes());
	AssertFalse(pcNode1->IsEmpty());

	pcNode1->Uncontain(8);
	AssertTrue(pcNode1->IsEmpty());
	AssertSize(0, pcNode1->GetFirstIndex());
	AssertSize(0, pcNode1->NumIndexes());
	AssertSize(0, pcNode1->GetLastIndex());

	pcNode1->Contain(254);
	AssertSize(254, pcNode1->GetFirstIndex());
	AssertSize(1, pcNode1->NumIndexes());
	AssertSize(254, pcNode1->GetLastIndex());
	AssertFalse(pcNode1->IsEmpty());

	pcNode1->Uncontain(254);
	AssertTrue(pcNode1->IsEmpty());
	AssertSize(0, pcNode1->GetFirstIndex());
	AssertSize(0, pcNode1->NumIndexes());
	AssertSize(0, pcNode1->GetLastIndex());

	pcNode1->Contain(0);
	AssertSize(0, pcNode1->GetFirstIndex());
	AssertSize(1, pcNode1->NumIndexes());
	AssertSize(0, pcNode1->GetLastIndex());
	AssertFalse(pcNode1->IsEmpty());
	pcNode1->Contain(1);
	AssertSize(0, pcNode1->GetFirstIndex());
	AssertSize(2, pcNode1->NumIndexes());
	AssertSize(1, pcNode1->GetLastIndex());
	AssertFalse(pcNode1->IsEmpty());
	pcNode1->Uncontain(0);
	AssertSize(1, pcNode1->GetFirstIndex());
	AssertSize(1, pcNode1->NumIndexes());
	AssertSize(1, pcNode1->GetLastIndex());
	AssertFalse(pcNode1->IsEmpty());
	pcNode1->Uncontain(1);
	AssertSize(0, pcNode1->GetFirstIndex());
	AssertSize(0, pcNode1->NumIndexes());
	AssertSize(0, pcNode1->GetLastIndex());

	pcNode1->Contain(1);
	AssertSize(1, pcNode1->GetFirstIndex());
	AssertSize(1, pcNode1->NumIndexes());
	AssertSize(1, pcNode1->GetLastIndex());
	AssertFalse(pcNode1->IsEmpty());
	pcNode1->Contain(0);
	AssertSize(0, pcNode1->GetFirstIndex());
	AssertSize(2, pcNode1->NumIndexes());
	AssertSize(1, pcNode1->GetLastIndex());
	AssertFalse(pcNode1->IsEmpty());
	pcNode1->Uncontain(1);
	AssertSize(0, pcNode1->GetFirstIndex());
	AssertSize(1, pcNode1->NumIndexes());
	AssertSize(0, pcNode1->GetLastIndex());
	AssertFalse(pcNode1->IsEmpty());
	pcNode1->Uncontain(0);
	AssertSize(0, pcNode1->GetFirstIndex());
	AssertSize(0, pcNode1->NumIndexes());
	AssertSize(0, pcNode1->GetLastIndex());

	TestIndexTreeNodeMemoryFree(pcNode1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeNodeMemoryFindNotEmpty(void)
{
	CIndexTreeNodeMemory*	pcNode1;
	CIndexTreeNodeMemory*	pcNode2;
	CIndexTreeNodeMemory*	pcNode3;
	CIndexTreeNodeMemory*	pcNode4;
	CIndexTreeNodeMemory*	pcNode5;
	CTestIndexTreeMemory	cTree;

	cTree.FakeInit(IKR_No);

	pcNode1 = TestIndexTreeNodeMemoryMalloc();
	pcNode2 = TestIndexTreeNodeMemoryMalloc();
	pcNode3 = TestIndexTreeNodeMemoryMalloc();
	pcNode4 = TestIndexTreeNodeMemoryMalloc();
	pcNode5 = TestIndexTreeNodeMemoryMalloc();

	pcNode1->Init(&cTree, NULL, 0);
	pcNode1->Contain(8);
	pcNode1->Contain(22);
	AssertSize(8, pcNode1->GetFirstIndex());
	AssertSize(15, pcNode1->NumIndexes());
	AssertSize(22, pcNode1->GetLastIndex());
	AssertFalse(pcNode1->IsEmpty());

	pcNode1->Set(8, pcNode2);
	pcNode1->Set(10, pcNode3);
	pcNode1->Set(20, pcNode4);
	pcNode1->Set(22, pcNode5);
	AssertFalse(pcNode1->IsEmpty());

	AssertSize(10, pcNode1->FindNextFirstIndex());
	AssertSize(20, pcNode1->FindPrevLastIndex());

	AssertPointer(pcNode2, pcNode1->Get(8));
	AssertPointer(pcNode3, pcNode1->Get(10));
	AssertPointer(pcNode4, pcNode1->Get(20));
	AssertPointer(pcNode5, pcNode1->Get(22));

	pcNode1->Set(8, NULL);
	pcNode1->Uncontain(8);
	AssertSize(10, pcNode1->GetFirstIndex());
	AssertSize(13, pcNode1->NumIndexes());
	AssertSize(22, pcNode1->GetLastIndex());
	AssertNull(pcNode1->Get(8));
	AssertPointer(pcNode3, pcNode1->Get(10));
	AssertPointer(pcNode4, pcNode1->Get(20));
	AssertPointer(pcNode5, pcNode1->Get(22));


	pcNode1->Set(22, NULL);
	pcNode1->Uncontain(22);
	AssertSize(10, pcNode1->GetFirstIndex());
	AssertSize(11, pcNode1->NumIndexes());
	AssertSize(20, pcNode1->GetLastIndex());
	AssertNull(pcNode1->Get(8));
	AssertPointer(pcNode3, pcNode1->Get(10));
	AssertPointer(pcNode4, pcNode1->Get(20));
	AssertNull(pcNode1->Get(22));

	pcNode1->Set(10, NULL);
	pcNode1->Uncontain(10);
	AssertSize(20, pcNode1->GetFirstIndex());
	AssertSize(1, pcNode1->NumIndexes());
	AssertSize(20, pcNode1->GetLastIndex());
	AssertNull(pcNode1->Get(8));
	AssertNull(pcNode1->Get(10));
	AssertPointer(pcNode4, pcNode1->Get(20));
	AssertNull(pcNode1->Get(22));
	AssertFalse(pcNode1->IsEmpty());

	pcNode1->Set(20, NULL);
	pcNode1->Uncontain(20);
	AssertSize(0, pcNode1->GetFirstIndex());
	AssertSize(0, pcNode1->NumIndexes());
	AssertSize(0, pcNode1->GetLastIndex());
	AssertNull(pcNode1->Get(8));
	AssertNull(pcNode1->Get(10));
	AssertNull(pcNode1->Get(20));
	AssertNull(pcNode1->Get(22));
	AssertTrue(pcNode1->IsEmpty());

	TestIndexTreeNodeMemoryFree(pcNode5);
	TestIndexTreeNodeMemoryFree(pcNode4);
	TestIndexTreeNodeMemoryFree(pcNode3);
	TestIndexTreeNodeMemoryFree(pcNode2);
	TestIndexTreeNodeMemoryFree(pcNode1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeNodeMemory(void)
{
	BeginTests();

	TestIndexTreeNodeMemoryInit();
	TestIndexTreeNodeMemoryContain();
	TestIndexTreeNodeMemoryUncontain();
	TestIndexTreeNodeMemoryFindNotEmpty();

	TestStatistics();
}

