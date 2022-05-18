#ifndef __JAVA_FLOAT_H__
#define __JAVA_FLOAT_H__
#include "BaseLib/Chars.h"
#include "BaseLib/PrimitiveTypes.h"
#include "JavaTokenLiteral.h"


enum EJavaTokenFloatType
{
	JFT_float32,
	JFT_float64,
	 
	JFT_Unknown = -1
};


class CJavaTokenFloat : public CJavaTokenLiteral
{
CONSTRUCTABLE(CJavaTokenFloat);
protected:
	float64					mfValue;
	EJavaTokenFloatType		meType;

public:
	void 	Init(STextPosition* psPosition, float32 fValue);
	void 	Init(STextPosition* psPosition, float64 fValue);
	void 	Kill(void);

	char*	GetType(void);
	BOOL	Is(float32 fValue);
	BOOL	Is(float64 fValue);

	void	Print(CChars* pszDest);
	BOOL	IsFloat(void);
};


#endif // !__JAVA_FLOAT_H__

