#include "BaseLib/FileUtil.h"
#include "BaseLib/FileCompare.h"
#include "AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertFilePath(char* szExpected, char* szActual, int iLine, char* szFile)
{
	CChars			szWorking;
	CChars			szExpectedWorking;
	CFileUtil		cFileUtil;
	bool			bResult;

	szWorking.Init();
	cFileUtil.CurrentDirectory(&szWorking);
	szExpectedWorking.Init(szExpected);
	cFileUtil.PrependToPath(&szExpectedWorking, szWorking.Text());
	bResult = PrivateAssertString(szExpectedWorking.Text(), szActual, false, NULL, iLine, szFile);
	szExpectedWorking.Kill();
	szWorking.Kill();

	return bResult;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertFile(const char* szExpectedFilename, char* szActualFilename, char* szPrefix, size iLine, char* szFile)
{
	CFileCompare	cCompare;
	bool			bResult;
	CChars			szExpected;
	CChars			szActual;

	szExpected.Init();
	szActual.Init();
	bResult = cCompare.Compare(szExpectedFilename, szActualFilename, &szExpected, &szActual);

	if (!bResult)
	{
		bResult = Failed((const char*)szExpected.Text(), (const char*)szActual.Text(), szPrefix, iLine, szFile, false);
		szActual.Kill();
		szExpected.Kill();
		return bResult;
	}
	else
	{
		szActual.Kill();
		szExpected.Kill();
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertFile(CChars szExpectedFilename, CChars szActualFilename, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertFile(szExpectedFilename.Text(), szActualFilename.Text(), szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertFileMemory(const char* szExpectedFilename, void* pcMemory, size iLength, char* szPrefix, size iLine, char* szFile)
{
	CFileCompare	cCompare;
	bool			bResult;
	CChars			szExpected;
	CChars			szActual;

	szExpected.Init();
	szActual.Init();
	bResult = cCompare.Compare(szExpectedFilename, pcMemory, iLength, &szExpected, &szActual);

	if (!bResult)
	{
		Failed((const char*)szExpected.Text(), (const char*)szActual.Text(), szPrefix, iLine, szFile, false);
		szActual.Kill();
		szExpected.Kill();
		return false;
	}
	else
	{
		szActual.Kill();
		szExpected.Kill();
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertFileString(const char* szExpectedFilename, const char* szString, char* szPrefix, size iLine, char* szFile)
{
	size iLength;

	iLength = strlen(szString);
	return PrivateAssertFileMemory(szExpectedFilename, (void*)szString, iLength, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertDirectory(char* szExpectedDirectoryName, char* szActualDirectoryName, char* szPrefix, size iLine, char* szFile)
{
	CArrayChars				aszExpectedFiles;
	CArrayChars				aszActualFiles;
	CFileUtil				cFileUtil;
	size					i;
	size					iNumElements;
	CChars*					pszExpectedFilename;
	CChars*					pszActualFilename;
	CChars					szExpectedFilename;
	CChars					szActualFilename;
	bool					bValid;

	aszExpectedFiles.Init();
	cFileUtil.FindAllFiles(szExpectedDirectoryName, &aszExpectedFiles, false, false);

	aszActualFiles.Init();
	cFileUtil.FindAllFiles(szActualDirectoryName, &aszActualFiles, false, false);
	AssertSize(aszExpectedFiles.NumElements(), aszActualFiles.NumElements());

	aszExpectedFiles.BubbleSort();
	aszActualFiles.BubbleSort();

	iNumElements = aszExpectedFiles.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pszExpectedFilename = aszExpectedFiles.Get(i);
		pszActualFilename = aszActualFiles.Get(i);

		szExpectedFilename.Init();
		cFileUtil.GetFileName(&szExpectedFilename, pszExpectedFilename->Text());

		szActualFilename.Init();
		cFileUtil.GetFileName(&szActualFilename, pszActualFilename->Text());
		
		bValid = PrivateAssertString(szExpectedFilename.Text(), szActualFilename.Text(), true, szPrefix, iLine, szFile);
		if (!bValid)
		{
			return false;
		}
		bValid = PrivateAssertFile(pszExpectedFilename->Text(), pszActualFilename->Text(), szPrefix, iLine, szFile);
		if (!bValid)
		{
			return false;
		}

		szExpectedFilename.Kill();
		szActualFilename.Kill();
	}

	aszExpectedFiles.Kill();
	aszActualFiles.Kill();

	return true;
}

