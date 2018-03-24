#include "BaseLib/FileUtil.h"
#include "IndexTreeHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeHelper::Init(char* szWorkingDirectory, char* szPrimary, char* szBackup, char* szRootFileName, BOOL bRemoveWorkingDirectory)
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

	mszRootFileName.Init(szWorkingDirectory);
	cFileUtil.AppendToPath(&mszRootFileName, szRootFileName);

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
	mszRootFileName.Kill();
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
char* CIndexTreeHelper::GetRootFileName(void)
{
	return mszRootFileName.Text();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeHelper::RemoveWorkingDirectory(void)
{
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir(mszWorkingDirectory.Text());
}

