#ifndef __JAVA_SYNTAX_H__
#define __JAVA_SYNTAX_H__
#include "BaseLib/Chars.h"
#include "JavaTokenIdentifier.h"


class CJavaSyntaxTree;
class CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntax);
protected:
	CJavaSyntaxTree*	mpcTree;
	CJavaSyntax*		mpcParent;

public:
	virtual void		Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	virtual void		Kill(void);
			void		Clear(void);

	virtual char*		GetType(void) =0;
	virtual void		TypePrint(CChars* pszDest, int iDepth);
	virtual void		PrettyPrint(CChars* pszDest, int iBlockDepth) =0;
			void		Dump(void);

	virtual BOOL		IsFile(void);
	virtual BOOL		IsImport(void);
	virtual BOOL		IsPackage(void);
	virtual BOOL		IsEnum(void);
	virtual BOOL		IsInterface(void);
	virtual BOOL		IsClass(void);
	virtual BOOL		IsType(void);
	virtual BOOL		IsGeneric(void);
	virtual BOOL		IsExtent(void);

	virtual BOOL		IsClassBlock(void);
	virtual BOOL		IsOriginalFor(void);
	virtual BOOL		IsIterativeFor(void);
	virtual BOOL		IsVariableDeclaration(void);
	virtual BOOL		IsVariableInitialiser(void);
	virtual BOOL		IsVoidExpression(void);
	virtual BOOL		IsValueExpression(void);
	virtual BOOL		IsArrayValueExpression(void);
	virtual BOOL		IsMethodCall(void);
	virtual BOOL		IsParentheses(void);
	virtual BOOL		IsOperator(void);

	virtual BOOL		IsStatement(void);
	virtual BOOL		IsClassCommon(void);
	virtual BOOL		IsTypeCommon(void);
	virtual BOOL		IsVariableCommon(void);
	virtual BOOL		IsExpressionCommon(void);
	virtual BOOL		IsLiteral(void);

	virtual BOOL		IsError(void);
	virtual BOOL		IsMismatch(void);

protected:
	void	PrintTokenArray(CChars* pszDest, CIdentifierArray* papcTokens);
};


#endif // !__JAVA_SYNTAX_H__

