#ifndef __GERBER_SVG_WRITER_H__
#define __GERBER_SVG_WRITER_H__
#include "BaseLib/Chars.h"
#include "GerberVectorImage.h"


//Writes vector images as SVG in millimetres, Y up.  Each layer is a group in its colour.  Clear objects become masks
//over the objects before them; exposure off parts of an object become masks over the parts before them.
class CGerberSvgWriter
{
protected:
	CChars*			mpsz;
	SFloat64Vec2	msMinimum;
	SFloat64Vec2	msMaximum;
	int				miMasks;

public:
	void	Init(void);
	void	Kill(void);

	void	Begin(CChars* psz, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, float64 fMargin, const char* szBackground);
	void	AddLayer(CGerberVectorImage* pcImage, const char* szColour, float64 fOpacity);
	void	End(void);

	void	Write(CChars* psz, CGerberVectorImage* pcImage, const char* szColour, const char* szBackground);

protected:
	void	WriteObject(CGerberVectorObject* pcObject);
	void	WriteParts(CGerberVectorObject* pcObject, size uiFirstPart, size uiEndPart, bool bExposure);
	void	WritePath(CGerberVectorObject* pcObject, size uiStart, size uiEnd);
	void	WriteRectangle(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, const char* szFill);
	void	WriteMaskStart(int iMask, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum);
	void	WriteNumber(float64 f);
	void	WritePoint(SFloat64Vec2* psPoint);
};


#endif // __GERBER_SVG_WRITER_H__

