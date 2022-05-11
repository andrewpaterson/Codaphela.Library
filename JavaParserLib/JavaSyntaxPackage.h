#ifndef __JAVA_SYNTAX_PACKAGE_H__
#define __JAVA_SYNTAX_PACKAGE_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntax.h"


class CJavaSyntaxPackage : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxPackage);
protected:
public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsPackage(void) override;
};


typedef CArrayTemplatePtr<CJavaSyntaxPackage>	CJavaSyntaxPackagePtrArray;


#endif // !__JAVA_SYNTAX_PACKAGE_H__

