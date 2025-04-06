#ifndef __GERBER_COMMAND_APERTURE_MACRO_H__
#define __GERBER_COMMAND_APERTURE_MACRO_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ListVariable.h"
#include "GerberCommand.h"
#include "GerberApertureMacro.h"
#include "GerberApertureMacroComment.h"
#include "GerberApertureMacroCircle.h"
#include "GerberApertureMacroCenterLine.h"
#include "GerberApertureMacroOutline.h"
#include "GerberApertureMacroPolygon.h"
#include "GerberApertureMacroVectorLine.h"
#include "GerberApertureMacroThermal.h"


class CGerberCommandApertureMacro : public CGerberCommand
{
protected:
	CChars			szName;
	CListVariable	macPrimitives;

public:
	void								Init(size iNameLength);
	void								Kill(void);

	bool								IsApertureMacro(void);

	char*								NameText(void);
	CGerberApertureMacroComment*		AddComment(size uiLength);
	CGerberApertureMacroCircle*			AddCircle(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace);
	CGerberApertureMacroCenterLine*		AddCenterLine(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace);
	CGerberApertureMacroOutline*		AddOutline(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace);
	CGerberApertureMacroPolygon*		AddPolygon(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace);
	CGerberApertureMacroVectorLine*		AddVectorLine(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace);
	CGerberApertureMacroThermal*		AddThermal(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace);

	size								NumPrimitives(void);
	CGerberApertureMacro*				GetPrimitive(size uiIndex);
};


#endif // __GERBER_COMMAND_APERTURE_MACRO_H__

