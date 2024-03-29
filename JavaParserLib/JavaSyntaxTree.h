#ifndef __JAVA_SYNTAX_TREE_H__
#define __JAVA_SYNTAX_TREE_H__
#include "JavaSyntaxFile.h"


class CJavaSyntaxTree
{
protected:
	CJavaSyntaxFile*	mpcRoot;

public:
	void				Init(void);
	void				Kill(void);

	void				TypePrint(CChars* pszDest);
	char*				PrettyPrint(CChars* pszDest);

	void				SetRoot(CJavaSyntaxFile* pcRoot);
	CJavaSyntaxFile*	GetRoot(void);
};


#endif // !__JAVA_SYNTAX_TREE_H__

