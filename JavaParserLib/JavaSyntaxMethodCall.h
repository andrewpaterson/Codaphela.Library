#ifndef __JAVA_SYNTAX_METHOD_CALL_H__
#define __JAVA_SYNTAX_METHOD_CALL_H__
#include "JavaSyntaxExpressionCommon.h"


class CJavaSyntaxMethodCall : public CJavaSyntaxExpressionCommon
{
CONSTRUCTABLE(CJavaSyntaxMethodCall);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;
	void	PrettyPrint(CChars* pszDest, int iBlockDepth = 0) override;

	bool	IsMethodCall(void) override;
	bool	IsCompoundStatement(void) override;
};


#endif // !__JAVA_SYNTAX_METHOD_CALL_H__

