/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela GerberLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "BaseLib/CalculatorParser.h"
#include "GerberString.h"
#include "GerberWhitespace.h"
#include "GerberIdentifier.h"
#include "GerberExpression.h"
#include "GerberParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberParser::Init(char* szText, lsize iTextLen, char* szFileName, CGerberCommands* pcCommands)
{
	STextParserConfig	sConfig;
	bool				bResult;

	sConfig.Init(SkipGerberWhitespace, ParseGerberStringUnset, ParseGerberExactIdentifierUnset, ParseGerberMacroVariable);

	//The text must not contain carriage returns (see PassifyNewlines).
	bResult = mcParser.Init(szText, iTextLen, &sConfig);
	mszFilename.Init(szFileName);

	mpcCommands = pcCommands;
	mbSkipWhitespace = true;

	mbFormatSpecified = false;
	muiXWholes = 0;
	muiXDecimals = 0;
	muiYWholes = 0;
	muiYDecimals = 0;

	mbInRegion = false;

	InitGerberCalculatorSymbols(&mcSymbols);

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InitGerberCalculatorSymbols(CCalculatorSymbols* pcSymbols)
{
	pcSymbols->Init(true);
	pcSymbols->SetOperator("+", CO_Add, 4);
	pcSymbols->SetOperator("-", CO_Subtract, 4);
	pcSymbols->SetOperator("x", CO_Multiply, 3);
	pcSymbols->SetOperator("/", CO_Divide, 2);
	pcSymbols->SetOperator("=", CO_Assignment, 5);
	pcSymbols->SetOperator("+", CO_UnaryAdd, 1);
	pcSymbols->SetOperator("-", CO_UnarySubtract, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberParser::Kill(void)
{
	mcSymbols.Kill();

	mpcCommands = NULL;

	mszFilename.Kill();
	mcParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::Error(char* szError)
{
	CChars			sz;
	SLogConfig		sLogConfig;

	mcParser.SetErrorSyntaxError();
	sz.Init(mszFilename);
	sz.Append('(');
	sz.Append(mcParser.Line());
	sz.Append(',');
	sz.Append(mcParser.Column());
	sz.Append("): ");
	sz.Append(szError);
	sz.AppendNewLine();
	mcParser.GetLineLocationMarkerString(&sz);
	gcLogger.GetConfig(&sLogConfig);
	gcLogger.SetBreakOnError(false);
	gcLogger.Error2(__METHOD__, " ", sz.Text(), NULL);
	gcLogger.SetConfig(&sLogConfig);
	sz.Kill();
	return TRIERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::GetCommentString(char* szString, size* piLength)
{
	TRISTATE	tResult;

	mcParser.SetStringParser(ParseGerberCommentString);
	tResult = mcParser.GetString(szString, piLength, mbSkipWhitespace);
	mcParser.SetStringParser(ParseGerberStringUnset);
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::GetFieldString(char* szString, size* piLength)
{
	TRISTATE	tResult;

	mcParser.SetStringParser(ParseGerberFieldString);
	tResult = mcParser.GetString(szString, piLength, mbSkipWhitespace);
	mcParser.SetStringParser(ParseGerberStringUnset);
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::GetNameString(char* szString, size* piLength)
{
	TRISTATE	tResult;

	mcParser.SetIdentifierParser(ParseGerberIdentifier);
	tResult = mcParser.GetIdentifier(szString, piLength, mbSkipWhitespace);
	mcParser.SetIdentifierParser(ParseGerberMacroVariable);
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::GetStandardNameString(char* szString, size* piLength)
{
	TRISTATE	tResult;

	mcParser.SetIdentifierParser(ParseGerberIdentifier);
	tResult = mcParser.GetIdentifier(szString, piLength, mbSkipWhitespace);
	mcParser.SetIdentifierParser(ParseGerberMacroVariable);
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::GetUserNameString(char* szString, size* piLength)
{
	TRISTATE	tResult;

	mcParser.SetIdentifierParser(ParseGerberIdentifier);
	tResult = mcParser.GetIdentifier(szString, piLength, mbSkipWhitespace);
	mcParser.SetIdentifierParser(ParseGerberMacroVariable);
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandG04()
{
	TRISTATE				tResult;
	size					iLength;
	CGerberCommandComment*	pcComment;
	
	tResult = mcParser.GetExactCharacterSequence("G04", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mcParser.PushPosition();
	tResult = GetCommentString(NULL, &iLength);
	mcParser.PopPosition();
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcComment = mpcCommands->AddComment(iLength);
	GetCommentString(pcComment->Text(), &iLength);
	
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandMO()
{
	TRISTATE				tResult;
	EGerberMeasurementMode	eMode;

	tResult = mcParser.GetExactCharacterSequence("%MO", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mcParser.PushPosition();
	tResult = mcParser.GetExactCharacterSequence("MM", mbSkipWhitespace);
	if (tResult == TRIERROR)
	{
		mcParser.PopPosition();
		return TRIERROR;
	}
	
	eMode = GMM_Unknown;
	if (tResult == TRITRUE)
	{
		eMode = GMM_Millimeters;
		mcParser.PassPosition();
	}
	else
	{
		tResult = mcParser.GetExactCharacterSequence("IN", mbSkipWhitespace);
		if (tResult == TRIERROR)
		{
			mcParser.PopPosition();
			return TRIERROR;
		}
		else if (tResult == TRITRUE)
		{
			eMode = GMM_Inches;
			mcParser.PassPosition();
		}
		else if (tResult == TRIFALSE)
		{
			mcParser.PassPosition();
			ReturnSyntanxError();
		}
		else
		{
			mcParser.PopPosition();
			ReturnSyntanxError();
		}
	}
	
	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetExactCharacter('%', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	mpcCommands->AddModeSet(eMode);
	
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandFS()
{
	TRISTATE	tResult;
	uint16		uiXWholes;
	uint16		uiXDecimals;
	uint16		uiYWholes;
	uint16		uiYDecimals;

	tResult = mcParser.GetExactCharacterSequence("%FS", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	tResult = mcParser.GetExactCharacterSequence("LA", mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetExactCharacter('X', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetDigit(&uiXWholes);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetDigit(&uiXDecimals);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetExactCharacter('Y', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetDigit(&uiYWholes);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetDigit(&uiYDecimals);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetExactCharacter('%', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	mpcCommands->AddFormatSpecifier(uiXWholes, uiXDecimals, uiYWholes, uiYDecimals);

	mbFormatSpecified = true;
	muiXWholes = uiXWholes;
	muiXDecimals = uiXDecimals;
	muiYWholes = uiYWholes;
	muiYDecimals = uiYDecimals;

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandAD()
{
	TRISTATE						tResult;
	uint64							ulliDCode;
	size							uiLength;
	CNumber							cNumber;
	CGerberCommandApertureDefine*	pcApertureDefine;

	tResult = mcParser.GetExactCharacterSequence("%AD", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	tResult = mcParser.GetExactCharacter('D', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetDigits(&ulliDCode, NULL, NULL, false, false, 10, NUMBER_SEPARATOR_NONE);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	if ((ulliDCode < 10) || (ulliDCode > MAX_UINT32))
	{
		return Error("Aperture D code must be 10 or greater, could not parse Command:");
	}

	//The template is C, R, O, P or the name of an aperture macro.
	mcParser.PushPosition();
	tResult = GetNameString(NULL, &uiLength);
	mcParser.PopPosition();
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcApertureDefine = mpcCommands->AddApertureDefine((uint32)ulliDCode, uiLength);
	GetNameString(pcApertureDefine->TemplateText(), &uiLength);

	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnError(tResult);
	if (tResult == TRITRUE)
	{
		for (;;)
		{
			tResult = mcParser.GetNumber(&cNumber);
			ReturnSyntaxErrorOnErrorOrFalse(tResult);
			pcApertureDefine->AddParameter(&cNumber);

			tResult = mcParser.GetExactCharacter('X', mbSkipWhitespace);
			ReturnSyntaxErrorOnError(tResult);
			if (tResult == TRIFALSE)
			{
				break;
			}
		}
	}

	tResult = mcParser.GetExactCharacterSequence("*%", mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandAM()
{
	CGerberCommandApertureMacro*	pcApertureMacro;
	TRISTATE						tResult;
	size							uiLength;

	tResult = mcParser.GetExactCharacterSequence("%AM", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mcParser.PushPosition();
	tResult = GetNameString(NULL, &uiLength);
	mcParser.PopPosition();
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcApertureMacro = mpcCommands->AddApertureMacro(uiLength);
	GetNameString(pcApertureMacro->NameText(), &uiLength);

	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	for (;;)
	{
		tResult = ParseApertureMacroPrimitive(pcApertureMacro);
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseApertureMacroVariableDefinition(pcApertureMacro);
		ContinueOnTrueReturnOnError(tResult);

		tResult = mcParser.GetExactCharacter('%', mbSkipWhitespace);
		ReturnSyntaxErrorOnErrorOrFalse(tResult);

		return TRITRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseApertureMacroPrimitive(CGerberCommandApertureMacro* pcApertureMacro)
{
	uint64							ulli;
	TRISTATE						tResult;
	EGerberApertureMacroPrimitive	uiPrimitiveType;

	tResult = mcParser.GetDigits(&ulli, NULL, NULL, mbSkipWhitespace, false, 10, NUMBER_SEPARATOR_NONE);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	uiPrimitiveType = (EGerberApertureMacroPrimitive)ulli;

	if (uiPrimitiveType != GAMP_Comment)
	{
		tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
		ReturnSyntaxErrorOnErrorOrFalse(tResult);
	}
		
	if (uiPrimitiveType == GAMP_Comment)
	{
		tResult = ParseApertureMacroComment(pcApertureMacro);
	}
	else if (uiPrimitiveType == GAMP_Circle)
	{
		tResult = ParseApertureMacroCircle(pcApertureMacro);
	}
	else if (uiPrimitiveType == GAMP_Vector_Line)
	{
		tResult = ParseApertureMacroVectorLine(pcApertureMacro);
	}
	else if (uiPrimitiveType == GAMP_Center_Line)
	{
		tResult = ParseApertureMacroCenterLine(pcApertureMacro);
	}
	else if (uiPrimitiveType == GAMP_Outline)
	{
		tResult = ParseApertureMacroOutline(pcApertureMacro);
	}
	else if (uiPrimitiveType == GAMP_Polygon)
	{
		tResult = ParseApertureMacroPolygon(pcApertureMacro);
	}
	else if (uiPrimitiveType == GAMP_Thermal_Center)
	{
		tResult = ParseApertureMacroThermal(pcApertureMacro);
	}
	else
	{
		tResult = TRIERROR;
	}

	if ((tResult == TRIERROR) || (tResult == TRIFALSE))
	{
		return TRIERROR;
	}
	
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseApertureMacroComment(CGerberCommandApertureMacro* pcApertureMacro)
{
	TRISTATE						tResult;
	size							uiLength;
	CGerberApertureMacroComment*	pcComment;

	mcParser.PushPosition();
	tResult = GetCommentString(NULL, &uiLength);
	mcParser.PopPosition();
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcComment = pcApertureMacro->AddComment(uiLength);
	GetCommentString(pcComment->Text(), &uiLength);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseExpression(CGerberExpression* pcGerberExpression)
{
	
	CCalculatorParser		cParser;
	CCalcExpression*		pcExpression;
	CChars					sz;
	CCalculator*			pcCalculator;

	pcCalculator = pcGerberExpression->GetCalculator();

	cParser.Init(pcCalculator);
	pcExpression = (CCalcExpression*)cParser.Parse(&mcParser, false);
	cParser.Kill();

	if (pcCalculator->HasError())
	{
		sz.Init(pcCalculator->GetError());
		sz.Append(", could not parse Expression:"); 
		Error(sz.Text());
		sz.Kill();
		return TRIERROR;
	}
	
	if (pcExpression == NULL)
	{
		return TRIFALSE;
	}

	pcGerberExpression->SetCalcExpression(pcExpression);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseApertureMacroCircle(CGerberCommandApertureMacro* pcApertureMacro)
{
	TRISTATE						tResult;
	CGerberApertureMacroCircle*		pcCircle;

	pcCircle = pcApertureMacro->AddCircle(&mcSymbols, mbSkipWhitespace);
	
	tResult = ParseExpression(pcCircle->GetExposure());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcCircle->GetDiameter());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcCircle->GetCenterX());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcCircle->GetCenterY());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnError(tResult);
	if (tResult == TRITRUE)
	{
		tResult = ParseExpression(pcCircle->GetRotation());
		ReturnSyntaxErrorOnErrorOrFalse(tResult);
	}

	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseApertureMacroVectorLine(CGerberCommandApertureMacro* pcApertureMacro)
{
	TRISTATE							tResult;
	CGerberApertureMacroVectorLine*		pcVectorLine;

	pcVectorLine = pcApertureMacro->AddVectorLine(&mcSymbols, mbSkipWhitespace);
	
	tResult = ParseExpression(pcVectorLine->GetExposure());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcVectorLine->GetWidth());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcVectorLine->GetStartX());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcVectorLine->GetStartY());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcVectorLine->GetEndX());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcVectorLine->GetEndY());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcVectorLine->GetRotation());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseApertureMacroCenterLine(CGerberCommandApertureMacro* pcApertureMacro)
{
	TRISTATE							tResult;
	CGerberApertureMacroCenterLine*		pcCenterLine;

	pcCenterLine = pcApertureMacro->AddCenterLine(&mcSymbols, mbSkipWhitespace);

	tResult = ParseExpression(pcCenterLine->GetExposure());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcCenterLine->GetWidth());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcCenterLine->GetHeight());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcCenterLine->GetCenterX());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcCenterLine->GetCenterY());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcCenterLine->GetRotation());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseApertureMacroOutline(CGerberCommandApertureMacro* pcApertureMacro)
{
	TRISTATE						tResult;
	CGerberApertureMacroOutline*	pcOutline;
	uint64							ulliVerticies;
	size							uiSize;
	size							i;
	CGerberExpressionPosition*		pcVertex;

	pcOutline = pcApertureMacro->AddOutline(&mcSymbols, mbSkipWhitespace);

	tResult = ParseExpression(pcOutline->GetExposure());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetInteger(&ulliVerticies, NULL, NULL, mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	//The start point is followed by one point per vertex, the last of which closes the outline.
	uiSize = (size)ulliVerticies + 1;
	for (i = 0; i < uiSize; i++)
	{
		pcVertex = pcOutline->Add(&mcSymbols, mbSkipWhitespace, false);

		tResult = ParseExpression(pcVertex->GetX());
		ReturnSyntaxErrorOnErrorOrFalse(tResult);
		tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
		ReturnSyntaxErrorOnErrorOrFalse(tResult);

		tResult = ParseExpression(pcVertex->GetY());
		ReturnSyntaxErrorOnErrorOrFalse(tResult);
		tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
		ReturnSyntaxErrorOnErrorOrFalse(tResult);
	}
	
	tResult = ParseExpression(pcOutline->GetRotation());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseApertureMacroPolygon(CGerberCommandApertureMacro* pcApertureMacro)
{
	TRISTATE						tResult;
	CGerberApertureMacroPolygon*	pcPolygon;

	pcPolygon = pcApertureMacro->AddPolygon(&mcSymbols, mbSkipWhitespace);

	tResult = ParseExpression(pcPolygon->GetExposure());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcPolygon->GetNumberOfVertices());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcPolygon->GetCenterX());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcPolygon->GetCenterY());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcPolygon->GetDiameter());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcPolygon->GetRotation());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseApertureMacroThermal(CGerberCommandApertureMacro* pcApertureMacro)
{
	TRISTATE						tResult;
	CGerberApertureMacroThermal*	pcThermal;

	pcThermal = pcApertureMacro->AddThermal(&mcSymbols, mbSkipWhitespace);


	tResult = ParseExpression(pcThermal->GetCenterX());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcThermal->GetCenterY());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcThermal->GetOuterDiameter());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcThermal->GetInnerDiameter());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcThermal->GetGapThickness());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = ParseExpression(pcThermal->GetRotation());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseApertureMacroVariableDefinition(CGerberCommandApertureMacro* pcApertureMacro)
{
	TRISTATE									tResult;
	uint64										ulliVariable;
	CGerberApertureMacroVariableDefinition*		pcVariableDefinition;

	tResult = mcParser.GetExactCharacter('$', mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	tResult = mcParser.GetDigits(&ulliVariable, NULL, NULL, false, false, 10, NUMBER_SEPARATOR_NONE);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	if ((ulliVariable == 0) || (ulliVariable > MAX_UINT32))
	{
		return Error("Macro variable must be $1 or greater, could not parse Command:");
	}

	tResult = mcParser.GetExactCharacter('=', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcVariableDefinition = pcApertureMacro->AddVariableDefinition((uint32)ulliVariable, &mcSymbols, mbSkipWhitespace);
	tResult = ParseExpression(pcVariableDefinition->GetExpression());
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandDnn()
{
	TRISTATE	tResult;
	uint64		ulliDCode;

	mcParser.PushPosition();
	tResult = mcParser.GetExactCharacter('D', mbSkipWhitespace);
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		ReturnSyntaxErrorOnError(tResult);
		return TRIFALSE;
	}

	tResult = mcParser.GetDigits(&ulliDCode, NULL, NULL, false, false, 10, NUMBER_SEPARATOR_NONE);
	if (tResult != TRITRUE)
	{
		mcParser.PassPosition();
		ReturnSyntanxError();
	}

	//D01, D02 and D03 are operations, not apertures.
	if (ulliDCode < 10)
	{
		mcParser.PopPosition();
		return TRIFALSE;
	}
	mcParser.PassPosition();

	if (ulliDCode > MAX_UINT32)
	{
		ReturnSyntanxError();
	}

	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	mpcCommands->AddSetAperture((uint32)ulliDCode);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandG74()
{
	TRISTATE	tResult;

	tResult = mcParser.GetExactCharacterSequence("G74*", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mpcCommands->AddQuadrantMode(GQM_Single);

	return TRITRUE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandG75()
{
	TRISTATE	tResult;

	tResult = mcParser.GetExactCharacterSequence("G75*", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mpcCommands->AddQuadrantMode(GQM_Multi);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandG01()
{
	TRISTATE	tResult;

	tResult = mcParser.GetExactCharacterSequence("G01*", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mpcCommands->AddPlotMode(GPM_Linear);
	
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandG02()
{
	TRISTATE	tResult;

	tResult = mcParser.GetExactCharacterSequence("G02*", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mpcCommands->AddPlotMode(GPM_CircularClockwise);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandG03()
{
	TRISTATE	tResult;

	tResult = mcParser.GetExactCharacterSequence("G03*", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mpcCommands->AddPlotMode(GPM_CircularAnticlockwise);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCoordinate(char cAxis, uint16 uiWholes, uint16 uiDecimals, CNumber* pcCoordinate)
{
	TRISTATE	tResult;
	uint64		ulliValue;
	int16		iSign;
	uint16		uiNumDigits;
	uint64		ulliDivisor;
	uint16		i;
	CChars		sz;
	CChars		szDecimals;

	tResult = mcParser.GetExactCharacter(cAxis, mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	if (!mbFormatSpecified)
	{
		return Error("Coordinate before Format Specification (FS), could not parse Command:");
	}

	tResult = mcParser.GetDigits(&ulliValue, &iSign, &uiNumDigits, false, true, 10, NUMBER_SEPARATOR_NONE);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	if (uiNumDigits > uiWholes + uiDecimals)
	{
		return Error("Coordinate has more digits than the Format Specification (FS) allows, could not parse Command:");
	}

	//Leading zeros are omitted so the last uiDecimals digits are the decimal part.
	ulliDivisor = 1;
	for (i = 0; i < uiDecimals; i++)
	{
		ulliDivisor *= 10;
	}

	sz.Init();
	if (iSign < 0)
	{
		sz.Append('-');
	}
	sz.Append(ulliValue / ulliDivisor);
	if (uiDecimals > 0)
	{
		szDecimals.Init();
		szDecimals.Append(ulliValue % ulliDivisor);
		sz.Append('.');
		sz.Append('0', uiDecimals - szDecimals.Length());
		sz.Append(szDecimals);
		szDecimals.Kill();
	}
	pcCoordinate->Init(sz.Text(), sz.Length());
	sz.Kill();

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandOperation()
{
	TRISTATE					tResult;
	CNumber						cX;
	CNumber						cY;
	CNumber						cI;
	CNumber						cJ;
	bool						bX;
	bool						bY;
	bool						bI;
	bool						bJ;
	uint64						ulliCode;
	EGerberCommandEnum			eType;
	CGerberCommandOperation*	pcOperation;

	//I and J use the X and Y formats.
	tResult = ParseCoordinate('X', muiXWholes, muiXDecimals, &cX);
	ReturnOnError(tResult);
	bX = (tResult == TRITRUE);

	tResult = ParseCoordinate('Y', muiYWholes, muiYDecimals, &cY);
	ReturnOnError(tResult);
	bY = (tResult == TRITRUE);

	tResult = ParseCoordinate('I', muiXWholes, muiXDecimals, &cI);
	ReturnOnError(tResult);
	bI = (tResult == TRITRUE);

	tResult = ParseCoordinate('J', muiYWholes, muiYDecimals, &cJ);
	ReturnOnError(tResult);
	bJ = (tResult == TRITRUE);

	tResult = mcParser.GetExactCharacter('D', mbSkipWhitespace);
	ReturnSyntaxErrorOnError(tResult);
	if (tResult == TRIFALSE)
	{
		if (bX || bY || bI || bJ)
		{
			//Coordinates without an operation code (deprecated) are not supported.
			ReturnSyntanxError();
		}
		return TRIFALSE;
	}

	tResult = mcParser.GetDigits(&ulliCode, NULL, NULL, false, false, 10, NUMBER_SEPARATOR_NONE);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	if (ulliCode == 1)
	{
		eType = GC_D01;
	}
	else if (ulliCode == 2)
	{
		eType = GC_D02;
	}
	else if (ulliCode == 3)
	{
		eType = GC_D03;
	}
	else
	{
		ReturnSyntanxError();
	}

	if (mbInRegion && (eType == GC_D03))
	{
		return Error("D03 is not allowed in a region statement, could not parse Command:");
	}

	if ((eType != GC_D01) && (bI || bJ))
	{
		return Error("I and J offsets are only allowed on D01, could not parse Command:");
	}

	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcOperation = mpcCommands->AddOperation(eType);
	if (bX)
	{
		pcOperation->SetX(&cX);
	}
	if (bY)
	{
		pcOperation->SetY(&cY);
	}
	if (bI)
	{
		pcOperation->SetI(&cI);
	}
	if (bJ)
	{
		pcOperation->SetJ(&cJ);
	}

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandLP()
{
	TRISTATE			tResult;
	EGerberPolarity		ePolarity;

	tResult = mcParser.GetExactCharacterSequence("%LP", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mcParser.PushPosition();
	tResult = mcParser.GetExactCharacter('D', mbSkipWhitespace);
	if (tResult == TRIERROR)
	{
		mcParser.PopPosition();
		return TRIERROR;
	}

	ePolarity = GP_Unknown;
	if (tResult == TRITRUE)
	{
		ePolarity = GP_Dark;
		mcParser.PassPosition();
	}
	else
	{
		tResult = mcParser.GetExactCharacter('C', mbSkipWhitespace);
		if (tResult == TRIERROR)
		{
			mcParser.PopPosition();
			return TRIERROR;
		}
		else if (tResult == TRITRUE)
		{
			ePolarity = GP_Clear;
			mcParser.PassPosition();
		}
		else if (tResult == TRIFALSE)
		{
			mcParser.PassPosition();
			ReturnSyntanxError();
		}
		else
		{
			mcParser.PopPosition();
			ReturnSyntanxError();
		}
	}

	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetExactCharacter('%', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	mpcCommands->AddLoadPolarity(ePolarity);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandLM()
{
	TRISTATE			tResult;
	EGerberMirroring	eMirroring;

	tResult = mcParser.GetExactCharacterSequence("%LM", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	eMirroring = GLM_Unknown;

	tResult = mcParser.GetExactCharacterSequence("N*%", mbSkipWhitespace);
	ReturnSyntaxErrorOnError(tResult);
	if (tResult == TRITRUE)
	{
		eMirroring = GLM_None;
	}
	else
	{
		tResult = mcParser.GetExactCharacterSequence("X*%", mbSkipWhitespace);
		ReturnSyntaxErrorOnError(tResult);
		if (tResult == TRITRUE)
		{
			eMirroring = GLM_X;
		}
		else
		{
			tResult = mcParser.GetExactCharacterSequence("Y*%", mbSkipWhitespace);
			ReturnSyntaxErrorOnError(tResult);
			if (tResult == TRITRUE)
			{
				eMirroring = GLM_Y;
			}
			else
			{
				tResult = mcParser.GetExactCharacterSequence("XY*%", mbSkipWhitespace);
				ReturnSyntaxErrorOnError(tResult);
				if (tResult == TRITRUE)
				{
					eMirroring = GLM_XY;
				}
				else
				{
					ReturnSyntanxError();
				}
			}
		}
	}

	mpcCommands->AddLoadMirroring(eMirroring);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandLR()
{
	TRISTATE	tResult;
	CNumber		cNumber;

	tResult = mcParser.GetExactCharacterSequence("%LR", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	tResult = mcParser.GetNumber(&cNumber);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetExactCharacterSequence("*%", mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	mpcCommands->AddLoadRotation(&cNumber);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandLS()
{
	TRISTATE	tResult;
	CNumber		cNumber;

	tResult = mcParser.GetExactCharacterSequence("%LS", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	tResult = mcParser.GetNumber(&cNumber);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetExactCharacterSequence("*%", mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	mpcCommands->AddLoadScaling(&cNumber);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseRegionStatement()
{
	TRISTATE				tResult;
	CGerberCommandRegion*	pcRegion;
	CGerberCommands*		pcParentCommands;

	tResult = mcParser.GetExactCharacterSequence("G36*", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	pcRegion = mpcCommands->AddRegion();

	pcParentCommands = mpcCommands;
	mpcCommands = pcRegion->GetCommands();
	mbInRegion = true;

	tResult = ParseRegionContours();

	mbInRegion = false;
	mpcCommands = pcParentCommands;

	return tResult;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseRegionContours(void)
{
	TRISTATE			tResult;
	size				i;
	size				uiNumCommands;
	CGerberCommand*		pcCommand;

	for (;;)
	{
		mcParser.SkipWhitespace();
		if (mcParser.IsOutside())
		{
			return Error("Region statement (G36) not closed with G37, could not parse Command:");
		}

		tResult = mcParser.GetExactCharacterSequence("G37*", mbSkipWhitespace);
		ReturnSyntaxErrorOnError(tResult);
		if (tResult == TRITRUE)
		{
			break;
		}

		tResult = ParseCommandG04();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandG01();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandG02();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandG03();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandOperation();
		ContinueOnTrueReturnOnError(tResult);

		ReturnSyntanxError();
	}

	//Each contour starts with a D02 so the first operation must be a D02.
	uiNumCommands = mpcCommands->NumCommands();
	for (i = 0; i < uiNumCommands; i++)
	{
		pcCommand = mpcCommands->GetCommand(i);
		if (pcCommand->IsOperation())
		{
			if (!pcCommand->IsType(GC_D02))
			{
				return Error("Region contour must start with a D02, could not parse Command:");
			}
			return TRITRUE;
		}
	}
	return Error("Region statement (G36) must contain a contour, could not parse Command:");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseABStatement()
{
	TRISTATE						tResult;
	uint64							ulliDCode;
	CGerberCommandApertureBlock*	pcBlock;
	CGerberCommands*				pcParentCommands;

	tResult = mcParser.GetExactCharacterSequence("%ABD", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	tResult = mcParser.GetDigits(&ulliDCode, NULL, NULL, false, false, 10, NUMBER_SEPARATOR_NONE);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	if ((ulliDCode < 10) || (ulliDCode > MAX_UINT32))
	{
		return Error("Aperture D code must be 10 or greater, could not parse Command:");
	}

	tResult = mcParser.GetExactCharacterSequence("*%", mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcBlock = mpcCommands->AddApertureBlock((uint32)ulliDCode);

	pcParentCommands = mpcCommands;
	mpcCommands = pcBlock->GetCommands();

	tResult = ParseBlockBody("%AB*%", "Block aperture statement (AB) not closed, could not parse Command:");

	mpcCommands = pcParentCommands;

	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseSRStatement()
{
	TRISTATE						tResult;
	uint64							ulliXRepeats;
	uint64							ulliYRepeats;
	CNumber							cXStep;
	CNumber							cYStep;
	CGerberCommandStepAndRepeat*	pcStepAndRepeat;
	CGerberCommands*				pcParentCommands;

	tResult = mcParser.GetExactCharacterSequence("%SR", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	tResult = mcParser.GetExactCharacter('X', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetDigits(&ulliXRepeats, NULL, NULL, false, false, 10, NUMBER_SEPARATOR_NONE);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetExactCharacter('Y', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetDigits(&ulliYRepeats, NULL, NULL, false, false, 10, NUMBER_SEPARATOR_NONE);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	if ((ulliXRepeats < 1) || (ulliXRepeats > MAX_UINT32) || (ulliYRepeats < 1) || (ulliYRepeats > MAX_UINT32))
	{
		return Error("Step and repeat (SR) X and Y must be 1 or greater, could not parse Command:");
	}

	tResult = mcParser.GetExactCharacter('I', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetNumber(&cXStep);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetExactCharacter('J', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);
	tResult = mcParser.GetNumber(&cYStep);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	tResult = mcParser.GetExactCharacterSequence("*%", mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcStepAndRepeat = mpcCommands->AddStepAndRepeat((uint32)ulliXRepeats, (uint32)ulliYRepeats, &cXStep, &cYStep);

	pcParentCommands = mpcCommands;
	mpcCommands = pcStepAndRepeat->GetCommands();

	//SR statements cannot be nested so the body is parsed as a block.
	tResult = ParseBlockBody("%SR*%", "Step and repeat statement (SR) not closed, could not parse Command:");

	mpcCommands = pcParentCommands;

	return tResult;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseBlockBody(const char* szClose, char* szNotClosedError)
{
	TRISTATE	tResult;

	for (;;)
	{
		mcParser.SkipWhitespace();
		if (mcParser.IsOutside())
		{
			return Error(szNotClosedError);
		}

		tResult = mcParser.GetExactCharacterSequence(szClose, mbSkipWhitespace);
		ReturnSyntaxErrorOnError(tResult);
		if (tResult == TRITRUE)
		{
			return TRITRUE;
		}

		tResult = ParseBlockStatement();
		ContinueOnTrueReturnOnError(tResult);

		ReturnSyntanxError();
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseBlockStatement(void)
{
	TRISTATE	tResult;

	//Everything allowed inside an AB or SR statement.  MO, FS, SR and M02 are not.
	tResult = ParseCommandG04();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandAD();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandAM();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandDnn();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandG74();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandG75();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandG01();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandG02();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandG03();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandOperation();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandLP();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandLM();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandLR();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandLS();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseRegionStatement();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseABStatement();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandTF();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandTA();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandTO();
	if (tResult != TRIFALSE) return tResult;

	tResult = ParseCommandTD();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseAttributeArguments(CGerberCommandAttribute* pcCommandAttribute)
{
	TRISTATE						tResult;
	size							iLength;
	char*							szValue;

	for (;;)
	{
		tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
		ReturnSyntaxErrorOnError(tResult);
		if (tResult == TRITRUE)
		{
			tResult = mcParser.GetExactCharacter('%', mbSkipWhitespace);
			ReturnSyntaxErrorOnErrorOrFalse(tResult);
			break;
		}

		tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
		ReturnOnError(tResult);		
		if (tResult == TRIFALSE)
		{
			ReturnSyntanxError();
		}

		mcParser.PushPosition();
		tResult = GetFieldString(NULL, &iLength);
		mcParser.PopPosition();
		ReturnSyntaxErrorOnErrorOrFalse(tResult);

		szValue = pcCommandAttribute->AddValue(iLength);
		GetFieldString(szValue, &iLength);
		
		mcParser.StepLeft();
	}

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandTF()
{
	TRISTATE						tResult;
	size							iLength;
	CGerberCommandFileAttribute*	pcFileAttribute;

	tResult = mcParser.GetExactCharacterSequence("%TF", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mcParser.PushPosition();
	tResult = GetNameString(NULL, &iLength);
	mcParser.PopPosition();
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcFileAttribute = mpcCommands->AddFileAttribute(iLength);
	GetNameString(pcFileAttribute->NameText(), &iLength);

	return ParseAttributeArguments(pcFileAttribute);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandTA()
{
	TRISTATE							tResult;
	size								iLength;
	CGerberCommandApertureAttribute*	pcApertureAttribute;

	tResult = mcParser.GetExactCharacterSequence("%TA", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mcParser.PushPosition();
	tResult = GetNameString(NULL, &iLength);
	mcParser.PopPosition();
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcApertureAttribute = mpcCommands->AddApertureAttribute(iLength);
	GetNameString(pcApertureAttribute->NameText(), &iLength);

	return ParseAttributeArguments(pcApertureAttribute);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandTO()
{
	TRISTATE						tResult;
	size							iLength;
	CGerberCommandObjectAttribute*	pcObjectAttribute;

	tResult = mcParser.GetExactCharacterSequence("%TO", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mcParser.PushPosition();
	tResult = GetNameString(NULL, &iLength);
	mcParser.PopPosition();
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcObjectAttribute = mpcCommands->AddObjectAttribute(iLength);
	GetNameString(pcObjectAttribute->NameText(), &iLength);

	return ParseAttributeArguments(pcObjectAttribute);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandTD()
{
	TRISTATE						tResult;
	size							iLength;
	CGerberCommandDeleteAttribute*	pcDeleteAttribute;

	tResult = mcParser.GetExactCharacterSequence("%TD", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	//No name deletes all attributes.
	mcParser.PushPosition();
	tResult = GetNameString(NULL, &iLength);
	mcParser.PopPosition();
	ReturnSyntaxErrorOnError(tResult);
	if (tResult == TRIFALSE)
	{
		iLength = 0;
	}

	pcDeleteAttribute = mpcCommands->AddDeleteAttribute(iLength);
	if (iLength > 0)
	{
		GetNameString(pcDeleteAttribute->NameText(), &iLength);
	}

	tResult = mcParser.GetExactCharacterSequence("*%", mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandM02()
{
	TRISTATE	tResult;

	tResult = mcParser.GetExactCharacterSequence("M02", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	mpcCommands->AddEndOfFile();

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::Parse(void)
{
	TRISTATE tResult;

	for (;;)
	{
		mcParser.SkipWhitespace();
		if (mcParser.IsOutside())
		{
			return TRITRUE;
		}

		tResult = ParseCommandMO();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandFS();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseSRStatement();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseBlockStatement();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandM02();
		if (tResult != TRITRUE)
		{
			ReturnSyntanxError();
		}
		else
		{
			return TRITRUE;
		}
	}
}

