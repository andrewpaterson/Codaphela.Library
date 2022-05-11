#ifndef __JAVA_SYNTAX_PACKAGE_H__
#define __JAVA_SYNTAX_PACKAGE_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntax.h"
#include "JavaIdentifier.h"


class CJavaSyntaxPackage : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxPackage);
protected:
	CJavaIdentifierPtrArray		mapcIdentifiers;

public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;
	void	AddIdentifier(CJavaIdentifier* pcIdentifier);

	BOOL	IsPackage(void) override;
};


typedef CArrayTemplatePtr<CJavaSyntaxPackage>	CJavaSyntaxPackagePtrArray;


#endif // !__JAVA_SYNTAX_PACKAGE_H__

