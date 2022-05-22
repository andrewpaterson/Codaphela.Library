#ifndef __JAVA_SYNTAX_CLASS_H__
#define __JAVA_SYNTAX_CLASS_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntaxType.h"
#include "JavaSyntaxTopLevel.h"


class CJavaSyntaxClass : public CJavaSyntaxTopLevel
{
CONSTRUCTABLE(CJavaSyntaxClass);
protected:
	CJavaSyntaxType*	mpcType;
	BOOL				mbAbstract;
	BOOL				mbFinal;
	BOOL				mbProtected;
	BOOL				mbPrivate;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsClass(void) override;

	void	SetSyntaxType(CJavaSyntaxType* pcType);
	void	SetAbstract(BOOL bAbstract);
	void	SetFinal(BOOL bFinal);

	BOOL	IsPackageModifier(void);
	BOOL	IsAbstract(void);
	BOOL	IsFinal(void);
	BOOL	IsProtected(void);
	BOOL	IsPrivate(void);
};


#endif // !__JAVA_SYNTAX_CLASS_H__

