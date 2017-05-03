#include "IndexedFiles.h"
#include "IndexTreeWriter.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeWriter::Write(CIndexTreeMemory* pcIndexTree, char* szDirectory)
{
	CIndexedFiles			cIndexedFiles;
	CDurableFileController	cController;
	BOOL					bResult;

	bResult = cController.Init(szDirectory, NULL);
	if (!bResult)
	{
		return FALSE;
	}

	cIndexedFiles.Init(&cController, "IDAT", "Index.IDX", NULL);

	WriteAll(pcIndexTree, &cIndexedFiles);
	SIndexTreeIterator	sIter;
	void*				pvData;
	int					iDataSize;
	BOOL				bExists;
	unsigned char		paucKey[8 KB + 1];
	int					iKeyLength;

	if (pcIndexTree->GetLargestKeySize() > 8 KB)
	{
		//Fix this to use a scratch pad.
		return FALSE;
	}

	bExists = pcIndexTree->StartIteration(&sIter, &pvData, &iDataSize);
	while (bExists)
	{
		iKeyLength = pcIndexTree->GetKey(paucKey, pvData, TRUE);


		bExists = pcIndexTree->Iterate(&sIter, &pvData, &iDataSize);
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeWriter::WriteAll(CIndexTreeMemory* pcIndexTree, CIndexedFiles* pcIndexedFiles)
{
	RecurseWriteAll(pcIndexTree->GetRoot(), pcIndexedFiles);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeWriter::RecurseWriteAll(CIndexTreeNodeMemory* pcNode, CIndexedFiles* pcIndexedFiles)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;
	void*					pvObject;

	if (pcNode != NULL)
	{
		pvObject = pcNode->GetObjectPtr();
		if (pvObject != NULL)
		{
			//Has object;
		}

		xxx
		for (i = 0; i < pcNode->GetNumIndexes(); i++)
		{
			pcChild = pcNode->GetNode(i);
			RecurseWriteAll(pcChild, pcIndexedFiles);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeWriter::Write(CIndexTreeNodeFile* pcNode, CIndexedFiles* pcIndexFiles)
{
	int					iWrittenPos;
	char				pvBuffer[8 KB];
	int					iNodeSize;
	CIndexedFile*		pcIndexFile;
	filePos				iFilePos;

	iWrittenPos = pcNode->WriteToBuffer(pvBuffer, 8 KB);
	if (iWrittenPos <= 0)
	{
		return FALSE;
	}

	iNodeSize = pcNode->CalculateBufferSize();
	pcIndexFile = pcIndexFiles->GetOrCreateFile(iNodeSize);
	if (!pcIndexFile)
	{
		return TRUE;
	}
	iFilePos = pcIndexFile->Write(pvBuffer);
	pcNode->SetFileIndex(pcIndexFile->GetFileIndex(), iFilePos);
	return TRUE;
}

