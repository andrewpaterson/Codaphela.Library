#ifndef __EXCELLON_COMMAND_ATTRIBUTE_H__
#define __EXCELLON_COMMAND_ATTRIBUTE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "ExcellonCommand.h"


//A Gerber X2 attribute written as a comment, for example: ; #@! TA.AperFunction,Plated,PTH,ViaDrill
class CExcellonCommandAttribute : public CExcellonCommand
{
protected:
	CChars		mszName;
	CArrayChars	maszValues;

public:
	void	Init(EExcellonCommandEnum eType, const char* szName, size uiLength);
	void	Kill(void);

	void	AddValue(const char* szValue, size uiLength);

	char*	NameText(void);
	size	NumValues(void);
	char*	ValueText(size uiIndex);
};


#endif // __EXCELLON_COMMAND_ATTRIBUTE_H__

