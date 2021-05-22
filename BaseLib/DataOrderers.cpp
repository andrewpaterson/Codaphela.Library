#include "GlobalMemory.h"
#include "Logger.h"
#include "LogString.h"
#include "ModificationDataOrderer.h"
#include "AccessDataOrderer.h"
#include "CreationDataOrderer.h"
#include "DataOrderers.h"


CDataOrderers	gcDataOrderers;
BOOL			gbDataOrderers = FALSE;

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
CIndexTreeDataOrderer* CDataOrderers::Read(CFileReader* pcFileReader)
{
	int						iLength;
	char					szName[1024];
	CIndexTreeDataOrderer**	ppcDataOrderer;
	CIndexTreeDataOrderer*	pcDataOrderer;
	
	if (!MemoryValidate() || !DataOrderersValidate())
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

	if (StrEmpty(szName))
	{
		return NULL;
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
	return pcDataOrderer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDataOrderers::Write(CFileWriter* pcFileWriter, CIndexTreeDataOrderer* pcDataOrderer)
{
	if (!MemoryValidate() || !DataOrderersValidate())
	{
		return FALSE;
	}

	if (pcDataOrderer == NULL)
	{
		if (!pcFileWriter->WriteString(""))
		{
			gcLogger.Error2(__METHOD__, " Could not write NULL data orderer.", NULL);
			return FALSE;
		}
	}
	else
	{
		if (!pcFileWriter->WriteString(pcDataOrderer->ClassName()))
		{
			gcLogger.Error2(__METHOD__, " Could not write data orderer name [", pcDataOrderer->ClassName(), "].", NULL);
			return FALSE;
		}
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DataOrderersInit(void)
{
	DataOrderersInit(&gcConstructors, &gcDataOrderers);
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

	gbDataOrderers = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DataOrderersKill(void)
{
	gcDataOrderers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL DataOrderersValidate(void)
{
	if (!gbDataOrderers)
	{
		gcLogger.Error("Global Data Orderers have not been initialised.  Call DataOrderersInit.");
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

