#ifndef __JAVA_SYNTAX_OPERATOR_H__
#define __JAVA_SYNTAX_OPERATOR_H__
#include "JavaTokenOperator.h"
#include "JavaSyntaxExpressionCommon.h"


class CJavaSyntaxOperator : public CJavaSyntaxExpressionCommon
{
CONSTRUCTABLE(CJavaSyntaxOperator);
protected:
	CJavaTokenOperator*		mpcTokenOperator;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;
	void	PrettyPrint(CChars* pszDest, int iBlockDepth = 0) override;

	BOOL	IsOperator(void) override;
	BOOL	IsCompoundStatement(void) override;

	void	SetOperator(CJavaTokenOperator* pcTokenOperator);
};


#endif // !__JAVA_SYNTAX_OPERATOR_H__

