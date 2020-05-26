#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "CoreLib/IndexTreeHelper.h"
#include "IndexedFiles.h"
#include "IndexTreeWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeWriter::Write(CIndexTreeMemory* pcIndexTreeMemory, char* szDirectory)
{
	CDurableFileController	cController;
	CIndexTreeFile			cIndexTreeFile;
	CIndexTreeHelper		cHelper;

	cHelper.Init(szDirectory, NULL, NULL, FALSE);
	ReturnOnFalse(cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory()));
	ReturnOnFalse(cController.Begin());

	ReturnOnFalse(cIndexTreeFile.Init(&cController, NULL, pcIndexTreeMemory->ReverseKeys()));
	
	RecurseAllocate(pcIndexTreeMemory->GetRoot(), &cIndexTreeFile, cIndexTreeFile.GetRoot());
	RecurseWrite(&cIndexTreeFile, cIndexTreeFile.GetRoot());
	
	ReturnOnFalse(cController.End());
	cController.Kill();
	cIndexTreeFile.Kill();
	cHelper.Kill(TRUE);
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

	pvObject = pcMemoryNode->GetDataPtr();
	if (pvObject != NULL)
	{
		pcFileNode->SetData(pvObject, pcMemoryNode->GetDataSize());
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
				iChildDataSize = pcMemoryChild->GetDataSize();

				if (pcMemoryChild->HasNodes())
				{
					iChildFirstIndex = pcMemoryChild->GetFirstIndex();
					iChildLastIndex = pcMemoryChild->GetLastIndex();

					pcFileChild = pcFileTree->ParentPut(pcFileNode, i, iChildDataSize, iChildFirstIndex, iChildLastIndex);

					RecurseAllocate(pcMemoryChild, pcFileTree, pcFileChild);
				}
				else
				{
					pcFileChild = pcFileTree->ParentPut(pcFileNode, i, iChildDataSize);

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
					return gcLogger.Error2(__METHOD__, " Should not be file bases nodes during memory tree write.", NULL);
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

