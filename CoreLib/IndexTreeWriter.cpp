#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "CoreLib/IndexTreeHelper.h"
#include "IndexedFiles.h"
#include "IndexTreeWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeWriter::Write(CIndexTreeMemory* pcIndexTreeMemory, char* szDirectory)
{
	CDurableFileController	cController;
	CIndexTreeFile			cIndexTreeFile;
	CIndexTreeHelper		cHelper;

	cHelper.Init(szDirectory, NULL, NULL, false);
	ReturnOnFalse(cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory()));
	ReturnOnFalse(cController.Begin());

	ReturnOnFalse(cIndexTreeFile.Init(&cController, NULL, IWT_Yes, pcIndexTreeMemory->ReverseKeys()));
	
	RecurseAllocate(pcIndexTreeMemory->GetRoot(), &cIndexTreeFile, cIndexTreeFile.GetRoot());
	RecurseWrite(&cIndexTreeFile, cIndexTreeFile.GetRoot());
	
	ReturnOnFalse(cController.End());
	cController.Kill();
	cIndexTreeFile.Kill();
	cHelper.Kill(true);
	return true;
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
bool CIndexTreeWriter::RecurseWrite(CIndexTreeFile* pcFileTree, CIndexTreeNodeFile* pcFileNode)
{
	int						i;
	CIndexTreeChildNode*	pcFileChild;
	CIndexTreeNodeFile*		pcMemoryChild;
	bool					bResult;
	CIndexTreeChildNode*	apcChildren;
	int						iNumNodes;

	if (pcFileNode->HasNodes())
	{
		iNumNodes = pcFileNode->NumIndexes();
		apcChildren = pcFileNode->GetNodes();
		for (i = 0; i < iNumNodes; i++)
		{
			pcFileChild = &apcChildren[i];
			if (pcFileChild != NULL)
			{
				if (pcFileChild->IsMemory())
				{
					pcMemoryChild = pcFileChild->u.mpcMemory;

					bResult = RecurseWrite(pcFileTree, pcMemoryChild);
					if (!bResult)
					{
						return false;
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
		return false;
	}
	return true;
}

