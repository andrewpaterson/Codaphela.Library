#ifndef __TILE_CEL_BRUSH_H__
#define __TILE_CEL_BRUSH_H__
#include "BaseLib/PrimitiveTypes.h"
#include "ImageCel.h"


class CTileCelBrush : public CObject
{
CONSTRUCTABLE(CTileCelBrush);
DESTRUCTABLE(CTileCelBrush);
protected:
	Ptr<CImageCel>	mpCel;
	size			miCelType;
	char*			mszPatterName;
	size			miWeight;

public:
	void				Init(Ptr<CImageCel> pCel, size iCelType, char* szPatterName, size iWeight);
	void				Free(void);
	void				Class(void);

	bool				Load(CObjectReader* pcFile);
	bool				Save(CObjectWriter* pcFile);

	bool				IsFor(size iCelType, char* szPatternConstantName);
	Ptr<CImageCel>		GetCel(void);
	size				GetWeight(void);
};


#endif // __TILE_CEL_BRUSH_H__

