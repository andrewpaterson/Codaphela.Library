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


class CGerberCommandApertureMacro : public CGerberCommand
{
protected:
	CChars			szName;
	CListVariable	macPrimitives;

public:
	void								Init(size iNameLength);
	void								Kill(void);

	char*								NameText(void);
	CGerberApertureMacroComment*		AddComment(size uiLength);
	CGerberApertureMacroCircle*			AddCircle(void);
	CGerberApertureMacroCenterLine*		AddCenterLine(void);
	CGerberApertureMacroOutline*		AddOutline(void);
	CGerberApertureMacroPolygon*		AddPolygon(void);
	CGerberApertureMacroVectorLine*		AddVectorLine(void);
};


#endif // __GERBER_COMMAND_APERTURE_MACRO_H__

