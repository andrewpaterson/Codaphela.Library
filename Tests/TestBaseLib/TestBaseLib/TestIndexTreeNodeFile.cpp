#include "BaseLib/Logger.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/SystemAllocator.h"
#include "BaseLib/TypeConverter.h"
#include "TestLib/Assert.h"
#include "BaseLib/IndexTreeNodeFile.h"
#include "BaseLib/IndexTreeFile.h"
#include "TestIndexTreeFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeNodeFileSizeofs(void)
{
#ifdef BUILD_64
	AssertSize(8, sizeof(CIndexTreeNodeFile*));  //The size of a memory pointer must be (at least) 4 bytes less than CIndexTreeChildNode.
#else
	AssertSize(4, sizeof(CIndexTreeNodeFile*));  //The size of a memory pointer must be (at least) 4 bytes less than CIndexTreeChildNode.
#endif // BUILD_64
	AssertSize(8, sizeof(CFileDataIndex));  //SizeOf is 16 but only the first 12 bytes are used.

#ifdef BUILD_64
	AssertSize(16, sizeof(CIndexTreeChildNode));  //The above is why the union works 
#else
	AssertSize(12, sizeof(CIndexTreeChildNode));  //The above is why the union works 
#endif // BUILD_64

#ifdef BUILD_64
	AssertSize(32, sizeof(CIndexTreeNodeFile));  
	AssertSize(24, sizeof(CIndexTreeNode));
#else
	AssertSize(24, sizeof(CIndexTreeNodeFile));  
	AssertSize(16, sizeof(CIndexTreeNode));
#endif // BUILD_64
	AssertTrue(sizeof(CIndexTreeNodeFile*) <= sizeof(CIndexTreeChildNode) - sizeof(int));
#ifdef __W65C816__
	AssertSize(2, sizeof(int));  //Just ot be sure.
#else
	AssertSize(4, sizeof(int));  //Just ot be sure.
#endif // __W65C816__
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeNodeFileUnion(void)
{
	CIndexTreeChildNode		cChild;

	memset(&cChild, 0, sizeof(CIndexTreeChildNode));
	cChild.u.mcFile.Init();
	AssertInt((uint32)0, cChild.iType);

	cChild.u.mpcMemory = (CIndexTreeNodeFile*)(size)0x80808080;
	AssertInt((uint32)0, cChild.iType);
	AssertFalse(cChild.IsValid());

	memset(&cChild, 1, sizeof(CIndexTreeChildNode));
	AssertTrue(cChild.IsValid());
	AssertTrue(cChild.IsMemory());
	AssertFalse(cChild.IsFile());

	memset(&cChild, 2, sizeof(CIndexTreeChildNode));
	AssertTrue(cChild.IsValid());
	AssertFalse(cChild.IsMemory());
	AssertTrue(cChild.IsFile());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeNodeFileRootMemory(void)
{
	CIndexTreeNodeFile*		pcNode;
	CTestIndexTreeFile		cIndex;
	CIndexTreeNodeFile		cChildNode;
	CIndexTreeChildNode*	pcResult;
	SLogConfig				sLogConfig;

	cIndex.FakeInit(IKR_No);

	pcNode = (CIndexTreeNodeFile*)malloc(sizeof(CIndexTreeNodeFile) + sizeof(CIndexTreeChildNode));  //Node.  No object.  One child.
	pcNode->Init(&cIndex, NULL, 0);

	AssertTrue(pcNode->IsEmpty());
	AssertFalse(pcNode->HasNodes());
	AssertFalse(pcNode->HasData());
	AssertTrue(pcNode->ValidateNodesEmpty());

	pcNode->Contain(33, INDEX_TREE_FILE_NODE_UNALLOCATED_BYTE);
	AssertFalse(pcNode->IsEmpty());
	AssertTrue(pcNode->HasNodes());
	AssertFalse(pcNode->HasData());

	gcLogger.GetConfig(&sLogConfig);
	gcLogger.SetBreakOnError(false);
	AssertFalse(pcNode->ValidateNodesEmpty());
	gcLogger.SetConfig(&sLogConfig);

	AssertSize(33, pcNode->GetFirstIndex());
	AssertSize(33, pcNode->GetLastIndex());

	pcNode->SetMemory(33, &cChildNode);
	AssertFalse(pcNode->IsEmpty());
	AssertTrue(pcNode->HasNodes());
	AssertFalse(pcNode->HasData());
	Pass();
	AssertSize(1, pcNode->NumValidIndexes());
	Pass();
	AssertTrue(pcNode->ValidateNodesEmpty());
	Pass();

	pcResult = pcNode->Get(33);
	AssertTrue(pcResult->IsMemory());
	AssertPointer(&cChildNode, pcResult->u.mpcMemory);

	AssertFalse(pcNode->ContainsIndex(32));
	AssertTrue(pcNode->ContainsIndex(33));
	AssertFalse(pcNode->ContainsIndex(34));

	free(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeNodeFile(void)
{
	TypeConverterInit();
	BeginTests();

	TestIndexTreeNodeFileSizeofs();
	TestIndexTreeNodeFileUnion();
	TestIndexTreeNodeFileRootMemory();

	TestStatistics();
	TypeConverterKill();
}

