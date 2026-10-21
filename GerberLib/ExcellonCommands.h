#ifndef __EXCELLON_COMMANDS_H__
#define __EXCELLON_COMMANDS_H__
#include "BaseLib/ListVariable.h"
#include "ExcellonCommandCode.h"
#include "ExcellonCommandComment.h"
#include "ExcellonCommandAttribute.h"
#include "ExcellonCommandFormat.h"
#include "ExcellonCommandUnit.h"
#include "ExcellonCommandToolDefine.h"
#include "ExcellonCommandToolSelect.h"
#include "ExcellonCommandCoordinate.h"
#include "ExcellonCommandSlot.h"


class CExcellonCommands
{
protected:
	CListVariable	macCommands;

public:
	void							Init(void);
	void							Kill(void);

	CExcellonCommandCode*			AddCode(EExcellonCommandEnum eType);
	CExcellonCommandComment*		AddComment(const char* szComment, size uiLength);
	CExcellonCommandAttribute*		AddAttribute(EExcellonCommandEnum eType, const char* szName, size uiLength);
	CExcellonCommandFormat*			AddFormat(uint16 uiFormat);
	CExcellonCommandUnit*			AddUnit(EExcellonUnit eUnit);
	CExcellonCommandToolDefine*		AddToolDefine(uint32 uiTool, CNumber* pcDiameter);
	CExcellonCommandToolSelect*		AddToolSelect(uint32 uiTool);
	CExcellonCommandCoordinate*		AddCoordinate(EExcellonCommandEnum eType);
	CExcellonCommandSlot*			AddSlot(void);

	size							NumCommands(void);
	CExcellonCommand*				GetCommand(size uiIndex);
};


#endif // __EXCELLON_COMMANDS_H__

