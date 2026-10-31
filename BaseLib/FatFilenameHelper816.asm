; FatFilenameHelper.cpp for the W65C816, with the compiler's calling convention (see FatInfoSectorCache816.asm): the
; same functions with the same names.  Characters are chars (signed, as the compiler's): one with its top bit set
; widens to 0xFFxx and is never upper or lower case.
;
; Every function here has the same frame (F_FRAME bytes); its arguments start at F_ARGUMENT (pointers take 4 bytes,
; uint16s, bools and chars 2).

		.include "Fat816.inc"

F_STRING		= 1				; StringLength's and FindDot's string.
F_DEST			= 5				; PutByte's and PutWord's destination.
F_WORK			= 9
F_NAME			= 13			; 14 bytes: GetFatShortNameForEntry's szName[13].
F_LENGTH		= 27
F_DOT			= 29
F_C				= 31
F_LFN			= 33
F_UPPER			= 35
F_FLAG			= 37			; Lower's flag.
F_DIGITS		= 39			; 6 bytes: GenerateShortNameWithSuffix's digits, last first.
F_FRAME			= 46
F_ARGUMENT		= F_FRAME + 6

NOT_FOUND		= 0xFFFF		; ARRAY_ELEMENT_NOT_FOUND (MAX_SIZE).
BACKSLASH		= 0x5C


		.section text.FatFilenameHelper816
		.relax


; The frame (F_FRAME bytes) for a function called with jsl: jsr'd straight after its label.  The return address (the
; jsr's 2 bytes) is moved below the frame.
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


; A = the length of the string at F_STRING (also in Y).
StringLength:
		.width (A, X), 16
		ldy 0
		sep m
StringLengthChar:
		lda [[D + F_STRING]:24 + Y]
		beq StringLengthEnd
		iny
		bra StringLengthChar
StringLengthEnd:
		rep m
		tya
		rts


; A = the index of the first '.' in the string at F_STRING at or after index Y, NOT_FOUND if none (only whether it
; is found matters after index 0: FindCharIndex's index counts from where it starts).
FindDot:
		.width (A, X), 16
		sep m
FindDotChar:
		lda [[D + F_STRING]:24 + Y]
		beq FindDotNone
		cmp '.'
		beq FindDotFound
		iny
		bra FindDotChar
FindDotFound:
		rep m
		tya
		rts
FindDotNone:
		.width A, 8
		rep m
		lda NOT_FOUND
		rts


; toupper: A (a character, 16 bit) upper case.
Upper:
		.width (A, X), 16
		cmp 'a'
		bcc UpperReturn
		cmp 'z' + 1
		bcs UpperReturn
		and 0xFFDF
UpperReturn:
		rts


; tolower if F_FLAG: A (a character, 16 bit) lower case, then widened as a char.
Lower:
		.width (A, X), 16
		pha
		lda [D + F_FLAG]
		and 0xFF
		beq LowerKept
		pla
		cmp 'A'
		bcc Widen
		cmp 'Z' + 1
		bcs Widen
		ora 0x20
		rts
LowerKept:
		pla
; A (a char, 16 bit) widened: 0xFFxx if its top bit is set.
Widen:
		cmp 0x80
		bcc WidenReturn
		ora 0xFF00
WidenReturn:
		rts


; *F_DEST++ = A (a byte: m 8 bit).
PutByte:
		.width A, 8
		sta [[D + F_DEST]:24]
		rep m
		inc [D + F_DEST]
		bne PutByteDone
		inc [D + F_DEST + 2]
PutByteDone:
		sep m
		rts


; *F_DEST++ = A (a word).
PutWord:
		.width (A, X), 16
		sta [[D + F_DEST]:24]
		lda [D + F_DEST]
		clc
		adc 2
		sta [D + F_DEST]
		bcc PutWordDone
		inc [D + F_DEST + 2]
PutWordDone:
		rts


; Copies the frame's 32 bit D + A to D + X.
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


; EFatCode GetFatLongNameForEntry(uint16* puiDest, char* szSource)
GL_DEST			= F_ARGUMENT
GL_SOURCE		= F_ARGUMENT + 4
		.global _GetFatLongNameForEntry__FPtPc
_GetFatLongNameForEntry__FPtPc:
		.width (A, X), 16
		jsr Enter
		lda GL_DEST
		ldx F_DEST
		jsr Copy32
		lda GL_SOURCE
		ldx F_STRING
		jsr Copy32
		jsr LongName
		lda FAT_SUCCESS
		jmp Leave


; The string at F_STRING to the words at F_DEST, each char widened, 0 ended.
LongName:
		.width (A, X), 16
		ldy 0
LongNameChar:
		lda [[D + F_STRING]:24 + Y]
		and 0xFF
		jsr Widen
		tax
		jsr PutWord
		cpx 0
		beq LongNameDone
		iny
		bra LongNameChar
LongNameDone:
		rts


; bool CompareFatShortName(char* szName1, char* szName2): the 11 bytes the same.
CS_NAME1		= F_ARGUMENT
CS_NAME2		= F_ARGUMENT + 4
		.global _CompareFatShortName__FPcPc
_CompareFatShortName__FPcPc:
		.width (A, X), 16
		jsr Enter
		ldy 0
		sep m
CompareShortChar:
		lda [[D + CS_NAME1]:24 + Y]
		cmp [[D + CS_NAME2]:24 + Y]
		bne CompareShortDifferent
		iny
		cpy 11
		bne CompareShortChar
		rep m
		lda 1
		jmp Leave
CompareShortDifferent:
		.width A, 8
		rep m
		lda 0
		jmp Leave


; int CompareFatLongName(uint16* puiName1, uint16* puiName2): 1 if the names (each character's low byte) are the same
; but for case, up to their 0 (or FAT_MAX_FILENAME + 1 characters).
CL_NAME1		= F_ARGUMENT
CL_NAME2		= F_ARGUMENT + 4
		.global _CompareFatLongName__FPtPt
_CompareFatLongName__FPtPt:
		.width (A, X), 16
		jsr Enter
		ldy 0
CompareLongChar:
		lda [[D + CL_NAME1]:24 + Y]
		and 0xFF
		jsr Upper
		sta [D + F_C]
		lda [[D + CL_NAME2]:24 + Y]
		and 0xFF
		jsr Upper
		cmp [D + F_C]
		bne CompareLongDifferent
		cmp 0
		beq CompareLongSame
		iny
		iny
		cpy (FAT_MAX_FILENAME + 1) * 2
		bne CompareLongChar
CompareLongSame:
		lda 1
		jmp Leave
CompareLongDifferent:
		lda 0
		jmp Leave


; EFatCode MatchesFatFileName(bool* pbMatch, bool* pbUsingLFN, char* szConstructedShortFileName, uint16*
; puiTargetFileLongName, char* szCurrentLevelPath, uint16* auiLongFilename, uint8* auiShortName)
MF_MATCH		= F_ARGUMENT
MF_USING_LFN	= F_ARGUMENT + 4
MF_SHORT		= F_ARGUMENT + 8
MF_TARGET		= F_ARGUMENT + 12
MF_PATH			= F_ARGUMENT + 16
MF_LONG			= F_ARGUMENT + 20
MF_ENTRY_SHORT	= F_ARGUMENT + 24
		.global _MatchesFatFileName__FPbPbPcPtPcPtPh
_MatchesFatFileName__FPbPbPcPtPcPtPh:
		.width (A, X), 16
		jsr Enter
		stz [D + F_LFN]
		; GetFatShortNameForEntry(szConstructedShortFileName, szCurrentLevelPath, 1)
		pea 1
		pei [D + MF_PATH + 2]
		pei [D + MF_PATH]
		pei [D + MF_SHORT + 2]
		pei [D + MF_SHORT]
		jsl _GetFatShortNameForEntry__FPcPcb
		ply
		ply
		ply
		ply
		ply
		cmp FAT_INVALID_FILENAME
		bne MatchesShort
		; A long name: GetFatLongNameForEntry(puiTargetFileLongName, szCurrentLevelPath).
		inc [D + F_LFN]
		lda MF_TARGET
		ldx F_DEST
		jsr Copy32
		lda MF_PATH
		ldx F_STRING
		jsr Copy32
		jsr LongName
		; No short name was made: one of zeros never matches an entry (a zero first character ends the directory).
		lda 0
		ldy 9
MatchesClear:
		sta [[D + MF_SHORT]:24 + Y]
		dey
		bpl MatchesClear
		; CompareFatLongName(puiTargetFileLongName, auiLongFilename) || CompareFatShortName(...)
		pei [D + MF_LONG + 2]
		pei [D + MF_LONG]
		pei [D + MF_TARGET + 2]
		pei [D + MF_TARGET]
		jsl _CompareFatLongName__FPtPt
		ply
		ply
		ply
		ply
		cmp 0
		bne MatchesResult
MatchesShort:
		pei [D + MF_ENTRY_SHORT + 2]
		pei [D + MF_ENTRY_SHORT]
		pei [D + MF_SHORT + 2]
		pei [D + MF_SHORT]
		jsl _CompareFatShortName__FPcPc
		ply
		ply
		ply
		ply
MatchesResult:
		sep m
		sta [[D + MF_MATCH]:24]
		lda [D + F_LFN]
		sta [[D + MF_USING_LFN]:24]
		rep m
		lda FAT_SUCCESS
		jmp Leave


; EFatCode GetFatShortNameForEntry(char* szDest, const char* szSource, bool bLFNDisabled): the 11 character short
; name (FAT_LFN_GENERATED if it was made from a long name).
SN_DEST			= F_ARGUMENT
SN_SOURCE		= F_ARGUMENT + 4
SN_DISABLED		= F_ARGUMENT + 8
		.global _GetFatShortNameForEntry__FPcPcb
_GetFatShortNameForEntry__FPcPcb:
		.width (A, X), 16
		jsr Enter
		stz [D + F_UPPER]
		lda [D + SN_DISABLED]
		and 0xFF
		beq ShortCheckLong
		jmp ShortStrip
ShortCheckLong:
		; A long name if: no dot and longer than 8, the dot first, after 8 or more than 3 after it, two dots, a
		; space or a lower case character.
		lda SN_SOURCE
		ldx F_STRING
		jsr Copy32
		jsr StringLength
		sta [D + F_LENGTH]
		ldy 0
		jsr FindDot
		sta [D + F_DOT]
		cmp NOT_FOUND
		bne ShortHasDot
		lda [D + F_LENGTH]
		cmp 9
		bcs ShortGenerate
		bra ShortCase
ShortHasDot:
		cmp 0
		beq ShortGenerate
		cmp 9
		bcs ShortGenerate
		lda [D + F_LENGTH]
		sec
		sbc [D + F_DOT]
		cmp 5
		bcs ShortGenerate
		ldy [D + F_DOT]
		iny
		jsr FindDot
		cmp NOT_FOUND
		bne ShortGenerate
ShortCase:
		ldy 0
ShortCaseChar:
		cpy [D + F_LENGTH]
		bne ShortCaseNext
		jmp ShortStrip
ShortCaseNext:
		lda [[D + SN_SOURCE]:24 + Y]
		and 0xFF
		cmp ' '
		beq ShortGenerate
		cmp 'a'
		bcc ShortCaseUpper
		cmp 'z' + 1
		bcc ShortGenerate
ShortCaseUpper:
		iny
		bra ShortCaseChar

ShortGenerate:
		; The last dot (not the first character), else the length.
		lda [D + F_LENGTH]
		sta [D + F_DOT]
		tay
ShortGenerateDot:
		dey
		beq ShortGenerateBase
		lda [[D + SN_SOURCE]:24 + Y]
		and 0xFF
		cmp '.'
		bne ShortGenerateDot
		sty [D + F_DOT]
ShortGenerateBase:
		; 8 characters before the dot, spaces and dots skipped (c is a uint8).
		stz [D + F_C]
		ldx 0
ShortBaseSkip:
		lda [D + F_C]
		cmp [D + F_DOT]
		bcs ShortBaseSpace
		tay
		lda [[D + SN_SOURCE]:24 + Y]
		and 0xFF
		cmp ' '
		beq ShortBaseNext
		cmp '.'
		bne ShortBaseCharacter
ShortBaseNext:
		jsr ShortNextC
		bra ShortBaseSkip
ShortBaseCharacter:
		jsr Upper
		sta [D + F_NAME + X]
		jsr ShortNextC
		bra ShortBaseStored
ShortBaseSpace:
		lda ' '
		sta [D + F_NAME + X]
ShortBaseStored:
		inx
		cpx 8
		bne ShortBaseSkip
		; 3 after the dot.
		lda [D + F_DOT]
		inc A
		and 0xFF
		sta [D + F_C]
ShortExtensionSkip:
		lda [D + F_C]
		cmp [D + F_LENGTH]
		bcs ShortExtensionSpace
		tay
		lda [[D + SN_SOURCE]:24 + Y]
		and 0xFF
		cmp ' '
		beq ShortExtensionNext
		cmp '.'
		bne ShortExtensionCharacter
ShortExtensionNext:
		jsr ShortNextC
		bra ShortExtensionSkip
ShortExtensionCharacter:
		jsr Upper
		sta [D + F_NAME + X]
		jsr ShortNextC
		bra ShortExtensionStored
ShortExtensionSpace:
		lda ' '
		sta [D + F_NAME + X]
ShortExtensionStored:
		inx
		cpx 11
		bne ShortExtensionSkip
		ldx 0
ShortGenerateCopy:
		txy
		lda [D + F_NAME + X]
		sep m
		sta [[D + SN_DEST]:24 + Y]
		rep m
		inx
		cpx 11
		bne ShortGenerateCopy
		lda FAT_LFN_GENERATED
		jmp Leave

ShortStrip:
		; StrCpySafeStripSurroundingSpaces(szName, szSource, 12)
		pea 12
		pei [D + SN_SOURCE + 2]
		pei [D + SN_SOURCE]
		pea 0
		tdc
		clc
		adc F_NAME
		sta [D + F_STRING]
		stz [D + F_STRING + 2]
		pha
		jsl _StrCpySafeStripSurroundingSpaces__FPcPct
		ply
		ply
		ply
		ply
		ply
		jsr StringLength
		sta [D + F_LENGTH]
		cmp 0
		beq ShortInvalid
		cmp 13
		bcs ShortInvalid
		ldy 0
		jsr FindDot
		sta [D + F_DOT]
		cmp NOT_FOUND
		bne ShortStripDot
		lda [D + F_LENGTH]
		cmp 9
		bcs ShortInvalid
		bra ShortStripBase
ShortStripDot:
		cmp 0
		beq ShortInvalid
		cmp 9
		bcs ShortInvalid
		lda [D + F_LENGTH]
		clc
		sbc [D + F_DOT]
		cmp 4
		bcs ShortInvalid
		ldy [D + F_DOT]
		iny
		jsr FindDot
		cmp NOT_FOUND
		beq ShortStripBase
ShortInvalid:
		lda FAT_INVALID_FILENAME
		jmp Leave
ShortStripBase:
		lda SN_DEST
		ldx F_DEST
		jsr Copy32
		ldx 0
ShortStripBaseCharacter:
		cpx [D + F_LENGTH]
		bcs ShortStripBaseSpace
		cpx [D + F_DOT]
		bcs ShortStripBaseSpace
		lda [D + F_NAME + X]
		jsr ShortPut
		bra ShortStripBaseNext
ShortStripBaseSpace:
		lda ' '
		jsr ShortPut
ShortStripBaseNext:
		inx
		cpx 8
		bne ShortStripBaseCharacter
		ldx 0
ShortStripExtension:
		lda [D + F_DOT]
		cmp NOT_FOUND
		beq ShortStripExtensionSpace
		txa
		sec
		adc [D + F_DOT]
		cmp [D + F_LENGTH]
		bcs ShortStripExtensionSpace
		phx
		tax
		lda [D + F_NAME + X]
		plx
		jsr ShortPut
		bra ShortStripExtensionNext
ShortStripExtensionSpace:
		lda ' '
		jsr ShortPut
ShortStripExtensionNext:
		inx
		cpx 3
		bne ShortStripExtension
		lda [D + F_UPPER]
		bne ShortInvalid
		lda FAT_SUCCESS
		jmp Leave


; c = (uint8)(c + 1).
ShortNextC:
		.width (A, X), 16
		lda [D + F_C]
		inc A
		and 0xFF
		sta [D + F_C]
		rts


; *szDest++ = toupper(A's low byte); a lower case character when bLFNDisabled sets F_UPPER.
ShortPut:
		.width (A, X), 16
		and 0xFF
		cmp 'a'
		bcc ShortPutByte
		cmp 'z' + 1
		bcs ShortPutByte
		pha
		lda [D + SN_DISABLED]
		and 0xFF
		beq ShortPutUpper
		sta [D + F_UPPER]
ShortPutUpper:
		pla
		and 0xFFDF
ShortPutByte:
		sep m
		jsr PutByte
		rep m
		rts


; void ConvertFATShortInternalNameInto8Dot3Format(uint8* puiDest, const uint8* puiSource): "NAME    EXT" to
; "NAME.EXT".
CF_DEST			= F_ARGUMENT
CF_SOURCE		= F_ARGUMENT + 4
		.global _ConvertFATShortInternalNameInto8Dot3Format__FPhPh
_ConvertFATShortInternalNameInto8Dot3Format__FPhPh:
		.width (A, X), 16
		jsr Enter
		lda CF_DEST
		ldx F_DEST
		jsr Copy32
		sep m
		lda [[D + CF_SOURCE]:24]
		cmp FAT_KANJI_SKIP_ENTRY
		bne ConvertFirst
		lda FAT_DELETED_ENTRY
ConvertFirst:
		jsr PutByte
		ldy 1
ConvertBase:
		lda [[D + CF_SOURCE]:24 + Y]
		cmp ' '
		beq ConvertExtension
		jsr PutByte
		iny
		cpy 8
		bne ConvertBase
ConvertExtension:
		ldy 8
		lda [[D + CF_SOURCE]:24 + Y]
		cmp ' '
		beq ConvertEnd
		lda '.'
		jsr PutByte
ConvertExtensionCharacter:
		lda [[D + CF_SOURCE]:24 + Y]
		cmp ' '
		beq ConvertEnd
		jsr PutByte
		iny
		cpy 11
		bne ConvertExtensionCharacter
ConvertEnd:
		lda 0
		sta [[D + F_DEST]:24]
		rep m
		jmp Leave


; void ConstructFatLongFileNameFromShortName(uint16* auiLongFilename, char* szShortName, bool bLowercaseBase, bool
; bLowercaseExtension)
CN_LONG			= F_ARGUMENT
CN_SHORT		= F_ARGUMENT + 4
CN_LOWER_BASE	= F_ARGUMENT + 8
CN_LOWER_EXT	= F_ARGUMENT + 10
		.global _ConstructFatLongFileNameFromShortName__FPtPcbb
_ConstructFatLongFileNameFromShortName__FPtPcbb:
		.width (A, X), 16
		jsr Enter
		lda CN_LONG
		ldx F_DEST
		jsr Copy32
		lda [D + CN_LOWER_BASE]
		sta [D + F_FLAG]
		ldy 0
ConstructBase:
		lda [[D + CN_SHORT]:24 + Y]
		and 0xFF
		cmp ' '
		beq ConstructBaseNext
		jsr Lower
		jsr PutWord
ConstructBaseNext:
		iny
		cpy 8
		bne ConstructBase
		lda [[D + CN_SHORT]:24 + Y]
		and 0xFF
		cmp ' '
		beq ConstructEnd
		lda '.'
		jsr PutWord
		lda [D + CN_LOWER_EXT]
		sta [D + F_FLAG]
ConstructExtension:
		lda [[D + CN_SHORT]:24 + Y]
		and 0xFF
		cmp ' '
		beq ConstructExtensionNext
		jsr Lower
		jsr PutWord
ConstructExtensionNext:
		iny
		cpy 11
		bne ConstructExtension
ConstructEnd:
		lda 0
		sta [[D + F_DEST]:24]
		jmp Leave


; const char* StepPathOverSlash(const char* szPathName)
		.global _StepPathOverSlash__FPc
_StepPathOverSlash__FPc:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT]
		ldx [D + F_ARGUMENT + 2]
		sta [D + F_WORK]
		ora [D + F_ARGUMENT + 2]
		beq StepOverReturn
		lda [[D + F_ARGUMENT]:24]
		and 0xFF
		cmp BACKSLASH
		bne StepOverKept
		lda [D + F_WORK]
		inc A
		bne StepOverReturn
		inx
		bra StepOverReturn
StepOverKept:
		lda [D + F_WORK]
StepOverReturn:
		jmp Leave


; const char* StepPathToSlash(const char* szPathName): back to the \ before it.
		.global _StepPathToSlash__FPc
_StepPathToSlash__FPc:
		.width (A, X), 16
		jsr Enter
StepToBack:
		lda [D + F_ARGUMENT]
		bne StepToLow
		dec [D + F_ARGUMENT + 2]
StepToLow:
		dec [D + F_ARGUMENT]
		lda [[D + F_ARGUMENT]:24]
		and 0xFF
		cmp BACKSLASH
		bne StepToBack
		lda [D + F_ARGUMENT]
		ldx [D + F_ARGUMENT + 2]
		jmp Leave


; void CopyLongFilenameIntoString(char* szDestination, uint16* auiSource): each character's low byte, if neither is
; empty.
CO_DEST			= F_ARGUMENT
CO_SOURCE		= F_ARGUMENT + 4
		.global _CopyLongFilenameIntoString__FPcPt
_CopyLongFilenameIntoString__FPcPt:
		.width (A, X), 16
		jsr Enter
		lda [[D + CO_DEST]:24]
		and 0xFF
		beq CopyLongDone
		lda [[D + CO_SOURCE]:24]
		beq CopyLongDone
		lda CO_DEST
		ldx F_DEST
		jsr Copy32
		ldy 0
CopyLongCharacter:
		lda [[D + CO_SOURCE]:24 + Y]
		tax
		sep m
		jsr PutByte
		rep m
		txa
		beq CopyLongDone
		iny
		iny
		cpy 512
		bne CopyLongCharacter
CopyLongDone:
		jmp Leave


; bool IsIllegalFilenameCharacter(char c)
		.global _IsIllegalFilenameCharacter__Fc
_IsIllegalFilenameCharacter__Fc:
		.width (A, X), 16
		jsr Enter
		lda [D + F_ARGUMENT]
		jsr IllegalCharacter
		lda 0
		rol A
		jmp Leave


; Carry set if A's low byte (a char) is illegal in a file name: a control character (or one with its top bit set:
; a negative char) or one of ILLEGAL_CHARS but '.'.
IllegalCharacter:
		.width (A, X), 16
		sep m
		cmp 0x20
		bcc IllegalCharacterYes
		cmp 0x80
		bcs IllegalCharacterYes
		cmp '.'
		beq IllegalCharacterNo
		ldx IllegalCharactersEnd - IllegalCharacters - 1
IllegalCharacterCompare:
		cmp [IllegalCharacters + X]:24
		beq IllegalCharacterYes
		dex
		bpl IllegalCharacterCompare
IllegalCharacterNo:
		rep m
		clc
		rts
IllegalCharacterYes:
		.width A, 8
		rep m
		sec
		rts
IllegalCharacters:
		.byte 0x22, 0x2A, 0x2B, 0x2C, 0x2E, 0x2F, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x5B, 0x5C, 0x5D, 0x7C
IllegalCharactersEnd:


; bool IsIllegalFilename(const char* szName, uint16 uiLength)
II_NAME			= F_ARGUMENT
II_LENGTH		= F_ARGUMENT + 4
		.global _IsIllegalFilename__FPct
_IsIllegalFilename__FPct:
		.width (A, X), 16
		jsr Enter
		lda II_NAME
		ldx F_STRING
		jsr Copy32
		ldy 0
		jsr FindDot
		cmp 0
		beq IllegalNameYes
		sta [D + F_WORK]
		lda [D + II_LENGTH]
		dec A
		cmp [D + F_WORK]
		beq IllegalNameYes
		ldy 0
IllegalNameCharacter:
		cpy [D + II_LENGTH]
		beq IllegalNameNo
		lda [[D + II_NAME]:24 + Y]
		jsr IllegalCharacter
		bcs IllegalNameYes
		iny
		bra IllegalNameCharacter
IllegalNameNo:
		lda 0
		jmp Leave
IllegalNameYes:
		lda 1
		jmp Leave


; void GenerateShortNameWithSuffix(uint16 uiNameSuffix, uint8* szShortName): "~" and the number after the name's
; characters (before its first space), within its 8.
GS_SUFFIX		= F_ARGUMENT
GS_NAME			= F_ARGUMENT + 2
		.global _GenerateShortNameWithSuffix__FtPh
_GenerateShortNameWithSuffix__FtPh:
		.width (A, X), 16
		jsr Enter
		; The digits, last first (F_LENGTH of them).
		stz [D + F_LENGTH]
		lda [D + GS_SUFFIX]
		sta [D + F_WORK]
GenerateDigit:
		; F_WORK /= 10, A the remainder.
		lda 0
		ldx 16
GenerateDivide:
		asl [D + F_WORK]
		rol A
		cmp 10
		bcc GenerateDivideNext
		sbc 10
		inc [D + F_WORK]
GenerateDivideNext:
		dex
		bne GenerateDivide
		clc
		adc '0'
		ldx [D + F_LENGTH]
		sep m
		sta [D + F_DIGITS + X]
		rep m
		inx
		stx [D + F_LENGTH]
		lda [D + F_WORK]
		bne GenerateDigit
		; Where it goes: the first space, at most 8 - its length (the ~ and digits).
		lda 7
		sec
		sbc [D + F_LENGTH]
		sta [D + F_WORK]
		ldy 0
GenerateFind:
		cpy [D + F_WORK]
		beq GenerateFound
		lda [[D + GS_NAME]:24 + Y]
		and 0xFF
		cmp ' '
		beq GenerateFound
		iny
		bra GenerateFind
GenerateFound:
		sep m
		lda '~'
		sta [[D + GS_NAME]:24 + Y]
		ldx [D + F_LENGTH]
GeneratePut:
		iny
		dex
		lda [D + F_DIGITS + X]
		sta [[D + GS_NAME]:24 + Y]
		cpx 0
		bne GeneratePut
		rep m
		jmp Leave


; void ParsePathAndFilename(const char* szPath, char* szPathPart, const char** pszFilenamePart): the path up to the
; last \ (or the start) copied, and the file name after it.
PP_PATH			= F_ARGUMENT
PP_PART			= F_ARGUMENT + 4
PP_FILENAME		= F_ARGUMENT + 8
		.global _ParsePathAndFilename__FPcPcPPc
_ParsePathAndFilename__FPcPcPPc:
		.width (A, X), 16
		jsr Enter
		lda PP_PATH
		ldx F_STRING
		jsr Copy32
		jsr StringLength
		clc
		adc [D + PP_PATH]
		sta [D + F_WORK]
		lda [D + PP_PATH + 2]
		adc 0
		sta [D + F_WORK + 2]
ParsePathBack:
		lda [D + F_WORK]
		bne ParsePathLow
		dec [D + F_WORK + 2]
ParsePathLow:
		dec [D + F_WORK]
		lda [[D + F_WORK]:24]
		and 0xFF
		cmp BACKSLASH
		beq ParsePathFound
		lda [D + F_WORK]
		cmp [D + PP_PATH]
		bne ParsePathBack
		lda [D + F_WORK + 2]
		cmp [D + PP_PATH + 2]
		bne ParsePathBack
ParsePathFound:
		; The path part: szPath up to the \.
		lda [D + F_WORK]
		sec
		sbc [D + PP_PATH]
		sta [D + F_LENGTH]
		ldy 0
		sep m
ParsePathCopy:
		cpy [D + F_LENGTH]
		beq ParsePathCopied
		lda [[D + PP_PATH]:24 + Y]
		sta [[D + PP_PART]:24 + Y]
		iny
		bra ParsePathCopy
ParsePathCopied:
		lda 0
		sta [[D + PP_PART]:24 + Y]
		rep m
		lda [D + F_WORK]
		clc
		adc 1
		sta [[D + PP_FILENAME]:24]
		lda [D + F_WORK + 2]
		adc 0
		ldy 2
		sta [[D + PP_FILENAME]:24 + Y]
		jmp Leave
