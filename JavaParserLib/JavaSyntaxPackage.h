#ifndef __JAVA_SYNTAX_PACKAGE_H__
#define __JAVA_SYNTAX_PACKAGE_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntax.h"
#include "JavaTokenIdentifier.h"


class CJavaSyntaxPackage : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxPackage);
protected:
	CJavaTokenIdentifierPtrArray		mapcIdentifiers;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	void	AddIdentifier(CJavaTokenIdentifier* pcIdentifier);

	BOOL	IsPackage(void) override;
};


typedef CArrayTemplatePtr<CJavaSyntaxPackage>	CJavaSyntaxPackagePtrArray;


#endif // !__JAVA_SYNTAX_PACKAGE_H__

