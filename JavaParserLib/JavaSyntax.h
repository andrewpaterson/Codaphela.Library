#ifndef __JAVA_SYNTAX_H__
#define __JAVA_SYNTAX_H__
#include "BaseLib/Chars.h"


class CJavaSyntaxTree;
class CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntax);
protected:
	CJavaSyntaxTree*	mpcTree;

public:
			void		Init(CJavaSyntaxTree* pcTree);
	virtual void		Kill(void);

	virtual char*		GetType(void) =0;

	virtual BOOL		IsFile(void);
	virtual BOOL		IsImport(void);
	virtual BOOL		IsPackage(void);
	virtual BOOL		IsTopLevel(void);
	virtual BOOL		IsEnum(void);
	virtual BOOL		IsInterface(void);
	virtual BOOL		IsClass(void);
	virtual BOOL		IsClassGeneric(void);
	virtual BOOL		IsType(void);
	virtual BOOL		IsTypeGeneric(void);

	virtual BOOL		IsError(void);
	virtual BOOL		IsMismatch(void);
};


#endif // !__JAVA_SYNTAX_H__

