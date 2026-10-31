; FatVolume.cpp for the W65C816, with the compiler's calling convention (see FatInfoSectorCache816.asm).  The public
; functions keep their names; the protected ones are local labels here, called with jsl as the C++ calls them (only
; CFatVolume calls them).  FAT12 isn't supported (as in the C++): only FAT16 and FAT32 volumes are mounted.
;
; Every function here has the same frame (F_FRAME bytes), so the helpers (jsr, within this one section) find this,
; the current sector cache and their scratch in the same places:
;
;   F_POINTER	scratch pointer
;   F_CACHE		an SFatCache: the sector ReadCache last read (its pointer, then its stamp)
;   F_SECTOR	a sector
;   F_OFFSET	an offset in a sector
;   F_CLUSTER	a cluster
;   F_WORK, F_WORK2	scratch
;   F_DEVICE, F_FUNCTION	the device and one of its virtual functions (DeviceFunction)
;   F_LOCALS...	each function's own
;
; this is at F_THIS and the arguments follow it (F_ARGUMENT + their offset: pointers and 32 bit values take 4
; bytes, 16 bit values, bools and enums 2).  Sector, cluster and offset arithmetic is shifts and masks: Mount only
; mounts volumes whose sector and cluster sizes are powers of two.

		.include "Fat816.inc"

VTABLE_ERASE		= 20					; CFileDrive's virtual functions: Kill, Read, Read (sectors), Write,
VTABLE_PAGE_SIZE	= 24					; Write (sectors), Erase, GetPageSize.

F_POINTER		= 1
F_CACHE			= 5				; 8 bytes: pvCachedSector, uiTimeStamp.
F_SECTOR		= 13
F_OFFSET		= 17
F_CLUSTER		= 21
F_WORK			= 25
F_WORK2			= 29
F_DEVICE		= 33
F_FUNCTION		= 37
F_COUNT			= 41				; RootDirectorySector's and RootDirectorySectorsBefore's loop count.
F_VALUE			= 43				; WriteFatEntryHere's value.
F_LOCALS		= 47
F_FRAME			= 120
F_THIS			= F_FRAME + 6
F_ARGUMENT		= F_FRAME + 10


		.section text.FatVolume816


; The frame (F_FRAME bytes) for a function called with jsl: jsr'd straight after its label.  The return address (the
; jsr's 2 bytes) is moved below the frame.
Enter:
		.width (A, X), 16
		.relax
		ply
		phd
		tsc
		sec
		sbc F_FRAME
		tcs
		tcd
		phy
		rts


; Leaves a function with A (and X) as its result.
Leave:
		.width (A, X), 16
		tay
		tdc
		clc
		adc F_FRAME
		tcs
		tya
		pld
		rtl


; Copies this's 32 bit member at offset Y to the frame at D + X.
GetMember32:
		.width (A, X), 16
		lda [[D + F_THIS]:24 + Y]
		sta [D + 0 + X]
		iny
		iny
		lda [[D + F_THIS]:24 + Y]
		sta [D + 2 + X]
		rts


; Copies the frame's D + X to this's 32 bit member at offset Y.
SetMember32:
		.width (A, X), 16
		lda [D + 0 + X]
		sta [[D + F_THIS]:24 + Y]
		iny
		iny
		lda [D + 2 + X]
		sta [[D + F_THIS]:24 + Y]
		rts


; this's 32 bit member at offset Y in A (low) and X (high).
LoadMember32:
		.width (A, X), 16
		iny
		iny
		lda [[D + F_THIS]:24 + Y]
		tax
		dey
		dey
		lda [[D + F_THIS]:24 + Y]
		rts


; this's 16 bit member at offset Y (an 8 bit member's high byte cleared with and 0xFF after).
LoadMember16:
		.width (A, X), 16
		lda [[D + F_THIS]:24 + Y]
		rts


; Z set if the volume is FAT32.
IsFat32:
		.width (A, X), 16
		ldy CFatVolume_meFileSystem
		lda [[D + F_THIS]:24 + Y]
		cmp FAT_FS_TYPE_FAT32
		rts


; &this->mcSectorCache into F_WORK2.
SectorCache:
		.width (A, X), 16
		lda [D + F_THIS]
		clc
		adc CFatVolume_mcSectorCache
		sta [D + F_WORK2]
		lda [D + F_THIS + 2]
		adc 0
		sta [D + F_WORK2 + 2]
		rts


; Reads sector F_SECTOR into the sector cache: its SFatCache into F_CACHE.  Carry set (A = FAT_CANNOT_READ_MEDIA) if
; it couldn't be read.  (READ_SECTOR.)
ReadCache:
		.width (A, X), 16
		jsr SectorCache
		pei [D + F_SECTOR + 2]
		pei [D + F_SECTOR]
		pea 0
		tdc
		clc
		adc F_CACHE
		pha
		pei [D + F_WORK2 + 2]
		pei [D + F_WORK2]
		jsl _CFatInfoSectorCache__ReadSector
		tsc
		clc
		adc 12
		tcs
		lda [D + F_CACHE]
		ora [D + F_CACHE + 2]
		bne ReadCacheRead
		lda FAT_CANNOT_READ_MEDIA
		sec
		rts
ReadCacheRead:
		clc
		rts


; Pushes F_CACHE (an SFatCache, by value) and &mcSectorCache, then calls the CFatInfoSectorCache function at X (low)
; and A (bank, as A's low byte): jsr'd to by the helpers below.  Returns what it returns, in A.
CacheCall:
		.width (A, X), 16
		stx [D + F_FUNCTION]
		sta [D + F_FUNCTION + 2]
		jsr SectorCache
		pei [D + F_CACHE + 6]
		pei [D + F_CACHE + 4]
		pei [D + F_CACHE + 2]
		pei [D + F_CACHE]
		pei [D + F_WORK2 + 2]
		pei [D + F_WORK2]
		phk
		per CacheCallReturn - 1
		jsr CallFunction
CacheCallReturn:
		.width (A, X), 16
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		rts


; mcSectorCache.Dirty(F_CACHE).
DirtyCache:
		.width (A, X), 16
		ldx _CFatInfoSectorCache__Dirty & 0xFFFF
		lda _CFatInfoSectorCache__Dirty >> 16
		bra CacheCall


; mcSectorCache.Lock(F_CACHE).
LockCache:
		.width (A, X), 16
		ldx _CFatInfoSectorCache__Lock & 0xFFFF
		lda _CFatInfoSectorCache__Lock >> 16
		bra CacheCall


; ValidateFatCache(F_CACHE): A = FAT_SUCCESS (carry clear) or FAT_INVALID_FAT_CACHE (carry set).
ValidateCache:
		.width (A, X), 16
		ldx _CFatInfoSectorCache__ValidateTimeStamp & 0xFFFF
		lda _CFatInfoSectorCache__ValidateTimeStamp >> 16
		jsr CacheCall
		and 0xFF
		beq ValidateCacheInvalid
		lda FAT_SUCCESS
		clc
		rts
ValidateCacheInvalid:
		lda FAT_INVALID_FAT_CACHE
		sec
		rts


; Goes to F_FUNCTION (an address, not - 1) as rtl does: the caller pushed the arguments and its return address (phk,
; per), and jsr'd here; this jsr's return address is dropped.
CallFunction:
		.width (A, X), 16
		ply
		sep m
		lda [D + F_FUNCTION + 2]
		pha
		rep m
		lda [D + F_FUNCTION]
		dec A
		pha
		rtl


; The device (mpcDevice) into F_DEVICE and the address of its virtual function at offset Y in its vtable into
; F_FUNCTION.
DeviceFunction:
		.width (A, X), 16
		phy
		ldy CFatVolume_mpcDevice
		ldx F_DEVICE
		jsr GetMember32
		lda [[D + F_DEVICE]:24]
		sta [D + F_FUNCTION]
		ldy 2
		lda [[D + F_DEVICE]:24 + Y]
		sta [D + F_FUNCTION + 2]
		ply
		lda [[D + F_FUNCTION]:24 + Y]
		tax
		iny
		iny
		lda [[D + F_FUNCTION]:24 + Y]
		sta [D + F_FUNCTION + 2]
		stx [D + F_FUNCTION]
		rts


; mpcDevice->Erase(F_SECTOR, F_OFFSET) (as uint64s): A is what it returns (a bool).
EraseSectors:
		.width (A, X), 16
		ldy VTABLE_ERASE
		jsr DeviceFunction
		pea 0
		pea 0
		pei [D + F_OFFSET + 2]
		pei [D + F_OFFSET]
		pea 0
		pea 0
		pei [D + F_SECTOR + 2]
		pei [D + F_SECTOR]
		pei [D + F_DEVICE + 2]
		pei [D + F_DEVICE]
		phk
		per EraseSectorsReturn - 1
		jsr CallFunction
EraseSectorsReturn:
		.width (A, X), 16
		tay
		tsc
		clc
		adc 20
		tcs
		tya
		and 0xFF
		rts


; F_SECTOR = the first sector of cluster F_CLUSTER: ((F_CLUSTER - 2) << muiSectorsPerClusterShift) + muiFirstDataSector.
FirstSector:
		.width (A, X), 16
		lda [D + F_CLUSTER]
		sec
		sbc 2
		sta [D + F_SECTOR]
		lda [D + F_CLUSTER + 2]
		sbc 0
		sta [D + F_SECTOR + 2]
		ldy CFatVolume_muiSectorsPerClusterShift
		lda [[D + F_THIS]:24 + Y]
		and 0xFF
		tax
FirstSectorShift:
		dex
		bmi FirstSectorAdd
		asl [D + F_SECTOR]
		rol [D + F_SECTOR + 2]
		bra FirstSectorShift
FirstSectorAdd:
		ldy CFatVolume_muiFirstDataSector
		ldx F_SECTOR
		bra AddMember32


; D + X += this's 32 bit member at offset Y.
AddMember32:
		.width (A, X), 16
		clc
		lda [D + 0 + X]
		adc [[D + F_THIS]:24 + Y]
		sta [D + 0 + X]
		iny
		iny
		lda [D + 2 + X]
		adc [[D + F_THIS]:24 + Y]
		sta [D + 2 + X]
		rts


; The FAT entry of cluster F_CLUSTER: its sector in F_SECTOR and its offset in that sector in F_OFFSET.
; (CalculateFATIndexAndOffset: the index is the cluster * 2 on FAT16 and * 4 on FAT32.)
FatEntryPlace:
		.width (A, X), 16
		lda [D + F_CLUSTER]
		asl A
		sta [D + F_OFFSET]
		lda [D + F_CLUSTER + 2]
		rol A
		sta [D + F_OFFSET + 2]
		jsr IsFat32
		bne FatEntryPlaced
		asl [D + F_OFFSET]
		rol [D + F_OFFSET + 2]
FatEntryPlaced:
		lda [D + F_OFFSET]
		sta [D + F_SECTOR]
		lda [D + F_OFFSET + 2]
		sta [D + F_SECTOR + 2]
		ldy CFatVolume_muiBytesPerSectorShift
		lda [[D + F_THIS]:24 + Y]
		and 0xFF
		tax
FatEntryShift:
		dex
		bmi FatEntryShifted
		lsr [D + F_SECTOR + 2]
		ror [D + F_SECTOR]
		bra FatEntryShift
FatEntryShifted:
		ldy CFatVolume_muiNoOfReservedSectors
		ldx F_SECTOR
		jsr AddMember32
		ldy CFatVolume_muiBytesPerSector
		lda [[D + F_THIS]:24 + Y]
		dec A
		and [D + F_OFFSET]
		sta [D + F_OFFSET]
		stz [D + F_OFFSET + 2]
		rts


; Carry set if X (high) and A (low) is the end of a chain (FatIsEOFEntry): 0xFFF8 or more on FAT16, 0x0FFFFFF8 or
; more on FAT32.  Keeps A and X.
IsEndOfChain:
		.width (A, X), 16
		sta [D + F_WORK]
		stx [D + F_WORK + 2]
		jsr IsFat32
		beq IsEndOfChain32
		ldy CFatVolume_meFileSystem
		lda [[D + F_THIS]:24 + Y]
		cmp FAT_FS_TYPE_FAT16
		bne IsEndOfChainNo
		lda [D + F_WORK + 2]
		bne IsEndOfChainYes
		lda [D + F_WORK]
		cmp 0xFFF8
		bra IsEndOfChainReturn
IsEndOfChain32:
		lda [D + F_WORK + 2]
		cmp 0x0FFF
		bne IsEndOfChainReturn
		lda [D + F_WORK]
		cmp 0xFFF8
		bra IsEndOfChainReturn
IsEndOfChainYes:
		sec
		bra IsEndOfChainReturn
IsEndOfChainNo:
		clc
IsEndOfChainReturn:
		lda [D + F_WORK]
		ldx [D + F_WORK + 2]
		rts


; Reads the FAT entry at F_SECTOR and F_OFFSET into F_CLUSTER (ReadFat16Entry, ReadFat32Entry).  Carry set (A the
; error) if it couldn't be read.
ReadFatEntryHere:
		.width (A, X), 16
		jsr ReadCache
		bcs ReadFatEntryReturn
		lda [D + F_CACHE]
		clc
		adc [D + F_OFFSET]
		sta [D + F_POINTER]
		lda [D + F_CACHE + 2]
		adc 0
		sta [D + F_POINTER + 2]
		lda [[D + F_POINTER]:24]
		sta [D + F_CLUSTER]
		stz [D + F_CLUSTER + 2]
		jsr IsFat32
		bne ReadFatEntryRead
		ldy 2
		lda [[D + F_POINTER]:24 + Y]
		and FAT32_CLUSTER_MASK >> 16
		sta [D + F_CLUSTER + 2]
ReadFatEntryRead:
		lda FAT_SUCCESS
		clc
ReadFatEntryReturn:
		rts


; F_CLUSTER = the cluster after F_CLUSTER in its chain (GetNextClusterEntry).  Carry set (A the error) if the FAT
; couldn't be read.
NextCluster:
		.width (A, X), 16
		jsr FatEntryPlace
		bra ReadFatEntryHere


; Stores X (high) and A (low) through the pointer at D + Y.
StoreThrough:
		.width (A, X), 16
		sty [D + F_WORK]
		phx
		ldx [D + F_WORK]
		sta [D + F_POINTER]
		lda [D + 0 + X]
		sta [D + F_WORK]
		lda [D + 2 + X]
		sta [D + F_WORK + 2]
		lda [D + F_POINTER]
		sta [[D + F_WORK]:24]
		pla
		ldy 2
		sta [[D + F_WORK]:24 + Y]
		rts


; The cluster a directory entry (the pointer at D + X) starts at, into F_CLUSTER (GetFirstClusterFromFatEntry: the
; high word only on FAT32; 0 for a NULL entry).
EntryFirstCluster:
		.width (A, X), 16
		stz [D + F_CLUSTER]
		stz [D + F_CLUSTER + 2]
		lda [D + 0 + X]
		sta [D + F_POINTER]
		lda [D + 2 + X]
		sta [D + F_POINTER + 2]
		ora [D + F_POINTER]
		beq EntryFirstClusterReturn
		ldy RAW_uiFirstClusterLowWord
		lda [[D + F_POINTER]:24 + Y]
		sta [D + F_CLUSTER]
		jsr IsFat32
		bne EntryFirstClusterReturn
		ldy RAW_uiFirstClusterHighWord
		lda [[D + F_POINTER]:24 + Y]
		sta [D + F_CLUSTER + 2]
EntryFirstClusterReturn:
		rts


; Z set if the directory entry at the pointer in D + X is NULL or starts at cluster 0 (both words 0: the root).
IsRootEntry:
		.width (A, X), 16
		lda [D + 0 + X]
		sta [D + F_POINTER]
		lda [D + 2 + X]
		sta [D + F_POINTER + 2]
		ora [D + F_POINTER]
		beq IsRootEntryReturn
		ldy RAW_uiFirstClusterLowWord
		lda [[D + F_POINTER]:24 + Y]
		ldy RAW_uiFirstClusterHighWord
		ora [[D + F_POINTER]:24 + Y]
IsRootEntryReturn:
		rts


; F_SECTOR = the first sector of the FAT16 root directory: muiNoOfReservedSectors + muiNoOfFatTables * muiFatSize
; (GetRootDirectorySector, CalculateRootSector's FAT16 half).
RootDirectorySector:
		.width (A, X), 16
		stz [D + F_SECTOR]
		stz [D + F_SECTOR + 2]
		ldy CFatVolume_muiNoOfFatTables
		lda [[D + F_THIS]:24 + Y]
		and 0xFF
		sta [D + F_COUNT]
RootDirectoryTables:
		dec [D + F_COUNT]
		bmi RootDirectoryReserved
		ldy CFatVolume_muiFatSize
		ldx F_SECTOR
		jsr AddMember32
		bra RootDirectoryTables
RootDirectoryReserved:
		ldy CFatVolume_muiNoOfReservedSectors
		ldx F_SECTOR
		jmp AddMember32


; Returns A (FAT_SUCCESS or an error) after a helper that sets carry on failure.
ReturnCarry:
		.width (A, X), 16
		jmp Leave


; Returns 0, or 1 (a bool).
ReturnFalse:
		.width (A, X), 16
		lda 0
		ldx 0
		jmp Leave
ReturnTrue:
		.width (A, X), 16
		lda 1
		ldx 0
		jmp Leave


; Getters: each the 32 bit member at the offset in X (Getter32) or the 16 bit one (Getter16) or the byte (Getter8).
; (X: Enter keeps it, not Y.)
Getter32:
		.width (A, X), 16
		jsr Enter
		txy
		jsr LoadMember32
		jmp Leave
Getter16:
		.width (A, X), 16
		jsr Enter
		txy
		jsr LoadMember16
		ldx 0
		jmp Leave
Getter8:
		.width (A, X), 16
		jsr Enter
		txy
		jsr LoadMember16
		and 0xFF
		ldx 0
		jmp Leave

		.global _CFatVolume__GetID
_CFatVolume__GetID:
		.width (A, X), 16
		ldx CFatVolume_muiID
		bra Getter32
		.global _CFatVolume__GetFatSize
_CFatVolume__GetFatSize:
		.width (A, X), 16
		ldx CFatVolume_muiFatSize
		bra Getter32
		.global _CFatVolume__GetRootCluster
_CFatVolume__GetRootCluster:
		.width (A, X), 16
		ldx CFatVolume_muiRootCluster
		bra Getter32
		.global _CFatVolume__GetRootSector
_CFatVolume__GetRootSector:
		.width (A, X), 16
		ldx CFatVolume_muiRootSector
		bra Getter32
		.global _CFatVolume__GetFirstDataSector
_CFatVolume__GetFirstDataSector:
		.width (A, X), 16
		ldx CFatVolume_muiFirstDataSector
		bra Getter32
		.global _CFatVolume__GetNoOfSectors
_CFatVolume__GetNoOfSectors:
		.width (A, X), 16
		ldx CFatVolume_muiNoOfSectors
		bra Getter32
		.global _CFatVolume__GetNoOfDataSectors
_CFatVolume__GetNoOfDataSectors:
		.width (A, X), 16
		ldx CFatVolume_muiNoOfDataSectors
		bra Getter32
		.global _CFatVolume__GetNumClusters
_CFatVolume__GetNumClusters:
		.width (A, X), 16
		ldx CFatVolume_muiNoOfClusters
		bra Getter32
		.global _CFatVolume__GetNextFreeCluster
_CFatVolume__GetNextFreeCluster:
		.width (A, X), 16
		ldx CFatVolume_muiNextFreeCluster
		bra Getter32
		.global _CFatVolume__GetTotalFreeClusters
_CFatVolume__GetTotalFreeClusters:
		.width (A, X), 16
		ldx CFatVolume_muiTotalFreeClusters
		bra Getter32
		.global _CFatVolume__GetFileSystemInfoSector
_CFatVolume__GetFileSystemInfoSector:
		.width (A, X), 16
		ldx CFatVolume_muiFileSystemInfoSector
		bra Getter32
		.global _CFatVolume__GetFileDrive
_CFatVolume__GetFileDrive:
		.width (A, X), 16
		ldx CFatVolume_mpcDevice
		bra Getter32
		.global _CFatVolume__GetRootDirectorySectors
_CFatVolume__GetRootDirectorySectors:
		.width (A, X), 16
		ldx CFatVolume_muiRootDirectorySectors
		bra Getter16
		.global _CFatVolume__GetFileSystemType
_CFatVolume__GetFileSystemType:
		.width (A, X), 16
		ldx CFatVolume_meFileSystem
		bra Getter16
		.global _CFatVolume__IsUseLongFilenames
_CFatVolume__IsUseLongFilenames:
		.width (A, X), 16
		ldx CFatVolume_mbUseLongFilenames
		bra Getter8
		.global _CFatVolume__GetNoOfFatTables
_CFatVolume__GetNoOfFatTables:
		.width (A, X), 16
		ldx CFatVolume_muiNoOfFatTables
		bra Getter8


; CFatInfoSectorCache* CFatVolume::GetSectorCache(void)
		.global _CFatVolume__GetSectorCache
_CFatVolume__GetSectorCache:
		.width (A, X), 16
		jsr Enter
		jsr SectorCache
		lda [D + F_WORK2]
		ldx [D + F_WORK2 + 2]
		jmp Leave


; char* CFatVolume::GetLabel(void)
		.global _CFatVolume__GetLabel
_CFatVolume__GetLabel:
		.width (A, X), 16
		jsr Enter
		jsr PushLabel
		jsl _CChars__Text
		ply
		ply
		jmp Leave


; Pushes &mszLabel (the return address moved over it).
PushLabel:
		.width (A, X), 16
		ply
		lda [D + F_THIS]
		clc
		adc CFatVolume_mszLabel
		tax
		lda [D + F_THIS + 2]
		adc 0
		pha
		phx
		phy
		rts


; uint32 CFatVolume::GetPageSize(void): mpcDevice->GetPageSize()
		.global _CFatVolume__GetPageSize
_CFatVolume__GetPageSize:
		.width (A, X), 16
		jsr Enter
		jsr PageSize
		jmp Leave


; mpcDevice->GetPageSize() in A (low) and X (high).
PageSize:
		.width (A, X), 16
		ldy VTABLE_PAGE_SIZE
		jsr DeviceFunction
		pei [D + F_DEVICE + 2]
		pei [D + F_DEVICE]
		phk
		per PageSizeReturn - 1
		jsr CallFunction
PageSizeReturn:
		.width (A, X), 16
		ply
		ply
		rts


; bool CFatVolume::FatIsEOFEntry(fatEntry uiFat)
		.global _CFatVolume__FatIsEOFEntry
_CFatVolume__FatIsEOFEntry:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT]
		ldx [D + F_ARGUMENT + 2]
		jsr IsEndOfChain
		bcs ReturnTrueShort
		jmp ReturnFalse
ReturnTrueShort:
		jmp ReturnTrue


; fatEntry CFatVolume::GetEndOfClusterMarker(void), GetBadClusterMarker(void) and GetClusterEntryMask(void): the
; FAT16 or FAT32 value (0 when the volume isn't mounted).  Each FAT16 value is its FAT32 one's low word, and the
; FAT32 ones' high words are all 0x0FFF.
		.global _CFatVolume__GetEndOfClusterMarker
_CFatVolume__GetEndOfClusterMarker:
		.width (A, X), 16
		jsr Enter
		lda FAT16_EOC
		bra FatMarker
		.global _CFatVolume__GetBadClusterMarker
_CFatVolume__GetBadClusterMarker:
		.width (A, X), 16
		jsr Enter
		lda FAT16_BAD_CLUSTER
		bra FatMarker
		.global _CFatVolume__GetClusterEntryMask
_CFatVolume__GetClusterEntryMask:
		.width (A, X), 16
		jsr Enter
		lda FAT16_CLUSTER_MASK
FatMarker:
		sta [D + F_WORK]
		ldy CFatVolume_meFileSystem
		lda [[D + F_THIS]:24 + Y]
		ldx FAT32_EOC >> 16
		cmp FAT_FS_TYPE_FAT32
		beq FatMarkerValue
		ldx 0
		cmp FAT_FS_TYPE_FAT16
		beq FatMarkerValue
		jmp ReturnFalse
FatMarkerValue:
		lda [D + F_WORK]
		jmp Leave


; uint32 CFatVolume::GetEntryCluster(SFatRawDirectoryEntry* psEntry): the entry's cluster masked with
; GetClusterEntryMask (both words, on FAT16 too).
		.global _CFatVolume__GetEntryCluster
_CFatVolume__GetEntryCluster:
		.width (A, X), 16
		jsr Enter
		ldy RAW_uiFirstClusterLowWord
		lda [[D + F_ARGUMENT]:24 + Y]
		sta [D + F_CLUSTER]
		ldy RAW_uiFirstClusterHighWord
		lda [[D + F_ARGUMENT]:24 + Y]
		sta [D + F_CLUSTER + 2]
		jsr IsFat32
		beq EntryCluster32
		ldy CFatVolume_meFileSystem
		lda [[D + F_THIS]:24 + Y]
		cmp FAT_FS_TYPE_FAT16
		beq EntryCluster16
		jmp ReturnFalse
EntryCluster32:
		lda [D + F_CLUSTER + 2]
		and FAT32_CLUSTER_MASK >> 16
		tax
		lda [D + F_CLUSTER]
		jmp Leave
EntryCluster16:
		lda [D + F_CLUSTER]
		ldx 0
		jmp Leave


; uint32 CFatVolume::CalculateFirstSectorOfCluster(uint32 uiCluster)
		.global _CFatVolume__CalculateFirstSectorOfCluster
_CFatVolume__CalculateFirstSectorOfCluster:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT]
		sta [D + F_CLUSTER]
		lda [D + F_ARGUMENT + 2]
		sta [D + F_CLUSTER + 2]
		jsr FirstSector
		lda [D + F_SECTOR]
		ldx [D + F_SECTOR + 2]
		jmp Leave


; void CFatVolume::CalculateFATIndexAndOffset(uint32* puiOffsetInSector, uint32 uiClusterIndex, uint32* puiSector)
		.global _CFatVolume__CalculateFATIndexAndOffset
_CFatVolume__CalculateFATIndexAndOffset:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT + 4]
		sta [D + F_CLUSTER]
		lda [D + F_ARGUMENT + 6]
		sta [D + F_CLUSTER + 2]
		jsr FatEntryPlace
		lda [D + F_OFFSET]
		ldx [D + F_OFFSET + 2]
		ldy F_ARGUMENT
		jsr StoreThrough
		lda [D + F_SECTOR]
		ldx [D + F_SECTOR + 2]
		ldy F_ARGUMENT + 8
		jsr StoreThrough
		jmp Leave


; SFatCache CFatVolume::ReadSector(uint32 uiFatInfoSector, bool bLock)
; this, the result pointer (F_ARGUMENT), the sector (F_ARGUMENT + 4), bLock (F_ARGUMENT + 8).
		.global _CFatVolume__ReadSector
_CFatVolume__ReadSector:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT + 4]
		sta [D + F_SECTOR]
		lda [D + F_ARGUMENT + 6]
		sta [D + F_SECTOR + 2]
		jsr ReadLockedCache
		ldx 6
ReadSectorCopy:
		lda [D + F_CACHE + X]
		txy
		sta [[D + F_ARGUMENT]:24 + Y]
		dex
		dex
		bpl ReadSectorCopy
		lda [D + F_ARGUMENT]
		ldx [D + F_ARGUMENT + 2]
		jmp Leave


; ReadCache, then mcSectorCache.Lock(F_CACHE) if it was read and bLock (F_ARGUMENT + 8) is true.
ReadLockedCache:
		.width (A, X), 16
		jsr ReadCache
		bcs ReadLockedCacheReturn
		lda [D + F_ARGUMENT + 8]
		and 0xFF
		beq ReadLockedCacheReturn
		jsr LockCache
ReadLockedCacheReturn:
		rts


; void CFatVolume::DirtySector(SFatCache sCachedSector)
		.global _CFatVolume__DirtySector
_CFatVolume__DirtySector:
		.width (A, X), 16
		jsr Enter
		jsr ArgumentCache
		jsr DirtyCache
		jmp Leave


; F_CACHE = the SFatCache argument (F_ARGUMENT).
ArgumentCache:
		.width (A, X), 16
		ldx 6
ArgumentCacheCopy:
		lda [D + F_ARGUMENT + X]
		sta [D + F_CACHE + X]
		dex
		dex
		bpl ArgumentCacheCopy
		rts


; EFatCode CFatVolume::ValidateFatCache(SFatCache sCache)
		.global _CFatVolume__ValidateFatCache
_CFatVolume__ValidateFatCache:
		.width (A, X), 16
		jsr Enter
		jsr ArgumentCache
		jsr ValidateCache
		jmp Leave


; EFatCode CFatVolume::ReadFatEntry(uint32 uiOffsetInSector, uint32 uiClusterIndex, uint32 uiSector, fatEntry* puiFatEntry)
		.global _CFatVolume__ReadFatEntry
_CFatVolume__ReadFatEntry:
		.width (A, X), 16
		jsr Enter
		jsr IsFat32
		beq ReadFatEntryMounted
		ldy CFatVolume_meFileSystem
		lda [[D + F_THIS]:24 + Y]
		cmp FAT_FS_TYPE_FAT16
		beq ReadFatEntryMounted
		lda FAT_UNKNOWN_ERROR
		jmp Leave
ReadFatEntryMounted:
		lda [D + F_ARGUMENT]
		sta [D + F_OFFSET]
		lda [D + F_ARGUMENT + 2]
		sta [D + F_OFFSET + 2]
		lda [D + F_ARGUMENT + 8]
		sta [D + F_SECTOR]
		lda [D + F_ARGUMENT + 10]
		sta [D + F_SECTOR + 2]
		jsr ReadFatEntryHere
		bcs ReadFatEntryFailed
		lda [D + F_CLUSTER]
		ldx [D + F_CLUSTER + 2]
		ldy F_ARGUMENT + 12
		jsr StoreThrough
		lda FAT_SUCCESS
ReadFatEntryFailed:
		jmp Leave


; EFatCode CFatVolume::GetNextClusterEntry(uint32 uiCurrentCluster, fatEntry* puiNextCluster)
		.global _CFatVolume__GetNextClusterEntry
_CFatVolume__GetNextClusterEntry:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT]
		sta [D + F_CLUSTER]
		lda [D + F_ARGUMENT + 2]
		sta [D + F_CLUSTER + 2]
		jsr NextCluster
		bcs GetNextClusterFailed
		lda [D + F_CLUSTER]
		ldx [D + F_CLUSTER + 2]
		ldy F_ARGUMENT + 4
		jsr StoreThrough
		lda FAT_SUCCESS
GetNextClusterFailed:
		jmp Leave


; EFatCode CFatVolume::SetClusterEntry(uint32 uiClusterIndex, fatEntry uiClusterInVolume): writes the cluster's FAT
; entry (SetFat16ClusterEntry, SetFat32ClusterEntry: FAT32 keeps the entry's top 4 bits... as the C++, it writes them
; 0).
		.global _CFatVolume__SetClusterEntry
_CFatVolume__SetClusterEntry:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT]
		sta [D + F_CLUSTER]
		lda [D + F_ARGUMENT + 2]
		sta [D + F_CLUSTER + 2]
		jsr FatEntryPlace
		lda [D + F_ARGUMENT + 4]
		ldx [D + F_ARGUMENT + 6]
		jsr WriteFatEntryHere
		jmp Leave


; Writes X (high) and A (low) as the FAT entry at F_SECTOR and F_OFFSET and dirties the sector: 16 bits on FAT16,
; masked to 28 bits on FAT32.  A = FAT_SUCCESS, or carry set and the error.
WriteFatEntryHere:
		.width (A, X), 16
		sta [D + F_VALUE]
		stx [D + F_VALUE + 2]
		jsr ReadCache
		bcs WriteFatEntryReturn
		lda [D + F_CACHE]
		clc
		adc [D + F_OFFSET]
		sta [D + F_POINTER]
		lda [D + F_CACHE + 2]
		adc 0
		sta [D + F_POINTER + 2]
		lda [D + F_VALUE]
		sta [[D + F_POINTER]:24]
		jsr IsFat32
		bne WriteFatEntryDirty
		lda [D + F_VALUE + 2]
		and FAT32_CLUSTER_MASK >> 16
		ldy 2
		sta [[D + F_POINTER]:24 + Y]
WriteFatEntryDirty:
		jsr DirtyCache
		lda FAT_SUCCESS
		clc
WriteFatEntryReturn:
		rts


; EFatCode CFatVolume::Flush(void)
		.global _CFatVolume__Flush
_CFatVolume__Flush:
		.width (A, X), 16
		jsr Enter
		jsr FlushCaches
		jmp Leave


; mcSectorCache.Flush(): A = FAT_SUCCESS or FAT_CANNOT_WRITE_MEDIA.
FlushCaches:
		.width (A, X), 16
		jsr SectorCache
		pei [D + F_WORK2 + 2]
		pei [D + F_WORK2]
		jsl _CFatInfoSectorCache__Flush
		ply
		ply
		and 0xFF
		beq FlushCachesFailed
		lda FAT_SUCCESS
		rts
FlushCachesFailed:
		lda FAT_CANNOT_WRITE_MEDIA
		rts


; bool CFatVolume::Erase(uint32 uiStartSector, uint32 uiStopSectorInclusive)
		.global _CFatVolume__Erase
_CFatVolume__Erase:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT]
		sta [D + F_SECTOR]
		lda [D + F_ARGUMENT + 2]
		sta [D + F_SECTOR + 2]
		lda [D + F_ARGUMENT + 4]
		sta [D + F_OFFSET]
		lda [D + F_ARGUMENT + 6]
		sta [D + F_OFFSET + 2]
		jsr EraseSectors
		ldx 0
		jmp Leave


; ------------------------------------------------------------------------------------------------------------------
; Directory queries (an SFatQueryState): QueryFirstEntry and QueryNextEntry (jsl, the C++'s arguments), the rest jsr
; helpers on the query at Q_QUERY and its current entry at Q_ENTRY.
; ------------------------------------------------------------------------------------------------------------------

Q_QUERY			= F_LOCALS			; The SFatQueryState*.
Q_ENTRY			= F_LOCALS + 4		; Its psCurrentEntryRaw.
Q_PASS			= F_LOCALS + 8
Q_FIRST			= F_LOCALS + 10		; QueryNextEntry's bFirstEntry.
Q_PREVIOUS		= F_LOCALS + 12		; The sector the query leaves.
Q_COUNT			= F_LOCALS + 16
Q_SUM			= F_LOCALS + 18
Q_RESULT		= F_LOCALS + 20		; QueryResult's SFatDirectoryEntry**.


; Q_ENTRY = the query's psCurrentEntryRaw.
QueryEntry:
		.width (A, X), 16
		ldy SFatQueryState_psCurrentEntryRaw
		lda [[D + Q_QUERY]:24 + Y]
		sta [D + Q_ENTRY]
		ldy SFatQueryState_psCurrentEntryRaw + 2
		lda [[D + Q_QUERY]:24 + Y]
		sta [D + Q_ENTRY + 2]
		rts


; The query's 8 bit field at offset Y in A (and 0xFF'd).
QueryByte:
		.width (A, X), 16
		lda [[D + Q_QUERY]:24 + Y]
		and 0xFF
		rts


; Stores A's low byte in the query's 8 bit field at offset Y.
SetQueryByte:
		.width (A, X), 16
		sep m
		sta [[D + Q_QUERY]:24 + Y]
		rep m
		rts


; The current entry's byte at offset Y in A.
EntryByte:
		.width (A, X), 16
		lda [[D + Q_ENTRY]:24 + Y]
		and 0xFF
		rts


; The query's sBuffer into F_CACHE (for ValidateCache).
QueryCache:
		.width (A, X), 16
		ldx 6
QueryCacheCopy:
		txa
		clc
		adc SFatQueryState_sBuffer
		tay
		lda [[D + Q_QUERY]:24 + Y]
		sta [D + F_CACHE + X]
		dex
		dex
		bpl QueryCacheCopy
		rts


; F_CACHE into the query's sBuffer, and its sector pointer into psFirstEntryRaw and psCurrentEntryRaw (Q_ENTRY too).
SetQueryCache:
		.width (A, X), 16
		ldx 6
SetQueryCacheCopy:
		txa
		clc
		adc SFatQueryState_sBuffer
		tay
		lda [D + F_CACHE + X]
		sta [[D + Q_QUERY]:24 + Y]
		dex
		dex
		bpl SetQueryCacheCopy
		lda [D + F_CACHE]
		sta [D + Q_ENTRY]
		ldy SFatQueryState_psFirstEntryRaw
		sta [[D + Q_QUERY]:24 + Y]
		ldy SFatQueryState_psCurrentEntryRaw
		sta [[D + Q_QUERY]:24 + Y]
		lda [D + F_CACHE + 2]
		sta [D + Q_ENTRY + 2]
		ldy SFatQueryState_psFirstEntryRaw + 2
		sta [[D + Q_QUERY]:24 + Y]
		ldy SFatQueryState_psCurrentEntryRaw + 2
		sta [[D + Q_QUERY]:24 + Y]
		rts


; Reads sector F_SECTOR locked into the query's buffer and locks it in the query (psQuery->sBuffer = ReadSector(.., true);
; psQuery->Lock).  Carry set (A the error) if it couldn't.
QueryReadSector:
		.width (A, X), 16
		jsr ReadCache
		bcs QueryReadSectorReturn
		jsr LockCache
		jsr SetQueryCache
		pei [D + F_SECTOR + 2]
		pei [D + F_SECTOR]
		pei [D + Q_QUERY + 2]
		pei [D + Q_QUERY]
		jsl _SFatQueryState__Lock
		ply
		ply
		ply
		ply
		and 0xFF
		bne QueryReadSectorLocked
		lda FAT_NO_CACHE_LOCK
		sec
		rts
QueryReadSectorLocked:
		lda FAT_SUCCESS
		clc
QueryReadSectorReturn:
		rts


; F_SECTOR = the sector the query is in: the root directory's (FAT16, cluster 0) or its cluster's first sector, plus
; uiCurrentSector.
QuerySector:
		.width (A, X), 16
		ldy SFatQueryState_uiCurrentCluster
		lda [[D + Q_QUERY]:24 + Y]
		sta [D + F_CLUSTER]
		ldy SFatQueryState_uiCurrentCluster + 2
		ora [[D + Q_QUERY]:24 + Y]
		bne QuerySectorCluster
		jsr RootDirectorySector
		bra QuerySectorAdd
QuerySectorCluster:
		lda [[D + Q_QUERY]:24 + Y]
		sta [D + F_CLUSTER + 2]
		jsr FirstSector
QuerySectorAdd:
		ldy SFatQueryState_uiCurrentSector
		lda [[D + Q_QUERY]:24 + Y]
		clc
		adc [D + F_SECTOR]
		sta [D + F_SECTOR]
		bcc QuerySectorReturn
		inc [D + F_SECTOR + 2]
QuerySectorReturn:
		rts


; EFatCode QueryFirstEntry(SFatRawDirectoryEntry* psDirectoryEntry, uint8 attributes, SFatQueryState* psQuery, bool bBufferLocked)
QF_DIRECTORY	= F_ARGUMENT
QF_ATTRIBUTES	= F_ARGUMENT + 4
QF_QUERY		= F_ARGUMENT + 6
QF_LOCKED		= F_ARGUMENT + 10
QueryFirstEntry:
		.width (A, X), 16
		jsr Enter
		lda [D + QF_QUERY]
		sta [D + Q_QUERY]
		lda [D + QF_QUERY + 2]
		sta [D + Q_QUERY + 2]
		lda 0
		ldy SFatQueryState_auiLongFilename
		sta [[D + Q_QUERY]:24 + Y]
		ldx QF_DIRECTORY
		jsr IsRootEntry
		bne QueryFirstDirectory
		ldy CFatVolume_muiRootCluster
		ldx F_CLUSTER
		jsr GetMember32
		ldy CFatVolume_muiRootSector
		ldx F_SECTOR
		jsr GetMember32
		bra QueryFirstSector
QueryFirstDirectory:
		ldy RAW_uiAttributes
		lda [[D + QF_DIRECTORY]:24 + Y]
		and FAT_ATTR_DIRECTORY
		bne QueryFirstIsDirectory
		lda FAT_NOT_A_DIRECTORY
		jmp Leave
QueryFirstIsDirectory:
		ldx QF_DIRECTORY
		jsr EntryFirstCluster
		jsr FirstSector
QueryFirstSector:
		lda [D + F_CLUSTER]
		ldy SFatQueryState_uiCurrentCluster
		sta [[D + Q_QUERY]:24 + Y]
		lda [D + F_CLUSTER + 2]
		ldy SFatQueryState_uiCurrentCluster + 2
		sta [[D + Q_QUERY]:24 + Y]
		jsr QueryReadSector
		bcc QueryFirstRead
		jmp Leave
QueryFirstRead:
		lda [D + QF_ATTRIBUTES]
		ldy SFatQueryState_uiAttributes
		jsr SetQueryByte
		lda 0
		ldy SFatQueryState_uiCurrentSector
		sta [[D + Q_QUERY]:24 + Y]
		pea 1
		pei [D + QF_LOCKED]
		pei [D + Q_QUERY + 2]
		pei [D + Q_QUERY]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl QueryNextEntry
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		jmp Leave


; EFatCode QueryNextEntry(SFatQueryState* psQuery, bool bBufferLocked, bool bFirstEntry)
QN_QUERY		= F_ARGUMENT
QN_FIRST		= F_ARGUMENT + 6
QueryNextEntry:
		.width (A, X), 16
		jsr Enter
		lda [D + QN_QUERY]
		sta [D + Q_QUERY]
		lda [D + QN_QUERY + 2]
		sta [D + Q_QUERY + 2]
		lda [D + QN_FIRST]
		and 0xFF
		sta [D + Q_FIRST]
QueryNextLoop:
		lda [D + Q_FIRST]
		bne QueryNextFirst
		jsr NextRawEntry
		cmp FAT_SHORT_NAME_FOUND
		bne QueryNextNotEnd
		lda FAT_SUCCESS
		jmp Leave
QueryNextNotEnd:
		cmp FAT_SUCCESS
		beq QueryNextEntryRead
		jmp Leave
QueryNextFirst:
		stz [D + Q_FIRST]
QueryNextEntryRead:
		jsr QueryEntry
		; A long name part that isn't free goes into the query's long name.
		ldy RAW_uiAttributes
		jsr EntryByte
		cmp FAT_ATTR_LONG_NAME
		bne QueryNextShort
		jsr IsFreeEntry
		beq QueryNextShort
		jsr LongNamePart
		ldy SFatQueryState_uiAttributes
		jsr QueryByte
		ldx 0
		cmp FAT_ATTR_LONG_NAME
		bne QueryNextPass
		inx
		bra QueryNextPass
QueryNextShort:
		ldy RAW_szShortName
		jsr EntryByte
		cmp FAT_DELETED_ENTRY
		bne QueryNextAttributes
		lda 0
		ldy SFatQueryState_uiSequence
		jsr SetQueryByte
QueryNextAttributes:
		; Each of hidden, system, volume and any long name bit the entry has, the query must ask for.
		ldx 0
		lda FAT_ATTR_HIDDEN
		jsr AttributeAllowed
		bcc QueryNextPass
		lda FAT_ATTR_SYSTEM
		jsr AttributeAllowed
		bcc QueryNextPass
		lda FAT_ATTR_VOLUME_ID
		jsr AttributeAllowed
		bcc QueryNextPass
		lda FAT_ATTR_LONG_NAME
		jsr AttributeAllowed
		bcc QueryNextPass
		inx
QueryNextPass:
		txa
		beq QueryNextLoopShort
		ldy RAW_szShortName
		jsr EntryByte
		cmp FAT_DELETED_ENTRY
		bne QueryNextFound
QueryNextLoopShort:
		jmp QueryNextLoop

QueryNextFound:
		; The long name belongs to this entry if all its parts (down to sequence 1) came just before it.
		ldy RAW_szShortName
		jsr EntryByte
		beq QueryNextLowerCase
		ldy SFatQueryState_uiSequence
		jsr QueryByte
		cmp 1
		bne QueryNextNoLongName
		jsr ShortNameChecksum
		ldy SFatQueryState_uiChecksum
		eor [[D + Q_QUERY]:24 + Y]
		and 0xFF
		beq QueryNextLowerCase
QueryNextNoLongName:
		lda 0
		ldy SFatQueryState_auiLongFilename
		sta [[D + Q_QUERY]:24 + Y]
QueryNextLowerCase:
		ldy SFatQueryState_auiLongFilename
		lda [[D + Q_QUERY]:24 + Y]
		bne QueryNextDone
		ldy RAW_uiReserved
		jsr EntryByte
		and FAT_LOWERCASE_EXTENSION | FAT_LOWERCASE_BASENAME
		beq QueryNextDone
		jsr LongNameFromShortName
QueryNextDone:
		lda 0
		ldy SFatQueryState_uiSequence
		jsr SetQueryByte
		ldy SFatQueryState_uiChecksum
		jsr SetQueryByte
		lda FAT_SUCCESS
		jmp Leave


; Carry clear if the current entry has attribute bit(s) A and the query asks for none of them (the entry doesn't
; pass).  Keeps X.
AttributeAllowed:
		.width (A, X), 16
		sta [D + Q_COUNT]
		ldy RAW_uiAttributes
		jsr EntryByte
		and [D + Q_COUNT]
		beq AttributeAllowedYes
		ldy SFatQueryState_uiAttributes
		jsr QueryByte
		and [D + Q_COUNT]
		beq AttributeAllowedNo
AttributeAllowedYes:
		sec
		rts
AttributeAllowedNo:
		clc
		rts


; Z set if the current entry is free: deleted or the directory's last (IsFreeDirectoryEntry).
IsFreeEntry:
		.width (A, X), 16
		ldy RAW_szShortName
		jsr EntryByte
		beq IsFreeEntryReturn
		cmp FAT_DELETED_ENTRY
IsFreeEntryReturn:
		rts


; The current entry's short name's checksum (FatLongEntryChecksum): in A.
ShortNameChecksum:
		.width (A, X), 16
		lda [D + Q_ENTRY]
		clc
		adc RAW_szShortName
		sta [D + F_POINTER]
		lda [D + Q_ENTRY + 2]
		adc 0
		sta [D + F_POINTER + 2]
		; Falls into Checksum.


; FatLongEntryChecksum of the 11 bytes at F_POINTER: in A (F_COUNT is its scratch: any function may call it).
Checksum:
		.width (A, X), 16
		stz [D + F_COUNT]
		ldy 0
ChecksumNext:
		lda [D + F_COUNT]
		lsr A
		bcc ChecksumEven
		ora 0x80
ChecksumEven:
		sta [D + F_COUNT]
		lda [[D + F_POINTER]:24 + Y]
		and 0xFF
		clc
		adc [D + F_COUNT]
		and 0xFF
		sta [D + F_COUNT]
		iny
		cpy 11
		bne ChecksumNext
		lda [D + F_COUNT]
		rts


; ConstructQueryLongFileNameFromShortName: ConstructFatLongFileNameFromShortName(auiLongFilename, szShortName,
; uiReserved & FAT_LOWERCASE_BASENAME, uiReserved & FAT_LOWERCASE_EXTENSION).
LongNameFromShortName:
		.width (A, X), 16
		ldy RAW_uiReserved
		jsr EntryByte
		and FAT_LOWERCASE_EXTENSION
		jsr PushBool
		ldy RAW_uiReserved
		jsr EntryByte
		and FAT_LOWERCASE_BASENAME
		jsr PushBool
		lda [D + Q_ENTRY]
		clc
		adc RAW_szShortName
		tax
		lda [D + Q_ENTRY + 2]
		adc 0
		pha
		phx
		lda [D + Q_QUERY]
		clc
		adc SFatQueryState_auiLongFilename
		tax
		lda [D + Q_QUERY + 2]
		adc 0
		pha
		phx
		jsl _ConstructFatLongFileNameFromShortName__FPtPcbb
		tsc
		clc
		adc 12
		tcs
		rts


; Pushes A as a bool (1 if it isn't 0), the return address moved over it.
PushBool:
		.width (A, X), 16
		ply
		cmp 0
		beq PushBoolFalse
		lda 1
PushBoolFalse:
		pha
		phy
		rts


; ProcessQueryLongFileNameEntry: the current entry is a long name part.
LongNamePart:
		.width (A, X), 16
		ldy LFN_uiSequence
		jsr EntryByte
		sta [D + Q_COUNT]
		and FAT_FIRST_LFN_ENTRY
		beq LongNamePartCheck
		; The last part (the first in the directory): the name's end is a 0.
		lda [D + Q_COUNT]
		eor FAT_FIRST_LFN_ENTRY
		inc A
		ldy SFatQueryState_uiSequence
		jsr SetQueryByte
		ldy LFN_uiChecksum
		jsr EntryByte
		ldy SFatQueryState_uiChecksum
		jsr SetQueryByte
		ldy SFatQueryState_uiSequence
		jsr QueryByte
		dec A
		jsr TimesThirteen
		cmp FAT_MAX_FILENAME + 1
		bcs LongNamePartCheck
		asl A
		clc
		adc SFatQueryState_auiLongFilename
		tay
		lda 0
		sta [[D + Q_QUERY]:24 + Y]
LongNamePartCheck:
		; A part with the checksum and the next sequence number (at most the 21st) is kept.
		ldy LFN_uiChecksum
		jsr EntryByte
		sta [D + Q_SUM]
		ldy SFatQueryState_uiChecksum
		jsr QueryByte
		cmp [D + Q_SUM]
		bne LongNamePartBroken
		lda [D + Q_COUNT]
		and 0xFF ^ FAT_FIRST_LFN_ENTRY
		sta [D + Q_COUNT]
		inc A
		sta [D + Q_SUM]
		ldy SFatQueryState_uiSequence
		jsr QueryByte
		cmp [D + Q_SUM]
		bne LongNamePartBroken
		cmp FAT_MAX_LFN_ENTRIES + 2
		bcs LongNamePartBroken
		lda [D + Q_COUNT]
		ldy SFatQueryState_uiSequence
		jsr SetQueryByte
		; SetQueryLongFilenamePart: its 13 characters as they are, 5 then 6 then 2 (the 20th part has room for 9).
		lda [D + Q_COUNT]
		beq LongNamePartReturn
		dec A
		jsr TimesThirteen
		asl A
		clc
		adc SFatQueryState_auiLongFilename
		adc [D + Q_QUERY]
		sta [D + F_POINTER]
		lda [D + Q_QUERY + 2]
		adc 0
		sta [D + F_POINTER + 2]
		ldx LFN_auiChars1
		ldy 0
		lda 10
		jsr CopyNamePart
		lda [D + Q_COUNT]
		cmp 20
		bcs LongNamePartLast
		ldx LFN_auiChars2
		ldy 10
		lda 12
		jsr CopyNamePart
		ldx LFN_auiChars3
		ldy 22
		lda 4
		jmp CopyNamePart
LongNamePartLast:
		ldx LFN_auiChars2
		ldy 10
		lda 8
		jmp CopyNamePart
LongNamePartBroken:
		lda 0
		ldy SFatQueryState_uiChecksum
		jsr SetQueryByte
		ldy SFatQueryState_uiSequence
		jsr SetQueryByte
LongNamePartReturn:
		rts


; A * 13 (A small).
TimesThirteen:
		.width (A, X), 16
		sta [D + F_WORK]
		asl A
		adc [D + F_WORK]
		asl A
		asl A
		adc [D + F_WORK]
		rts


; Copies A bytes from the current entry at offset X to F_POINTER + Y.
CopyNamePart:
		.width (A, X), 16
		sta [D + F_WORK]
		sep m
CopyNamePartNext:
		phy
		txy
		lda [[D + Q_ENTRY]:24 + Y]
		ply
		sta [[D + F_POINTER]:24 + Y]
		inx
		iny
		dec [D + F_WORK]
		bne CopyNamePartNext
		rep m
		rts


; FindNextRawDirectoryEntry: the query on to its next entry, reading the next sector (or cluster) when it is past
; its sector's last.  A = FAT_SUCCESS, FAT_SHORT_NAME_FOUND past the directory's end, or an error.
NextRawEntry:
		.width (A, X), 16
		jsr QueryEntry
		lda [D + Q_QUERY]
		clc
		adc SFatQueryState_sEndEntry
		cmp [D + Q_ENTRY]
		bne NextRawEntryNotEnd
		lda [D + Q_QUERY + 2]
		adc 0
		cmp [D + Q_ENTRY + 2]
		bne NextRawEntryNotEnd
		lda FAT_SHORT_NAME_FOUND
		rts
NextRawEntryNotEnd:
		jsr QueryCache
		jsr ValidateCache
		bcc NextRawEntryValid
		rts
NextRawEntryValid:
		; Past the sector's last entry?
		ldy SFatQueryState_psFirstEntryRaw
		lda [D + Q_ENTRY]
		sec
		sbc [[D + Q_QUERY]:24 + Y]
		clc
		adc SIZEOF_SFatRawDirectoryEntry
		ldy CFatVolume_muiBytesPerSector
		cmp [[D + F_THIS]:24 + Y]
		beq NextRawEntrySector
		lda [D + Q_ENTRY]
		clc
		adc SIZEOF_SFatRawDirectoryEntry
		ldy SFatQueryState_psCurrentEntryRaw
		sta [[D + Q_QUERY]:24 + Y]
		lda [D + Q_ENTRY + 2]
		adc 0
		ldy SFatQueryState_psCurrentEntryRaw + 2
		sta [[D + Q_QUERY]:24 + Y]
		lda FAT_SUCCESS
		rts

NextRawEntrySector:
		; Only the sector the query is in stays locked: the one it leaves is unlocked once the next is read.
		jsr QuerySector
		lda [D + F_SECTOR]
		sta [D + Q_PREVIOUS]
		lda [D + F_SECTOR + 2]
		sta [D + Q_PREVIOUS + 2]
		ldy SFatQueryState_uiCurrentCluster
		lda [[D + Q_QUERY]:24 + Y]
		ldy SFatQueryState_uiCurrentCluster + 2
		ora [[D + Q_QUERY]:24 + Y]
		beq NextRawEntryNextSector
		ldy CFatVolume_muiNoOfSectorsPerCluster
		lda [[D + F_THIS]:24 + Y]
		dec A
		ldy SFatQueryState_uiCurrentSector
		cmp [[D + Q_QUERY]:24 + Y]
		bne NextRawEntryNextSector
		; The cluster's last sector: the chain's next cluster, or the end.
		ldy SFatQueryState_uiCurrentCluster
		lda [[D + Q_QUERY]:24 + Y]
		sta [D + F_CLUSTER]
		ldy SFatQueryState_uiCurrentCluster + 2
		lda [[D + Q_QUERY]:24 + Y]
		sta [D + F_CLUSTER + 2]
		jsr NextCluster
		bcc NextRawEntryNextCluster
		rts
NextRawEntryNextCluster:
		lda [D + F_CLUSTER]
		ldx [D + F_CLUSTER + 2]
		jsr IsEndOfChain
		bcs EndQuery
		lda [D + F_CLUSTER]
		ldy SFatQueryState_uiCurrentCluster
		sta [[D + Q_QUERY]:24 + Y]
		lda [D + F_CLUSTER + 2]
		ldy SFatQueryState_uiCurrentCluster + 2
		sta [[D + Q_QUERY]:24 + Y]
		lda 0
		ldy SFatQueryState_uiCurrentSector
		sta [[D + Q_QUERY]:24 + Y]
		bra NextRawEntryRead
NextRawEntryNextSector:
		ldy SFatQueryState_uiCurrentSector
		lda [[D + Q_QUERY]:24 + Y]
		inc A
		sta [[D + Q_QUERY]:24 + Y]
		ldy SFatQueryState_uiCurrentCluster
		lda [[D + Q_QUERY]:24 + Y]
		ldy SFatQueryState_uiCurrentCluster + 2
		ora [[D + Q_QUERY]:24 + Y]
		bne NextRawEntryRead
		; The FAT16 root directory is a run of sectors.
		ldy SFatQueryState_uiCurrentSector
		lda [[D + Q_QUERY]:24 + Y]
		ldy CFatVolume_muiRootDirectorySectors
		cmp [[D + F_THIS]:24 + Y]
		beq EndQuery
NextRawEntryRead:
		jsr QuerySector
		jsr QueryCache
		jsr ValidateCache
		bcs NextRawEntryReturn
		jsr QueryReadSector
		bcs NextRawEntryReturn
		jsr SectorCache
		pei [D + F_WORK2 + 2]
		pei [D + F_WORK2]
		pei [D + Q_PREVIOUS + 2]
		pei [D + Q_PREVIOUS]
		pei [D + Q_QUERY + 2]
		pei [D + Q_QUERY]
		jsl _SFatQueryState__Unlock
		tsc
		clc
		adc 12
		tcs
		lda FAT_SUCCESS
NextRawEntryReturn:
		rts


; EndQuery: the query is past the directory's last sector.  Its current entry is its own zeroed sEndEntry; A =
; FAT_SHORT_NAME_FOUND.
EndQuery:
		.width (A, X), 16
		lda [D + Q_QUERY]
		clc
		adc SFatQueryState_sEndEntry
		sta [D + Q_ENTRY]
		ldy SFatQueryState_psCurrentEntryRaw
		sta [[D + Q_QUERY]:24 + Y]
		lda [D + Q_QUERY + 2]
		adc 0
		sta [D + Q_ENTRY + 2]
		ldy SFatQueryState_psCurrentEntryRaw + 2
		sta [[D + Q_QUERY]:24 + Y]
		lda 0
		ldy SIZEOF_SFatRawDirectoryEntry - 2
EndQueryClear:
		sta [[D + Q_ENTRY]:24 + Y]
		dey
		dey
		bpl EndQueryClear
		lda FAT_SHORT_NAME_FOUND
		rts


; ------------------------------------------------------------------------------------------------------------------
; Mounting
; ------------------------------------------------------------------------------------------------------------------

M_MBR			= F_LOCALS			; The MBR's SFatCache (8 bytes).
M_HIDDEN		= F_LOCALS + 8
M_TRIED			= F_LOCALS + 12
M_PARTITION		= F_LOCALS + 14		; SFatPartitionEntry*.
M_SHIFT			= F_LOCALS + 18
M_BPB			= F_LOCALS + 20		; &msBPB.
M_QUERY			= F_LOCALS + 24		; InitialiseVolumeLabel's SFatQueryState (below the frame).
M_RESULT		= F_LOCALS + 28


; this's 16 bit BPB field at offset Y in A.
BPBWord:
		.width (A, X), 16
		lda [[D + M_BPB]:24 + Y]
		rts


; this's 8 bit BPB field at offset Y in A.
BPBByte:
		.width (A, X), 16
		lda [[D + M_BPB]:24 + Y]
		and 0xFF
		rts


; this's 32 bit BPB field at offset Y into D + X.
BPBLong:
		.width (A, X), 16
		lda [[D + M_BPB]:24 + Y]
		sta [D + 0 + X]
		iny
		iny
		lda [[D + M_BPB]:24 + Y]
		sta [D + 2 + X]
		rts


; The power of two A is (PowerOfTwoShift), in A.
PowerOfTwo:
		.width (A, X), 16
		ldx 0
PowerOfTwoNext:
		lsr A
		beq PowerOfTwoDone
		inx
		bra PowerOfTwoNext
PowerOfTwoDone:
		txa
		rts


; D + X >>= the shift in A (32 bits).
ShiftRight32:
		.width (A, X), 16
		tay
ShiftRight32Next:
		dey
		bmi ShiftRight32Done
		lsr [D + 2 + X]
		ror [D + 0 + X]
		bra ShiftRight32Next
ShiftRight32Done:
		rts


; Copies (A) the label's 11 bytes at the pointer in F_POINTER into mszLabel, stripped (mszLabel.Kill(), Init(sz, 0,
; 10), StripWhitespace()).
SetLabel:
		.width (A, X), 16
		jsr PushLabel
		jsl _CChars__Kill
		ply
		ply
		pea 10
		pea 0
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		jsr PushLabel
		jsl _CChars__Init__6
		tsc
		clc
		adc 12
		tcs
		jsr PushLabel
		jsl _CChars__StripWhitespace
		ply
		ply
		rts


; F_POINTER = &msBPB + Y.
BPBPointer:
		.width (A, X), 16
		tya
		clc
		adc [D + M_BPB]
		sta [D + F_POINTER]
		lda [D + M_BPB + 2]
		adc 0
		sta [D + F_POINTER + 2]
		rts


; InitialiseVolumeParametersFromBPB(&uiPartitionsTried, &msBPB, uiHiddenSectors): the volume's sizes and places from
; its BIOS parameter block.  Carry set (and M_TRIED counted) if it isn't a FAT16 or FAT32 volume this can mount.
; The sector and cluster sizes are powers of two (FindBiosParameterBlock checked).
VolumeParameters:
		.width (A, X), 16
		ldy BPB_uiBytsPerSector
		jsr BPBWord
		ldy CFatVolume_muiBytesPerSector
		sta [[D + F_THIS]:24 + Y]
		jsr PowerOfTwo
		sta [D + M_SHIFT]
		ldy CFatVolume_muiBytesPerSectorShift
		jsr SetMemberByte
		ldy BPB_uiSectorsPerCluster
		jsr BPBByte
		ldy CFatVolume_muiNoOfSectorsPerCluster
		sta [[D + F_THIS]:24 + Y]
		jsr PowerOfTwo
		ldy CFatVolume_muiSectorsPerClusterShift
		jsr SetMemberByte
		clc
		adc [D + M_SHIFT]
		ldy CFatVolume_muiBytesPerClusterShift
		jsr SetMemberByte
		; muiBytesPerCluster = 1 << the shift.
		tax
		lda 1
		sta [D + F_WORK]
		stz [D + F_WORK + 2]
VolumeClusterBytes:
		dex
		bmi VolumeClusterBytesDone
		asl [D + F_WORK]
		rol [D + F_WORK + 2]
		bra VolumeClusterBytes
VolumeClusterBytesDone:
		ldy CFatVolume_muiBytesPerCluster
		ldx F_WORK
		jsr SetMember32
		ldy BPB_uiNumFileAllocationTables
		jsr BPBByte
		ldy CFatVolume_muiNoOfFatTables
		jsr SetMemberByte

		; muiRootDirectorySectors = (uiRootEntryCount * 32 + muiBytesPerSector - 1) >> the shift.
		ldy BPB_uiRootEntryCount
		jsr BPBWord
		sta [D + F_WORK]
		stz [D + F_WORK + 2]
		ldx 5
VolumeRootEntries:
		asl [D + F_WORK]
		rol [D + F_WORK + 2]
		dex
		bne VolumeRootEntries
		ldy CFatVolume_muiBytesPerSector
		lda [[D + F_THIS]:24 + Y]
		dec A
		clc
		adc [D + F_WORK]
		sta [D + F_WORK]
		bcc VolumeRootEntriesAdded
		inc [D + F_WORK + 2]
VolumeRootEntriesAdded:
		lda [D + M_SHIFT]
		ldx F_WORK
		jsr ShiftRight32
		lda [D + F_WORK]
		ldy CFatVolume_muiRootDirectorySectors
		sta [[D + F_THIS]:24 + Y]

		; muiFatSize and muiNoOfSectors: the FAT16 fields, else the FAT32 ones.
		ldy BPB_uiFATSzFat16
		jsr BPBWord
		sta [D + F_WORK]
		stz [D + F_WORK + 2]
		bne VolumeFatSize
		ldy BPB32_uiFATSzFat32
		ldx F_WORK
		jsr BPBLong
VolumeFatSize:
		ldy CFatVolume_muiFatSize
		ldx F_WORK
		jsr SetMember32
		ldy BPB_uiTotalSectorsFat16
		jsr BPBWord
		sta [D + F_WORK]
		stz [D + F_WORK + 2]
		bne VolumeSectors
		ldy BPB_uiTotalSectorsFat32
		ldx F_WORK
		jsr BPBLong
VolumeSectors:
		ldy CFatVolume_muiNoOfSectors
		ldx F_WORK
		jsr SetMember32

		; F_SECTOR = uiReservedSectorCount + the FATs + muiRootDirectorySectors (the sectors before the data).
		jsr RootDirectorySectorsBefore
		; muiNoOfDataSectors = muiNoOfSectors - those; muiNoOfClusters = that >> the cluster shift.
		lda [D + F_WORK]
		sec
		sbc [D + F_SECTOR]
		sta [D + F_WORK]
		lda [D + F_WORK + 2]
		sbc [D + F_SECTOR + 2]
		sta [D + F_WORK + 2]
		ldy CFatVolume_muiNoOfDataSectors
		ldx F_WORK
		jsr SetMember32
		ldy CFatVolume_muiSectorsPerClusterShift
		lda [[D + F_THIS]:24 + Y]
		and 0xFF
		ldx F_WORK
		jsr ShiftRight32
		ldy CFatVolume_muiNoOfClusters
		ldx F_WORK
		jsr SetMember32

		; FAT16 below 65525 clusters, else FAT32; FAT12 (4084 clusters or fewer) isn't mounted.
		lda FAT_FS_TYPE_FAT32
		ldx [D + F_WORK + 2]
		bne VolumeFat32
		ldx [D + F_WORK]
		cpx FAT16_MAX_CLUSTERS + 1
		bcs VolumeFat32
		cpx FAT12_MAX_CLUSTERS + 1
		bcs VolumeFat16
		jmp VolumeNotMounted
VolumeFat16:
		lda FAT_FS_TYPE_FAT16
VolumeFat32:
		ldy CFatVolume_meFileSystem
		sta [[D + F_THIS]:24 + Y]

		; CheckFileAllocationTableLargeEnough: muiFatSize >= (clusters * 2 (or 4) + muiBytesPerSector - 1) >> the shift.
		asl [D + F_WORK]
		rol [D + F_WORK + 2]
		jsr IsFat32
		bne VolumeFatEntries
		asl [D + F_WORK]
		rol [D + F_WORK + 2]
VolumeFatEntries:
		ldy CFatVolume_muiBytesPerSector
		lda [[D + F_THIS]:24 + Y]
		dec A
		clc
		adc [D + F_WORK]
		sta [D + F_WORK]
		bcc VolumeFatEntriesAdded
		inc [D + F_WORK + 2]
VolumeFatEntriesAdded:
		lda [D + M_SHIFT]
		ldx F_WORK
		jsr ShiftRight32
		ldy CFatVolume_muiFatSize + 2
		lda [[D + F_THIS]:24 + Y]
		cmp [D + F_WORK + 2]
		bne VolumeFatCompared
		ldy CFatVolume_muiFatSize
		lda [[D + F_THIS]:24 + Y]
		cmp [D + F_WORK]
VolumeFatCompared:
		bcs VolumeLargeEnough
		jmp VolumeNotMounted
VolumeLargeEnough:

		; muiHiddenSectors, muiNoOfReservedSectors (with the hidden sectors), muiFirstDataSector, the information
		; sector.
		ldy CFatVolume_muiHiddenSectors
		ldx M_HIDDEN
		jsr SetMember32
		ldy BPB_uiReservedSectorCount
		jsr BPBWord
		clc
		adc [D + M_HIDDEN]
		sta [D + F_WORK]
		lda [D + M_HIDDEN + 2]
		adc 0
		sta [D + F_WORK + 2]
		ldy CFatVolume_muiNoOfReservedSectors
		ldx F_WORK
		jsr SetMember32
		lda [D + F_SECTOR]
		clc
		adc [D + M_HIDDEN]
		sta [D + F_SECTOR]
		lda [D + F_SECTOR + 2]
		adc [D + M_HIDDEN + 2]
		sta [D + F_SECTOR + 2]
		ldy CFatVolume_muiFirstDataSector
		ldx F_SECTOR
		jsr SetMember32
		ldy BPB32_uiFileSystemInformation
		jsr BPBWord
		sta [D + F_WORK]
		stz [D + F_WORK + 2]
		ldy CFatVolume_muiFileSystemInfoSector
		ldx F_WORK
		jsr SetMember32

		; The label and ID, and the root.
		jsr IsFat32
		beq VolumeLabel32
		ldy BPB16_uiVolumeID
		ldx F_WORK
		jsr BPBLong
		ldy BPB16_szVolumeLabel
		bra VolumeLabel
VolumeLabel32:
		ldy BPB32_uiVolumeID
		ldx F_WORK
		jsr BPBLong
		ldy BPB32_szVolumeLabel
VolumeLabel:
		jsr BPBPointer
		ldy CFatVolume_muiID
		ldx F_WORK
		jsr SetMember32
		jsr SetLabel
		stz [D + F_CLUSTER]
		stz [D + F_CLUSTER + 2]
		jsr IsFat32
		bne VolumeRoot16
		ldy BPB32_uiRootCluster
		ldx F_CLUSTER
		jsr BPBLong
		jsr FirstSector
		bra VolumeRoot
VolumeRoot16:
		jsr RootDirectorySector
VolumeRoot:
		ldy CFatVolume_muiRootCluster
		ldx F_CLUSTER
		jsr SetMember32
		ldy CFatVolume_muiRootSector
		ldx F_SECTOR
		jsr SetMember32
		clc
		rts
VolumeNotMounted:
		inc [D + M_TRIED]
		sec
		rts


; Stores A's low byte in this's 8 bit member at offset Y.  Keeps A.
SetMemberByte:
		.width (A, X), 16
		sep m
		sta [[D + F_THIS]:24 + Y]
		rep m
		rts


; F_SECTOR = the BPB's uiReservedSectorCount + uiNumFileAllocationTables * muiFatSize + muiRootDirectorySectors.
RootDirectorySectorsBefore:
		.width (A, X), 16
		ldy BPB_uiReservedSectorCount
		jsr BPBWord
		sta [D + F_SECTOR]
		stz [D + F_SECTOR + 2]
		ldy CFatVolume_muiRootDirectorySectors
		lda [[D + F_THIS]:24 + Y]
		clc
		adc [D + F_SECTOR]
		sta [D + F_SECTOR]
		bcc RootSectorsTables
		inc [D + F_SECTOR + 2]
RootSectorsTables:
		ldy BPB_uiNumFileAllocationTables
		jsr BPBByte
		sta [D + F_COUNT]
RootSectorsTable:
		dec [D + F_COUNT]
		bmi RootSectorsDone
		ldy CFatVolume_muiFatSize
		ldx F_SECTOR
		jsr AddMember32
		bra RootSectorsTable
RootSectorsDone:
		rts


; EFatCode CFatVolume::Mount(CFileDrive* pcDevice)
		.global _CFatVolume__Mount
_CFatVolume__Mount:
		.width (A, X), 16
		jsr Enter
		; mcSectorCache.Init(pcDevice, 3), mszLabel.Init()
		jsr SectorCache
		pea 3
		pei [D + F_ARGUMENT + 2]
		pei [D + F_ARGUMENT]
		pei [D + F_WORK2 + 2]
		pei [D + F_WORK2]
		jsl _CFatInfoSectorCache__Init
		tsc
		clc
		adc 10
		tcs
		jsr PushLabel
		jsl _CChars__Init
		ply
		ply
		; Not mounted until the BIOS parameter block is found: unmounting after a mount that fails writes nothing.
		lda FAT_FS_TYPE_UNSPECIFIED
		ldy CFatVolume_meFileSystem
		sta [[D + F_THIS]:24 + Y]
		lda 0xFFFF
		sta [D + F_WORK]
		sta [D + F_WORK + 2]
		ldy CFatVolume_muiFileSystemInfoSector
		ldx F_WORK
		jsr SetMember32
		ldy CFatVolume_mpcDevice
		ldx F_ARGUMENT
		jsr SetMember32
		lda [D + F_THIS]
		clc
		adc CFatVolume_msBPB
		sta [D + M_BPB]
		lda [D + F_THIS + 2]
		adc 0
		sta [D + M_BPB + 2]

		stz [D + F_SECTOR]
		stz [D + F_SECTOR + 2]
		jsr ReadCache
		bcc MountRead
		jmp Leave
MountRead:
		jsr FindBiosParameterBlock
		bcc MountFound
		jmp Leave
MountFound:
		jsr InitialiseVolumeLabel
		cmp FAT_SUCCESS
		beq MountLabelled
		jmp Leave
MountLabelled:
		; A valid FAT32 information sector gives the free clusters.
		jsr IsFat32
		bne MountMounted
		ldy CFatVolume_muiFileSystemInfoSector
		jsr LoadMember32
		sta [D + F_SECTOR]
		stx [D + F_SECTOR + 2]
		ora [D + F_SECTOR + 2]
		beq MountMounted
		lda [D + F_SECTOR]
		and [D + F_SECTOR + 2]
		cmp 0xFFFF
		beq MountMounted
		jsr Fat32FileSystemInfo
		bcc MountMounted
		jmp Leave
MountMounted:
		lda FAT_SUCCESS
		jmp Leave


; FindBiosParameterBlock(sMBRSector = F_CACHE): the partitionless volume, else the MBR's partitions in turn.  Carry
; set and A the error if none is a FAT16 or FAT32 volume.
FindBiosParameterBlock:
		.width (A, X), 16
		ldx 6
FindBPBKeepMBR:
		lda [D + F_CACHE + X]
		sta [D + M_MBR + X]
		dex
		dex
		bpl FindBPBKeepMBR
		jsr ValidateCache
		bcc FindBPBValid
		rts
FindBPBValid:
		stz [D + M_HIDDEN]
		stz [D + M_HIDDEN + 2]
		lda [D + F_CACHE]
		ldx [D + F_CACHE + 2]
		jsr CopyBPB
		stz [D + M_TRIED]
		lda [D + F_CACHE]
		clc
		adc 0x1BE
		sta [D + M_PARTITION]
		lda [D + F_CACHE + 2]
		adc 0
		sta [D + M_PARTITION + 2]
FindBPBNext:
		ldx 6
FindBPBMBR:
		lda [D + M_MBR + X]
		sta [D + F_CACHE + X]
		dex
		dex
		bpl FindBPBMBR
		jsr ValidateCache
		bcc FindBPBMBRValid
		rts
FindBPBMBRValid:
		lda [D + M_TRIED]
		beq FindBPBCheck
		; Every partition tried: not a volume.
		cmp 5
		bcc FindBPBPartition
		lda FAT_INVALID_FAT_VOLUME
		sec
		rts
FindBPBPartition:
		cmp 2
		bcc FindBPBThisPartition
		lda [D + M_PARTITION]
		clc
		adc SIZEOF_SFatPartitionEntry
		sta [D + M_PARTITION]
		bcc FindBPBThisPartition
		inc [D + M_PARTITION + 2]
FindBPBThisPartition:
		ldy SFatPartitionEntry_uiLBAFirstSector
		lda [[D + M_PARTITION]:24 + Y]
		sta [D + M_HIDDEN]
		sta [D + F_SECTOR]
		iny
		iny
		lda [[D + M_PARTITION]:24 + Y]
		sta [D + M_HIDDEN + 2]
		sta [D + F_SECTOR + 2]
		; It must be on the device: first + total <= GetTotalSectors() (muiMaxSector - 1), in 32 bits as the C++.
		ldy SFatPartitionEntry_uiTotalSectors
		lda [[D + M_PARTITION]:24 + Y]
		clc
		adc [D + M_HIDDEN]
		sta [D + F_WORK]
		iny
		iny
		lda [[D + M_PARTITION]:24 + Y]
		adc [D + M_HIDDEN + 2]
		sta [D + F_WORK + 2]
		ldy CFatVolume_mpcDevice
		ldx F_POINTER
		jsr GetMember32
		ldy CFileDrive_muiMaxSector + 4
		lda [[D + F_POINTER]:24 + Y]
		iny
		iny
		ora [[D + F_POINTER]:24 + Y]
		bne FindBPBOnDevice
		ldy CFileDrive_muiMaxSector
		lda [[D + F_POINTER]:24 + Y]
		sec
		sbc 1
		sta [D + F_WORK2]
		iny
		iny
		lda [[D + F_POINTER]:24 + Y]
		sbc 0
		cmp [D + F_WORK + 2]
		bne FindBPBDeviceCompared
		lda [D + F_WORK2]
		cmp [D + F_WORK]
FindBPBDeviceCompared:
		bcs FindBPBOnDevice
		jmp FindBPBTryNext
FindBPBOnDevice:
		jsr ReadCache
		bcc FindBPBPartitionRead
		rts
FindBPBPartitionRead:
		lda [D + F_CACHE]
		ldx [D + F_CACHE + 2]
		jsr CopyBPB
FindBPBCheck:
		; A sector size above what this allows isn't mounted at all.
		ldy BPB_uiBytsPerSector
		jsr BPBWord
		cmp MAX_SECTOR_LENGTH + 1
		bcc FindBPBSectorSize
		lda FAT_SECTOR_SIZE_NOT_SUPPORTED
		sec
		rts
FindBPBSectorSize:
		; Sector and cluster sizes: not 0, and powers of two.
		tax
		beq FindBPBTryNextShort
		dec A
		sta [D + F_WORK]
		txa
		and [D + F_WORK]
		bne FindBPBTryNextShort
		ldy BPB_uiSectorsPerCluster
		jsr BPBByte
		tax
		beq FindBPBTryNextShort
		dec A
		sta [D + F_WORK]
		txa
		and [D + F_WORK]
		bne FindBPBTryNextShort
		jsr VolumeParameters
		bcs FindBPBNextShort
		; The FAT's first byte is the media type.
		ldy CFatVolume_muiNoOfReservedSectors
		ldx F_SECTOR
		jsr GetMember32
		jsr ReadCache
		bcc FindBPBFatRead
		rts
FindBPBFatRead:
		lda [[D + F_CACHE]:24]
		sta [D + F_WORK]
		ldy BPB_uiMediaType
		jsr BPBByte
		eor [D + F_WORK]
		and 0xFF
		bne FindBPBTryNextShort
		lda FAT_SUCCESS
		clc
		rts
FindBPBTryNextShort:
FindBPBTryNext:
		inc [D + M_TRIED]
FindBPBNextShort:
		jmp FindBPBNext


; memcpy(&msBPB, A (low) X (high), sizeof(SFatBIOSParameterBlock))
CopyBPB:
		.width (A, X), 16
		pea 0
		pea SIZEOF_SFatBIOSParameterBlock
		phx
		pha
		pei [D + M_BPB + 2]
		pei [D + M_BPB]
		jsl _memcpy
		tsc
		clc
		adc 12
		tcs
		rts


; InitialiseVolumeLabel: the root directory's volume ID entry, if it has one, is the label.  A = the result.
InitialiseVolumeLabel:
		.width (A, X), 16
		; The query on the stack, under the return address (which is pulled and pushed again around it).
		ply
		tsc
		sec
		sbc SIZEOF_SFatQueryState
		tcs
		inc A
		sta [D + M_QUERY]
		stz [D + M_QUERY + 2]
		phy
		pei [D + M_QUERY + 2]
		pei [D + M_QUERY]
		jsl _SFatQueryState__Init
		ply
		ply
		pea 1
		pei [D + M_QUERY + 2]
		pei [D + M_QUERY]
		pea FAT_ATTR_VOLUME_ID
		pea 0
		pea 0
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl QueryFirstEntry
		sta [D + M_RESULT]
		tsc
		clc
		adc 16
		tcs
		lda [D + M_RESULT]
		cmp FAT_SUCCESS
		bne InitialiseLabelDone
		ldy SFatQueryState_psCurrentEntryRaw
		lda [[D + M_QUERY]:24 + Y]
		sta [D + F_POINTER]
		ldy SFatQueryState_psCurrentEntryRaw + 2
		lda [[D + M_QUERY]:24 + Y]
		sta [D + F_POINTER + 2]
		lda [[D + F_POINTER]:24]
		and 0xFF
		beq InitialiseLabelDone
		ldy RAW_uiAttributes
		lda [[D + F_POINTER]:24 + Y]
		and 0xFF
		cmp FAT_ATTR_VOLUME_ID
		bne InitialiseLabelDone
		jsr SetLabel
InitialiseLabelDone:
		jsr SectorCache
		pei [D + F_WORK2 + 2]
		pei [D + F_WORK2]
		pei [D + M_QUERY + 2]
		pei [D + M_QUERY]
		jsl _SFatQueryState__Kill
		tsc
		clc
		adc 8
		tcs
		ply
		tsc
		clc
		adc SIZEOF_SFatQueryState
		tcs
		phy
		lda [D + M_RESULT]
		rts


; InitialiseFat32FileSystemInfo (F_SECTOR is muiFileSystemInfoSector): the next free cluster and the free clusters
; from a valid FSInfo sector.  Carry set (A the error) if it couldn't be read.
Fat32FileSystemInfo:
		.width (A, X), 16
		ldy CFatVolume_muiHiddenSectors
		ldx F_SECTOR
		jsr AddMember32
		ldy CFatVolume_muiFileSystemInfoSector
		ldx F_SECTOR
		jsr SetMember32
		jsr ReadCache
		bcc Fat32InfoRead
		rts
Fat32InfoRead:
		lda [D + F_CACHE]
		sta [D + F_POINTER]
		lda [D + F_CACHE + 2]
		sta [D + F_POINTER + 2]
		ldy SFatFileSystemInfo_uiLeadSignature
		lda 0x5252
		ldx 0x4161
		jsr InfoMatches
		bne Fat32InfoInvalid
		ldy SFatFileSystemInfo_uiStructSignature
		lda 0x7272
		ldx 0x6141
		jsr InfoMatches
		bne Fat32InfoInvalid
		ldy SFatFileSystemInfo_uiTrailSignature
		lda 0x0000
		ldx 0xAA55
		jsr InfoMatches
		bne Fat32InfoInvalid
		ldy SFatFileSystemInfo_uiNextFreeCluster
		ldx F_WORK
		jsr InfoLong
		ldy CFatVolume_muiNextFreeCluster
		ldx F_WORK
		jsr SetMember32
		; Free clusters: the FSInfo's if it is less than the volume's clusters, else the clusters - 1.
		ldy SFatFileSystemInfo_uiNumFreeClusters
		ldx F_WORK
		jsr InfoLong
		ldy CFatVolume_muiNoOfClusters + 2
		lda [D + F_WORK + 2]
		cmp [[D + F_THIS]:24 + Y]
		bne Fat32InfoCompared
		ldy CFatVolume_muiNoOfClusters
		lda [D + F_WORK]
		cmp [[D + F_THIS]:24 + Y]
Fat32InfoCompared:
		bcs Fat32InfoClusters
		bra Fat32InfoFree
Fat32InfoInvalid:
		lda 0xFFFF
		sta [D + F_WORK]
		sta [D + F_WORK + 2]
		ldy CFatVolume_muiNextFreeCluster
		ldx F_WORK
		jsr SetMember32
Fat32InfoClusters:
		ldy CFatVolume_muiNoOfClusters
		jsr LoadMember32
		sec
		sbc 1
		sta [D + F_WORK]
		txa
		sbc 0
		sta [D + F_WORK + 2]
Fat32InfoFree:
		ldy CFatVolume_muiTotalFreeClusters
		ldx F_WORK
		jsr SetMember32
		lda FAT_SUCCESS
		clc
		rts


; Z set if the FSInfo sector's 32 bit field at offset Y is X (high) and A (low).
InfoMatches:
		.width (A, X), 16
		cmp [[D + F_POINTER]:24 + Y]
		bne InfoMatchesReturn
		txa
		iny
		iny
		cmp [[D + F_POINTER]:24 + Y]
InfoMatchesReturn:
		rts


; The FSInfo sector's 32 bit field at offset Y into D + X.
InfoLong:
		.width (A, X), 16
		lda [[D + F_POINTER]:24 + Y]
		sta [D + 0 + X]
		iny
		iny
		lda [[D + F_POINTER]:24 + Y]
		sta [D + 2 + X]
		rts


; Writes X (high) and A (low) into the FSInfo sector's 32 bit field at offset Y.
SetInfoLong:
		.width (A, X), 16
		sta [[D + F_POINTER]:24 + Y]
		txa
		iny
		iny
		sta [[D + F_POINTER]:24 + Y]
		rts


; EFatCode CFatVolume::Unmount(void)
		.global _CFatVolume__Unmount
_CFatVolume__Unmount:
		.width (A, X), 16
		jsr Enter
		jsr IsFat32
		bne UnmountCaches
		ldy CFatVolume_muiFileSystemInfoSector
		ldx F_SECTOR
		jsr GetMember32
		lda [D + F_SECTOR]
		and [D + F_SECTOR + 2]
		cmp 0xFFFF
		beq UnmountCaches
		jsr ReadCache
		bcc UnmountInfoRead
		jmp Leave
UnmountInfoRead:
		lda [D + F_CACHE]
		sta [D + F_POINTER]
		lda [D + F_CACHE + 2]
		sta [D + F_POINTER + 2]
		ldy CFatVolume_muiNextFreeCluster
		jsr LoadMember32
		ldy SFatFileSystemInfo_uiNextFreeCluster
		jsr SetInfoLong
		ldy CFatVolume_muiTotalFreeClusters
		jsr LoadMember32
		ldy SFatFileSystemInfo_uiNumFreeClusters
		jsr SetInfoLong
		lda 0x5252
		ldx 0x4161
		ldy SFatFileSystemInfo_uiLeadSignature
		jsr SetInfoLong
		lda 0x7272
		ldx 0x6141
		ldy SFatFileSystemInfo_uiStructSignature
		jsr SetInfoLong
		lda 0x0000
		ldx 0xAA55
		ldy SFatFileSystemInfo_uiTrailSignature
		jsr SetInfoLong
		jsr DirtyCache
		jsr FlushCaches
		cmp FAT_SUCCESS
		beq UnmountCaches
		jmp Leave
UnmountCaches:
		jsr PushLabel
		jsl _CChars__Kill
		ply
		ply
		jsr SectorCache
		pei [D + F_WORK2 + 2]
		pei [D + F_WORK2]
		jsl _CFatInfoSectorCache__Kill
		ply
		ply
		and 0xFF
		bne UnmountDone
		lda FAT_NO_CACHE_LOCK
		jmp Leave
UnmountDone:
		lda FAT_SUCCESS
		jmp Leave


; ------------------------------------------------------------------------------------------------------------------
; Directory entries
; ------------------------------------------------------------------------------------------------------------------

; Copies Y + 2 bytes (Y even) from the pointer at F_WORK to the pointer at F_POINTER.
CopyWords:
		.width (A, X), 16
		lda [[D + F_WORK]:24 + Y]
		sta [[D + F_POINTER]:24 + Y]
		dey
		dey
		bpl CopyWords
		rts


; Copies the raw entry at F_WORK into the directory entry at F_POINTER's sRaw (F_POINTER is moved to it).
CopyRaw:
		.width (A, X), 16
		lda [D + F_POINTER]
		clc
		adc SFatDirectoryEntry_sRaw
		sta [D + F_POINTER]
		bcc CopyRawPointer
		inc [D + F_POINTER + 2]
CopyRawPointer:
		ldy SIZEOF_SFatRawDirectoryEntry - 2
		bra CopyWords


; FillDirectoryEntryFromRawEntry(psEntry = F_POINTER, psRawEntry = F_WORK): its 8.3 name, attributes, size and raw
; entry.  F_POINTER and F_WORK are kept.
FillEntry:
		.width (A, X), 16
		pei [D + F_WORK + 2]
		pei [D + F_WORK]
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		jsl _ConvertFATShortInternalNameInto8Dot3Format__FPhPh
		pla
		sta [D + F_POINTER]
		pla
		sta [D + F_POINTER + 2]
		pla
		sta [D + F_WORK]
		pla
		sta [D + F_WORK + 2]
		ldy RAW_uiAttributes
		lda [[D + F_WORK]:24 + Y]
		sep m
		ldy SFatDirectoryEntry_uiAttributes
		sta [[D + F_POINTER]:24 + Y]
		rep m
		ldy RAW_uiSize
		lda [[D + F_WORK]:24 + Y]
		ldy SFatDirectoryEntry_uiSize
		sta [[D + F_POINTER]:24 + Y]
		ldy RAW_uiSize + 2
		lda [[D + F_WORK]:24 + Y]
		ldy SFatDirectoryEntry_uiSize + 2
		sta [[D + F_POINTER]:24 + Y]
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		jsr CopyRaw
		pla
		sta [D + F_POINTER]
		pla
		sta [D + F_POINTER + 2]
		rts


; void CFatVolume::FillDirectoryEntryFromRawEntry(SFatDirectoryEntry* psEntry, SFatRawDirectoryEntry* psRawEntry)
		.global _CFatVolume__FillDirectoryEntryFromRawEntry
_CFatVolume__FillDirectoryEntryFromRawEntry:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT]
		sta [D + F_POINTER]
		lda [D + F_ARGUMENT + 2]
		sta [D + F_POINTER + 2]
		lda [D + F_ARGUMENT + 4]
		sta [D + F_WORK]
		lda [D + F_ARGUMENT + 6]
		sta [D + F_WORK + 2]
		jsr FillEntry
		jmp Leave


; The rest of FindFirstFATEntry and FindNextFATEntry: the query's current entry into its sCurrentEntry (Q_QUERY is the
; SFatFileSystemQuery, Q_RESULT the SFatDirectoryEntry** to set).  A = FAT_SUCCESS.
QueryResult:
		.width (A, X), 16
		; The query's sQuery for the query helpers; its sCurrentEntry at F_POINTER.
		lda [D + Q_QUERY]
		sta [D + F_POINTER]
		lda [D + Q_QUERY + 2]
		sta [D + F_POINTER + 2]
		lda [D + Q_QUERY]
		clc
		adc SFatFileSystemQuery_sQuery
		sta [D + Q_QUERY]
		bcc QueryResultQuery
		inc [D + Q_QUERY + 2]
QueryResultQuery:
		jsr QueryEntry
		lda [D + Q_ENTRY]
		ora [D + Q_ENTRY + 2]
		bne QueryResultEntry
		; No more entries: the name is empty.
		lda 0
		sep m
		ldy SFatDirectoryEntry_szName
		sta [[D + F_POINTER]:24 + Y]
		rep m
		lda FAT_SUCCESS
		rts
QueryResultEntry:
		lda [D + Q_ENTRY]
		sta [D + F_WORK]
		lda [D + Q_ENTRY + 2]
		sta [D + F_WORK + 2]
		jsr FillEntry
		jsr QuerySector
		lda [D + F_SECTOR]
		ldy SFatDirectoryEntry_uiSectorAddress
		sta [[D + F_POINTER]:24 + Y]
		lda [D + F_SECTOR + 2]
		ldy SFatDirectoryEntry_uiSectorAddress + 2
		sta [[D + F_POINTER]:24 + Y]
		jsr EntryOffset
		ldy SFatDirectoryEntry_uiSectorOffset
		sta [[D + F_POINTER]:24 + Y]
		; CopyLongFilenameIntoString(sCurrentEntry.szName, sQuery.auiLongFilename)
		lda [D + Q_QUERY]
		clc
		adc SFatQueryState_auiLongFilename
		tax
		lda [D + Q_QUERY + 2]
		adc 0
		pha
		phx
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		jsl _CopyLongFilenameIntoString__FPcPt
		tsc
		clc
		adc 8
		tcs
		; SafeAssign(ppsDirectoryEntry, &sCurrentEntry)
		lda [D + Q_RESULT]
		ora [D + Q_RESULT + 2]
		beq QueryResultDone
		lda [D + F_POINTER]
		sta [[D + Q_RESULT]:24]
		lda [D + F_POINTER + 2]
		ldy 2
		sta [[D + Q_RESULT]:24 + Y]
QueryResultDone:
		lda FAT_SUCCESS
		rts


; The query's current entry's offset in its sector (psCurrentEntryRaw - psFirstEntryRaw), in A.
EntryOffset:
		.width (A, X), 16
		ldy SFatQueryState_psFirstEntryRaw
		lda [D + Q_ENTRY]
		sec
		sbc [[D + Q_QUERY]:24 + Y]
		rts


; EFatCode CFatVolume::FindFirstFATEntry(const char* szParentPath, uint8 uiAttributes, SFatDirectoryEntry** ppsDirectoryEntry,
;                                        SFatFileSystemQuery* psQuery)
FF_PATH			= F_ARGUMENT
FF_ATTRIBUTES	= F_ARGUMENT + 4
FF_RESULT		= F_ARGUMENT + 6
FF_QUERY		= F_ARGUMENT + 10
FF_PARENT		= F_LOCALS + 24		; The parent's SFatDirectoryEntry (below the frame).
		.global _CFatVolume__FindFirstFATEntry
_CFatVolume__FindFirstFATEntry:
		.width (A, X), 16
		jsr Enter
		tsc
		sec
		sbc SIZEOF_SFatDirectoryEntry
		tcs
		inc A
		sta [D + FF_PARENT]
		stz [D + FF_PARENT + 2]
		pei [D + FF_PATH + 2]
		pei [D + FF_PATH]
		jsl _StepPathOverSlash__FPc
		ply
		ply
		sta [D + F_WORK]
		stx [D + F_WORK + 2]
		; The query's sQuery.
		lda [D + FF_QUERY]
		clc
		adc SFatFileSystemQuery_sQuery
		sta [D + F_WORK2]
		lda [D + FF_QUERY + 2]
		adc 0
		sta [D + F_WORK2 + 2]
		pea 0
		pei [D + F_WORK2 + 2]
		pei [D + F_WORK2]
		pei [D + FF_ATTRIBUTES]
		lda [D + F_WORK]
		ora [D + F_WORK + 2]
		bne FindFirstParent
		; The root directory.
		pea 0
		pea 0
		bra FindFirstQuery
FindFirstParent:
		pei [D + FF_PARENT + 2]
		pei [D + FF_PARENT]
		pei [D + F_WORK + 2]
		pei [D + F_WORK]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl _CFatVolume__GetFileEntry
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		cmp FAT_SUCCESS
		beq FindFirstParentFound
		jmp Leave
FindFirstParentFound:
		lda [D + FF_PARENT]
		clc
		adc SFatDirectoryEntry_sRaw
		tax
		pei [D + FF_PARENT + 2]
		phx
FindFirstQuery:
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl QueryFirstEntry
		tay
		tsc
		clc
		adc 16
		tcs
		tya
		cmp FAT_SUCCESS
		beq FindFirstResult
		jmp Leave
FindFirstResult:
		lda [D + FF_QUERY]
		sta [D + Q_QUERY]
		lda [D + FF_QUERY + 2]
		sta [D + Q_QUERY + 2]
		lda [D + FF_RESULT]
		sta [D + Q_RESULT]
		lda [D + FF_RESULT + 2]
		sta [D + Q_RESULT + 2]
		jsr QueryResult
		jmp Leave


; EFatCode CFatVolume::FindNextFATEntry(SFatDirectoryEntry** ppsDirectoryEntry, SFatFileSystemQuery* psQuery)
		.global _CFatVolume__FindNextFATEntry
_CFatVolume__FindNextFATEntry:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT + 4]
		clc
		adc SFatFileSystemQuery_sQuery
		tax
		lda [D + F_ARGUMENT + 6]
		adc 0
		pea 0
		pea 0
		pha
		phx
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl QueryNextEntry
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		cmp FAT_SUCCESS
		beq FindNextResult
		jmp Leave
FindNextResult:
		lda [D + F_ARGUMENT + 4]
		sta [D + Q_QUERY]
		lda [D + F_ARGUMENT + 6]
		sta [D + Q_QUERY + 2]
		lda [D + F_ARGUMENT]
		sta [D + Q_RESULT]
		lda [D + F_ARGUMENT + 2]
		sta [D + Q_RESULT + 2]
		jsr QueryResult
		jmp Leave


; CreateFakeRootEntry(psEntry at F_POINTER): "ROOT", a directory at the root cluster.
FakeRootEntry:
		.width (A, X), 16
		pea RootName >> 16
		pea RootName & 0xFFFF
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		jsl _strcpy
		tsc
		clc
		adc 8
		tcs
		; GetFatShortNameForEntry(sRaw.szShortName, szName, true)
		pea 1
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		lda [D + F_POINTER]
		clc
		adc SFatDirectoryEntry_sRaw + RAW_szShortName
		tax
		lda [D + F_POINTER + 2]
		adc 0
		pha
		phx
		jsl _GetFatShortNameForEntry__FPcPcb
		tsc
		clc
		adc 10
		tcs
		lda FAT_ATTR_DIRECTORY
		sep m
		ldy SFatDirectoryEntry_uiAttributes
		sta [[D + F_POINTER]:24 + Y]
		ldy SFatDirectoryEntry_sRaw + RAW_uiAttributes
		sta [[D + F_POINTER]:24 + Y]
		rep m
		lda 0
		ldy SFatDirectoryEntry_uiSize
		sta [[D + F_POINTER]:24 + Y]
		ldy SFatDirectoryEntry_uiSize + 2
		sta [[D + F_POINTER]:24 + Y]
		ldy SFatDirectoryEntry_sRaw + RAW_uiSize
		sta [[D + F_POINTER]:24 + Y]
		ldy SFatDirectoryEntry_sRaw + RAW_uiSize + 2
		sta [[D + F_POINTER]:24 + Y]
		ldy SFatDirectoryEntry_uiSectorAddress
		sta [[D + F_POINTER]:24 + Y]
		ldy SFatDirectoryEntry_uiSectorAddress + 2
		sta [[D + F_POINTER]:24 + Y]
		ldy SFatDirectoryEntry_uiSectorOffset
		sta [[D + F_POINTER]:24 + Y]
		ldy CFatVolume_muiRootCluster
		lda [[D + F_THIS]:24 + Y]
		ldy SFatDirectoryEntry_sRaw + RAW_uiFirstClusterLowWord
		sta [[D + F_POINTER]:24 + Y]
		ldy CFatVolume_muiRootCluster + 2
		lda [[D + F_THIS]:24 + Y]
		ldy SFatDirectoryEntry_sRaw + RAW_uiFirstClusterHighWord
		sta [[D + F_POINTER]:24 + Y]
		lda FAT_SUCCESS
		rts
RootName:
		.byte "ROOT", 0


; EFatCode CFatVolume::GetFileEntry(const char* szPath, SFatDirectoryEntry* psEntry)
GE_PATH			= F_ARGUMENT
GE_ENTRY		= F_ARGUMENT + 4
GE_CURRENT		= F_LOCALS + 24		; The directory entry the next level is in (NULL: the root).
GE_LONG			= F_LOCALS + 28		; uint16 auiConstructedLongFilename[FAT_MAX_PATH + 1] (below the frame)
GE_LEVEL		= F_LOCALS + 32		; char szCurrentLevelPath[FAT_MAX_PATH + 1] (below the frame)
GE_SHORT		= F_LOCALS + 36		; char szConstructedShortFileName[13] (below the frame)
GE_MATCH		= F_LOCALS + 40		; bool bMatch
GE_LONG_NAME	= F_LOCALS + 42		; bool bLongFilename
GE_RESULT		= F_LOCALS + 44
GE_BELOW		= SIZEOF_SFatQueryState + (FAT_MAX_PATH + 1) * 2 + (FAT_MAX_PATH + 1) + 13 + 1
		.global _CFatVolume__GetFileEntry
_CFatVolume__GetFileEntry:
		.width (A, X), 16
		jsr Enter
		pei [D + GE_PATH + 2]
		pei [D + GE_PATH]
		jsl _StepPathOverSlash__FPc
		ply
		ply
		sta [D + GE_PATH]
		stx [D + GE_PATH + 2]
		lda [[D + GE_PATH]:24]
		and 0xFF
		bne GetEntryPath
		lda [D + GE_ENTRY]
		sta [D + F_POINTER]
		lda [D + GE_ENTRY + 2]
		sta [D + F_POINTER + 2]
		jsr FakeRootEntry
		jmp Leave
GetEntryPath:
		; The query and the names below the frame.
		tsc
		sec
		sbc GE_BELOW
		tcs
		inc A
		sta [D + Q_QUERY]
		clc
		adc SIZEOF_SFatQueryState
		sta [D + GE_LONG]
		clc
		adc (FAT_MAX_PATH + 1) * 2
		sta [D + GE_LEVEL]
		clc
		adc FAT_MAX_PATH + 1
		sta [D + GE_SHORT]
		stz [D + Q_QUERY + 2]
		stz [D + GE_LONG + 2]
		stz [D + GE_LEVEL + 2]
		stz [D + GE_SHORT + 2]
		stz [D + GE_CURRENT]
		stz [D + GE_CURRENT + 2]
		pea 0
		pea FAT_MAX_PATH + 1
		pea 0
		pei [D + GE_LEVEL + 2]
		pei [D + GE_LEVEL]
		jsl _memset
		tsc
		clc
		adc 10
		tcs
		pei [D + Q_QUERY + 2]
		pei [D + Q_QUERY]
		jsl _SFatQueryState__Init
		ply
		ply
GetEntryLevel:
		jsr NextPathItem
		bcc GetEntryLevelFound
		lda FAT_INVALID_FILENAME
		jmp GetEntryDone
GetEntryLevelFound:
		pea 1
		pei [D + Q_QUERY + 2]
		pei [D + Q_QUERY]
		pea 0
		pei [D + GE_CURRENT + 2]
		pei [D + GE_CURRENT]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl QueryFirstEntry
		tay
		tsc
		clc
		adc 16
		tcs
		tya
		cmp FAT_SUCCESS
		beq GetEntryFirst
		jmp GetEntryDone
GetEntryFirst:
		jsr QueryEntry
		ldy RAW_szShortName
		jsr EntryByte
		bne GetEntryMatch
		jmp GetEntryNotFound
GetEntryMatch:
		; MatchesFatFileName(&bMatch, &bLongFilename, szConstructedShortFileName, auiConstructedLongFilename,
		;                    szCurrentLevelPath, sQuery.auiLongFilename, psCurrentEntryRaw->szShortName)
		lda [D + Q_ENTRY]
		clc
		adc RAW_szShortName
		tax
		lda [D + Q_ENTRY + 2]
		adc 0
		pha
		phx
		lda [D + Q_QUERY]
		clc
		adc SFatQueryState_auiLongFilename
		tax
		lda [D + Q_QUERY + 2]
		adc 0
		pha
		phx
		pei [D + GE_LEVEL + 2]
		pei [D + GE_LEVEL]
		pei [D + GE_LONG + 2]
		pei [D + GE_LONG]
		pei [D + GE_SHORT + 2]
		pei [D + GE_SHORT]
		pea 0
		tdc
		clc
		adc GE_LONG_NAME
		pha
		pea 0
		tdc
		clc
		adc GE_MATCH
		pha
		jsl _MatchesFatFileName__FPbPbPcPtPcPtPh
		tay
		tsc
		clc
		adc 28
		tcs
		tya
		cmp FAT_SUCCESS
		beq GetEntryMatching
		jmp GetEntryDone
GetEntryMatching:
		lda [D + GE_MATCH]
		and 0xFF
		beq GetEntryNext
		jmp GetEntryMatched
GetEntryNext:
		pea 0
		pea 1
		pei [D + Q_QUERY + 2]
		pei [D + Q_QUERY]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl QueryNextEntry
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		cmp FAT_SUCCESS
		beq GetEntryNextFound
		jmp GetEntryDone
GetEntryNextFound:
		jsr QueryCache
		jsr ValidateCache
		bcc GetEntryNextValid
		jmp Leave
GetEntryNextValid:
		jsr QueryEntry
		ldy RAW_szShortName
		jsr EntryByte
		bne GetEntryCompare
		jmp GetEntryNotFound
GetEntryCompare:
		lda [D + GE_LONG_NAME]
		and 0xFF
		beq GetEntryCompareShort
		lda [D + Q_QUERY]
		clc
		adc SFatQueryState_auiLongFilename
		tax
		lda [D + Q_QUERY + 2]
		adc 0
		pha
		phx
		pei [D + GE_LONG + 2]
		pei [D + GE_LONG]
		jsl _CompareFatLongName__FPtPt
		ply
		ply
		ply
		ply
		cmp 0
		bne GetEntryCompared
GetEntryCompareShort:
		lda [D + Q_ENTRY]
		clc
		adc RAW_szShortName
		tax
		lda [D + Q_ENTRY + 2]
		adc 0
		pha
		phx
		pei [D + GE_SHORT + 2]
		pei [D + GE_SHORT]
		jsl _CompareFatShortName__FPcPc
		ply
		ply
		ply
		ply
		and 0xFF
GetEntryCompared:
		sta [D + GE_MATCH]
		jmp GetEntryMatching
GetEntryMatched:
		lda [D + Q_ENTRY]
		sta [D + GE_CURRENT]
		lda [D + Q_ENTRY + 2]
		sta [D + GE_CURRENT + 2]
		lda [[D + GE_PATH]:24]
		and 0xFF
		beq GetEntryFound
		jmp GetEntryLevel

GetEntryFound:
		jsr QueryCache
		jsr ValidateCache
		bcc GetEntryValid
		jmp Leave
GetEntryValid:
		jsr QueryEntry
		lda [D + GE_ENTRY]
		sta [D + F_POINTER]
		lda [D + GE_ENTRY + 2]
		sta [D + F_POINTER + 2]
		lda [D + Q_ENTRY]
		sta [D + F_WORK]
		lda [D + Q_ENTRY + 2]
		sta [D + F_WORK + 2]
		jsr FillEntry
		jsr QuerySector
		lda [D + F_SECTOR]
		ldy SFatDirectoryEntry_uiSectorAddress
		sta [[D + F_POINTER]:24 + Y]
		lda [D + F_SECTOR + 2]
		ldy SFatDirectoryEntry_uiSectorAddress + 2
		sta [[D + F_POINTER]:24 + Y]
		jsr EntryOffset
		ldy SFatDirectoryEntry_uiSectorOffset
		sta [[D + F_POINTER]:24 + Y]
		lda FAT_SUCCESS
		bra GetEntryDone
GetEntryNotFound:
		lda 0
		sep m
		sta [[D + GE_ENTRY]:24]
		rep m
		lda FAT_SUCCESS
GetEntryDone:
		sta [D + GE_RESULT]
		jsr KillQuery
		lda [D + GE_RESULT]
		jmp Leave


; Q_QUERY->Kill(&mcSectorCache)
KillQuery:
		.width (A, X), 16
		jsr SectorCache
		pei [D + F_WORK2 + 2]
		pei [D + F_WORK2]
		pei [D + Q_QUERY + 2]
		pei [D + Q_QUERY]
		jsl _SFatQueryState__Kill
		tsc
		clc
		adc 8
		tcs
		rts


; FindNextPathItem(GE_PATH, GE_LEVEL): the path's next level into the level name and GE_PATH past it.  Carry set if
; the level is longer than FAT_MAX_PATH.
NextPathItem:
		.width (A, X), 16
		pei [D + GE_PATH + 2]
		pei [D + GE_PATH]
		jsl _StepPathOverSlash__FPc
		ply
		ply
		sta [D + GE_PATH]
		stx [D + GE_PATH + 2]
		ldy 0
		sep m
NextPathItemCopy:
		lda [[D + GE_PATH]:24 + Y]
		beq NextPathItemEnd
		cmp '\\'
		beq NextPathItemEnd
		cpy FAT_MAX_PATH + 1
		bcs NextPathItemLong
		sta [[D + GE_LEVEL]:24 + Y]
		iny
		bra NextPathItemCopy
NextPathItemLong:
		rep m
		sec
		rts
NextPathItemEnd:
		.width A, 8
		lda 0
		sta [[D + GE_LEVEL]:24 + Y]
		rep m
		tya
		clc
		adc [D + GE_PATH]
		sta [D + GE_PATH]
		bcc NextPathItemReturn
		inc [D + GE_PATH + 2]
NextPathItemReturn:
		clc
		rts


; EFatCode CFatVolume::SeekByClusterCount(uint32 uiClusterIndex, uint16 uiClusterCount, uint32* puiNewClusterIndex)
		.global _CFatVolume__SeekByClusterCount
_CFatVolume__SeekByClusterCount:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT]
		sta [D + F_CLUSTER]
		lda [D + F_ARGUMENT + 2]
		sta [D + F_CLUSTER + 2]
		lda [D + F_ARGUMENT + 4]
		beq SeekDone
SeekNext:
		lda [D + F_CLUSTER + 2]
		bne SeekCluster
		lda [D + F_CLUSTER]
		cmp 2
		bcs SeekCluster
		lda FAT_INVALID_CLUSTER
		jmp Leave
SeekCluster:
		jsr NextCluster
		bcc SeekRead
		jmp Leave
SeekRead:
		lda [D + F_CLUSTER]
		ldx [D + F_CLUSTER + 2]
		jsr IsEndOfChain
		bcc SeekCount
		lda FAT_UNKNOWN_ERROR
		jmp Leave
SeekCount:
		lda [D + F_ARGUMENT + 4]
		dec A
		sta [D + F_ARGUMENT + 4]
		bne SeekNext
SeekDone:
		lda [D + F_CLUSTER]
		ldx [D + F_CLUSTER + 2]
		ldy F_ARGUMENT + 6
		jsr StoreThrough
		lda FAT_SUCCESS
		jmp Leave


; EFatCode CFatVolume::FreeChain(uint32 uiClusterIndex): frees each cluster of the chain (its FAT entry 0; a FAT32
; entry's top 4 bits are kept) and counts it free.
		.global _CFatVolume__FreeChain
_CFatVolume__FreeChain:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT]
		sta [D + F_CLUSTER]
		lda [D + F_ARGUMENT + 2]
		sta [D + F_CLUSTER + 2]
		jsr FreeChain
		jmp Leave


; FreeChain(F_CLUSTER): A = the result.
FreeChain:
		.width (A, X), 16
		lda [D + F_CLUSTER + 2]
		bne FreeChainCluster
		lda [D + F_CLUSTER]
		cmp 2
		bcs FreeChainCluster
		lda FAT_INVALID_CLUSTER
		rts
FreeChainCluster:
		jsr FatEntryPlace
		jsr ReadCache
		bcc FreeChainRead
		rts
FreeChainRead:
		lda [D + F_CACHE]
		clc
		adc [D + F_OFFSET]
		sta [D + F_POINTER]
		lda [D + F_CACHE + 2]
		adc 0
		sta [D + F_POINTER + 2]
		lda [[D + F_POINTER]:24]
		sta [D + F_CLUSTER]
		lda 0
		sta [[D + F_POINTER]:24]
		stz [D + F_CLUSTER + 2]
		jsr IsFat32
		bne FreeChainFreed
		ldy 2
		lda [[D + F_POINTER]:24 + Y]
		pha
		and FAT32_CLUSTER_MASK >> 16
		sta [D + F_CLUSTER + 2]
		pla
		and (FAT32_CLUSTER_MASK >> 16) ^ 0xFFFF
		sta [[D + F_POINTER]:24 + Y]
FreeChainFreed:
		jsr DirtyCache
		; SetTotalFreeClusters(GetTotalFreeClusters() + 1)
		ldy CFatVolume_muiTotalFreeClusters
		lda [[D + F_THIS]:24 + Y]
		inc A
		sta [[D + F_THIS]:24 + Y]
		bne FreeChainCounted
		iny
		iny
		lda [[D + F_THIS]:24 + Y]
		inc A
		sta [[D + F_THIS]:24 + Y]
FreeChainCounted:
		lda [D + F_CLUSTER]
		ldx [D + F_CLUSTER + 2]
		jsr IsEndOfChain
		bcc FreeChain
		lda FAT_SUCCESS
		rts


; EFatCode CFatVolume::FreeDirectoryEntry(SFatDirectoryEntry* psEntry): frees the entry's chain and writes the entry
; back without it.
		.global _CFatVolume__FreeDirectoryEntry
_CFatVolume__FreeDirectoryEntry:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT]
		clc
		adc SFatDirectoryEntry_sRaw
		sta [D + F_WORK]
		lda [D + F_ARGUMENT + 2]
		adc 0
		sta [D + F_WORK + 2]
		ldx F_WORK
		jsr EntryFirstCluster
		lda [D + F_CLUSTER]
		ora [D + F_CLUSTER + 2]
		bne FreeEntryCluster
		lda FAT_SUCCESS
		jmp Leave
FreeEntryCluster:
		lda 0
		ldy RAW_uiFirstClusterLowWord
		sta [[D + F_WORK]:24 + Y]
		ldy RAW_uiFirstClusterHighWord
		sta [[D + F_WORK]:24 + Y]
		ldy RAW_uiSize
		sta [[D + F_WORK]:24 + Y]
		ldy RAW_uiSize + 2
		sta [[D + F_WORK]:24 + Y]
		lda [D + F_ARGUMENT]
		sta [D + F_POINTER]
		lda [D + F_ARGUMENT + 2]
		sta [D + F_POINTER + 2]
		pei [D + F_CLUSTER + 2]
		pei [D + F_CLUSTER]
		jsr FillEntry
		jsr WriteEntryBack
		pla
		sta [D + F_CLUSTER]
		pla
		sta [D + F_CLUSTER + 2]
		bcc FreeEntryWritten
		jmp Leave
FreeEntryWritten:
		jsr FreeChain
		jmp Leave


; Writes the directory entry at F_POINTER's sRaw back to its place in its directory (memcpy into its sector and
; dirty it).  Carry set (A the error) if the sector couldn't be read.
WriteEntryBack:
		.width (A, X), 16
		ldy SFatDirectoryEntry_uiSectorAddress
		lda [[D + F_POINTER]:24 + Y]
		sta [D + F_SECTOR]
		ldy SFatDirectoryEntry_uiSectorAddress + 2
		lda [[D + F_POINTER]:24 + Y]
		sta [D + F_SECTOR + 2]
		jsr ReadCache
		bcc WriteEntryBackRead
		rts
WriteEntryBackRead:
		lda [D + F_POINTER]
		clc
		adc SFatDirectoryEntry_sRaw
		sta [D + F_WORK]
		lda [D + F_POINTER + 2]
		adc 0
		sta [D + F_WORK + 2]
		ldy SFatDirectoryEntry_uiSectorOffset
		lda [[D + F_POINTER]:24 + Y]
		clc
		adc [D + F_CACHE]
		sta [D + F_POINTER]
		lda [D + F_CACHE + 2]
		adc 0
		sta [D + F_POINTER + 2]
		ldy SIZEOF_SFatRawDirectoryEntry - 2
		jsr CopyWords
		jsr DirtyCache
		lda FAT_SUCCESS
		clc
		rts


; EFatCode CFatVolume::UpdateDirectoryEntry(SFatDirectoryEntry* psEntry, uint32 uiFirstCluster, uint32 uiNewClusterInVolume,
;                                           uint32 uiClustersInFile): only an entry without clusters yet (first cluster 0)
; gets the new cluster (and the archive attribute) and is written back.
		.global _CFatVolume__UpdateDirectoryEntry
_CFatVolume__UpdateDirectoryEntry:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT + 4]
		ora [D + F_ARGUMENT + 6]
		bne UpdateEntryDone
		lda [D + F_ARGUMENT]
		sta [D + F_POINTER]
		lda [D + F_ARGUMENT + 2]
		sta [D + F_POINTER + 2]
		lda [D + F_ARGUMENT + 8]
		ldy SFatDirectoryEntry_sRaw + RAW_uiFirstClusterLowWord
		sta [[D + F_POINTER]:24 + Y]
		lda [D + F_ARGUMENT + 10]
		ldy SFatDirectoryEntry_sRaw + RAW_uiFirstClusterHighWord
		sta [[D + F_POINTER]:24 + Y]
		lda FAT_ATTR_ARCHIVE
		sep m
		ldy SFatDirectoryEntry_sRaw + RAW_uiAttributes
		sta [[D + F_POINTER]:24 + Y]
		rep m
		jsr WriteEntryBack
		bcc UpdateEntryDone
		jmp Leave
UpdateEntryDone:
		lda FAT_SUCCESS
		jmp Leave


; ------------------------------------------------------------------------------------------------------------------
; Allocating clusters
; ------------------------------------------------------------------------------------------------------------------

; F_WORK % F_WORK2 into F_WORK (32 bits; F_WORK2 not 0), by shifting and subtracting.  Uses F_POINTER.
Modulo32:
		.width (A, X), 16
		stz [D + F_POINTER]
		stz [D + F_POINTER + 2]
		ldx 32
Modulo32Bit:
		asl [D + F_WORK]
		rol [D + F_WORK + 2]
		rol [D + F_POINTER]
		rol [D + F_POINTER + 2]
		lda [D + F_POINTER]
		sec
		sbc [D + F_WORK2]
		tay
		lda [D + F_POINTER + 2]
		sbc [D + F_WORK2 + 2]
		bcc Modulo32Next
		sta [D + F_POINTER + 2]
		sty [D + F_POINTER]
Modulo32Next:
		dex
		bne Modulo32Bit
		lda [D + F_POINTER]
		sta [D + F_WORK]
		lda [D + F_POINTER + 2]
		sta [D + F_WORK + 2]
		rts


; Carry set if the page size at D + X is more than muiNoOfSectorsPerCluster (pages span clusters).
PagesSpanClusters:
		.width (A, X), 16
		lda [D + 2 + X]
		bne PagesSpanYes
		ldy CFatVolume_muiNoOfSectorsPerCluster
		lda [D + 0 + X]
		cmp [[D + F_THIS]:24 + Y]
		beq PagesSpanNo
		rts
PagesSpanNo:
		clc
		rts
PagesSpanYes:
		sec
		rts


; The clusters a page is (the page size at D + X >> muiSectorsPerClusterShift) into F_WORK2.
ClustersPerPage:
		.width (A, X), 16
		lda [D + 0 + X]
		sta [D + F_WORK2]
		lda [D + 2 + X]
		sta [D + F_WORK2 + 2]
		ldy CFatVolume_muiSectorsPerClusterShift
		lda [[D + F_THIS]:24 + Y]
		and 0xFF
		ldx F_WORK2
		jmp ShiftRight32


; EFatCode CFatVolume::AllocateClusters(uint32 uiClustersNeeded, uint32 uiCluster, uint32* puiNewClusterInVolume, uint32 uiPreviousCluster)
AC_NEEDED		= F_ARGUMENT
AC_NEW			= F_ARGUMENT + 8
AC_PREVIOUS		= F_ARGUMENT + 12
AC_PAGE			= F_LOCALS
AC_RESULT		= F_LOCALS + 4
		.global _CFatVolume__AllocateClusters
_CFatVolume__AllocateClusters:
		.width (A, X), 16
		jsr Enter
		jsr PageSize
		sta [D + AC_PAGE]
		stx [D + AC_PAGE + 2]
		ldx AC_PAGE
		jsr PagesSpanClusters
		bcs AllocatePages
		lda 1
		sta [D + AC_PAGE]
		stz [D + AC_PAGE + 2]
		bra AllocateCall
AllocatePages:
		; Whole pages: the clusters needed rounded up to the clusters in a page.
		ldx AC_PAGE
		jsr ClustersPerPage
		lda [D + AC_NEEDED]
		sta [D + F_WORK]
		lda [D + AC_NEEDED + 2]
		sta [D + F_WORK + 2]
		jsr Modulo32
		lda [D + F_WORK]
		ora [D + F_WORK + 2]
		beq AllocateCall
		lda [D + F_WORK2]
		sec
		sbc [D + F_WORK]
		tax
		lda [D + F_WORK2 + 2]
		sbc [D + F_WORK + 2]
		tay
		txa
		clc
		adc [D + AC_NEEDED]
		sta [D + AC_NEEDED]
		tya
		adc [D + AC_NEEDED + 2]
		sta [D + AC_NEEDED + 2]
AllocateCall:
		; AllocateClusters(NULL, uiClustersNeeded, false, uiPageSize, &eResult, uiPreviousCluster)
		pei [D + AC_PREVIOUS + 2]
		pei [D + AC_PREVIOUS]
		pea 0
		tdc
		clc
		adc AC_RESULT
		pha
		pei [D + AC_PAGE + 2]
		pei [D + AC_PAGE]
		pea 0
		pei [D + AC_NEEDED + 2]
		pei [D + AC_NEEDED]
		pea 0
		pea 0
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl AllocateClusters
		tay
		tsc
		clc
		adc 26
		tcs
		tya
		sta [D + F_CLUSTER]
		stx [D + F_CLUSTER + 2]
		ldy AC_NEW
		jsr StoreThrough
		lda [D + AC_RESULT]
		cmp FAT_SUCCESS
		beq AllocateErase
		jmp Leave
AllocateErase:
		; EraseClusterChainContents(*puiNewClusterInVolume)
AllocateEraseNext:
		lda [D + F_CLUSTER]
		ldx [D + F_CLUSTER + 2]
		jsr IsEndOfChain
		bcs AllocateErased
		jsr EraseCluster
		bcc AllocateEraseCluster
		jmp Leave
AllocateEraseCluster:
		jsr NextCluster
		bcc AllocateEraseNext
		jmp Leave
AllocateErased:
		lda FAT_SUCCESS
		jmp Leave


; EraseClusterContents(F_CLUSTER): erases its sectors.  Carry set (A = FAT_CANNOT_WRITE_MEDIA) if it couldn't.
EraseCluster:
		.width (A, X), 16
		jsr FirstSector
		ldy CFatVolume_muiNoOfSectorsPerCluster
		lda [[D + F_THIS]:24 + Y]
		dec A
		clc
		adc [D + F_SECTOR]
		sta [D + F_OFFSET]
		lda [D + F_SECTOR + 2]
		adc 0
		sta [D + F_OFFSET + 2]
		jsr EraseSectors
		cmp 0
		beq EraseClusterFailed
		lda FAT_SUCCESS
		clc
		rts
EraseClusterFailed:
		lda FAT_CANNOT_WRITE_MEDIA
		sec
		rts


; uint32 AllocateClusters(SFatRawDirectoryEntry* psParentDirectory, uint32 uiClusterCount, bool bEraseCluster,
;                         uint32 uiPageSize, EFatCode* peResult, uint32 uiPreviousCluster)
; Allocates the clusters as a chain (linked to uiPreviousCluster's unless that is an end of chain) from the next free
; cluster on (whole pages: uiPageSize sectors).  Returns the first.
AL_PARENT		= F_ARGUMENT
AL_COUNT		= F_ARGUMENT + 4
AL_ERASE		= F_ARGUMENT + 8
AL_PAGE			= F_ARGUMENT + 10
AL_RESULT		= F_ARGUMENT + 14
AL_PREVIOUS		= F_ARGUMENT + 18
AL_FIRST_EMPTY	= F_LOCALS			; uiFirstEmptyCluster
AL_LAST			= F_LOCALS + 4		; uiLastClusterIndex
AL_LAST_SECTOR	= F_LOCALS + 8
AL_LAST_OFFSET	= F_LOCALS + 12
AL_START		= F_LOCALS + 16
AL_CURRENT		= F_LOCALS + 20		; uiCurrentSector: the FAT sector being looked through.
AL_STEP			= F_LOCALS + 24
AL_WRAPPED		= F_LOCALS + 26
AL_CLUSTER		= F_LOCALS + 28		; The cluster looked at (F_CLUSTER is used by the FAT helpers).
AL_SECTOR		= F_LOCALS + 32		; Its FAT entry's sector,
AL_OFFSET		= F_LOCALS + 36		; and offset.
AllocateClusters:
		.width (A, X), 16
		jsr Enter
		lda [D + AL_COUNT]
		ora [D + AL_COUNT + 2]
		beq AllocateUnknown
		lda [D + AL_COUNT + 2]
		bne AllocateMany
		lda [D + AL_COUNT]
		cmp 2
		bcc AllocateCountOk
AllocateMany:
		lda [D + AL_PARENT]
		ora [D + AL_PARENT + 2]
		beq AllocateCountOk
AllocateUnknown:
		lda FAT_UNKNOWN_ERROR
		jmp AllocateFailed
AllocateCountOk:
		stz [D + AL_FIRST_EMPTY]
		stz [D + AL_FIRST_EMPTY + 2]
		lda 2
		sta [D + AL_CLUSTER]
		stz [D + AL_CLUSTER + 2]
		; HasNextFreeCluser: muiNextFreeCluster isn't 0xFFFFFFFF and is at most muiNoOfClusters + 1.
		ldy CFatVolume_muiNextFreeCluster
		ldx F_WORK
		jsr GetMember32
		lda [D + F_WORK]
		and [D + F_WORK + 2]
		cmp 0xFFFF
		beq AllocateStep
		ldy CFatVolume_muiNoOfClusters
		ldx F_WORK2
		jsr GetMember32
		inc [D + F_WORK2]
		bne AllocateCompareNext
		inc [D + F_WORK2 + 2]
AllocateCompareNext:
		lda [D + F_WORK2 + 2]
		cmp [D + F_WORK + 2]
		bne AllocateComparedNext
		lda [D + F_WORK2]
		cmp [D + F_WORK]
AllocateComparedNext:
		bcc AllocateStep
		lda [D + F_WORK]
		sta [D + AL_CLUSTER]
		lda [D + F_WORK + 2]
		sta [D + AL_CLUSTER + 2]
AllocateStep:
		; CalculateClusterStepSize: the clusters in a page when pages span clusters, else 1.
		lda 1
		sta [D + AL_STEP]
		ldx AL_PAGE
		jsr PagesSpanClusters
		bcc AllocatePageCluster
		ldx AL_PAGE
		jsr ClustersPerPage
		lda [D + F_WORK2]
		sta [D + AL_STEP]
		; FindNextPageCluster: on to the first cluster on a page (at most a step on).
		stz [D + AL_FIRST_EMPTY + 2]		; (the step count, before it is the first empty cluster)
AllocatePageNext:
		lda [D + AL_CLUSTER]
		sta [D + F_CLUSTER]
		lda [D + AL_CLUSTER + 2]
		sta [D + F_CLUSTER + 2]
		jsr FirstSector
		lda [D + F_SECTOR]
		sta [D + F_WORK]
		lda [D + F_SECTOR + 2]
		sta [D + F_WORK + 2]
		lda [D + AL_PAGE]
		sta [D + F_WORK2]
		lda [D + AL_PAGE + 2]
		sta [D + F_WORK2 + 2]
		jsr Modulo32
		lda [D + F_WORK]
		ora [D + F_WORK + 2]
		beq AllocatePaged
		lda [D + AL_FIRST_EMPTY + 2]
		cmp [D + AL_STEP]
		bcs AllocatePaged
		inc [D + AL_FIRST_EMPTY + 2]
		inc [D + AL_CLUSTER]
		bne AllocatePageNext
		inc [D + AL_CLUSTER + 2]
		bra AllocatePageNext
AllocatePaged:
		stz [D + AL_FIRST_EMPTY + 2]
AllocatePageCluster:
		; The previous cluster's FAT entry (to link from), unless it is an end of chain.
		stz [D + AL_LAST_SECTOR]
		stz [D + AL_LAST_SECTOR + 2]
		stz [D + AL_LAST_OFFSET]
		stz [D + AL_LAST_OFFSET + 2]
		lda [D + AL_PREVIOUS]
		ldx [D + AL_PREVIOUS + 2]
		jsr IsEndOfChain
		bcs AllocateNoPrevious
		sta [D + F_CLUSTER]
		stx [D + F_CLUSTER + 2]
		jsr FatEntryPlace
		lda [D + F_SECTOR]
		sta [D + AL_LAST_SECTOR]
		lda [D + F_SECTOR + 2]
		sta [D + AL_LAST_SECTOR + 2]
		lda [D + F_OFFSET]
		sta [D + AL_LAST_OFFSET]
AllocateNoPrevious:
		jsr AllocatePlace
		lda [D + AL_CLUSTER]
		sta [D + AL_START]
		lda [D + AL_CLUSTER + 2]
		sta [D + AL_START + 2]
		lda [D + AL_PREVIOUS]
		sta [D + AL_LAST]
		lda [D + AL_PREVIOUS + 2]
		sta [D + AL_LAST + 2]
		stz [D + AL_WRAPPED]
AllocateSector:
		lda [D + AL_SECTOR]
		sta [D + AL_CURRENT]
		lda [D + AL_SECTOR + 2]
		sta [D + AL_CURRENT + 2]
AllocateCluster:
		; Past the last cluster: wrap round to cluster 2 once (WrapAllocate), then the volume is full.
		ldy CFatVolume_muiNoOfClusters
		ldx F_WORK2
		jsr GetMember32
		inc [D + F_WORK2]
		bne AllocateCompareLast
		inc [D + F_WORK2 + 2]
AllocateCompareLast:
		lda [D + F_WORK2 + 2]
		cmp [D + AL_CLUSTER + 2]
		bne AllocateComparedLast
		lda [D + F_WORK2]
		cmp [D + AL_CLUSTER]
AllocateComparedLast:
		bcs AllocateRead
		jsr WrapAllocate
		bcc AllocateWrapped
		jmp AllocateFailed
AllocateWrapped:
		bra AllocateSector
AllocateRead:
		lda [D + AL_SECTOR]
		sta [D + F_SECTOR]
		lda [D + AL_SECTOR + 2]
		sta [D + F_SECTOR + 2]
		lda [D + AL_OFFSET]
		sta [D + F_OFFSET]
		stz [D + F_OFFSET + 2]
		jsr ReadFatEntryHere
		bcc AllocateEntryRead
		jmp AllocateFailed
AllocateEntryRead:
		; IsFreeFat
		lda [D + F_CLUSTER]
		ora [D + F_CLUSTER + 2]
		beq AllocateFree
		jmp AllocateNext
AllocateFree:
		lda [D + AL_FIRST_EMPTY]
		ora [D + AL_FIRST_EMPTY + 2]
		bne AllocateWrite
		lda [D + AL_CLUSTER]
		sta [D + AL_FIRST_EMPTY]
		lda [D + AL_CLUSTER + 2]
		sta [D + AL_FIRST_EMPTY + 2]
AllocateWrite:
		jsr WriteAllocatedFatEntry
		bcc AllocateWritten
		jmp AllocateFailed
AllocateWritten:
		lda [D + AL_COUNT]
		sec
		sbc 1
		sta [D + AL_COUNT]
		lda [D + AL_COUNT + 2]
		sbc 0
		sta [D + AL_COUNT + 2]
		ora [D + AL_COUNT]
		bne AllocateMore
		; The last: initialise it (a directory's, or zeroed).
		lda [D + AL_CLUSTER]
		sta [D + F_CLUSTER]
		lda [D + AL_CLUSTER + 2]
		sta [D + F_CLUSTER + 2]
		lda [D + AL_PARENT]
		ora [D + AL_PARENT + 2]
		beq AllocateDataCluster
		jsr InitialiseDirectoryCluster
		bcc AllocateDone
		jmp AllocateFailed
AllocateDataCluster:
		lda [D + AL_ERASE]
		and 0xFF
		beq AllocateDone
		jsr EraseCluster
		bcc AllocateDone
		jmp AllocateFailed
AllocateDone:
		lda FAT_SUCCESS
		ldy 0
		sta [[D + AL_RESULT]:24 + Y]
		lda [D + AL_FIRST_EMPTY]
		ldx [D + AL_FIRST_EMPTY + 2]
		jmp Leave
AllocateMore:
		ldx 2
AllocateLast:
		lda [D + AL_CLUSTER + X]
		sta [D + AL_LAST + X]
		lda [D + AL_SECTOR + X]
		sta [D + AL_LAST_SECTOR + X]
		lda [D + AL_OFFSET + X]
		sta [D + AL_LAST_OFFSET + X]
		dex
		dex
		bpl AllocateLast
AllocateNext:
		; The next cluster: a step on until the first is found, then one on.
		lda 1
		ldx [D + AL_FIRST_EMPTY]
		bne AllocateAdvance
		ldx [D + AL_FIRST_EMPTY + 2]
		bne AllocateAdvance
		lda [D + AL_STEP]
AllocateAdvance:
		clc
		adc [D + AL_CLUSTER]
		sta [D + AL_CLUSTER]
		bcc AllocateAdvanced
		inc [D + AL_CLUSTER + 2]
AllocateAdvanced:
		jsr AllocatePlace
		lda [D + AL_SECTOR]
		cmp [D + AL_CURRENT]
		bne AllocateNextSector
		lda [D + AL_SECTOR + 2]
		cmp [D + AL_CURRENT + 2]
		bne AllocateNextSector
		jmp AllocateCluster
AllocateNextSector:
		jmp AllocateSector
AllocateFailed:
		ldy 0
		sta [[D + AL_RESULT]:24 + Y]
		lda 0
		ldx 0
		jmp Leave


; AL_SECTOR and AL_OFFSET = the FAT entry's place of AL_CLUSTER.
AllocatePlace:
		.width (A, X), 16
		lda [D + AL_CLUSTER]
		sta [D + F_CLUSTER]
		lda [D + AL_CLUSTER + 2]
		sta [D + F_CLUSTER + 2]
		jsr FatEntryPlace
		lda [D + F_SECTOR]
		sta [D + AL_SECTOR]
		lda [D + F_SECTOR + 2]
		sta [D + AL_SECTOR + 2]
		lda [D + F_OFFSET]
		sta [D + AL_OFFSET]
		stz [D + AL_OFFSET + 2]
		rts


; WrapAllocate: past the last cluster.  The first time (when it didn't start at 2), back to cluster 2; the second
; time the volume is full: the clusters allocated so far are freed (A = FAT_INSUFFICIENT_DISK_SPACE, carry set).
WrapAllocate:
		.width (A, X), 16
		lda [D + AL_WRAPPED]
		bne WrapAllocateWrapped
		lda [D + AL_START + 2]
		bne WrapAllocateToStart
		lda [D + AL_START]
		cmp 3
		bcc WrapAllocateUnknown
WrapAllocateToStart:
		lda 2
		sta [D + AL_CLUSTER]
		stz [D + AL_CLUSTER + 2]
		sta [D + AL_WRAPPED]
		jsr AllocatePlace
		clc
		rts
WrapAllocateWrapped:
		lda [D + AL_CLUSTER + 2]
		cmp [D + AL_START + 2]
		bne WrapAllocateCompared
		lda [D + AL_CLUSTER]
		cmp [D + AL_START]
WrapAllocateCompared:
		bcc WrapAllocateUnknown
		lda [D + AL_LAST]
		ldx [D + AL_LAST + 2]
		jsr IsEndOfChain
		bcs WrapAllocateFull
		sta [D + F_CLUSTER]
		stx [D + F_CLUSTER + 2]
		jsr FreeChain
WrapAllocateFull:
		lda FAT_INSUFFICIENT_DISK_SPACE
		sec
		rts
WrapAllocateUnknown:
		lda FAT_UNKNOWN_ERROR
		sec
		rts


; WriteAllocatedFatEntry(AL_CLUSTER, AL_OFFSET, AL_SECTOR, AL_LAST, AL_LAST_SECTOR, AL_LAST_OFFSET): the next free
; cluster is the one after, one cluster fewer is free; its FAT entry is an end of chain, and the previous cluster's
; (unless it was an end of chain itself) links to it.  Carry set (A the error) if a FAT sector couldn't be read.
WriteAllocatedFatEntry:
		.width (A, X), 16
		lda [D + AL_CLUSTER]
		clc
		adc 1
		sta [D + F_WORK]
		lda [D + AL_CLUSTER + 2]
		adc 0
		sta [D + F_WORK + 2]
		ldy CFatVolume_muiNextFreeCluster
		ldx F_WORK
		jsr SetMember32
		ldy CFatVolume_muiTotalFreeClusters
		lda [[D + F_THIS]:24 + Y]
		sec
		sbc 1
		sta [[D + F_THIS]:24 + Y]
		iny
		iny
		lda [[D + F_THIS]:24 + Y]
		sbc 0
		sta [[D + F_THIS]:24 + Y]
		; The end of chain: 0xFFFF on FAT16, 0x0FFFFFFF (the whole 32 bits) on FAT32.
		lda [D + AL_SECTOR]
		sta [D + F_SECTOR]
		lda [D + AL_SECTOR + 2]
		sta [D + F_SECTOR + 2]
		lda [D + AL_OFFSET]
		sta [D + F_OFFSET]
		lda 0xFFFF
		ldx FAT32_EOC >> 16
		jsr WriteFatEntryHere
		bcc WriteAllocatedLink
		rts
WriteAllocatedLink:
		lda [D + AL_LAST]
		ldx [D + AL_LAST + 2]
		jsr IsEndOfChainMarker
		bne WriteAllocatedPrevious
		clc
		rts
WriteAllocatedPrevious:
		lda [D + AL_LAST_SECTOR]
		sta [D + F_SECTOR]
		lda [D + AL_LAST_SECTOR + 2]
		sta [D + F_SECTOR + 2]
		lda [D + AL_LAST_OFFSET]
		sta [D + F_OFFSET]
		jsr ReadCache
		bcc WriteAllocatedPreviousRead
		rts
WriteAllocatedPreviousRead:
		lda [D + F_CACHE]
		clc
		adc [D + F_OFFSET]
		sta [D + F_POINTER]
		lda [D + F_CACHE + 2]
		adc 0
		sta [D + F_POINTER + 2]
		lda [D + AL_CLUSTER]
		sta [[D + F_POINTER]:24]
		jsr IsFat32
		bne WriteAllocatedDirty
		; FAT32: the entry's top 4 bits are kept.
		ldy 2
		lda [[D + F_POINTER]:24 + Y]
		and (FAT32_CLUSTER_MASK >> 16) ^ 0xFFFF
		sta [D + F_WORK]
		lda [D + AL_CLUSTER + 2]
		and FAT32_CLUSTER_MASK >> 16
		ora [D + F_WORK]
		sta [[D + F_POINTER]:24 + Y]
WriteAllocatedDirty:
		jsr DirtyCache
		clc
		rts


; Z set if X (high) and A (low) is exactly the volume's end of chain marker (FAT16_EOC or FAT32_EOC), as
; WriteFat16Entry and WriteFat32Entry check the previous entry.
IsEndOfChainMarker:
		.width (A, X), 16
		cmp 0xFFFF
		bne IsEndOfChainMarkerReturn
		jsr IsFat32
		beq IsEndOfChainMarker32
		lda 0
		rts
IsEndOfChainMarker32:
		txa
		cmp FAT32_EOC >> 16
IsEndOfChainMarkerReturn:
		rts


; InitializeDirectoryCluster(AL_PARENT, F_CLUSTER): the new directory's first sector zeroed with its "." and ".."
; entries, the cluster's other sectors erased.  Carry set (A the error) if it couldn't.
InitialiseDirectoryCluster:
		.width (A, X), 16
		jsr FirstSector
		jsr ReadCache
		bcc InitialiseDirectoryRead
		rts
InitialiseDirectoryRead:
		ldy CFatVolume_muiBytesPerSector
		lda [[D + F_THIS]:24 + Y]
		pea 0
		pha
		pea 0
		pei [D + F_CACHE + 2]
		pei [D + F_CACHE]
		jsl _memset
		tsc
		clc
		adc 10
		tcs
		; "." is this cluster.
		lda [D + F_CACHE]
		sta [D + F_POINTER]
		lda [D + F_CACHE + 2]
		sta [D + F_POINTER + 2]
		lda '.' | (' ' << 8)
		jsr DottyEntry
		; ".." is the parent's cluster (GetEntryCluster), 0 when the parent is the FAT32 root.
		lda [D + F_POINTER]
		clc
		adc SIZEOF_SFatRawDirectoryEntry
		sta [D + F_POINTER]
		bcc InitialiseDotDot
		inc [D + F_POINTER + 2]
InitialiseDotDot:
		pei [D + F_CLUSTER + 2]
		pei [D + F_CLUSTER]
		; EntryFirstCluster points F_POINTER at the parent's entry: the ".." entry's pointer is kept.
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		ldx AL_PARENT
		jsr EntryFirstCluster
		pla
		sta [D + F_POINTER]
		pla
		sta [D + F_POINTER + 2]
		jsr IsFat32
		bne InitialiseDotDotCluster
		ldy CFatVolume_muiRootCluster
		lda [[D + F_THIS]:24 + Y]
		cmp [D + F_CLUSTER]
		bne InitialiseDotDotCluster
		iny
		iny
		lda [[D + F_THIS]:24 + Y]
		cmp [D + F_CLUSTER + 2]
		bne InitialiseDotDotCluster
		stz [D + F_CLUSTER]
		stz [D + F_CLUSTER + 2]
InitialiseDotDotCluster:
		lda '.' | ('.' << 8)
		jsr DottyEntry
		pla
		sta [D + F_CLUSTER]
		pla
		sta [D + F_CLUSTER + 2]
		jsr DirtyCache
		; The cluster's other sectors erased (if it has any).
		ldy CFatVolume_muiNoOfSectorsPerCluster
		lda [[D + F_THIS]:24 + Y]
		cmp 2
		bcc InitialiseDirectoryDone
		jsr FirstSector
		inc [D + F_SECTOR]
		bne InitialiseDirectoryLast
		inc [D + F_SECTOR + 2]
InitialiseDirectoryLast:
		ldy CFatVolume_muiNoOfSectorsPerCluster
		lda [[D + F_THIS]:24 + Y]
		sec
		sbc 2
		clc
		adc [D + F_SECTOR]
		sta [D + F_OFFSET]
		lda [D + F_SECTOR + 2]
		adc 0
		sta [D + F_OFFSET + 2]
		jsr EraseSectors
		cmp 0
		bne InitialiseDirectoryDone
		lda FAT_CANNOT_WRITE_MEDIA
		sec
		rts
InitialiseDirectoryDone:
		lda FAT_SUCCESS
		clc
		rts


; InitialiseDottyDirectoryEntry(F_POINTER, A (its 2 name characters), F_CLUSTER): a directory entry named . or ..
DottyEntry:
		.width (A, X), 16
		sta [[D + F_POINTER]:24]
		lda ' ' | (' ' << 8)
		ldy 2
DottyEntrySpaces:
		sta [[D + F_POINTER]:24 + Y]
		iny
		iny
		cpy 10
		bne DottyEntrySpaces
		lda ' ' | (FAT_ATTR_DIRECTORY << 8)
		sta [[D + F_POINTER]:24 + Y]
		lda 0
		ldy RAW_uiReserved
		sta [[D + F_POINTER]:24 + Y]
		ldy RAW_uiSize
		sta [[D + F_POINTER]:24 + Y]
		ldy RAW_uiSize + 2
		sta [[D + F_POINTER]:24 + Y]
		lda [D + F_CLUSTER]
		ldy RAW_uiFirstClusterLowWord
		sta [[D + F_POINTER]:24 + Y]
		lda [D + F_CLUSTER + 2]
		ldy RAW_uiFirstClusterHighWord
		sta [[D + F_POINTER]:24 + Y]
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		jsl _FatSetCreateDateTime__FPC21SFatRawDirectoryEntry
		ply
		ply
		ldy RAW_uiCreateDate
		lda [[D + F_POINTER]:24 + Y]
		ldy RAW_uiModifyDate
		sta [[D + F_POINTER]:24 + Y]
		ldy RAW_uiAccessDate
		sta [[D + F_POINTER]:24 + Y]
		ldy RAW_uiCreateTime
		lda [[D + F_POINTER]:24 + Y]
		ldy RAW_uiModifyTime
		sta [[D + F_POINTER]:24 + Y]
		lda 0xC6
		sep m
		ldy RAW_uiCreateTimeTenths
		sta [[D + F_POINTER]:24 + Y]
		rep m
		rts


; ------------------------------------------------------------------------------------------------------------------
; Creating directory entries
; ------------------------------------------------------------------------------------------------------------------

; EFatCode CFatVolume::CreateFATEntry(SFatRawDirectoryEntry* psParentDirectory, const char* szName, uint8 uiAttributes,
;                                     uint32 uiEntryCluster, SFatDirectoryEntry* psNewEntry)
CE_PARENT		= F_ARGUMENT
CE_NAME			= F_ARGUMENT + 4
CE_ATTRIBUTES	= F_ARGUMENT + 8
CE_CLUSTER		= F_ARGUMENT + 10
CE_ENTRY		= F_ARGUMENT + 14
CE_NEEDED		= F_LOCALS + 24		; iLFNEntriesNeeded (above the query helpers' locals: UniqueShortName queries)
CE_RESULT		= F_LOCALS + 26
CE_LENGTH		= F_LOCALS + 28		; strlen(szName)
CE_SHORT		= F_LOCALS + 30		; &psNewEntry->sRaw.szShortName
CE_SUFFIX		= F_LOCALS + 34
CE_VALID		= F_LOCALS + 36
		.global _CFatVolume__CreateFATEntry
_CFatVolume__CreateFATEntry:
		.width (A, X), 16
		jsr Enter
		; CheckEntryName: not too long, no illegal characters.
		pei [D + CE_NAME + 2]
		pei [D + CE_NAME]
		jsl _strlen
		ply
		ply
		sta [D + CE_LENGTH]
		txy
		bne CreateEntryTooLong
		cmp FAT_MAX_FILENAME + 1
		bcc CreateEntryLength
CreateEntryTooLong:
		lda FAT_FILENAME_TOO_LONG
		jmp Leave
CreateEntryLength:
		pha
		pei [D + CE_NAME + 2]
		pei [D + CE_NAME]
		jsl _IsIllegalFilename__FPct
		ply
		ply
		ply
		and 0xFF
		beq CreateEntryLegal
		lda FAT_INVALID_FILENAME
		jmp Leave
CreateEntryLegal:
		; memset(&psNewEntry->sRaw, 0, sizeof(SFatRawDirectoryEntry)); the short name (or one made up for a long name).
		lda [D + CE_ENTRY]
		clc
		adc SFatDirectoryEntry_sRaw
		sta [D + CE_SHORT]
		lda [D + CE_ENTRY + 2]
		adc 0
		sta [D + CE_SHORT + 2]
		lda 0
		ldy SIZEOF_SFatRawDirectoryEntry - 2
CreateEntryClear:
		sta [[D + CE_SHORT]:24 + Y]
		dey
		dey
		bpl CreateEntryClear
		pea 0
		pei [D + CE_NAME + 2]
		pei [D + CE_NAME]
		pei [D + CE_SHORT + 2]
		pei [D + CE_SHORT]
		jsl _GetFatShortNameForEntry__FPcPcb
		tay
		tsc
		clc
		adc 10
		tcs
		tya
		ldx 1
		cmp FAT_SUCCESS
		beq CreateEntryNeeded
		cmp FAT_LFN_GENERATED
		beq CreateEntryLongName
		lda FAT_INVALID_FILENAME
		jmp Leave
CreateEntryLongName:
		jsr UniqueShortName
		cmp FAT_SUCCESS
		beq CreateEntryUnique
		jmp Leave
CreateEntryUnique:
		; (strlen + 12) / 13 long name entries, and the short name's.
		lda [D + CE_LENGTH]
		clc
		adc 12
		ldx 1
CreateEntryThirteens:
		cmp 13
		bcc CreateEntryNeeded
		sbc 13
		inx
		bra CreateEntryThirteens
CreateEntryNeeded:
		stx [D + CE_NEEDED]
		; A new directory gets its cluster now.
		lda [D + CE_CLUSTER]
		ora [D + CE_CLUSTER + 2]
		bne CreateEntryInitialise
		lda [D + CE_ATTRIBUTES]
		and FAT_ATTR_DIRECTORY
		beq CreateEntryInitialise
		; AllocateDirectoryCluster(psParentDirectory, &eResult): AllocateClusters(parent, 1, true, 1, &eResult, GetEndOfClusterMarker())
		jsr EndOfChainMarker
		phx
		pha
		pea 0
		tdc
		clc
		adc CE_RESULT
		pha
		pea 0
		pea 1
		pea 1
		pea 0
		pea 1
		pei [D + CE_PARENT + 2]
		pei [D + CE_PARENT]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl AllocateClusters
		tay
		tsc
		clc
		adc 26
		tcs
		tya
		sta [D + CE_CLUSTER]
		stx [D + CE_CLUSTER + 2]
		lda [D + CE_RESULT]
		cmp FAT_SUCCESS
		beq CreateEntryInitialise
		jmp Leave
CreateEntryInitialise:
		jsr InitialiseNewEntry
		; WriteFATEntry(psNewEntry, szName, psParentDirectory, iLFNEntriesNeeded)
		pei [D + CE_NEEDED]
		pei [D + CE_PARENT + 2]
		pei [D + CE_PARENT]
		pei [D + CE_NAME + 2]
		pei [D + CE_NAME]
		pei [D + CE_ENTRY + 2]
		pei [D + CE_ENTRY]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl WriteFATEntry
		tay
		tsc
		clc
		adc 18
		tcs
		tya
		jmp Leave


; GetEndOfClusterMarker() in A (low) and X (high).
EndOfChainMarker:
		.width (A, X), 16
		ldx 0
		jsr IsFat32
		bne EndOfChainMarkerReturn
		ldx FAT32_EOC >> 16
EndOfChainMarkerReturn:
		lda 0xFFFF
		rts


; InitialiseNewEntry(psNewEntry, szName, uiAttributes, uiEntryCluster): CreateFATEntry's new entry.
InitialiseNewEntry:
		.width (A, X), 16
		pei [D + CE_NAME + 2]
		pei [D + CE_NAME]
		pei [D + CE_ENTRY + 2]
		pei [D + CE_ENTRY]
		jsl _strcpy
		tsc
		clc
		adc 8
		tcs
		lda [D + CE_ENTRY]
		sta [D + F_POINTER]
		lda [D + CE_ENTRY + 2]
		sta [D + F_POINTER + 2]
		lda [D + CE_ATTRIBUTES]
		sep m
		ldy SFatDirectoryEntry_uiAttributes
		sta [[D + F_POINTER]:24 + Y]
		ldy SFatDirectoryEntry_sRaw + RAW_uiAttributes
		sta [[D + F_POINTER]:24 + Y]
		rep m
		lda 0
		ldy SFatDirectoryEntry_uiSize
		sta [[D + F_POINTER]:24 + Y]
		ldy SFatDirectoryEntry_uiSize + 2
		sta [[D + F_POINTER]:24 + Y]
		ldy SFatDirectoryEntry_sRaw + RAW_uiSize
		sta [[D + F_POINTER]:24 + Y]
		ldy SFatDirectoryEntry_sRaw + RAW_uiSize + 2
		sta [[D + F_POINTER]:24 + Y]
		; uiReserved and uiCreateTimeTenths
		ldy SFatDirectoryEntry_sRaw + RAW_uiReserved
		sta [[D + F_POINTER]:24 + Y]
		lda [D + CE_CLUSTER]
		ldy SFatDirectoryEntry_sRaw + RAW_uiFirstClusterLowWord
		sta [[D + F_POINTER]:24 + Y]
		lda [D + CE_CLUSTER + 2]
		ldy SFatDirectoryEntry_sRaw + RAW_uiFirstClusterHighWord
		sta [[D + F_POINTER]:24 + Y]
		lda [D + F_POINTER]
		clc
		adc SFatDirectoryEntry_sRaw
		pha
		lda [D + F_POINTER + 2]
		adc 0
		sta [D + F_POINTER + 2]
		pla
		sta [D + F_POINTER]
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		jsl _FatSetCreateDateTime__FPC21SFatRawDirectoryEntry
		ply
		ply
		ldy RAW_uiCreateDate
		lda [[D + F_POINTER]:24 + Y]
		ldy RAW_uiModifyDate
		sta [[D + F_POINTER]:24 + Y]
		ldy RAW_uiAccessDate
		sta [[D + F_POINTER]:24 + Y]
		ldy RAW_uiCreateTime
		lda [[D + F_POINTER]:24 + Y]
		ldy RAW_uiModifyTime
		sta [[D + F_POINTER]:24 + Y]
		rts


; GenerateUniqueShortNameWithSuffix(psParentDirectory, sRaw.szShortName): a "~n" short name no entry in the parent
; has.  A = the result.  The query is on the stack, under the return address.
UniqueShortName:
		.width (A, X), 16
		ply
		tsc
		sec
		sbc SIZEOF_SFatQueryState
		tcs
		inc A
		sta [D + Q_QUERY]
		stz [D + Q_QUERY + 2]
		phy
		stz [D + CE_SUFFIX]
		pei [D + Q_QUERY + 2]
		pei [D + Q_QUERY]
		jsl _SFatQueryState__Init
		ply
		ply
UniqueShortNameTry:
		lda 1
		sta [D + CE_VALID]
		jsr KillQuery
		pea 0
		pei [D + Q_QUERY + 2]
		pei [D + Q_QUERY]
		pea 0
		pei [D + CE_PARENT + 2]
		pei [D + CE_PARENT]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl QueryFirstEntry
		tay
		tsc
		clc
		adc 16
		tcs
		tya
		cmp FAT_SUCCESS
		bne UniqueShortNameDone
		jsr NextShortName
UniqueShortNameEntry:
		jsr QueryEntry
		lda [[D + Q_ENTRY]:24]
		and 0xFF
		beq UniqueShortNameChecked
		; memcmp(szShortName, the entry's, 11)
		pea 0
		pea 11
		pei [D + CE_SHORT + 2]
		pei [D + CE_SHORT]
		pei [D + Q_ENTRY + 2]
		pei [D + Q_ENTRY]
		jsl _memcmp
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		cmp 0
		bne UniqueShortNameNext
		stz [D + CE_VALID]
		bra UniqueShortNameChecked
UniqueShortNameNext:
		pea 0
		pea 0
		pei [D + Q_QUERY + 2]
		pei [D + Q_QUERY]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl QueryNextEntry
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		cmp FAT_SUCCESS
		beq UniqueShortNameEntry
		bra UniqueShortNameDone
UniqueShortNameChecked:
		lda [D + CE_VALID]
		bne UniqueShortNameFound
		jsr NextShortName
		bra UniqueShortNameTry
UniqueShortNameFound:
		lda FAT_SUCCESS
UniqueShortNameDone:
		sta [D + CE_RESULT]
		jsr KillQuery
		ply
		tsc
		clc
		adc SIZEOF_SFatQueryState
		tcs
		phy
		lda [D + CE_RESULT]
		rts


; GenerateShortNameWithSuffix(uiNameSuffix++, szShortName)
NextShortName:
		.width (A, X), 16
		pei [D + CE_SHORT + 2]
		pei [D + CE_SHORT]
		pei [D + CE_SUFFIX]
		jsl _GenerateShortNameWithSuffix__FtPh
		ply
		ply
		ply
		inc [D + CE_SUFFIX]
		rts


; EFatCode WriteFATEntry(SFatDirectoryEntry* psNewEntry, const char* szName, SFatRawDirectoryEntry* psParentDirectory,
;                        int iLFNEntriesNeeded): the long name's entries then the short name's, in the first run of
; free entries long enough (the directory grows by a cluster if it has none).
WE_ENTRY		= F_ARGUMENT
WE_NAME			= F_ARGUMENT + 4
WE_PARENT		= F_ARGUMENT + 8
WE_NEEDED		= F_ARGUMENT + 12
WE_SECTOR		= F_LOCALS			; uiSector
WE_FOUND		= F_LOCALS + 4		; iLFNEntriesFound
WE_FIRST		= F_LOCALS + 6		; uiFirstSectorOfCluster
WE_DIRECTORY	= F_LOCALS + 10		; uiDirectoryCluster
WE_LAST_DIRECTORY	= F_LOCALS + 14	; uiLastDirectoryCluster
WE_CHECKSUM		= F_LOCALS + 18
WE_PARENT_ENTRY	= F_LOCALS + 20		; psParentEntry
WE_ENTRIES		= F_LOCALS + 24		; uiDirectoryEntries
WE_ALL			= F_LOCALS + 26		; bAllLFNEntriesFound
WE_LENGTH		= F_LOCALS + 28		; strlen(szName)
WE_LAST_ENTRY	= F_LOCALS + 30		; The sector's last entry.
WE_FIRST_DIRECTORY	= F_LOCALS + 34	; GetDirectoryCluster(psParentDirectory)
WE_RESULT		= F_LOCALS + 38
WriteFATEntry:
		.width (A, X), 16
		jsr Enter
		jsr FindEnoughEntries
		bcc WriteEntryEnough
		jmp Leave
WriteEntryEnough:
		jsr RewindFoundEntries
		bcc WriteEntryRewound
		jmp Leave
WriteEntryRewound:
		lda [D + WE_ENTRY]
		clc
		adc SFatDirectoryEntry_sRaw + RAW_szShortName
		sta [D + F_POINTER]
		lda [D + WE_ENTRY + 2]
		adc 0
		sta [D + F_POINTER + 2]
		jsr Checksum
		sta [D + WE_CHECKSUM]
		pei [D + WE_NAME + 2]
		pei [D + WE_NAME]
		jsl _strlen
		ply
		ply
		sta [D + WE_LENGTH]
		lda [D + WE_NEEDED]
		sta [D + WE_FOUND]
WriteEntryNext:
		lda [D + WE_FOUND]
		bne WriteEntryMore
		jmp WriteEntryWritten
WriteEntryMore:
		dec A
		sta [D + WE_FOUND]
		beq WriteEntryShort
		; A long name entry.
		jsr ReadWriteSector
		bcc WriteEntryLongRead
		jmp Leave
WriteEntryLongRead:
		jsr InitialiseParentEntry
		jsr DirtyCache
		lda [D + WE_PARENT_ENTRY + 2]
		cmp [D + WE_LAST_ENTRY + 2]
		bne WriteEntryCompared
		lda [D + WE_PARENT_ENTRY]
		cmp [D + WE_LAST_ENTRY]
WriteEntryCompared:
		bcs WriteEntryNextSector
		lda [D + WE_PARENT_ENTRY]
		clc
		adc SIZEOF_SFatRawDirectoryEntry
		sta [D + WE_PARENT_ENTRY]
		bcc WriteEntryNext
		inc [D + WE_PARENT_ENTRY + 2]
		bra WriteEntryNext
WriteEntryNextSector:
		; The next sector, or the next cluster (allocated if the directory has no more).
		lda [D + WE_DIRECTORY]
		ora [D + WE_DIRECTORY + 2]
		beq WriteEntryIncrementSector
		jsr FirstSectorOfClusterEnd
		bcs WriteEntryIncrementCluster
WriteEntryIncrementSector:
		jsr IncrementDirectorySector
		bcc WriteEntryIncremented
		jmp Leave
WriteEntryIncrementCluster:
		lda [D + WE_DIRECTORY]
		sta [D + WE_LAST_DIRECTORY]
		lda [D + WE_DIRECTORY + 2]
		sta [D + WE_LAST_DIRECTORY + 2]
		jsr IncrementOrAllocateDirectoryCluster
		bcc WriteEntryAllocated
		jmp Leave
WriteEntryAllocated:
		lda [D + WE_SECTOR]
		sta [D + WE_FIRST]
		lda [D + WE_SECTOR + 2]
		sta [D + WE_FIRST + 2]
WriteEntryIncremented:
		jsr ReadWriteSector
		bcc WriteEntrySectorRead
		jmp Leave
WriteEntrySectorRead:
		lda [D + F_CACHE]
		sta [D + WE_PARENT_ENTRY]
		lda [D + F_CACHE + 2]
		sta [D + WE_PARENT_ENTRY + 2]
		jmp WriteEntryNext
WriteEntryShort:
		; The short name entry: the new entry's raw entry.
		jsr ReadWriteSector
		bcc WriteEntryShortRead
		jmp Leave
WriteEntryShortRead:
		lda [D + WE_ENTRY]
		clc
		adc SFatDirectoryEntry_sRaw
		sta [D + F_WORK]
		lda [D + WE_ENTRY + 2]
		adc 0
		sta [D + F_WORK + 2]
		lda [D + WE_PARENT_ENTRY]
		sta [D + F_POINTER]
		lda [D + WE_PARENT_ENTRY + 2]
		sta [D + F_POINTER + 2]
		ldy SIZEOF_SFatRawDirectoryEntry - 2
		jsr CopyWords
		jsr DirtyCache
		jmp WriteEntryNext
WriteEntryWritten:
		jsr ValidateCache
		bcc WriteEntryValid
		jmp Leave
WriteEntryValid:
		lda [D + WE_ENTRY]
		sta [D + F_POINTER]
		lda [D + WE_ENTRY + 2]
		sta [D + F_POINTER + 2]
		lda [D + WE_SECTOR]
		ldy SFatDirectoryEntry_uiSectorAddress
		sta [[D + F_POINTER]:24 + Y]
		lda [D + WE_SECTOR + 2]
		ldy SFatDirectoryEntry_uiSectorAddress + 2
		sta [[D + F_POINTER]:24 + Y]
		lda [D + WE_PARENT_ENTRY]
		sec
		sbc [D + F_CACHE]
		ldy SFatDirectoryEntry_uiSectorOffset
		sta [[D + F_POINTER]:24 + Y]
		lda FAT_SUCCESS
		jmp Leave


; Reads WE_SECTOR into F_CACHE, and WE_LAST_ENTRY = its last entry.  Carry set (A the error) if it couldn't.
ReadWriteSector:
		.width (A, X), 16
		lda [D + WE_SECTOR]
		sta [D + F_SECTOR]
		lda [D + WE_SECTOR + 2]
		sta [D + F_SECTOR + 2]
		jsr ReadCache
		bcs ReadWriteSectorReturn
		ldy CFatVolume_muiBytesPerSector
		lda [[D + F_THIS]:24 + Y]
		sec
		sbc SIZEOF_SFatRawDirectoryEntry
		clc
		adc [D + F_CACHE]
		sta [D + WE_LAST_ENTRY]
		lda [D + F_CACHE + 2]
		adc 0
		sta [D + WE_LAST_ENTRY + 2]
		lda FAT_SUCCESS
		clc
ReadWriteSectorReturn:
		rts


; Carry set if WE_SECTOR is its cluster's last (uiSector >= uiFirstSectorOfCluster + NumSectorsPerCluster() - 1).
FirstSectorOfClusterEnd:
		.width (A, X), 16
		ldy CFatVolume_muiNoOfSectorsPerCluster
		lda [[D + F_THIS]:24 + Y]
		dec A
		clc
		adc [D + WE_FIRST]
		sta [D + F_WORK]
		lda [D + WE_FIRST + 2]
		adc 0
		sta [D + F_WORK + 2]
		lda [D + WE_SECTOR + 2]
		cmp [D + F_WORK + 2]
		bne FirstSectorOfClusterEndReturn
		lda [D + WE_SECTOR]
		cmp [D + F_WORK]
FirstSectorOfClusterEndReturn:
		rts


; IncrementDirectorySector(&uiSector, uiFirstSectorOfCluster, uiDirectoryCluster): the next sector; past the FAT16 root
; directory's sectors is FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED (carry set).
IncrementDirectorySector:
		.width (A, X), 16
		inc [D + WE_SECTOR]
		bne IncrementDirectorySectorRoot
		inc [D + WE_SECTOR + 2]
IncrementDirectorySectorRoot:
		lda [D + WE_DIRECTORY]
		ora [D + WE_DIRECTORY + 2]
		bne IncrementDirectorySectorDone
		jsr RootLimit
		bcc IncrementDirectorySectorDone
		lda FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED
		rts
IncrementDirectorySectorDone:
		lda FAT_SUCCESS
		clc
		rts


; Carry set if WE_SECTOR is past the FAT16 root directory (> WE_FIRST + muiRootDirectorySectors).
RootLimit:
		.width (A, X), 16
		ldy CFatVolume_muiRootDirectorySectors
		lda [[D + F_THIS]:24 + Y]
		clc
		adc [D + WE_FIRST]
		sta [D + F_WORK]
		lda [D + WE_FIRST + 2]
		adc 0
		cmp [D + WE_SECTOR + 2]
		bne RootLimitCompared
		lda [D + F_WORK]
		cmp [D + WE_SECTOR]
RootLimitCompared:
		; Carry clear if WE_FIRST + sectors < WE_SECTOR: past it.
		bcc RootLimitPast
		clc
		rts
RootLimitPast:
		sec
		rts


; IncrementOrAllocateDirectoryCluster(&uiSector, &uiDirectoryCluster, uiLastDirectoryCluster): the directory's next
; cluster (a new one, zeroed and linked, at its end); WE_SECTOR its first sector.  Carry set (A the error) if not.
IncrementOrAllocateDirectoryCluster:
		.width (A, X), 16
		lda [D + WE_DIRECTORY]
		sta [D + F_CLUSTER]
		lda [D + WE_DIRECTORY + 2]
		sta [D + F_CLUSTER + 2]
		jsr NextCluster
		bcc IncrementClusterRead
		rts
IncrementClusterRead:
		lda [D + F_CLUSTER]
		ldx [D + F_CLUSTER + 2]
		jsr IsEndOfChain
		bcc IncrementClusterHave
		lda [D + WE_LAST_DIRECTORY]
		ldx [D + WE_LAST_DIRECTORY + 2]
		jsr AllocateDirectoryData
		bcc IncrementClusterHave
		rts
IncrementClusterHave:
		lda [D + F_CLUSTER]
		sta [D + WE_DIRECTORY]
		lda [D + F_CLUSTER + 2]
		sta [D + WE_DIRECTORY + 2]
		jsr FirstSector
		lda [D + F_SECTOR]
		sta [D + WE_SECTOR]
		lda [D + F_SECTOR + 2]
		sta [D + WE_SECTOR + 2]
		lda FAT_SUCCESS
		clc
		rts


; A new zeroed data cluster after the directory's last (X high, A low: AllocateDataCluster(1, true, &eResult, it)),
; linked from WE_LAST_DIRECTORY (SetClusterEntry); F_CLUSTER is the new cluster.  Carry set (A the error) if not.
AllocateDirectoryData:
		.width (A, X), 16
		phx
		pha
		pea 0
		tdc
		clc
		adc WE_RESULT
		pha
		pea 0
		pea 1
		pea 1
		pea 0
		pea 1
		pea 0
		pea 0
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl AllocateClusters
		tay
		tsc
		clc
		adc 26
		tcs
		tya
		sta [D + F_WORK]
		stx [D + F_WORK + 2]
		lda [D + WE_RESULT]
		cmp FAT_SUCCESS
		bne AllocateDirectoryDataFailed
		; SetClusterEntry(uiLastDirectoryCluster, uiNewFat)
		lda [D + WE_LAST_DIRECTORY]
		sta [D + F_CLUSTER]
		lda [D + WE_LAST_DIRECTORY + 2]
		sta [D + F_CLUSTER + 2]
		jsr FatEntryPlace
		lda [D + F_WORK]
		ldx [D + F_WORK + 2]
		pha
		phx
		jsr WriteFatEntryHere
		plx
		ply
		bcs AllocateDirectoryDataReturn
		sty [D + F_CLUSTER]
		stx [D + F_CLUSTER + 2]
		lda FAT_SUCCESS
		clc
		rts
AllocateDirectoryDataFailed:
		sec
AllocateDirectoryDataReturn:
		rts


; FindEnoughEntries: the first run of iLFNEntriesNeeded free entries (or the directory's last entry), growing the
; directory by a cluster at its end if it has none.  WE_DIRECTORY, WE_FIRST, WE_SECTOR, WE_PARENT_ENTRY and WE_FOUND
; are where the run ends.  Carry set (A the error) if not.
FindEnoughEntries:
		.width (A, X), 16
		; GetDirectoryCluster and GetDirectorySector
		ldx WE_PARENT
		jsr IsRootEntry
		bne FindEnoughDirectory
		ldy CFatVolume_muiRootCluster
		ldx WE_DIRECTORY
		jsr GetMember32
		ldy CFatVolume_muiRootSector
		ldx WE_FIRST
		jsr GetMember32
		bra FindEnoughStart
FindEnoughDirectory:
		ldx WE_PARENT
		jsr EntryFirstCluster
		lda [D + F_CLUSTER]
		sta [D + WE_DIRECTORY]
		lda [D + F_CLUSTER + 2]
		sta [D + WE_DIRECTORY + 2]
		stz [D + WE_FIRST]
		stz [D + WE_FIRST + 2]
FindEnoughStart:
		lda [D + WE_DIRECTORY]
		sta [D + WE_FIRST_DIRECTORY]
		lda [D + WE_DIRECTORY + 2]
		sta [D + WE_FIRST_DIRECTORY + 2]
		stz [D + WE_ENTRIES]
		stz [D + WE_FOUND]
		stz [D + WE_ALL]
		stz [D + WE_PARENT_ENTRY]
		stz [D + WE_PARENT_ENTRY + 2]
FindEnoughCluster:
		lda [D + WE_DIRECTORY]
		ora [D + WE_DIRECTORY + 2]
		beq FindEnoughFirstSector
		lda [D + WE_DIRECTORY]
		sta [D + F_CLUSTER]
		lda [D + WE_DIRECTORY + 2]
		sta [D + F_CLUSTER + 2]
		jsr FirstSector
		lda [D + F_SECTOR]
		sta [D + WE_FIRST]
		lda [D + F_SECTOR + 2]
		sta [D + WE_FIRST + 2]
FindEnoughFirstSector:
		lda [D + WE_FIRST]
		sta [D + WE_SECTOR]
		lda [D + WE_FIRST + 2]
		sta [D + WE_SECTOR + 2]
FindEnoughSector:
		; The FAT16 root directory, or a sector of the cluster.
		lda [D + WE_DIRECTORY]
		ora [D + WE_DIRECTORY + 2]
		beq FindEnoughRead
		jsr FirstSectorOfClusterEnd
		bcc FindEnoughRead
		beq FindEnoughRead
		jmp FindEnoughNextCluster
FindEnoughRead:
		jsr ReadWriteSector
		bcc FindEnoughSectorRead
		rts
FindEnoughSectorRead:
		lda [D + F_CACHE]
		sta [D + WE_PARENT_ENTRY]
		lda [D + F_CACHE + 2]
		sta [D + WE_PARENT_ENTRY + 2]
FindEnoughEntry:
		lda [D + WE_PARENT_ENTRY + 2]
		cmp [D + WE_LAST_ENTRY + 2]
		bne FindEnoughCompared
		lda [D + WE_PARENT_ENTRY]
		cmp [D + WE_LAST_ENTRY]
FindEnoughCompared:
		beq FindEnoughLook
		bcs FindEnoughSectorDone
FindEnoughLook:
		lda [D + WE_ENTRIES]
		clc
		adc [D + WE_NEEDED]
		cmp FAT_MAX_DIRECTORY_ENTRIES
		bne FindEnoughCount
		lda FAT_DIRECTORY_LIMIT_EXCEEDED
		sec
		rts
FindEnoughCount:
		inc [D + WE_ENTRIES]
		lda [[D + WE_PARENT_ENTRY]:24]
		and 0xFF
		beq FindEnoughFreeLast
		cmp FAT_DELETED_ENTRY
		bne FindEnoughUsed
		inc [D + WE_FOUND]
		lda [D + WE_FOUND]
		cmp [D + WE_NEEDED]
		beq FindEnoughAll
		bra FindEnoughStep
FindEnoughFreeLast:
		inc [D + WE_FOUND]
FindEnoughAll:
		lda 1
		sta [D + WE_ALL]
		bra FindEnoughSectorDone
FindEnoughUsed:
		stz [D + WE_FOUND]
FindEnoughStep:
		lda [D + WE_PARENT_ENTRY]
		clc
		adc SIZEOF_SFatRawDirectoryEntry
		sta [D + WE_PARENT_ENTRY]
		bcc FindEnoughEntry
		inc [D + WE_PARENT_ENTRY + 2]
		bra FindEnoughEntry
FindEnoughSectorDone:
		lda [D + WE_ALL]
		bne FindEnoughFound
		inc [D + WE_SECTOR]
		bne FindEnoughSectorRoot
		inc [D + WE_SECTOR + 2]
FindEnoughSectorRoot:
		lda [D + WE_DIRECTORY]
		ora [D + WE_DIRECTORY + 2]
		bne FindEnoughSectorNext
		jsr RootLimit
		bcc FindEnoughSectorNext
		lda FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED
		rts
FindEnoughSectorNext:
		jmp FindEnoughSector
FindEnoughNextCluster:
		; The cluster's sectors are used: the next cluster, or a new one.
		lda [D + WE_DIRECTORY]
		sta [D + WE_LAST_DIRECTORY]
		sta [D + F_CLUSTER]
		lda [D + WE_DIRECTORY + 2]
		sta [D + WE_LAST_DIRECTORY + 2]
		sta [D + F_CLUSTER + 2]
		jsr NextCluster
		bcc FindEnoughNextRead
		rts
FindEnoughNextRead:
		lda [D + F_CLUSTER]
		ldx [D + F_CLUSTER + 2]
		jsr IsEndOfChain
		bcc FindEnoughHaveCluster
		; AllocateDataCluster(1, true, &eResult, GetEndOfClusterMarker()), then SetClusterEntry(uiLastDirectoryCluster, it).
		jsr EndOfChainMarker
		jsr AllocateDirectoryData
		bcc FindEnoughHaveCluster
		rts
FindEnoughHaveCluster:
		lda [D + F_CLUSTER]
		sta [D + WE_DIRECTORY]
		lda [D + F_CLUSTER + 2]
		sta [D + WE_DIRECTORY + 2]
		jmp FindEnoughCluster
FindEnoughFound:
		lda FAT_SUCCESS
		clc
		rts


; RewindFoundEntries: back from the run's end (WE_PARENT_ENTRY in WE_SECTOR) to its first entry, iLFNEntriesFound - 1
; entries back, into earlier sectors and clusters.  Carry set (A the error) if not.
RewindFoundEntries:
		.width (A, X), 16
RewindNext:
		lda [D + WE_FOUND]
		cmp 2
		bcs RewindEntry
		lda FAT_SUCCESS
		clc
		rts
RewindEntry:
		jsr ReadWriteSector
		bcc RewindRead
		rts
RewindRead:
		lda [D + WE_PARENT_ENTRY + 2]
		cmp [D + F_CACHE + 2]
		bne RewindCompared
		lda [D + WE_PARENT_ENTRY]
		cmp [D + F_CACHE]
RewindCompared:
		beq RewindSector
		bcc RewindSector
		lda [D + WE_PARENT_ENTRY]
		sec
		sbc SIZEOF_SFatRawDirectoryEntry
		sta [D + WE_PARENT_ENTRY]
		bcs RewindCount
		dec [D + WE_PARENT_ENTRY + 2]
		bra RewindCount
RewindSector:
		; The entries may start in an earlier sector, or an earlier cluster (a FAT16 root directory is one run of sectors).
		lda [D + WE_SECTOR + 2]
		cmp [D + WE_FIRST + 2]
		bne RewindSectorCompared
		lda [D + WE_SECTOR]
		cmp [D + WE_FIRST]
RewindSectorCompared:
		beq RewindCluster
		bcc RewindCluster
		lda [D + WE_SECTOR]
		bne RewindSectorDecrement
		dec [D + WE_SECTOR + 2]
RewindSectorDecrement:
		dec [D + WE_SECTOR]
		bra RewindLastEntry
RewindCluster:
		; FindPreviousCluster(GetDirectoryCluster(psParentDirectory), uiDirectoryCluster, &uiDirectoryCluster)
		lda [D + WE_FIRST_DIRECTORY]
		sta [D + F_CLUSTER]
		lda [D + WE_FIRST_DIRECTORY + 2]
		sta [D + F_CLUSTER + 2]
RewindPreviousCluster:
		lda [D + F_CLUSTER]
		ldx [D + F_CLUSTER + 2]
		jsr IsEndOfChain
		bcc RewindPreviousNext
		lda FAT_CORRUPTED_FILE
		rts
RewindPreviousNext:
		sta [D + WE_LAST_DIRECTORY]
		stx [D + WE_LAST_DIRECTORY + 2]
		jsr NextCluster
		bcc RewindPreviousRead
		rts
RewindPreviousRead:
		lda [D + F_CLUSTER]
		cmp [D + WE_DIRECTORY]
		bne RewindPreviousCluster
		lda [D + F_CLUSTER + 2]
		cmp [D + WE_DIRECTORY + 2]
		bne RewindPreviousCluster
		lda [D + WE_LAST_DIRECTORY]
		sta [D + WE_DIRECTORY]
		sta [D + F_CLUSTER]
		lda [D + WE_LAST_DIRECTORY + 2]
		sta [D + WE_DIRECTORY + 2]
		sta [D + F_CLUSTER + 2]
		jsr FirstSector
		lda [D + F_SECTOR]
		sta [D + WE_FIRST]
		lda [D + F_SECTOR + 2]
		sta [D + WE_FIRST + 2]
		ldy CFatVolume_muiNoOfSectorsPerCluster
		lda [[D + F_THIS]:24 + Y]
		dec A
		clc
		adc [D + WE_FIRST]
		sta [D + WE_SECTOR]
		lda [D + WE_FIRST + 2]
		adc 0
		sta [D + WE_SECTOR + 2]
RewindLastEntry:
		jsr ReadWriteSector
		bcc RewindLastRead
		rts
RewindLastRead:
		lda [D + WE_LAST_ENTRY]
		sta [D + WE_PARENT_ENTRY]
		lda [D + WE_LAST_ENTRY + 2]
		sta [D + WE_PARENT_ENTRY + 2]
RewindCount:
		dec [D + WE_FOUND]
		jmp RewindNext


; InitialiseParentEntry(psParentEntry, szName, uiChecksum, iLFNEntriesNeeded, iLFNEntriesFound): a long name entry
; with characters (iLFNEntriesFound - 1) * 13 on (0x0000 at the name's end, then FAT_LONG_NAME_END_WCHAR).
InitialiseParentEntry:
		.width (A, X), 16
		lda [D + WE_PARENT_ENTRY]
		sta [D + F_POINTER]
		lda [D + WE_PARENT_ENTRY + 2]
		sta [D + F_POINTER + 2]
		lda [D + WE_CHECKSUM]
		sep m
		ldy LFN_uiChecksum
		sta [[D + F_POINTER]:24 + Y]
		lda FAT_ATTR_LONG_NAME
		ldy LFN_uiAttributes
		sta [[D + F_POINTER]:24 + Y]
		lda 0
		ldy LFN_uiType
		sta [[D + F_POINTER]:24 + Y]
		rep m
		lda 0
		ldy LFN_uiFirstCluster
		sta [[D + F_POINTER]:24 + Y]
		; The first entry (the name's last part) has FAT_FIRST_LFN_ENTRY.
		ldx 0
		lda [D + WE_NEEDED]
		dec A
		cmp [D + WE_FOUND]
		bne InitialiseParentSequence
		ldx FAT_FIRST_LFN_ENTRY
InitialiseParentSequence:
		txa
		ora [D + WE_FOUND]
		sep m
		ldy LFN_uiSequence
		sta [[D + F_POINTER]:24 + Y]
		rep m
		; The characters: (iLFNEntriesFound - 1) * 13 into the name.
		lda [D + WE_FOUND]
		dec A
		jsr TimesThirteen
		sta [D + F_WORK2]
		ldx 0
InitialiseParentCharacter:
		; X is the character (0 to 12): its place in the entry from the table.
		txa
		clc
		adc [D + F_WORK2]
		cmp [D + WE_LENGTH]
		beq InitialiseParentNameEnd
		bcs InitialiseParentPast
		tay
		lda [[D + WE_NAME]:24 + Y]
		and 0xFF
		bra InitialiseParentStore
InitialiseParentNameEnd:
		lda 0
		bra InitialiseParentStore
InitialiseParentPast:
		lda FAT_LONG_NAME_END_WCHAR
InitialiseParentStore:
		pha
		lda [LongNameCharacterPlaces + X]:24
		and 0xFF
		tay
		pla
		sta [[D + F_POINTER]:24 + Y]
		inx
		cpx 13
		bne InitialiseParentCharacter
		rts
LongNameCharacterPlaces:
		.byte 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30


; ------------------------------------------------------------------------------------------------------------------
; Directories, deleting and renaming
; ------------------------------------------------------------------------------------------------------------------

; EFatCode CFatVolume::CreateDirectory(const char* szDirectory)
CD_PATH			= F_ARGUMENT
CD_ENTRY		= F_LOCALS			; SFatDirectoryEntry (below the frame).
		.global _CFatVolume__CreateDirectory
_CFatVolume__CreateDirectory:
		.width (A, X), 16
		jsr Enter
		tsc
		sec
		sbc SIZEOF_SFatDirectoryEntry
		tcs
		inc A
		sta [D + CD_ENTRY]
		stz [D + CD_ENTRY + 2]
		; StrEmpty(szDirectory) || strlen(szDirectory) > FAT_MAX_PATH
		lda [D + CD_PATH]
		ora [D + CD_PATH + 2]
		beq CreateDirectoryInvalid
		pei [D + CD_PATH + 2]
		pei [D + CD_PATH]
		jsl _strlen
		ply
		ply
		cmp 0
		beq CreateDirectoryInvalid
		txy
		bne CreateDirectoryInvalid
		cmp FAT_MAX_PATH + 1
		bcc CreateDirectoryLength
CreateDirectoryInvalid:
		lda FAT_INVALID_FILENAME
		jmp Leave
CreateDirectoryLength:
		pei [D + CD_ENTRY + 2]
		pei [D + CD_ENTRY]
		pei [D + CD_PATH + 2]
		pei [D + CD_PATH]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl _CFatVolume__GetFileEntry
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		cmp FAT_SUCCESS
		beq CreateDirectoryLooked
		jmp Leave
CreateDirectoryLooked:
		lda [[D + CD_ENTRY]:24]
		and 0xFF
		beq CreateDirectoryNew
		lda FAT_FILENAME_ALREADY_EXISTS
		jmp Leave
CreateDirectoryNew:
		; CreateDirectory(szDirectory, &sEntry): the parent path, then the directory in it.
		jsr CreateDirectoryIn
		jmp Leave


; CreateDirectory(CD_PATH, CD_ENTRY): the parent's entry (the path up to the last \), then a directory entry in it.
; A = the result.
CD_PARENT		= F_LOCALS + 4		; The parent's SFatDirectoryEntry,
CD_PARENT_PATH	= F_LOCALS + 8		; and its path (FAT_MAX_PATH + 1), below the frame.
CD_NAME			= F_LOCALS + 12
CreateDirectoryIn:
		.width (A, X), 16
		ply
		tsc
		sec
		sbc SIZEOF_SFatDirectoryEntry + FAT_MAX_PATH + 2
		tcs
		inc A
		sta [D + CD_PARENT]
		clc
		adc SIZEOF_SFatDirectoryEntry
		sta [D + CD_PARENT_PATH]
		stz [D + CD_PARENT + 2]
		stz [D + CD_PARENT_PATH + 2]
		phy
		; A path ending in \ isn't a directory's.
		pei [D + CD_PATH + 2]
		pei [D + CD_PATH]
		jsl _StrLen__FPc
		ply
		ply
		dec A
		clc
		adc [D + CD_PATH]
		sta [D + CD_NAME]
		lda [D + CD_PATH + 2]
		adc 0
		sta [D + CD_NAME + 2]
		lda [[D + CD_NAME]:24]
		and 0xFF
		cmp '\\'
		bne CreateDirectoryInName
		lda FAT_INVALID_FILENAME
		jmp CreateDirectoryInDone
CreateDirectoryInName:
		pei [D + CD_NAME + 2]
		pei [D + CD_NAME]
		jsl _StepPathToSlash__FPc
		ply
		ply
		sta [D + CD_NAME]
		stx [D + CD_NAME + 2]
		; memcpy(szFilePath, szDirectory, uiPathLength); szFilePath[uiPathLength] = '\0'
		sec
		sbc [D + CD_PATH]
		sta [D + F_WORK]
		pea 0
		pha
		pei [D + CD_PATH + 2]
		pei [D + CD_PATH]
		pei [D + CD_PARENT_PATH + 2]
		pei [D + CD_PARENT_PATH]
		jsl _memcpy
		tsc
		clc
		adc 12
		tcs
		ldy [D + F_WORK]
		lda 0
		sep m
		sta [[D + CD_PARENT_PATH]:24 + Y]
		rep m
		inc [D + CD_NAME]
		bne CreateDirectoryInParent
		inc [D + CD_NAME + 2]
CreateDirectoryInParent:
		pei [D + CD_PARENT + 2]
		pei [D + CD_PARENT]
		pei [D + CD_PARENT_PATH + 2]
		pei [D + CD_PARENT_PATH]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl _CFatVolume__GetFileEntry
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		cmp FAT_SUCCESS
		bne CreateDirectoryInDone
		lda [[D + CD_PARENT]:24]
		and 0xFF
		bne CreateDirectoryInCreate
		lda FAT_DIRECTORY_DOES_NOT_EXIST
		bra CreateDirectoryInDone
CreateDirectoryInCreate:
		; CreateFATEntry(&sParentEntry.sRaw, szPathScanner, FAT_ATTR_DIRECTORY, 0, psEntry)
		pei [D + CD_ENTRY + 2]
		pei [D + CD_ENTRY]
		pea 0
		pea 0
		pea FAT_ATTR_DIRECTORY
		pei [D + CD_NAME + 2]
		pei [D + CD_NAME]
		lda [D + CD_PARENT]
		clc
		adc SFatDirectoryEntry_sRaw
		pea 0
		pha
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl _CFatVolume__CreateFATEntry
		tay
		tsc
		clc
		adc 22
		tcs
		tya
CreateDirectoryInDone:
		ply
		tax
		tsc
		clc
		adc SIZEOF_SFatDirectoryEntry + FAT_MAX_PATH + 2
		tcs
		phy
		txa
		rts


; EFatCode CFatVolume::Delete(const char* szFilename)
DE_PATH			= F_ARGUMENT
DE_ENTRY		= F_LOCALS			; SFatDirectoryEntry (below the frame)
DE_PATH_PART	= F_LOCALS + 4		; char szPathPart[FAT_MAX_PATH] (below the frame)
DE_NAME_PART	= F_LOCALS + 8		; const char* szNamePart
DE_CHECKSUM		= F_LOCALS + 12
		.global _CFatVolume__Delete
_CFatVolume__Delete:
		.width (A, X), 16
		jsr Enter
		tsc
		sec
		sbc SIZEOF_SFatDirectoryEntry + FAT_MAX_PATH
		tcs
		inc A
		sta [D + DE_ENTRY]
		clc
		adc SIZEOF_SFatDirectoryEntry
		sta [D + DE_PATH_PART]
		stz [D + DE_ENTRY + 2]
		stz [D + DE_PATH_PART + 2]
		pei [D + DE_ENTRY + 2]
		pei [D + DE_ENTRY]
		pei [D + DE_PATH + 2]
		pei [D + DE_PATH]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl _CFatVolume__GetFileEntry
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		cmp FAT_SUCCESS
		beq DeleteLooked
		jmp Leave
DeleteLooked:
		lda [[D + DE_ENTRY]:24]
		and 0xFF
		bne DeleteFound
		lda FAT_FILE_NOT_FOUND
		jmp Leave
DeleteFound:
		ldx DE_ENTRY
		jsr EntryChecksum
		sta [D + DE_CHECKSUM]
		ldy SFatDirectoryEntry_uiAttributes
		lda [[D + DE_ENTRY]:24 + Y]
		and FAT_ATTR_DIRECTORY
		beq DeleteFile
		lda FAT_NOT_A_FILE
		jmp Leave
DeleteFile:
		lda [D + DE_ENTRY]
		clc
		adc SFatDirectoryEntry_sRaw
		sta [D + F_WORK]
		lda [D + DE_ENTRY + 2]
		adc 0
		sta [D + F_WORK + 2]
		ldx F_WORK
		jsr EntryFirstCluster
		lda [D + F_CLUSTER]
		ora [D + F_CLUSTER + 2]
		beq DeleteNoClusters
		jsr FreeChain
		cmp FAT_SUCCESS
		beq DeleteNoClusters
		jmp Leave
DeleteNoClusters:
		; ParsePathAndFilename(szFilename, szPathPart, &szNamePart), then the long name's entries, then the short's.
		pea 0
		tdc
		clc
		adc DE_NAME_PART
		pha
		pei [D + DE_PATH_PART + 2]
		pei [D + DE_PATH_PART]
		pei [D + DE_PATH + 2]
		pei [D + DE_PATH]
		jsl _ParsePathAndFilename__FPcPcPPc
		tsc
		clc
		adc 12
		tcs
		lda [D + DE_ENTRY]
		sta [D + F_POINTER]
		lda [D + DE_ENTRY + 2]
		sta [D + F_POINTER + 2]
		ldx DE_PATH_PART
		lda [D + DE_CHECKSUM]
		jsr DeleteLongName
		cmp FAT_SUCCESS
		beq DeleteShortName
		jmp Leave
DeleteShortName:
		lda [D + DE_ENTRY]
		sta [D + F_POINTER]
		lda [D + DE_ENTRY + 2]
		sta [D + F_POINTER + 2]
		jsr DeleteEntry
		jmp Leave


; Marks the directory entry at F_POINTER deleted and writes it back.  A = the result.
DeleteEntry:
		.width (A, X), 16
		lda FAT_DELETED_ENTRY
		sep m
		ldy SFatDirectoryEntry_sRaw + RAW_szShortName
		sta [[D + F_POINTER]:24 + Y]
		rep m
		jsr WriteEntryBack
		rts


; The checksum of the short name of the directory entry at D + X (FatLongEntryChecksum), in A.
EntryChecksum:
		.width (A, X), 16
		lda [D + 0 + X]
		clc
		adc SFatDirectoryEntry_sRaw + RAW_szShortName
		sta [D + F_POINTER]
		lda [D + 2 + X]
		adc 0
		sta [D + F_POINTER + 2]
		jmp Checksum


; DeleteLongNameEntries(the path at D + X, the entry at F_POINTER's uiSectorAddress and uiSectorOffset, the checksum in
; A): A = the result.
DeleteLongName:
		.width (A, X), 16
		pha
		ldy SFatDirectoryEntry_uiSectorOffset
		lda [[D + F_POINTER]:24 + Y]
		pha
		ldy SFatDirectoryEntry_uiSectorAddress + 2
		lda [[D + F_POINTER]:24 + Y]
		pha
		ldy SFatDirectoryEntry_uiSectorAddress
		lda [[D + F_POINTER]:24 + Y]
		pha
		lda [D + 2 + X]
		pha
		lda [D + 0 + X]
		pha
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl DeleteLongNameEntries
		tay
		tsc
		clc
		adc 16
		tcs
		tya
		rts


; EFatCode DeleteLongNameEntries(const char* szParentPath, uint32 uiSectorAddress, uint16 uiSectorOffset, uint8 uiChecksum):
; marks deleted the long name entries just before the short name entry at uiSectorAddress and uiSectorOffset in
; szParentPath (with its checksum).  The short name entry must not be deleted yet (queries skip deleted ones).
DL_PATH			= F_ARGUMENT
DL_SECTOR		= F_ARGUMENT + 4
DL_OFFSET		= F_ARGUMENT + 8
DL_CHECKSUM		= F_ARGUMENT + 10
DL_QUERY		= F_LOCALS			; SFatFileSystemQuery (below the frame)
DL_SECTORS		= F_LOCALS + 4		; uint32 auiSectors[FAT_MAX_LFN_ENTRIES] (below the frame)
DL_OFFSETS		= F_LOCALS + 8		; uint16 auiOffsets[FAT_MAX_LFN_ENTRIES] (below the frame)
DL_RUN			= F_LOCALS + 12
DL_RESULT		= F_LOCALS + 14
DL_INDEX		= F_LOCALS + 16
DL_BELOW		= SIZEOF_SFatFileSystemQuery + FAT_MAX_LFN_ENTRIES * 6
DeleteLongNameEntries:
		.width (A, X), 16
		jsr Enter
		tsc
		sec
		sbc DL_BELOW
		tcs
		inc A
		sta [D + DL_QUERY]
		clc
		adc SIZEOF_SFatFileSystemQuery
		sta [D + DL_SECTORS]
		clc
		adc FAT_MAX_LFN_ENTRIES * 4
		sta [D + DL_OFFSETS]
		stz [D + DL_QUERY + 2]
		stz [D + DL_SECTORS + 2]
		stz [D + DL_OFFSETS + 2]
		stz [D + DL_RUN]
		pei [D + DL_QUERY + 2]
		pei [D + DL_QUERY]
		jsl _SFatFileSystemQuery__Init
		ply
		ply
		; FindFirstFATEntry(szParentPath, FAT_ATTR_LONG_NAME, 0, &sQuery)
		pei [D + DL_QUERY + 2]
		pei [D + DL_QUERY]
		pea 0
		pea 0
		pea FAT_ATTR_LONG_NAME
		pei [D + DL_PATH + 2]
		pei [D + DL_PATH]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl _CFatVolume__FindFirstFATEntry
		tay
		tsc
		clc
		adc 18
		tcs
		tya
DeleteLongNext:
		sta [D + DL_RESULT]
		cmp FAT_SUCCESS
		beq DeleteLongEntry
		jmp DeleteLongDone
DeleteLongEntry:
		; The query's sCurrentEntry (at the start of the SFatFileSystemQuery).
		lda [D + DL_QUERY]
		sta [D + F_POINTER]
		lda [D + DL_QUERY + 2]
		sta [D + F_POINTER + 2]
		ldy SFatDirectoryEntry_sRaw + RAW_szShortName
		lda [[D + F_POINTER]:24 + Y]
		and 0xFF
		bne DeleteLongLook
		jmp DeleteLongDone
DeleteLongLook:
		ldy SFatDirectoryEntry_sRaw + RAW_uiAttributes
		lda [[D + F_POINTER]:24 + Y]
		and 0xFF
		cmp FAT_ATTR_LONG_NAME
		bne DeleteLongShort
		; A long name part: one of the run if it has the checksum (and the run isn't full).
		ldy SFatDirectoryEntry_sRaw + LFN_uiChecksum
		lda [[D + F_POINTER]:24 + Y]
		eor [D + DL_CHECKSUM]
		and 0xFF
		bne DeleteLongRunBroken
		lda [D + DL_RUN]
		cmp FAT_MAX_LFN_ENTRIES
		beq DeleteLongRunBroken
		asl A
		asl A
		tay
		phy
		ldy SFatDirectoryEntry_uiSectorAddress
		lda [[D + F_POINTER]:24 + Y]
		ply
		sta [[D + DL_SECTORS]:24 + Y]
		phy
		ldy SFatDirectoryEntry_uiSectorAddress + 2
		lda [[D + F_POINTER]:24 + Y]
		ply
		iny
		iny
		sta [[D + DL_SECTORS]:24 + Y]
		lda [D + DL_RUN]
		asl A
		tay
		phy
		ldy SFatDirectoryEntry_uiSectorOffset
		lda [[D + F_POINTER]:24 + Y]
		ply
		sta [[D + DL_OFFSETS]:24 + Y]
		inc [D + DL_RUN]
		bra DeleteLongFindNext
DeleteLongRunBroken:
		stz [D + DL_RUN]
		bra DeleteLongFindNext
DeleteLongShort:
		; The short name entry: if it is the one, its run is deleted.
		ldy SFatDirectoryEntry_uiSectorAddress
		lda [[D + F_POINTER]:24 + Y]
		cmp [D + DL_SECTOR]
		bne DeleteLongRunBroken
		ldy SFatDirectoryEntry_uiSectorAddress + 2
		lda [[D + F_POINTER]:24 + Y]
		cmp [D + DL_SECTOR + 2]
		bne DeleteLongRunBroken
		ldy SFatDirectoryEntry_uiSectorOffset
		lda [[D + F_POINTER]:24 + Y]
		cmp [D + DL_OFFSET]
		bne DeleteLongRunBroken
		stz [D + DL_INDEX]
DeleteLongMark:
		lda [D + DL_INDEX]
		cmp [D + DL_RUN]
		bcs DeleteLongDone
		asl A
		asl A
		tay
		lda [[D + DL_SECTORS]:24 + Y]
		sta [D + F_SECTOR]
		iny
		iny
		lda [[D + DL_SECTORS]:24 + Y]
		sta [D + F_SECTOR + 2]
		jsr ReadCache
		bcc DeleteLongMarkRead
		sta [D + DL_RESULT]
		bra DeleteLongDone
DeleteLongMarkRead:
		lda [D + DL_INDEX]
		asl A
		tay
		lda [[D + DL_OFFSETS]:24 + Y]
		tay
		lda FAT_DELETED_ENTRY
		sep m
		sta [[D + F_CACHE]:24 + Y]
		rep m
		jsr DirtyCache
		inc [D + DL_INDEX]
		bra DeleteLongMark
DeleteLongFindNext:
		pei [D + DL_QUERY + 2]
		pei [D + DL_QUERY]
		pea 0
		pea 0
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl _CFatVolume__FindNextFATEntry
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		jmp DeleteLongNext
DeleteLongDone:
		jsr SectorCache
		pei [D + F_WORK2 + 2]
		pei [D + F_WORK2]
		pei [D + DL_QUERY + 2]
		pei [D + DL_QUERY]
		jsl _SFatFileSystemQuery__Kill
		tsc
		clc
		adc 8
		tcs
		lda [D + DL_RESULT]
		jmp Leave


; EFatCode CFatVolume::Rename(const char* szOriginalFilename, const char* szNewFilename)
RN_ORIGINAL		= F_ARGUMENT
RN_NEW			= F_ARGUMENT + 4
RN_NEW_PARENT	= F_LOCALS			; char szNewParent[256]
RN_ORIGINAL_PART	= F_LOCALS + 4	; const char* szOriginalFilenamePart
RN_ORIGINAL_ENTRY	= F_LOCALS + 8	; SFatDirectoryEntry sOriginalEntry
RN_NEW_ENTRY	= F_LOCALS + 12		; SFatDirectoryEntry sNewEntry
RN_ORIGINAL_PARENT	= F_LOCALS + 16	; char szOriginalParent[256]
RN_NEW_PART		= F_LOCALS + 20		; const char* szNewFilenamePart
RN_PARENT		= F_LOCALS + 24		; SFatDirectoryEntry parent
RN_CHECKSUM		= F_LOCALS + 28
RN_CLUSTER		= F_LOCALS + 30
RN_BELOW		= 256 + 256 + SIZEOF_SFatDirectoryEntry * 3
		.global _CFatVolume__Rename
_CFatVolume__Rename:
		.width (A, X), 16
		jsr Enter
		tsc
		sec
		sbc RN_BELOW
		tcs
		inc A
		sta [D + RN_NEW_PARENT]
		clc
		adc 256
		sta [D + RN_ORIGINAL_PARENT]
		clc
		adc 256
		sta [D + RN_ORIGINAL_ENTRY]
		clc
		adc SIZEOF_SFatDirectoryEntry
		sta [D + RN_NEW_ENTRY]
		clc
		adc SIZEOF_SFatDirectoryEntry
		sta [D + RN_PARENT]
		stz [D + RN_NEW_PARENT + 2]
		stz [D + RN_ORIGINAL_PARENT + 2]
		stz [D + RN_ORIGINAL_ENTRY + 2]
		stz [D + RN_NEW_ENTRY + 2]
		stz [D + RN_PARENT + 2]
		ldx RN_ORIGINAL
		ldy RN_ORIGINAL_PARENT
		lda RN_ORIGINAL_PART
		jsr ParsePath
		ldx RN_NEW
		ldy RN_NEW_PARENT
		lda RN_NEW_PART
		jsr ParsePath
		; The new name mustn't be there (GetFileEntry's results are ignored, as the C++).
		ldx RN_NEW
		ldy RN_NEW_ENTRY
		jsr GetEntryOf
		lda [[D + RN_NEW_ENTRY]:24]
		and 0xFF
		beq RenameNewFree
		lda FAT_FILENAME_ALREADY_EXISTS
		jmp Leave
RenameNewFree:
		ldx RN_ORIGINAL
		ldy RN_ORIGINAL_ENTRY
		jsr GetEntryOf
		lda [[D + RN_ORIGINAL_ENTRY]:24]
		and 0xFF
		bne RenameFound
		lda FAT_FILE_NOT_FOUND
		jmp Leave
RenameFound:
		ldx RN_ORIGINAL_ENTRY
		jsr EntryChecksum
		sta [D + RN_CHECKSUM]
		lda [D + RN_ORIGINAL_ENTRY]
		clc
		adc SFatDirectoryEntry_sRaw
		sta [D + F_WORK]
		lda [D + RN_ORIGINAL_ENTRY + 2]
		adc 0
		sta [D + F_WORK + 2]
		ldx F_WORK
		jsr EntryFirstCluster
		lda [D + F_CLUSTER]
		sta [D + RN_CLUSTER]
		lda [D + F_CLUSTER + 2]
		sta [D + RN_CLUSTER + 2]
		ldx RN_NEW_PARENT
		ldy RN_PARENT
		jsr GetEntryOf
		cmp FAT_SUCCESS
		beq RenameParentFound
		jmp Leave
RenameParentFound:
		; CreateFATEntry(&parent.sRaw, szNewFilenamePart, sOriginalEntry.uiAttributes, uiEntryCluster, &sNewEntry)
		pei [D + RN_NEW_ENTRY + 2]
		pei [D + RN_NEW_ENTRY]
		pei [D + RN_CLUSTER + 2]
		pei [D + RN_CLUSTER]
		ldy SFatDirectoryEntry_uiAttributes
		lda [[D + RN_ORIGINAL_ENTRY]:24 + Y]
		and 0xFF
		pha
		pei [D + RN_NEW_PART + 2]
		pei [D + RN_NEW_PART]
		lda [D + RN_PARENT]
		clc
		adc SFatDirectoryEntry_sRaw
		pea 0
		pha
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl _CFatVolume__CreateFATEntry
		tay
		tsc
		clc
		adc 22
		tcs
		tya
		cmp FAT_SUCCESS
		beq RenameCreated
		jmp Leave
RenameCreated:
		; CopyRawEntry(&sOriginalEntry, &sNewEntry): all of the raw entry but its name.
		lda [D + RN_ORIGINAL_ENTRY]
		clc
		adc SFatDirectoryEntry_sRaw
		sta [D + F_WORK]
		lda [D + RN_ORIGINAL_ENTRY + 2]
		adc 0
		sta [D + F_WORK + 2]
		lda [D + RN_NEW_ENTRY]
		clc
		adc SFatDirectoryEntry_sRaw
		sta [D + F_POINTER]
		lda [D + RN_NEW_ENTRY + 2]
		adc 0
		sta [D + F_POINTER + 2]
		ldy RAW_uiAttributes
		lda [[D + F_WORK]:24 + Y]
		sep m
		sta [[D + F_POINTER]:24 + Y]
		rep m
		ldy SIZEOF_SFatRawDirectoryEntry - 2
RenameCopyRaw:
		lda [[D + F_WORK]:24 + Y]
		sta [[D + F_POINTER]:24 + Y]
		dey
		dey
		cpy RAW_uiReserved
		bne RenameCopyRaw
		lda [[D + F_WORK]:24 + Y]
		sta [[D + F_POINTER]:24 + Y]
		lda [D + RN_NEW_ENTRY]
		sta [D + F_POINTER]
		lda [D + RN_NEW_ENTRY + 2]
		sta [D + F_POINTER + 2]
		jsr WriteEntryBack
		bcc RenameNewWritten
		jmp Leave
RenameNewWritten:
		lda [D + RN_ORIGINAL_ENTRY]
		sta [D + F_POINTER]
		lda [D + RN_ORIGINAL_ENTRY + 2]
		sta [D + F_POINTER + 2]
		ldx RN_ORIGINAL_PARENT
		lda [D + RN_CHECKSUM]
		jsr DeleteLongName
		cmp FAT_SUCCESS
		beq RenameDeleteOriginal
		jmp Leave
RenameDeleteOriginal:
		lda [D + RN_ORIGINAL_ENTRY]
		sta [D + F_POINTER]
		lda [D + RN_ORIGINAL_ENTRY + 2]
		sta [D + F_POINTER + 2]
		jsr DeleteEntry
		bcs RenameDone
		lda FAT_SUCCESS
RenameDone:
		jmp Leave


; ParsePathAndFilename(the path at D + X, the buffer pointed to by D + Y, &the D + A pointer)
ParsePath:
		.width (A, X), 16
		sta [D + F_WORK2]
		stx [D + F_WORK2 + 2]
		pea 0
		tdc
		clc
		adc [D + F_WORK2]
		pha
		tyx
		lda [D + 2 + X]
		pha
		lda [D + 0 + X]
		pha
		ldx [D + F_WORK2 + 2]
		lda [D + 2 + X]
		pha
		lda [D + 0 + X]
		pha
		jsl _ParsePathAndFilename__FPcPcPPc
		tsc
		clc
		adc 12
		tcs
		rts


; GetFileEntry(the path at D + X, the entry pointed to by D + Y): A = the result.
GetEntryOf:
		.width (A, X), 16
		stx [D + F_WORK2]
		tyx
		lda [D + 2 + X]
		pha
		lda [D + 0 + X]
		pha
		ldx [D + F_WORK2]
		lda [D + 2 + X]
		pha
		lda [D + 0 + X]
		pha
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl _CFatVolume__GetFileEntry
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		rts
