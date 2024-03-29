#include "IndexTreeMemoryConfig.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemoryConfig::Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
{
	CIndexTreeConfig::Init(cMalloc, eKeyReverse, iMaxDataSize, iMaxKeySize, cDataOrderer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemoryConfig::Init(CFileReader* pcFileReader)
{
	return CIndexTreeConfig::Init(pcFileReader);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemoryConfig::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemoryConfig::Write(CFileWriter* pcFileWriter)
{
	return CIndexTreeConfig::Write(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemoryConfig::Read(CFileReader* pcFileReader)
{
	return CIndexTreeConfig::Read(pcFileReader);
}

