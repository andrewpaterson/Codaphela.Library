#ifndef __GERBER_APERTURE_MACRO_POLYGON_H__
#define __GERBER_APERTURE_MACRO_POLYGON_H__
#include "BaseLib/Chars.h"
#include "GerberExpression.h"
#include "GerberApertureMacro.h"


class CGerberApertureMacroPolygon : public CGerberApertureMacro
{
protected:

public:
	void Init(CCalculatorSymbols* pcSymbols);
	void Kill(void);
};


#endif // __GERBER_APERTURE_MACRO_POLYGON_H__

