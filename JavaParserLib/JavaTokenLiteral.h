#ifndef __JAVA_LITERAL_H__
#define __JAVA_LITERAL_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


enum EJavaTokenLiteralType
{
	JLT_Character,
	JLT_Boolean,
	JLT_String,
	JLT_Integer,
	JLT_Float,
	JLT_Null
};


class CJavaTokenLiteral : public CJavaToken
{
CONSTRUCTABLE(CJavaTokenLiteral);
protected:
	EJavaTokenLiteralType	meLiteralType;

public:
			void 	Init(EJavaTokenLiteralType eType);
			void 	Kill(void);

			char*	GetType(void) override;
			BOOL	IsLiteralType(EJavaTokenLiteralType eType);

			BOOL	IsLiteral(void) override;
	virtual BOOL	IsCharacter(void);
	virtual BOOL	IsBoolean(void);
	virtual BOOL	IsString(void);
	virtual BOOL	IsInteger(void);
	virtual BOOL	IsFloat(void);
	virtual BOOL	IsNull(void);
};


#endif // !__JAVA_LITERAL_H__

