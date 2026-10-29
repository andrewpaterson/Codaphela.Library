; memcpy, memmove and memset for the W65C816, with its block moves: mvn copies A + 1 bytes
; from [source bank : X] up to [destination bank : Y], 7 cycles a byte; mvp copies down from
; the ends.  The banks are the instruction's operand bytes, so each move writes them first
; (the code runs in RAM).  X and Y wrap in their bank, so a move stops at the end of the
; source's or the destination's bank and the next goes on from there.  memset stores the
; first byte and moves the rest up from one byte below, so each byte copies the one before
; it.  string.cpp has the other string functions (and these, for the 65C64's reference).
; The code changes itself, so it isn't reentrant: an interrupt handler mustn't call these.
; The calling convention is the compiler's (IR816Backend.h): arguments are pushed last first
; (pointers and size_t 4 bytes, int 2), A and X are 16 bits wide on the way in and out, the
; result is in A (low) and X (high), D and the data bank are kept and A, X and Y change.
; Each function (but memset within one bank, which reads its arguments from S) pushes the data
; bank, 14 bytes of locals and D, and points D at the stack:
;	D + 1		the caller's D
;	D + 3		Count: the bytes of this move less 1
;	D + 5		Dst: the destination (memmove down: its last byte)
;	D + 9		Src: the source (memmove down: its last byte)
;	D + 13		Bytes: the bytes left
;	D + 17		the caller's data bank
;	D + 18		the return address
;	D + 21		the arguments: the destination first

Count		= 3
Dst			= 5
Src			= 9
Bytes		= 13
Argument	= 21


		.section text.MemMove816
		.global _memcpy, _memmove, _memset

; void* memcpy(void* pvDest, const void* pvSource, size_t uiBytes)
_memcpy:
		.width (A, X), 16
		phb
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		jsr Arguments
		brl MoveUp

; void* memmove(void* pvDest, const void* pvSource, size_t uiBytes): up unless the destination
; is above the source and in it.
_memmove:
		.width (A, X), 16
		phb
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		jsr Arguments
		lda [D + Dst + 2]
		cmp [D + Src + 2]
		bne Ordered
		lda [D + Dst]
		cmp [D + Src]
Ordered:
		.width (A, X), 16
		bcc ToUp
		beq ToUp
		clc
		lda [D + Src]
		adc [D + Bytes]
		tax
		lda [D + Src + 2]
		adc [D + Bytes + 2]
		cmp [D + Dst + 2]
		bne Ended
		txa
		cmp [D + Dst]
Ended:
		.width (A, X), 16
		bcc ToUp
		beq ToUp
		brl MoveDown
ToUp:
		.width (A, X), 16
		brl MoveUp

; void* memset(void* pvDest, int iValue, size_t uiBytes): within one bank, without a frame
; (the arguments are read from S), the data bank set to the destination's and the value's
; byte twice in a word stored 16 bytes at a time (sta [n + Y], about 4 cycles a byte), then
; the words and the byte left; otherwise (or for 0 bytes) the first byte, then each from the
; one before.
_memset:
		.width (A, X), 16
		lda [S + 12]
		bne MemsetMove
		lda [S + 10]
		beq MemsetMove
		clc
		adc [S + 4]
		bcs MemsetMove
		phb
		sep m
		lda [S + 7]
		pha
		plb
		lda [S + 9]
		xba
		lda [S + 9]
		rep m
		pha
		lda [S + 7]
		tay
		lda [S + 13]
		lsr A
		lsr A
		lsr A
		lsr A
		beq MemsetWords
		tax
		lda [S + 1]
MemsetSixteen:
		.width (A, X), 16
		sta [0 + Y]
		sta [2 + Y]
		sta [4 + Y]
		sta [6 + Y]
		sta [8 + Y]
		sta [10 + Y]
		sta [12 + Y]
		sta [14 + Y]
		tya
		clc
		adc 16
		tay
		lda [S + 1]
		dex
		bne MemsetSixteen
MemsetWords:
		.width (A, X), 16
		lda [S + 13]
		and 14
		lsr A
		beq MemsetByte
		tax
		lda [S + 1]
MemsetWord:
		.width (A, X), 16
		sta [0 + Y]
		iny
		iny
		dex
		bne MemsetWord
MemsetByte:
		.width (A, X), 16
		lda [S + 13]
		and 1
		beq MemsetDone
		sep m
		lda [S + 1]
		sta [0 + Y]
		rep m
MemsetDone:
		.width (A, X), 16
		ply
		plb
		lda [S + 6]
		tax
		lda [S + 4]
		rtl
MemsetMove:
		.width (A, X), 16
		phb
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		lda [D + Argument + 6]
		ora [D + Argument + 8]
		bne Set
		brl Exit
Set:
		.width (A, X), 16
		sep m
		lda [D + Argument + 4]
		sta [[D + Argument]:24]
		rep m
		lda [D + Argument]
		sta [D + Src]
		clc
		adc 1
		sta [D + Dst]
		lda [D + Argument + 2]
		sta [D + Src + 2]
		adc 0
		sta [D + Dst + 2]
		sec
		lda [D + Argument + 6]
		sbc 1
		sta [D + Bytes]
		lda [D + Argument + 8]
		sbc 0
		sta [D + Bytes + 2]

; Moves up while there are bytes left: as many as the rest of the source's and the
; destination's banks hold, and at most 65536.
MoveUp:
		.width (A, X), 16
		lda [D + Bytes]
		ora [D + Bytes + 2]
		beq Exit
		jsr Limit
		lda [D + Src]
		eor 65535
		cmp [D + Count]
		bcs UpSource
		sta [D + Count]
UpSource:
		.width (A, X), 16
		lda [D + Dst]
		eor 65535
		cmp [D + Count]
		bcs UpDestination
		sta [D + Count]
UpDestination:
		.width (A, X), 16
		sep m
		lda [D + Src + 2]
		sta [Up + 2]:24
		lda [D + Dst + 2]
		sta [Up + 1]:24
		rep m
		ldx [D + Src]
		ldy [D + Dst]
		lda [D + Count]
Up:
		mvn 0, 0
		sec
		lda [D + Src]
		adc [D + Count]
		sta [D + Src]
		lda [D + Src + 2]
		adc 0
		sta [D + Src + 2]
		sec
		lda [D + Dst]
		adc [D + Count]
		sta [D + Dst]
		lda [D + Dst + 2]
		adc 0
		sta [D + Dst + 2]
		jsr Moved
		bra MoveUp

; The result is the destination argument.
Exit:
		.width (A, X), 16
		lda [D + Argument]
		ldx [D + Argument + 2]
		pld
		ply
		ply
		ply
		ply
		ply
		ply
		ply
		plb
		rtl

; Moves down from the ends while there are bytes left, as many as the source's and the
; destination's banks hold below them.
MoveDown:
		.width (A, X), 16
		clc
		lda [D + Src]
		adc [D + Bytes]
		sta [D + Src]
		lda [D + Src + 2]
		adc [D + Bytes + 2]
		sta [D + Src + 2]
		clc
		lda [D + Dst]
		adc [D + Bytes]
		sta [D + Dst]
		lda [D + Dst + 2]
		adc [D + Bytes + 2]
		sta [D + Dst + 2]
		lda [D + Src]
		bne SourceEnd
		dec [D + Src + 2]
SourceEnd:
		.width (A, X), 16
		dec [D + Src]
		lda [D + Dst]
		bne DestinationEnd
		dec [D + Dst + 2]
DestinationEnd:
		.width (A, X), 16
		dec [D + Dst]
DownLoop:
		.width (A, X), 16
		lda [D + Bytes]
		ora [D + Bytes + 2]
		beq Exit
		jsr Limit
		lda [D + Src]
		cmp [D + Count]
		bcs DownSource
		sta [D + Count]
DownSource:
		.width (A, X), 16
		lda [D + Dst]
		cmp [D + Count]
		bcs DownDestination
		sta [D + Count]
DownDestination:
		.width (A, X), 16
		sep m
		lda [D + Src + 2]
		sta [Down + 2]:24
		lda [D + Dst + 2]
		sta [Down + 1]:24
		rep m
		ldx [D + Src]
		ldy [D + Dst]
		lda [D + Count]
Down:
		mvp 0, 0
		clc
		lda [D + Src]
		sbc [D + Count]
		sta [D + Src]
		lda [D + Src + 2]
		sbc 0
		sta [D + Src + 2]
		clc
		lda [D + Dst]
		sbc [D + Count]
		sta [D + Dst]
		lda [D + Dst + 2]
		sbc 0
		sta [D + Dst + 2]
		jsr Moved
		bra DownLoop

; Copies the destination, source and size arguments to Dst, Src and Bytes.
Arguments:
		.width (A, X), 16
		lda [D + Argument]
		sta [D + Dst]
		lda [D + Argument + 2]
		sta [D + Dst + 2]
		lda [D + Argument + 4]
		sta [D + Src]
		lda [D + Argument + 6]
		sta [D + Src + 2]
		lda [D + Argument + 8]
		sta [D + Bytes]
		lda [D + Argument + 10]
		sta [D + Bytes + 2]
		rts

; Count = the bytes left less 1, at most 65535.
Limit:
		.width (A, X), 16
		lda [D + Bytes + 2]
		bne Most
		lda [D + Bytes]
		dec A
		sta [D + Count]
		rts
Most:
		.width (A, X), 16
		lda 65535
		sta [D + Count]
		rts

; Bytes -= Count + 1 (the clear carry takes the 1 more).
Moved:
		.width (A, X), 16
		clc
		lda [D + Bytes]
		sbc [D + Count]
		sta [D + Bytes]
		lda [D + Bytes + 2]
		sbc 0
		sta [D + Bytes + 2]
		rts


; The other string functions (string.cpp keeps strstr, which uses these), a byte at a time
; through the 24 bit pointers ([[D + n]:24 + Y]; memcmp a word at a time).  When Y wraps
; past 65535 the pointers' banks are moved on.  Each function pushes 6 bytes of locals and
; D, and points D at the stack:
;	D + 1		the caller's D
;	D + 3		Hi: a size_t count's high word (X is its low word)
;	D + 5		Save: the destination to return, or what is being worked out
;	D + 9		the return address
;	D + 12		the arguments: the first, then the second at D + 16

Hi			= 3
Save		= 5
First		= 12
Second		= 16


		.section text.String816
		.global _memcmp, _memchr, _strlen, _strcpy, _strncpy, _strcat, _strncat
		.global _strcmp, _strncmp, _strchr, _strrchr

; size_t strlen(const char* sz): A the low word, X the high.
_strlen:
		.width (A, X), 16
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		sep m
		ldy 0
		ldx 0
StrlenByte:
		.width A, 8
		.width X, 16
		lda [[D + First]:24 + Y]
		beq StrlenEnd
		iny
		bne StrlenByte
		inx
		inc [D + First + 2]
		bra StrlenByte
StrlenEnd:
		.width A, 8
		.width X, 16
		rep m
		tya
		pld
		ply
		ply
		ply
		rtl

; char* strcpy(char* szDest, const char* szSource)
_strcpy:
		.width (A, X), 16
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		jsr SaveFirst
		sep m
		ldy 0
		brl CopyString

; char* strcat(char* szDest, const char* szSource): strcpy to the destination's end.
_strcat:
		.width (A, X), 16
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		jsr SaveFirst
		jsr FindEnd
		brl CopyString

; char* strncpy(char* szDest, const char* szSource, size_t uiChars): the source's characters,
; then zeros, uiChars in all.
_strncpy:
		.width (A, X), 16
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		jsr SaveFirst
		ldx [D + Second + 4]
		lda [D + Second + 6]
		sta [D + Hi]
		sep m
		ldy 0
StrncpyChar:
		.width A, 8
		.width X, 16
		cpx 0
		bne StrncpyHave
		jsr More
		bcs StrncpyEnd
StrncpyHave:
		.width A, 8
		.width X, 16
		dex
		lda [[D + Second]:24 + Y]
		sta [[D + First]:24 + Y]
		beq StrncpyPadNext
		iny
		bne StrncpyChar
		inc [D + First + 2]
		inc [D + Second + 2]
		bra StrncpyChar
StrncpyPadNext:
		.width A, 8
		.width X, 16
		iny
		bne StrncpyPad
		inc [D + First + 2]
StrncpyPad:
		.width A, 8
		.width X, 16
		cpx 0
		bne StrncpyZero
		jsr More
		bcs StrncpyEnd
StrncpyZero:
		.width A, 8
		.width X, 16
		dex
		lda 0
		sta [[D + First]:24 + Y]
		bra StrncpyPadNext
StrncpyEnd:
		.width A, 8
		.width X, 16
		brl ReturnSaved

; char* strncat(char* szDest, const char* szSource, size_t uiChars): at most uiChars of the
; source at the destination's end, then a zero.
_strncat:
		.width (A, X), 16
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		jsr SaveFirst
		jsr FindEnd
		rep m
		ldx [D + Second + 4]
		lda [D + Second + 6]
		sta [D + Hi]
		sep m
StrncatChar:
		.width A, 8
		.width X, 16
		cpx 0
		bne StrncatHave
		jsr More
		bcs StrncatEnd
StrncatHave:
		.width A, 8
		.width X, 16
		dex
		lda [[D + Second]:24 + Y]
		beq StrncatEnd
		sta [[D + First]:24 + Y]
		iny
		bne StrncatChar
		inc [D + First + 2]
		inc [D + Second + 2]
		bra StrncatChar
StrncatEnd:
		.width A, 8
		.width X, 16
		lda 0
		sta [[D + First]:24 + Y]
		brl ReturnSaved

; Copies the source's characters to the destination at Y up to and with the zero (A 8 bits
; wide), then returns the saved destination.
CopyString:
		.width A, 8
		.width X, 16
		lda [[D + Second]:24 + Y]
		sta [[D + First]:24 + Y]
		beq ReturnSaved
		iny
		bne CopyString
		inc [D + First + 2]
		inc [D + Second + 2]
		bra CopyString
ReturnSaved:
		.width A, 8
		.width X, 16
		rep m
		lda [D + Save]
		ldx [D + Save + 2]
		pld
		ply
		ply
		ply
		rtl

; int strcmp(const char* sz1, const char* sz2)
_strcmp:
		.width (A, X), 16
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		sep m
		ldy 0
StrcmpChar:
		.width A, 8
		.width X, 16
		lda [[D + First]:24 + Y]
		cmp [[D + Second]:24 + Y]
		bne Difference
		cmp 0
		beq Same
		iny
		bne StrcmpChar
		inc [D + First + 2]
		inc [D + Second + 2]
		bra StrcmpChar

; int strncmp(const char* sz1, const char* sz2, size_t uiChars)
_strncmp:
		.width (A, X), 16
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		ldx [D + Second + 4]
		lda [D + Second + 6]
		sta [D + Hi]
		sep m
		ldy 0
StrncmpChar:
		.width A, 8
		.width X, 16
		cpx 0
		bne StrncmpHave
		jsr More
		bcs Same
StrncmpHave:
		.width A, 8
		.width X, 16
		dex
		lda [[D + First]:24 + Y]
		cmp [[D + Second]:24 + Y]
		bne Difference
		cmp 0
		beq Same
		iny
		bne StrncmpChar
		inc [D + First + 2]
		inc [D + Second + 2]
		bra StrncmpChar

; The result of a compare that found equal strings or memory (A 8 bits wide).
Same:
		.width A, 8
		.width X, 16
		rep m
		lda 0
		pld
		ply
		ply
		ply
		rtl

; The result of a compare that found two bytes that differ at Y: the first's (in A, 8 bits
; wide) less the second's, as unsigned chars.
Difference:
		.width A, 8
		.width X, 16
		rep m
		and 255
		sta [D + Save]
		lda [[D + Second]:24 + Y]
		and 255
		sta [D + Save + 2]
		lda [D + Save]
		sec
		sbc [D + Save + 2]
		pld
		ply
		ply
		ply
		rtl

; int memcmp(const void* pv1, const void* pv2, size_t uiBytes): words while there are two
; bytes left, then the byte left; the bytes of a word that differs are compared first to
; last.  One byte (CNumber compares a digit at a time) is compared straight away.
_memcmp:
		.width (A, X), 16
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		lda [D + Second + 6]
		bne MemcmpWords
		lda [D + Second + 4]
		cmp 1
		bne MemcmpWords
		ldy 0
		sep m
		lda [[D + First]:24 + Y]
		cmp [[D + Second]:24 + Y]
		bne Difference
		bra MemcmpSame
MemcmpWords:
		.width (A, X), 16
		lda [D + Second + 6]
		lsr A
		sta [D + Hi]
		lda [D + Second + 4]
		ror A
		tax
		ldy 0
MemcmpWord:
		.width (A, X), 16
		cpx 0
		bne MemcmpHave
		jsr MoreWide
		bcs MemcmpOdd
MemcmpHave:
		.width (A, X), 16
		dex
		lda [[D + First]:24 + Y]
		cmp [[D + Second]:24 + Y]
		bne MemcmpWordDiffers
		iny
		iny
		bne MemcmpWord
		inc [D + First + 2]
		inc [D + Second + 2]
		bra MemcmpWord
MemcmpWordDiffers:
		.width (A, X), 16
		sep m
		lda [[D + First]:24 + Y]
		cmp [[D + Second]:24 + Y]
		bne Difference
		iny
		lda [[D + First]:24 + Y]
		bra Difference
MemcmpOdd:
		.width (A, X), 16
		sep m
		lda [D + Second + 4]
		and 1
		beq MemcmpSame
		lda [[D + First]:24 + Y]
		cmp [[D + Second]:24 + Y]
		bne MemcmpDifference
MemcmpSame:
		.width A, 8
		.width X, 16
		rep m
		lda 0
		pld
		ply
		ply
		ply
		rtl
MemcmpDifference:
		.width A, 8
		.width X, 16
		brl Difference

; void* memchr(const void* pv, int iValue, size_t uiBytes): iValue as an unsigned char.
_memchr:
		.width (A, X), 16
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		ldx [D + Second + 2]
		lda [D + Second + 4]
		sta [D + Hi]
		sep m
		ldy 0
MemchrByte:
		.width A, 8
		.width X, 16
		cpx 0
		bne MemchrHave
		jsr More
		bcs NotFound
MemchrHave:
		.width A, 8
		.width X, 16
		dex
		lda [[D + First]:24 + Y]
		cmp [D + Second]
		beq Found
		iny
		bne MemchrByte
		inc [D + First + 2]
		bra MemchrByte

; char* strchr(const char* sz, int c): c as a char, the zero at the end included.
_strchr:
		.width (A, X), 16
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		sep m
		ldy 0
StrchrChar:
		.width A, 8
		.width X, 16
		lda [[D + First]:24 + Y]
		cmp [D + Second]
		beq Found
		cmp 0
		beq NotFound
		iny
		bne StrchrChar
		inc [D + First + 2]
		bra StrchrChar

; The first argument's address plus Y (A 8 bits wide).
Found:
		.width A, 8
		.width X, 16
		rep m
		tya
		clc
		adc [D + First]
		sta [D + First]
		lda [D + First + 2]
		adc 0
		tax
		lda [D + First]
		pld
		ply
		ply
		ply
		rtl

; NULL (A 8 bits wide).
NotFound:
		.width A, 8
		.width X, 16
		rep m
		lda 0
		ldx 0
		pld
		ply
		ply
		ply
		rtl

; char* strrchr(const char* sz, int c): the last c, Save is where it is (NULL until found).
_strrchr:
		.width (A, X), 16
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		sep m
		ldy 0
StrrchrChar:
		.width A, 8
		.width X, 16
		lda [[D + First]:24 + Y]
		cmp [D + Second]
		bne StrrchrOther
		rep m
		tya
		clc
		adc [D + First]
		sta [D + Save]
		lda [D + First + 2]
		adc 0
		sta [D + Save + 2]
		sep m
		lda [[D + First]:24 + Y]
StrrchrOther:
		.width A, 8
		.width X, 16
		cmp 0
		beq StrrchrEnd
		iny
		bne StrrchrChar
		inc [D + First + 2]
		bra StrrchrChar
StrrchrEnd:
		.width A, 8
		.width X, 16
		brl ReturnSaved

; Save = the first argument (the destination to return).
SaveFirst:
		.width (A, X), 16
		lda [D + First]
		sta [D + Save]
		lda [D + First + 2]
		sta [D + Save + 2]
		rts

; Moves the destination on to its zero and Y to 0 (so the source is copied from its start),
; A 8 bits wide from then on.
FindEnd:
		.width (A, X), 16
		sep m
		ldy 0
FindEndChar:
		.width A, 8
		.width X, 16
		lda [[D + First]:24 + Y]
		beq FindEndDone
		iny
		bne FindEndChar
		inc [D + First + 2]
		bra FindEndChar
FindEndDone:
		.width A, 8
		.width X, 16
		rep m
		tya
		clc
		adc [D + First]
		sta [D + First]
		lda [D + First + 2]
		adc 0
		sta [D + First + 2]
		sep m
		ldy 0
		rts

; Called when X is 0: the next 65536 bytes from Hi.  Carry set when there are none (A 8 bits
; wide on the way in and out).
More:
		.width A, 8
		.width X, 16
		rep m
		jsr MoreWide
		sep m
		rts

; As More with A 16 bits wide.
MoreWide:
		.width (A, X), 16
		lda [D + Hi]
		beq MoreNone
		dec A
		sta [D + Hi]
		clc
		rts
MoreNone:
		.width (A, X), 16
		sec
		rts


		.section text.MemSwp816
		.global _MemSwp__FPvPvj

; void MemSwp(const void* pv1, const void* pv2, size uiLength) (BaseLib's StringHelper.cpp, left
; out for the W65C816): a word at a time, then the byte left, Y running up from both pointers
; (carrying into the next bank), 64KB at a time.  D points at the stack: the pointers are at
; D + 6 and D + 10, the length at D + 14.
_MemSwp__FPvPvj:
		.width (A, X), 16
		phd
		tsc
		tcd
MemSwpChunk:
		.width (A, X), 16
		lda [D + 16]
		beq MemSwpLast
		ldy 0
		ldx 32768
		jsr MemSwpWords
		sep m
		inc [D + 8]
		inc [D + 12]
		rep m
		dec [D + 16]
		bra MemSwpChunk
MemSwpLast:
		.width (A, X), 16
		ldy 0
		lda [D + 14]
		lsr A
		beq MemSwpOdd
		tax
		jsr MemSwpWords
MemSwpOdd:
		.width (A, X), 16
		lda [D + 14]
		and 1
		beq MemSwpDone
		sep m
		lda [[D + 6]:24 + Y]
		pha
		lda [[D + 10]:24 + Y]
		sta [[D + 6]:24 + Y]
		pla
		sta [[D + 10]:24 + Y]
		rep m
MemSwpDone:
		.width (A, X), 16
		pld
		rtl

; X words from Y, Y after them.
MemSwpWords:
		.width (A, X), 16
		lda [[D + 6]:24 + Y]
		pha
		lda [[D + 10]:24 + Y]
		sta [[D + 6]:24 + Y]
		pla
		sta [[D + 10]:24 + Y]
		iny
		iny
		dex
		bne MemSwpWords
		rts
