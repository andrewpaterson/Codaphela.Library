#ifndef __LAYERED_CONNECTION_H__
#define __LAYERED_CONNECTION_H__
#include "ArrayTemplatePtr.h"
#include "MapHelper.h"
#include "DataConnection.h"
#include "Sequence.h"


//A stack of Codabases that CObjects sees as one connection (see CodabaseLayers.txt).  The top layer is writable; the
//layers below it are read-only and are only ever read.
//
//  - An object is read from the highest layer that holds it.  The same OIndex in two layers: the top one wins.
//  - Writes go to the writable layer.  A changed lower-layer object is copied up with its name and keeps its OIndex.
//  - Removing an object a lower layer holds records its OIndex in the Removed Codabase (a tombstone), so it does not
//    reappear.  Names need no tombstones: a record carries its object's name, so a name found in a lower layer only
//    counts when no higher layer holds (or removed) its OIndex.
//  - The writable sequence starts above every read-only layer's own sequence.
//
//The Codabases and sequences are owned by the caller.  The writable and Removed Codabases must be open (writes need
//their sessions).  The Removed Codabase holds a byte per removed object: CCodabaseFactory::CreateSmall makes one.  Only one index iteration and one name iteration may be in progress at a time.
class CLayeredConnection : public CDataConnection
{
protected:
	CArrayTemplatePtr<CDataReader>	mapcLayers;		//Top first.  Layer 0 is mpcWritable.
	CDataConnection*				mpcWritable;
	CDataConnection*				mpcRemoved;		//Tombstones: the OIndexes removed from the layers below the writable one.
	CSequence*						mpcSequence;
	size							miIndexLayer;	//The layer the index iteration is in.
	size							miNameLayer;	//The layer the name iteration is in.
	char							mszIterateName[MAX_KEY_SIZE + 1];

public:
	void			Init(CDataConnection* pcWritable, CDataConnection* pcRemoved, CSequence* pcSequence);
	bool			AddReadOnly(CDataReader* pcLayer, CSequenceConnection* pcLayerSequence);
	void			Kill(void) override;

	CSequence*		GetSequence(void) const;
	size			NumLayers(void) const;

	bool	Add(OIndex oi, void* pvData, lsize uiDataSize) override;
	bool	Add(OIndex oi, const char* szName, void* pvData, lsize uiDataSize) override;
	bool	Add(OIndex oi, CChars* szName, void* pvData, lsize uiDataSize) override;

	bool	Set(OIndex oi, void* pvData) override;
	bool	Set(OIndex oi, void* pvData, lsize uiDataSize) override;
	bool	Set(const char* szName, void* pvData) override;
	bool	Set(const char* szName, void* pvData, lsize uiDataSize) override;
	bool	Set(CChars* szName, void* pvData) override;
	bool	Set(CChars* szName, void* pvData, lsize uiDataSize) override;

	bool	Put(OIndex oi, void* pvData, lsize uiDataSize) override;
	bool	Put(OIndex oi, const char* szName, void* pvData, lsize uiDataSize) override;
	bool	Put(OIndex oi, CChars* szName, void* pvData, lsize uiDataSize) override;

	bool	Get(OIndex oi, void* pvData) override;
	bool	Get(OIndex oi, lsize* puiDataSize, void* pvData, lsize uiMaxDataSize) override;
	bool	Get(const char* szName, void* pvData) override;
	bool	Get(const char* szName, lsize* puiDataSize, void* pvData, lsize uiMaxDataSize) override;
	bool	Get(CChars* szName, void* pvData) override;
	bool	Get(CChars* szName, lsize* puiDataSize, void* pvData, lsize uiMaxDataSize) override;

	bool	GetName(OIndex oi, CChars* szName) override;
	bool	GetName(OIndex oi, char* szName, size* puiNameLength, size uiMaxNameLength) override;
	OIndex	GetIndex(const char* szName) override;
	OIndex	GetIndex(CChars* szName) override;

	bool	Contains(OIndex oi) override;
	bool	Contains(const char* szName) override;
	bool	Contains(CChars* szName) override;

	bool	Remove(OIndex oi) override;
	bool	Remove(const char* szName) override;
	bool	Remove(CChars* szName) override;

	bool	Flush(void) override;
	bool	Flush(bool bClearCache) override;
	bool	IsWriteThrough(void) override;

	OIndex	StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, lsize* piDataSize, lsize iMaxDataSize) override;
	OIndex	IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, lsize* piDataSize, lsize iMaxDataSize) override;
	bool	StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) override;
	bool	NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) override;

protected:
	size	FindLayer(OIndex oi);
	bool	IsRemoved(OIndex oi);
	bool	ClearRemoved(OIndex oi);
	bool	HeldBelowWritable(OIndex oi);
	bool	PutCopyingUp(OIndex oi, void* pvData, lsize uiDataSize);
	OIndex	NextIndex(SIndexTreeFileIterator* psIterator, OIndex oi, void* pvData, lsize* piDataSize, lsize iMaxDataSize);
	bool	NextName(SIndexTreeFileIterator* psIterator, bool bExists, OIndex oi, char* szKey, OIndex* poi);
};


#endif // __LAYERED_CONNECTION_H__

