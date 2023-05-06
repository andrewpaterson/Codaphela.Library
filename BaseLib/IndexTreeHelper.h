#ifndef __INDEX_TREE_BLOCK_FILE_HELPER_H__
#define __INDEX_TREE_BLOCK_FILE_HELPER_H__
#include "Chars.h"


class CIndexTreeHelper
{
public:
	CChars	mszWorkingDirectory;
	CChars	mszDirectory;
	CChars	mszRewriteDirectory;

	void	Init(char* szWorkingDirectory, char* szPrimary, char* szBackup, bool bRemoveWorkingDirectory);
	void	Kill(bool bRemoveWorkingDirectory);

	char*	GetPrimaryDirectory(void);
	char*	GetBackupDirectory(void);

	bool	RemoveWorkingDirectory(void);
};


#endif // !__INDEX_TREE_BLOCK_FILE_HELPER_H__

