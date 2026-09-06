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

public:
	void				Init(Ptr<CImageCel> pCel, size iCelType, char* szPatterName);
	void				Free(void);
	void				Class(void);

	bool				Load(CObjectReader* pcFile);
	bool				Save(CObjectWriter* pcFile);
};


#endif // __TILE_CEL_BRUSH_H__

