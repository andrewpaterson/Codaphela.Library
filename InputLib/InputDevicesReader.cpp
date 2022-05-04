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
BOOL CInputDevicesReader::ReadInputDevicesXML(void)
{
	CXMLFile		cXMLFile;
	CMarkupTag*		pcTag;
	CMarkupTag*		pcFormatsTag;
	BOOL			bResult;
	CTypeNames		cTypeNames;
	STagIterator	sIter;
	CMarkup*		pcMarkup;

	cXMLFile.Init();
	bResult = cXMLFile.Read("InputDevices.xml", mszDirectory.Text());
	if (!bResult)
	{
		cXMLFile.Kill();
		gcLogger.Error("InputDevices.xml not found or could not be parsed");
		return FALSE;
	}
	pcMarkup = &cXMLFile.mcMarkup;

	if (pcMarkup->GetRootTag() == NULL)
	{
		pcMarkup->Kill();
		gcLogger.Error("XML root tag not found");
		return FALSE;
	}

	pcTag = pcMarkup->GetRootTag()->GetTag("Categories");
	if (!pcTag)
	{
		pcMarkup->Kill();
		gcLogger.Error("'Categories' tag not found");
		return FALSE;
	}

	bResult = ReadCategories(pcTag);
	if (!bResult)
	{
		pcMarkup->Kill();
		return FALSE;
	}

	pcFormatsTag = pcMarkup->GetRootTag()->GetTag("Formats", &sIter);
	if (!pcFormatsTag)
	{
		pcMarkup->Kill();
		gcLogger.Error("'Formats' tag not found");
		return FALSE;
	}

	cTypeNames.Init();
	while (pcFormatsTag)
	{
		bResult = ReadDataFormats(pcFormatsTag, &cTypeNames);
		if (!bResult)
		{
			pcMarkup->Kill();
			cTypeNames.Kill();
			return FALSE;
		}
		pcFormatsTag = pcMarkup->GetRootTag()->GetNextTag(&sIter);
	}

	bResult = ReadDevices(pcMarkup->GetRootTag(), &cTypeNames);
	if (!bResult)
	{
		pcMarkup->Kill();
		cTypeNames.Kill();
		return FALSE;
	}

	//pcMarkup->Kill();
	cXMLFile.Kill();
	cTypeNames.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadCategories(CMarkupTag* pcParentTag)
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
			return FALSE;
		}

		szText.Init();
		pcTag->GetText(&szText);

		if (szText.Empty())
		{
			return FALSE;
		}

		if (mpcInputDevices->GetCategory(szText.Text()) != NULL)
		{
			return FALSE;
		}

		pcInputCategory = mpcInputDevices->AddCategory(szText.Text());
		szText.Kill();
		if (!pcInputCategory)
		{
			return FALSE;
		}

		pcActionsTag = pcCategoryTag->GetTag("Generics");
		if (pcActionsTag)
		{
			pcActionTag = pcActionsTag->GetTag("Generic", &sActionIter);
			if (!pcActionTag)
			{
				return FALSE;
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
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadDataFormats(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames)
{
	CMarkupTag*			pcTag;
	BOOL				bResult;
	STagIterator		sIter;

	pcTag = pcParentTag->GetTag("Format", &sIter);
	while (pcTag)
	{
		bResult = ReadNamedDataFormat(pcTag, pcTypeNames);
		if (!bResult)
		{
			return FALSE;
		}
		pcTag = pcParentTag->GetNextTag(&sIter);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadDataFormatChannels(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames, CInputDataFormat* pcFormat)
{
	CMarkupTag*			pcTag;
	char*				szType;
	char*				szName;
	char*				szCount;
	BOOL				bEmptyName;
	BOOL				bEmptyCount;
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
			return FALSE;
		}

		eType = pcTypeNames->GetTypeFromPrettyName(szType);
		if (!(((eType >= PT_int32 ) && (eType <= PT_uint64)) || ((eType >= PT_bit ) && (eType <= PT_sixbits))))
		{
			gcLogger.Error("Attribute 'Type' is not a primitive type");
			return FALSE;
		}

		bEmptyName = (!szName || (strlen(szName) == 0));
		if (!bEmptyName)
		{
			iIndex = pcFormat->GetIndex(szName);
			if (iIndex != -1)
			{
				gcLogger.Error2("Data format '", pcFormat->GetCommonName(), "' does not have a name '", szName, "'", NULL);
				return FALSE;
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
				return FALSE;
			}
			if ((iCount <= 0) || (iCount >= 32768))
			{
				return FALSE;
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
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadNamedDataFormat(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames)
{
	char*				szName;
	BOOL				bEmptyName;
	CInputDataFormat*	pcFormat;

	szName = pcParentTag->GetAttribute("Name");
	bEmptyName = (!szName || (strlen(szName) == 0));
	if (bEmptyName)
	{
		gcLogger.Error("'Format' tag attribute 'Name' is empty");
		return FALSE;
	}

	pcFormat = mpcInputDevices->mcDataFormats.Get(szName);
	if (pcFormat)
	{
		gcLogger.Error2("'Format' named '", szName, "' already exists", NULL);
		return FALSE;
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
	BOOL				bEmptyType;
	CInputDataFormat*	pcFormat;
	BOOL				bResult;

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
BOOL CInputDevicesReader::ReadDevices(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames)
{
	CMarkupTag*			pcTag;
	BOOL				bResult;
	char*				szType;
	char*				szPhyscical;
	STagIterator		sIter;
	BOOL				bPhysical;

	pcTag = pcParentTag->GetTag("Device", &sIter);
	while (pcTag)
	{
		szPhyscical = pcTag->GetAttribute("Physical");
		bPhysical = TRUE;
		if (szPhyscical)
		{
			if (strcmp(szPhyscical, "False") == 0)
			{
				bPhysical = FALSE;
			}
		}

		szType = pcTag->GetAttribute("Type");
		if (!szType || (strlen(szType) == 0))
		{
			gcLogger.Error2("'", pcTag->GetName(), "' tag attribute 'Type' is empty", NULL);
			return FALSE;
		}

		if (strcmp(szType, "Compound") == 0)
		{
			bResult = ReadDevices(pcTag, pcTypeNames);
			if (!bResult)
			{
				return FALSE;
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
				return FALSE;
			}

			bResult = ReadDevice(pcTag, pcTypeNames, bPhysical);
			if (!bResult)
			{
				return FALSE;
			}
		}
		pcTag = pcParentTag->GetNextTag(&sIter);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadDevice(CMarkupTag* pcParentTag, CTypeNames* pcTypeNames, BOOL bPhysical)
{
	CMarkupTag*					pcCategoryTag;
	CMarkupTag*					pcIDTag;
	CMarkupTag*					pcFriendlyTag;
	CMarkupTag*					pcSourcesTag;
	CMarkupTag*					pcChangesTag;
	CMarkupTag*					pcCommentTag;
	CMarkupTag*					pcVariablesTag;
	BOOL						bResult;
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
		return FALSE;
	}
	if (!pcIDTag)
	{
		gcLogger.Error2("'ID' tag not found in tag '", pcParentTag->GetName(), "'", NULL);
		return FALSE;
	}
	if (!pcFriendlyTag)
	{
		gcLogger.Error2("'Friendly' tag not found in tag '", pcParentTag->GetName(), "'", NULL);
		return FALSE;
	}

	szID.Init();
	pcIDTag->GetText(&szID);
	if (szID.Empty())
	{
		gcLogger.Error2("'", pcIDTag->GetName(), "' tag in tag '", pcParentTag->GetName(), "' is empty", NULL);
		szID.Kill();
		return FALSE;
	}

	pcDeviceDesc = mpcInputDevices->GetDescription(szID.Text());
	if (pcDeviceDesc != NULL)
	{
		gcLogger.Error2("'", szID.Text(), "' already defined", NULL);
		szID.Kill();
		return FALSE;
	}

	szCategory.Init();
	pcCategoryTag->GetText(&szCategory);
	if (szCategory.Empty())
	{
		gcLogger.Error2("'", pcCategoryTag->GetName(), "' tag in tag '", pcParentTag->GetName(), "' is empty", NULL);
		szID.Kill();
		szCategory.Kill();
		return FALSE;
	}

	pcCategory = mpcInputDevices->GetCategory(szCategory.Text());
	if (pcCategory == NULL)
	{
		gcLogger.Error2("'", szCategory.Text(), "' category in tag '", pcCategoryTag->GetName(), "' not found", NULL);
		szCategory.Kill();
		szID.Kill();
		return FALSE;
	}

	szFriendly.Init();
	pcFriendlyTag->GetText(&szFriendly);
	if (szFriendly.Empty())
	{
		gcLogger.Error2("'", pcFriendlyTag->GetName(), "' tag in tag '", pcParentTag->GetName(), "' is empty", NULL);
		szID.Kill();
		szCategory.Kill();
		szFriendly.Kill();
		return FALSE;
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
		bResult = ReadSources(pcSourcesTag, pcDeviceDesc, pcTypeNames, FALSE);
		if (!bResult)
		{
			return FALSE;
		}
		pcSourcesTag = pcParentTag->GetNextTag(&sIter);
	}

	if (iSourcesCount == 0)
	{
		gcLogger.Error2("Device '", pcDeviceDesc->GetFriendlyName(), "' has requires at least one 'Sources' tag", NULL);
		return FALSE;
	}

	pcDefaultVirtualDesc = pcDeviceDesc->CreateDefaultVirtualDesc();
	
	pcVariablesTag = pcParentTag->GetTag("Variables", &sIter);
	if (pcVariablesTag)
	{
		bResult = ReadVariables1(pcVariablesTag, pcDeviceDesc);
		if (!bResult)
		{
			return FALSE;
		}
		bResult = ReadVariables2(pcVariablesTag, pcDeviceDesc);
		if (!bResult)
		{
			return FALSE;
		}
	}

	pcChangesTag = pcParentTag->GetTag("Changes", &sIter);
	while (pcChangesTag)
	{
		bResult = ReadChanges(pcChangesTag, pcDeviceDesc);
		if (!bResult)
		{
			return FALSE;
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

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadChanges(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc)
{
	CMarkupTag*			pcTag;
	BOOL				bResult;
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
			return FALSE;
		}

		szReplacement.Init();
		pcTag->GetText(&szReplacement);

		if (szReplacement.Empty())
		{
			gcLogger.Error2("'", pcTag->GetName(), "' is empty", NULL);
			szReplacement.Kill();
			return FALSE;
		}

		bResult = pcDeviceDesc->RenameSource(szExisting, szReplacement.Text());

		if (!bResult)
		{
			gcLogger.Error2("'", pcDeviceDesc->GetID(), "' does not have a source called '", szExisting, "'", NULL);
			szReplacement.Kill();
			return FALSE;
		}
		szReplacement.Kill();
		pcTag = pcParentTag->GetNextTag(&sIter);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadSources(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc, CTypeNames* pcTypeNames, BOOL bDump)
{
	CMarkupTag*					pcTag;
	BOOL						bResult;
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
		return FALSE;
	}

	if (strcmp(szAttribute, "Defined") == 0)
	{
		pcFormatTag = pcParentTag->GetTag("Format");
		if (!pcFormatTag)
		{
			gcLogger.Error2("'Format' tag not found in tag '", pcParentTag->GetName(), "' for 'Defined'", NULL);
			return FALSE;
		}

		pcDataFormat = ReadDefinedDataFormat(pcFormatTag, pcTypeNames);
		if (!pcDataFormat)
		{
			return FALSE;
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
				return FALSE;
			}
			pcTag = pcParentTag->GetNextTag(&sIter);
		}
		
		if (bDump)
		{
			szDump.Init("    </Single>\n");
			szDump.Dump();
			szDump.Kill();
		}
		return TRUE;
	}
	else if (strcmp(szAttribute, "Named") == 0)
	{
		szNamedID.Init();
		pcParentTag->GetText(&szNamedID);
		if (szNamedID.Empty())
		{
			gcLogger.Error2("'", pcParentTag->GetName(), "' tag is empty for 'Named'", NULL);
			szNamedID.Kill();
			return FALSE;
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
				return FALSE;
			}

			cCopyContext.Init(pcSourceDesc, pcDeviceDesc);
			pcDeviceDesc->CopySources(&cCopyContext);
			szNamedID.Kill();
			return TRUE;
		}
		else
		{
			gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Named' did not contain a valid device ID [", szNamedID.Text(), "] while parsing [", pcDeviceDesc->GetID(), "]", NULL);
			return FALSE;
		}
	}
	else
	{
		gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Type' invalid.   Only 'Named' and 'Defined' allowed", NULL);
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadSource(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc, BOOL bDump)
{
	char*					szAttribute;
	EInputSourceType		eType;
	CMarkupTag*				pcRestTag;
	CMarkupTag*				pcFriendlyTag;
	CMarkupTag*				pcTag;
	CMarkupTag*				pcGenericTag;
	CChars					szFriendly;
	CInputSourceDesc*		pcSourceDesc;
	BOOL					bResult;
	float		 			fValue;
	char*					szType;
	char*					szEmit;
	BOOL					bEmitRestEvent;
	BOOL					bHasRestValue;
	CChars					szGeneric;
	STagIterator			sIter;
	CInputCategoryGeneric*	pcGeneric;
	CChars					szDump;

	szAttribute = pcParentTag->GetAttribute("Type");
	if (!szAttribute || (strlen(szAttribute) == 0))
	{
		gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Type' is empty", NULL);
		return FALSE;
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
		return FALSE;
	}

	pcFriendlyTag = pcParentTag->GetTag("Friendly");
	if (pcFriendlyTag == NULL)
	{
		gcLogger.Error2("'Friendly' tag not found in tag '", pcParentTag->GetName(), "'", NULL);
		return FALSE;
	}
	szFriendly.Init();
	pcFriendlyTag->GetText(&szFriendly);
	if (szFriendly.Empty())
	{
		szFriendly.Kill();
		return NULL;
	}

	fValue = 0;
	bEmitRestEvent = FALSE;
	bHasRestValue = TRUE;
	pcRestTag = pcParentTag->GetTag("Rest");
	if (pcRestTag)
	{
		szType = pcRestTag->GetAttribute("Type");
		if (!szType || (strlen(szType) == 0))
		{
			szFriendly.Kill();
			return FALSE;
		}

		if (strcmp(szType, "Numeric") == 0)
		{
			bResult = CMarkupTextParser::ReadFloat(pcRestTag, &fValue);
			if (!bResult)
			{
				szFriendly.Kill();
				return FALSE;
			}

			if ((fValue < 0.0f) || (fValue > 1.0f))
			{
				gcLogger.Error("'Rest' tag with numeric value must be in the range 0.0f to 1.0f");
				szFriendly.Kill();
				return FALSE;
			}
		}
		else if (strcmp(szType, "None") == 0)
		{
			bHasRestValue = FALSE;
		}
		else
		{
			szFriendly.Kill();
			return FALSE;
		}

		szEmit = pcRestTag->GetAttribute("Emit");
		if (szEmit)
		{
			if (strcmp(szEmit, "True") == 0)
			{
				bEmitRestEvent = TRUE;
			}
			else if (strcmp(szEmit, "False") == 0)
			{
				bEmitRestEvent = FALSE;
			}
			else
			{
				szFriendly.Kill();
				return FALSE;
			}
		}
	}
	if ((fValue > 1.0) || (fValue < 0.0f))
	{
		szFriendly.Kill();
		return FALSE;
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
			return FALSE;
		}
		pcGeneric = pcDeviceDesc->GetCategory()->GetGeneric(szGeneric.Text());
		pcSourceDesc->GetGenerics()->Add(&pcGeneric);
		szGeneric.Kill();

		pcGenericTag = pcParentTag->GetNextTag(&sIter);
	}

	pcTag = pcParentTag->GetTag(szAttribute, &sIter);
	if (pcTag == NULL)
	{
		return FALSE;
	}

	if (eType == ISET_Delta)
	{
		bResult = ReadSourceValue(pcTag, pcSourceDesc);
		if (!bResult)
		{
			return FALSE;
		}
	}
	else
	{
		while (pcTag)
		{
			bResult = ReadSourceValue(pcTag, pcSourceDesc);
			if (!bResult)
			{
				return FALSE;
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
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadSourceValue(CMarkupTag* pcParentTag, CInputSourceDesc* pcSourceDesc)
{
	CMarkupTag*				pcTag;
	CMarkupTag*				pcOrderTag;
	CInputSourceValue*		pcSourceValue;
	CMarkupTag*				pcValueTag;
	char*					szType;
	CChars					szChannel;
	int						iIndex;
	float		 			fValue;
	BOOL					bResult;
	int						iOrder;
	STagIterator			sIter;

	pcValueTag = pcParentTag->GetTag("Value");
	if (pcValueTag == NULL)
	{
		return FALSE;
	}

	szType = pcValueTag->GetAttribute("Type");
	if (!szType || (strlen(szType) == 0))
	{
		return FALSE;
	}

	if (strcmp(szType, "Channel") == 0)
	{
		szChannel.Init();
		pcValueTag->GetText(&szChannel);
		iIndex = pcSourceDesc->GetDeviceDesc()->GetDataFormat()->GetIndex(szChannel.Text());
		szChannel.Kill();
		if (iIndex == -1)
		{
			return FALSE;
		}
		pcSourceValue = pcSourceDesc->AddValue(iIndex);
	}
	else if (strcmp(szType, "Numeric") == 0)
	{
		bResult = CMarkupTextParser::ReadFloat(pcValueTag, &fValue);
		if (!bResult)
		{
			return FALSE;
		}

		if ((fValue < 0.0f) || (fValue > 1.0f))
		{
			gcLogger.Error("'Value' tag with numeric value must be in the range 0.0f to 1.0f");
			return FALSE;
		}

		pcSourceValue = pcSourceDesc->AddValue(fValue);
	}
	else
	{
		return FALSE;
	}

	if (pcSourceDesc->GetType() == ISET_State)
	{
		bResult = ReadStateDetail(pcParentTag, pcSourceValue);
		if (!bResult)
		{
			return FALSE;
		}
	}
	else if (pcSourceDesc->GetType() == ISET_Delta)
	{
		bResult = ReadDeltaDetail(pcParentTag, pcSourceValue);
		if (!bResult)
		{
			return FALSE;
		}
	}

	pcOrderTag = pcParentTag->GetTag("Order");
	if (pcOrderTag)
	{
		bResult = CMarkupTextParser::ReadInteger(pcOrderTag, &iOrder);
		if (!bResult)
		{
			return FALSE;
		}
		pcSourceValue->SetOrder(iOrder);
	}

	pcTag = pcParentTag->GetTag("Channel", &sIter);
	while (pcTag)
	{
		bResult = ReadValueChannel(pcTag, pcSourceValue);
		if (!bResult)
		{
			return FALSE;
		}
		pcTag = pcParentTag->GetNextTag(&sIter);
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadStateDetail(CMarkupTag* pcParentTag, CInputSourceValue* pcSourceValue)
{
	CMarkupTag*		pcMaxTag;
	CMarkupTag*		pcMinTag;
	float			fMax;
	float			fMin;
	BOOL			bResult;
	BOOL			bMin;
	BOOL			bMax;

	pcMaxTag = pcParentTag->GetTag("Max");
	pcMinTag = pcParentTag->GetTag("Min");

	fMin = 0.0f;
	fMax = 1.0f;

	bMax = FALSE;
	bMin = FALSE;
	if (pcMaxTag)
	{
		bResult = CMarkupTextParser::ReadFloat(pcMaxTag, &fMax);
		if (!bResult)
		{
			return FALSE;
		}
		bMax = TRUE;
	}

	if (pcMinTag)
	{
		bResult = CMarkupTextParser::ReadFloat(pcMinTag, &fMin);
		if (!bResult)
		{
			return FALSE;
		}
		bMin = TRUE;
	}

	if (pcSourceValue->GetValueType() == ISVT_Channel)
	{
		if (!(bMax && bMin))
		{
			gcLogger.Error2("'", pcParentTag->GetName(), "' has a channel value but no 'Min' and 'Max' tags", NULL);
			return FALSE;
		}
	}

	pcSourceValue->SetStateDetail(fMax, fMin);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadDeltaDetail(CMarkupTag* pcParentTag, CInputSourceValue* pcSourceValue)
{
	CMarkupTag*		pcSensitivityTag;
	CMarkupTag*		pcOffsetTag;
	float			fSensitivity;
	float			fOffset;
	BOOL			bResult;

	pcSensitivityTag = pcParentTag->GetTag("Sensitivity");
	pcOffsetTag = pcParentTag->GetTag("Offset");

	fSensitivity = 1.0f;
	if (pcSensitivityTag)
	{
		bResult = CMarkupTextParser::ReadFloat(pcSensitivityTag, &fSensitivity);
		if (!bResult)
		{
			return FALSE;
		}
	}

	fOffset = 0.0f;
	if (pcOffsetTag)
	{
		bResult = CMarkupTextParser::ReadFloat(pcOffsetTag, &fOffset);
		if (!bResult)
		{
			return FALSE;
		}
	}

	pcSourceValue->SetDeltaDetail(fSensitivity, fOffset);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadValueChannel(CMarkupTag* pcParentTag, CInputSourceValue* pcSourceValue)
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
		return FALSE;
	}

	iIndex = pcSourceValue->GetDataFormat()->GetIndex(szName);
	if (iIndex == -1)
	{
		return FALSE;
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
		return FALSE;
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
			return FALSE;
		}
	}

	pcChannel = pcSourceValue->AddChannel(iIndex, eComparator, vTest);
	szTestValue.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadVariables1(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc)
{
	CMarkupTag*					pcTag;
	STagIterator				sIter;
	char*						szName;
	CInputDeviceVariableDesc*	pcVariable;
	CMarkupTag*					pcValueTag;
	STagIterator				sValueIter;
	BOOL						bResult;
	CChars						szNamedID;
	CInputDeviceDesc*			pcSourceDesc;
	char*						szAttribute;
	int							iInitialValue;
	
	szAttribute = pcParentTag->GetAttribute("Type");
	if (!szAttribute || (strlen(szAttribute) == 0))
	{
		gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Type' is empty", NULL);
		return FALSE;
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
				return FALSE;
			}

			pcVariable = pcDeviceDesc->GetVariable(szName);
			if (pcVariable)
			{
				gcLogger.Error2("Variable '", szName, "' already exists on device '", pcDeviceDesc->GetFriendlyName(), "'", NULL);
				return FALSE;
			}

			pcVariable = pcDeviceDesc->AddVariable(szName);
			pcValueTag = pcTag->GetTag("Value", &sValueIter);

			if (!pcValueTag)
			{
				gcLogger.Error2("Variable '", szName, "' on device '", pcDeviceDesc->GetFriendlyName(), "' has no values", NULL);
				return FALSE;
			}

			iInitialValue = 0;
			while (pcValueTag)
			{
				bResult = ReadVariableValue1(pcValueTag, pcVariable, &iInitialValue);
				if (!bResult)
				{
					return FALSE;
				}

				pcValueTag = pcTag->GetNextTag(&sValueIter);
			}
			pcTag = pcParentTag->GetNextTag(&sIter);
		}
		return TRUE;
	}
	else if (strcmp(szAttribute, "Named") == 0)
	{
		szNamedID.Init();
		pcParentTag->GetText(&szNamedID);
		if (szNamedID.Empty())
		{
			gcLogger.Error2("'", pcParentTag->GetName(), "' tag is empty for 'Named'", NULL);
			szNamedID.Kill();
			return FALSE;
		}
		pcSourceDesc = mpcInputDevices->GetDescription(szNamedID.Text());
		if (pcSourceDesc)
		{
			szNamedID.Kill();
			return TRUE;
		}
		else
		{
			szNamedID.Kill();
			gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Named' did not contain a valid device ID [", szNamedID.Text(), "] while parsing [", pcDeviceDesc->GetID(), "]", NULL);
			return FALSE;
		}
	}
	else
	{
		gcLogger.Error2("'", pcParentTag->GetName(), "' tag attribute 'Type' invalid.   Only 'Named' and 'Defined' allowed", NULL);
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadVariables2(CMarkupTag* pcParentTag, CInputDeviceDesc* pcDeviceDesc)
{
	CMarkupTag*					pcTag;
	STagIterator				sIter;
	char*						szName;
	CInputDeviceVariableDesc*	pcVariable;
	CMarkupTag*					pcValueTag;
	STagIterator				sValueIter;
	BOOL						bResult;
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
					return FALSE;
				}

				pcValueTag = pcTag->GetNextTag(&sValueIter);
			}
			pcTag = pcParentTag->GetNextTag(&sIter);
		}
		return TRUE;
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
		return TRUE;
	}
	return FALSE;
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
BOOL CInputDevicesReader::ReadVariableValue1(CMarkupTag* pcValueTag, CInputDeviceVariableDesc* pcVariable, int* piInitialValue)
{
	char*						szValueName;
	char*						szInitial;

	szValueName = pcValueTag->GetAttribute("Name");
	if ((szValueName == NULL) || (strlen(szValueName) == 0))
	{
		gcLogger.Error("'Value' tag attribute 'Name' is empty");
		return FALSE;
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
				return FALSE;
			}
		}
		else if (strcmp("False", szInitial) == 0)
		{
		}
		else
		{
			gcLogger.Error2("'", pcValueTag->GetName(), "' variable tag attribute 'Initial' only allows 'True' and 'False'", NULL);
			return FALSE;
		}
	}

	pcVariable->AddValue(szValueName);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadVariableValue2(CMarkupTag* pcValueTag, CInputDeviceVariableDesc* pcVariable, CInputChordDescs* pcChordDescs)
{
	CInputChordReader				cChordReader;
	BOOL							bResult;
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
			pcVariableValueDesc->SetInitial(TRUE);
		}
	}
	
	cChordReader.Init(pcValueTag, pcChordDescs, INPUT_DEVICE_VARIABLE_ACTION, pcVariable->GetDeviceDesc());
	bResult = cChordReader.Read();
	if (!bResult)
	{
		cChordReader.Kill();
		return FALSE;
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
			return FALSE;
		}
		pcValueVariableTag = pcValueTag->GetNextTag(&sIter);
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevicesReader::ReadVariableValueVariable(CMarkupTag* pcTag, CInputDeviceVariableValueDesc* pcVariableValueDesc)
{
	char*							szName;
	char*							szEquals;
	CInputDeviceVariableDesc*		pcConditionVariableDesc;
	CInputDeviceVariableValueDesc*	pcConditionValueDesc;

	szName = pcTag->GetAttribute("Name");
	if ((szName == NULL) || (strlen(szName) == 0))
	{
		gcLogger.Error("'Variable' tag attribute 'Name' is empty");
		return FALSE;
	}

	szEquals = pcTag->GetAttribute("Equals");
	if ((szEquals == NULL) || (strlen(szEquals) == 0))
	{
		gcLogger.Error("'Variable' tag attribute 'Equals' is empty");
		return FALSE;
	}

	pcConditionVariableDesc = pcVariableValueDesc->GetVariableDesc()->GetDeviceDesc()->GetVariable(szName);
	if (!pcConditionVariableDesc)
	{
		gcLogger.Error2("Could not find variable '", szName, "' for condition in variable '", pcVariableValueDesc->GetVariableDesc()->GetName(), "'", NULL);
		return FALSE;
	}

	pcConditionValueDesc = pcConditionVariableDesc->GetValue(szEquals);
	if (!pcConditionValueDesc)
	{
		gcLogger.Error2("Variable '", szName, "' does not allow value '", szEquals, "' for condition Equals", NULL);
		return FALSE;
	}

	pcVariableValueDesc->AddCondition(pcConditionValueDesc);
	return TRUE;
}

