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
			void 	Init(STextPosition* psPosition, EJavaTokenLiteralType eType);
			void 	Kill(void);

			char*	GetType(void) override;
			bool	IsLiteralType(EJavaTokenLiteralType eType);

			bool	IsLiteral(void) override;
	virtual bool	IsCharacter(void);
	virtual bool	IsBoolean(void);
	virtual bool	IsString(void);
	virtual bool	IsInteger(void);
	virtual bool	IsFloat(void);
	virtual bool	IsNull(void);
};


#endif // !__JAVA_LITERAL_H__

