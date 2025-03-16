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
#include "GerberParser.h"
#include "GerberString.h"
#include "GerberWhitespace.h"
#include "BaseLib/Logger.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberParser::Init(char* szText, size iTextLen, char* szFileName, CGerberCommands* pcCommands)
{
	STextParserConfig	sConfig;

	sConfig.Init(SkipGerberWhitespace, ParseGerberStringUnset);

	mcParser.Init(szText, iTextLen, &sConfig);
	mszFilename.Init(szFileName);

	mpcCommands = pcCommands;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberParser::Kill(void)
{
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
TRISTATE CGerberParser::ParseCommandG04()
{
	TRISTATE				tResult;
	size					iLength;
	CGerberCommandComment*	pcComment;

	tResult = mcParser.GetExactCharacterSequence("G04", false);
	ReturnOnFalseOrCommandSyntaxError(tResult);

	mcParser.PushPosition();
	tResult = GetCommentString(NULL, &iLength);
	mcParser.PopPosition();
	ReturnErrorOnFalseOrCommandSyntaxError(tResult);

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
	EGerberMode				eMode;

	tResult = mcParser.GetExactCharacterSequence("%MO", false);
	ReturnOnFalseOrCommandSyntaxError(tResult);

	mcParser.PushPosition();
	tResult = mcParser.GetExactCharacterSequence("MM", false);
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
		tResult = mcParser.GetExactCharacterSequence("IN", false);
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
			mcParser.PopPosition();
			ReturnSyntanxError();
		}
		else
		{
			mcParser.PopPosition();
			ReturnSyntanxError();
		}
	}
	

	mpcCommands->AddModeSet(eMode);
	
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandFS()
{
	return TRIFALSE;
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
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandG02()
{
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CGerberParser::ParseCommandG03()
{
	return TRIFALSE;
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
	return TRIFALSE;
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

	tResult = mcParser.GetExactCharacterSequence("%TF", false);
	ReturnOnFalseOrCommandSyntaxError(tResult);

	mcParser.PushPosition();
	tResult = mcParser.GetIdentifier(NULL, &iLength, false, false);
	mcParser.PopPosition();
	ReturnErrorOnFalseOrCommandSyntaxError(tResult);

	pcFileAttribute = mpcCommands->AddFileAttribute(iLength);
	mcParser.GetIdentifier(pcFileAttribute->NameText(), &iLength, false, false);

	for (;;)
	{
		tResult = mcParser.GetExactCharacter('*', false);
		ReturnOnError(tResult);
		if (tResult == TRITRUE)
		{
			break;
		}

		tResult = mcParser.GetExactCharacter(',', false);
		ReturnOnError(tResult);		
		if (tResult == TRIFALSE)
		{
			ReturnSyntanxError();
		}

		mcParser.PushPosition();
		tResult = GetFieldString(NULL, &iLength);
		mcParser.PopPosition();
		ReturnErrorOnFalseOrCommandSyntaxError(tResult);

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
TRISTATE CGerberParser::ParseEnd()
{
	return TRIFALSE;
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

		tResult = ParseEnd();
		if (tResult != TRITRUE)
		{
			return TRIERROR;
		}
		else
		{
			return TRITRUE;
		}
	}
}

