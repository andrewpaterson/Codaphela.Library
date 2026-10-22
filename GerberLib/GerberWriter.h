#ifndef __GERBER_WRITER_H__
#define __GERBER_WRITER_H__
#include "BaseLib/Chars.h"
#include "GerberCommands.h"


//Writes commands as Gerber text, one command per line.  Coordinates are written in the format of the FS command and
//must fit it exactly; nothing is rounded.
class CGerberWriter
{
protected:
	CChars*		mpsz;
	bool		mbFormatSpecified;
	uint16		muiXWholes;
	uint16		muiXDecimals;
	uint16		muiYWholes;
	uint16		muiYDecimals;

public:
	void	Init(void);
	void	Kill(void);

	bool	Write(CGerberCommands* pcCommands, CChars* psz);

	//Writes commands one at a time to psz, eg: to join several files.  A format specifier must be written before any coordinates.
	void	Begin(CChars* psz);
	bool	WriteCommand(CGerberCommand* pcCommand);

protected:
	bool	WriteCommands(CGerberCommands* pcCommands);
	bool	WriteApertureDefine(CGerberCommandApertureDefine* pcDefine);
	bool	WriteApertureMacro(CGerberCommandApertureMacro* pcMacro);
	bool	WritePrimitive(CGerberApertureMacro* pcPrimitive);
	bool	WriteExpressions(const char* szCode, CGerberExpression** apcExpressions, size uiNumExpressions);
	bool	WriteOperation(CGerberCommandOperation* pcOperation);
	bool	WriteCoordinate(char cAxis, CNumber* pcValue, uint16 uiWholes, uint16 uiDecimals);
	void	WriteAttribute(const char* szCode, CGerberCommandAttribute* pcAttribute);
	void	WriteNumber(CNumber* pcNumber);
	void	WriteString(const char* szString, bool bField);
	void	WriteEscape(uint32 uiCodePoint);

	bool	Error(const char* szError);
};


#endif // __GERBER_WRITER_H__

