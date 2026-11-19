#include "FileUtil.h"
#include "IndexTreeHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeHelper::Init(const char* szWorkingDirectory, const char* szPrimary, const char* szBackup, bool bRemoveWorkingDirectory)
{
	CFileUtil					cFileUtil;
	bool						bPrimary;
	bool						bBackup;

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
void CIndexTreeHelper::Kill(bool bRemoveWorkingDirectory)
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
const char* CIndexTreeHelper::GetPrimaryDirectory(void) const
{
	return mszDirectory.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CIndexTreeHelper::GetBackupDirectory(void) const
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
bool CIndexTreeHelper::RemoveWorkingDirectory(void)
{
	CFileUtil	cFileUtil;

	return cFileUtil.RemoveDir(mszWorkingDirectory.Text());
}

