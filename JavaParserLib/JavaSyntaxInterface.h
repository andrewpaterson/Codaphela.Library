#ifndef __JAVA_SYNTAX_INTERFACE_H__
#define __JAVA_SYNTAX_INTERFACE_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntaxTopLevel.h"


class CJavaSyntaxInterface : public CJavaSyntaxTopLevel
{
CONSTRUCTABLE(CJavaSyntaxInterface);
protected:
public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsInterface(void) override;
};


typedef CArrayTemplatePtr<CJavaSyntaxInterface>	CJavaSyntaxInterfacePtrArray;


#endif // !__JAVA_SYNTAX_INTERFACE_H__

