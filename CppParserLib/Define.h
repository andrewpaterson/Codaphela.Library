#ifndef __PP_DEFINE_H__
#define __PP_DEFINE_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ExternalString.h"
#include "PPLine.h"


#define DEFINE_FLAGS_BRACKETED	0x01
#define DEFINE_FLAGS_SPECIAL	0x02
#define DEFINE_FLAGS_IN_MAP		0x04


class CDefineMap;
class CDefine
{
public:
	CPPLine			mcReplacement;
	CArrayChars		mcArguments;	//This is unnecessary but useful for reference.
	int64			mlliID;			//If a token is undef'd and redef'd it's muiID will change.
	CDefineMap*		mpcDefineMap;
	int				miFlags;
	CChars			mszName;

	void 	Init(CExternalString* pcName, int64 uiID, CDefineMap* pcDefineMap);
	void 	Kill(void);
	void 	AddArgument(CExternalString* pcName);
	void 	AddReplacmentToken(CPPToken* pcToken);
	BOOL 	Equals(CDefine* pcOther);
	char*	GetName(void);
	int64	GetID(void);
	BOOL	IsBacketed(void);
	BOOL	IsSpecial(void);
	BOOL	IsInMap(void);
	void	SetSpecial(BOOL b);
	void	SetBracketed(BOOL b);
	void	SetDefineMap(CDefineMap* pcDefineMap);
	void	Dump(void);
};


typedef CArrayTemplate<CDefine>	CArrayDefines;


#endif // !__PP_DEFINE_H__
