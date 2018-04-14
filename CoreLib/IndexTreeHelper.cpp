#include "BaseLib/FileUtil.h"
#include "IndexTreeHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeHelper::Init(char* szWorkingDirectory, char* szPrimary, char* szBackup, BOOL bRemoveWorkingDirectory)
{
	CFileUtil					cFileUtil;
	BOOL						bPrimary;
	BOOL						bBackup;

	if (bRemoveWorkingDirectory)
	{
		cFileUtil.RemoveDir(szWorkingDirectory);
	}

	mszDirectory.Init(szWorkingDirectory);
	bPrimary = !StrEmpty(szPrimary);
	if (bPrimary)
	{
		cFileUtil.AppendToPath(&mszDirectory, szPrimary);
	}

	bBackup = !StrEmpty(szBackup);
	if (bPrimary && bBackup)
	{
		mszRewriteDirectory.Init(szWorkingDirectory);
		cFileUtil.AppendToPath(&mszRewriteDirectory, szBackup);
	}
	else
	{
		mszRewriteDirectory.Init();
	}

	mszWorkingDirectory.Init(szWorkingDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeHelper::Kill(BOOL bRemoveWorkingDirectory)
{
	CFileUtil					cFileUtil;

	if (bRemoveWorkingDirectory)
	{
		cFileUtil.RemoveDir(mszWorkingDirectory.Text());
	}

	mszWorkingDirectory.Kill();
	mszDirectory.Kill();
	mszRewriteDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexTreeHelper::GetPrimaryDirectory(void)
{
	return mszDirectory.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexTreeHelper::GetBackupDirectory(void)
{
	if (mszRewriteDirectory.Length() > 0)
	{
		return mszRewriteDirectory.Text();
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeHelper::RemoveWorkingDirectory(void)
{
	CFileUtil	cFileUtil;

	return cFileUtil.RemoveDir(mszWorkingDirectory.Text());
}

