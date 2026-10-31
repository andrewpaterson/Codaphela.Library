; FatFile.cpp for the W65C816, with the compiler's calling convention (see FatInfoSectorCache816.asm).  The public
; functions (and FindBackslash) keep their names; the protected ones are helpers here (only CFatFile calls them).
;
; Every function here has the same frame (F_FRAME bytes), so the helpers (jsr, within this one section) find this,
; the volume and their scratch in the same places.  this is at F_THIS and the arguments follow it (F_ARGUMENT + their
; offset: pointers and 32 bit values take 4 bytes, 16 bit values, bools, uint8s and enums 2).  F_VOLUME is a copy of
; mpcVolume.  Cluster arithmetic is shifts and masks (the volume's sizes are powers of two: Mount checks).

		.include "Fat816.inc"

F_VOLUME		= 1				; mpcVolume.
F_ADDRESS		= 5				; A pointer argument (FrameAddress, ThisAddress).
F_POINTER		= 9
F_CLUSTER		= 13			; IsEOF's and NextCluster's cluster.
F_NEXT			= 17			; Where NextCluster puts the next cluster (through F_ADDRESS).
F_FIRST			= 21			; FirstCluster's result.
F_COUNT			= 25			; Clusters in the file, a cluster index.
F_INDEX			= 29			; HasCluster's count down.
F_PREVIOUS		= 33
F_NEW			= 37			; The volume's AllocateClusters' first new cluster.
F_BYTES			= 41			; AllocateFileClusters' bytes.
F_WORK			= 45
F_POSITION		= 49
F_SECTOR		= 53			; The first sector of the position's cluster.
F_REMAINING		= 57			; Read's and Write's bytes still to go (the cluster cache counts them down).
F_DONE			= 61
F_TOTAL			= 65
F_DATA			= 69			; Read's destination, Write's source.
F_CACHE			= 73			; 8 bytes: an SFatCache.
F_OFFSET		= 81
F_MAXIMUM		= 83
F_RESULT		= 85
F_FLAGS			= 87
F_DROP			= 89
F_SCANNER		= 91			; Open's FindBackslash result.
F_PATH			= 95			; Open's parent path and entry (below the frame).
F_PARENT		= 99
F_FRAME			= 102
F_THIS			= F_FRAME + 6
F_ARGUMENT		= F_FRAME + 10

RAW				= SFatFile_sDirectoryEntry + SFatDirectoryEntry_sRaw			; msFile's raw directory entry.
OPEN_PATH_SIZE	= (FAT_MAX_PATH + 2) & 0xFFFE
OPEN_LOCALS		= OPEN_PATH_SIZE + SIZEOF_SFatDirectoryEntry


		.section text.FatFile816
		.relax


; The frame (F_FRAME bytes) for a function called with jsl: jsr'd straight after its label.  The return address (the
; jsr's 2 bytes) is moved below the frame.  F_VOLUME = this's mpcVolume.
Enter:
		.width (A, X), 16
		ply
		phd
		tsc
		sec
		sbc F_FRAME
		tcs
		tcd
		phy
		ldy CFatFile_mpcVolume
		lda [[D + F_THIS]:24 + Y]
		sta [D + F_VOLUME]
		iny
		iny
		lda [[D + F_THIS]:24 + Y]
		sta [D + F_VOLUME + 2]
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


; jsr'd straight after a jsl: removes X bytes of its arguments from the stack, A kept (X and Y not).
Drop:
		.width (A, X), 16
		stx [D + F_DROP]
		plx
		tay
		tsc
		clc
		adc [D + F_DROP]
		tcs
		tya
		phx
		rts


; F_ADDRESS = the address of the frame's D + A (bank 0).
FrameAddress:
		.width (A, X), 16
		sta [D + F_ADDRESS]
		tdc
		clc
		adc [D + F_ADDRESS]
		sta [D + F_ADDRESS]
		stz [D + F_ADDRESS + 2]
		rts


; F_ADDRESS = this + A.
ThisAddress:
		.width (A, X), 16
		clc
		adc [D + F_THIS]
		sta [D + F_ADDRESS]
		lda [D + F_THIS + 2]
		adc 0
		sta [D + F_ADDRESS + 2]
		rts


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


; Copies the frame's 32 bit value at D + A to D + X.
Copy32:
		.width (A, X), 16
		phx
		tax
		lda [D + 0 + X]
		tay
		lda [D + 2 + X]
		plx
		sta [D + 2 + X]
		tya
		sta [D + 0 + X]
		rts


; The frame's 32 bit D + X shifted right by the volume's cluster shift (bytes to clusters).
ShiftCluster:
		.width (A, X), 16
		ldy CFatVolume_muiBytesPerClusterShift
		lda [[D + F_VOLUME]:24 + Y]
		and 0xFF
		beq ShiftClusterReturn
		tay
ShiftClusterBit:
		lsr [D + 2 + X]
		ror [D + 0 + X]
		dey
		bne ShiftClusterBit
ShiftClusterReturn:
		rts


; A = the frame's D + X's offset in its cluster (Z set if 0).  A cluster size of 64KB masks with 0xFFFF.
OffsetInCluster:
		.width (A, X), 16
		ldy CFatVolume_muiBytesPerCluster
		lda [[D + F_VOLUME]:24 + Y]
		dec A
		and [D + 0 + X]
		rts


; F_FIRST = the file's first cluster (GetFirstClusterFromFatEntry: the high word only on FAT32).
FirstCluster:
		.width (A, X), 16
		ldy RAW + RAW_uiFirstClusterLowWord
		lda [[D + F_THIS]:24 + Y]
		sta [D + F_FIRST]
		stz [D + F_FIRST + 2]
		jsr IsFat32
		bne FirstClusterReturn
		ldy RAW + RAW_uiFirstClusterHighWord
		lda [[D + F_THIS]:24 + Y]
		sta [D + F_FIRST + 2]
FirstClusterReturn:
		rts


; Z set if the volume is FAT32.
IsFat32:
		.width (A, X), 16
		ldy CFatVolume_meFileSystem
		lda [[D + F_VOLUME]:24 + Y]
		cmp FAT_FS_TYPE_FAT32
		rts


; Marks the file busy.
SetBusy:
		.width (A, X), 16
		sep m
		lda 1
		ldy SFatFile_bBusy
		sta [[D + F_THIS]:24 + Y]
		rep m
		rts


; Marks the file not busy, A kept.
ClearBusy:
		.width (A, X), 16
		pha
		sep m
		lda 0
		ldy SFatFile_bBusy
		sta [[D + F_THIS]:24 + Y]
		rep m
		pla
		rts


; ValidateCanRead: carry set (A the error) if the file isn't open or is busy.
CheckHandle:
		.width (A, X), 16
		ldy SFatFile_uiMagic
		lda [[D + F_THIS]:24 + Y]
		and 0xFF
		cmp FAT_OPEN_HANDLE_MAGIC
		bne CheckHandleInvalid
CheckBusy:
		ldy SFatFile_bBusy
		lda [[D + F_THIS]:24 + Y]
		and 0xFF
		bne CheckHandleBusy
		clc
		rts
CheckHandleInvalid:
		lda FAT_INVALID_HANDLE
		sec
		rts
CheckHandleBusy:
		lda FAT_FILE_HANDLE_IN_USE
		sec
		rts


; ValidateCanWrite: CheckHandle, and carry set if the file wasn't opened for writing.
CheckWritable:
		.width (A, X), 16
		ldy SFatFile_uiMagic
		lda [[D + F_THIS]:24 + Y]
		and 0xFF
		cmp FAT_OPEN_HANDLE_MAGIC
		bne CheckHandleInvalid
		ldy SFatFile_uiAccessFlags
		lda [[D + F_THIS]:24 + Y]
		and FAT_FILE_ACCESS_WRITE
		bne CheckBusy
		lda FAT_FILE_NOT_OPENED_FOR_WRITE_ACCESS
		sec
		rts


; A = mpcVolume->FatIsEOFEntry(F_CLUSTER) (Z clear if it is).
IsEOF:
		.width (A, X), 16
		pei [D + F_CLUSTER + 2]
		pei [D + F_CLUSTER]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__FatIsEOFEntry
		ply
		ply
		ply
		ply
		and 0xFF
		rts


; A = mpcVolume->GetNextClusterEntry(F_CLUSTER, F_ADDRESS) (Z set if FAT_SUCCESS).
NextCluster:
		.width (A, X), 16
		pei [D + F_ADDRESS + 2]
		pei [D + F_ADDRESS]
		pei [D + F_CLUSTER + 2]
		pei [D + F_CLUSTER]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__GetNextClusterEntry
		ldx 12
		jsr Drop
		cmp FAT_SUCCESS
		rts


; A = mpcVolume->GetNextClusterEntry(msFile.uiFilePositionCluster, &msFile.uiFilePositionCluster).
NextPositionCluster:
		.width (A, X), 16
		ldy SFatFile_uiFilePositionCluster
		ldx F_CLUSTER
		jsr GetMember32
		lda SFatFile_uiFilePositionCluster
		jsr ThisAddress
		jmp NextCluster


; StepToClusterOfPosition: a read or write that ends on a cluster boundary inside the file leaves the position at the
; start of the next cluster: move to it, as Seek does.  A the result.
StepToPosition:
		.width (A, X), 16
		ldy SFatFile_uiFilePosition
		ldx F_POSITION
		jsr GetMember32
		ldx F_POSITION
		jsr OffsetInCluster
		bne StepToPositionDone
		ldy SFatFile_uiFileSize + 2
		lda [D + F_POSITION + 2]
		cmp [[D + F_THIS]:24 + Y]
		bcc StepToPositionNext
		bne StepToPositionDone
		dey
		dey
		lda [D + F_POSITION]
		cmp [[D + F_THIS]:24 + Y]
		bcs StepToPositionDone
StepToPositionNext:
		jmp NextPositionCluster
StepToPositionDone:
		lda FAT_SUCCESS
		rts


; void CFatFile::Init(CFatVolume* pcVolume)
		.global _CFatFile__Init
_CFatFile__Init:
		.width (A, X), 16
		jsr Enter
		lda 0
		ldy SIZEOF_SFatFile - 2
InitClear:
		sta [[D + F_THIS]:24 + Y]
		dey
		dey
		bpl InitClear
		ldy CFatFile_mpcVolume
		lda [D + F_ARGUMENT]
		sta [[D + F_THIS]:24 + Y]
		iny
		iny
		lda [D + F_ARGUMENT + 2]
		sta [[D + F_THIS]:24 + Y]
		jmp Leave


; EFatCode FindBackslash(const char* szFilename, const char** ppcFilenameScanner): the last \ in the path (not its
; last character).
		.global _FindBackslash__FPcPPc
_FindBackslash__FPcPPc:
		.width (A, X), 16
		jsr Enter
		ldy 0
		sep m
FindBackslashEnd:
		lda [[D + F_THIS]:24 + Y]
		beq FindBackslashEnded
		iny
		bra FindBackslashEnd
FindBackslashEnded:
		cpy 0
		beq FindBackslashNoPath
		dey
		lda [[D + F_THIS]:24 + Y]
		cmp 0x5C
		beq FindBackslashLast
FindBackslashScan:
		lda [[D + F_THIS]:24 + Y]
		cmp 0x5C
		beq FindBackslashFound
		cpy 0
		beq FindBackslashNoPath
		dey
		bra FindBackslashScan
FindBackslashFound:
		rep m
		tya
		clc
		adc [D + F_THIS]
		sta [[D + F_ARGUMENT]:24]
		lda [D + F_THIS + 2]
		adc 0
		ldy 2
		sta [[D + F_ARGUMENT]:24 + Y]
		lda FAT_SUCCESS
		jmp Leave
FindBackslashLast:
		.width A, 8
		rep m
		lda FAT_INVALID_FILENAME
		jmp Leave
FindBackslashNoPath:
		.width A, 8
		rep m
		lda FAT_INVALID_PATH
		jmp Leave


; mcCache.Init(mpcVolume->GetFileDrive(), mpcVolume->GetClusterSize(), mpcVolume->GetSectorSize()).
CacheInitialise:
		.width (A, X), 16
		ldy CFatVolume_muiBytesPerSector
		lda [[D + F_VOLUME]:24 + Y]
		pha
		ldy CFatVolume_muiBytesPerCluster
		lda [[D + F_VOLUME]:24 + Y]
		pha
		ldy CFatVolume_mpcDevice + 2
		lda [[D + F_VOLUME]:24 + Y]
		pha
		ldy CFatVolume_mpcDevice
		lda [[D + F_VOLUME]:24 + Y]
		pha
		lda CFatFile_mcCache
		jsr ThisAddress
		pei [D + F_ADDRESS + 2]
		pei [D + F_ADDRESS]
		jsl _CFatClusterCache__Init
		ldx 12
		jsr Drop
		rts


; mcCache.Kill(), A kept.
CacheKill:
		.width (A, X), 16
		pha
		lda CFatFile_mcCache
		jsr ThisAddress
		pei [D + F_ADDRESS + 2]
		pei [D + F_ADDRESS]
		jsl _CFatClusterCache__Kill
		ply
		ply
		pla
		rts


; mcCache.Flush().
CacheFlush:
		.width (A, X), 16
		lda CFatFile_mcCache
		jsr ThisAddress
		pei [D + F_ADDRESS + 2]
		pei [D + F_ADDRESS]
		jsl _CFatClusterCache__Flush
		ply
		ply
		rts


; EFatCode CFatFile::Open(const char* szFilename, uint8 uiAccessFlags)
		.global _CFatFile__Open
_CFatFile__Open:
		.width (A, X), 16
		jsr Enter
		lda 0
		ldy CFatFile_mszName
OpenClearName:
		sta [[D + F_THIS]:24 + Y]
		iny
		iny
		cpy CFatFile_mszName + ((FAT_MAX_FILENAME + 1) & 0xFFFE)
		bne OpenClearName
		jsr CacheInitialise
		; szFilename NULL or longer than FAT_MAX_PATH.
		lda [D + F_ARGUMENT]
		ora [D + F_ARGUMENT + 2]
		beq OpenInvalidName
		ldy 0
		sep m
OpenLength:
		lda [[D + F_ARGUMENT]:24 + Y]
		beq OpenLengthDone
		iny
		cpy FAT_MAX_PATH + 1
		bne OpenLength
		rep m
OpenInvalidName:
		lda FAT_INVALID_FILENAME
		jmp Leave
OpenLengthDone:
		.width A, 8
		rep m
		; GetFileEntry(szFilename, &msFile.sDirectoryEntry): msFile.sDirectoryEntry is at this.
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		pei [D + F_ARGUMENT + 2]
		pei [D + F_ARGUMENT]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__GetFileEntry
		ldx 12
		jsr Drop
		cmp FAT_SUCCESS
		beq OpenLooked
		jmp Leave
OpenLooked:
		ldy SFatFile_sDirectoryEntry + SFatDirectoryEntry_szName
		lda [[D + F_THIS]:24 + Y]
		and 0xFF
		beq OpenNotFound
		jmp OpenFound
OpenNotFound:
		lda [D + F_ARGUMENT + 4]
		and FAT_FILE_ACCESS_CREATE
		bne OpenCreate
		lda FAT_FILE_NOT_FOUND
		jmp Leave
OpenCreate:
		; The parent's path and entry below the frame (Leave puts S back).
		tsc
		sec
		sbc OPEN_LOCALS
		tcs
		inc A
		sta [D + F_PATH]
		stz [D + F_PATH + 2]
		clc
		adc OPEN_PATH_SIZE
		sta [D + F_PARENT]
		stz [D + F_PARENT + 2]
		; FindBackslash(szFilename, &pcFilenameScanner)
		lda F_SCANNER
		jsr FrameAddress
		pei [D + F_ADDRESS + 2]
		pei [D + F_ADDRESS]
		pei [D + F_ARGUMENT + 2]
		pei [D + F_ARGUMENT]
		jsl _FindBackslash__FPcPPc
		ldx 8
		jsr Drop
		cmp FAT_SUCCESS
		beq OpenParentPath
		jmp Leave
OpenParentPath:
		; The path up to the \.
		lda [D + F_SCANNER]
		sec
		sbc [D + F_ARGUMENT]
		sta [D + F_COUNT]
		ldy 0
		sep m
OpenCopyPath:
		cpy [D + F_COUNT]
		beq OpenCopiedPath
		lda [[D + F_ARGUMENT]:24 + Y]
		sta [[D + F_PATH]:24 + Y]
		iny
		bra OpenCopyPath
OpenCopiedPath:
		lda 0
		sta [[D + F_PATH]:24 + Y]
		rep m
		; GetFileEntry(szFilePath, &sParentEntry)
		pei [D + F_PARENT + 2]
		pei [D + F_PARENT]
		pei [D + F_PATH + 2]
		pei [D + F_PATH]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__GetFileEntry
		ldx 12
		jsr Drop
		cmp FAT_SUCCESS
		beq OpenParentLooked
		jmp Leave
OpenParentLooked:
		lda [[D + F_PARENT]:24]
		and 0xFF
		bne OpenParentFound
		lda FAT_DIRECTORY_DOES_NOT_EXIST
		jmp Leave
OpenParentFound:
		; CreateFATEntry(&sParentEntry.sRaw, pcFilenameScanner + 1, FAT_ATTR_ARCHIVE, 0, &msFile.sDirectoryEntry)
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		pea 0
		pea 0
		pea FAT_ATTR_ARCHIVE
		lda [D + F_SCANNER]
		clc
		adc 1
		tax
		lda [D + F_SCANNER + 2]
		adc 0
		pha
		phx
		lda [D + F_PARENT]
		clc
		adc SFatDirectoryEntry_sRaw
		tax
		lda [D + F_PARENT + 2]
		adc 0
		pha
		phx
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__CreateFATEntry
		ldx 22
		jsr Drop
		cmp FAT_SUCCESS
		beq OpenCreated
		jmp Leave
OpenCreated:
		lda [D + F_ARGUMENT + 4]
		and 0xFF ^ FAT_FILE_ACCESS_APPEND
		sta [D + F_ARGUMENT + 4]
		lda 0
		ldy SFatFile_uiFileSize
		sta [[D + F_THIS]:24 + Y]
		iny
		iny
		sta [[D + F_THIS]:24 + Y]
		ldy SFatFile_bBusy
		lda FAT_OPEN_HANDLE_MAGIC << 8
		sta [[D + F_THIS]:24 + Y]
		sep m
		lda [D + F_ARGUMENT + 4]
		ldy SFatFile_uiAccessFlags
		sta [[D + F_THIS]:24 + Y]
		rep m
OpenFound:
		; strcpy(mszName, msFile.sDirectoryEntry.szName): szName is at this.
		lda CFatFile_mszName
		jsr ThisAddress
		ldy 0
		sep m
OpenCopyName:
		lda [[D + F_THIS]:24 + Y]
		sta [[D + F_ADDRESS]:24 + Y]
		beq OpenCopiedName
		iny
		bra OpenCopyName
OpenCopiedName:
		rep m
		; OpenFileByEntry(&msFile.sDirectoryEntry, uiAccessFlags)
		lda [D + F_THIS]
		sta [D + F_POINTER]
		lda [D + F_THIS + 2]
		sta [D + F_POINTER + 2]
		lda [D + F_ARGUMENT + 4]
		sta [D + F_FLAGS]
		jsr OpenByEntry
		jmp Leave


; EFatCode CFatFile::Open(SFatDirectoryEntry* psEntry, uint8 uiAccessFlags)
		.global _CFatFile__Open__1
_CFatFile__Open__1:
		.width (A, X), 16
		jsr Enter
		jsr CacheInitialise
		lda [D + F_ARGUMENT]
		sta [D + F_POINTER]
		lda [D + F_ARGUMENT + 2]
		sta [D + F_POINTER + 2]
		lda [D + F_ARGUMENT + 4]
		sta [D + F_FLAGS]
		jsr OpenByEntry
		jmp Leave


; OpenFileByEntry(F_POINTER, F_FLAGS): A the result.
OpenByEntry:
		.width (A, X), 16
		lda [D + F_FLAGS]
		and 0xFF
		ora FAT_FILE_ACCESS_READ
		bit FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_APPEND | FAT_FILE_ACCESS_OVERWRITE
		beq OpenByEntryFlags
		ora FAT_FILE_ACCESS_WRITE
OpenByEntryFlags:
		sta [D + F_FLAGS]
		ldy SFatDirectoryEntry_uiAttributes
		lda [[D + F_POINTER]:24 + Y]
		and FAT_ATTR_DIRECTORY
		beq OpenByEntryFile
		lda FAT_NOT_A_FILE
		rts
OpenByEntryFile:
		; msFile.sDirectoryEntry = *psEntry (it may be msFile.sDirectoryEntry: copying it onto itself changes nothing).
		ldy SIZEOF_SFatDirectoryEntry - 2
OpenByEntryCopy:
		lda [[D + F_POINTER]:24 + Y]
		sta [[D + F_THIS]:24 + Y]
		dey
		dey
		bpl OpenByEntryCopy
		ldy SFatDirectoryEntry_uiSize
		ldx F_WORK
		jsr GetMember32
		ldy SFatFile_uiFileSize
		ldx F_WORK
		jsr SetMember32
		ldy SFatFile_bBusy
		lda FAT_OPEN_HANDLE_MAGIC << 8
		sta [[D + F_THIS]:24 + Y]
		sep m
		lda [D + F_FLAGS]
		ldy SFatFile_uiAccessFlags
		sta [[D + F_THIS]:24 + Y]
		rep m
		jsr FirstCluster
		ldy SFatFile_uiFilePositionCluster
		ldx F_FIRST
		jsr SetMember32
		lda [D + F_FLAGS]
		and FAT_FILE_ACCESS_APPEND
		beq OpenByEntryNotAppend
		; Seek(0, FAT_SEEK_END)
		ldy SFatFile_uiFileSize
		ldx F_POSITION
		jsr GetMember32
		jsr SeekTo
		cmp FAT_SUCCESS
		bne OpenByEntryFailed
		rts
OpenByEntryNotAppend:
		lda [D + F_FLAGS]
		and FAT_FILE_ACCESS_OVERWRITE
		beq OpenByEntryStart
		; FreeDirectoryEntry(&msFile.sDirectoryEntry), then no size, position or cluster.
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__FreeDirectoryEntry
		ldx 8
		jsr Drop
		cmp FAT_SUCCESS
		bne OpenByEntryFailed
		jsr ClearPosition
		lda FAT_SUCCESS
		rts
OpenByEntryStart:
		lda 0
		ldy SFatFile_uiFilePosition
		sta [[D + F_THIS]:24 + Y]
		iny
		iny
		sta [[D + F_THIS]:24 + Y]
		lda FAT_SUCCESS
		rts
OpenByEntryFailed:
		pha
		sep m
		lda 0
		ldy SFatFile_uiMagic
		sta [[D + F_THIS]:24 + Y]
		rep m
		pla
		rts


; msFile.uiFileSize, uiFilePosition and uiFilePositionCluster = 0 (they follow each other).
ClearPosition:
		.width (A, X), 16
		lda 0
		ldy SFatFile_uiFileSize
ClearPositionWord:
		sta [[D + F_THIS]:24 + Y]
		iny
		iny
		cpy SFatFile_uiFilePositionCluster + 4
		bne ClearPositionWord
		rts


; uint32 CFatFile::CalculateFirstCluster(void)
		.global _CFatFile__CalculateFirstCluster
_CFatFile__CalculateFirstCluster:
		.width (A, X), 16
		jsr Enter
		jsr FirstCluster
		lda [D + F_FIRST]
		ldx [D + F_FIRST + 2]
		jmp Leave


; EFatCode CFatFile::Seek(uint32 uiOffset, EFatSeek eMode)
		.global _CFatFile__Seek
_CFatFile__Seek:
		.width (A, X), 16
		jsr Enter
		jsr CheckHandle
		bcc SeekMode
		jmp Leave
SeekMode:
		lda [D + F_ARGUMENT + 4]
		cmp FAT_SEEK_START
		beq SeekStart
		cmp FAT_SEEK_CURRENT
		beq SeekCurrent
		cmp FAT_SEEK_END
		beq SeekEnd
		lda FAT_SEEK_FAILED
		jmp Leave
SeekStart:
		lda [D + F_ARGUMENT]
		sta [D + F_POSITION]
		lda [D + F_ARGUMENT + 2]
		sta [D + F_POSITION + 2]
		bra SeekPosition
SeekCurrent:
		ldy SFatFile_uiFilePosition
		lda [[D + F_THIS]:24 + Y]
		clc
		adc [D + F_ARGUMENT]
		sta [D + F_POSITION]
		iny
		iny
		lda [[D + F_THIS]:24 + Y]
		adc [D + F_ARGUMENT + 2]
		sta [D + F_POSITION + 2]
		bra SeekPosition
SeekEnd:
		ldy SFatFile_uiFileSize
		lda [[D + F_THIS]:24 + Y]
		sec
		sbc [D + F_ARGUMENT]
		sta [D + F_POSITION]
		iny
		iny
		lda [[D + F_THIS]:24 + Y]
		sbc [D + F_ARGUMENT + 2]
		sta [D + F_POSITION + 2]
SeekPosition:
		jsr SeekTo
		jmp Leave


; Seek to F_POSITION (not past the end): the position's cluster found from the first.  A the result.
SeekTo:
		.width (A, X), 16
		ldy SFatFile_uiFileSize + 2
		lda [[D + F_THIS]:24 + Y]
		cmp [D + F_POSITION + 2]
		bcc SeekToFailed
		bne SeekToInside
		dey
		dey
		lda [[D + F_THIS]:24 + Y]
		cmp [D + F_POSITION]
		bcs SeekToInside
SeekToFailed:
		lda FAT_SEEK_FAILED
		rts
SeekToInside:
		lda F_POSITION
		ldx F_COUNT
		jsr Copy32
		ldx F_COUNT
		jsr ShiftCluster
		; The end of a file that fills its last cluster has no cluster after it (until a write adds one): stay on the last.
		lda [D + F_COUNT]
		ora [D + F_COUNT + 2]
		beq SeekToFirst
		ldy SFatFile_uiFileSize
		lda [[D + F_THIS]:24 + Y]
		cmp [D + F_POSITION]
		bne SeekToFirst
		iny
		iny
		lda [[D + F_THIS]:24 + Y]
		cmp [D + F_POSITION + 2]
		bne SeekToFirst
		ldx F_POSITION
		jsr OffsetInCluster
		bne SeekToFirst
		jsr HasCluster
		bcs SeekToFirst
		lda [D + F_COUNT]
		bne SeekToLast
		dec [D + F_COUNT + 2]
SeekToLast:
		dec [D + F_COUNT]
SeekToFirst:
		jsr FirstCluster
		ldy SFatFile_uiFilePositionCluster
		ldx F_FIRST
		jsr SetMember32
		lda [D + F_COUNT]
		ora [D + F_COUNT + 2]
		beq SeekToDone
		; SeekByClusterCount(uiFilePositionCluster, (uint16)uiClusterIndex, &msFile.uiFilePositionCluster)
		lda SFatFile_uiFilePositionCluster
		jsr ThisAddress
		pei [D + F_ADDRESS + 2]
		pei [D + F_ADDRESS]
		pei [D + F_COUNT]
		pei [D + F_FIRST + 2]
		pei [D + F_FIRST]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__SeekByClusterCount
		ldx 14
		jsr Drop
		cmp FAT_SUCCESS
		beq SeekToDone
		lda FAT_CORRUPTED_FILE
		rts
SeekToDone:
		ldy SFatFile_uiFilePosition
		ldx F_POSITION
		jsr SetMember32
		lda FAT_SUCCESS
		rts


; HasCluster(F_COUNT): carry set if the file's chain has a cluster at index F_COUNT (counting from 0).
HasCluster:
		.width (A, X), 16
		jsr FirstCluster
		lda F_FIRST
		ldx F_CLUSTER
		jsr Copy32
		lda F_COUNT
		ldx F_INDEX
		jsr Copy32
HasClusterStep:
		lda [D + F_INDEX]
		ora [D + F_INDEX + 2]
		beq HasClusterLast
		lda [D + F_CLUSTER]
		ora [D + F_CLUSTER + 2]
		beq HasClusterNot
		jsr IsEOF
		bne HasClusterNot
		lda F_NEXT
		jsr FrameAddress
		jsr NextCluster
		bne HasClusterNot
		lda F_NEXT
		ldx F_CLUSTER
		jsr Copy32
		lda [D + F_INDEX]
		bne HasClusterCount
		dec [D + F_INDEX + 2]
HasClusterCount:
		dec [D + F_INDEX]
		bra HasClusterStep
HasClusterLast:
		lda [D + F_CLUSTER]
		ora [D + F_CLUSTER + 2]
		beq HasClusterNot
		jsr IsEOF
		bne HasClusterNot
		sec
		rts
HasClusterNot:
		clc
		rts


; EFatCode CFatFile::AllocateClusters(uint32 uiBytes)
		.global _CFatFile__AllocateClusters
_CFatFile__AllocateClusters:
		.width (A, X), 16
		jsr Enter
		jsr CheckHandle
		bcs AllocateClustersReturn
		lda F_ARGUMENT
		ldx F_BYTES
		jsr Copy32
		jsr AllocateFileClusters
AllocateClustersReturn:
		jmp Leave


; AllocateClusters(F_BYTES): the file's chain grown to hold F_BYTES bytes.  A the result.
AllocateFileClusters:
		.width (A, X), 16
		jsr SetBusy
		jsr FirstCluster
		; CounterClustersInFile: F_COUNT clusters, F_PREVIOUS the last (the end of chain marker if none).
		stz [D + F_COUNT]
		stz [D + F_COUNT + 2]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__GetEndOfClusterMarker
		ply
		ply
		sta [D + F_PREVIOUS]
		stx [D + F_PREVIOUS + 2]
		lda F_FIRST
		ldx F_CLUSTER
		jsr Copy32
		lda [D + F_CLUSTER]
		ora [D + F_CLUSTER + 2]
		beq AllocateFileCounted
AllocateFileCount:
		jsr IsEOF
		bne AllocateFileCounted
		lda F_NEXT
		jsr FrameAddress
		jsr NextCluster
		beq AllocateFileNext
		rts
AllocateFileNext:
		inc [D + F_COUNT]
		bne AllocateFileCounting
		inc [D + F_COUNT + 2]
AllocateFileCounting:
		lda F_CLUSTER
		ldx F_PREVIOUS
		jsr Copy32
		lda F_NEXT
		ldx F_CLUSTER
		jsr Copy32
		bra AllocateFileCount
AllocateFileCounted:
		; F_WORK = clusters needed: (uiBytes + cluster size - 1) >> cluster shift.
		ldy CFatVolume_muiBytesPerCluster
		lda [[D + F_VOLUME]:24 + Y]
		sec
		sbc 1
		sta [D + F_WORK]
		iny
		iny
		lda [[D + F_VOLUME]:24 + Y]
		sbc 0
		sta [D + F_WORK + 2]
		lda [D + F_WORK]
		clc
		adc [D + F_BYTES]
		sta [D + F_WORK]
		lda [D + F_WORK + 2]
		adc [D + F_BYTES + 2]
		sta [D + F_WORK + 2]
		ldx F_WORK
		jsr ShiftCluster
		; Enough already?
		lda [D + F_COUNT + 2]
		cmp [D + F_WORK + 2]
		bcc AllocateFileMore
		bne AllocateFileEnough
		lda [D + F_COUNT]
		cmp [D + F_WORK]
		bcc AllocateFileMore
AllocateFileEnough:
		lda FAT_SUCCESS
		jmp ClearBusy
AllocateFileMore:
		lda [D + F_WORK]
		sec
		sbc [D + F_COUNT]
		sta [D + F_WORK]
		lda [D + F_WORK + 2]
		sbc [D + F_COUNT + 2]
		sta [D + F_WORK + 2]
		; mpcVolume->AllocateClusters(uiClustersNeeded, uiFirstCluster, &uiNewCluster, uiPreviousCluster)
		pei [D + F_PREVIOUS + 2]
		pei [D + F_PREVIOUS]
		lda F_NEW
		jsr FrameAddress
		pei [D + F_ADDRESS + 2]
		pei [D + F_ADDRESS]
		pei [D + F_FIRST + 2]
		pei [D + F_FIRST]
		pei [D + F_WORK + 2]
		pei [D + F_WORK]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__AllocateClusters
		ldx 20
		jsr Drop
		cmp FAT_SUCCESS
		bne AllocateFileDone
		; mpcVolume->UpdateDirectoryEntry(&msFile.sDirectoryEntry, uiFirstCluster, uiNewCluster, uiClustersInFile)
		pei [D + F_COUNT + 2]
		pei [D + F_COUNT]
		pei [D + F_NEW + 2]
		pei [D + F_NEW]
		pei [D + F_FIRST + 2]
		pei [D + F_FIRST]
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__UpdateDirectoryEntry
		ldx 20
		jsr Drop
AllocateFileDone:
		jmp ClearBusy


; The position's cluster for the cluster cache: F_SECTOR its first sector, F_OFFSET the position's offset in it and
; F_MAXIMUM how much of it the file had (all of it before the file's last cluster).
ClusterPosition:
		.width (A, X), 16
		ldy SFatFile_uiFilePositionCluster + 2
		lda [[D + F_THIS]:24 + Y]
		pha
		dey
		dey
		lda [[D + F_THIS]:24 + Y]
		pha
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__CalculateFirstSectorOfCluster
		ply
		ply
		ply
		ply
		sta [D + F_SECTOR]
		stx [D + F_SECTOR + 2]
		ldy SFatFile_uiFilePosition
		ldx F_POSITION
		jsr GetMember32
		ldx F_POSITION
		jsr OffsetInCluster
		sta [D + F_OFFSET]
		ldy SFatFile_uiFileSize
		ldx F_WORK
		jsr GetMember32
		ldx F_WORK
		jsr ShiftCluster
		ldx F_POSITION
		jsr ShiftCluster
		lda [D + F_POSITION + 2]
		cmp [D + F_WORK + 2]
		bcc ClusterPositionFull
		bne ClusterPositionPart
		lda [D + F_POSITION]
		cmp [D + F_WORK]
		bcc ClusterPositionFull
ClusterPositionPart:
		ldy SFatFile_uiFileSize
		ldx F_WORK
		jsr GetMember32
		ldx F_WORK
		jsr OffsetInCluster
		sta [D + F_MAXIMUM]
		rts
ClusterPositionFull:
		ldy CFatVolume_muiBytesPerCluster
		lda [[D + F_VOLUME]:24 + Y]
		sta [D + F_MAXIMUM]
		rts


; Calls the cluster cache's Read (A 0) or Write (A not 0): (F_DATA, uiFilePositionCluster, F_SECTOR, F_OFFSET,
; &F_REMAINING, F_MAXIMUM).  F_DONE = the bytes it did, and the data pointer and position moved past them.  A its
; bool result.
CacheTransfer:
		.width (A, X), 16
		sta [D + F_POINTER]
		lda [D + F_REMAINING]
		sta [D + F_DONE]
		lda [D + F_REMAINING + 2]
		sta [D + F_DONE + 2]
		pei [D + F_MAXIMUM]
		lda F_REMAINING
		jsr FrameAddress
		pei [D + F_ADDRESS + 2]
		pei [D + F_ADDRESS]
		pei [D + F_OFFSET]
		pei [D + F_SECTOR + 2]
		pei [D + F_SECTOR]
		ldy SFatFile_uiFilePositionCluster + 2
		lda [[D + F_THIS]:24 + Y]
		pha
		dey
		dey
		lda [[D + F_THIS]:24 + Y]
		pha
		pei [D + F_DATA + 2]
		pei [D + F_DATA]
		lda CFatFile_mcCache
		jsr ThisAddress
		pei [D + F_ADDRESS + 2]
		pei [D + F_ADDRESS]
		lda [D + F_POINTER]
		beq CacheTransferRead
		jsl _CFatClusterCache__Write
		bra CacheTransferCalled
CacheTransferRead:
		jsl _CFatClusterCache__Read
CacheTransferCalled:
		ldx 24
		jsr Drop
		and 0xFF
		sta [D + F_RESULT]
		lda [D + F_DONE]
		sec
		sbc [D + F_REMAINING]
		sta [D + F_DONE]
		lda [D + F_DONE + 2]
		sbc [D + F_REMAINING + 2]
		sta [D + F_DONE + 2]
		lda [D + F_DATA]
		clc
		adc [D + F_DONE]
		sta [D + F_DATA]
		lda [D + F_DATA + 2]
		adc [D + F_DONE + 2]
		sta [D + F_DATA + 2]
		lda [D + F_TOTAL]
		clc
		adc [D + F_DONE]
		sta [D + F_TOTAL]
		lda [D + F_TOTAL + 2]
		adc [D + F_DONE + 2]
		sta [D + F_TOTAL + 2]
		ldy SFatFile_uiFilePosition
		lda [[D + F_THIS]:24 + Y]
		clc
		adc [D + F_DONE]
		sta [[D + F_THIS]:24 + Y]
		iny
		iny
		lda [[D + F_THIS]:24 + Y]
		adc [D + F_DONE + 2]
		sta [[D + F_THIS]:24 + Y]
		lda [D + F_RESULT]
		rts


; Carry set if msFile.uiFilePosition >= msFile.uiFileSize.
PositionAtEnd:
		.width (A, X), 16
		ldy SFatFile_uiFilePosition + 2
		lda [[D + F_THIS]:24 + Y]
		ldy SFatFile_uiFileSize + 2
		cmp [[D + F_THIS]:24 + Y]
		bne PositionAtEndReturn
		ldy SFatFile_uiFilePosition
		lda [[D + F_THIS]:24 + Y]
		ldy SFatFile_uiFileSize
		cmp [[D + F_THIS]:24 + Y]
PositionAtEndReturn:
		rts


; EFatCode CFatFile::Write(uint8* pvSource, lsize uiLength)
		.global _CFatFile__Write
_CFatFile__Write:
		.width (A, X), 16
		jsr Enter
		jsr CheckWritable
		bcc WriteValid
		jmp Leave
WriteValid:
		; PotentiallyAllocateClusters(uiLength): clusters for the position + uiLength bytes, then Seek to the position.
		ldy SFatFile_uiFilePosition
		ldx F_POSITION
		jsr GetMember32
		lda [D + F_POSITION]
		clc
		adc [D + F_ARGUMENT + 4]
		sta [D + F_BYTES]
		lda [D + F_POSITION + 2]
		adc [D + F_ARGUMENT + 6]
		sta [D + F_BYTES + 2]
		ldy SFatFile_uiFileSize + 2
		lda [[D + F_THIS]:24 + Y]
		cmp [D + F_BYTES + 2]
		bcc WriteAllocate
		bne WriteClusters
		dey
		dey
		lda [[D + F_THIS]:24 + Y]
		cmp [D + F_BYTES]
		bcs WriteClusters
WriteAllocate:
		jsr AllocateFileClusters
		cmp FAT_SUCCESS
		bne WriteReturn
		jsr CheckHandle
		bcs WriteReturn
		ldy SFatFile_uiFilePosition
		ldx F_POSITION
		jsr GetMember32
		jsr SeekTo
		cmp FAT_SUCCESS
		bne WriteReturn
WriteClusters:
		; WriteIntoExistingClusters(uiLength, pvSource)
		lda F_ARGUMENT + 4
		ldx F_REMAINING
		jsr Copy32
		lda F_ARGUMENT
		ldx F_DATA
		jsr Copy32
		jsr SetBusy
WriteCluster:
		jsr ClusterPosition
		lda 1
		jsr CacheTransfer
		bne WriteWritten
		lda FAT_CANNOT_WRITE_MEDIA
		bra WriteDone
WriteWritten:
		; The file grows to the position.
		jsr PositionAtEnd
		bcc WriteInside
		ldy SFatFile_uiFilePosition
		ldx F_WORK
		jsr GetMember32
		ldy SFatFile_uiFileSize
		ldx F_WORK
		jsr SetMember32
WriteInside:
		lda [D + F_REMAINING]
		ora [D + F_REMAINING + 2]
		bne WriteNext
		jsr StepToPosition
		bra WriteDone
WriteNext:
		jsr NextPositionCluster
		beq WriteCluster
WriteDone:
		jsr ClearBusy
WriteReturn:
		jmp Leave


; EFatCode CFatFile::Read(uint8* puiDestination, lsize uiLength, lsize* puiBytesRead)
		.global _CFatFile__Read
_CFatFile__Read:
		.width (A, X), 16
		jsr Enter
		jsr CheckHandle
		bcc ReadValid
		jmp Leave
ReadValid:
		; ReadFromClusters(uiLength, puiBytesRead, puiDestination)
		lda F_ARGUMENT + 4
		ldx F_REMAINING
		jsr Copy32
		lda F_ARGUMENT
		ldx F_DATA
		jsr Copy32
		stz [D + F_TOTAL]
		stz [D + F_TOTAL + 2]
		lda FAT_SUCCESS
		sta [D + F_RESULT]
		; Nothing to read: an empty file, no cluster, or at the end.
		ldy SFatFile_uiFileSize
		lda [[D + F_THIS]:24 + Y]
		iny
		iny
		ora [[D + F_THIS]:24 + Y]
		beq ReadBytesRead
		ldy SFatFile_uiFilePositionCluster
		lda [[D + F_THIS]:24 + Y]
		iny
		iny
		ora [[D + F_THIS]:24 + Y]
		beq ReadBytesRead
		jsr PositionAtEnd
		bcs ReadBytesRead
		jsr SetBusy
ReadCluster:
		jsr ClusterPosition
		lda 0
		jsr CacheTransfer
		; At (or past) the end: the position is the end and there's no more to read.
		jsr PositionAtEnd
		bcc ReadInside
		ldy SFatFile_uiFileSize
		ldx F_WORK
		jsr GetMember32
		ldy SFatFile_uiFilePosition
		ldx F_WORK
		jsr SetMember32
		stz [D + F_REMAINING]
		stz [D + F_REMAINING + 2]
ReadInside:
		lda [D + F_RESULT]
		bne ReadRead
		lda FAT_CANNOT_READ_MEDIA
		bra ReadDone
ReadRead:
		lda [D + F_REMAINING]
		ora [D + F_REMAINING + 2]
		bne ReadNext
		jsr StepToPosition
		bra ReadDone
ReadNext:
		jsr NextPositionCluster
		beq ReadCluster
ReadDone:
		jsr ClearBusy
		sta [D + F_RESULT]
ReadBytesRead:
		; *puiBytesRead = the bytes read (if puiBytesRead isn't NULL).
		lda [D + F_ARGUMENT + 8]
		ora [D + F_ARGUMENT + 10]
		beq ReadReturn
		lda [D + F_TOTAL]
		sta [[D + F_ARGUMENT + 8]:24]
		lda [D + F_TOTAL + 2]
		ldy 2
		sta [[D + F_ARGUMENT + 8]:24 + Y]
ReadReturn:
		lda [D + F_RESULT]
		jmp Leave


; EFatCode CFatFile::Flush(void)
		.global _CFatFile__Flush
_CFatFile__Flush:
		.width (A, X), 16
		jsr Enter
		jsr FlushFile
		jmp Leave


; Flush: if the file was opened for writing, its cluster cache flushed and its directory entry (size, modify and
; access dates) written.  A the result.
FlushFile:
		.width (A, X), 16
		jsr CheckHandle
		bcc FlushValid
		rts
FlushValid:
		ldy SFatFile_uiAccessFlags
		lda [[D + F_THIS]:24 + Y]
		and FAT_FILE_ACCESS_WRITE
		bne FlushCacheUpdateAccess
		lda FAT_SUCCESS
		rts
FlushCacheUpdateAccess:
		jsr SetBusy
		jsr CacheFlush
		ldy SFatFile_uiFileSize
		ldx F_WORK
		jsr GetMember32
		ldy RAW + RAW_uiSize
		ldx F_WORK
		jsr SetMember32
		lda RAW
		jsr ThisAddress
		pei [D + F_ADDRESS + 2]
		pei [D + F_ADDRESS]
		jsl _FatSetModifyDateTime__FPC21SFatRawDirectoryEntry
		ply
		ply
		ldy RAW + RAW_uiModifyDate
		lda [[D + F_THIS]:24 + Y]
		ldy RAW + RAW_uiAccessDate
		sta [[D + F_THIS]:24 + Y]
		; F_CACHE = mpcVolume->ReadSector(msFile.sDirectoryEntry.uiSectorAddress)
		pea 0
		ldy SFatFile_sDirectoryEntry + SFatDirectoryEntry_uiSectorAddress + 2
		lda [[D + F_THIS]:24 + Y]
		pha
		dey
		dey
		lda [[D + F_THIS]:24 + Y]
		pha
		lda F_CACHE
		jsr FrameAddress
		pei [D + F_ADDRESS + 2]
		pei [D + F_ADDRESS]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__ReadSector
		ldx 14
		jsr Drop
		lda [D + F_CACHE + SFatCache_pvCachedSector]
		ora [D + F_CACHE + SFatCache_pvCachedSector + 2]
		bne FlushRead
		lda FAT_CANNOT_READ_MEDIA
		jmp ClearBusy
FlushRead:
		; The raw entry copied into the sector at uiSectorOffset.
		ldy SFatFile_sDirectoryEntry + SFatDirectoryEntry_uiSectorOffset
		lda [[D + F_THIS]:24 + Y]
		clc
		adc [D + F_CACHE + SFatCache_pvCachedSector]
		sta [D + F_POINTER]
		lda [D + F_CACHE + SFatCache_pvCachedSector + 2]
		adc 0
		sta [D + F_POINTER + 2]
		lda RAW
		jsr ThisAddress
		ldy SIZEOF_SFatRawDirectoryEntry - 2
FlushCopy:
		lda [[D + F_ADDRESS]:24 + Y]
		sta [[D + F_POINTER]:24 + Y]
		dey
		dey
		bpl FlushCopy
		; mpcVolume->DirtySector(F_CACHE): the SFatCache passed by value.
		pei [D + F_CACHE + 6]
		pei [D + F_CACHE + 4]
		pei [D + F_CACHE + 2]
		pei [D + F_CACHE]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__DirtySector
		ldx 12
		jsr Drop
		lda FAT_SUCCESS
		jmp ClearBusy


; EFatCode CFatFile::Truncate(void): the file ends at its position.  (An error leaves the file busy, as the C++.)
		.global _CFatFile__Truncate
_CFatFile__Truncate:
		.width (A, X), 16
		jsr Enter
		ldy SFatFile_uiAccessFlags
		lda [[D + F_THIS]:24 + Y]
		and FAT_FILE_ACCESS_WRITE
		bne TruncateWritable
		lda FAT_FILE_NOT_OPENED_FOR_WRITE_ACCESS
		jmp Leave
TruncateWritable:
		jsr SetBusy
		ldy SFatFile_uiFilePosition
		ldx F_POSITION
		jsr GetMember32
		lda [D + F_POSITION]
		ora [D + F_POSITION + 2]
		bne TruncatePosition
		jmp TruncateAll
TruncatePosition:
		; The last cluster kept (F_PREVIOUS) holds the byte before the position (on a cluster boundary the position's
		; cluster goes too).
		ldy SFatFile_uiFilePositionCluster
		ldx F_PREVIOUS
		jsr GetMember32
		ldx F_POSITION
		jsr OffsetInCluster
		bne TruncateLast
		jsr FirstCluster
		lda F_FIRST
		ldx F_PREVIOUS
		jsr Copy32
		; Past the first cluster: SeekByClusterCount(uiLastCluster, BytesToClusters(position) - 1, &uiLastCluster).
		ldy CFatVolume_muiBytesPerCluster + 2
		lda [[D + F_VOLUME]:24 + Y]
		cmp [D + F_POSITION + 2]
		bcc TruncateSeek
		bne TruncateSought
		dey
		dey
		lda [[D + F_VOLUME]:24 + Y]
		cmp [D + F_POSITION]
		bcs TruncateSought
TruncateSeek:
		lda F_POSITION
		ldx F_WORK
		jsr Copy32
		ldx F_WORK
		jsr ShiftCluster
		lda [D + F_WORK]
		dec A
		sta [D + F_WORK]
		lda F_PREVIOUS
		jsr FrameAddress
		pei [D + F_ADDRESS + 2]
		pei [D + F_ADDRESS]
		pei [D + F_WORK]
		pei [D + F_PREVIOUS + 2]
		pei [D + F_PREVIOUS]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__SeekByClusterCount
		ldx 14
		jsr Drop
		cmp FAT_SUCCESS
		beq TruncateSought
		jmp Leave
TruncateSought:
		ldy SFatFile_uiFilePositionCluster
		ldx F_PREVIOUS
		jsr SetMember32
TruncateLast:
		; F_CLUSTER = the cluster after the last kept.
		lda F_PREVIOUS
		ldx F_CLUSTER
		jsr Copy32
		lda F_NEXT
		jsr FrameAddress
		jsr NextCluster
		beq TruncateNext
		jmp Leave
TruncateNext:
		lda F_NEXT
		ldx F_CLUSTER
		jsr Copy32
		ldy SFatFile_uiFileSize
		ldx F_POSITION
		jsr SetMember32
		jsr IsEOF
		bne TruncateDone
		; FreeChain(uiFatEntry), then the last kept cluster ends the chain.
		pei [D + F_CLUSTER + 2]
		pei [D + F_CLUSTER]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__FreeChain
		ldx 8
		jsr Drop
		cmp FAT_SUCCESS
		beq TruncateFreed
		jmp Leave
TruncateFreed:
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__GetEndOfClusterMarker
		ply
		ply
		phx
		pha
		pei [D + F_PREVIOUS + 2]
		pei [D + F_PREVIOUS]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__SetClusterEntry
		ldx 12
		jsr Drop
		cmp FAT_SUCCESS
		beq TruncateDone
		jmp Leave
TruncateAll:
		pei [D + F_THIS + 2]
		pei [D + F_THIS]
		pei [D + F_VOLUME + 2]
		pei [D + F_VOLUME]
		jsl _CFatVolume__FreeDirectoryEntry
		ldx 8
		jsr Drop
		cmp FAT_SUCCESS
		beq TruncateCleared
		jmp Leave
TruncateCleared:
		jsr ClearPosition
TruncateDone:
		lda FAT_SUCCESS
		jsr ClearBusy
		jmp Leave


; EFatCode CFatFile::Close(void)
		.global _CFatFile__Close
_CFatFile__Close:
		.width (A, X), 16
		jsr Enter
		ldy SFatFile_uiMagic
		lda [[D + F_THIS]:24 + Y]
		and 0xFF
		cmp FAT_OPEN_HANDLE_MAGIC
		beq CloseOpen
		lda FAT_INVALID_HANDLE
		bra CloseKill
CloseOpen:
		ldy SFatFile_bBusy
		lda [[D + F_THIS]:24 + Y]
		and 0xFF
		beq CloseNotBusy
		lda FAT_FILE_HANDLE_IN_USE
		jmp Leave
CloseNotBusy:
		jsr FlushFile
		cmp FAT_SUCCESS
		bne CloseKill
		jsr CacheFlush
		sep m
		lda 0
		ldy SFatFile_uiMagic
		sta [[D + F_THIS]:24 + Y]
		rep m
		lda FAT_SUCCESS
CloseKill:
		jsr CacheKill
		jmp Leave


; Getters: this's member at Y in A (and X).
		.global _CFatFile__Size
_CFatFile__Size:
		.width (A, X), 16
		ldy SFatFile_uiFileSize
		bra Getter32
		.global _CFatFile__Tell
_CFatFile__Tell:
		.width (A, X), 16
		ldy SFatFile_uiFilePosition
Getter32:
		phd
		tsc
		tcd
		iny
		iny
		lda [[D + 6]:24 + Y]
		tax
		dey
		dey
		lda [[D + 6]:24 + Y]
		pld
		rtl
		.global _CFatFile__IsBusy
_CFatFile__IsBusy:
		.width (A, X), 16
		ldy SFatFile_bBusy
		bra Getter8
		.global _CFatFile__GetAccessFlags
_CFatFile__GetAccessFlags:
		.width (A, X), 16
		ldy SFatFile_uiAccessFlags
Getter8:
		phd
		tsc
		tcd
		lda [[D + 6]:24 + Y]
		and 0xFF
		pld
		rtl
