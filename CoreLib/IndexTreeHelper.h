#ifndef __INDEX_TREE_BLOCK_FILE_HELPER_H__
#define __INDEX_TREE_BLOCK_FILE_HELPER_H__
#include "BaseLib/Chars.h"


class CIndexTreeHelper
{
public:
	CChars	mszWorkingDirectory;
	CChars	mszDirectory;
	CChars	mszRewriteDirectory;

	void	Init(char* szWorkingDirectory, char* szPrimary, char* szBackup, BOOL bRemoveWorkingDirectory);
	void	Kill(BOOL bRemoveWorkingDirectory);

	char*	GetPrimaryDirectory(void);
	char*	GetBackupDirectory(void);

	BOOL	RemoveWorkingDirectory(void);
};


#endif // __INDEX_TREE_BLOCK_FILE_HELPER_H__

