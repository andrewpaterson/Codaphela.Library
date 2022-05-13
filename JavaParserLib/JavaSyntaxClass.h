#ifndef __JAVA_SYNTAX_CLASS_H__
#define __JAVA_SYNTAX_CLASS_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntaxClassGeneric.h"
#include "JavaSyntaxTopLevel.h"


class CJavaSyntaxClass : public CJavaSyntaxTopLevel
{
CONSTRUCTABLE(CJavaSyntaxClass);
protected:
	CJavaSyntaxClassGenericPtrArray		mapcGenerics;

public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsClass(void) override;

	void	AddGeneric(CJavaSyntaxClassGeneric* pcGeneric);
};


#endif // !__JAVA_SYNTAX_CLASS_H__

