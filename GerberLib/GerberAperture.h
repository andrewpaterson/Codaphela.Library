#ifndef __GERBER_APERTURE_H__
#define __GERBER_APERTURE_H__
#include "GerberNumbers.h"
#include "GerberEvaluatedPrimitive.h"


class CGerberCommandApertureMacro;
class CGerberCommandApertureBlock;


enum EGerberApertureType
{
	GAT_Unknown,
	GAT_Circle,		//C: Diameter[, Hole diameter]
	GAT_Rectangle,	//R: X size, Y size[, Hole diameter]
	GAT_Obround,	//O: X size, Y size[, Hole diameter]
	GAT_Polygon,	//P: Outer diameter, Vertices[, Rotation[, Hole diameter]]
	GAT_Macro,		//An aperture macro with the AD parameters as $1, $2 ...
	GAT_Block,		//An AB statement.
};


//An entry in the aperture dictionary, created by AD or AB.
class CGerberAperture
{
protected:
	uint32							muiDCode;
	EGerberApertureType				meType;
	CArrayGerberNumber				macParameters;
	CGerberCommandApertureMacro*	mpcMacro;
	CGerberEvaluatedPrimitives		mcPrimitives;
	CGerberCommandApertureBlock*	mpcBlock;

public:
	void							Init(uint32 uiDCode, EGerberApertureType eType);
	void							Kill(void);

	CNumber*						AddParameter(CNumber* pcParameter);
	void							SetMacro(CGerberCommandApertureMacro* pcMacro);
	void							SetBlock(CGerberCommandApertureBlock* pcBlock);

	uint32							GetDCode(void);
	EGerberApertureType				GetType(void);
	bool							IsType(EGerberApertureType eType);
	bool							IsSolidCircle(void);

	size							NumParameters(void);
	CNumber*						GetParameter(size uiIndex);

	CGerberCommandApertureMacro*	GetMacro(void);
	CGerberEvaluatedPrimitives*		GetPrimitives(void);
	CGerberCommandApertureBlock*	GetBlock(void);
};


#endif // __GERBER_APERTURE_H__

