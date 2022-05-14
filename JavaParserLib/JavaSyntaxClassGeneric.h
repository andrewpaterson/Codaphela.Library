#ifndef __JAVA_SYNTAX_CLASS_GENERIC_H__
#define __JAVA_SYNTAX_CLASS_GENERIC_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntaxType.h"
#include "JavaSyntax.h"
#include "JavaIdentifier.h"


class CJavaSyntaxClassGeneric : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxClassGeneric);
protected:
	CJavaIdentifier*	mpcName;
	CJavaSyntaxType*	mpcExtends;  //NULL if does not extend a Type.

public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsClassGeneric(void) override;

	void	SetName(CJavaIdentifier* pcName);
	void	SetExtends(CJavaSyntaxType* pcExtends);
};


typedef CArrayTemplatePtr<CJavaSyntaxClassGeneric>	CJavaSyntaxClassGenericPtrArray;


#endif // !__JAVA_SYNTAX_CLASS_GENERIC_H__

