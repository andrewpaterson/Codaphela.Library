#ifndef __GERBER_LAYER_FILE_H__
#define __GERBER_LAYER_FILE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "GerberBoardLayer.h"
#include "GerberBoardSide.h"
#include "GerberCommands.h"


//One Gerber file of a board, eg: "Adder 16bit-F_Cu.gtl".  The layer and side come from its %TF.FileFunction attribute.
class CGerberLayerFile
{
protected:
	CChars				mszFileName;  //Without the directory.
	CGerberCommands		mcCommands;
	EGerberBoardLayer	meLayer;
	EGerberBoardSide	meSide;

public:
	void				Init(void);
	void				Kill(void);

	bool				Load(char* szPathName);

	char*				GetFileName(void);
	CGerberCommands*	GetCommands(void);
	EGerberBoardLayer	GetLayer(void);
	EGerberBoardSide	GetSide(void);

protected:
	void				IdentifyLayer(void);
	EGerberBoardSide	GetSide(char* szSide);
};


typedef CArrayTemplatePtr<CGerberLayerFile>	CArrayGerberLayerFilePtr;


#endif // __GERBER_LAYER_FILE_H__
