#ifndef __JAVA_SYNTAX_VALUE_EXPRESSION_H__
#define __JAVA_SYNTAX_VALUE_EXPRESSION_H__
#include "JavaSyntaxExpressionCommon.h"


class CJavaSyntaxValueExpression : public CJavaSyntaxExpressionCommon
{
CONSTRUCTABLE(CJavaSyntaxValueExpression);
protected:
	CExpressionArray	mapcExpressions;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;
	void	PrettyPrint(CChars* pszDest, int iBlockDepth = 0) override;

	BOOL	IsValueExpression(void) override;
	BOOL	IsCompoundStatement(void) override;

	void	AddExpression(CJavaSyntaxExpressionCommon* pcExpression);
};


#endif // !__JAVA_SYNTAX_VALUE_EXPRESSION_H__

