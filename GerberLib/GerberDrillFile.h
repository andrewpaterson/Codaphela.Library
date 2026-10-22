#ifndef __GERBER_DRILL_FILE_H__
#define __GERBER_DRILL_FILE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "ExcellonCommands.h"


enum EGerberDrillPlating
{
	GDP_Plated,
	GDP_NonPlated,
	GDP_Unknown,
};


//One Excellon drill file of a board, eg: "Adder 16bit-PTH.drl".  The plating comes from its TF.FileFunction attribute.
class CGerberDrillFile
{
protected:
	CChars					mszFileName;  //Without the directory.
	CExcellonCommands		mcCommands;
	EGerberDrillPlating		mePlating;

public:
	void					Init(void);
	void					Kill(void);

	bool					Load(char* szPathName);

	char*					GetFileName(void);
	CExcellonCommands*		GetCommands(void);
	EGerberDrillPlating		GetPlating(void);

protected:
	void					IdentifyPlating(void);
};


typedef CArrayTemplatePtr<CGerberDrillFile>	CArrayGerberDrillFilePtr;


#endif // __GERBER_DRILL_FILE_H__
