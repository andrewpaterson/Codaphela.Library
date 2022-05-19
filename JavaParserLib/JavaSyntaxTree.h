#ifndef __JAVA_SYNTAX_TREE_H__
#define __JAVA_SYNTAX_TREE_H__
#include "JavaSyntaxFile.h"


class CJavaSyntaxTree
{
protected:
	CJavaSyntaxFile*	mpcRoot;
	CChars				mszFilename;

public:
	void	Init(char* szFilename);
	void	Kill(void);

	void	SetRoot(CJavaSyntaxFile* pcRoot);
	char*	GetFilename(void);
};


#endif // !__JAVA_SYNTAX_TREE_H__

