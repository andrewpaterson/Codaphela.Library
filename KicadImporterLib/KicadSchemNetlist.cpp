/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "KicadSchemNetlist.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNetlist::Init(CKicadSchemFile* pcFile)
{
	mpcFile = pcFile;
	macSubnets.Init();
	macNets.Init();
	macBuses.Init();
	macBusNets.Init();
	masNodes.Init();
	maiParents.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNetlist::Kill(void)
{
	size	i;

	maiParents.Kill();
	masNodes.Kill();

	for (i = 0; i < macBusNets.NumElements(); i++)
	{
		GetBusNet(i)->Kill();
	}
	macBusNets.Kill();

	for (i = 0; i < macBuses.NumElements(); i++)
	{
		GetBus(i)->Kill();
	}
	macBuses.Kill();

	for (i = 0; i < macNets.NumElements(); i++)
	{
		GetNet(i)->Kill();
	}
	macNets.Kill();

	for (i = 0; i < macSubnets.NumElements(); i++)
	{
		GetSubnet(i)->Kill();
	}
	macSubnets.Kill();

	mpcFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Fails if a placed symbol's library symbol isn't in the file.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemNetlist::Build(void)
{
	if (!AddNodes())
	{
		return false;
	}
	JoinNodes();
	CreateSubnets();
	CreateNets();
	CreateBuses();
	AddBusTaps();
	AddBusMembers();

	maiParents.Kill();
	masNodes.Kill();
	maiParents.Init();
	masNodes.Init();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The wires first, then everything that is at a point.  Junctions and
// no-connects are last so a subnet is made by something it holds.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemNetlist::AddNodes(void)
{
	size					i;
	size					j;
	CKicadSchemWire*		pcWire;
	CKicadSchemSymbol*		pcSymbol;
	CKicadSchemLibSymbol*	pcLibSymbol;
	CKicadSchemLibPin*		pcPin;
	SKicadSchemPoint		sPosition;
	CKicadSchemLabel*		pcLabel;
	CKicadSchemSheet*		pcSheet;
	CKicadSchemSheetPin*	pcSheetPin;
	CKicadSchemJunction*	pcJunction;
	CKicadSchemNoConnect*	pcNoConnect;

	for (i = 0; i < mpcFile->NumWires(); i++)
	{
		pcWire = mpcFile->GetWire(i);
		if (!pcWire->IsBus())
		{
			AddNode(KSNNT_Wire, pcWire, NULL, pcWire->GetStart());
		}
	}

	for (i = 0; i < mpcFile->NumSymbols(); i++)
	{
		pcSymbol = mpcFile->GetSymbol(i);
		pcLibSymbol = mpcFile->GetLibSymbol(pcSymbol);
		if (pcLibSymbol == NULL)
		{
			return gcLogger.Error2(__METHOD__, " Symbol [", pcSymbol->GetReference(), "] uses library symbol [", pcSymbol->GetLibSymbolName(), "] which is not in [", mpcFile->GetFileName(), "].", NULL);
		}

		for (j = 0; j < pcLibSymbol->NumPins(); j++)
		{
			pcPin = pcLibSymbol->GetPin(j);
			if (pcPin->IsOn(pcSymbol->GetUnit(), pcSymbol->GetBodyStyle()))
			{
				mpcFile->GetPinPosition(&sPosition, pcSymbol, pcPin);
				AddNode(KSNNT_Pin, pcPin, pcSymbol, &sPosition);
			}
		}
	}

	for (i = 0; i < mpcFile->NumLabels(); i++)
	{
		pcLabel = mpcFile->GetLabel(i);
		AddNode(KSNNT_Label, pcLabel, NULL, pcLabel->GetPosition());
	}

	for (i = 0; i < mpcFile->NumSheets(); i++)
	{
		pcSheet = mpcFile->GetSheet(i);
		for (j = 0; j < pcSheet->NumPins(); j++)
		{
			pcSheetPin = pcSheet->GetPin(j);
			AddNode(KSNNT_SheetPin, pcSheetPin, pcSheet, pcSheetPin->GetPosition());
		}
	}

	for (i = 0; i < mpcFile->NumJunctions(); i++)
	{
		pcJunction = mpcFile->GetJunction(i);
		AddNode(KSNNT_Junction, pcJunction, NULL, pcJunction->GetPosition());
	}

	for (i = 0; i < mpcFile->NumNoConnects(); i++)
	{
		pcNoConnect = mpcFile->GetNoConnect(i);
		AddNode(KSNNT_NoConnect, pcNoConnect, NULL, pcNoConnect->GetPosition());
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SKicadSchemNetNode* CKicadSchemNetlist::AddNode(EKicadSchemNetNodeType eType, void* pvItem, void* pvOwner, SKicadSchemPoint* psPosition)
{
	SKicadSchemNetNode*	psNode;

	psNode = masNodes.Add();
	psNode->eType = eType;
	psNode->pvItem = pvItem;
	psNode->pvOwner = pvOwner;
	psNode->sPosition = *psPosition;
	maiParents.Add((int)(masNodes.NumElements() - 1));
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//
// Wires join where an end of one is on the other.  A point joins the
// wires it is on and the other points at the same place.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNetlist::JoinNodes(void)
{
	size				i;
	size				j;
	SKicadSchemNetNode*	psNode;
	SKicadSchemNetNode*	psOther;
	CKicadSchemWire*	pcWire;
	CKicadSchemWire*	pcOtherWire;

	for (i = 0; i < masNodes.NumElements(); i++)
	{
		psNode = masNodes.Get(i);
		for (j = i + 1; j < masNodes.NumElements(); j++)
		{
			psOther = masNodes.Get(j);
			if (IsWireNode(psNode))
			{
				pcWire = (CKicadSchemWire*)psNode->pvItem;
				if (IsWireNode(psOther))
				{
					pcOtherWire = (CKicadSchemWire*)psOther->pvItem;
					if (pcWire->Contains(pcOtherWire->GetStart()) || pcWire->Contains(pcOtherWire->GetEnd()) ||
						pcOtherWire->Contains(pcWire->GetStart()) || pcOtherWire->Contains(pcWire->GetEnd()))
					{
						JoinKicadSchemRoots(&maiParents, (int)i, (int)j);
					}
				}
				else if (pcWire->Contains(&psOther->sPosition))
				{
					JoinKicadSchemRoots(&maiParents, (int)i, (int)j);
				}
			}
			else if (IsWireNode(psOther))
			{
				pcOtherWire = (CKicadSchemWire*)psOther->pvItem;
				if (pcOtherWire->Contains(&psNode->sPosition))
				{
					JoinKicadSchemRoots(&maiParents, (int)i, (int)j);
				}
			}
			else if (psNode->sPosition.Equals(&psOther->sPosition))
			{
				JoinKicadSchemRoots(&maiParents, (int)i, (int)j);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A subnet for each set of joined nodes that holds a wire, pin, label or
// sheet pin.  A junction or no-connect on nothing is dropped.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNetlist::CreateSubnets(void)
{
	CArrayInt			aiRootSubnets;
	size				i;
	int					iRoot;
	int					iSubnet;
	SKicadSchemNetNode*	psNode;
	CKicadSchemSubnet*	pcSubnet;

	aiRootSubnets.Init();
	for (i = 0; i < masNodes.NumElements(); i++)
	{
		aiRootSubnets.Add(-1);
	}

	for (i = 0; i < masNodes.NumElements(); i++)
	{
		psNode = masNodes.Get(i);
		iRoot = FindKicadSchemRoot(&maiParents, (int)i);
		iSubnet = aiRootSubnets.GetValue(iRoot);
		if (iSubnet == -1)
		{
			if ((psNode->eType == KSNNT_Junction) || (psNode->eType == KSNNT_NoConnect))
			{
				continue;
			}
			pcSubnet = (CKicadSchemSubnet*)macSubnets.Add(sizeof(CKicadSchemSubnet));
			pcSubnet->Init();
			iSubnet = (int)(macSubnets.NumElements() - 1);
			aiRootSubnets.SetValue(iRoot, iSubnet);
		}
		else
		{
			pcSubnet = GetSubnet(iSubnet);
		}

		switch (psNode->eType)
		{
		case KSNNT_Wire:
			pcSubnet->AddWire((CKicadSchemWire*)psNode->pvItem);
			break;
		case KSNNT_Pin:
			pcSubnet->AddPin((CKicadSchemSymbol*)psNode->pvOwner, (CKicadSchemLibPin*)psNode->pvItem, &psNode->sPosition);
			break;
		case KSNNT_Label:
			pcSubnet->AddLabel((CKicadSchemLabel*)psNode->pvItem);
			break;
		case KSNNT_SheetPin:
			pcSubnet->AddSheetPin((CKicadSchemSheet*)psNode->pvOwner, (CKicadSchemSheetPin*)psNode->pvItem);
			break;
		case KSNNT_Junction:
			pcSubnet->AddJunction((CKicadSchemJunction*)psNode->pvItem);
			break;
		case KSNNT_NoConnect:
			pcSubnet->SetNoConnect();
			break;
		}
	}
	aiRootSubnets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Subnets with a name in common are one net.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNetlist::CreateNets(void)
{
	CArrayChars			aszNames;
	CArrayInt			aiNameSubnets;
	CArrayInt			aiSubnetParents;
	CArrayInt			aiRootNets;
	size				i;
	int					iRoot;
	int					iNet;
	CKicadSchemNet*		pcNet;

	aszNames.Init();
	aiNameSubnets.Init();
	aiSubnetParents.Init();
	aiRootNets.Init();
	for (i = 0; i < macSubnets.NumElements(); i++)
	{
		aiSubnetParents.Add((int)i);
		aiRootNets.Add(-1);
	}

	for (i = 0; i < macSubnets.NumElements(); i++)
	{
		AddSubnetNames(GetSubnet(i), i, &aszNames, &aiNameSubnets, &aiSubnetParents);
	}

	for (i = 0; i < macSubnets.NumElements(); i++)
	{
		iRoot = FindKicadSchemRoot(&aiSubnetParents, (int)i);
		iNet = aiRootNets.GetValue(iRoot);
		if (iNet == -1)
		{
			pcNet = (CKicadSchemNet*)macNets.Add(sizeof(CKicadSchemNet));
			pcNet->Init();
			iNet = (int)(macNets.NumElements() - 1);
			aiRootNets.SetValue(iRoot, iNet);
		}
		else
		{
			pcNet = GetNet(iNet);
		}
		pcNet->AddSubnet(GetSubnet(i));
	}

	for (i = 0; i < macNets.NumElements(); i++)
	{
		NameNet(GetNet(i));
	}

	aiRootNets.Kill();
	aiSubnetParents.Kill();
	aiNameSubnets.Kill();
	aszNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Names (labels of every kind and power names) are in scope N.  A shared
// pin is in scope P as reference.number: it is placed with each unit but
// is one pin, so the subnets of its places are one net.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNetlist::AddSubnetNames(CKicadSchemSubnet* pcSubnet, size uiSubnet, CArrayChars* paszNames, CArrayInt* paiNameSubnets, CArrayInt* paiSubnetParents)
{
	size					i;
	SKicadSchemNetPin*		psPin;
	CKicadSchemLibSymbol*	pcLibSymbol;
	CKicadSchemLabel*		pcLabel;
	CChars					szPin;

	for (i = 0; i < pcSubnet->NumPins(); i++)
	{
		psPin = pcSubnet->GetPin(i);
		pcLibSymbol = mpcFile->GetLibSymbol(psPin->pcSymbol);
		if (IsGlobalPowerPin(pcLibSymbol, psPin->pcPin))
		{
			AddSubnetName("N", GetGlobalPowerName(psPin->pcSymbol, pcLibSymbol, psPin->pcPin), uiSubnet, paszNames, paiNameSubnets, paiSubnetParents);
		}
		else if (IsSharedPin(pcLibSymbol, psPin->pcSymbol, psPin->pcPin))
		{
			szPin.Init(psPin->pcSymbol->GetReference());
			szPin.Append('.');
			szPin.Append(psPin->pcPin->GetNumber());
			AddSubnetName("P", szPin.Text(), uiSubnet, paszNames, paiNameSubnets, paiSubnetParents);
			szPin.Kill();
		}
	}

	for (i = 0; i < pcSubnet->NumLabels(); i++)
	{
		pcLabel = pcSubnet->GetLabel(i);
		AddSubnetName("N", pcLabel->GetText(), uiSubnet, paszNames, paiNameSubnets, paiSubnetParents);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNetlist::AddSubnetName(char* szScope, char* szName, size uiSubnet, CArrayChars* paszNames, CArrayInt* paiNameSubnets, CArrayInt* paiSubnetParents)
{
	CChars	szKey;
	size	uiIndex;

	if (StrEmpty(szName))
	{
		return;
	}

	szKey.Init(szScope);
	szKey.Append(':');
	szKey.Append(szName);
	uiIndex = paszNames->GetIndex(szKey.Text());
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		paszNames->Add(szKey.Text());
		paiNameSubnets->Add((int)uiSubnet);
	}
	else
	{
		JoinKicadSchemRoots(paiSubnetParents, paiNameSubnets->GetValue(uiIndex), (int)uiSubnet);
	}
	szKey.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The first global name (by text), else the first local name, else made
// from the pin with the first reference and number.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNetlist::NameNet(CKicadSchemNet* pcNet)
{
	size					i;
	size					j;
	CKicadSchemSubnet*		pcSubnet;
	SKicadSchemNetPin*		psPin;
	SKicadSchemNetPin*		psFirstPin;
	CKicadSchemLibSymbol*	pcLibSymbol;
	CKicadSchemLabel*		pcLabel;
	char*					szGlobal;
	char*					szLocal;
	char*					szName;
	int						iCompare;
	CChars					szPinName;

	szGlobal = NULL;
	szLocal = NULL;
	psFirstPin = NULL;
	for (i = 0; i < pcNet->NumSubnets(); i++)
	{
		pcSubnet = pcNet->GetSubnet(i);
		for (j = 0; j < pcSubnet->NumPins(); j++)
		{
			psPin = pcSubnet->GetPin(j);
			pcLibSymbol = mpcFile->GetLibSymbol(psPin->pcSymbol);
			if (IsGlobalPowerPin(pcLibSymbol, psPin->pcPin))
			{
				szName = GetGlobalPowerName(psPin->pcSymbol, pcLibSymbol, psPin->pcPin);
				if (!StrEmpty(szName) && ((szGlobal == NULL) || (StringCompare(szName, szGlobal) < 0)))
				{
					szGlobal = szName;
				}
			}
			else if (!pcLibSymbol->IsPower())
			{
				if (psFirstPin == NULL)
				{
					psFirstPin = psPin;
				}
				else
				{
					iCompare = StringCompare(psPin->pcSymbol->GetReference(), psFirstPin->pcSymbol->GetReference());
					if ((iCompare < 0) || ((iCompare == 0) && (StringCompare(psPin->pcPin->GetNumber(), psFirstPin->pcPin->GetNumber()) < 0)))
					{
						psFirstPin = psPin;
					}
				}
			}
		}

		for (j = 0; j < pcSubnet->NumLabels(); j++)
		{
			pcLabel = pcSubnet->GetLabel(j);
			szName = pcLabel->GetText();
			if (StrEmpty(szName))
			{
				continue;
			}
			if (pcLabel->GetType() == KSLT_Global)
			{
				if ((szGlobal == NULL) || (StringCompare(szName, szGlobal) < 0))
				{
					szGlobal = szName;
				}
			}
			else if ((szLocal == NULL) || (StringCompare(szName, szLocal) < 0))
			{
				szLocal = szName;
			}
		}
	}

	if (szGlobal != NULL)
	{
		pcNet->SetName(szGlobal, KSNNS_Global);
	}
	else if (szLocal != NULL)
	{
		pcNet->SetName(szLocal, KSNNS_Local);
	}
	else
	{
		szPinName.Init("Net-(");
		if (psFirstPin != NULL)
		{
			szPinName.Append(psFirstPin->pcSymbol->GetReference());
			szPinName.Append("-Pad");
			szPinName.Append(psFirstPin->pcPin->GetNumber());
		}
		else
		{
			szPinName.Append("Unconnected-");
			szPinName.Append(macNets.IndexOf(pcNet));
		}
		szPinName.Append(')');
		pcNet->SetName(szPinName.Text(), KSNNS_Pin);
		szPinName.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemNetlist::IsWireNode(SKicadSchemNetNode* psNode)
{
	return psNode->eType == KSNNT_Wire;
}


//////////////////////////////////////////////////////////////////////////
//
// A pin on every unit (unit 0) of a symbol with more than one, as the
// shared power pins of a quad gate.  The symbol must be annotated: before
// it is, every "U?" has the same reference.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemNetlist::IsSharedPin(CKicadSchemLibSymbol* pcLibSymbol, CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin)
{
	char*	szReference;
	size	uiLength;

	if ((pcPin->GetUnit() != 0) || (pcLibSymbol->NumUnits() <= 1) || pcLibSymbol->IsPower())
	{
		return false;
	}
	szReference = pcSymbol->GetReference();
	uiLength = StrLen(szReference);
	return (uiLength > 0) && (szReference[uiLength - 1] != '?');
}


//////////////////////////////////////////////////////////////////////////
//
// As KiCad's SCH_PIN::IsGlobalPower.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemNetlist::IsGlobalPowerPin(CKicadSchemLibSymbol* pcLibSymbol, CKicadSchemLibPin* pcPin)
{
	return (pcPin->GetType() == KSPT_PowerIn) && (pcPin->IsHidden() || pcLibSymbol->IsPower());
}


//////////////////////////////////////////////////////////////////////////
//
// A power symbol's Value (KiCad 6 power symbols named their nets by their
// pin's name, which was the same) or a hidden power pin's name.
//
//////////////////////////////////////////////////////////////////////////
char* CKicadSchemNetlist::GetGlobalPowerName(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol, CKicadSchemLibPin* pcPin)
{
	char*	szValue;

	if (pcLibSymbol->IsPower())
	{
		szValue = pcSymbol->GetValue();
		if (!StrEmpty(szValue))
		{
			return szValue;
		}
	}
	return pcPin->GetName();
}


//////////////////////////////////////////////////////////////////////////
//
// The bus wires joined where an end of one is on another, with the
// junctions and labels on them.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNetlist::CreateBuses(void)
{
	CArrayTemplatePtr<CKicadSchemWire>	apcWires;
	CArrayInt							aiParents;
	CArrayInt							aiRootBuses;
	size								i;
	size								j;
	CKicadSchemWire*					pcWire;
	CKicadSchemWire*					pcOther;
	int									iRoot;
	CKicadSchemBus*						pcBus;
	bool								bVertical;

	apcWires.Init();
	aiParents.Init();
	aiRootBuses.Init();
	for (i = 0; i < mpcFile->NumWires(); i++)
	{
		pcWire = mpcFile->GetWire(i);
		if (pcWire->IsBus())
		{
			apcWires.Add(pcWire);
			aiParents.Add((int)(apcWires.NumElements() - 1));
			aiRootBuses.Add(-1);
		}
	}
	for (i = 0; i < apcWires.NumElements(); i++)
	{
		pcWire = apcWires.GetPtr(i);
		for (j = i + 1; j < apcWires.NumElements(); j++)
		{
			pcOther = apcWires.GetPtr(j);
			if (pcWire->Contains(pcOther->GetStart()) || pcWire->Contains(pcOther->GetEnd()) || pcOther->Contains(pcWire->GetStart()) || pcOther->Contains(pcWire->GetEnd()))
			{
				JoinKicadSchemRoots(&aiParents, (int)i, (int)j);
			}
		}
	}
	for (i = 0; i < apcWires.NumElements(); i++)
	{
		iRoot = FindKicadSchemRoot(&aiParents, (int)i);
		if (aiRootBuses.GetValue(iRoot) == -1)
		{
			pcBus = (CKicadSchemBus*)macBuses.Add(sizeof(CKicadSchemBus));
			pcBus->Init();
			aiRootBuses.SetValue(iRoot, (int)(macBuses.NumElements() - 1));
		}
		GetBus(aiRootBuses.GetValue(iRoot))->AddWire(apcWires.GetPtr(i));
	}
	aiRootBuses.Kill();
	aiParents.Kill();
	apcWires.Kill();

	for (i = 0; i < macBuses.NumElements(); i++)
	{
		pcBus = GetBus(i);
		for (j = 0; j < mpcFile->NumJunctions(); j++)
		{
			if (pcBus->Contains(mpcFile->GetJunction(j)->GetPosition(), &bVertical))
			{
				pcBus->AddJunction(mpcFile->GetJunction(j));
			}
		}
		for (j = 0; j < mpcFile->NumLabels(); j++)
		{
			if (pcBus->Contains(mpcFile->GetLabel(j)->GetPosition(), &bVertical))
			{
				pcBus->AddLabel(mpcFile->GetLabel(j));
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each bus entry with an end on a bus and the other on a member that is in
// a net.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNetlist::AddBusTaps(void)
{
	size					i;
	size					j;
	CKicadSchemBusEntry*	pcEntry;
	SKicadSchemPoint		sStart;
	SKicadSchemPoint		sEnd;
	CKicadSchemBus*			pcBus;
	CKicadSchemSubnet*		pcMember;
	bool					bVertical;

	for (i = 0; i < mpcFile->NumBusEntries(); i++)
	{
		pcEntry = mpcFile->GetBusEntry(i);
		sStart = *pcEntry->GetPosition();
		pcEntry->GetEnd(&sEnd);
		for (j = 0; j < macBuses.NumElements(); j++)
		{
			pcBus = GetBus(j);
			if (pcBus->Contains(&sStart, &bVertical))
			{
				pcMember = GetSubnetAt(&sEnd);
				if ((pcMember != NULL) && (pcMember->GetNet() != NULL))
				{
					pcBus->AddTap(pcEntry, &sStart, &sEnd, pcMember, bVertical);
				}
				break;
			}
			if (pcBus->Contains(&sEnd, &bVertical))
			{
				pcMember = GetSubnetAt(&sStart);
				if ((pcMember != NULL) && (pcMember->GetNet() != NULL))
				{
					pcBus->AddTap(pcEntry, &sEnd, &sStart, pcMember, bVertical);
				}
				break;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The members of each set of buses with the same name (an unnamed bus is
// a set of its own): the members its labels name and then the nets of the
// members its entries tap.  Each set is a bus net.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNetlist::AddBusMembers(void)
{
	CArrayInt			aiDone;
	CArrayInt			aiSet;
	CArrayChars			aszMembers;
	size				i;
	size				j;
	size				k;
	CKicadSchemBus*		pcBus;
	CKicadSchemBus*		pcOther;
	CKicadSchemNet*		pcNet;
	char*				szName;
	CChars				szNetName;

	aiDone.Init();
	for (i = 0; i < macBuses.NumElements(); i++)
	{
		aiDone.Add(0);
	}
	for (i = 0; i < macBuses.NumElements(); i++)
	{
		if (aiDone.GetValue(i) != 0)
		{
			continue;
		}
		pcBus = GetBus(i);
		szName = pcBus->GetName();
		aiSet.Init();
		for (j = i; j < macBuses.NumElements(); j++)
		{
			pcOther = GetBus(j);
			if ((j == i) || ((szName != NULL) && (aiDone.GetValue(j) == 0) && (pcOther->GetName() != NULL) && (StringCompare(pcOther->GetName(), szName) == 0)))
			{
				aiSet.Add((int)j);
				aiDone.SetValue(j, 1);
			}
		}

		aszMembers.Init();
		for (j = 0; j < aiSet.NumElements(); j++)
		{
			pcOther = GetBus(aiSet.GetValue(j));
			for (k = 0; k < pcOther->NumLabels(); k++)
			{
				ParseKicadSchemBusMembers(pcOther->GetLabel(k)->GetText(), &aszMembers);
			}
		}
		for (j = 0; j < aiSet.NumElements(); j++)
		{
			pcOther = GetBus(aiSet.GetValue(j));
			for (k = 0; k < pcOther->NumTaps(); k++)
			{
				aszMembers.AddIfUnique(pcOther->GetTap(k)->pcMember->GetNet()->GetName());
			}
		}

		pcNet = (CKicadSchemNet*)macBusNets.Add(sizeof(CKicadSchemNet));
		pcNet->Init();
		if (szName != NULL)
		{
			pcNet->SetName(szName, KSNNS_Local);
		}
		else
		{
			szNetName.Init("Bus-(");
			szNetName.Append(macBusNets.NumElements());
			szNetName.Append(')');
			pcNet->SetName(szNetName.Text(), KSNNS_Pin);
			szNetName.Kill();
		}
		for (j = 0; j < aiSet.NumElements(); j++)
		{
			pcOther = GetBus(aiSet.GetValue(j));
			for (k = 0; k < aszMembers.NumElements(); k++)
			{
				pcOther->AddMember(aszMembers.GetText(k));
			}
			pcNet->AddSubnet(pcOther->GetSubnet());
		}
		aszMembers.Kill();
		aiSet.Kill();
	}
	aiDone.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if there's no net of the name., or NULL.
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemSubnet* CKicadSchemNetlist::GetSubnetAt(SKicadSchemPoint* psPoint)
{
	size				i;
	size				j;
	CKicadSchemSubnet*	pcSubnet;

	for (i = 0; i < macSubnets.NumElements(); i++)
	{
		pcSubnet = GetSubnet(i);
		for (j = 0; j < pcSubnet->NumWires(); j++)
		{
			if (pcSubnet->GetWire(j)->Contains(psPoint))
			{
				return pcSubnet;
			}
		}
		for (j = 0; j < pcSubnet->NumPins(); j++)
		{
			if (pcSubnet->GetPin(j)->sPosition.Equals(psPoint))
			{
				return pcSubnet;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the label is on a bus.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemNetlist::IsBusLabel(CKicadSchemLabel* pcLabel)
{
	size				i;
	size				j;
	CKicadSchemBus*		pcBus;

	for (i = 0; i < macBuses.NumElements(); i++)
	{
		pcBus = GetBus(i);
		for (j = 0; j < pcBus->NumLabels(); j++)
		{
			if (pcBus->GetLabel(j) == pcLabel)
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The first bus with the name, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemBus* CKicadSchemNetlist::GetBus(char* szName)
{
	size	i;

	for (i = 0; i < macBuses.NumElements(); i++)
	{
		if ((GetBus(i)->GetName() != NULL) && (StringCompare(GetBus(i)->GetName(), szName) == 0))
		{
			return GetBus(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemNetlist::NumBuses(void) { return macBuses.NumElements(); }
CKicadSchemBus* CKicadSchemNetlist::GetBus(size uiIndex) { return (CKicadSchemBus*)macBuses.Get(uiIndex); }
size CKicadSchemNetlist::NumBusNets(void) { return macBusNets.NumElements(); }
CKicadSchemNet* CKicadSchemNetlist::GetBusNet(size uiIndex) { return (CKicadSchemNet*)macBusNets.Get(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// NULL if there's no net of the name.
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemNet* CKicadSchemNetlist::GetNet(char* szName)
{
	size				i;
	CKicadSchemNet*		pcNet;

	for (i = 0; i < macNets.NumElements(); i++)
	{
		pcNet = GetNet(i);
		if (StringCompare(pcNet->GetName(), szName) == 0)
		{
			return pcNet;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemSubnet* CKicadSchemNetlist::GetPinSubnet(CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin)
{
	size				i;
	CKicadSchemSubnet*	pcSubnet;

	for (i = 0; i < macSubnets.NumElements(); i++)
	{
		pcSubnet = GetSubnet(i);
		if (pcSubnet->GetPin(pcSymbol, pcPin) != NULL)
		{
			return pcSubnet;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The subnet of the pin numbered szPinNumber on the placed unit of the
// symbol with the reference that has it.
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemSubnet* CKicadSchemNetlist::GetPinSubnet(char* szReference, char* szPinNumber)
{
	size				i;
	size				j;
	CKicadSchemSubnet*	pcSubnet;
	SKicadSchemNetPin*	psPin;

	for (i = 0; i < macSubnets.NumElements(); i++)
	{
		pcSubnet = GetSubnet(i);
		for (j = 0; j < pcSubnet->NumPins(); j++)
		{
			psPin = pcSubnet->GetPin(j);
			if ((StringCompare(psPin->pcSymbol->GetReference(), szReference) == 0) && (StringCompare(psPin->pcPin->GetNumber(), szPinNumber) == 0))
			{
				return pcSubnet;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemNet* CKicadSchemNetlist::GetPinNet(char* szReference, char* szPinNumber)
{
	CKicadSchemSubnet*	pcSubnet;

	pcSubnet = GetPinSubnet(szReference, szPinNumber);
	if (pcSubnet == NULL)
	{
		return NULL;
	}
	return pcSubnet->GetNet();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemNetlist::NumSubnets(void) { return macSubnets.NumElements(); }
CKicadSchemSubnet* CKicadSchemNetlist::GetSubnet(size uiIndex) { return (CKicadSchemSubnet*)macSubnets.Get(uiIndex); }
size CKicadSchemNetlist::NumNets(void) { return macNets.NumElements(); }
CKicadSchemNet* CKicadSchemNetlist::GetNet(size uiIndex) { return (CKicadSchemNet*)macNets.Get(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// Union find with path halving.
//
//////////////////////////////////////////////////////////////////////////
int FindKicadSchemRoot(CArrayInt* paiParents, int iNode)
{
	int		iParent;

	for (;;)
	{
		iParent = paiParents->GetValue(iNode);
		if (iParent == iNode)
		{
			return iNode;
		}
		paiParents->SetValue(iNode, paiParents->GetValue(iParent));
		iNode = iParent;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The lower root becomes the root of both.
//
//////////////////////////////////////////////////////////////////////////
void JoinKicadSchemRoots(CArrayInt* paiParents, int iNode1, int iNode2)
{
	int		iRoot1;
	int		iRoot2;

	iRoot1 = FindKicadSchemRoot(paiParents, iNode1);
	iRoot2 = FindKicadSchemRoot(paiParents, iNode2);
	if (iRoot1 < iRoot2)
	{
		paiParents->SetValue(iRoot2, iRoot1);
	}
	else if (iRoot2 < iRoot1)
	{
		paiParents->SetValue(iRoot1, iRoot2);
	}
}

