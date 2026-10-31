; FatClusterCache.cpp for the W65C816, with the same names and the compiler's calling convention (see
; FatInfoSectorCache816.asm).  Every function here has the same frame (F_FRAME bytes), so the helpers (jsr, within
; this one section) find this, the cluster being worked on and their scratch in the same places.
;
; A sector's bit in pbCachedSectors and pbDirtySectors is bit (i & 7) of byte i >> 3, as IntegerHelper's GetBit and
; SetBit have it.

		.include "Fat816.inc"

NOT_FOUND		= 0xFFFF								; ARRAY_ELEMENT_NOT_FOUND: size is 16 bits on the W65C816.
VTABLE_READ		= 4										; CFileDrive's virtual functions: Kill, Read, Read (sectors),
VTABLE_READS	= 8										; Write, Write (sectors), Erase, GetPageSize.
VTABLE_WRITES	= 16
CACHED			= SClusterCache_pbCachedSectors
DIRTY			= SClusterCache_pbDirtySectors

F_DRIVE			= 1				; The drive, then its virtual function's address - 1 (CallDrive).
F_FUNCTION		= 5
F_CLUSTER		= 9				; The SClusterCache worked on.
F_POINTER		= 13			; Scratch.
F_LENGTH		= 17			; 4 bytes.
F_FIRST			= 21
F_LAST			= 23
F_MAXIMUM		= 25
F_START			= 27
F_END			= 29
F_INDEX			= 31
F_OFFSET		= 33
F_SECTOR_LENGTH	= 35
F_BIT			= 37			; FindBit: the bit wanted (0 or 1).
F_FRAME			= 38
F_THIS			= F_FRAME + 6
F_ARGUMENT1		= F_FRAME + 10
F_ARGUMENT2		= F_FRAME + 14
F_ARGUMENT3		= F_FRAME + 18
F_ARGUMENT4		= F_FRAME + 22	; A uint16, then
F_ARGUMENT5		= F_FRAME + 24	; a pointer, then
F_ARGUMENT6		= F_FRAME + 28	; a uint16 (Read and Write).


		.section text.FatClusterCache816


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


; F_CLUSTER = this->msCluster.
ThisCluster:
		.width (A, X), 16
		lda [D + F_THIS]
		clc
		adc CFatClusterCache_msCluster
		sta [D + F_CLUSTER]
		lda [D + F_THIS + 2]
		adc 0
		sta [D + F_CLUSTER + 2]
		rts


; F_CLUSTER = the SClusterCache* argument (F_ARGUMENT1).
ArgumentCluster:
		.width (A, X), 16
		lda [D + F_ARGUMENT1]
		sta [D + F_CLUSTER]
		lda [D + F_ARGUMENT1 + 2]
		sta [D + F_CLUSTER + 2]
		rts


; The byte holding bit X of the F_CLUSTER bit array at offset Y (CACHED or DIRTY) into F_POINTER, its mask in A.
BitPointer:
		.width (A, X), 16
		lda [[D + F_CLUSTER]:24 + Y]
		sta [D + F_POINTER]
		iny
		iny
		lda [[D + F_CLUSTER]:24 + Y]
		sta [D + F_POINTER + 2]
		txa
		lsr A
		lsr A
		lsr A
		clc
		adc [D + F_POINTER]
		sta [D + F_POINTER]
		bcc BitPointerMask
		inc [D + F_POINTER + 2]
BitPointerMask:
		txa
		and 7
		tax
		lda 1
BitPointerShift:
		dex
		bmi BitPointerReturn
		asl A
		bra BitPointerShift
BitPointerReturn:
		rts


; Z clear (A not 0) if bit X of the bit array at offset Y is set.
GetBit:
		.width (A, X), 16
		jsr BitPointer
		sep m
		and [[D + F_POINTER]:24]
		rep m
		and 0xFF
		rts


; Sets bit X of the bit array at offset Y.
SetBit:
		.width (A, X), 16
		jsr BitPointer
		sep m
		ora [[D + F_POINTER]:24]
		sta [[D + F_POINTER]:24]
		rep m
		rts


; Clears bit X of the bit array at offset Y.
ClearBit:
		.width (A, X), 16
		jsr BitPointer
		eor 0xFFFF
		sep m
		and [[D + F_POINTER]:24]
		sta [[D + F_POINTER]:24]
		rep m
		rts


; The first bit from X (to muiSectorsPerCluster) of the bit array at offset Y that is F_BIT (0 or 1): its index in A,
; or NOT_FOUND.  (FindNextSetBit, FindNextClearBit and FindFirstSetBit.)
FindBit:
		.width (A, X), 16
		stx [D + F_INDEX]
		sty [D + F_OFFSET]
FindBitNext:
		ldy CFatClusterCache_muiSectorsPerCluster
		lda [D + F_INDEX]
		cmp [[D + F_THIS]:24 + Y]
		bcs FindBitNone
		tax
		ldy [D + F_OFFSET]
		jsr GetBit
		beq FindBitClear
		lda 1
FindBitClear:
		cmp [D + F_BIT]
		beq FindBitFound
		inc [D + F_INDEX]
		bra FindBitNext
FindBitNone:
		lda NOT_FOUND
		rts
FindBitFound:
		lda [D + F_INDEX]
		rts


; &F_CLUSTER->pvCache[A << muiSectorSizeShift] into F_POINTER.
SectorPointer:
		.width (A, X), 16
		pha
		ldy CFatClusterCache_muiSectorSizeShift
		lda [[D + F_THIS]:24 + Y]
		tax
		pla
		; A 16 bit offset: a cluster is at most 64KB (muiClusterSize is 16 bits).
SectorPointerShift:
		dex
		bmi SectorPointerAdd
		asl A
		bra SectorPointerShift
SectorPointerAdd:
		; &pvCache[A]
OffsetPointer:
		ldy SClusterCache_pvCache
		clc
		adc [[D + F_CLUSTER]:24 + Y]
		sta [D + F_POINTER]
		ldy SClusterCache_pvCache + 2
		lda [[D + F_CLUSTER]:24 + Y]
		adc 0
		sta [D + F_POINTER + 2]
		rts


; The drive (this->mpcDrive) into F_DRIVE and the address - 1 of its virtual function at offset Y in its vtable
; into F_FUNCTION.
DriveFunction:
		.width (A, X), 16
		phy
		ldy CFatClusterCache_mpcDrive
		lda [[D + F_THIS]:24 + Y]
		sta [D + F_DRIVE]
		ldy CFatClusterCache_mpcDrive + 2
		lda [[D + F_THIS]:24 + Y]
		sta [D + F_DRIVE + 2]
		lda [[D + F_DRIVE]:24]
		sta [D + F_FUNCTION]
		ldy 2
		lda [[D + F_DRIVE]:24 + Y]
		sta [D + F_FUNCTION + 2]
		ply
		lda [[D + F_FUNCTION]:24 + Y]
		tax
		iny
		iny
		lda [[D + F_FUNCTION]:24 + Y]
		sta [D + F_FUNCTION + 2]
		dex
		stx [D + F_FUNCTION]
		rts


; Goes to F_FUNCTION as rtl does: the caller pushed the arguments, the drive and its return address (phk, per), and
; jsr'd here; this jsr's return address is dropped.
CallFunction:
		.width (A, X), 16
		ply
		sep m
		lda [D + F_FUNCTION + 2]
		pha
		rep m
		pei [D + F_FUNCTION]
		rtl


; Pushes F_CLUSTER->uiClusterFirstSector + A as a uint64 (the return address moved over it).
PushSector:
		.width (A, X), 16
		ply
		sty [D + F_INDEX]
		pea 0
		pea 0
		ldy SClusterCache_uiClusterFirstSector
		clc
		adc [[D + F_CLUSTER]:24 + Y]
		tax
		ldy SClusterCache_uiClusterFirstSector + 2
		lda [[D + F_CLUSTER]:24 + Y]
		adc 0
		pha
		phx
		lda [D + F_INDEX]
		pha
		rts


; void CFatClusterCache::Init(CFileDrive* pcDrive, uint16 uiClusterSize, uint16 uiSectorSize)
		.global _CFatClusterCache__Init
_CFatClusterCache__Init:
		jsr Enter
		lda [D + F_ARGUMENT2]
		ldy CFatClusterCache_muiClusterSize
		sta [[D + F_THIS]:24 + Y]
		lda [D + F_ARGUMENT2 + 2]
		ldy CFatClusterCache_muiSectorSize
		sta [[D + F_THIS]:24 + Y]
		; Sector sizes are powers of two: muiSectorsPerCluster = muiClusterSize >> muiSectorSizeShift.
		ldx 0
		lda 1
InitShift:
		cmp [D + F_ARGUMENT2 + 2]
		bcs InitShifted
		asl A
		inx
		bra InitShift
InitShifted:
		txa
		ldy CFatClusterCache_muiSectorSizeShift
		sta [[D + F_THIS]:24 + Y]
		lda [D + F_ARGUMENT2]
InitDivide:
		dex
		bmi InitDivided
		lsr A
		bra InitDivide
InitDivided:
		ldy CFatClusterCache_muiSectorsPerCluster
		sta [[D + F_THIS]:24 + Y]
		ldy CFatClusterCache_muiBytesForBits
		sta [[D + F_THIS]:24 + Y]
		lda [D + F_ARGUMENT1]
		ldy CFatClusterCache_mpcDrive
		sta [[D + F_THIS]:24 + Y]
		lda [D + F_ARGUMENT1 + 2]
		ldy CFatClusterCache_mpcDrive + 2
		sta [[D + F_THIS]:24 + Y]
		jsr ThisCluster
		lda [D + F_ARGUMENT2]
		ldy SClusterCache_pvCache
		jsr AllocateZeroed
		ldy CFatClusterCache_muiBytesForBits
		lda [[D + F_THIS]:24 + Y]
		ldy CACHED
		jsr AllocateZeroed
		ldy CFatClusterCache_muiBytesForBits
		lda [[D + F_THIS]:24 + Y]
		ldy DIRTY
		jsr AllocateZeroed
		pei [D + F_CLUSTER + 2]
		pei [D + F_CLUSTER]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl _CFatClusterCache__Invalidate
		ply
		ply
		ply
		ply
		jmp Leave


; malloc's A bytes into F_CLUSTER's pointer at offset Y and zeroes them.
AllocateZeroed:
		.width (A, X), 16
		sta [D + F_LENGTH]
		sty [D + F_INDEX]
		pea 0
		pha
		jsl _malloc
		ply
		ply
		ldy [D + F_INDEX]
		sta [[D + F_CLUSTER]:24 + Y]
		iny
		iny
		pha
		txa
		sta [[D + F_CLUSTER]:24 + Y]
		pla
		pea 0
		pei [D + F_LENGTH]
		pea 0
		phx
		pha
		jsl _memset
		tsc
		clc
		adc 10
		tcs
		rts


; void CFatClusterCache::Kill(void)
		.global _CFatClusterCache__Kill
_CFatClusterCache__Kill:
		jsr Enter
		jsr ThisCluster
		ldy DIRTY
		jsr FreePointer
		ldy CACHED
		jsr FreePointer
		ldy SClusterCache_pvCache
		jsr FreePointer
		jmp Leave


; SafeFree of F_CLUSTER's pointer at offset Y.
FreePointer:
		.width (A, X), 16
		sty [D + F_INDEX]
		iny
		iny
		lda [[D + F_CLUSTER]:24 + Y]
		tax
		ldy [D + F_INDEX]
		ora [[D + F_CLUSTER]:24 + Y]
		beq FreePointerReturn
		phx
		lda [[D + F_CLUSTER]:24 + Y]
		pha
		jsl _free
		ply
		ply
		lda 0
		ldy [D + F_INDEX]
		sta [[D + F_CLUSTER]:24 + Y]
		iny
		iny
		sta [[D + F_CLUSTER]:24 + Y]
FreePointerReturn:
		rts


; uint16 CFatClusterCache::LastSectorBefore(uint16 uiEnd): the sector of the byte before uiEnd (0 when uiEnd is 0).
		.global _CFatClusterCache__LastSectorBefore
_CFatClusterCache__LastSectorBefore:
		jsr Enter
		lda [D + F_ARGUMENT1]
		jsr SectorBefore
		jmp Leave


; The sector of the byte before A (0 when A is 0).
SectorBefore:
		.width (A, X), 16
		beq SectorBeforeReturn
		dec A
		jsr ShiftToSector
SectorBeforeReturn:
		rts


; A >> muiSectorSizeShift.
ShiftToSector:
		.width (A, X), 16
		pha
		ldy CFatClusterCache_muiSectorSizeShift
		lda [[D + F_THIS]:24 + Y]
		tax
		pla
ShiftToSectorNext:
		dex
		bmi ShiftToSectorReturn
		lsr A
		bra ShiftToSectorNext
ShiftToSectorReturn:
		rts


; The offset and the length (F_ARGUMENT4, F_LENGTH) checked against muiClusterSize and the maximum offset
; (F_ARGUMENT6); the cluster made the cached one (flushing the one cached) and F_FIRST and F_LAST its first and last
; sectors.  Carry set if it can't be read or written (Read and Write return false).
Begin:
		.width (A, X), 16
		ldy CFatClusterCache_muiClusterSize
		lda [D + F_ARGUMENT4]
		cmp [[D + F_THIS]:24 + Y]
		bcs BeginFail
		lda [D + F_ARGUMENT6]
		cmp [D + F_ARGUMENT4]
		bcc BeginFail
		lda [[D + F_THIS]:24 + Y]
		cmp [D + F_ARGUMENT6]
		bcc BeginFail
		jsr ThisCluster
		ldy SClusterCache_uiCluster
		lda [[D + F_CLUSTER]:24 + Y]
		cmp [D + F_ARGUMENT2]
		bne BeginOtherCluster
		ldy SClusterCache_uiCluster + 2
		lda [[D + F_CLUSTER]:24 + Y]
		cmp [D + F_ARGUMENT2 + 2]
		beq BeginSameCluster
BeginOtherCluster:
		pei [D + F_CLUSTER + 2]
		pei [D + F_CLUSTER]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl _CFatClusterCache__FlushAndInvalidate
		ply
		ply
		ply
		ply
		and 0xFF
		beq BeginFail
		ldx 2
BeginCopy:
		lda [D + F_ARGUMENT2 + X]
		txy
		sta [[D + F_CLUSTER]:24 + Y]
		lda [D + F_ARGUMENT3 + X]
		pha
		txa
		clc
		adc SClusterCache_uiClusterFirstSector
		tay
		pla
		sta [[D + F_CLUSTER]:24 + Y]
		dex
		dex
		bpl BeginCopy
BeginSameCluster:
		lda [[D + F_ARGUMENT5]:24]
		sta [D + F_LENGTH]
		ldy 2
		lda [[D + F_ARGUMENT5]:24 + Y]
		sta [D + F_LENGTH + 2]
		clc
		rts
BeginFail:
		sec
		rts


; F_FIRST = the offset's sector, F_LAST = the sector of offset + length - 1 (worked out in 32 bits, then 16).
Sectors:
		.width (A, X), 16
		lda [D + F_ARGUMENT4]
		jsr ShiftToSector
		sta [D + F_FIRST]
		lda [D + F_LENGTH]
		sec
		sbc 1
		sta [D + F_POINTER]
		lda [D + F_LENGTH + 2]
		sbc 0
		sta [D + F_POINTER + 2]
		lda [D + F_POINTER]
		clc
		adc [D + F_ARGUMENT4]
		sta [D + F_POINTER]
		bcc SectorsShift
		inc [D + F_POINTER + 2]
SectorsShift:
		ldy CFatClusterCache_muiSectorSizeShift
		lda [[D + F_THIS]:24 + Y]
		tax
SectorsShiftNext:
		dex
		bmi SectorsShifted
		lsr [D + F_POINTER + 2]
		ror [D + F_POINTER]
		bra SectorsShiftNext
SectorsShifted:
		lda [D + F_POINTER]
		sta [D + F_LAST]
		rts


; F_LAST past the cluster's last sector: it ends at the cluster's end, and the length (F_LENGTH) is what is left of
; the cluster after the offset.  Carry set if it did.
ClipToCluster:
		.width (A, X), 16
		ldy CFatClusterCache_muiSectorsPerCluster
		lda [D + F_LAST]
		cmp [[D + F_THIS]:24 + Y]
		bcc ClipReturn
		lda [[D + F_THIS]:24 + Y]
		dec A
		sta [D + F_LAST]
		ldy CFatClusterCache_muiClusterSize
		lda [[D + F_THIS]:24 + Y]
		sec
		sbc [D + F_ARGUMENT4]
		sta [D + F_LENGTH]
		stz [D + F_LENGTH + 2]
		sec
ClipReturn:
		rts


; *puiLength -= F_LENGTH.
SubtractLength:
		.width (A, X), 16
		lda [[D + F_ARGUMENT5]:24]
		sec
		sbc [D + F_LENGTH]
		sta [[D + F_ARGUMENT5]:24]
		ldy 2
		lda [[D + F_ARGUMENT5]:24 + Y]
		sbc [D + F_LENGTH + 2]
		sta [[D + F_ARGUMENT5]:24 + Y]
		rts


; bool CFatClusterCache::Read(uint8* pvDestination, uint32 uiCluster, uint32 uiClusterFirstSector, uint16 uiOffset,
;                             uint32* puiLength, uint16 uiMaximumOffset)
		.global _CFatClusterCache__Read
_CFatClusterCache__Read:
		jsr Enter
		jsr Begin
		bcc ReadBegun
		jmp ReturnFalse
ReadBegun:
		; uiOffset + uiLength > uiMaximumOffset: uiLength = uiMaximumOffset - uiOffset.
		lda [D + F_ARGUMENT4]
		clc
		adc [D + F_LENGTH]
		tax
		lda [D + F_LENGTH + 2]
		adc 0
		bne ReadLimit
		cpx [D + F_ARGUMENT6]
		beq ReadLimited
		bcc ReadLimited
ReadLimit:
		lda [D + F_ARGUMENT6]
		sec
		sbc [D + F_ARGUMENT4]
		sta [D + F_LENGTH]
		stz [D + F_LENGTH + 2]
ReadLimited:
		jsr Sectors
		lda [D + F_ARGUMENT6]
		jsr SectorBefore
		cmp [D + F_FIRST]
		bcs ReadInCluster
		jmp ReturnFalse
ReadInCluster:
		jsr ClipToCluster
		jsr SubtractLength

		; Reads the sectors from the first to the last that aren't cached, a run at a time.
		lda [D + F_FIRST]
		sta [D + F_START]
ReadRun:
		ldx [D + F_START]
		ldy CACHED
		stz [D + F_BIT]
		jsr FindBit
		sta [D + F_START]
		cmp NOT_FOUND
		beq ReadCopy
		lda [D + F_LAST]
		cmp [D + F_START]
		bcc ReadCopy
		ldx [D + F_START]
		ldy CACHED
		lda 1
		sta [D + F_BIT]
		jsr FindBit
		sta [D + F_END]
		cmp NOT_FOUND
		beq ReadRunToLast
		lda [D + F_LAST]
		cmp [D + F_END]
		bcs ReadRunEnded
ReadRunToLast:
		lda [D + F_LAST]
		inc A
		sta [D + F_END]
ReadRunEnded:
		; mpcDrive->Read(uiClusterFirstSector + iStart, uiNumSectors, &pvCache[iStart << muiSectorSizeShift])
		ldy VTABLE_READS
		jsr DriveFunction
		lda [D + F_START]
		jsr SectorPointer
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		pea 0
		lda [D + F_END]
		sec
		sbc [D + F_START]
		pha
		lda [D + F_START]
		jsr PushSector
		pei [D + F_DRIVE + 2]
		pei [D + F_DRIVE]
		phk
		per ReadRunRead - 1
		jsr CallFunction
ReadRunRead:
		.width (A, X), 16
		tsc
		clc
		adc 20
		tcs
		; SetBits(iStart, pbCachedSectors, true, uiNumSectors)
ReadRunCached:
		ldx [D + F_START]
		ldy CACHED
		jsr SetBit
		inc [D + F_START]
		lda [D + F_START]
		cmp [D + F_END]
		bcc ReadRunCached
		lda [D + F_LAST]
		cmp [D + F_END]
		bcs ReadRun

		; memcpy(pvDestination, &pvCache[uiOffset], uiLength)
ReadCopy:
		lda [D + F_ARGUMENT4]
		jsr OffsetPointer
		pei [D + F_LENGTH + 2]
		pei [D + F_LENGTH]
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		pei [D + F_ARGUMENT1 + 2]
		pei [D + F_ARGUMENT1]
		jsl _memcpy
		tsc
		clc
		adc 12
		tcs
		jmp ReturnTrue


; bool CFatClusterCache::Write(uint8* pvSource, uint32 uiCluster, uint32 uiClusterFirstSector, uint16 uiOffset,
;                              uint32* puiLength, uint16 uiPreviousMaximumOffset)
		.global _CFatClusterCache__Write
_CFatClusterCache__Write:
		jsr Enter
		jsr Begin
		bcc WriteBegun
		jmp ReturnFalse
WriteBegun:
		jsr Sectors
		jsr ClipToCluster
		bcc WriteAll
		jsr SubtractLength
		bra WriteClipped
WriteAll:
		lda 0
		sta [[D + F_ARGUMENT5]:24]
		ldy 2
		sta [[D + F_ARGUMENT5]:24 + Y]
WriteClipped:
		lda [D + F_ARGUMENT6]
		jsr SectorBefore
		sta [D + F_MAXIMUM]

		; The first sector keeps what the write does not cover: before it, and after it when it ends in the same sector.
		cmp [D + F_FIRST]
		bcc WriteFirst
		ldy CFatClusterCache_muiSectorSize
		lda [[D + F_THIS]:24 + Y]
		dec A
		and [D + F_ARGUMENT4]
		bne WriteCacheFirst
		lda [D + F_FIRST]
		cmp [D + F_LAST]
		bne WriteFirst
		jsr EndsInSector
		beq WriteFirst
WriteCacheFirst:
		ldx [D + F_FIRST]
		ldy CACHED
		jsr GetBit
		bne WriteFirst
		lda [D + F_FIRST]
		jsr CacheThisSector
		bcc WriteFirst
		jmp ReturnFalse

WriteFirst:
		; uiSectorLength = min(muiSectorSize - (uiOffset & (muiSectorSize - 1)), uiLength)
		ldy CFatClusterCache_muiSectorSize
		lda [[D + F_THIS]:24 + Y]
		dec A
		and [D + F_ARGUMENT4]
		eor 0xFFFF
		sec
		adc [[D + F_THIS]:24 + Y]
		sta [D + F_SECTOR_LENGTH]
		lda [D + F_LENGTH + 2]
		bne WriteFirstLength
		lda [D + F_LENGTH]
		cmp [D + F_SECTOR_LENGTH]
		bcs WriteFirstLength
		sta [D + F_SECTOR_LENGTH]
WriteFirstLength:
		stz [D + F_INDEX]
		lda [D + F_ARGUMENT4]
		sta [D + F_OFFSET]
		lda [D + F_FIRST]
		jsr WriteSector

		; The sectors between the first and the last, whole.
		ldy CFatClusterCache_muiSectorSize
		lda [[D + F_THIS]:24 + Y]
		sta [D + F_SECTOR_LENGTH]
		lda [D + F_FIRST]
		sta [D + F_START]
WriteMiddle:
		inc [D + F_START]
		lda [D + F_START]
		cmp [D + F_LAST]
		bcs WriteLast
		jsr WriteSector
		bra WriteMiddle

WriteLast:
		lda [D + F_FIRST]
		cmp [D + F_LAST]
		beq ReturnTrue
		lda [D + F_MAXIMUM]
		cmp [D + F_LAST]
		bcc WriteLastSector
		jsr EndsInSector
		beq WriteLastSector
		ldx [D + F_LAST]
		ldy CACHED
		jsr GetBit
		bne WriteLastSector
		lda [D + F_LAST]
		jsr CacheThisSector
		bcs ReturnFalse
WriteLastSector:
		lda [D + F_LENGTH]
		sta [D + F_SECTOR_LENGTH]
		lda [D + F_LAST]
		jsr WriteSector
		bra ReturnTrue


; Z clear if (uiOffset + uiLength) & (muiSectorSize - 1) isn't 0 (F_OFFSET and F_LENGTH once the writing starts,
; F_ARGUMENT4 before: WriteSector keeps F_OFFSET up to date, and F_ARGUMENT4 is copied to it first).
EndsInSector:
		.width (A, X), 16
		ldy CFatClusterCache_muiSectorSize
		lda [[D + F_THIS]:24 + Y]
		dec A
		sta [D + F_POINTER]
		lda [D + F_ARGUMENT4]
		ldx [D + F_INDEX]
		beq EndsInSectorOffset
		lda [D + F_OFFSET]
EndsInSectorOffset:
		clc
		adc [D + F_LENGTH]
		and [D + F_POINTER]
		rts


; Copies F_SECTOR_LENGTH bytes from pvSource[F_INDEX] to pvCache[F_OFFSET], marks sector A cached and dirty and moves
; the index, the offset and the length (F_LENGTH) on.
WriteSector:
		.width (A, X), 16
		sta [D + F_END]
		pea 0
		pei [D + F_SECTOR_LENGTH]
		lda [D + F_ARGUMENT1]
		clc
		adc [D + F_INDEX]
		tax
		lda [D + F_ARGUMENT1 + 2]
		adc 0
		pha
		phx
		lda [D + F_OFFSET]
		jsr OffsetPointer
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		jsl _memcpy
		tsc
		clc
		adc 12
		tcs
		ldx [D + F_END]
		ldy CACHED
		jsr SetBit
		ldx [D + F_END]
		ldy DIRTY
		jsr SetBit
		lda [D + F_INDEX]
		clc
		adc [D + F_SECTOR_LENGTH]
		sta [D + F_INDEX]
		lda [D + F_OFFSET]
		clc
		adc [D + F_SECTOR_LENGTH]
		sta [D + F_OFFSET]
		lda [D + F_LENGTH]
		sec
		sbc [D + F_SECTOR_LENGTH]
		sta [D + F_LENGTH]
		lda [D + F_LENGTH + 2]
		sbc 0
		sta [D + F_LENGTH + 2]
		rts


; CacheSector(&msCluster, A): carry set if it failed.
CacheThisSector:
		.width (A, X), 16
		pea 0
		pha
		pei [D + F_CLUSTER + 2]
		pei [D + F_CLUSTER]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl _CFatClusterCache__CacheSector
		tay
		tsc
		clc
		adc 12
		tcs
		tya
		and 0xFF
		clc
		bne CacheThisSectorReturn
		sec
CacheThisSectorReturn:
		rts


ReturnTrue:
		lda 1
		jmp Leave
ReturnFalse:
		lda 0
		jmp Leave


; bool CFatClusterCache::Flush(void)
		.global _CFatClusterCache__Flush
_CFatClusterCache__Flush:
		jsr Enter
		jsr ThisCluster
		bra FlushCluster


; bool CFatClusterCache::Flush(SClusterCache* psCluster): writes each run of dirty sectors.
		.global _CFatClusterCache__Flush__1
_CFatClusterCache__Flush__1:
		jsr Enter
		jsr ArgumentCluster
FlushCluster:
		ldy SClusterCache_uiCluster
		lda [[D + F_CLUSTER]:24 + Y]
		ldy SClusterCache_uiCluster + 2
		and [[D + F_CLUSTER]:24 + Y]
		cmp 0xFFFF
		beq ReturnTrue
		ldx 0
		ldy DIRTY
		lda 1
		sta [D + F_BIT]
		jsr FindBit
		sta [D + F_START]
		cmp NOT_FOUND
		beq ReturnTrue
FlushRun:
		ldx [D + F_START]
		ldy DIRTY
		stz [D + F_BIT]
		jsr FindBit
		sta [D + F_END]
		cmp NOT_FOUND
		bne FlushWrite
		ldy CFatClusterCache_muiSectorsPerCluster
		lda [[D + F_THIS]:24 + Y]
		sta [D + F_END]
FlushWrite:
		; mpcDrive->Write(uiClusterFirstSector + iFirstDirtySector, iLastDirtySector - iFirstDirtySector, &pvCache[...])
		ldy VTABLE_WRITES
		jsr DriveFunction
		lda [D + F_START]
		jsr SectorPointer
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		pea 0
		lda [D + F_END]
		sec
		sbc [D + F_START]
		pha
		lda [D + F_START]
		jsr PushSector
		pei [D + F_DRIVE + 2]
		pei [D + F_DRIVE]
		phk
		per FlushWritten - 1
		jsr CallFunction
FlushWritten:
		.width (A, X), 16
		tay
		tsc
		clc
		adc 20
		tcs
		tya
		and 0xFF
		bne FlushNext
		jmp ReturnFalse
FlushNext:
		ldy CFatClusterCache_muiSectorsPerCluster
		lda [D + F_END]
		cmp [[D + F_THIS]:24 + Y]
		bcs FlushClean
		tax
		ldy DIRTY
		lda 1
		sta [D + F_BIT]
		jsr FindBit
		sta [D + F_START]
		cmp NOT_FOUND
		bne FlushRun
FlushClean:
		ldy DIRTY
		jsr ClearBits
		jmp ReturnTrue


; memset(F_CLUSTER's bit array at offset Y, 0, muiBytesForBits)
ClearBits:
		.width (A, X), 16
		pea 0
		phy
		ldy CFatClusterCache_muiBytesForBits
		lda [[D + F_THIS]:24 + Y]
		ply
		pha
		pea 0
		iny
		iny
		lda [[D + F_CLUSTER]:24 + Y]
		pha
		dey
		dey
		lda [[D + F_CLUSTER]:24 + Y]
		pha
		jsl _memset
		tsc
		clc
		adc 10
		tcs
		rts


; bool CFatClusterCache::FlushAndInvalidate(SClusterCache* psCluster)
		.global _CFatClusterCache__FlushAndInvalidate
_CFatClusterCache__FlushAndInvalidate:
		jsr Enter
		pei [D + F_ARGUMENT1 + 2]
		pei [D + F_ARGUMENT1]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		jsl _CFatClusterCache__Flush__1
		sta [D + F_INDEX]
		jsl _CFatClusterCache__Invalidate
		ply
		ply
		ply
		ply
		lda [D + F_INDEX]
		and 0xFF
		jmp Leave


; bool CFatClusterCache::IsCached(uint32 uiSectorIndex)
		.global _CFatClusterCache__IsCached
_CFatClusterCache__IsCached:
		jsr Enter
		jsr ThisCluster
		ldx [D + F_ARGUMENT1]
		ldy CACHED
		jsr GetBit
		beq ReturnFalseShort
		jmp ReturnTrue
ReturnFalseShort:
		jmp ReturnFalse


; bool CFatClusterCache::CacheSector(SClusterCache* psCluster, uint32 uiSectorIndex)
		.global _CFatClusterCache__CacheSector
_CFatClusterCache__CacheSector:
		jsr Enter
		jsr ArgumentCluster
		; mpcDrive->Read(uiClusterFirstSector + uiSectorIndex, &pvCache[uiSectorIndex << muiSectorSizeShift])
		ldy VTABLE_READ
		jsr DriveFunction
		lda [D + F_ARGUMENT2]
		jsr SectorPointer
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		lda [D + F_ARGUMENT2]
		jsr PushSector
		pei [D + F_DRIVE + 2]
		pei [D + F_DRIVE]
		phk
		per CacheSectorRead - 1
		jsr CallFunction
CacheSectorRead:
		.width (A, X), 16
		tay
		tsc
		clc
		adc 16
		tcs
		tya
		and 0xFF
		beq ReturnFalseShort
		; SetCachedSector and SetDirtySector check the index against muiBytesForBits.
		jsr ThisCluster
		ldx [D + F_ARGUMENT2]
		jsr IndexInBits
		bcs CacheSectorDone
		ldy CACHED
		jsr SetBit
		ldx [D + F_ARGUMENT2]
		ldy DIRTY
		jsr ClearBit
CacheSectorDone:
		jmp ReturnTrue


; Carry clear if X is less than muiBytesForBits (SetCachedSector and SetDirtySector's check).
IndexInBits:
		.width (A, X), 16
		txa
		ldy CFatClusterCache_muiBytesForBits
		cmp [[D + F_THIS]:24 + Y]
		rts


; void CFatClusterCache::Clear(void)
		.global _CFatClusterCache__Clear
_CFatClusterCache__Clear:
		jsr Enter
		jsr ThisCluster
		bra InvalidateCluster


; void CFatClusterCache::Invalidate(SClusterCache* psCluster): zeroes the cached sectors (the others are zero).
		.global _CFatClusterCache__Invalidate
_CFatClusterCache__Invalidate:
		jsr Enter
		jsr ArgumentCluster
InvalidateCluster:
		lda NO_CLUSTER_CACHED & 0xFFFF
		ldy SClusterCache_uiCluster
		sta [[D + F_CLUSTER]:24 + Y]
		ldy SClusterCache_uiCluster + 2
		sta [[D + F_CLUSTER]:24 + Y]
		stz [D + F_START]
InvalidateNext:
		ldy CFatClusterCache_muiSectorsPerCluster
		lda [D + F_START]
		cmp [[D + F_THIS]:24 + Y]
		bcs InvalidateBits
		tax
		ldy CACHED
		jsr GetBit
		beq InvalidateSkip
		ldy CFatClusterCache_muiSectorSize
		lda [[D + F_THIS]:24 + Y]
		pea 0
		pha
		pea 0
		lda [D + F_START]
		jsr SectorPointer
		pei [D + F_POINTER + 2]
		pei [D + F_POINTER]
		jsl _memset
		tsc
		clc
		adc 10
		tcs
InvalidateSkip:
		inc [D + F_START]
		bra InvalidateNext
InvalidateBits:
		ldy CACHED
		jsr ClearBits
		ldy DIRTY
		jsr ClearBits
		jmp Leave


; uint8* CFatClusterCache::GetCache(void)
		.global _CFatClusterCache__GetCache
_CFatClusterCache__GetCache:
		jsr Enter
		jsr ThisCluster
		ldy SClusterCache_pvCache + 2
		lda [[D + F_CLUSTER]:24 + Y]
		tax
		ldy SClusterCache_pvCache
		lda [[D + F_CLUSTER]:24 + Y]
		jmp Leave


; bool CFatClusterCache::IsSectorDirty(uint16 iSectorIndex)
		.global _CFatClusterCache__IsSectorDirty
_CFatClusterCache__IsSectorDirty:
		jsr Enter
		ldy DIRTY
		bra IsSector


; bool CFatClusterCache::IsSectorCached(uint16 iSectorIndex)
		.global _CFatClusterCache__IsSectorCached
_CFatClusterCache__IsSectorCached:
		jsr Enter
		ldy CACHED
IsSector:
		phy
		jsr ThisCluster
		ply
		ldx [D + F_ARGUMENT1]
		txa
		phy
		ldy CFatClusterCache_muiSectorsPerCluster
		cmp [[D + F_THIS]:24 + Y]
		ply
		bcs IsSectorFalse
		jsr GetBit
		beq IsSectorFalse
		jmp ReturnTrue
IsSectorFalse:
		jmp ReturnFalse


; uint16 CFatClusterCache::GetSectorsPerCluster(void)
		.global _CFatClusterCache__GetSectorsPerCluster
_CFatClusterCache__GetSectorsPerCluster:
		jsr Enter
		ldy CFatClusterCache_muiSectorsPerCluster
		lda [[D + F_THIS]:24 + Y]
		jmp Leave


; bool CFatClusterCache::SetCachedSector(size uiSectorIndex, bool bCached)
		.global _CFatClusterCache__SetCachedSector
_CFatClusterCache__SetCachedSector:
		jsr Enter
		ldy CACHED
		bra SetSector


; bool CFatClusterCache::SetDirtySector(size uiSectorIndex, bool bDirty)
		.global _CFatClusterCache__SetDirtySector
_CFatClusterCache__SetDirtySector:
		jsr Enter
		ldy DIRTY
SetSector:
		phy
		jsr ThisCluster
		ldx [D + F_ARGUMENT1]
		jsr IndexInBits
		ply
		bcs IsSectorFalse
		lda [D + F_ARGUMENT1 + 2]
		and 0xFF
		beq SetSectorClear
		jsr SetBit
		jmp ReturnTrue
SetSectorClear:
		jsr ClearBit
		jmp ReturnTrue
