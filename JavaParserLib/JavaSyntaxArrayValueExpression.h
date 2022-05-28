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
	void	TypePrint(CChars* pszDest, int iDepth) override;
	char*	PrettyPrint(CChars* pszDest) override;

	BOOL	IsArrayValueExpression(void) override;
};


#endif // !__JAVA_SYNTAX_ARRAY_VALUE_EXPRESSION_H__

