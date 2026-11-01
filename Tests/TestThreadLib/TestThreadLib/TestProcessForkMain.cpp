//#include "BaseLib/WindowsHeaders.h"
//MessageBox(NULL, sz.Text(), "ClientMain invoked", MB_OK);
#include "BaseLib/Logger.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestClientProcessMain(int argc, char* argv[])
{
	char*		szFileName;
	CTextFile	cTextFile;
	CFileUtil	cFileUtil;
	CChars		sz;
	bool		bResult;
	
	if (argc == 1)
	{
		szFileName = argv[0];
		sz.Init();
		cFileUtil.CurrentDirectory(&sz);
		sz.Append(_FS_);
		sz.Append(szFileName);
		sz.Kill();

		cTextFile.Init("Hello World\n");
		bResult = cTextFile.Write(szFileName);
		cTextFile.Kill();
		if (!bResult)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return -1;
	}
}

