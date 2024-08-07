#include "Logger.h"
#include "FileBasic.h"
#include "LogToMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogToMemory::Start(bool bSetSilent)
{
	mcMemoryFile.Init();
	gcLogger.AddOutput(&mcMemoryFile);

	mbSetSilent = bSetSilent;
	if (mbSetSilent)
	{
		msSilentConfig = gcLogger.SetSilent();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogToMemory::Stop(char* szOutput, size uiOutputSize)
{
	CFileBasic	cFileBasic;

	gcLogger.RemoveOutput(&mcMemoryFile);
	mcMemoryFile.Open(EFM_Read);
	cFileBasic.Init(&mcMemoryFile);
	memset(szOutput, 0, uiOutputSize);
	cFileBasic.ReadStringChars(szOutput, (size)cFileBasic.Size());
	mcMemoryFile.Close();
	mcMemoryFile.Kill();
	cFileBasic.Kill();

	if (mbSetSilent)
	{
		gcLogger.SetConfig(&msSilentConfig);
	}
}

