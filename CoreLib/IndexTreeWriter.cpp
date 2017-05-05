#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
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
	
	RecurseAllocate(pcIndexTree->GetRoot(), &cIndexTreeFile, cIndexTreeFile.GetRoot());
	RecurseWrite(cIndexTreeFile.GetIndexFiles(), cIndexTreeFile.GetRoot());
	cDurableController.End();

	cIndexTreeFile.Kill();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeWriter::RecurseAllocate(CIndexTreeNodeMemory* pcMemoryNode, CIndexTreeFile* pcFileTree, CIndexTreeNodeFile* pcFileNode)
{
	int						i;
	CIndexTreeNodeMemory*	pcMemoryChild;
	CIndexTreeNodeFile*		pcFileChild;
	void*					pvObject;
	int						iChildFirstIndex;
	int						iChildLastIndex;
	int						iChildDataSize;
	int						iFirstIndex;
	int						iLastIndex;


	pvObject = pcMemoryNode->GetObjectPtr();
	if (pvObject != NULL)
	{
		pcFileNode->SetObject(pvObject, pcMemoryNode->GetObjectSize());
	}

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

					RecurseAllocate(pcMemoryChild, pcFileTree, pcFileChild);
				}
				else
				{
					pcFileChild = pcFileTree->SetParentWithExisting(pcFileNode, i, iChildDataSize);

					RecurseAllocate(pcMemoryChild, pcFileTree, pcFileChild);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeWriter::RecurseWrite(CIndexedFiles* pcIndexFiles, CIndexTreeNodeFile* pcFileNode)
{
	int						i;
	CIndexTreeChildNode*	pcFileChild;
	int						iFirstIndex;
	int						iLastIndex;
	CIndexTreeNodeFile*		pcMemoryChild;
	BOOL					bResult;

	if (pcFileNode->HasNodes())
	{
		iFirstIndex = pcFileNode->GetFirstIndex();
		iLastIndex = pcFileNode->GetLastIndex();
		for (i = iFirstIndex; i <= iLastIndex; i++)
		{
			pcFileChild = pcFileNode->GetNode(i - iFirstIndex);
			if (pcFileChild != NULL)
			{
				if (pcFileChild->IsMemory())
				{
					pcMemoryChild = pcFileChild->u.mpcMemory;

					bResult = RecurseWrite(pcIndexFiles, pcMemoryChild);
					if (!bResult)
					{
						return FALSE;
					}
				}
				else if (pcFileChild->IsFile())
				{
					gcLogger.Error2(__METHOD__, " Should not be file bases nodes during memory tree write.", NULL);
					return FALSE;
				}
			}
		}
	}

	bResult = Write(pcFileNode, pcIndexFiles);
	if (!bResult)
	{
		return FALSE;
	}
	return TRUE;
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

