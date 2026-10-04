#ifndef __TILE_CEL_BRUSH_H__
#define __TILE_CEL_BRUSH_H__
#include "BaseLib/PrimitiveTypes.h"
#include "ImageCel.h"


class CTileCelBrush;
typedef void (*CelBrushSelectionCallback)(Ptr<CTileCelBrush>, size, size, void*);


class CTileCelBrush : public CObject
{
CONSTRUCTABLE(CTileCelBrush);
DESTRUCTABLE(CTileCelBrush);
protected:
	Ptr<CImageCel>				mpCel;
	size						miCelType;
	char*						mszPatternName;
	size						miWeight;
	CelBrushSelectionCallback	mfCallback;
	void*						mpvCallbackData;

public:
	void				Init(Ptr<CImageCel> pCel, size iCelType, char* szPatterName, size iWeight);
	void				Init(Ptr<CImageCel> pCel, size iCelType, char* szPatterName, size iWeight, CelBrushSelectionCallback fCallback, void* pvCallbackData);
	void				Free(void);
	void				Class(void);

	bool				Load(CObjectReader* pcFile);
	bool				Save(CObjectWriter* pcFile);

	bool				IsFor(size iCelType, char* szPatternConstantName);
	Ptr<CImageCel>		GetCel(void);
	size				GetWeight(void);

	void				SetCallback(CelBrushSelectionCallback fCallback, void* pvCallbackData);
	void				Callback(size x, size y);
	char*				GetPatternName(void);
};


#endif // __TILE_CEL_BRUSH_H__

