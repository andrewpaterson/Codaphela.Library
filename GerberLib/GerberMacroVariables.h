#ifndef __GERBER_MACRO_VARIABLES_H__
#define __GERBER_MACRO_VARIABLES_H__
#include "BaseLib/Number.h"
#include "BaseLib/ArrayTemplate.h"


struct SGerberMacroVariable
{
	bool		bDefined;
	CNumber		cValue;
};


typedef CArrayTemplate<SGerberMacroVariable>	CArrayGerberMacroVariables;


//The values of $1, $2 ... while an aperture macro is evaluated.  Undefined variables are 0 and
//variables cannot be redefined.  4.5.4.3
class CGerberMacroVariables
{
protected:
	CArrayGerberMacroVariables	masVariables;  //Variable $n is at index n - 1.

public:
	void	Init(void);
	void	Kill(void);

	bool	Set(uint32 uiVariable, CNumber* pcValue);
	void	Get(uint32 uiVariable, CNumber* pcValue);
	bool	IsDefined(uint32 uiVariable);
};


#endif // __GERBER_MACRO_VARIABLES_H__

