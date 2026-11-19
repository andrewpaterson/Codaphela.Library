#ifndef __INDEX_TREE_BLOCK_FILE_HELPER_H__
#define __INDEX_TREE_BLOCK_FILE_HELPER_H__
#include "Chars.h"


class CIndexTreeHelper
{
public:
	CChars	mszWorkingDirectory;
	CChars	mszDirectory;
	CChars	mszRewriteDirectory;

	void		Init(const char* szWorkingDirectory, const char* szPrimary, const char* szBackup, bool bRemoveWorkingDirectory);
	void		Kill(bool bRemoveWorkingDirectory);

	const char*	GetPrimaryDirectory(void) const;
	const char*	GetBackupDirectory(void) const;

	bool		RemoveWorkingDirectory(void);
};


#endif // __INDEX_TREE_BLOCK_FILE_HELPER_H__

