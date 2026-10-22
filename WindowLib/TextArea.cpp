#include "BaseLib/UTF8.h"
#include "StandardLib/ClassDefines.h"
#include "MapsCanvasDraw.h"
#include "Window.h"
#include "TextArea.h"


//////////////////////////////////////////////////////////////////////////
//
// szText is UTF8.  Line feeds start new lines.  pWhiteFont's image must
// be white (see CFontColours); the text area makes its own colours of it.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Init(Ptr<CWindow> pWindow, Ptr<CFont> pWhiteFont, const char* szText)
{
	Init(pWindow, OMalloc<CFontColours>(pWhiteFont), szText);
}


//////////////////////////////////////////////////////////////////////////
//
// szText is UTF8.  Line feeds start new lines.  pColours can be shared by
// text areas so each colour is only made once.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Init(Ptr<CWindow> pWindow, Ptr<CFontColours> pColours, const char* szText)
{
	Ptr<CMapsCanvasDraw>	pDraw;

	PreInit();

	mszText.Init(szText);
	mpColours = pColours;
	msColour = FONT_COLOURS_WHITE;
	masSpans.Init();
	mbTextDirty = false;
	mpText = NULL;
	mpLayout = OMalloc<CTextLayout>();
	mpSpriteMap = OMalloc<CSpriteMap>();
	mpFontDraw = OMalloc<CMapFontDraw>(mpSpriteMap);
	mpMaps = OMalloc<CMaps>();
	mpMaps->AddMap(mpSpriteMap);
	mpMaps->SetViewportPosition(0, 0);
	mbWordWrap = false;
	msTextSize.Init(0, 0);

	//The text is drawn over whatever is behind the text area.
	pDraw = OMalloc<CMapsCanvasDraw>(mpMaps);
	CCanvas::Init(pWindow, pDraw);
	SetTransparent(true);

	ReadText();
	LayoutText();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Free(void)
{
	masSpans.Kill();
	mszText.Kill();
	CCanvas::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Class(void)
{
	CCanvas::Class();

	U_String(mszText);
	M_Pointer(mpColours);
	U_UInt32(msColour);
	U_Data(CArrayTextAreaSpan, masSpans);
	U_Bool(mbTextDirty);
	M_Pointer(mpText);
	M_Pointer(mpLayout);
	M_Pointer(mpSpriteMap);
	M_Pointer(mpFontDraw);
	M_Pointer(mpMaps);
	U_Bool(mbWordWrap);
	U_2Int32(msTextSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The spans are removed; they were ranges of the old text.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetText(const char* szText)
{
	mszText.Kill();
	mszText.Init(szText);
	masSpans.ReInit();
	ReadText();
	LayoutText();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds szText (UTF8) to the end of the text in sColour.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::AppendText(const char* szText, ARGB32 sColour)
{
	size	uiStart;

	uiStart = mszText.Length();
	mszText.Append(szText);
	AddSpan(uiStart, mszText.Length() - uiStart, sColour);
	ReadText();
	LayoutText();
}


//////////////////////////////////////////////////////////////////////////
//
// pWhiteFont's image must be white (see CFontColours).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetFont(Ptr<CFont> pWhiteFont)
{
	SetFontColours(OMalloc<CFontColours>(pWhiteFont));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetFontColours(Ptr<CFontColours> pColours)
{
	mpColours = pColours;
	ReadText();
	LayoutText();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetWordWrap(bool bWordWrap)
{
	if (mbWordWrap != bWordWrap)
	{
		mbWordWrap = bWordWrap;
		LayoutText();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The colour of text that isn't in a span.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetColour(ARGB32 sColour)
{
	if (msColour != sColour)
	{
		msColour = sColour;
		mbTextDirty = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// uiStart and uiLength are in bytes of the UTF8 text.  A span that starts
// or ends inside a character starts or ends at the next character.
// Returns false if the span goes past the end of the text.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::AddSpan(size uiStart, size uiLength, ARGB32 sColour)
{
	size			uiTextLength;
	STextAreaSpan*	psSpan;

	uiTextLength = mszText.Length();
	if ((uiStart > uiTextLength) || (uiLength > uiTextLength - uiStart))
	{
		return false;
	}
	if (uiLength == 0)
	{
		return true;
	}

	psSpan = masSpans.Add();
	psSpan->uiStart = uiStart;
	psSpan->uiLength = uiLength;
	psSpan->sColour = sColour;
	mbTextDirty = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ClearSpans(void)
{
	if (masSpans.NumElements() > 0)
	{
		masSpans.ReInit();
		mbTextDirty = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The colour of the byte at uiPosition of the UTF8 text: the colour of the
// last span added that covers it, otherwise the text area's colour.
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CTextArea::GetColourAt(size uiPosition)
{
	size			i;
	STextAreaSpan*	psSpan;

	for (i = masSpans.NumElements(); i > 0; i--)
	{
		psSpan = masSpans.Get(i - 1);
		if ((uiPosition >= psSpan->uiStart) && (uiPosition < psSpan->uiStart + psSpan->uiLength))
		{
			return psSpan->sColour;
		}
	}
	return msColour;
}


//////////////////////////////////////////////////////////////////////////
//
// The text is laid out again for the width the text area is given, and
// read again if its spans or colour changed.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	int32	iHeight;

	CCanvas::Layout(sPosition, sAreaSize);

	if (mbTextDirty || (mpLayout->GetWidth() != GetWrapWidth()))
	{
		//Wrapped to a new width the text can have a different number of lines, so the text area needs a different
		//height and the window must lay its components out again.
		iHeight = msTextSize.y;
		if (mbTextDirty)
		{
			ReadText();
		}
		LayoutText();
		if (msTextSize.y != iHeight)
		{
			mpWindow->LayoutAgain();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Spans or a colour set since the text area was last laid out are drawn.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::Draw(void)
{
	UpdateText();
	return CCanvas::Draw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::UpdateText(void)
{
	if (mbTextDirty)
	{
		ReadText();
		LayoutText();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The width lines are wrapped to, -1 if they aren't.  A text area that
// hasn't been laid out yet has no width so its lines aren't wrapped.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextArea::GetWrapWidth(void)
{
	int32	iWidth;

	iWidth = mcActualBounds.GetWidth();
	if (mbWordWrap && (iWidth > 0))
	{
		return iWidth;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the text from the UTF8 in the colours of the font.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ReadText(void)
{
	CSimpleTextReader		cReader;
	CUTF8					cUTF8;
	CArrayTextFontChange	asChanges;
	Ptr<CFont>				pFont;

	asChanges.Init();
	MakeFontChanges(&asChanges);

	pFont = mpColours->GetFont(msColour);
	cUTF8.Init(mszText.Text());
	cReader.Init(&pFont);
	mpText = cReader.Read(&cUTF8, true, &asChanges);
	cReader.Kill();
	cUTF8.Kill();
	asChanges.Kill();

	mbTextDirty = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Each byte of the text is given the last span added that covers it, then
// a font change is made wherever the colour changes.  Text before the
// first change is in the text area's colour.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::MakeFontChanges(CArrayTextFontChange* pasChanges)
{
	size				uiTextLength;
	size				i;
	size				uiPosition;
	size				uiEnd;
	size*				puiSpans;
	STextAreaSpan*		psSpan;
	ARGB32				sCurrent;
	ARGB32				sColour;
	Ptr<CFont>			pFont;
	STextFontChange*	psChange;

	uiTextLength = mszText.Length();
	if ((masSpans.NumElements() == 0) || (uiTextLength == 0))
	{
		return;
	}

	//Each byte's span plus one, 0 where there is no span.
	puiSpans = (size*)malloc(uiTextLength * sizeof(size));
	memset(puiSpans, 0, uiTextLength * sizeof(size));
	for (i = 0; i < masSpans.NumElements(); i++)
	{
		psSpan = masSpans.Get(i);
		uiEnd = psSpan->uiStart + psSpan->uiLength;
		for (uiPosition = psSpan->uiStart; uiPosition < uiEnd; uiPosition++)
		{
			puiSpans[uiPosition] = i + 1;
		}
	}

	sCurrent = msColour;
	for (uiPosition = 0; uiPosition < uiTextLength; uiPosition++)
	{
		if (puiSpans[uiPosition] == 0)
		{
			sColour = msColour;
		}
		else
		{
			sColour = masSpans.Get(puiSpans[uiPosition] - 1)->sColour;
		}

		if (sColour != sCurrent)
		{
			pFont = mpColours->GetFont(sColour);
			if (pFont.IsNull())
			{
				pFont = mpColours->GetWhite();
			}
			psChange = pasChanges->Add();
			psChange->uiPosition = uiPosition;
			psChange->pcFont = &pFont;
			sCurrent = sColour;
		}
	}
	free(puiSpans);
}


//////////////////////////////////////////////////////////////////////////
//
// Places a sprite for each glyph, wrapped to the actual width if word wrap
// is on, and works out the size they need.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::LayoutText(void)
{
	mpLayout->SetWidth(GetWrapWidth());

	mpFontDraw->Begin();
	if (mpText.IsNotNull())
	{
		mpLayout->Layout(mpFontDraw, mpText);
		msTextSize = mpLayout->GetSize();
	}
	else
	{
		msTextSize.Init(0, 0);
	}
	mpFontDraw->End();

	//Even a text area with no text is a line high.
	if (msTextSize.y < GetFont()->GetLineHeight())
	{
		msTextSize.y = GetFont()->GetLineHeight();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The height of the lines as they were last laid out.  A word wrapped text
// area has no required width (-1), it takes whatever width it is given.
// Otherwise the width is the width of the text from the text layout.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CTextArea::GetRequiredSize(void)
{
	SInt32Vec2	sSize;

	sSize = msTextSize;
	if (mbWordWrap)
	{
		sSize.x = -1;
	}
	return sSize;
}


//////////////////////////////////////////////////////////////////////////
//
// A desired size overrides the required size on each axis it is set on.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CTextArea::GetDesiredSize(void)
{
	SInt32Vec2	sSize;

	sSize = GetRequiredSize();
	if (msDesiredSize.x >= 0)
	{
		sSize.x = msDesiredSize.x;
	}
	if (msDesiredSize.y >= 0)
	{
		sSize.y = msDesiredSize.y;
	}
	return sSize;
}


//////////////////////////////////////////////////////////////////////////
//
// The width of the widest line and the height of all the lines as they
// were last laid out.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CTextArea::GetTextSize(void)
{
	return msTextSize;
}


//////////////////////////////////////////////////////////////////////////
//
// The white font the colours are made from.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CTextArea::GetFont(void)
{
	return mpColours->GetWhite();
}


char* CTextArea::GetText(void) { return mszText.Text(); }
Ptr<CFontColours> CTextArea::GetFontColours(void) { return mpColours; }
bool CTextArea::IsWordWrap(void) { return mbWordWrap; }
ARGB32 CTextArea::GetColour(void) { return msColour; }
size CTextArea::NumSpans(void) { return masSpans.NumElements(); }

