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
	size		uiFileSize;

	gcLogger.RemoveOutput(&mcMemoryFile);
	mcMemoryFile.Open(EFM_Read);
	cFileBasic.Init(&mcMemoryFile);
	memset(szOutput, 0, uiOutputSize);
	uiFileSize = (size)cFileBasic.Size();
	if (uiFileSize + 1 > uiOutputSize)
	{
		uiFileSize = uiOutputSize - 1;
	}
	cFileBasic.ReadStringChars(szOutput, uiFileSize);
	mcMemoryFile.Close();
	mcMemoryFile.Kill();
	cFileBasic.Kill();

	if (mbSetSilent)
	{
		gcLogger.SetConfig(&msSilentConfig);
	}
}

