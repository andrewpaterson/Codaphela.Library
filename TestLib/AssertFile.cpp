#include "BaseLib/FileUtil.h"
#include "AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL PrivateAssertFilePath(char* szExpected, char* szActual, int iLine, char* szFile)
{
	CChars			szWorking;
	CChars			szExpectedWorking;
	CFileUtil		cFileUtil;
	BOOL			bResult;

	szWorking.Init();
	cFileUtil.CurrentDirectory(&szWorking);
	szExpectedWorking.Init(szExpected);
	cFileUtil.PrependToPath(&szExpectedWorking, szWorking.Text());
	bResult = PrivateAssertString(szExpectedWorking.Text(), szActual, FALSE, iLine, szFile);
	szExpectedWorking.Kill();
	szWorking.Kill();

	return bResult;
}

