#include "BaseLib/UTF8.h"
#include "StandardLib/ClassDefines.h"
#include "SupportLib/SimpleTextReader.h"
#include "MapsCanvasDraw.h"
#include "Label.h"


//////////////////////////////////////////////////////////////////////////
//
// szText is UTF8.  Line feeds are left out, a label is one line.
//
//////////////////////////////////////////////////////////////////////////
void CLabel::Init(Ptr<CWindow> pWindow, Ptr<CFont> pFont, const char* szText)
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
	msRequiredSize.Init(0, 0);

	//The text is drawn over whatever is behind the label.
	pDraw = OMalloc<CMapsCanvasDraw>(mpMaps);
	CCanvas::Init(pWindow, pDraw);
	SetTransparent(true);

	LayoutText();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLabel::Free(void)
{
	mszText.Kill();
	CCanvas::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLabel::Class(void)
{
	CCanvas::Class();

	U_String(mszText);
	M_Pointer(mpFont);
	M_Pointer(mpText);
	M_Pointer(mpLayout);
	M_Pointer(mpSpriteMap);
	M_Pointer(mpFontDraw);
	M_Pointer(mpMaps);
	U_2Int32(msRequiredSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLabel::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLabel::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLabel::SetText(const char* szText)
{
	mszText.Kill();
	mszText.Init(szText);
	LayoutText();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLabel::SetFont(Ptr<CFont> pFont)
{
	mpFont = pFont;
	LayoutText();
}


//////////////////////////////////////////////////////////////////////////
//
// Places a sprite for each glyph and works out the size they need.
//
//////////////////////////////////////////////////////////////////////////
void CLabel::LayoutText(void)
{
	CSimpleTextReader	cReader;
	CUTF8				cUTF8;

	cUTF8.Init(mszText.Text());
	cReader.Init(&mpFont);
	mpText = cReader.Read(&cUTF8, false);
	cReader.Kill();
	cUTF8.Kill();

	mpFontDraw->Begin();
	if (mpText.IsNotNull())
	{
		mpLayout->Layout(mpFontDraw, mpText);
		msRequiredSize.x = mpLayout->GetSize().x;
	}
	else
	{
		msRequiredSize.x = 0;
	}
	mpFontDraw->End();

	//Even a label with no text is a line high.
	msRequiredSize.y = mpFont->GetLineHeight();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CLabel::GetRequiredSize(void)
{
	return msRequiredSize;
}


//////////////////////////////////////////////////////////////////////////
//
// A desired size overrides the required size on each axis it is set on.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CLabel::GetDesiredSize(void)
{
	SInt32Vec2	sSize;

	sSize = msRequiredSize;
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


char* CLabel::GetText(void) { return mszText.Text(); }
Ptr<CFont> CLabel::GetFont(void) { return mpFont; }
