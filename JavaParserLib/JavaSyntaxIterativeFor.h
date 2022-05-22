#ifndef __JAVA_SYNTAX_ITERATIVE_FOR_H__
#define __JAVA_SYNTAX_ITERATIVE_FOR_H__
#include "JavaSyntaxStatement.h"

//for {? : ?} { ... }
class CJavaSyntaxIterativeFor : public CJavaSyntaxStatement
{
	CONSTRUCTABLE(CJavaSyntaxIterativeFor);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsIterativeFor(void) override;
};


#endif // !__JAVA_SYNTAX_ITERATIVE_FOR_H__

