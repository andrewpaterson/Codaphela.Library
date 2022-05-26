#ifndef __JAVA_SYNTAX_ARRAY_VALUE_EXPRESSION_H__
#define __JAVA_SYNTAX_ARRAY_VALUE_EXPRESSION_H__
#include "JavaSyntaxExpressionCommon.h"


class CJavaSyntaxArrayValueExpression : public CJavaSyntaxExpressionCommon
{
CONSTRUCTABLE(CJavaSyntaxArrayValueExpression);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsArrayValueExpression(void) override;
};


#endif // !__JAVA_SYNTAX_ARRAY_VALUE_EXPRESSION_H__

