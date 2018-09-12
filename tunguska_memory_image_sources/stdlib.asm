; Tunguska, ternary virtual machine
;
; Copyright (C) 2007,2008 Viktor Lofgren
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
;

; ---------------------------------------------------------------
; The beginnings of a universal function library
; ---------------------------------------------------------------
@ORG 		%333DDD

; Semi-universal text cursor

cursor:		@DT 	%4AD

keybuffer:	@REST 	40
.length:	@DT 	0, 0
.got_line:	@DT 	0

; -- Get string --
; 
;

getstring:
		LDA 	keybuffer.got_line		; Wait for string
		JEQ 	getstring

		SEI				; Disable interrupts 
						;(no typing while
						; loading the buffer)

		LDA 	#0			; Clear got_line flag
		STA 	keybuffer.got_line

		LDX 	#0
.copy:
		LDA 	keybuffer,X		; Copy the interrupt-buffer 
						; to the gets
		STA	.buffer,X		;  buffer
		INX
		TXA
		CMP 	#40
		JNE 	.copy

		CLI				; Restore interrupts

		RST

.buffer:	@REST	40			; Character buffer
		@DT 	0

;  ----
;   Feed screen down one line
;   Arguments: None

;   Not pretty, but does the work
;  ----

feedscreen:

		PSH	A			; Save A
		PHX
		PHY

		SEI				; Halt interrupts

		LAD	%DDBDDD			; Copy to %DDBDDD
		STX	agdp.R1
		STY	agdp.R1+1
		LAD	%DDBD2D			; Copy to %DDBDDD + 54
		STX	agdp.R2
		STY	agdp.R2+1
		LAD	{%DD0DDD - %DDBD2D}	; Copy	  %2000 - 54 chars 
		STX	agdp.R3
		STY	agdp.R3+1
		LDA	#agdp.BLT		
		STA	agdp.OP			; Do it!

		; Blank the new line
		LAD	{%DD0DDD - 54}		; Last line
		STX	agdp.R1
		STY	agdp.R1+1
		LAD	0			; Fill it with 0
		STX	agdp.R2
		STX	agdp.R2+1
		LAD	54			; Fill 54 characters
		STX	agdp.R3
		STY	agdp.R3+1
		LDA	#agdp.BLS
		STA	agdp.OP			; Do it!
		
		LDA 	#%4AD
		STA 	cursor			; Restore cursor

		CLI				; Resume interrupts
		JSR	repaint

		PLY
		PLX
		PLL	A

		RST

; ----------------------------------
; Put character
; Arguments: Character (A)
;
; Uses @cursor
; ----------------------------------

putchar:
		CMP	#2
		JEQ	.scroll
		
		LDX	cursor
		STA	screen.page2,X

		INX

		JVC	.nofeed
		LDX	#%4AD
		JSR	feedscreen
.nofeed:

		STX	cursor

		PSH	A
		JSR	repaint
		PLL	A
		RST
.scroll:
		JSR	feedscreen
		RST

; ----------------------------------
;  Put number in nonary
;  Arguments: Number (A)
; ----------------------------------
putnon:
		PSH	A			; Save A

		EOR 	#%D00
		LSR
		LSR
		LSR
		LSR
		TAX
		LDA 	.nonc+4,X		; Compensate for the possible
						; negative index of -4
		JSR	putchar

		PLL	A			; Restore and re-save A
		PSH	A

		EOR	#%0D0
		LSR
		LSR
		TAX
		LDA 	.nonc+4,X
		JSR	putchar

		PLL	A			; Restore A

		EOR 	#%00D
		TAX
		LDA 	.nonc+4,X
		JSR	putchar

		RST

.nonc:		@DT	'DCBA01234'

; ----------------------------------
;  Put string
;  Arguments: String page (X), String offset (Y)
;  String is 0-ended
; ----------------------------------

puts:
		SEI				; Disable interrupts
		PSH	A

		STX 	.string			; Store string in memory
		STY 	.string+1	
.loop:
		LDA 	(.string)	
		JEQ 	.done			; Continue until null

		JSR	putchar

		LDA 	.string
		INC 	.string+1
		ADD 	#0			; Add with carry
		STA 	.string

		JMP 	.loop			;
.done:
		; Redraw screen

		JSR	repaint

		PLL	A			; Restore accumulator
		CLI				; Restore interrupts
		RST				; Return

.string:	@DW 	0

; -------------------------------------
; String comparison
; Arguments:
;   (pushed in this order)
;   String1 page (stack), String1 offset (stack), 
;   String2 page (stack), String2 offset (stack)
; Returns
;   0 if equal (in acc)
; -------------------------------------

strcmp:
		PLX				; Save return address
		PLY	

		; Get string addresses
		PLL	.str2+1
		PLL 	.str2+0
		PLL	.str1+1
		PLL	.str1+0

		PHY				; Restore return address
		PHX

.loop:
		LDA 	(.str1)
		CMP 	(.str2)
		JNE 	.mismatch

		LDA 	(.str1)
		JEQ 	.match
	
		LDA 	.str1
		INC 	.str1+1
		ADD 	#0
		STA 	.str1
	
		LDA 	.str2
		INC 	.str2+1
		ADD 	#0
		STA 	.str2

		JMP 	.loop
		
.mismatch:
		PLX				; Save return address
		PLY
	
		LDA 	#1			; Return 1
	
		PHY				; Restore return address
		PHX
	
		RST				; Return
.match:
		PLX				; Save return address
		PLY
	
		LDA 	#0			; Return 0
	
		PHY				; Restore return address
		PHX
	
		RST				; Return

.str1: 		@DW 	1
.str2: 		@DW 	1
	
; ------------------------------------
;  String length
;  Arguments: string page (X), string offset (Y)
;  Returns: string length (A)
; ------------------------------------

strlen:
		STX	.str
		STY	.str+1
		LDX	#0

.loop:
		LDA	(.str)
		JEQ	.done

		LDA	.str
		INC	.str+1
		ADD	#0
		STA	.str

		INX

		JMP	.loop
.done:
		TXA
		RST


.str:		@DW 	0

; -- index, ... like stdlib:s index() --
; Arguments: A -- Character to match,
;            X, Y -- Pointer to string
; Returns:
;	     X,Y -- Pointer to position in string matching A
;			or 0,0 if zero

index:
.loop:
		CMP	 X,Y			; Compare to given character
		JEQ 	.match			;

		PSH	A
		LDA 	X,Y			; Compare to zero
		JEQ 	.mmatch
		PLL	A

		PSH	A			; Preserve A
		TXA				; Increase pointer by one
		INY
		ADD 	#0
		TAX
		PLL	A

		JMP 	.loop
.match:
		RST
.mmatch:
		PLL	A
		LAD 	null
		RST
; -- strspn (works like stdlib's epinymous function) --
; Arguments: 
;    Pushed in this order: 
;    (1) String page
;    (2) String offset
;    (3) Valid characters page
;    (4) Valid characters offset
; Returns
;   A: Length of span in primary string consisting 
;      entirely of characters in second string
;

strspn:
		PLX				; Save return address
		PLY	

		PLL	.valid+1
		PLL	.valid
		PLL	.string+1
		PLL	.string

		PHY
		PHX

		LDA	#0
		STA	.length

.loop:
		; Check if string is null
		LDA	(.string)
		JEQ	.done

		; Search for current character in valid characters
		LAD	.valid
		JSR	index
		TXA
		JNE	.done
		TYA
		JNE	.done

		; Increase span
		INC	.length

		; Increase string pointer
		;
		LDA	.string
		INC	.string+1
		ADD	#0
		STA	.string

		JMP	.loop
.done:
		LDA 	.length
		RST

.string: 	@DW 	0
.valid:  	@DW 	0
.length: 	@DT 	0

; -- strcspn (works like stdlib's epinymous function) --
; Arguments: 
;    Pushed in this order: 
;    (1) String page
;    (2) String offset
;    (3) Invalid characters page
;    (4) Invalid characters offset
; Returns
;   A: Length of span in primary string consisting 
;      entirely of characters in second string
;

strcspn:
		PLX			; Save return address
		PLY	

		PLL	.invalid+1
		PLL	.invalid
		PLL	.string+1
		PLL	.string

		PHY
		PHX

		LDA 	#0
		STA 	.length

.loop:
		; Check if string is null
		LDA 	(.string)
		JEQ 	.done

		; Search for current character in invalid characters
		LAD 	.invalid
		JSR 	index
		TXA
		JNE 	$$+7
		TYA
		JEQ 	.done

		; Increase span
		INC 	.length

		; Increase string pointer
		;
		LDA 	.string
		INC 	.string+1
		ADD 	#0
		STA 	.string
.done:
		LDA 	.length
		RST

.string: 	@DW 	0
.invalid:  	@DW 	0
.length: 	@DT 	0

; -- memset (6 trit version) --
; Arguments, pushed in this order: Value, Length, Memory page, Memory offset
;
;

memset6:
		PLX
		PLY

		PLL	.memory+1
		PLL	.memory
		PLL	.length
		PLL	.character

		PHY
		PHX

.loop:
		LDA 	.length
		LAD	.memory
		DEBUG
		LDA	.character
		STA	(.memory)

		LDA 	.memory
		INC	.memory+1
		ADD	#0
		STA	.memory

		DEC	.length
		JNE	.loop

		RST
		
.length:	@DT	0	
.memory:	@DW	0
.character:	@DT	0

random:
		LDA	.data
		CMP	.lastrandom
		JEQ	random

		LDA	.data
		STA	.lastrandom
		RST
.data:		@DT	0
.lastrandom:	@DT	0


; Between: takes a value and two limits, and returns the value if it is 
; within the limits, or the limit it breaks if it is outside.
; Argument: 
;	A   -  The value
;	X   -  The lower limit
;	Y   -  The upper limit
; Returns:
;	A   -  The chopped value
;
;
; BEHAVIOR IF X > Y IS UNDEFINED!

between:
		STX	.lower
		STY	.upper
		
		CMP	.lower		; Check if it is outside of the lower
		JLT	.toolow		; bounds

		CMP	.upper		; ... upper bounds
		JGT	.toohigh

		RST			; No change necessary
.toolow:
		LDA	.lower		; Trim it to the lower bounds
		RST
.toohigh:
		LDA	.upper		; Trim it to the upper bounds
		RST
.lower:		@DT	0
.upper: 	@DT	0

; -- Repaint the screen (if enabled)
;
repaint:
		LDA	redraw
		TSH	#%001
		STA	redraw
.done:
		RST
.enabled:	@DT	1

; Returns the case of input letter
;  Returns
;      /   %DDD if lowercase
; A =  >   %000 if not a letter
;      \   %444 if uppercase
;
;  Argument: A -- the letter to test

getcase:
		CMP	#10
		JLT	.notaletter
		CMP	#36
		JLT	.uppercase
		CMP	#50
		JLT	.notaletter
		CMP	#76
		JLT	.lowercase

		JMP	.notaletter
.notaletter:	STA	#0
		RST
.uppercase:
		STA	#%444
		RST
.lowercase:
		STA	#%DDD
		RST

; Replace a specific trit in a tryte without touching the remaining trits
;
; Arguments:   A -- tryte
;	       X&1 -- trit
;	       Y -- position

tritsplice:
		STA	.val
		STX	.trt
		STY	.pos

		LDA	.trt
		EOR	#%00A
		STA	.trt
		LDA	#1
		STA	.refval


		; Roll .trt into position
		LDX	.pos
.rol:		DEX
		JEQ	.roldone

		ASL	.trt
		ASL	.refval
		JMP	.rol
.roldone:
		; Create a tritmask that is [ 0 0 ... -1 .. 0 0 ]
		LDA	.refval
		EOR	.refval
		; Shift it upwards, it now is [ 1 1 ... 0 ... 1 1 ]
		TSH	#%444
		; Invert it
		EOR	#%444
		; Use it to zero the memory position in the value
		EOR	.val

		; Shift it into position
		TSH	.trt

		; Done
		RST

.val:		@DT	0
.refval:	@DT	0
.trt:		@DT	0
.pos:		@DT	0

;
; ---------------------------------------------------
;


; Disk I/O
;
;
;

floppy:
	@EQU	.noop		0
	@EQU	.read		1
	@EQU	.write		2
	@EQU	.sync		3
	@EQU	.seek		4
	@EQU	.poll		5
	@EQU	.status		6
	@EQU	.unload		7
	@EQU	.load		8

	@EQU	.memory		%DDDDDA


; Read a block from floppy
;   A: Floppy page 
;   Y: Memory page
;

fl_read_block:
	; Tell floppy to seek to A
	LDX	#floppy.seek
	STX	floppy.memory

	LDX	#floppy.read
	STX	floppy.memory

	RST

; Write a block to floppy
;   A: Floppy page 
;   Y: Memory page
;

fl_write_block:
		; Tell floppy to seek to A
		LDX	#floppy.seek
		STX	floppy.memory
	
		LDX	#floppy.write
		STX	floppy.memory

		LDX	#floppy.sync
		STX	floppy.memory

		RST

; Dump entire memory to floppy

fl_dump_memory:
		LDA	#%DDD
		LDX	#floppy.seek
		STX	floppy.memory
	
		LDY	#%DDD
		LDX	#floppy.write
.loop:
		STX	floppy.memory
		INY
		JVC	.loop

		LDX	#floppy.sync
		STX	floppy.memory

		RST

