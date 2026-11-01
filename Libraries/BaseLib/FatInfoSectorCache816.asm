; FatInfoSectorCache.cpp for the W65C816 (all but CFatInfoSectorCache::Dump, which stays C++), with the same names
; and the compiler's calling convention (CppParserLib/W65C816.txt): arguments from S + 4 after the jsl in 16 bit units,
; this first then a hidden result pointer for an object result, results in A (and X), m and x 16 bit, D kept.
;
; A function with a frame of N bytes makes it on the stack and points D at it (phd, tsc, sbc N, tcs, tcd): its locals
; are D + 1 to D + N and its arguments start at D + N + 6.  The functions here use three frames, so that the helpers
; (jsr, within this one section) find what they need in the same places:
;
;   walkers (W_FRAME):  the node at W_NODE, this at W_THIS, the arguments after it at W_ARGUMENT1 and W_ARGUMENT2.
;   drive callers (V_FRAME): the drive, the virtual function and a data pointer; this at V_THIS.
;   SFatCache takers (C_FRAME): the CFatSectorCache at C_CACHE; this at C_THIS, the SFatCache at C_SECTOR, C_STAMP.
;
; The cached sectors are a CLinkedListBlock of nodes: an SLLBlockNode (psNext first) followed by a CFatSectorCache
; followed by the sector's bytes.  The list is walked through the nodes here, as CLinkedListBlock's GetHead and
; GetNext do.

		.include "Fat816.inc"

CACHE			= SIZEOF_SLLBlockNode							; A node's CFatSectorCache.
SECTOR			= SIZEOF_CFatSectorCache						; A CFatSectorCache's sector bytes.
NODE_SECTOR		= CACHE + CFatSectorCache_muiInfoSector
NODE_LOCKS		= CACHE + CFatSectorCache_muiLocks
NODE_STAMP		= CACHE + CFatSectorCache_muiCreationStamp
LIST_HEAD		= CFatInfoSectorCache_mllcCachedSectors + CLinkedListBlock_mpsHead
VTABLE_READ		= 4												; CFileDrive's virtual functions: Kill, Read, Read (sectors),
VTABLE_WRITE	= 12											; Write, Write (sectors), Erase, GetPageSize.

W_NODE			= 1
W_EMPTY			= 5
W_OLDEST		= 9
W_STAMP			= 13			; 8 bytes.
W_WANT			= 13			; The sector FindNode looks for (before W_STAMP is used).
W_COUNT			= 21
W_FRAME			= 22
W_THIS			= W_FRAME + 6
W_ARGUMENT1		= W_FRAME + 10
W_ARGUMENT2		= W_FRAME + 14

V_DRIVE			= 1
V_FUNCTION		= 5				; The virtual function's address - 1, for rtl.
V_DATA			= 9
V_FRAME			= 12
V_THIS			= V_FRAME + 6
V_ARGUMENT1		= V_FRAME + 10
V_ARGUMENT2		= V_FRAME + 14

C_CACHE			= 1
C_FRAME			= 4
C_THIS			= C_FRAME + 6
C_SECTOR		= C_FRAME + 10
C_STAMP			= C_FRAME + 14


		.section text.FatInfoSectorCache816


; void CFatSectorCache::Init(uint32 uiInfoSector, uint64 uiCreationStamp)
		.global _CFatSectorCache__Init
_CFatSectorCache__Init:
		.width (A, X), 16
		.relax
		phd
		tsc
		tcd
		lda [D + 10]
		ldy CFatSectorCache_muiInfoSector
		sta [[D + 6]:24 + Y]
		lda [D + 12]
		ldy CFatSectorCache_muiInfoSector + 2
		sta [[D + 6]:24 + Y]
		lda [D + 14]
		ldy CFatSectorCache_muiCreationStamp
		sta [[D + 6]:24 + Y]
		lda [D + 16]
		ldy CFatSectorCache_muiCreationStamp + 2
		sta [[D + 6]:24 + Y]
		lda [D + 18]
		ldy CFatSectorCache_muiCreationStamp + 4
		sta [[D + 6]:24 + Y]
		lda [D + 20]
		ldy CFatSectorCache_muiCreationStamp + 6
		sta [[D + 6]:24 + Y]
		lda 0
		ldy CFatSectorCache_muiLocks
		sta [[D + 6]:24 + Y]
		sep m
		ldy CFatSectorCache_mbDirty
		sta [[D + 6]:24 + Y]
		rep m
		pld
		rtl


; void* CFatSectorCache::GetCache(void)
		.global _CFatSectorCache__GetCache
_CFatSectorCache__GetCache:
		.width (A, X), 16
		lda [S + 6]
		tax
		lda [S + 4]
		clc
		adc SECTOR
		bcc GetCacheReturn
		inx
GetCacheReturn:
		rtl


; void CFatSectorCache::Lock(void)
		.global _CFatSectorCache__Lock
_CFatSectorCache__Lock:
		.width (A, X), 16
		phd
		tsc
		tcd
		ldy CFatSectorCache_muiLocks
		lda [[D + 6]:24 + Y]
		inc A
		sta [[D + 6]:24 + Y]
		pld
		rtl


; bool CFatSectorCache::Unlock(void)
		.global _CFatSectorCache__Unlock
_CFatSectorCache__Unlock:
		.width (A, X), 16
		phd
		tsc
		tcd
		ldy CFatSectorCache_muiLocks
		lda [[D + 6]:24 + Y]
		beq SectorUnlockReturn
		dec A
		sta [[D + 6]:24 + Y]
		lda 1
SectorUnlockReturn:
		pld
		rtl


; void CFatSectorCache::Dirty(void)
		.global _CFatSectorCache__Dirty
_CFatSectorCache__Dirty:
		.width (A, X), 16
		phd
		tsc
		tcd
		sep m
		lda 1
		ldy CFatSectorCache_mbDirty
		sta [[D + 6]:24 + Y]
		rep m
		pld
		rtl


; void CFatInfoSectorCache::Init(CFileDrive* pcDrive, uint16 uiMinimumUnlockedCaches)
		.global _CFatInfoSectorCache__Init
_CFatInfoSectorCache__Init:
		.width (A, X), 16
		phd
		tsc
		tcd
		lda [D + 10]
		ldy CFatInfoSectorCache_mpcDrive
		sta [[D + 6]:24 + Y]
		lda [D + 12]
		ldy CFatInfoSectorCache_mpcDrive + 2
		sta [[D + 6]:24 + Y]
		; CFileDrive::GetSectorSize is muiSectorSize (a sector is 512 bytes or so).
		ldy CFileDrive_muiSectorSize
		lda [[D + 10]:24 + Y]
		ldy CFatInfoSectorCache_muiSectorSize
		sta [[D + 6]:24 + Y]
		lda [D + 14]
		ldy CFatInfoSectorCache_muiMinimumUnlockedCaches
		sta [[D + 6]:24 + Y]
		lda 0
		ldy CFatInfoSectorCache_muiCaches
		sta [[D + 6]:24 + Y]
		ldy CFatInfoSectorCache_muiCreationStamp
		sta [[D + 6]:24 + Y]
		ldy CFatInfoSectorCache_muiCreationStamp + 2
		sta [[D + 6]:24 + Y]
		ldy CFatInfoSectorCache_muiCreationStamp + 4
		sta [[D + 6]:24 + Y]
		ldy CFatInfoSectorCache_muiCreationStamp + 6
		sta [[D + 6]:24 + Y]
		lda [D + 6]
		clc
		adc CFatInfoSectorCache_mllcCachedSectors
		tax
		lda [D + 8]
		adc 0
		pha
		phx
		jsl _CLinkedListBlock__Init
		ply
		ply
		pld
		rtl


; bool CFatInfoSectorCache::Kill(void)
		.global _CFatInfoSectorCache__Kill
_CFatInfoSectorCache__Kill:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc W_FRAME
		tcs
		tcd
		stz [D + W_COUNT]
		jsr FirstNode
KillNext:
		lda [D + W_NODE]
		ora [D + W_NODE + 2]
		beq KillList
		jsr IsCaching
		beq KillUncached
		ldy NODE_LOCKS
		lda [[D + W_NODE]:24 + Y]
		clc
		adc [D + W_COUNT]
		sta [D + W_COUNT]
KillUncached:
		jsr NextNode
		bra KillNext
KillList:
		lda [D + W_THIS]
		clc
		adc CFatInfoSectorCache_mllcCachedSectors
		tax
		lda [D + W_THIS + 2]
		adc 0
		pha
		phx
		jsl _CLinkedListBlock__Kill
		ply
		ply
		lda [D + W_COUNT]
		beq KillUnlocked
		lda 0
		jmp WalkerReturn
KillUnlocked:
		lda 1
		jmp WalkerReturn


; SFatCache CFatInfoSectorCache::ReadSector(uint32 uiInfoSector)
; this, the result pointer (W_ARGUMENT1), the sector (W_ARGUMENT2).
		.global _CFatInfoSectorCache__ReadSector
_CFatInfoSectorCache__ReadSector:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc W_FRAME
		tcs
		tcd
		pei [D + W_ARGUMENT2 + 2]
		pei [D + W_ARGUMENT2]
		pei [D + W_THIS + 2]
		pei [D + W_THIS]
		jsl _CFatInfoSectorCache__GetOrCacheSector
		ply
		ply
		ply
		ply
		jsr StoreCache
		jmp WalkerReturn


; Writes the SFatCache for the CFatSectorCache* in A (low) and X (high), or for none when it is NULL, through the
; result pointer at W_ARGUMENT1.  Returns that pointer in A and X.
StoreCache:
		.width (A, X), 16
		sta [D + W_NODE]
		stx [D + W_NODE + 2]
		txa
		ora [D + W_NODE]
		beq StoreNoCache
		; The stamp's low 32 bits, then the sector after the CFatSectorCache.
		ldy CFatSectorCache_muiCreationStamp
		lda [[D + W_NODE]:24 + Y]
		ldy SFatCache_uiTimeStamp
		sta [[D + W_ARGUMENT1]:24 + Y]
		ldy CFatSectorCache_muiCreationStamp + 2
		lda [[D + W_NODE]:24 + Y]
		ldy SFatCache_uiTimeStamp + 2
		sta [[D + W_ARGUMENT1]:24 + Y]
		lda [D + W_NODE]
		clc
		adc SECTOR
		ldy SFatCache_pvCachedSector
		sta [[D + W_ARGUMENT1]:24 + Y]
		lda [D + W_NODE + 2]
		adc 0
		ldy SFatCache_pvCachedSector + 2
		sta [[D + W_ARGUMENT1]:24 + Y]
		bra StoreReturn
StoreNoCache:
		ldy SFatCache_pvCachedSector
		sta [[D + W_ARGUMENT1]:24 + Y]
		ldy SFatCache_pvCachedSector + 2
		sta [[D + W_ARGUMENT1]:24 + Y]
		lda 0xFFFF
		ldy SFatCache_uiTimeStamp
		sta [[D + W_ARGUMENT1]:24 + Y]
		ldy SFatCache_uiTimeStamp + 2
		sta [[D + W_ARGUMENT1]:24 + Y]
StoreReturn:
		ldx [D + W_ARGUMENT1 + 2]
		lda [D + W_ARGUMENT1]
		rts


; bool CFatInfoSectorCache::ValidateTimeStamp(SFatCache sSectorCache)
; The CFatSectorCache is SECTOR bytes before the sector; its stamp's low 32 bits are compared.
		.global _CFatInfoSectorCache__ValidateTimeStamp
_CFatInfoSectorCache__ValidateTimeStamp:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc C_FRAME
		tcs
		tcd
		jsr CacheOfSector
		ldy CFatSectorCache_muiCreationStamp
		lda [[D + C_CACHE]:24 + Y]
		cmp [D + C_STAMP]
		bne ValidateFalse
		ldy CFatSectorCache_muiCreationStamp + 2
		lda [[D + C_CACHE]:24 + Y]
		cmp [D + C_STAMP + 2]
		bne ValidateFalse
		lda 1
		bra CacheReturn
ValidateFalse:
		lda 0
		bra CacheReturn


; The SFatCache's CFatSectorCache, into C_CACHE.
CacheOfSector:
		.width (A, X), 16
		lda [D + C_SECTOR]
		sec
		sbc SECTOR
		sta [D + C_CACHE]
		lda [D + C_SECTOR + 2]
		sbc 0
		sta [D + C_CACHE + 2]
		rts


; void CFatInfoSectorCache::Lock(SFatCache sSectorCache)
		.global _CFatInfoSectorCache__Lock
_CFatInfoSectorCache__Lock:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc C_FRAME
		tcs
		tcd
		jsr CacheOfSector
		ldy CFatSectorCache_muiLocks
		lda [[D + C_CACHE]:24 + Y]
		inc A
		sta [[D + C_CACHE]:24 + Y]
		bra CacheReturn


; bool CFatInfoSectorCache::Unlock(SFatCache sSectorCache)
		.global _CFatInfoSectorCache__Unlock
_CFatInfoSectorCache__Unlock:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc C_FRAME
		tcs
		tcd
		jsr CacheOfSector
		ldy CFatSectorCache_muiLocks
		lda [[D + C_CACHE]:24 + Y]
		beq CacheReturn
		dec A
		sta [[D + C_CACHE]:24 + Y]
		lda 1
		bra CacheReturn


; void CFatInfoSectorCache::Dirty(SFatCache sSectorCache)
		.global _CFatInfoSectorCache__Dirty
_CFatInfoSectorCache__Dirty:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc C_FRAME
		tcs
		tcd
		jsr CacheOfSector
		sep m
		lda 1
		ldy CFatSectorCache_mbDirty
		sta [[D + C_CACHE]:24 + Y]
		rep m
CacheReturn:
		tay
		tdc
		clc
		adc C_FRAME
		tcs
		tya
		pld
		rtl


; CFatSectorCache* CFatInfoSectorCache::AddNewCache(void)
; The CFatSectorCache and the sector's bytes, zeroed.  W_EMPTY is the size, W_OLDEST the new cache.
		.global _CFatInfoSectorCache__AddNewCache
_CFatInfoSectorCache__AddNewCache:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc W_FRAME
		tcs
		tcd
		ldy CFatInfoSectorCache_muiSectorSize
		lda [[D + W_THIS]:24 + Y]
		clc
		adc SECTOR
		sta [D + W_EMPTY]
		pea 0
		pha
		lda [D + W_THIS]
		clc
		adc CFatInfoSectorCache_mllcCachedSectors
		tax
		lda [D + W_THIS + 2]
		adc 0
		pha
		phx
		jsl _CLinkedListBlock__Add
		ply
		ply
		ply
		ply
		sta [D + W_OLDEST]
		stx [D + W_OLDEST + 2]
		pea 0
		pei [D + W_EMPTY]
		pea 0
		phx
		pha
		jsl _memset
		tsc
		clc
		adc 10
		tcs
		ldx [D + W_OLDEST + 2]
		lda [D + W_OLDEST]
		jmp WalkerReturn


; CFatSectorCache* CFatInfoSectorCache::GetOrCacheSector(uint32 uiInfoSector)
; this, the sector (W_ARGUMENT1).  Returns the node's CFatSectorCache (the node + CACHE).
		.global _CFatInfoSectorCache__GetOrCacheSector
_CFatInfoSectorCache__GetOrCacheSector:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc W_FRAME
		tcs
		tcd
		; Found (as most are): nothing else matters.
		lda [D + W_ARGUMENT1]
		sta [D + W_WANT]
		lda [D + W_ARGUMENT1 + 2]
		sta [D + W_WANT + 2]
		jsr FindNode
		beq GetNotCached
		jmp NodeCacheReturn

		; Not cached: an empty cache, else a new one, else the oldest unlocked one.
GetNotCached:
		stz [D + W_EMPTY]
		stz [D + W_EMPTY + 2]
		stz [D + W_OLDEST]
		stz [D + W_OLDEST + 2]
		stz [D + W_COUNT]
		lda 0xFFFF
		sta [D + W_STAMP]
		sta [D + W_STAMP + 2]
		sta [D + W_STAMP + 4]
		sta [D + W_STAMP + 6]
		jsr FirstNode
GetScanNext:
		lda [D + W_NODE]
		ora [D + W_NODE + 2]
		bne GetScanNode
		jmp GetScanned
GetScanNode:
		lda [D + W_EMPTY]
		ora [D + W_EMPTY + 2]
		bne GetScanLocks
		jsr IsCaching
		bne GetScanLocks
		lda [D + W_NODE]
		sta [D + W_EMPTY]
		lda [D + W_NODE + 2]
		sta [D + W_EMPTY + 2]
GetScanLocks:
		ldy NODE_LOCKS
		lda [[D + W_NODE]:24 + Y]
		bne GetScanSkip
		inc [D + W_COUNT]
		; muiCreationStamp < uiOldestStamp, from the top word down.
		ldy NODE_STAMP + 6
		lda [[D + W_NODE]:24 + Y]
		cmp [D + W_STAMP + 6]
		bne GetScanCompared
		ldy NODE_STAMP + 4
		lda [[D + W_NODE]:24 + Y]
		cmp [D + W_STAMP + 4]
		bne GetScanCompared
		ldy NODE_STAMP + 2
		lda [[D + W_NODE]:24 + Y]
		cmp [D + W_STAMP + 2]
		bne GetScanCompared
		ldy NODE_STAMP
		lda [[D + W_NODE]:24 + Y]
		cmp [D + W_STAMP]
GetScanCompared:
		bcs GetScanSkip
		lda [D + W_NODE]
		sta [D + W_OLDEST]
		lda [D + W_NODE + 2]
		sta [D + W_OLDEST + 2]
		ldx 6
GetScanStamp:
		txa
		clc
		adc NODE_STAMP
		tay
		lda [[D + W_NODE]:24 + Y]
		sta [D + W_STAMP + X]
		dex
		dex
		bpl GetScanStamp
GetScanSkip:
		jsr NextNode
		jmp GetScanNext

GetScanned:
		lda [D + W_EMPTY]
		ora [D + W_EMPTY + 2]
		beq GetNoEmpty
		lda [D + W_EMPTY]
		clc
		adc CACHE
		tax
		lda [D + W_EMPTY + 2]
		adc 0
		bra GetInitialise
GetNoEmpty:
		ldy CFatInfoSectorCache_muiMinimumUnlockedCaches
		lda [D + W_COUNT]
		cmp [[D + W_THIS]:24 + Y]
		bcs GetReuse
		pei [D + W_THIS + 2]
		pei [D + W_THIS]
		jsl _CFatInfoSectorCache__AddNewCache
		ply
		ply
		phx
		tax
		pla
		bra GetInitialise
GetReuse:
		lda [D + W_OLDEST]
		clc
		adc CACHE
		sta [D + W_OLDEST]
		lda [D + W_OLDEST + 2]
		adc 0
		sta [D + W_OLDEST + 2]
		pha
		pei [D + W_OLDEST]
		pei [D + W_THIS + 2]
		pei [D + W_THIS]
		jsl _CFatInfoSectorCache__FlushCache
		ply
		ply
		ply
		ply
		and 0xFF
		bne GetFlushed
		ldx 0
		bra WalkerReturn
GetFlushed:
		ldx [D + W_OLDEST]
		lda [D + W_OLDEST + 2]
		; The cache: A high, X low.
GetInitialise:
		pha
		phx
		pei [D + W_ARGUMENT1 + 2]
		pei [D + W_ARGUMENT1]
		pei [D + W_THIS + 2]
		pei [D + W_THIS]
		jsl _CFatInfoSectorCache__InitialiseCache
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		bra WalkerReturn

		; Returns the CFatSectorCache of the node at W_NODE.
NodeCacheReturn:
		lda [D + W_NODE + 2]
		tax
		lda [D + W_NODE]
		clc
		adc CACHE
		bcc WalkerReturn
		inx
		; Returns A (and X) from a W_FRAME function.
WalkerReturn:
		tay
		tdc
		clc
		adc W_FRAME
		tcs
		tya
		pld
		rtl


; The list's first node, into W_NODE.
FirstNode:
		.width (A, X), 16
		ldy LIST_HEAD
		lda [[D + W_THIS]:24 + Y]
		sta [D + W_NODE]
		ldy LIST_HEAD + 2
		lda [[D + W_THIS]:24 + Y]
		sta [D + W_NODE + 2]
		rts


; The node after W_NODE, into W_NODE.
NextNode:
		.width (A, X), 16
		ldy SLLNode_psNext + 2
		lda [[D + W_NODE]:24 + Y]
		tax
		ldy SLLNode_psNext
		lda [[D + W_NODE]:24 + Y]
		sta [D + W_NODE]
		stx [D + W_NODE + 2]
		rts


; Z clear if W_NODE caches a sector (its sector isn't NO_SECTOR_CACHED).
IsCaching:
		.width (A, X), 16
		ldy NODE_SECTOR
		lda [[D + W_NODE]:24 + Y]
		ldy NODE_SECTOR + 2
		and [[D + W_NODE]:24 + Y]
		cmp 0xFFFF
		rts


; The node caching the sector at W_WANT, into W_NODE: Z clear if there is one, set (A = 0) if there isn't.
FindNode:
		.width (A, X), 16
		jsr FirstNode
FindNodeNext:
		lda [D + W_NODE]
		ora [D + W_NODE + 2]
		beq FindNodeReturn
		ldy NODE_SECTOR
		lda [[D + W_NODE]:24 + Y]
		cmp [D + W_WANT]
		bne FindNodeSkip
		ldy NODE_SECTOR + 2
		lda [[D + W_NODE]:24 + Y]
		cmp [D + W_WANT + 2]
		bne FindNodeSkip
		lda 1
FindNodeReturn:
		rts
FindNodeSkip:
		jsr NextNode
		bra FindNodeNext


; bool CFatInfoSectorCache::Unlock(uint32 uiSector): unlocks the sector's cache if it is cached.
		.global _CFatInfoSectorCache__Unlock__1
_CFatInfoSectorCache__Unlock__1:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc W_FRAME
		tcs
		tcd
		lda [D + W_ARGUMENT1]
		sta [D + W_WANT]
		lda [D + W_ARGUMENT1 + 2]
		sta [D + W_WANT + 2]
		jsr FindNode
		beq WalkerReturn
		ldy NODE_LOCKS
		lda [[D + W_NODE]:24 + Y]
		beq UnlockSectorTrue
		dec A
		sta [[D + W_NODE]:24 + Y]
UnlockSectorTrue:
		lda 1
		bra WalkerReturn


; SFatCache CFatInfoSectorCache::FindCachedSector(uint32 uiInfoSector)
; this, the result pointer (W_ARGUMENT1), the sector (W_ARGUMENT2).
		.global _CFatInfoSectorCache__FindCachedSector
_CFatInfoSectorCache__FindCachedSector:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc W_FRAME
		tcs
		tcd
		lda [D + W_ARGUMENT2]
		sta [D + W_WANT]
		lda [D + W_ARGUMENT2 + 2]
		sta [D + W_WANT + 2]
		jsr FindNode
		tax
		beq FindCachedStore
		lda [D + W_NODE + 2]
		tax
		lda [D + W_NODE]
		clc
		adc CACHE
		bcc FindCachedStore
		inx
FindCachedStore:
		jsr StoreCache
		bra WalkerReturn


; bool CFatInfoSectorCache::Flush(void)
		.global _CFatInfoSectorCache__Flush
_CFatInfoSectorCache__Flush:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc W_FRAME
		tcs
		tcd
		lda 1
		sta [D + W_COUNT]
		jsr FirstNode
FlushNext:
		lda [D + W_NODE]
		ora [D + W_NODE + 2]
		beq FlushReturn
		jsr IsCaching
		beq FlushSkip
		lda [D + W_NODE]
		clc
		adc CACHE
		tax
		lda [D + W_NODE + 2]
		adc 0
		pha
		phx
		pei [D + W_THIS + 2]
		pei [D + W_THIS]
		jsl _CFatInfoSectorCache__FlushCache
		ply
		ply
		ply
		ply
		and 0xFF
		bne FlushSkip
		stz [D + W_COUNT]
FlushSkip:
		jsr NextNode
		bra FlushNext
FlushReturn:
		lda [D + W_COUNT]
		jmp WalkerReturn


; uint16 CFatInfoSectorCache::GetNumAllocatedSectors(void), GetNumCachedSectors(void) and GetLockedCachedSectors(void):
; the nodes counted: all of them, those caching a sector, those locked.  W_EMPTY is which.
COUNT_ALL		= 0
COUNT_CACHED	= 1
COUNT_LOCKED	= 2
		.global _CFatInfoSectorCache__GetNumAllocatedSectors
_CFatInfoSectorCache__GetNumAllocatedSectors:
		.width (A, X), 16
		lda COUNT_ALL
		bra CountNodes
		.global _CFatInfoSectorCache__GetNumCachedSectors
_CFatInfoSectorCache__GetNumCachedSectors:
		.width (A, X), 16
		lda COUNT_CACHED
		bra CountNodes
		.global _CFatInfoSectorCache__GetLockedCachedSectors
_CFatInfoSectorCache__GetLockedCachedSectors:
		.width (A, X), 16
		lda COUNT_LOCKED
CountNodes:
		phd
		tax
		tsc
		sec
		sbc W_FRAME
		tcs
		tcd
		stx [D + W_EMPTY]
		stz [D + W_COUNT]
		jsr FirstNode
CountNext:
		lda [D + W_NODE]
		ora [D + W_NODE + 2]
		beq CountReturn
		lda [D + W_EMPTY]
		beq CountOne
		cmp COUNT_CACHED
		bne CountLocked
		jsr IsCaching
		beq CountSkip
		bra CountOne
CountLocked:
		ldy NODE_LOCKS
		lda [[D + W_NODE]:24 + Y]
		beq CountSkip
CountOne:
		inc [D + W_COUNT]
CountSkip:
		jsr NextNode
		bra CountNext
CountReturn:
		lda [D + W_COUNT]
		jmp WalkerReturn


; CFatSectorCache* CFatInfoSectorCache::InitialiseCache(uint32 uiInfoSector, CFatSectorCache* pcCachedSector)
; this, the sector (V_ARGUMENT1), the cache (V_ARGUMENT2).
		.global _CFatInfoSectorCache__InitialiseCache
_CFatInfoSectorCache__InitialiseCache:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc V_FRAME
		tcs
		tcd
		; muiCreationStamp++
		ldy CFatInfoSectorCache_muiCreationStamp
InitialiseIncrement:
		lda [[D + V_THIS]:24 + Y]
		inc A
		sta [[D + V_THIS]:24 + Y]
		bne InitialiseStamped
		iny
		iny
		cpy CFatInfoSectorCache_muiCreationStamp + 8
		bne InitialiseIncrement
InitialiseStamped:
		; pcCachedSector->Init(uiInfoSector, muiCreationStamp)
		ldx 6
InitialiseStamp:
		txa
		clc
		adc CFatInfoSectorCache_muiCreationStamp
		tay
		lda [[D + V_THIS]:24 + Y]
		pha
		txa
		clc
		adc CFatSectorCache_muiCreationStamp
		tay
		pla
		sta [[D + V_ARGUMENT2]:24 + Y]
		dex
		dex
		bpl InitialiseStamp
		lda [D + V_ARGUMENT1]
		ldy CFatSectorCache_muiInfoSector
		sta [[D + V_ARGUMENT2]:24 + Y]
		lda [D + V_ARGUMENT1 + 2]
		ldy CFatSectorCache_muiInfoSector + 2
		sta [[D + V_ARGUMENT2]:24 + Y]
		lda 0
		ldy CFatSectorCache_muiLocks
		sta [[D + V_ARGUMENT2]:24 + Y]
		sep m
		ldy CFatSectorCache_mbDirty
		sta [[D + V_ARGUMENT2]:24 + Y]
		rep m
		; mpcDrive->Read(uiInfoSector, pvData)
		ldy VTABLE_READ
		jsr DriveFunction
		lda [D + V_ARGUMENT2]
		clc
		adc SECTOR
		tax
		lda [D + V_ARGUMENT2 + 2]
		adc 0
		pha
		phx
		pea 0
		pea 0
		pei [D + V_ARGUMENT1 + 2]
		pei [D + V_ARGUMENT1]
		pei [D + V_DRIVE + 2]
		pei [D + V_DRIVE]
		phk
		per InitialiseRead - 1
		jsr CallFunction
InitialiseRead:
		.width (A, X), 16
		tay
		tsc
		clc
		adc 16
		tcs
		tya
		and 0xFF
		beq InitialiseFailed
		ldx [D + V_ARGUMENT2 + 2]
		lda [D + V_ARGUMENT2]
		bra DriveReturn
InitialiseFailed:
		ldx 0
		bra DriveReturn


; The drive (this->mpcDrive) into V_DRIVE and the address - 1 of its virtual function at offset Y in its vtable into
; V_FUNCTION.
DriveFunction:
		.width (A, X), 16
		phy
		ldy CFatInfoSectorCache_mpcDrive
		lda [[D + V_THIS]:24 + Y]
		sta [D + V_DRIVE]
		ldy CFatInfoSectorCache_mpcDrive + 2
		lda [[D + V_THIS]:24 + Y]
		sta [D + V_DRIVE + 2]
		lda [[D + V_DRIVE]:24]
		sta [D + V_FUNCTION]
		ldy 2
		lda [[D + V_DRIVE]:24 + Y]
		sta [D + V_FUNCTION + 2]
		ply
		lda [[D + V_FUNCTION]:24 + Y]
		tax
		iny
		iny
		lda [[D + V_FUNCTION]:24 + Y]
		sta [D + V_FUNCTION + 2]
		dex
		stx [D + V_FUNCTION]
		rts


; Goes to V_FUNCTION, as rtl does: the caller pushed the arguments and its return address (phk, per), and jsr'd here
; so that this return address (2 bytes) is dropped.
CallFunction:
		.width (A, X), 16
		ply
		sep m
		lda [D + V_FUNCTION + 2]
		pha
		rep m
		pei [D + V_FUNCTION]
		rtl


; bool CFatInfoSectorCache::FlushCache(CFatSectorCache* pcCachedSector)
; this, the cache (V_ARGUMENT1).
		.global _CFatInfoSectorCache__FlushCache
_CFatInfoSectorCache__FlushCache:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc V_FRAME
		tcs
		tcd
		ldy CFatSectorCache_mbDirty
		lda [[D + V_ARGUMENT1]:24 + Y]
		and 0xFF
		beq FlushCacheClean
		; mpcDrive->Write(pcCachedSector->muiInfoSector, pvData)
		ldy VTABLE_WRITE
		jsr DriveFunction
		lda [D + V_ARGUMENT1]
		clc
		adc SECTOR
		tax
		lda [D + V_ARGUMENT1 + 2]
		adc 0
		pha
		phx
		pea 0
		pea 0
		ldy CFatSectorCache_muiInfoSector + 2
		lda [[D + V_ARGUMENT1]:24 + Y]
		pha
		ldy CFatSectorCache_muiInfoSector
		lda [[D + V_ARGUMENT1]:24 + Y]
		pha
		pei [D + V_DRIVE + 2]
		pei [D + V_DRIVE]
		phk
		per FlushCacheWritten - 1
		jsr CallFunction
FlushCacheWritten:
		.width (A, X), 16
		tay
		tsc
		clc
		adc 16
		tcs
		tya
		and 0xFF
		beq DriveReturn
		sep m
		lda 0
		ldy CFatSectorCache_mbDirty
		sta [[D + V_ARGUMENT1]:24 + Y]
		rep m
FlushCacheClean:
		lda 1
DriveReturn:
		tay
		tdc
		clc
		adc V_FRAME
		tcs
		tya
		pld
		rtl
