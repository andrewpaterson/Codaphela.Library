#ifndef __JAVA_LITERAL_H__
#define __JAVA_LITERAL_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


enum EJavaLiteralType
{
	JLT_Character,
	JLT_Boolean,
	JLT_String,
	JLT_Integer,
	JLT_Float,
	JLT_Null
};


class CJavaLiteral : public CJavaToken
{
CONSTRUCTABLE(CJavaLiteral);
protected:
	EJavaLiteralType	meLiteralType;

public:
			void 	Init(EJavaLiteralType eType);
			void 	Kill(void);

			char*	GetType(void) override;
			BOOL	IsLiteralType(EJavaLiteralType eType);

			BOOL	IsLiteral(void) override;
	virtual BOOL	IsCharacter(void);
	virtual BOOL	IsBoolean(void);
	virtual BOOL	IsString(void);
	virtual BOOL	IsInteger(void);
	virtual BOOL	IsFloat(void);
	virtual BOOL	IsNull(void);
};


#endif // !__JAVA_LITERAL_H__

