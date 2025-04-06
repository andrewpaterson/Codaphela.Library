#include "GerberCommandApertureMacro.h"


#define ADD_PRIMITIVE(type) 	type*	pcPrimitive;\
pcPrimitive = (type*)macPrimitives.Add(sizeof(type)); \
New<type>(pcPrimitive);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommandApertureMacro::Init(size iNameLength)
{
	CGerberCommand::Init(GC_AM);
	szName.InitLength(iNameLength);
	macPrimitives.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommandApertureMacro::Kill(void)
{
	size					i;
	size					uiNumElements;
	CGerberApertureMacro*	pcCommand;


	uiNumElements = macPrimitives.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcCommand = (CGerberApertureMacro*)macPrimitives.Get(i);
		pcCommand->Kill();
	}
	macPrimitives.Kill();
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CGerberCommandApertureMacro::NameText(void)
{
	return szName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberApertureMacroComment* CGerberCommandApertureMacro::AddComment(size uiLength)
{
	ADD_PRIMITIVE(CGerberApertureMacroComment);
	pcPrimitive->Init(uiLength);
	return pcPrimitive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberApertureMacroCircle* CGerberCommandApertureMacro::AddCircle(CCalculatorSymbols* pcSymbols)
{
	ADD_PRIMITIVE(CGerberApertureMacroCircle);
	pcPrimitive->Init(pcSymbols);
	return pcPrimitive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberApertureMacroCenterLine* CGerberCommandApertureMacro::AddCenterLine(CCalculatorSymbols* pcSymbols)
{
	ADD_PRIMITIVE(CGerberApertureMacroCenterLine);
	pcPrimitive->Init(pcSymbols);
	return pcPrimitive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberApertureMacroOutline* CGerberCommandApertureMacro::AddOutline(CCalculatorSymbols* pcSymbols)
{
	ADD_PRIMITIVE(CGerberApertureMacroOutline);
	pcPrimitive->Init(pcSymbols);
	return pcPrimitive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberApertureMacroPolygon* CGerberCommandApertureMacro::AddPolygon(CCalculatorSymbols* pcSymbols)
{
	ADD_PRIMITIVE(CGerberApertureMacroPolygon);
	pcPrimitive->Init(pcSymbols);
	return pcPrimitive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberApertureMacroVectorLine* CGerberCommandApertureMacro::AddVectorLine(CCalculatorSymbols* pcSymbols)
{
	ADD_PRIMITIVE(CGerberApertureMacroVectorLine);
	pcPrimitive->Init(pcSymbols);
	return pcPrimitive;
}

