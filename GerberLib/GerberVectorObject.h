#ifndef __GERBER_VECTOR_OBJECT_H__
#define __GERBER_VECTOR_OBJECT_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Float64Vec2.h"
#include "GerberCommandLoadPolarity.h"


#define GERBER_PI 3.14159265358979323846


enum EGerberVectorSegmentType
{
	GVS_Part,	//Starts a part; a set of contours drawn with an exposure.
	GVS_Move,	//Starts a contour at the point.
	GVS_Line,	//A straight edge to the point.
	GVS_Arc,	//A circular edge to the point around the center.  The end equal to the start is a full circle.
};


struct SGerberVectorSegment
{
	EGerberVectorSegmentType	eType;
	bool						bExposure;		//Part: on draws, off erases the earlier parts of the object.
	bool						bClockwise;		//Arc.
	SFloat64Vec2				sPoint;
	SFloat64Vec2				sCenter;		//Arc.
};


typedef CArrayTemplate<SGerberVectorSegment>	CArrayGerberVectorSegment;


//Mirror, then rotate (degrees counterclockwise), then scale, then translate.  4.9.1
struct SGerberVectorTransform
{
	bool			bMirrorX;		//x = -x
	bool			bMirrorY;		//y = -y
	float64			fRotation;
	float64			fScale;
	SFloat64Vec2	sTranslation;

	void	Init(void);
	void	Transform(SFloat64Vec2* psPoint);
	bool	FlipsWinding(void);
};


//A graphical object of a layer in millimetres.  Its parts are closed contours made of straight and circular edges,
//filled with the non-zero winding rule; holes wind the other way.  Parts with exposure off erase the earlier parts of
//the same object only.  The polarity says whether the object darkens or clears the layer beneath it.
class CGerberVectorObject
{
protected:
	EGerberPolarity				mePolarity;
	CArrayGerberVectorSegment	masSegments;
	SFloat64Vec2				msMinimum;
	SFloat64Vec2				msMaximum;
	bool						mbBounds;

public:
	void					Init(EGerberPolarity ePolarity);
	void					Init(CGerberVectorObject* pcSource);
	void					Kill(void);

	void					BeginPart(bool bExposure);
	void					MoveTo(SFloat64Vec2* psPoint);
	void					LineTo(SFloat64Vec2* psPoint);
	void					ArcTo(SFloat64Vec2* psPoint, SFloat64Vec2* psCenter, bool bClockwise);

	void					AddCircle(SFloat64Vec2* psCenter, float64 fRadius, bool bClockwise);
	void					AddPolygon(SFloat64Vec2* asPoints, size uiNumPoints, bool bReverse);

	void					Transform(SGerberVectorTransform* psTransform);
	void					SetPolarity(EGerberPolarity ePolarity);
	void					CalculateBounds(void);

	EGerberPolarity			GetPolarity(void);
	bool					IsDark(void);
	size					NumSegments(void);
	SGerberVectorSegment*	GetSegment(size uiIndex);
	bool					HasBounds(void);
	SFloat64Vec2*			GetMinimum(void);
	SFloat64Vec2*			GetMaximum(void);
	bool					HasExposureOff(void);
	float64					Area(void);
};


//The counterclockwise or clockwise angle (radians) from the start to the end around the center, in (0, 2 pi].  Equal
//points are a full circle.
float64 GetGerberArcSweep(SFloat64Vec2* psStart, SFloat64Vec2* psEnd, SFloat64Vec2* psCenter, bool bClockwise);


#endif // __GERBER_VECTOR_OBJECT_H__

