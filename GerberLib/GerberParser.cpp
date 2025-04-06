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
void CGerberParser::Init(char* szText, size iTextLen, char* szFileName, CGerberCommands* pcCommands)
{
	STextParserConfig	sConfig;

	sConfig.Init(SkipGerberWhitespace, ParseGerberStringUnset, ParseGerberExactIdentifierUnset, ParseGerberIdentifier);

	mcParser.Init(szText, iTextLen, &sConfig);
	mszFilename.Init(szFileName);

	mpcCommands = pcCommands;
	mbSkipWhitespace = true;

	mcSymbols.Init(true);
	mcSymbols.SetOperator("+", CO_Add, 4);
	mcSymbols.SetOperator("-", CO_Subtract, 4);
	mcSymbols.SetOperator("x", CO_Multiply, 3);
	mcSymbols.SetOperator("/", CO_Divide, 2);
	mcSymbols.SetOperator("=", CO_Assignment, 5);
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
	tResult = mcParser.GetString(szString, piLength, false);
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
	tResult = mcParser.GetString(szString, piLength, false);
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
	tResult = mcParser.GetIdentifier(szString, piLength, false);
	mcParser.SetIdentifierParser(ParseGerberIdentifier);
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
	tResult = mcParser.GetIdentifier(szString, piLength, false);
	mcParser.SetIdentifierParser(ParseGerberIdentifier);
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
	tResult = mcParser.GetIdentifier(szString, piLength, false);
	mcParser.SetIdentifierParser(ParseGerberIdentifier);
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
	
	eMode = GM_Unknown;
	if (tResult == TRITRUE)
	{
		eMode = GM_Millimeters;
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
			eMode = GM_Inches;
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

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandAD()
{
	return TRIFALSE;
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

	tResult = mcParser.GetExactCharacter(',', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	uiPrimitiveType = (EGerberApertureMacroPrimitive)ulli;
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
	tResult = GetNameString(NULL, &uiLength);
	mcParser.PopPosition();
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcComment = pcApertureMacro->AddComment(uiLength);
	GetNameString(pcComment->Text(), &uiLength);

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

	tResult = ParseExpression(pcCenterLine->GetWidth());
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

	uiSize = (size)ulliVerticies;
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
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandDnn()
{
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandG75()
{
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandG01()
{
	TRISTATE				tResult;

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
	TRISTATE				tResult;

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
	TRISTATE				tResult;

	tResult = mcParser.GetExactCharacterSequence("G03*", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mpcCommands->AddPlotMode(GPM_CircularAnticlockwise);

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandD01()
{
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandD02()
{
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandD03()
{
	return TRIFALSE;
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
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandLR()
{
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandLS()
{
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseRegionStatement()
{
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseABStatement()
{
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseSRStatement()
{
	return TRIFALSE;
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
	char*							szValue;

	tResult = mcParser.GetExactCharacterSequence("%TF", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	mcParser.PushPosition();
	tResult = GetNameString(NULL, &iLength);
	mcParser.PopPosition();
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcFileAttribute = mpcCommands->AddFileAttribute(iLength);
	GetNameString(pcFileAttribute->NameText(), &iLength);

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

		szValue = pcFileAttribute->AddValue(iLength);
		GetFieldString(szValue, &iLength);
		
		mcParser.StepLeft();
	}

	return TRITRUE;

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandTA()
{
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandTO()
{
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandTD()
{
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandM02()
{
	TRISTATE					tResult;
	CGerberCommandEndOfFile*	pcEndOfFile;

	tResult = mcParser.GetExactCharacterSequence("M02", mbSkipWhitespace);
	ReturnOnFalseOrSyntaxErrorOnError(tResult);

	tResult = mcParser.GetExactCharacter('*', mbSkipWhitespace);
	ReturnSyntaxErrorOnErrorOrFalse(tResult);

	pcEndOfFile = mpcCommands->AddEndOfFile();

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

		tResult = ParseCommandG04();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandMO();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandFS();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandAD();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandAM();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandDnn();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandG75();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandG01();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandG02();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandG03();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandD01();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandD02();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandD03();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandLP();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandLM();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandLR();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandLS();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseRegionStatement();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseABStatement();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseSRStatement();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandTF();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandTA();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandTO();
		ContinueOnTrueReturnOnError(tResult);

		tResult = ParseCommandTD();
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

