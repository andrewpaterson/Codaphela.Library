#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "CoreLib/IndexTreeHelper.h"
#include "IndexedFiles.h"
#include "IndexTreeWriter.h"

//This is not a writer only.  It should be possible to use CIndexTreeFile in memory only mode?  How to track edits to CIndexTreeFile.

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
	
	RecurseWrite(pcIndexTree->GetRoot(), &cIndexTreeFile, cIndexTreeFile.GetRoot());
	cDurableController.End();

	cIndexTreeFile.Kill();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeWriter::RecurseWrite(CIndexTreeNodeMemory* pcMemoryNode, CIndexTreeFile* pcFileTree, CIndexTreeNodeFile* pcFileNode)
{
	int						i;
	CIndexTreeNodeMemory*	pcMemoryChild;
	CIndexTreeNodeFile*		pcFileChild;
	void*					pvObject;
	int						iChildFirstIndex;
	int						iChildLastIndex;
	int						iChildDataSize;


	pvObject = pcMemoryNode->GetObjectPtr();
	if (pvObject != NULL)
	{
		pcFileNode->SetObject(pvObject, pcMemoryNode->GetObjectSize());
	}

	int iFirstIndex;
	int iLastIndex;

	if (pcMemoryNode->HasNodes())
	{
		iFirstIndex = pcMemoryNode->GetFirstIndex();
		iLastIndex = pcMemoryNode->GetLastIndex();
		for (i = iFirstIndex; i <= iLastIndex; i++)
		{
			pcMemoryChild = pcMemoryNode->GetNode(i - iFirstIndex);
			if (pcMemoryChild != NULL)
			{
				iChildDataSize = pcMemoryChild->GetObjectSize();
				if (pcMemoryChild->HasNodes())
				{
					iChildFirstIndex = pcMemoryChild->GetFirstIndex();
					iChildLastIndex = pcMemoryChild->GetLastIndex();

					pcFileChild = pcFileTree->SetParentWithExisting(pcFileNode, i, iChildFirstIndex, iChildLastIndex, iChildDataSize);

					RecurseWrite(pcMemoryChild, pcFileTree, pcFileChild);
				}
				else
				{
					pcFileChild = pcFileTree->SetParentWithExisting(pcFileNode, i, iChildDataSize);

					RecurseWrite(pcMemoryChild, pcFileTree, pcFileChild);
				}
			}
		}

		//for (i = iFirstIndex; i <= iLastIndex; i++)
		//{
		//	pcMemoryChild = pcMemoryNode->GetNode(i - iFirstIndex);
		//	if (pcMemoryChild != NULL)
		//	{
		//		iChildDataSize = pcMemoryChild->GetObjectSize();
		//		if (pcMemoryChild->HasNodes())
		//		{
		//		}
		//	}
		//}
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
	if (iWrittenPos != iNodeSize)
	{
		gcLogger.Error2(__METHOD__, " Could not write IndexTreeNodeFile.  Expected size [", IntToString(iNodeSize), "] is not equal to written buffer size [", IntToString(iWrittenPos), "].", NULL);
		return FALSE;
	}

	pcIndexFile = pcIndexFiles->GetOrCreateFile(iNodeSize);
	if (!pcIndexFile)
	{
		return TRUE;
	}
	iFilePos = pcIndexFile->Write(pvBuffer);
	pcNode->SetFileIndex(pcIndexFile->GetFileIndex(), iFilePos);
	return TRUE;
}

