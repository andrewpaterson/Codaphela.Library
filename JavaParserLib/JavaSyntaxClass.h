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

public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsClass(void) override;

	void	SetSyntaxType(CJavaSyntaxType* pcType);
	void	SetAbstract(BOOL bAbstract);
	void	SetFinal(BOOL bFinal);
};


#endif // !__JAVA_SYNTAX_CLASS_H__

