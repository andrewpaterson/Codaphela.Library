#include "BaseLib/UTF8.h"
#include "StandardLib/ClassDefines.h"
#include "SupportLib/SimpleTextReader.h"
#include "MapsCanvasDraw.h"
#include "Window.h"
#include "TextArea.h"


//////////////////////////////////////////////////////////////////////////
//
// szText is UTF8.  Line feeds start new lines.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Init(Ptr<CWindow> pWindow, Ptr<CFont> pFont, const char* szText)
{
	Ptr<CMapsCanvasDraw>	pDraw;

	PreInit();

	mszText.Init(szText);
	mpFont = pFont;
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
	M_Pointer(mpFont);
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
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetText(const char* szText)
{
	mszText.Kill();
	mszText.Init(szText);
	ReadText();
	LayoutText();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetFont(Ptr<CFont> pFont)
{
	mpFont = pFont;
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
// The text is laid out again for the width the text area is given.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	int32	iHeight;

	CCanvas::Layout(sPosition, sAreaSize);

	if (mpLayout->GetWidth() != GetWrapWidth())
	{
		//Wrapped to a new width the text can have a different number of lines, so the text area needs a different
		//height and the window must lay its components out again.
		iHeight = msTextSize.y;
		LayoutText();
		if (msTextSize.y != iHeight)
		{
			mpWindow->LayoutAgain();
		}
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
// Makes the text from the UTF8 with the font.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ReadText(void)
{
	CSimpleTextReader	cReader;
	CUTF8				cUTF8;

	cUTF8.Init(mszText.Text());
	cReader.Init(&mpFont);
	mpText = cReader.Read(&cUTF8, true);
	cReader.Kill();
	cUTF8.Kill();
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
	if (msTextSize.y < mpFont->GetLineHeight())
	{
		msTextSize.y = mpFont->GetLineHeight();
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


char* CTextArea::GetText(void) { return mszText.Text(); }
Ptr<CFont> CTextArea::GetFont(void) { return mpFont; }
bool CTextArea::IsWordWrap(void) { return mbWordWrap; }

