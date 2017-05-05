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
	RecurseWrite(&cIndexTreeFile, cIndexTreeFile.GetRoot());
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
BOOL CIndexTreeWriter::RecurseWrite(CIndexTreeFile* pcFileTree, CIndexTreeNodeFile* pcFileNode)
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

					bResult = RecurseWrite(pcFileTree, pcMemoryChild);
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

	bResult = pcFileTree->Write(pcFileNode);
	if (!bResult)
	{
		return FALSE;
	}
	return TRUE;
}

