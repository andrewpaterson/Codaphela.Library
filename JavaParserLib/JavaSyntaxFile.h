#ifndef __JAVA_SYNTAX_FILE_H__
#define __JAVA_SYNTAX_FILE_H__
#include "BaseLib/Chars.h"
#include "JavaSyntax.h"
#include "JavaSyntaxImport.h"
#include "JavaSyntaxPackage.h"
#include "JavaSyntaxTopLevel.h"


class CJavaSyntaxFile : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxFile);
protected:
	CJavaSyntaxPackage*				mpcPackage;
	CJavaSyntaxImportPtrArray		mapcImports;

	CJavaSyntaxTopLevel*			mpcPublicClass;			// The only public class, interface or enum in the file.
	CJavaSyntaxTopLevelPtrArray		mapcPackageClasses;		// All other package classes, interfaces and enums in the file.

public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsFile(void) override;

	BOOL	SetPackage(CJavaSyntaxPackage* pcPackage);
	void	AddImport(CJavaSyntaxImport* pcImport);

	BOOL	SetPublicClass(CJavaSyntaxTopLevel* pcTopLevel);
	void	AddPackageClass(CJavaSyntaxTopLevel* pcTopLevel);
};


#endif // !__JAVA_SYNTAX_FILE_H__

