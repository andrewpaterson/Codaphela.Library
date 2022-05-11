#ifndef __JAVA_SYNTAX_TREE_H__
#define __JAVA_SYNTAX_TREE_H__
#include "JavaSyntaxFile.h"


class CJavaSyntaxTree
{
protected:
	CJavaSyntaxFile*	mpcRoot;
	CChars				mszFileName;

public:
	void	Init(char* szFileFile);
	void	Kill(void);

	void	SetRoot(CJavaSyntaxFile* pcRoot);
	char*	GetFileName(void);
};


#endif // !__JAVA_SYNTAX_TREE_H__

