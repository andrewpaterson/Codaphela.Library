/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __COMPONENT_PROPERTIES_H__
#define __COMPONENT_PROPERTIES_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayTemplate.h"


//What a property is edited as, as Logicim's TypeEditorFactory picks an editor by a field's type.  A divider isn't a
//value; it's a titled line between groups of properties, as Logicim's Divider fields.
enum EComponentPropertyType
{
	CPT_Text,
	CPT_Integer,
	CPT_Float,
	CPT_Boolean,
	CPT_Choice,
	CPT_Divider,
};


//One property: its name (as Logicim's field name, which its label is made from), what it's edited as, its value as
//text, and for a choice the choices (the value is the chosen one's text).  Numbers are clamped to their minimum and
//maximum when they're read back, as Logicim's clampProperties.  A float shows its unit after its label.
struct SComponentProperty
{
	CChars					szName;
	CChars					szLabel;
	CChars					szUnit;
	EComponentPropertyType	eType;
	CChars					szValue;
	CArrayChars				aszChoices;
	float64					fMinimum;
	float64					fMaximum;

	void	Init(const char* szName, EComponentPropertyType eType, const char* szValue);
	void	Kill(void);
};


typedef CArrayTemplate<SComponentProperty>	CArrayComponentProperty;


class CFamilyVoltageConfigurationStore;
class CCircuitFonts;
class CCircuitViews;


//What views need to list and make their properties: the families a chip can be of, the fonts views are measured with
//and the project's circuits, which a subcircuit instance's subcircuit is found in.
struct SComponentPropertiesContext
{
	CFamilyVoltageConfigurationStore*	pcFamilies;
	CCircuitFonts*						pcFonts;
	CCircuitViews*						pcCircuits;
};


//A component view's properties, as Logicim's ComponentProperties are read and written by reflection.  C++ has no
//reflection, so each view lists its own: CComponentView::GetProperties adds them (the name, then a chip's family and
//power, then its own, through the class hierarchy) with their current values, and
//CComponentView::InitFromProperties makes a new view from them, as Logicim's ViewFactory makes one from new properties.
//
//Values are text whatever their type, as a dialog's editors hold them; the typed getters parse them, falling back to
//the default given when the property isn't there or doesn't parse.
class CComponentProperties
{
protected:
	CArrayComponentProperty	masProperties;

public:
	void				Init(void);
	void				Kill(void);

	void				AddText(const char* szName, const char* szValue);
	void				AddInteger(const char* szName, int64 iValue, int64 iMinimum, int64 iMaximum);
	void				AddFloat(const char* szName, const char* szUnit, float64 fValue, float64 fMinimum, float64 fMaximum);
	void				AddBoolean(const char* szName, bool bValue);
	void				AddChoice(const char* szName, const char** aszChoices, size uiNumChoices, size uiChosen);
	void				AddChoice(const char* szName, CArrayChars* paszChoices, const char* szChosen);
	void				AddDivider(const char* szName);

	size				NumProperties(void);
	SComponentProperty*	Get(size uiIndex);
	SComponentProperty*	Get(const char* szName);
	bool				SetValue(const char* szName, const char* szValue);

	const char*			GetText(const char* szName, const char* szDefault);
	int64				GetInteger(const char* szName, int64 iDefault);
	float64				GetFloat(const char* szName, float64 fDefault);
	bool				GetBoolean(const char* szName, bool bDefault);
	size				GetChoice(const char* szName, size uiDefault);

protected:
	SComponentProperty*	Add(const char* szName, EComponentPropertyType eType, const char* szValue);
};


void	JavaNameToHumanReadable(CChars* pszDest, const char* szName);


#endif // __COMPONENT_PROPERTIES_H__

