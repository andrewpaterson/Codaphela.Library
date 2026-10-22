#ifndef __GERBER_FILE_SET_H__
#define __GERBER_FILE_SET_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/Float64Vec2.h"
#include "GerberLayerFile.h"
#include "GerberDrillFile.h"


//The Gerber layer files and Excellon drill files of one board, loaded from one directory.  The set is named after the directory, eg: "Adder 16bit".
//The outline bounds are the bounding rectangle of the edge cuts layer's centre lines in millimetres, in the Gerber coordinates (y up).
class CGerberFileSet
{
protected:
	CChars						mszName;
	CChars						mszDirectory;
	CArrayGerberLayerFilePtr	macLayers;  //Sorted by file name.
	CArrayGerberDrillFilePtr	macDrills;  //Sorted by file name.
	bool						mbOutlineBounds;  //False if there is no edge cuts layer or it draws nothing.
	SFloat64Vec2				msOutlineMinimum;
	SFloat64Vec2				msOutlineMaximum;

public:
	void				Init(void);
	void				Kill(void);

	bool				Load(char* szDirectory);

	char*				GetName(void);
	char*				GetDirectory(void);

	size				NumLayers(void);
	CGerberLayerFile*	GetLayer(size uiIndex);
	CGerberLayerFile*	GetLayer(EGerberBoardLayer eLayer, EGerberBoardSide eSide);

	size				NumDrills(void);
	CGerberDrillFile*	GetDrill(size uiIndex);
	CGerberDrillFile*	GetDrill(EGerberDrillPlating ePlating);

	bool				HasOutlineBounds(void);
	SFloat64Vec2*		GetOutlineMinimum(void);
	SFloat64Vec2*		GetOutlineMaximum(void);
	SFloat64Vec2		GetOutlineSize(void);

protected:
	bool				CalculateOutlineBounds(void);
	bool				LoadLayer(char* szPathName);
	bool				LoadDrill(char* szPathName);
	bool				IsGerberFileName(CChars* szFileName);
	bool				IsDrillFileName(CChars* szFileName);
};


typedef CArrayTemplatePtr<CGerberFileSet>	CArrayGerberFileSetPtr;


#endif // __GERBER_FILE_SET_H__
