#ifndef __PP_SPECIAL_OPERATOR_H__
#define __PP_SPECIAL_OPERATOR_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ExternalString.h"


enum EPreprocessorSpecialOperator
{
	PPSO_Defined,
	PPSO_HasAttribute,
	PPSO_HasCppAttribute,
	PPSO_HasBuiltIn,
	PPSO_HasInclude
};


class CSpecialOperator
{
public:
	CChars							mszName;
	EPreprocessorSpecialOperator	meType;
	int64							mlliID;

	void 							Init(char* pcName, EPreprocessorSpecialOperator eType, int64 lliID);
	void 							Kill(void);
	bool 							Equals(CSpecialOperator* pcOther);
	char*							GetName(void);
	EPreprocessorSpecialOperator	GetType(void);
	void							Dump(void);
};


#endif // !__PP_SPECIAL_OPERATOR_H__

