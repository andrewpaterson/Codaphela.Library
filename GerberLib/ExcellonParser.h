#ifndef __EXCELLON_PARSER_H__
#define __EXCELLON_PARSER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Number.h"
#include "ExcellonCommands.h"


//Parses the Excellon drill files written by KiCad: decimal coordinates, absolute positions, round holes and
//slots, both routed (G00, M15, G01, M16) and canned (G85).
//Excellon is line based.  Lines can end with LF or CRLF.
class CExcellonParser
{
protected:
	const char*			mszText;
	size				muiTextLength;
	CChars				mszFilename;
	CExcellonCommands*	mpcCommands;

	size				muiPosition;		//Start of the next line.
	size				muiLine;			//Of the current line, starting at 1.
	const char*			mszLine;
	size				muiLineLength;		//Without the line ending or trailing whitespace.

	bool				mbHeaderStarted;
	bool				mbHeaderEnded;
	bool				mbEndOfProgram;

public:
	void		Init(char* szText, size uiTextLength, char* szFileName, CExcellonCommands* pcCommands);
	void		Kill(void);

	TRISTATE	Parse(void);

protected:
	bool		NextLine(void);
	TRISTATE	ParseLine(void);
	TRISTATE	ParseComment(void);
	TRISTATE	ParseHeaderLine(void);
	TRISTATE	ParseBodyLine(void);
	TRISTATE	ParseToolDefine(void);
	TRISTATE	ParseToolSelect(void);
	TRISTATE	ParseCoordinate(EExcellonCommandEnum eType, size uiPosition);
	TRISTATE	ParseCoordinates(size* puiPosition, CNumber* pcX, bool* pbX, CNumber* pcY, bool* pbY);

	bool		LineIs(const char* szText);
	bool		LineStartsWith(const char* szText);
	bool		GetUnsigned(size* puiPosition, uint32* puiValue);
	bool		GetNumber(size* puiPosition, CNumber* pcNumber, bool bNeedPoint);

	TRISTATE	Error(const char* szError);
};


#endif // __EXCELLON_PARSER_H__

