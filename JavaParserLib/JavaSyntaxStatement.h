#ifndef __JAVA_SYNTAX_STATEMENT_H__
#define __JAVA_SYNTAX_STATEMENT_H__
#include "JavaSyntax.h"


class CJavaSyntaxStatement : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxStatement);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsStatement(void) override;
};


#endif // !__JAVA_SYNTAX_STATEMENT_H__

