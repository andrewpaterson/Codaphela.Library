#include "BaseLib/UTF8.h"
#include "StandardLib/ClassDefines.h"
#include "MapsCanvasDraw.h"
#include "Window.h"
#include "TextArea.h"


//////////////////////////////////////////////////////////////////////////
//
// szText is UTF8.  Line feeds start new lines.  pWhiteFont's image must
// be white (see CFontColours); the text area makes its own colours of it.
// It is the only style and size the text area has.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Init(Ptr<CWindow> pWindow, Ptr<CFont> pWhiteFont, const char* szText)
{
	Init(pWindow, OMalloc<CFontColours>(pWhiteFont), szText);
}


//////////////////////////////////////////////////////////////////////////
//
// szText is UTF8.  pColours can be shared by text areas so each colour is
// only made once.  It is the only style and size the text area has.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Init(Ptr<CWindow> pWindow, Ptr<CFontColours> pColours, const char* szText)
{
	Init(pWindow, OMalloc<CFontFamily>(pColours), szText);
}


//////////////////////////////////////////////////////////////////////////
//
// szText is UTF8.  Line feeds start new lines.  pFamily can be shared by
// text areas.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Init(Ptr<CWindow> pWindow, Ptr<CFontFamily> pFamily, const char* szText)
{
	Ptr<CMapsCanvasDraw>	pDraw;

	PreInit();

	mszText.Init(szText);
	mpFamily = pFamily;
	msStyle.Init();
	masSpans.Init();
	mbTextDirty = false;
	mbSizeDirty = false;
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
	M_Pointer(mpFamily);
	U_Data(STextStyle, msStyle);
	U_Data(CArrayTextAreaSpan, masSpans);
	U_Bool(mbTextDirty);
	U_Bool(mbSizeDirty);
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
// pWhiteFont's image must be white (see CFontColours).  It becomes the only
// style and size the text area has.
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
	SetFontFamily(OMalloc<CFontFamily>(pColours));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetFontFamily(Ptr<CFontFamily> pFamily)
{
	mpFamily = pFamily;
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
// The style of text that isn't in a span, or where spans don't set a
// property.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetStyle(STextStyle* psStyle)
{
	if (!msStyle.EqualsFont(psStyle))
	{
		mbSizeDirty = true;
	}
	msStyle = *psStyle;
	mbTextDirty = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::GetStyle(STextStyle* psStyle)
{
	*psStyle = msStyle;
}


//////////////////////////////////////////////////////////////////////////
//
// The colour of text that isn't in a span that sets a colour.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetColour(ARGB32 sColour)
{
	if (msStyle.sColour != sColour)
	{
		msStyle.sColour = sColour;
		mbTextDirty = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A span that only sets a colour.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::AddSpan(size uiStart, size uiLength, ARGB32 sColour)
{
	STextStyleOverride	sOverride;

	sOverride.Init();
	sOverride.SetColour(sColour);
	return AddSpan(uiStart, uiLength, &sOverride);
}


//////////////////////////////////////////////////////////////////////////
//
// uiStart and uiLength are in bytes of the UTF8 text.  A span that starts
// or ends inside a character starts or ends at the next character.
// Returns false if the span goes past the end of the text.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::AddSpan(size uiStart, size uiLength, STextStyleOverride* psOverride)
{
	size			uiTextLength;
	STextAreaSpan*	psSpan;

	uiTextLength = mszText.Length();
	if ((uiStart > uiTextLength) || (uiLength > uiTextLength - uiStart))
	{
		return false;
	}
	if ((uiLength == 0) || (psOverride->uiSet == 0))
	{
		return true;
	}

	psSpan = masSpans.Add();
	psSpan->uiStart = uiStart;
	psSpan->uiLength = uiLength;
	psSpan->sOverride = *psOverride;
	mbTextDirty = true;
	if (psOverride->ChangesFont())
	{
		mbSizeDirty = true;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ClearSpans(void)
{
	size	i;

	for (i = 0; i < masSpans.NumElements(); i++)
	{
		if (masSpans.Get(i)->sOverride.ChangesFont())
		{
			mbSizeDirty = true;
		}
	}
	if (masSpans.NumElements() > 0)
	{
		masSpans.ReInit();
		mbTextDirty = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The style of the byte at uiPosition of the UTF8 text: the text area's
// style with every span that covers it applied in the order they were
// added.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::GetStyleAt(size uiPosition, STextStyle* psStyle)
{
	size			i;
	STextAreaSpan*	psSpan;

	*psStyle = msStyle;
	for (i = 0; i < masSpans.NumElements(); i++)
	{
		psSpan = masSpans.Get(i);
		if ((uiPosition >= psSpan->uiStart) && (uiPosition < psSpan->uiStart + psSpan->uiLength))
		{
			psSpan->sOverride.ApplyTo(psStyle);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CTextArea::GetColourAt(size uiPosition)
{
	STextStyle	sStyle;

	GetStyleAt(uiPosition, &sStyle);
	return sStyle.sColour;
}


//////////////////////////////////////////////////////////////////////////
//
// The text is laid out again for the width the text area is given, and
// read again if its spans or style changed.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	SInt32Vec2	sTextSize;

	CCanvas::Layout(sPosition, sAreaSize);

	if (mbTextDirty || (mpLayout->GetWidth() != GetWrapWidth()))
	{
		//Wrapped to a new width, or in new styles, the text can be a different size so the window must lay its
		//components out again.
		sTextSize = msTextSize;
		if (mbTextDirty)
		{
			ReadText();
		}
		LayoutText();
		if ((msTextSize.x != sTextSize.x) || (msTextSize.y != sTextSize.y))
		{
			mpWindow->LayoutAgain();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Spans or a style set since the text area was last laid out are drawn.
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
// Before the size is asked for, spans or a style that can change it are
// applied.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::UpdateSize(void)
{
	if (mbSizeDirty)
	{
		UpdateText();
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
// Makes the text from the UTF8 in the fonts of its styles.  Without a font
// for the text area's style there is no text (the family logs why).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ReadText(void)
{
	CSimpleTextReader		cReader;
	CUTF8					cUTF8;
	CArrayTextFontChange	asChanges;
	Ptr<CFont>				pFont;
	int16					iBaselineShift;

	mbTextDirty = false;
	mbSizeDirty = false;

	pFont = mpFamily->GetRunFont(&msStyle, &iBaselineShift);
	if (pFont.IsNull())
	{
		mpText = NULL;
		return;
	}

	//The reader's font has no baseline shift so a superscript or subscript style needs a change at the start.
	asChanges.Init();
	if (iBaselineShift != 0)
	{
		asChanges.Add()->Init(0, &pFont, iBaselineShift);
	}
	MakeFontChanges(&asChanges, pFont, iBaselineShift);

	cUTF8.Init(mszText.Text());
	cReader.Init(&pFont);
	mpText = cReader.Read(&cUTF8, true, &asChanges);
	cReader.Kill();
	cUTF8.Kill();
	asChanges.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each byte of the text is given the text area's style with the spans
// that cover it applied in the order they were added, then a font change
// is made wherever the style changes.  Text before the first change is in
// pDefaultFont.  A style the family has no font for is drawn in
// pDefaultFont (the family logs an error).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::MakeFontChanges(CArrayTextFontChange* pasChanges, Ptr<CFont> pDefaultFont, int16 iDefaultBaselineShift)
{
	size				uiTextLength;
	size				i;
	size				uiPosition;
	size				uiEnd;
	STextStyle*			psStyles;
	STextAreaSpan*		psSpan;
	STextStyle*			psCurrent;
	Ptr<CFont>			pFont;
	int16				iBaselineShift;

	uiTextLength = mszText.Length();
	if ((masSpans.NumElements() == 0) || (uiTextLength == 0))
	{
		return;
	}

	psStyles = (STextStyle*)malloc(uiTextLength * sizeof(STextStyle));
	for (uiPosition = 0; uiPosition < uiTextLength; uiPosition++)
	{
		psStyles[uiPosition] = msStyle;
	}
	for (i = 0; i < masSpans.NumElements(); i++)
	{
		psSpan = masSpans.Get(i);
		uiEnd = psSpan->uiStart + psSpan->uiLength;
		for (uiPosition = psSpan->uiStart; uiPosition < uiEnd; uiPosition++)
		{
			psSpan->sOverride.ApplyTo(&psStyles[uiPosition]);
		}
	}

	psCurrent = &msStyle;
	for (uiPosition = 0; uiPosition < uiTextLength; uiPosition++)
	{
		if (!psStyles[uiPosition].Equals(psCurrent))
		{
			pFont = mpFamily->GetRunFont(&psStyles[uiPosition], &iBaselineShift);
			if (pFont.IsNull())
			{
				pFont = pDefaultFont;
				iBaselineShift = iDefaultBaselineShift;
			}
			pasChanges->Add()->Init(uiPosition, &pFont, iBaselineShift);
			psCurrent = &psStyles[uiPosition];
		}
	}
	free(psStyles);
}


//////////////////////////////////////////////////////////////////////////
//
// Places a sprite for each glyph, wrapped to the actual width if word wrap
// is on, and works out the size they need.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::LayoutText(void)
{
	Ptr<CFont>	pFont;

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
	pFont = GetFont();
	if (pFont.IsNotNull() && (msTextSize.y < pFont->GetLineHeight()))
	{
		msTextSize.y = pFont->GetLineHeight();
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

	UpdateSize();
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
	UpdateSize();
	return msTextSize;
}


//////////////////////////////////////////////////////////////////////////
//
// The colours of the text area's style's face and size.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFontColours> CTextArea::GetFontColours(void)
{
	return mpFamily->GetColours(msStyle.GetFontStyle(), msStyle.fPixelSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The white font of the text area's style's face and size.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CTextArea::GetFont(void)
{
	Ptr<CFontColours>	pColours;

	pColours = GetFontColours();
	if (pColours.IsNull())
	{
		return NULL;
	}
	return pColours->GetWhite();
}


char* CTextArea::GetText(void) { return mszText.Text(); }
Ptr<CFontFamily> CTextArea::GetFontFamily(void) { return mpFamily; }
bool CTextArea::IsWordWrap(void) { return mbWordWrap; }
ARGB32 CTextArea::GetColour(void) { return msStyle.sColour; }
size CTextArea::NumSpans(void) { return masSpans.NumElements(); }

