#include "BaseLib/FileUtil.h"
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
	bResult = PrivateAssertString(szExpectedWorking.Text(), szActual, false, iLine, szFile);
	szExpectedWorking.Kill();
	szWorking.Kill();

	return bResult;
}

