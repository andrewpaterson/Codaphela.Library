/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "BaseLib/TypeParser.h"
#include "BaseLib/MarkupTextParser.h"
#include "CoreLib/XMLFile.h"
#include "InputDeviceVariableListener.h"
#include "InputDeviceVariableValue.h"
#include "InputDevices.h"
#include "InputChordReader.h"
#include "InputVirtualDeviceDesc.h"
#include "InputDevicesReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevicesReader::Init(CInputDevices* pcInputDevices, char* szDirectory)
{
	mszDirectory.Init(szDirectory);
	mpcInputDevices = pcInputDevices;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevicesReader::Kill(void)
{
	mszDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadInputDevicesXML(void)
{
	CXMLFile		cXMLFile;
	CMarkupTag*		pcTag;
	CMarkupTag*		pcFormatsTag;
	bool			bResult;
	CTypeNames		cTypeNames;
	STagIterator	sIter;
	CMarkup*		pcMarkup;

	cXMLFile.Init();
	bResult = cXMLFile.Read("InputDevices.xml", mszDirectory.Text());
	if (!bResult)
	{
		cXMLFile.Kill();
		gcLogger.Error("InputDevices.xml not found or could not be parsed");
		return false;
	}
	pcMarkup = &cXMLFile.mcMarkup;

	if (pcMarkup->GetRootTag() == NULL)
	{
		pcMarkup->Kill();
		gcLogger.Error("XML root tag not found");
		return false;
	}

	pcTag = pcMarkup->GetRootTag()->GetTag("Categories");
	if (!pcTag)
	{
		pcMarkup->Kill();
		gcLogger.Error("'Categories' tag not found");
		return false;
	}

	bResult = ReadCategories(pcTag);
	if (!bResult)
	{
		pcMarkup->Kill();
		return false;
	}

	pcFormatsTag = pcMarkup->GetRootTag()->GetTag("Formats", &sIter);
	if (!pcFormatsTag)
	{
		pcMarkup->Kill();
		gcLogger.Error("'Formats' tag not found");
		return false;
	}

	cTypeNames.Init();
	while (pcFormatsTag)
	{
		bResult = ReadDataFormats(pcFormatsTag, &cTypeNames);
		if (!bResult)
		{
			pcMarkup->Kill();
			cTypeNames.Kill();
			return false;
		}
		pcFormatsTag = pcMarkup->GetRootTag()->GetNextTag(&sIter);
	}

	bResult = ReadDevices(pcMarkup->GetRootTag(), &cTypeNames);
	if (!bResult)
	{
		pcMarkup->Kill();
		cTypeNames.Kill();
		return false;
	}

	//pcMarkup->Kill();
	cXMLFile.Kill();
	cTypeNames.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadCategories(CMarkupTag* pcParentTag)
{
	CMarkupTag*		pcCategoryTag;
	CMarkupTag*		pcActionsTag;
	CMarkupTag*		pcActionTag;
	CMarkupTag*		pcTag;
	CChars			szText;
	CChars			szAction;
	STagIterator	sCategoryIter;
	STagIterator	sActionIter;
	CInputCategory*	pcInputCategory;

	pcCategoryTag = pcParentTag->GetTag("Category", &sCategoryIter);
	while (pcCategoryTag)
	{
		pcTag = pcCategoryTag->GetTag("Name");
		if (!pcTag)
		{
			return false;
		}

		szText.Init();
		pcTag->GetText(&szText);

		if (szText.Empty())
		{
			return false;
		}

		if (mpcInputDevices->GetCategory(szText.Text()) != NULL)
		{
			return false;
		}

		pcInputCategory = mpcInputDevices->AddCategory(szText.Text());
		szText.Kill();
		if (!pcInputCategory)
		{
			return false;
		}

		pcActionsTag = pcCategoryTag->GetTag("Generics");
		if (pcActionsTag)
		{
			pcActionTag = pcActionsTag->GetTag("Generic", &sActionIter);
			if (!pcActionTag)
			{
				return false;
			}

			while (pcActionTag)
			{
				szAction.Init();
				pcActionTag->GetText(&szAction);
				pcInputCategory->AddGeneric(szAction.Text());
				szAction.Kill();

				pcActionTag = pcActionsTag->GetNextTag(&sActionIter);
			}
		}

		pcCategoryTag = pcParentTag->GetNextTag(&sCategoryIter);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadDataFormats(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames)
{
	CMarkupTag*			pcTag;
	bool				bResult;
	STagIterator		sIter;

	pcTag = pcParentTag->GetTag("Format", &sIter);
	while (pcTag)
	{
		bResult = ReadNamedDataFormat(pcTag, pcTypeNames);
		if (!bResult)
		{
			return false;
		}
		pcTag = pcParentTag->GetNextTag(&sIter);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadDataFormatChannels(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames, CInputDataFormat* pcFormat)
{
	CMarkupTag*			pcTag;
	char*				szType;
	char*				szName;
	char*				szCount;
	bool				bEmptyName;
	bool				bEmptyCount;
	EPrimitiveType		eType;
	int					iIndex;
	CTextParser			cTextParser;
	int					iCount;
	TRISTATE			tResult;
	int					i;
	CChars				szArrayName;
	STagIterator		sIter;

	pcTag = pcParentTag->GetTag("Channel", &sIter);
	while (pcTag)
	{
		szType = pcTag->GetAttribute("Type");
		szName = pcTag->GetAttribute("Name");
		szCount = pcTag->GetAttribute("Count");

		if (!szType || (strlen(szType) == 0))
		{
			gcLogger.Error("Attribute 'Type' is empty");
			return false;
		}

		eType = pcTypeNames->GetTypeFromPrettyName(szType);
		if (!(((eType >= PT_int32 ) && (eType <= PT_uint64)) || ((eType >= PT_bit ) && (eType <= PT_sixbits))))
		{
			gcLogger.Error("Attribute 'Type' is not a primitive type");
			return false;
		}

		bEmptyName = (!szName || (strlen(szName) == 0));
		if (!bEmptyName)
		{
			iIndex = pcFormat->GetIndex(szName);
			if (iIndex != -1)
			{
				gcLogger.Error2("Data format '", pcFormat->GetCommonName(), "' does not have a name '", szName, "'", NULL);
				return false;
			}
		}

		bEmptyCount = (!szCount || (strlen(szCount) == 0));
		if (bEmptyCount)
		{
			pcFormat->Add(eType, szName);
		}
		else
		{
			cTextParser.Init(szCount);
			tResult = cTextParser.GetInteger(&iCount);
			cTextParser.Kill();
			if (tResult != TRITRUE)
			{
				return false;
			}
			if ((iCount <= 0) || (iCount >= 32768))
			{
				return false;
			}

			for (i = 0; i < iCount; i++)
			{
				szArrayName.Init(szName);
				szArrayName.Append("[");
				szArrayName.Append(i);
				szArrayName.Append("]");
				pcFormat->Add(eType, szArrayName.Text());
				szArrayName.Kill();
			}
		}

		pcTag = pcParentTag->GetNextTag(&sIter);
	}
	pcFormat->Done();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadNamedDataFormat(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames)
{
	char*				szName;
	bool				bEmptyName;
	CInputDataFormat*	pcFormat;

	szName = pcParentTag->GetAttribute("Name");
	bEmptyName = (!szName || (strlen(szName) == 0));
	if (bEmptyName)
	{
		gcLogger.Error("'Format' tag attribute 'Name' is empty");
		return false;
	}

	pcFormat = mpcInputDevices->mcDataFormats.Get(szName);
	if (pcFormat)
	{
		gcLogger.Error2("'Format' named '", szName, "' already exists", NULL);
		return false;
	}

	pcFormat = mpcInputDevices->mcDataFormats.Add(szName);
	return ReadDataFormatChannels(pcParentTag, pcTypeNames, pcFormat);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDataFormat* CInputDevicesReader::ReadDefinedDataFormat(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames)
{
	char*				szType;
	CChars				szName;
	bool				bEmptyType;
	CInputDataFormat*	pcFormat;
	bool				bResult;

	szType = pcParentTag->GetAttribute("Type");
	bEmptyType = (!szType || (strlen(szType) == 0));
	if (bEmptyType)
	{
		gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Type' is empty", NULL);
		return NULL;
	}

	if (strcmp(szType, "Named") == 0)
	{
		szName.Init();
		pcParentTag->GetText(&szName);
		pcFormat = mpcInputDevices->mcDataFormats.Get(szName.Text());
		if (pcFormat)
		{
			szName.Kill();
			return pcFormat;
		}
		gcLogger.Error2("'", pcParentTag->GetName(), "' tag format Named '", szName.Text(), "' not found" , NULL);
		szName.Kill();
		return NULL;
	}
	else if (strcmp(szType, "Defined") == 0)
	{
		pcFormat = mpcInputDevices->mcDataFormats.Add();
		bResult = ReadDataFormatChannels(pcParentTag, pcTypeNames, pcFormat);
		if (bResult)
		{
			return pcFormat;
		}
		return NULL;
	}
	else
	{
		gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Type' invalid.   Only 'Named' and 'Defined' allowed", NULL);
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadDevices(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames)
{
	CMarkupTag*			pcTag;
	bool				bResult;
	char*				szType;
	char*				szPhyscical;
	STagIterator		sIter;
	bool				bPhysical;

	pcTag = pcParentTag->GetTag("Device", &sIter);
	while (pcTag)
	{
		szPhyscical = pcTag->GetAttribute("Physical");
		bPhysical = true;
		if (szPhyscical)
		{
			if (strcmp(szPhyscical, "False") == 0)
			{
				bPhysical = false;
			}
		}

		szType = pcTag->GetAttribute("Type");
		if (!szType || (strlen(szType) == 0))
		{
			gcLogger.Error2("'", pcTag->GetName(), "' tag attribute 'Type' is empty", NULL);
			return false;
		}

		if (strcmp(szType, "Compound") == 0)
		{
			bResult = ReadDevices(pcTag, pcTypeNames);
			if (!bResult)
			{
				return false;
			}
		}
		else
		{
			if (strcmp(szType, "WinRaw") == 0)
			{
			}
			else if (strcmp(szType, "DirectInput") == 0)
			{
			}
			else if (strcmp(szType, "XInput") == 0)
			{
			}
			else
			{
				gcLogger.Error2("'", pcTag->GetName(), "' tag attribute 'Type' invalid.   Only 'WinRaw', 'DirectInput', 'XInput' and 'Compound' allowed", NULL);
				return false;
			}

			bResult = ReadDevice(pcTag, pcTypeNames, bPhysical);
			if (!bResult)
			{
				return false;
			}
		}
		pcTag = pcParentTag->GetNextTag(&sIter);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadDevice(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames, bool bPhysical)
{
	CMarkupTag*					pcCategoryTag;
	CMarkupTag*					pcIDTag;
	CMarkupTag*					pcFriendlyTag;
	CMarkupTag*					pcSourcesTag;
	CMarkupTag*					pcChangesTag;
	CMarkupTag*					pcCommentTag;
	CMarkupTag*					pcVariablesTag;
	bool						bResult;
	CInputDeviceDesc*			pcDeviceDesc;
	CChars						szID;
	CChars						szCategory;
	CChars						szFriendly;
	CInputCategory*				pcCategory;
	STagIterator				sIter;
	int							iSourcesCount;
	CChars						szComment;
	CInputVirtualDeviceDesc*	pcDefaultVirtualDesc;

	pcCategoryTag = pcParentTag->GetTag("Category");
	pcIDTag = pcParentTag->GetTag("ID");
	pcFriendlyTag = pcParentTag->GetTag("Friendly");
	if (!pcCategoryTag)
	{
		gcLogger.Error2("'Category' tag not found in tag '", pcParentTag->GetName(), "'", NULL);
		return false;
	}
	if (!pcIDTag)
	{
		gcLogger.Error2("'ID' tag not found in tag '", pcParentTag->GetName(), "'", NULL);
		return false;
	}
	if (!pcFriendlyTag)
	{
		gcLogger.Error2("'Friendly' tag not found in tag '", pcParentTag->GetName(), "'", NULL);
		return false;
	}

	szID.Init();
	pcIDTag->GetText(&szID);
	if (szID.Empty())
	{
		gcLogger.Error2("'", pcIDTag->GetName(), "' tag in tag '", pcParentTag->GetName(), "' is empty", NULL);
		szID.Kill();
		return false;
	}

	pcDeviceDesc = mpcInputDevices->GetDescription(szID.Text());
	if (pcDeviceDesc != NULL)
	{
		gcLogger.Error2("'", szID.Text(), "' already defined", NULL);
		szID.Kill();
		return false;
	}

	szCategory.Init();
	pcCategoryTag->GetText(&szCategory);
	if (szCategory.Empty())
	{
		gcLogger.Error2("'", pcCategoryTag->GetName(), "' tag in tag '", pcParentTag->GetName(), "' is empty", NULL);
		szID.Kill();
		szCategory.Kill();
		return false;
	}

	pcCategory = mpcInputDevices->GetCategory(szCategory.Text());
	if (pcCategory == NULL)
	{
		gcLogger.Error2("'", szCategory.Text(), "' category in tag '", pcCategoryTag->GetName(), "' not found", NULL);
		szCategory.Kill();
		szID.Kill();
		return false;
	}

	szFriendly.Init();
	pcFriendlyTag->GetText(&szFriendly);
	if (szFriendly.Empty())
	{
		gcLogger.Error2("'", pcFriendlyTag->GetName(), "' tag in tag '", pcParentTag->GetName(), "' is empty", NULL);
		szID.Kill();
		szCategory.Kill();
		szFriendly.Kill();
		return false;
	}

	pcDeviceDesc = mpcInputDevices->CreateDescription(szID.Text(), szFriendly.Text(), pcCategory, bPhysical);
	szID.Kill();
	szCategory.Kill();
	szFriendly.Kill();

	iSourcesCount = pcDeviceDesc->NumInputs();
	pcSourcesTag = pcParentTag->GetTag("Sources", &sIter);
	while (pcSourcesTag)
	{
		iSourcesCount++;
		bResult = ReadSources(pcSourcesTag, pcDeviceDesc, pcTypeNames, false);
		if (!bResult)
		{
			return false;
		}
		pcSourcesTag = pcParentTag->GetNextTag(&sIter);
	}

	if (iSourcesCount == 0)
	{
		gcLogger.Error2("Device '", pcDeviceDesc->GetFriendlyName(), "' has requires at least one 'Sources' tag", NULL);
		return false;
	}

	pcDefaultVirtualDesc = pcDeviceDesc->CreateDefaultVirtualDesc();
	
	pcVariablesTag = pcParentTag->GetTag("Variables", &sIter);
	if (pcVariablesTag)
	{
		bResult = ReadVariables1(pcVariablesTag, pcDeviceDesc);
		if (!bResult)
		{
			return false;
		}
		bResult = ReadVariables2(pcVariablesTag, pcDeviceDesc);
		if (!bResult)
		{
			return false;
		}
	}

	pcChangesTag = pcParentTag->GetTag("Changes", &sIter);
	while (pcChangesTag)
	{
		bResult = ReadChanges(pcChangesTag, pcDeviceDesc);
		if (!bResult)
		{
			return false;
		}
		pcChangesTag = pcParentTag->GetNextTag(&sIter);
	}

	pcCommentTag = pcParentTag->GetTag("Comment");
	if (pcCommentTag)
	{
		szComment.Init();
		pcCommentTag->GetText(&szComment);
		if (szComment.Length() > 0)
		{
			pcDeviceDesc->SetComment(szComment.Text());
		}
		szComment.Kill();
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadChanges(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc)
{
	CMarkupTag*			pcTag;
	bool				bResult;
	STagIterator		sIter;
	char*				szExisting;
	CChars				szReplacement;

	pcTag = pcParentTag->GetTag("Rename", &sIter);
	while (pcTag)
	{
		szExisting = pcTag->GetAttribute("Source");
		if (!szExisting || (strlen(szExisting) == 0))
		{
			gcLogger.Error2("'", pcTag->GetName(), "' tag attribute 'Source' is empty", NULL);
			return false;
		}

		szReplacement.Init();
		pcTag->GetText(&szReplacement);

		if (szReplacement.Empty())
		{
			gcLogger.Error2("'", pcTag->GetName(), "' is empty", NULL);
			szReplacement.Kill();
			return false;
		}

		bResult = pcDeviceDesc->RenameSource(szExisting, szReplacement.Text());

		if (!bResult)
		{
			gcLogger.Error2("'", pcDeviceDesc->GetID(), "' does not have a source called '", szExisting, "'", NULL);
			szReplacement.Kill();
			return false;
		}
		szReplacement.Kill();
		pcTag = pcParentTag->GetNextTag(&sIter);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadSources(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc, CTypeNames* pcTypeNames, bool bDump)
{
	CMarkupTag*					pcTag;
	bool						bResult;
	char*						szAttribute;
	CInputDataFormat*			pcDataFormat;
	CMarkupTag*					pcFormatTag;
	STagIterator				sIter;
	CChars						szNamedID;
	CInputDeviceDesc*			pcSourceDesc;
	CInputDeviceCopyContext		cCopyContext;
	CChars						szDump;

	szAttribute = pcParentTag->GetAttribute("Type");
	if (!szAttribute || (strlen(szAttribute) == 0))
	{
		gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Type' is empty", NULL);
		return false;
	}

	if (strcmp(szAttribute, "Defined") == 0)
	{
		pcFormatTag = pcParentTag->GetTag("Format");
		if (!pcFormatTag)
		{
			gcLogger.Error2("'Format' tag not found in tag '", pcParentTag->GetName(), "' for 'Defined'", NULL);
			return false;
		}

		pcDataFormat = ReadDefinedDataFormat(pcFormatTag, pcTypeNames);
		if (!pcDataFormat)
		{
			return false;
		}

		if (bDump)
		{
			szDump.Init("    <Single Name=\"Virtual\" Device=\"");
			szDump.Append(pcDeviceDesc->GetID());
			szDump.Append("\" Number=\"Agnostic\">\n");
			szDump.Dump();
			szDump.Kill();
		}

		pcDeviceDesc->SetDataFormat(pcDataFormat);
		pcTag = pcParentTag->GetTag("Source", &sIter);
		while (pcTag)
		{
			bResult = ReadSource(pcTag, pcDeviceDesc, bDump);
			if (!bResult)
			{
				return false;
			}
			pcTag = pcParentTag->GetNextTag(&sIter);
		}
		
		if (bDump)
		{
			szDump.Init("    </Single>\n");
			szDump.Dump();
			szDump.Kill();
		}
		return true;
	}
	else if (strcmp(szAttribute, "Named") == 0)
	{
		szNamedID.Init();
		pcParentTag->GetText(&szNamedID);
		if (szNamedID.Empty())
		{
			gcLogger.Error2("'", pcParentTag->GetName(), "' tag is empty for 'Named'", NULL);
			szNamedID.Kill();
			return false;
		}
		pcSourceDesc = mpcInputDevices->GetDescription(szNamedID.Text());
		if (pcSourceDesc)
		{
			if (pcDeviceDesc->GetDataFormat() == NULL)
			{
				pcDeviceDesc->SetDataFormat(pcSourceDesc->GetDataFormat());
			}
			else if (pcDeviceDesc->GetDataFormat() != pcSourceDesc->GetDataFormat())
			{
				gcLogger.Error("The devices current data format is different to the new sources data format");
				return false;
			}

			cCopyContext.Init(pcSourceDesc, pcDeviceDesc);
			pcDeviceDesc->CopySources(&cCopyContext);
			szNamedID.Kill();
			return true;
		}
		else
		{
			gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Named' did not contain a valid device ID [", szNamedID.Text(), "] while parsing [", pcDeviceDesc->GetID(), "]", NULL);
			return false;
		}
	}
	else
	{
		gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Type' invalid.   Only 'Named' and 'Defined' allowed", NULL);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadSource(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc, bool bDump)
{
	char*					szAttribute;
	EInputSourceType		eType;
	CMarkupTag*				pcRestTag;
	CMarkupTag*				pcFriendlyTag;
	CMarkupTag*				pcTag;
	CMarkupTag*				pcGenericTag;
	CChars					szFriendly;
	CInputSourceDesc*		pcSourceDesc;
	bool					bResult;
	float		 			fValue;
	char*					szType;
	char*					szEmit;
	bool					bEmitRestEvent;
	bool					bHasRestValue;
	CChars					szGeneric;
	STagIterator			sIter;
	CInputCategoryGeneric*	pcGeneric;
	CChars					szDump;

	szAttribute = pcParentTag->GetAttribute("Type");
	if (!szAttribute || (strlen(szAttribute) == 0))
	{
		gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Type' is empty", NULL);
		return false;
	}

	if (strcmp(szAttribute, "State") == 0)
	{
		eType = ISET_State;
	}
	else if (strcmp(szAttribute, "Delta") == 0)
	{
		eType = ISET_Delta;
	}
	else
	{
		gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Type' invalid.   Only 'State' and 'Delta' allowed", NULL);
		return false;
	}

	pcFriendlyTag = pcParentTag->GetTag("Friendly");
	if (pcFriendlyTag == NULL)
	{
		gcLogger.Error2("'Friendly' tag not found in tag '", pcParentTag->GetName(), "'", NULL);
		return false;
	}
	szFriendly.Init();
	pcFriendlyTag->GetText(&szFriendly);
	if (szFriendly.Empty())
	{
		szFriendly.Kill();
		return NULL;
	}

	fValue = 0;
	bEmitRestEvent = false;
	bHasRestValue = true;
	pcRestTag = pcParentTag->GetTag("Rest");
	if (pcRestTag)
	{
		szType = pcRestTag->GetAttribute("Type");
		if (!szType || (strlen(szType) == 0))
		{
			szFriendly.Kill();
			return false;
		}

		if (strcmp(szType, "Numeric") == 0)
		{
			bResult = CMarkupTextParser::ReadFloat(pcRestTag, &fValue);
			if (!bResult)
			{
				szFriendly.Kill();
				return false;
			}

			if ((fValue < 0.0f) || (fValue > 1.0f))
			{
				gcLogger.Error("'Rest' tag with numeric value must be in the range 0.0f to 1.0f");
				szFriendly.Kill();
				return false;
			}
		}
		else if (strcmp(szType, "None") == 0)
		{
			bHasRestValue = false;
		}
		else
		{
			szFriendly.Kill();
			return false;
		}

		szEmit = pcRestTag->GetAttribute("Emit");
		if (szEmit)
		{
			if (strcmp(szEmit, "True") == 0)
			{
				bEmitRestEvent = true;
			}
			else if (strcmp(szEmit, "False") == 0)
			{
				bEmitRestEvent = false;
			}
			else
			{
				szFriendly.Kill();
				return false;
			}
		}
	}
	if ((fValue > 1.0) || (fValue < 0.0f))
	{
		szFriendly.Kill();
		return false;
	}

	pcSourceDesc = pcDeviceDesc->AddInput(eType, szFriendly.Text());
	szFriendly.Kill();

	pcGenericTag = pcParentTag->GetTag("Generic", &sIter);
	while (pcGenericTag)
	{
		szGeneric.Init();
		pcGenericTag->GetText(&szGeneric);
		if (!pcSourceDesc->GetDeviceDesc()->GetCategory()->ContainsGeneric(szGeneric.Text()))
		{
			gcLogger.Error2("'", pcSourceDesc->GetDeviceDesc()->GetFriendlyName(), "' device does not contain category action '",  szGeneric.Text(), "'", NULL);
			szGeneric.Kill();
			return false;
		}
		pcGeneric = pcDeviceDesc->GetCategory()->GetGeneric(szGeneric.Text());
		pcSourceDesc->GetGenerics()->Add(&pcGeneric);
		szGeneric.Kill();

		pcGenericTag = pcParentTag->GetNextTag(&sIter);
	}

	pcTag = pcParentTag->GetTag(szAttribute, &sIter);
	if (pcTag == NULL)
	{
		return false;
	}

	if (eType == ISET_Delta)
	{
		bResult = ReadSourceValue(pcTag, pcSourceDesc);
		if (!bResult)
		{
			return false;
		}
	}
	else
	{
		while (pcTag)
		{
			bResult = ReadSourceValue(pcTag, pcSourceDesc);
			if (!bResult)
			{
				return false;
			}
			pcTag = pcParentTag->GetNextTag(&sIter);
		}
	}

	pcSourceDesc->SetRest(fValue, bEmitRestEvent, bHasRestValue);

	if (bDump)
	{
		szDump.Init("      <Source Friendly=\"");
		szDump.Append(pcSourceDesc->GetFriendlyName());
		szDump.Append("\"/>\n");
		szDump.Dump();
		szDump.Kill();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadSourceValue(CMarkupTag* pcParentTag, CInputSourceDesc* pcSourceDesc)
{
	CMarkupTag*				pcTag;
	CMarkupTag*				pcOrderTag;
	CInputSourceValue*		pcSourceValue;
	CMarkupTag*				pcValueTag;
	char*					szType;
	CChars					szChannel;
	int						iIndex;
	float		 			fValue;
	bool					bResult;
	int						iOrder;
	STagIterator			sIter;

	pcValueTag = pcParentTag->GetTag("Value");
	if (pcValueTag == NULL)
	{
		return false;
	}

	szType = pcValueTag->GetAttribute("Type");
	if (!szType || (strlen(szType) == 0))
	{
		return false;
	}

	if (strcmp(szType, "Channel") == 0)
	{
		szChannel.Init();
		pcValueTag->GetText(&szChannel);
		iIndex = pcSourceDesc->GetDeviceDesc()->GetDataFormat()->GetIndex(szChannel.Text());
		szChannel.Kill();
		if (iIndex == -1)
		{
			return false;
		}
		pcSourceValue = pcSourceDesc->AddValue(iIndex);
	}
	else if (strcmp(szType, "Numeric") == 0)
	{
		bResult = CMarkupTextParser::ReadFloat(pcValueTag, &fValue);
		if (!bResult)
		{
			return false;
		}

		if ((fValue < 0.0f) || (fValue > 1.0f))
		{
			gcLogger.Error("'Value' tag with numeric value must be in the range 0.0f to 1.0f");
			return false;
		}

		pcSourceValue = pcSourceDesc->AddValue(fValue);
	}
	else
	{
		return false;
	}

	if (pcSourceDesc->GetType() == ISET_State)
	{
		bResult = ReadStateDetail(pcParentTag, pcSourceValue);
		if (!bResult)
		{
			return false;
		}
	}
	else if (pcSourceDesc->GetType() == ISET_Delta)
	{
		bResult = ReadDeltaDetail(pcParentTag, pcSourceValue);
		if (!bResult)
		{
			return false;
		}
	}

	pcOrderTag = pcParentTag->GetTag("Order");
	if (pcOrderTag)
	{
		bResult = CMarkupTextParser::ReadInteger(pcOrderTag, &iOrder);
		if (!bResult)
		{
			return false;
		}
		pcSourceValue->SetOrder(iOrder);
	}

	pcTag = pcParentTag->GetTag("Channel", &sIter);
	while (pcTag)
	{
		bResult = ReadValueChannel(pcTag, pcSourceValue);
		if (!bResult)
		{
			return false;
		}
		pcTag = pcParentTag->GetNextTag(&sIter);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadStateDetail(CMarkupTag* pcParentTag, CInputSourceValue* pcSourceValue)
{
	CMarkupTag*		pcMaxTag;
	CMarkupTag*		pcMinTag;
	float			fMax;
	float			fMin;
	bool			bResult;
	bool			bMin;
	bool			bMax;

	pcMaxTag = pcParentTag->GetTag("Max");
	pcMinTag = pcParentTag->GetTag("Min");

	fMin = 0.0f;
	fMax = 1.0f;

	bMax = false;
	bMin = false;
	if (pcMaxTag)
	{
		bResult = CMarkupTextParser::ReadFloat(pcMaxTag, &fMax);
		if (!bResult)
		{
			return false;
		}
		bMax = true;
	}

	if (pcMinTag)
	{
		bResult = CMarkupTextParser::ReadFloat(pcMinTag, &fMin);
		if (!bResult)
		{
			return false;
		}
		bMin = true;
	}

	if (pcSourceValue->GetValueType() == ISVT_Channel)
	{
		if (!(bMax && bMin))
		{
			gcLogger.Error2("'", pcParentTag->GetName(), "' has a channel value but no 'Min' and 'Max' tags", NULL);
			return false;
		}
	}

	pcSourceValue->SetStateDetail(fMax, fMin);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadDeltaDetail(CMarkupTag* pcParentTag, CInputSourceValue* pcSourceValue)
{
	CMarkupTag*		pcSensitivityTag;
	CMarkupTag*		pcOffsetTag;
	float			fSensitivity;
	float			fOffset;
	bool			bResult;

	pcSensitivityTag = pcParentTag->GetTag("Sensitivity");
	pcOffsetTag = pcParentTag->GetTag("Offset");

	fSensitivity = 1.0f;
	if (pcSensitivityTag)
	{
		bResult = CMarkupTextParser::ReadFloat(pcSensitivityTag, &fSensitivity);
		if (!bResult)
		{
			return false;
		}
	}

	fOffset = 0.0f;
	if (pcOffsetTag)
	{
		bResult = CMarkupTextParser::ReadFloat(pcOffsetTag, &fOffset);
		if (!bResult)
		{
			return false;
		}
	}

	pcSourceValue->SetDeltaDetail(fSensitivity, fOffset);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadValueChannel(CMarkupTag* pcParentTag, CInputSourceValue* pcSourceValue)
{
	char*							szName;
	int								iIndex;
	CMarkupTag*						pcEquals;
	CMarkupTag*						pcLessThan;
	CMarkupTag*						pcLessThanOrEquals;
	CMarkupTag*						pcGreaterThan;
	CMarkupTag*						pcGreaterThanOrEquals;
	CMarkupTag*						pcNotEquals;
	CMarkupTag*						pcAlwaysValid;
	CMarkupTag*						pcValid;
	int								iNumComparators;
	EInputSourceChannelComparator	eComparator;
	CInputSouceChannel*				pcChannel;
	CChars							szTestValue;
	CTypeParser						cTypeParser;
	CChannel*						pcExternalChannel;
	char							vTest[8];
	TRISTATE						tResult;

	szName = pcParentTag->GetAttribute("Name");

	if ((!szName || (strlen(szName) == 0)))
	{
		return false;
	}

	iIndex = pcSourceValue->GetDataFormat()->GetIndex(szName);
	if (iIndex == -1)
	{
		return false;
	}

	pcEquals = pcParentTag->GetTag("Equals");
	pcLessThan = pcParentTag->GetTag("LessThan");
	pcLessThanOrEquals = pcParentTag->GetTag("LessThanOrEquals");
	pcGreaterThan = pcParentTag->GetTag("GreaterThan");
	pcGreaterThanOrEquals = pcParentTag->GetTag("GreaterThanOrEquals");
	pcNotEquals = pcParentTag->GetTag("NotEquals");
	pcAlwaysValid = pcParentTag->GetTag("AlwaysValid");

	iNumComparators = 0;
	eComparator = ISCC_Unknown;
	pcValid = NULL;
	if (pcEquals)
	{
		pcValid = pcEquals;
		iNumComparators++;
		eComparator = ISCC_Equals;
	}
	if (pcLessThan)
	{
		pcValid = pcLessThan;
		iNumComparators++;
		eComparator = ISCC_LessThan;
	}
	if (pcLessThanOrEquals)
	{
		pcValid = pcLessThanOrEquals;
		iNumComparators++;
		eComparator = ISCC_LessThanEquals;
	}
	if (pcGreaterThan)
	{
		pcValid = pcGreaterThan;
		iNumComparators++;
		eComparator = ISCC_GreaterThan;
	}
	if (pcGreaterThanOrEquals)
	{
		pcValid = pcGreaterThanOrEquals;
		iNumComparators++;
		eComparator = ISCC_GreaterThanEquals;
	}
	if (pcNotEquals)
	{
		pcValid = pcNotEquals;
		iNumComparators++;
		eComparator = ISCC_NotEquals;
	}	
	if (pcAlwaysValid)
	{
		pcValid = pcAlwaysValid;
		iNumComparators++;
		eComparator = ISCC_AlwaysValid;
	}

	if ((eComparator == ISCC_Unknown) || (iNumComparators > 1))
	{
		return false;
	}

	vTest[0] = vTest[1] = vTest[2] = vTest[3] = vTest[4] = vTest[5] = vTest[6] = vTest[7] = 0;

	pcExternalChannel = pcSourceValue->GetDataFormat()->Get(iIndex);
	szTestValue.Init();
	pcValid->GetText(&szTestValue);
	cTypeParser.Init(&szTestValue);

	if (eComparator != ISCC_AlwaysValid)
	{
		tResult = cTypeParser.Parse(pcExternalChannel->eType, vTest, 8);
		if (tResult != TRITRUE)
		{
			szTestValue.Kill();
			return false;
		}
	}

	pcChannel = pcSourceValue->AddChannel(iIndex, eComparator, vTest);
	szTestValue.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadVariables1(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc)
{
	CMarkupTag*					pcTag;
	STagIterator				sIter;
	char*						szName;
	CInputDeviceVariableDesc*	pcVariable;
	CMarkupTag*					pcValueTag;
	STagIterator				sValueIter;
	bool						bResult;
	CChars						szNamedID;
	CInputDeviceDesc*			pcSourceDesc;
	char*						szAttribute;
	int							iInitialValue;
	
	szAttribute = pcParentTag->GetAttribute("Type");
	if (!szAttribute || (strlen(szAttribute) == 0))
	{
		gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Type' is empty", NULL);
		return false;
	}

	if (strcmp(szAttribute, "Defined") == 0)
	{
		pcTag = pcParentTag->GetTag("Variable", &sIter);
		while (pcTag)
		{
			szName = pcTag->GetAttribute("Name");
			if (!szName || (strlen(szName) == 0))
			{
				gcLogger.Error2("'", pcTag->GetName(), "' variable tag attribute 'Name' is empty", NULL);
				return false;
			}

			pcVariable = pcDeviceDesc->GetVariable(szName);
			if (pcVariable)
			{
				gcLogger.Error2("Variable '", szName, "' already exists on device '", pcDeviceDesc->GetFriendlyName(), "'", NULL);
				return false;
			}

			pcVariable = pcDeviceDesc->AddVariable(szName);
			pcValueTag = pcTag->GetTag("Value", &sValueIter);

			if (!pcValueTag)
			{
				gcLogger.Error2("Variable '", szName, "' on device '", pcDeviceDesc->GetFriendlyName(), "' has no values", NULL);
				return false;
			}

			iInitialValue = 0;
			while (pcValueTag)
			{
				bResult = ReadVariableValue1(pcValueTag, pcVariable, &iInitialValue);
				if (!bResult)
				{
					return false;
				}

				pcValueTag = pcTag->GetNextTag(&sValueIter);
			}
			pcTag = pcParentTag->GetNextTag(&sIter);
		}
		return true;
	}
	else if (strcmp(szAttribute, "Named") == 0)
	{
		szNamedID.Init();
		pcParentTag->GetText(&szNamedID);
		if (szNamedID.Empty())
		{
			gcLogger.Error2("'", pcParentTag->GetName(), "' tag is empty for 'Named'", NULL);
			szNamedID.Kill();
			return false;
		}
		pcSourceDesc = mpcInputDevices->GetDescription(szNamedID.Text());
		if (pcSourceDesc)
		{
			szNamedID.Kill();
			return true;
		}
		else
		{
			szNamedID.Kill();
			gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Named' did not contain a valid device ID [", szNamedID.Text(), "] while parsing [", pcDeviceDesc->GetID(), "]", NULL);
			return false;
		}
	}
	else
	{
		gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Type' invalid.   Only 'Named' and 'Defined' allowed", NULL);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadVariables2(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc)
{
	CMarkupTag*					pcTag;
	STagIterator				sIter;
	char*						szName;
	CInputDeviceVariableDesc*	pcVariable;
	CMarkupTag*					pcValueTag;
	STagIterator				sValueIter;
	bool						bResult;
	char*						szAttribute;
	CChars						szNamedID;
	CInputDeviceDesc*			pcSourceDesc;
	CInputDeviceCopyContext		cCopyContext;

	szAttribute = pcParentTag->GetAttribute("Type");
	if (strcmp(szAttribute, "Defined") == 0)
	{
		pcTag = pcParentTag->GetTag("Variable", &sIter);
		while (pcTag)
		{
			szName = pcTag->GetAttribute("Name");
			pcVariable = pcDeviceDesc->GetVariable(szName);
			pcValueTag = pcTag->GetTag("Value", &sValueIter);

			while (pcValueTag)
			{
				bResult = ReadVariableValue2(pcValueTag, pcVariable, pcDeviceDesc->GetVariableChordDescs());
				if (!bResult)
				{
					return false;
				}

				pcValueTag = pcTag->GetNextTag(&sValueIter);
			}
			pcTag = pcParentTag->GetNextTag(&sIter);
		}
		return true;
	}
	else if (strcmp(szAttribute, "Named") == 0)
	{
		szNamedID.Init();
		pcParentTag->GetText(&szNamedID);

		pcSourceDesc = mpcInputDevices->GetDescription(szNamedID.Text());

		cCopyContext.Init(pcSourceDesc, pcDeviceDesc);
		CopyVirtualDeviceDescChords(&cCopyContext);
		pcDeviceDesc->CopyVariables(&cCopyContext);
		cCopyContext.Kill();
		szNamedID.Kill();
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevicesReader::CopyVirtualDeviceDescChords(CInputDeviceCopyContext* pcContext)
{
	pcContext->mpcDestDeviceDesc->GetVariableChordDescs()->CopyChordDescs(pcContext->mpcSourceDeviceDesc->GetVariableChordDescs(), pcContext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadVariableValue1(CMarkupTag* pcValueTag, CInputDeviceVariableDesc* pcVariable, int* piInitialValue)
{
	char*						szValueName;
	char*						szInitial;

	szValueName = pcValueTag->GetAttribute("Name");
	if ((szValueName == NULL) || (strlen(szValueName) == 0))
	{
		gcLogger.Error("'Value' tag attribute 'Name' is empty");
		return false;
	}

	szInitial = pcValueTag->GetAttribute("Initial");
	if (szInitial)
	{
		if (strcmp("True", szInitial) == 0)
		{
			(*piInitialValue)++;
			if ((*piInitialValue) > 1)
			{
				gcLogger.Error("Only one initial value is allowed per variable");
				return false;
			}
		}
		else if (strcmp("False", szInitial) == 0)
		{
		}
		else
		{
			gcLogger.Error2("'", pcValueTag->GetName(), "' variable tag attribute 'Initial' only allows 'True' and 'False'", NULL);
			return false;
		}
	}

	pcVariable->AddValue(szValueName);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadVariableValue2(CMarkupTag* pcValueTag, CInputDeviceVariableDesc* pcVariable, CInputChordDescs* pcChordDescs)
{
	CInputChordReader				cChordReader;
	bool							bResult;
	STagIterator					sIter;
	CMarkupTag*						pcValueVariableTag;
	char*							szValueName;
	CInputDeviceVariableValueDesc*	pcVariableValueDesc;
	char*							szInitial;

	szValueName = pcValueTag->GetAttribute("Name");
	pcVariableValueDesc = pcVariable->GetValue(szValueName);

	szInitial = pcValueTag->GetAttribute("Initial");
	if (szInitial)
	{
		if (strcmp("True", szInitial) == 0)
		{
			pcVariableValueDesc->SetInitial(true);
		}
	}
	
	cChordReader.Init(pcValueTag, pcChordDescs, INPUT_DEVICE_VARIABLE_ACTION, pcVariable->GetDeviceDesc());
	bResult = cChordReader.Read();
	if (!bResult)
	{
		cChordReader.Kill();
		return false;
	}

	if (cChordReader.mpcReadChordDesc)
	{
		pcVariableValueDesc->AddCondition(cChordReader.mpcReadChordDesc);
	}
	cChordReader.Kill();

	pcValueVariableTag = pcValueTag->GetTag("Variable", &sIter);
	while (pcValueVariableTag)
	{
		bResult = ReadVariableValueVariable(pcValueVariableTag, pcVariableValueDesc);
		if (!bResult)
		{
			return false;
		}
		pcValueVariableTag = pcValueTag->GetNextTag(&sIter);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevicesReader::ReadVariableValueVariable(CMarkupTag* pcTag, CInputDeviceVariableValueDesc* pcVariableValueDesc)
{
	char*							szName;
	char*							szEquals;
	CInputDeviceVariableDesc*		pcConditionVariableDesc;
	CInputDeviceVariableValueDesc*	pcConditionValueDesc;

	szName = pcTag->GetAttribute("Name");
	if ((szName == NULL) || (strlen(szName) == 0))
	{
		gcLogger.Error("'Variable' tag attribute 'Name' is empty");
		return false;
	}

	szEquals = pcTag->GetAttribute("Equals");
	if ((szEquals == NULL) || (strlen(szEquals) == 0))
	{
		gcLogger.Error("'Variable' tag attribute 'Equals' is empty");
		return false;
	}

	pcConditionVariableDesc = pcVariableValueDesc->GetVariableDesc()->GetDeviceDesc()->GetVariable(szName);
	if (!pcConditionVariableDesc)
	{
		gcLogger.Error2("Could not find variable '", szName, "' for condition in variable '", pcVariableValueDesc->GetVariableDesc()->GetName(), "'", NULL);
		return false;
	}

	pcConditionValueDesc = pcConditionVariableDesc->GetValue(szEquals);
	if (!pcConditionValueDesc)
	{
		gcLogger.Error2("Variable '", szName, "' does not allow value '", szEquals, "' for condition Equals", NULL);
		return false;
	}

	pcVariableValueDesc->AddCondition(pcConditionValueDesc);
	return true;
}

