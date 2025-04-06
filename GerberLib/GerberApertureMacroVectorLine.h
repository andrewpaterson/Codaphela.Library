#ifndef __GERBER_APERTURE_MACRO_VECTOR_LINE_H__
#define __GERBER_APERTURE_MACRO_VECTOR_LINE_H__
#include "BaseLib/Chars.h"
#include "GerberExpression.h"
#include "GerberApertureMacro.h"


class CGerberApertureMacroVectorLine : public CGerberApertureMacro
{
protected:

public:
	void Init(CCalculatorSymbols* pcSymbols);
	void Kill(void);
};


#endif // __GERBER_APERTURE_MACRO_VECTOR_LINE_H__

