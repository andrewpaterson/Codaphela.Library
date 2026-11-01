#ifndef __TEST_INDEX_TREE_FILE_H__
#define __TEST_INDEX_TREE_FILE_H__
#include "BaseLib/IndexTreeFile.h"


class CTestIndexTreeFile : public CIndexTreeFile
{
public:
	void FakeInit(EIndexKeyReverse eKeyReverse)
	{
		CIndexTree::Init(LifeLocal<CMallocator>(&gcSystemAllocator), eKeyReverse, sizeof(CIndexTreeNodeFile), sizeof(CIndexTreeNodeFile) + sizeof(CIndexTreeDataNode), sizeof(CIndexTreeChildNode), MAX_DATA_SIZE, MAX_KEY_SIZE, LifeNull<CIndexTreeDataOrderer>());
		mpcRoot = NULL;
		mpcDurableFileControl = NULL;
	}

	lsize ByteSize(void)
	{
		return CIndexTreeFile::ByteSize();
	}

	lsize GetUserMemorySize(void)
	{
		return CIndexTreeFile::ByteSize();
	}
	lsize GetSystemMemorySize(void)
	{
		return CIndexTreeFile::GetSystemMemorySize();
	}

	CIndexedFiles* GetIndexFiles(void)
	{
		return CIndexTreeFile::GetIndexFiles();
	}

	void GetFiles(CArrayIndexedFilePtr* pac)
	{
		return CIndexTreeFile::GetFiles(pac);
	}

	CIndexedFile* GetFile(int iFile)
	{
		return CIndexTreeFile::GetFile(iFile);
	}

	uint32 NumNodes(void)
	{
		return CIndexTreeFile::NumNodes();
	}

	lsize NumMemoryNodes(void)
	{
		return CIndexTreeFile::NumMemoryNodes();
	}

	lsize NumMemoryElements(void)
	{
		return CIndexTreeFile::NumMemoryElements();
	}

	void SetWriteThrough(EIndexWriteThrough eWriteThrough)
	{
		CIndexTreeFile::SetWriteThrough(eWriteThrough);
	}

	bool ValidateIndexTree(bool bReadNodes)
	{
		return CIndexTreeFile::ValidateIndexTree(bReadNodes);
	}

	bool ValidateIndexTree(void)
	{
		return CIndexTreeFile::ValidateIndexTree();
	}

	bool ValidateKey(void* pvKey, size iKeySize)
	{
		return CIndexTreeFile::ValidateKey((uint8*)pvKey, iKeySize);
	}

	CIndexTreeNodeFile* GetNode(const void* pvKey, size iKeySize)
	{
		return CIndexTreeFile::GetNode((uint8*)pvKey, iKeySize);
	}

	size GetNodeKeySize(CIndexTreeNodeFile* pcNode)
	{
		return CIndexTreeFile::GetNodeKeySize(pcNode);
	}

	size GetNodeKey(CIndexTreeNodeFile* pcNode, uint8* pvDestKey, size iDestKeySize)
	{
		return CIndexTreeFile::GetNodeKey(pcNode, pvDestKey, iDestKeySize);
	}

	CIndexTreeNodeFile* GetRoot(void)
	{
		return CIndexTreeFile::GetRoot();
	}

	void GetNodeKey(CIndexTreeNodeFile* pcNode, CArrayChar* pacKey)
	{
		CIndexTree::GetNodeKey(pcNode, pacKey);
	}

	CListCharsMinimal* GetNodesStringKeys(CArrayVoidPtr* apvNodes)
	{
		return CIndexTreeFile::GetNodesStringKeys(apvNodes);
	}
};


#endif // __TEST_INDEX_TREE_FILE_H__


