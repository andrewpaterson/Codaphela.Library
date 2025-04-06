#ifndef __GERBER_APERTURE_MACRO_OUTLINE_H__
#define __GERBER_APERTURE_MACRO_OUTLINE_H__
#include "BaseLib/Chars.h"
#include "GerberExpression.h"
#include "GerberApertureMacro.h"


class CGerberApertureMacroOutline : public CGerberApertureMacro
{
protected:

public:
	void Init(CCalculatorSymbols* pcSymbols);
	void Kill(void);
};


#endif // __GERBER_APERTURE_MACRO_OUTLINE_H__

