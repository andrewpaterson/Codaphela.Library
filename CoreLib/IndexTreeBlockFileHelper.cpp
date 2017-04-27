#include "BaseLib/FileUtil.h"
#include "IndexTreeBlockFileHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlockFileHelper::Init(char* szWorkingDirectory, char* szPrimary, char* szBackup, char* szRootFileName, BOOL bRemoveWorkingDirectory)
{
	CFileUtil					cFileUtil;

	if (bRemoveWorkingDirectory)
	{
		cFileUtil.RemoveDir(szWorkingDirectory);
	}

	mszDirectory.Init(szWorkingDirectory);
	cFileUtil.AppendToPath(&mszDirectory, szPrimary);

	mszRewriteDirectory.Init(szWorkingDirectory);
	cFileUtil.AppendToPath(&mszRewriteDirectory, szBackup);

	mszRootFileName.Init(szWorkingDirectory);
	cFileUtil.AppendToPath(&mszRootFileName, szRootFileName);

	mszWorkingDirectory.Init(szWorkingDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlockFileHelper::Kill(BOOL bRemoveWorkingDirectory)
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
char* CIndexTreeBlockFileHelper::GetPrimaryDirectory(void)
{
	return mszDirectory.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexTreeBlockFileHelper::GetBackupDirectory(void)
{
	return mszRewriteDirectory.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexTreeBlockFileHelper::GetRootFileName(void)
{
	return mszRootFileName.Text();
}

