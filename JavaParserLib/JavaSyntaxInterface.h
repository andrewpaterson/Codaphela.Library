#ifndef __JAVA_SYNTAX_INTERFACE_H__
#define __JAVA_SYNTAX_INTERFACE_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntaxType.h"
#include "JavaSyntaxTopLevel.h"


class CJavaSyntaxInterface : public CJavaSyntaxTopLevel
{
CONSTRUCTABLE(CJavaSyntaxInterface);
protected:
	CJavaSyntaxType*	mpcType;
	BOOL				mbAbstract;
	BOOL				mbFinal;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsInterface(void) override;

	void	SetSyntaxType(CJavaSyntaxType* pcType);
	void	SetAbstract(BOOL bAbstract);
	void	SetFinal(BOOL bFinal);
};


typedef CArrayTemplatePtr<CJavaSyntaxInterface>	CJavaSyntaxInterfacePtrArray;


#endif // !__JAVA_SYNTAX_INTERFACE_H__

