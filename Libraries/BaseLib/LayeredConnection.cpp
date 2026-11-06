#include "Logger.h"
#include "PointerFunctions.h"
#include "LayeredConnection.h"


//////////////////////////////////////////////////////////////////////////
//
// pcWritable is the top layer.  pcRemoved holds the tombstones.
// pcSequence hands out the OIndexes of objects added through this
// connection.
//
//////////////////////////////////////////////////////////////////////////
void CLayeredConnection::Init(CDataConnection* pcWritable, CDataConnection* pcRemoved, CSequence* pcSequence)
{
	mapcLayers.Init();
	mapcLayers.Add(pcWritable);
	mpcWritable = pcWritable;
	mpcRemoved = pcRemoved;
	mpcSequence = pcSequence;
	miIndexLayer = 0;
	miNameLayer = 0;
	mszIterateName[0] = '\0';
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a read-only layer below the layers already added.  The writable
// sequence is moved above the layer's own sequence, so no object added
// through this connection takes an OIndex the layer already uses.
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::AddReadOnly(CDataReader* pcLayer, CSequenceConnection* pcLayerSequence)
{
	if ((pcLayer == NULL) || (pcLayerSequence == NULL))
	{
		return gcLogger.Error2(__METHOD__, " A read-only layer needs a Codabase and its sequence.", NULL);
	}

	mapcLayers.Add(pcLayer);
	return mpcSequence->AdvanceTo(pcLayerSequence->PeekNext());
}


//////////////////////////////////////////////////////////////////////////
//
// The layers and the sequence belong to the caller.
//
//////////////////////////////////////////////////////////////////////////
void CLayeredConnection::Kill(void)
{
	mapcLayers.Kill();
	mpcWritable = NULL;
	mpcRemoved = NULL;
	mpcSequence = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSequence* CLayeredConnection::GetSequence(void)
{
	return mpcSequence;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CLayeredConnection::NumLayers(void)
{
	return mapcLayers.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// The highest layer holding oi, or ARRAY_ELEMENT_NOT_FOUND if none does
// or it was removed.  The writable layer never holds a removed OIndex.
//
//////////////////////////////////////////////////////////////////////////
size CLayeredConnection::FindLayer(OIndex oi)
{
	size	i;
	size	uiLayers;

	if (mpcWritable->Contains(oi))
	{
		return 0;
	}
	if (IsRemoved(oi))
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	uiLayers = mapcLayers.NumElements();
	for (i = 1; i < uiLayers; i++)
	{
		if (mapcLayers.GetPtr(i)->Contains(oi))
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::IsRemoved(OIndex oi)
{
	return mpcRemoved->Contains(oi);
}


//////////////////////////////////////////////////////////////////////////
//
// An object written to the writable layer is no longer removed.
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::ClearRemoved(OIndex oi)
{
	if (IsRemoved(oi))
	{
		return mpcRemoved->Remove(oi);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a read-only layer holds oi, removed or not.
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::HeldBelowWritable(OIndex oi)
{
	size	i;
	size	uiLayers;

	uiLayers = mapcLayers.NumElements();
	for (i = 1; i < uiLayers; i++)
	{
		if (mapcLayers.GetPtr(i)->Contains(oi))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes oi to the writable layer.  An object only a lower layer holds is
// copied up with its name.
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::PutCopyingUp(OIndex oi, void* pvData, lsize uiDataSize)
{
	size	uiLayer;
	CChars	szName;
	bool	bResult;

	uiLayer = FindLayer(oi);
	if ((uiLayer == 0) || (uiLayer == ARRAY_ELEMENT_NOT_FOUND))
	{
		return ClearRemoved(oi) && mpcWritable->Put(oi, pvData, uiDataSize);
	}

	szName.Init();
	if (mapcLayers.GetPtr(uiLayer)->GetName(oi, &szName) && !szName.Empty())
	{
		bResult = mpcWritable->Put(oi, szName.Text(), pvData, uiDataSize);
	}
	else
	{
		bResult = mpcWritable->Put(oi, pvData, uiDataSize);
	}
	szName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Add(OIndex oi, void* pvData, lsize uiDataSize)
{
	if (Contains(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot add Index [", IndexToString(oi), "], a layer already holds it.", NULL);
	}
	return ClearRemoved(oi) && mpcWritable->Add(oi, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Add(OIndex oi, const char* szName, void* pvData, lsize uiDataSize)
{
	if (StrEmpty(szName))
	{
		return Add(oi, pvData, uiDataSize);
	}
	if (Contains(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot add Index [", IndexToString(oi), "], a layer already holds it.", NULL);
	}
	if (Contains(szName))
	{
		return gcLogger.Error2(__METHOD__, " Cannot add Name [", szName, "], a layer already holds it.", NULL);
	}
	return ClearRemoved(oi) && mpcWritable->Add(oi, szName, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Add(OIndex oi, CChars* szName, void* pvData, lsize uiDataSize)
{
	return Add(oi, szName->Text(), pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
// Changes an object without changing its size.
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Set(OIndex oi, void* pvData)
{
	size	uiLayer;
	lsize	uiDataSize;

	uiLayer = FindLayer(oi);
	if (uiLayer == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	if (uiLayer == 0)
	{
		return mpcWritable->Set(oi, pvData);
	}

	if (!mapcLayers.GetPtr(uiLayer)->Get(oi, &uiDataSize, NULL, 0))
	{
		return false;
	}
	return PutCopyingUp(oi, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Set(OIndex oi, void* pvData, lsize uiDataSize)
{
	if (!Contains(oi))
	{
		return false;
	}
	return PutCopyingUp(oi, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Set(const char* szName, void* pvData)
{
	OIndex	oi;

	oi = GetIndex(szName);
	if (oi == INVALID_O_INDEX)
	{
		return false;
	}
	return Set(oi, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Set(const char* szName, void* pvData, lsize uiDataSize)
{
	OIndex	oi;

	oi = GetIndex(szName);
	if (oi == INVALID_O_INDEX)
	{
		return false;
	}
	return Set(oi, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Set(CChars* szName, void* pvData)
{
	return Set(szName->Text(), pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Set(CChars* szName, void* pvData, lsize uiDataSize)
{
	return Set(szName->Text(), pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
// A lower-layer object keeps its name when it is copied up.
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Put(OIndex oi, void* pvData, lsize uiDataSize)
{
	return PutCopyingUp(oi, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Put(OIndex oi, const char* szName, void* pvData, lsize uiDataSize)
{
	OIndex	oiExisting;
	bool	bExists;

	if (StrEmpty(szName))
	{
		return Put(oi, pvData, uiDataSize);
	}

	//The same rules as CNamedIndexedData::Put, over all the layers: an object keeps the name it has.
	oiExisting = GetIndex(szName);
	bExists = Contains(oi);
	if (bExists && (oiExisting == oi))
	{
		return PutCopyingUp(oi, pvData, uiDataSize);
	}
	else if (!bExists && (oiExisting == INVALID_O_INDEX))
	{
		return ClearRemoved(oi) && mpcWritable->Put(oi, szName, pvData, uiDataSize);
	}
	else if (oiExisting != INVALID_O_INDEX)
	{
		return gcLogger.Error2(__METHOD__, " Cannot Put Index [", IndexToString(oi), "], Name [", szName, "] is Index [", IndexToString(oiExisting), "].", NULL);
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot Put existing Index [", IndexToString(oi), "] with a different name [", szName, "].", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Put(OIndex oi, CChars* szName, void* pvData, lsize uiDataSize)
{
	return Put(oi, szName->Text(), pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Get(OIndex oi, void* pvData)
{
	size	uiLayer;

	uiLayer = FindLayer(oi);
	if (uiLayer == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	return mapcLayers.GetPtr(uiLayer)->Get(oi, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Get(OIndex oi, lsize* puiDataSize, void* pvData, lsize uiMaxDataSize)
{
	size	uiLayer;

	uiLayer = FindLayer(oi);
	if (uiLayer == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	return mapcLayers.GetPtr(uiLayer)->Get(oi, puiDataSize, pvData, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Get(const char* szName, void* pvData)
{
	OIndex	oi;

	oi = GetIndex(szName);
	if (oi == INVALID_O_INDEX)
	{
		return false;
	}
	return Get(oi, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Get(const char* szName, lsize* puiDataSize, void* pvData, lsize uiMaxDataSize)
{
	OIndex	oi;

	oi = GetIndex(szName);
	if (oi == INVALID_O_INDEX)
	{
		return false;
	}
	return Get(oi, puiDataSize, pvData, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Get(CChars* szName, void* pvData)
{
	return Get(szName->Text(), pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Get(CChars* szName, lsize* puiDataSize, void* pvData, lsize uiMaxDataSize)
{
	return Get(szName->Text(), puiDataSize, pvData, uiMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::GetName(OIndex oi, CChars* szName)
{
	size	uiLayer;

	uiLayer = FindLayer(oi);
	if (uiLayer == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	return mapcLayers.GetPtr(uiLayer)->GetName(oi, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::GetName(OIndex oi, char* szName, size* puiNameLength, size uiMaxNameLength)
{
	size	uiLayer;

	uiLayer = FindLayer(oi);
	if (uiLayer == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	return mapcLayers.GetPtr(uiLayer)->GetName(oi, szName, puiNameLength, uiMaxNameLength);
}


//////////////////////////////////////////////////////////////////////////
//
// Names resolve top down.  A name a layer holds only counts if that layer
// is the highest holding its OIndex: a copied-up record carries the
// object's current name (or none).
//
//////////////////////////////////////////////////////////////////////////
OIndex CLayeredConnection::GetIndex(const char* szName)
{
	size	i;
	size	uiLayers;
	OIndex	oi;

	if (StrEmpty(szName))
	{
		return INVALID_O_INDEX;
	}

	uiLayers = mapcLayers.NumElements();
	for (i = 0; i < uiLayers; i++)
	{
		oi = mapcLayers.GetPtr(i)->GetIndex(szName);
		if ((oi != INVALID_O_INDEX) && (FindLayer(oi) == i))
		{
			return oi;
		}
	}
	return INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CLayeredConnection::GetIndex(CChars* szName)
{
	return GetIndex(szName->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Contains(OIndex oi)
{
	return FindLayer(oi) != ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Contains(const char* szName)
{
	return GetIndex(szName) != INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Contains(CChars* szName)
{
	return Contains(szName->Text());
}


//////////////////////////////////////////////////////////////////////////
//
// Removes oi from the writable layer, and records it as removed if a
// read-only layer holds it.
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Remove(OIndex oi)
{
	bool	bResult;
	uint8	uiRemoved;

	if (!Contains(oi))
	{
		return false;
	}

	bResult = true;
	if (mpcWritable->Contains(oi))
	{
		bResult = mpcWritable->Remove(oi);
	}
	if (bResult && HeldBelowWritable(oi))
	{
		uiRemoved = 1;
		bResult = mpcRemoved->Put(oi, &uiRemoved, sizeof(uint8));
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Remove(const char* szName)
{
	OIndex	oi;

	oi = GetIndex(szName);
	if (oi == INVALID_O_INDEX)
	{
		return false;
	}
	return Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Remove(CChars* szName)
{
	return Remove(szName->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Flush(void)
{
	bool	bResult;

	bResult = mpcWritable->Flush();
	bResult &= mpcRemoved->Flush();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::Flush(bool bClearCache)
{
	bool	bResult;

	bResult = mpcWritable->Flush(bClearCache);
	bResult &= mpcRemoved->Flush(bClearCache);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::IsWriteThrough(void)
{
	return mpcWritable->IsWriteThrough();
}


//////////////////////////////////////////////////////////////////////////
//
// Every OIndex the connection holds, once: each layer's in turn, skipping
// those a higher layer holds or that were removed.
//
//////////////////////////////////////////////////////////////////////////
OIndex CLayeredConnection::StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, lsize* piDataSize, lsize iMaxDataSize)
{
	OIndex	oi;

	miIndexLayer = 0;
	oi = mapcLayers.GetPtr(0)->StartIndexIteration(psIterator, pvData, piDataSize, iMaxDataSize);
	return NextIndex(psIterator, oi, pvData, piDataSize, iMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CLayeredConnection::IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, lsize* piDataSize, lsize iMaxDataSize)
{
	OIndex	oi;

	if (miIndexLayer >= mapcLayers.NumElements())
	{
		return INVALID_O_INDEX;
	}
	oi = mapcLayers.GetPtr(miIndexLayer)->IndexIterate(psIterator, pvData, piDataSize, iMaxDataSize);
	return NextIndex(psIterator, oi, pvData, piDataSize, iMaxDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
// oi is the current layer's iteration result.  Moves on to the next layer
// at the end of each, until an OIndex this layer is the highest to hold.
//
//////////////////////////////////////////////////////////////////////////
OIndex CLayeredConnection::NextIndex(SIndexTreeFileIterator* psIterator, OIndex oi, void* pvData, lsize* piDataSize, lsize iMaxDataSize)
{
	CDataReader*	pcLayer;

	for (;;)
	{
		pcLayer = mapcLayers.GetPtr(miIndexLayer);
		while (oi != INVALID_O_INDEX)
		{
			if (FindLayer(oi) == miIndexLayer)
			{
				return oi;
			}
			oi = pcLayer->IndexIterate(psIterator, pvData, piDataSize, iMaxDataSize);
		}

		miIndexLayer++;
		if (miIndexLayer >= mapcLayers.NumElements())
		{
			return INVALID_O_INDEX;
		}
		oi = mapcLayers.GetPtr(miIndexLayer)->StartIndexIteration(psIterator, pvData, piDataSize, iMaxDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every name the connection resolves, once, with the OIndex it resolves
// to.  szKey (at least MAX_KEY_SIZE + 1 chars) and poi may be NULL.
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	bool	bExists;
	OIndex	oi;

	miNameLayer = 0;
	bExists = mapcLayers.GetPtr(0)->StartNameIteration(psIterator, mszIterateName, &oi);
	return NextName(psIterator, bExists, oi, szKey, poi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	bool	bExists;
	OIndex	oi;

	if (miNameLayer >= mapcLayers.NumElements())
	{
		return NextName(psIterator, false, INVALID_O_INDEX, szKey, poi);
	}
	bExists = mapcLayers.GetPtr(miNameLayer)->NameIterate(psIterator, mszIterateName, &oi);
	return NextName(psIterator, bExists, oi, szKey, poi);
}


//////////////////////////////////////////////////////////////////////////
//
// bExists and oi are the current layer's iteration result.  A name counts
// in the highest layer holding its OIndex, if the whole connection
// resolves it to that OIndex.
//
//////////////////////////////////////////////////////////////////////////
bool CLayeredConnection::NextName(SIndexTreeFileIterator* psIterator, bool bExists, OIndex oi, char* szKey, OIndex* poi)
{
	CDataReader*	pcLayer;

	while (miNameLayer < mapcLayers.NumElements())
	{
		pcLayer = mapcLayers.GetPtr(miNameLayer);
		while (bExists)
		{
			if ((FindLayer(oi) == miNameLayer) && (GetIndex(mszIterateName) == oi))
			{
				if (szKey)
				{
					StrCpySafe(szKey, mszIterateName, MAX_KEY_SIZE + 1);
				}
				SafeAssign(poi, oi);
				return true;
			}
			bExists = pcLayer->NameIterate(psIterator, mszIterateName, &oi);
		}

		miNameLayer++;
		if (miNameLayer < mapcLayers.NumElements())
		{
			bExists = mapcLayers.GetPtr(miNameLayer)->StartNameIteration(psIterator, mszIterateName, &oi);
		}
	}

	SafeAssign(poi, INVALID_O_INDEX);
	if (szKey)
	{
		szKey[0] = '\0';
	}
	return false;
}

