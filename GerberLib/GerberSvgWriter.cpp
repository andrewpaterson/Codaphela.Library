#include <math.h>
#include "GerberSvgWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberSvgWriter::Init(void)
{
	mpsz = NULL;
	msMinimum.Init(0, 0);
	msMaximum.Init(0, 0);
	miMasks = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberSvgWriter::Kill(void)
{
	mpsz = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberSvgWriter::WriteNumber(float64 f)
{
	int64	lliValue;
	int64	lliWhole;
	int64	lliFraction;
	int		iDigits;
	char	szFraction[8];

	//To 0.000001 mm, without trailing zeros.  The same text on every build.
	lliValue = (int64)llround(f * 1000000.0);
	if (lliValue < 0)
	{
		mpsz->Append('-');
		lliValue = -lliValue;
	}
	lliWhole = lliValue / 1000000;
	lliFraction = lliValue % 1000000;
	mpsz->Append(lliWhole);
	if (lliFraction != 0)
	{
		for (iDigits = 5; iDigits >= 0; iDigits--)
		{
			szFraction[iDigits] = (char)('0' + lliFraction % 10);
			lliFraction /= 10;
		}
		iDigits = 6;
		while (szFraction[iDigits - 1] == '0')
		{
			iDigits--;
		}
		szFraction[iDigits] = 0;
		mpsz->Append('.');
		mpsz->Append(szFraction);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberSvgWriter::WritePoint(SFloat64Vec2* psPoint)
{
	//SVG Y is down.
	WriteNumber(psPoint->x);
	mpsz->Append(' ');
	WriteNumber(psPoint->y == 0 ? 0 : -psPoint->y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberSvgWriter::WriteRectangle(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, const char* szFill)
{
	mpsz->Append("<rect x=\"");
	WriteNumber(psMinimum->x);
	mpsz->Append("\" y=\"");
	WriteNumber(-psMaximum->y);
	mpsz->Append("\" width=\"");
	WriteNumber(psMaximum->x - psMinimum->x);
	mpsz->Append("\" height=\"");
	WriteNumber(psMaximum->y - psMinimum->y);
	mpsz->Append("\" fill=\"");
	mpsz->Append(szFill);
	mpsz->Append("\"/>");
	mpsz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberSvgWriter::WriteMaskStart(int iMask, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum)
{
	mpsz->Append("<mask id=\"m");
	mpsz->Append(iMask);
	mpsz->Append("\" maskUnits=\"userSpaceOnUse\" x=\"");
	WriteNumber(psMinimum->x);
	mpsz->Append("\" y=\"");
	WriteNumber(-psMaximum->y);
	mpsz->Append("\" width=\"");
	WriteNumber(psMaximum->x - psMinimum->x);
	mpsz->Append("\" height=\"");
	WriteNumber(psMaximum->y - psMinimum->y);
	mpsz->Append("\">");
	mpsz->AppendNewLine();
	WriteRectangle(psMinimum, psMaximum, "white");
	mpsz->Append("<g fill=\"black\">");
	mpsz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberSvgWriter::Begin(CChars* psz, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, float64 fMargin, const char* szBackground)
{
	mpsz = psz;
	miMasks = 0;
	msMinimum.Init(psMinimum->x - fMargin, psMinimum->y - fMargin);
	msMaximum.Init(psMaximum->x + fMargin, psMaximum->y + fMargin);

	mpsz->Append("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"");
	WriteNumber(msMaximum.x - msMinimum.x);
	mpsz->Append("mm\" height=\"");
	WriteNumber(msMaximum.y - msMinimum.y);
	mpsz->Append("mm\" viewBox=\"");
	WriteNumber(msMinimum.x);
	mpsz->Append(' ');
	WriteNumber(-msMaximum.y);
	mpsz->Append(' ');
	WriteNumber(msMaximum.x - msMinimum.x);
	mpsz->Append(' ');
	WriteNumber(msMaximum.y - msMinimum.y);
	mpsz->Append("\">");
	mpsz->AppendNewLine();
	if (szBackground)
	{
		WriteRectangle(&msMinimum, &msMaximum, szBackground);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberSvgWriter::End(void)
{
	mpsz->Append("</svg>");
	mpsz->AppendNewLine();
	mpsz = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberSvgWriter::Write(CChars* psz, CGerberVectorImage* pcImage, const char* szColour, const char* szBackground)
{
	SFloat64Vec2	sMinimum;
	SFloat64Vec2	sMaximum;

	if (!pcImage->GetBounds(&sMinimum, &sMaximum))
	{
		sMinimum.Init(0, 0);
		sMaximum.Init(1, 1);
	}
	Begin(psz, &sMinimum, &sMaximum, 1.0, szBackground);
	AddLayer(pcImage, szColour, 1.0);
	End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberSvgWriter::AddLayer(CGerberVectorImage* pcImage, const char* szColour, float64 fOpacity)
{
	size		i;
	size		uiNumObjects;
	size		uiFirstMask;
	int			iMask;
	int			iNumClearRuns;
	bool		bPreviousDark;
	bool		bDark;

	//(((D1 - C1) + D2) - C2) + D3: a mask for each run of clear objects over everything before it.
	uiNumObjects = pcImage->NumObjects();
	iNumClearRuns = 0;
	bPreviousDark = true;
	for (i = 0; i < uiNumObjects; i++)
	{
		bDark = pcImage->GetObject(i)->IsDark();
		if (!bDark && bPreviousDark)
		{
			iNumClearRuns++;
		}
		bPreviousDark = bDark;
	}

	mpsz->Append("<g fill=\"");
	mpsz->Append(szColour);
	mpsz->Append('"');
	if (fOpacity < 1)
	{
		mpsz->Append(" opacity=\"");
		WriteNumber(fOpacity);
		mpsz->Append('"');
	}
	mpsz->Append(">");
	mpsz->AppendNewLine();

	uiFirstMask = miMasks;
	if (iNumClearRuns > 0)
	{
		mpsz->Append("<defs>");
		mpsz->AppendNewLine();
		bPreviousDark = true;
		for (i = 0; i < uiNumObjects; i++)
		{
			bDark = pcImage->GetObject(i)->IsDark();
			if (!bDark && bPreviousDark)
			{
				WriteMaskStart(miMasks, &msMinimum, &msMaximum);
				miMasks++;
			}
			else if (bDark && !bPreviousDark)
			{
				mpsz->Append("</g></mask>");
				mpsz->AppendNewLine();
			}
			if (!bDark)
			{
				WriteObject(pcImage->GetObject(i));
			}
			bPreviousDark = bDark;
		}
		if (!bPreviousDark)
		{
			mpsz->Append("</g></mask>");
			mpsz->AppendNewLine();
		}
		mpsz->Append("</defs>");
		mpsz->AppendNewLine();

		for (iMask = (int)uiFirstMask + iNumClearRuns - 1; iMask >= (int)uiFirstMask; iMask--)
		{
			mpsz->Append("<g mask=\"url(#m");
			mpsz->Append(iMask);
			mpsz->Append(")\">");
			mpsz->AppendNewLine();
		}
	}

	bPreviousDark = true;
	for (i = 0; i < uiNumObjects; i++)
	{
		bDark = pcImage->GetObject(i)->IsDark();
		if (!bDark && bPreviousDark)
		{
			mpsz->Append("</g>");
			mpsz->AppendNewLine();
		}
		if (bDark)
		{
			WriteObject(pcImage->GetObject(i));
		}
		bPreviousDark = bDark;
	}

	mpsz->Append("</g>");
	mpsz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberSvgWriter::WriteObject(CGerberVectorObject* pcObject)
{
	size					i;
	size					uiNumSegments;
	size					uiNumParts;
	SFloat64Vec2			sMinimum;
	SFloat64Vec2			sMaximum;
	SVectorSegment*	psSegment;
	int						iFirstMask;
	int						iNumOffRuns;
	int						iMask;
	bool					bPreviousOn;
	size					uiRunStart;

	uiNumSegments = pcObject->NumSegments();
	if (!pcObject->HasExposureOff())
	{
		//Each part on its own; parts can overlap and wind differently.
		uiRunStart = 0;
		for (i = 1; i <= uiNumSegments; i++)
		{
			if ((i == uiNumSegments) || (pcObject->GetSegment(i)->eType == VS_Part))
			{
				WritePath(pcObject, uiRunStart, i);
				uiRunStart = i;
			}
		}
		return;
	}

	//Parts with exposure off erase the parts before them, inside the object only.
	uiNumParts = 0;
	iNumOffRuns = 0;
	bPreviousOn = true;
	for (i = 0; i < uiNumSegments; i++)
	{
		psSegment = pcObject->GetSegment(i);
		if (psSegment->eType == VS_Part)
		{
			if (!psSegment->bExposure && bPreviousOn)
			{
				iNumOffRuns++;
			}
			bPreviousOn = psSegment->bExposure;
			uiNumParts++;
		}
	}

	sMinimum.Init(pcObject->GetMinimum()->x - 0.001, pcObject->GetMinimum()->y - 0.001);
	sMaximum.Init(pcObject->GetMaximum()->x + 0.001, pcObject->GetMaximum()->y + 0.001);
	iFirstMask = miMasks;
	mpsz->Append("<defs>");
	mpsz->AppendNewLine();
	bPreviousOn = true;
	uiRunStart = 0;
	for (i = 0; i <= uiNumSegments; i++)
	{
		if ((i == uiNumSegments) || (pcObject->GetSegment(i)->eType == VS_Part))
		{
			if ((i > uiRunStart) && !bPreviousOn)
			{
				WritePath(pcObject, uiRunStart, i);
			}
			if (i == uiNumSegments)
			{
				break;
			}
			psSegment = pcObject->GetSegment(i);
			if (!psSegment->bExposure && bPreviousOn)
			{
				WriteMaskStart(miMasks, &sMinimum, &sMaximum);
				miMasks++;
			}
			else if (psSegment->bExposure && !bPreviousOn)
			{
				mpsz->Append("</g></mask>");
				mpsz->AppendNewLine();
			}
			bPreviousOn = psSegment->bExposure;
			uiRunStart = i;
		}
	}
	if (!bPreviousOn)
	{
		mpsz->Append("</g></mask>");
		mpsz->AppendNewLine();
	}
	mpsz->Append("</defs>");
	mpsz->AppendNewLine();

	for (iMask = iFirstMask + iNumOffRuns - 1; iMask >= iFirstMask; iMask--)
	{
		mpsz->Append("<g mask=\"url(#m");
		mpsz->Append(iMask);
		mpsz->Append(")\">");
		mpsz->AppendNewLine();
	}

	bPreviousOn = true;
	uiRunStart = 0;
	for (i = 0; i <= uiNumSegments; i++)
	{
		if ((i == uiNumSegments) || (pcObject->GetSegment(i)->eType == VS_Part))
		{
			if ((i > uiRunStart) && bPreviousOn)
			{
				WritePath(pcObject, uiRunStart, i);
			}
			if (i == uiNumSegments)
			{
				break;
			}
			psSegment = pcObject->GetSegment(i);
			if (!psSegment->bExposure && bPreviousOn)
			{
				mpsz->Append("</g>");
				mpsz->AppendNewLine();
			}
			bPreviousOn = psSegment->bExposure;
			uiRunStart = i;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberSvgWriter::WritePath(CGerberVectorObject* pcObject, size uiStart, size uiEnd)
{
	size					i;
	SVectorSegment*	psSegment;
	SFloat64Vec2			sCurrent;
	SFloat64Vec2			sOpposite;
	float64					fRadius;
	float64					fSweep;
	bool					bContour;
	bool					bEmpty;
	char*					szSweep;

	bEmpty = true;
	for (i = uiStart; i < uiEnd; i++)
	{
		if (pcObject->GetSegment(i)->eType != VS_Part)
		{
			bEmpty = false;
			break;
		}
	}
	if (bEmpty)
	{
		return;
	}

	mpsz->Append("<path d=\"");
	bContour = false;
	sCurrent.Init(0, 0);
	for (i = uiStart; i < uiEnd; i++)
	{
		psSegment = pcObject->GetSegment(i);
		switch (psSegment->eType)
		{
			case VS_Move:
				if (bContour)
				{
					mpsz->Append("Z");
				}
				mpsz->Append('M');
				WritePoint(&psSegment->sPoint);
				bContour = true;
				break;

			case VS_Line:
				mpsz->Append('L');
				WritePoint(&psSegment->sPoint);
				break;

			case VS_Arc:
				//SVG Y is down so a counterclockwise arc has sweep flag 0.
				fRadius = hypot(sCurrent.x - psSegment->sCenter.x, sCurrent.y - psSegment->sCenter.y);
				szSweep = psSegment->bClockwise ? (char*)" 1 " : (char*)" 0 ";
				if ((sCurrent.x == psSegment->sPoint.x) && (sCurrent.y == psSegment->sPoint.y))
				{
					//A full circle is two halves; SVG cannot draw an arc to its own start.
					sOpposite.Init(2 * psSegment->sCenter.x - sCurrent.x, 2 * psSegment->sCenter.y - sCurrent.y);
					mpsz->Append('A');
					WriteNumber(fRadius);
					mpsz->Append(' ');
					WriteNumber(fRadius);
					mpsz->Append(" 0 0");
					mpsz->Append(szSweep);
					WritePoint(&sOpposite);
					mpsz->Append('A');
					WriteNumber(fRadius);
					mpsz->Append(' ');
					WriteNumber(fRadius);
					mpsz->Append(" 0 0");
					mpsz->Append(szSweep);
					WritePoint(&psSegment->sPoint);
				}
				else
				{
					fSweep = Float64Vec2ArcSweep(&sCurrent, &psSegment->sPoint, &psSegment->sCenter, psSegment->bClockwise);
					mpsz->Append('A');
					WriteNumber(fRadius);
					mpsz->Append(' ');
					WriteNumber(fRadius);
					mpsz->Append((fSweep > N_PI_64) ? " 0 1" : " 0 0");
					mpsz->Append(szSweep);
					WritePoint(&psSegment->sPoint);
				}
				break;

			default:
				break;
		}
		if (psSegment->eType != VS_Part)
		{
			sCurrent = psSegment->sPoint;
		}
	}
	if (bContour)
	{
		mpsz->Append("Z");
	}
	mpsz->Append("\"/>");
	mpsz->AppendNewLine();
}

