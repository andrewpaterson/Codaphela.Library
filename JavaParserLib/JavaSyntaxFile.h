#ifndef __JAVA_SYNTAX_FILE_H__
#define __JAVA_SYNTAX_FILE_H__
#include "BaseLib/Chars.h"
#include "JavaSyntax.h"
#include "JavaSyntaxImport.h"
#include "JavaSyntaxPackage.h"
#include "JavaSyntaxClassCommon.h"


class CJavaSyntaxFile : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxFile);
protected:
	CChars					mszFilename;

	CJavaSyntaxPackage*		mpcPackage;
	CImportArray			mapcImports;

	CClassCommonArray		mapcClasses;

public:
	void 					Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 					Kill(void);

	char*					GetType(void) override;
	void					TypePrint(CChars* pszDest, int iDepth) override;
	void					PrettyPrint(CChars* pszDest, int iBlockDepth) override;

	bool					IsFile(void) override;

	void					SetFileName(char* szFileName);

	bool					SetPackage(CJavaSyntaxPackage* pcPackage);
	void					AddImport(CJavaSyntaxImport* pcImport);
	void					AddClass(CJavaSyntaxClassCommon* pcClassCommon);

	CJavaSyntaxPackage*		GetPackage(void);
	CImportArray*			GetImports(void);
	CClassCommonArray*		GetClasses(void);
};


#endif // !__JAVA_SYNTAX_FILE_H__

