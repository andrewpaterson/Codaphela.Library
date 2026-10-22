#ifndef __GERBER_PANEL_FILES_H__
#define __GERBER_PANEL_FILES_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "GerberPanelOutlines.h"
#include "GerberPanelTabs.h"
#include "GerberPanelEdgeCuts.h"


//Writes the fabrication files of a packed panel: every Gerber layer merged, the edge cuts with the tabs, the plated
//drills merged and the non-plated drills merged with the mouse bites.  Files are named after the panel with each
//board file's KiCad suffix, eg: "Panel-F_Cu.gtl", "Panel-Edge_Cuts.gm1", "Panel-NPTH.drl".
class CGerberPanelFiles
{
protected:
	CGerberPanel*			mpcPanel;
	CGerberPanelOutlines	mcOutlines;
	CGerberPanelTabs		mcTabs;
	CGerberPanelEdgeCuts	mcEdgeCuts;
	CArrayChars				maszFiles;  //The names of the files written.

public:
	void					Init(CGerberPanel* pcPanel);
	void					Kill(void);

	bool					Calculate(void);
	bool					Write(const char* szDirectory, const char* szName);

	CGerberPanelOutlines*	GetOutlines(void);
	CGerberPanelTabs*		GetTabs(void);
	CGerberPanelEdgeCuts*	GetEdgeCuts(void);
	size					NumFiles(void);
	char*					GetFile(size uiIndex);

protected:
	bool					WriteFile(const char* szDirectory, const char* szName, const char* szSuffix, CChars* pszText);
	void					GetSuffix(const char* szFileName, CChars* pszSuffix);
	bool					WriteLayers(const char* szDirectory, const char* szName);
	bool					WriteDrills(const char* szDirectory, const char* szName, EGerberDrillPlating ePlating, const char* szDefaultSuffix);
};


#endif // __GERBER_PANEL_FILES_H__
