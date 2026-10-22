#ifndef __EXCELLON_WRITER_H__
#define __EXCELLON_WRITER_H__
#include "BaseLib/Chars.h"
#include "ExcellonCommands.h"


//Writes Excellon commands as KiCad style decimal drill text, one command per line.  The output is always in
//millimetres; inch coordinates and diameters are converted (x 25.4, which is exact).
class CExcellonWriter
{
protected:
	CChars*		mpsz;
	bool		mbInches;

public:
	void	Init(void);
	void	Kill(void);

	bool	Write(CExcellonCommands* pcCommands, CChars* psz);

protected:
	bool	WriteCommand(CExcellonCommand* pcCommand);
	void	WriteComment(CExcellonCommandComment* pcComment);
	void	WriteAttribute(const char* szCode, CExcellonCommandAttribute* pcAttribute);
	void	WriteCoordinates(CNumber* pcX, bool bX, CNumber* pcY, bool bY);
	void	WriteNumber(CNumber* pcNumber);

	bool	Error(const char* szError);
};


#endif // __EXCELLON_WRITER_H__

