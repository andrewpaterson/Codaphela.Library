#ifndef __JAVA_SYNTAX_VOID_EXPRESSION_H__
#define __JAVA_SYNTAX_VOID_EXPRESSION_H__
#include "JavaSyntaxStatement.h"


class CJavaSyntaxVoidExpression : public CJavaSyntaxStatement
{
CONSTRUCTABLE(CJavaSyntaxVoidExpression);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;

	BOOL	IsVoidExpression(void) override;
};


#endif // !__JAVA_SYNTAX_VOID_EXPRESSION_H__

