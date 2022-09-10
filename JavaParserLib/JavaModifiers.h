#ifndef __JAVA_MODIFIER_H__
#define __JAVA_MODIFIER_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"


enum EJavaModifier
{
	JM_none =		0x00,
	JM_public =		0x01,
	JM_protected =	0x02,
	JM_private =	0x04,
	JM_static =		0x08,
	JM_abstract =	0x10,
	JM_final =		0x20,
	JM_strictfp =	0x40,
};


class CJavaModifiers
{
protected:
	uint8	muiModifiers;

public:
	void Init(void);
	void Kill(void);
	void ReInit(void);

	void Set(EJavaModifier eModifier);

	bool IsNone(void);
	bool IsPublic(void);
	bool IsProtected(void);
	bool IsPrivate(void);
	bool IsStatic(void);
	bool IsAbstract(void);
	bool IsFinal(void);
	bool IsStrictfp(void);

	bool IsPackageModifier(void);
	bool IsValid(void);

	void Print(CChars* pszDest);
};


#endif // !__JAVA_MODIFIER_H__

