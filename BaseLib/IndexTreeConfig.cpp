#include "GlobalMemory.h"
#include "Logger.h"
#include "Mallocators.h"
#include "StackMemory.h"
#include "DataOrderers.h"
#include "IndexTreeConfig.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeConfig::Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, size iMaxDataSize, size iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
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
bool CIndexTreeConfig::Init(CFileReader* pcFileReader)
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
	uint32				iLength;
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

	if (StringCompare("IKR_Yes", szName) == 0)
	{
		eKeyReverse = IKR_Yes;
	}
	else if (StringCompare("IKR_No", szName) == 0)
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
bool CIndexTreeConfig::WriteKeyReverse(CFileWriter* pcFileWriter, EIndexKeyReverse	eKeyReverse)
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
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeDataOrderer* CIndexTreeConfig::ReadDataOrderer(CFileReader* pcFileReader)
{
	CIndexTreeDataOrderer*	pcDataOrderer;

	pcDataOrderer = gcDataOrderers.Read(pcFileReader);
	return pcDataOrderer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeConfig::WriteDataOrderer(CFileWriter* pcFileWriter, CIndexTreeDataOrderer* pcDataOrderer)
{
	return gcDataOrderers.Write(pcFileWriter, pcDataOrderer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeConfig::Write(CFileWriter* pcFileWriter)
{
	bool	bResult;

	bResult = gcMallocators.Write(pcFileWriter, mcMalloc.GetLife());
	ReturnOnFalse(bResult);
	bResult = WriteKeyReverse(pcFileWriter, meKeyReverse);
	ReturnOnFalse(bResult);
	bResult = pcFileWriter->WriteSize(miMaxDataSize);
	ReturnOnFalse(bResult);
	bResult = pcFileWriter->WriteSize(miMaxKeySize);
	ReturnOnFalse(bResult);
	bResult = WriteDataOrderer(pcFileWriter, mcDataOrderer.GetLife());
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeConfig::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	CMallocator*						pcMalloc;
	EIndexKeyReverse					eKeyReverse;
	size								iMaxDataSize;
	size								iMaxKeySize;
	CIndexTreeDataOrderer*				pcDataOrderer;
	bool								bResult;
	CLifeInit<CMallocator>				cMalloc;
	CLifeInit<CIndexTreeDataOrderer>	cDataOrderer;

	pcMalloc = gcMallocators.Read(pcFileReader);
	if (pcMalloc == NULL)
	{
		return false;
	}

	eKeyReverse = ReadKeyReverse(pcFileReader);
	if (eKeyReverse == IKR_Unknown)
	{
		return false;
	}

	bResult = pcFileReader->ReadSize(&iMaxDataSize);
	ReturnOnFalse(bResult);

	bResult = pcFileReader->ReadSize(&iMaxKeySize);
	ReturnOnFalse(bResult);

	pcDataOrderer = ReadDataOrderer(pcFileReader);

	cMalloc.Init(pcMalloc, pcMalloc->IsLocal(), pcMalloc->IsLocal());
	cDataOrderer.Init(pcDataOrderer, true, true);
	Init(cMalloc, eKeyReverse, iMaxDataSize, iMaxKeySize, cDataOrderer);
	return true;
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

