#include "CoreLib/IndexTreeHelper.h"
#include "IndexedFiles.h"
#include "IndexTreeWriter.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeWriter::Write(CIndexTreeMemory* pcIndexTree, char* szDirectory)
{
	CDurableFileController	cDurableController;
	BOOL					bResult;
	CIndexTreeFile			cIndexTreeFile;
	CIndexTreeHelper		cHelper;

	cHelper.Init(szDirectory, NULL, NULL, "RootFile.IDX", FALSE);
	bResult = cDurableController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	if (!bResult)
	{
		return FALSE;
	}
	
	cDurableController.Begin();
	cIndexTreeFile.Init(&cDurableController, cHelper.GetRootFileName(), &gcSystemAllocator);

	
	RecurseWrite(pcIndexTree->GetRoot(), cIndexTreeFile.GetRoot());
	cDurableController.End();

	cIndexTreeFile.Kill();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeWriter::RecurseWrite(CIndexTreeNodeMemory* pcMemoryNode, CIndexTreeNodeFile* pcFileNode)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;
	void*					pvObject;

	pvObject = pcMemoryNode->GetObjectPtr();
	if (pvObject != NULL)
	{
		//Has object;
	}

	int iFirstIndex;
	int iLastIndex;

	iFirstIndex = pcMemoryNode->GetFirstIndex();
	iLastIndex = pcMemoryNode->GetLastIndex();
	for (i = iFirstIndex; i <= iLastIndex; i++)
	{
		pcChild = pcMemoryNode->GetNode(i - iFirstIndex);
		if (pcChild != NULL)
		{
//			pcChild->
//			RecurseWrite(pcChild)
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

