#include "GlobalMemory.h"
#include "Logger.h"
#include "LogString.h"
#include "ModificationDataOrderer.h"
#include "AccessDataOrderer.h"
#include "CreationDataOrderer.h"
#include "DataOrderers.h"


CDataOrderers	gcDataOrderers;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataOrderers::Init(void)
{
	mmszClasses.Init(TRUE, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataOrderers::Kill(void)
{
	mmszClasses.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDataOrderers::Add(CIndexTreeDataOrderer* pcMalloc)
{
	char* sz;

	sz = (char*)(pcMalloc->ClassName());
	if (mmszClasses.Get(sz))
	{
		gcLogger.Error2(__METHOD__, " A mallocator named [", sz, "] already exists.", NULL);
		return FALSE;
	}

	mmszClasses.Put(sz, &pcMalloc);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeDataOrderer* CDataOrderers::Read(CFileReader* pcFileReader, CIndexTree* pcIndexTree)
{
	int						iLength;
	char					szName[1024];
	CIndexTreeDataOrderer**	ppcDataOrderer;
	CIndexTreeDataOrderer*	pcDataOrderer;

	if (!MemoryValidate())
	{
		return NULL;
	}

	if (!pcFileReader->ReadStringLength(&iLength))
	{
		gcLogger.Error2(__METHOD__, " Could not read data orderer name length.", NULL);
		return FALSE;
	}

	if ((iLength < 0 || iLength >= 1024))
	{
		gcLogger.Error2(__METHOD__, " Could not read data orderer name, too long [", IntToString(iLength), "].", NULL);
	}

	if (!pcFileReader->ReadStringChars(szName, iLength))
	{
		gcLogger.Error2(__METHOD__, " Could not read data orderer name.", NULL);
		return FALSE;
	}

	ppcDataOrderer = mmszClasses.Get(szName);
	if (!ppcDataOrderer)
	{
		gcLogger.Error2(__METHOD__, " Could not find data orderer named [", szName, "].", NULL);
		return FALSE;
	}

	pcDataOrderer = *ppcDataOrderer;
	pcDataOrderer = (CIndexTreeDataOrderer*)gcConstructors.Construct(pcDataOrderer->ClassName(), &gcSystemAllocator);
	if (!pcDataOrderer)
	{
		gcLogger.Error2(__METHOD__, " Could not construct data orderer [", (*ppcDataOrderer)->ClassName(), "].", NULL);
		return NULL;
	}
	pcDataOrderer->SetIndexTree(pcIndexTree);
	return pcDataOrderer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDataOrderers::Write(CFileWriter* pcFileWriter, CIndexTreeDataOrderer* pcMalloc)
{
	if (!MemoryValidate())
	{
		return FALSE;
	}

	if (pcMalloc == NULL)
	{
		gcLogger.Error2(__METHOD__, " Could not write NULL data orderer.", NULL);
		return FALSE;
	}

	if (!pcFileWriter->WriteString(pcMalloc->ClassName()))
	{
		gcLogger.Error2(__METHOD__, " Could not write data orderer name [", pcMalloc->ClassName(), "].", NULL);
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DataOrderersInit(CConstructors* pcConstructors, CDataOrderers* pcDataOrderers)
{
	CAccessDataOrderer*			pcAccessDataOrderer;
	CModificationDataOrderer*	pcModificationDataOrderer;
	CCreationDataOrderer*		pcCreationDataOrderer;

	gcDataOrderers.Init();

	pcAccessDataOrderer = pcConstructors->Add<CAccessDataOrderer>();
	pcModificationDataOrderer = pcConstructors->Add<CModificationDataOrderer>();
	pcCreationDataOrderer = pcConstructors->Add<CCreationDataOrderer>();

	pcDataOrderers->Add(pcAccessDataOrderer);
	pcDataOrderers->Add(pcModificationDataOrderer);
	pcDataOrderers->Add(pcCreationDataOrderer);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DataOrderersKill(void)
{
	gcDataOrderers.Kill();
}

