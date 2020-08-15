#include "GlobalMemory.h"
#include "Logger.h"
#include "Mallocators.h"
#include "StackMemory.h"
#include "IndexTreeConfig.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeConfig::Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
{
	mcMalloc = cMalloc;
	meKeyReverse = eKeyReverse;
	miMaxDataSize = iMaxDataSize;
	miMaxKeySize = iMaxKeySize;
	mcDataOrderer = cDataOrderer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeConfig::Init(CFileReader* pcFileReader)
{
	
	return Read(pcFileReader);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeConfig::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EIndexKeyReverse CIndexTreeConfig::ReadKeyReverse(CFileReader* pcFileReader)
{
	int					iLength;
	CStackMemory<32>	cStack;
	char*				szName;
	EIndexKeyReverse	eKeyReverse;

	if (!pcFileReader->ReadStringLength(&iLength))
	{
		gcLogger.Error2(__METHOD__, " Could not read Key Reverse name length.", NULL);
		return IKR_Unknown;
	}

	szName = (char*)cStack.Init(iLength);
	if (!pcFileReader->ReadStringChars(szName, iLength))
	{
		gcLogger.Error2(__METHOD__, " Could not read Key Reverse name.", NULL);
		cStack.Kill();
		return IKR_Unknown;
	}

	if (strcmp("IKR_Yes", szName) == 0)
	{
		eKeyReverse = IKR_Yes;
	}
	else if (strcmp("IKR_No", szName) == 0)
	{
		eKeyReverse = IKR_No;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Could not match Key Reverse name.", NULL);
		eKeyReverse = IKR_Unknown;
	}
	cStack.Kill();

	return eKeyReverse;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeConfig::WriteKeyReverse(CFileWriter* pcFileWriter, EIndexKeyReverse	eKeyReverse)
{
	if (eKeyReverse == IKR_Yes)
	{
		return pcFileWriter->WriteString("IKR_Yes");
	}
	else if (eKeyReverse = IKR_No)
	{
		return pcFileWriter->WriteString("IKR_No");
	}
	else
	{
		pcFileWriter->WriteString("IKR_Unknown");
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeDataOrderer* CIndexTreeConfig::ReadDataOrderer(CFileReader* pcFileReader)
{
	int						iLength;
	CStackMemory<1024>		cStack;
	char*					szName;
	CIndexTreeDataOrderer* pcDataOrderer;

	if (!MemoryValidate())
	{
		return NULL;
	}

	if (!pcFileReader->ReadStringLength(&iLength))
	{
		gcLogger.Error2(__METHOD__, " Could not read Data Orderer name length.", NULL);
		return FALSE;
	}
	
	szName = (char*)cStack.Init(iLength);
	if (!pcFileReader->ReadStringChars(szName, iLength))
	{
		gcLogger.Error2(__METHOD__, " Could not read Data Orderer name.", NULL);
		cStack.Kill();
		return FALSE;
	}

	pcDataOrderer = (CAccessDataOrderer*)gcConstructors.Construct(szName, &gcSystemAllocator);
	cStack.Kill();

	return pcDataOrderer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeConfig::WriteDataOrderer(CFileWriter* pcFileWriter, CIndexTreeDataOrderer* pcDataOrderer)
{
	return pcFileWriter->WriteString(pcDataOrderer->ClassName());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeConfig::Write(CFileWriter* pcFileWriter)
{
	BOOL	bResult;

	bResult = gcMallocators.WriteMallocator(pcFileWriter, mcMalloc.GetLife());
	ReturnOnFalse(bResult);
	bResult = WriteKeyReverse(pcFileWriter, meKeyReverse);
	ReturnOnFalse(bResult);
	bResult = pcFileWriter->WriteInt(miMaxDataSize);
	ReturnOnFalse(bResult);
	bResult = pcFileWriter->WriteInt(miMaxKeySize);
	ReturnOnFalse(bResult);
	bResult = WriteDataOrderer(pcFileWriter, mcDataOrderer.GetLife());
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeConfig::Read(CFileReader* pcFileReader)
{
	CMallocator*						pcMalloc;
	EIndexKeyReverse					eKeyReverse;
	int									iMaxDataSize;
	int									iMaxKeySize;
	CIndexTreeDataOrderer*				pcDataOrderer;
	BOOL								bResult;
	CLifeInit<CMallocator>				cMalloc;
	CLifeInit<CIndexTreeDataOrderer>	cDataOrderer;

	pcMalloc = gcMallocators.ReadMallocator(pcFileReader);
	if (pcMalloc == NULL)
	{
		return FALSE;
	}

	eKeyReverse = ReadKeyReverse(pcFileReader);
	if (eKeyReverse == IKR_Unknown)
	{
		return FALSE;
	}

	bResult = pcFileReader->ReadInt(&iMaxDataSize);
	ReturnOnFalse(bResult);

	bResult = pcFileReader->ReadInt(&iMaxKeySize);
	ReturnOnFalse(bResult);

	pcDataOrderer = ReadDataOrderer(pcFileReader);
	if (pcDataOrderer == NULL)
	{
		return FALSE;
	}

	cMalloc.Init(pcMalloc, pcMalloc->IsLocal(), pcMalloc->IsLocal());
	cDataOrderer.Init(pcDataOrderer, TRUE, TRUE);
	Init(cMalloc, eKeyReverse, iMaxDataSize, iMaxKeySize, cDataOrderer);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CMallocator> CIndexTreeConfig::GetMalloc(void) { return mcMalloc; }
EIndexKeyReverse CIndexTreeConfig::GetKeyReverse(void) { return meKeyReverse; }
int CIndexTreeConfig::GetMaxDataSize(void) { return miMaxDataSize; }
int CIndexTreeConfig::GetMaxKeySize(void) { return miMaxKeySize; }
CLifeInit<CIndexTreeDataOrderer> CIndexTreeConfig::GetDataOrderer(void) { return mcDataOrderer; }

