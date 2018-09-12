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

@INC 'stdlib.asm'
@INC 'raster.asm'
@INC 'screen.asm'
@INC 'agdp.asm'

		@ORG	%DDDDDD			;  Beginning of memory
irq:		@DT	0
.data:		@DT	0
redraw: 	@DT	0			; Screen repaints when
						; different from 0
floppyio:	@DT	0

; -------------------------------------------------------------
		@ORG	%DDCDDD			; Stack page
; --------------------------------------------------------------
		@ORG	%DDBDDD			; Screen buffer
screen:
		@ORG	%DDB000
.page1:
		@ORG	%DDA000
.page2:

		@ORG	%DD0DDD
.end:						; End of screen	

; ---------------------------------------------------------------

		@ORG	%000000			; Initial PC value
origin:			; <--- Note to self: This is a really bad place to put
null:			; debug code, it breaks a bunch of functions
			; relying on null to be %000000
		JMP	inloop

		@ORG	%001000
jumpvector:
.getstring:	@DW	getstring
.feedscreen:	@DW	feedscreen
.putchar:	@DW	putchar
.putnon:	@DW	putnon
.puts:		@DW	puts
.strcmp:	@DW	strcmp
.strlen:	@DW	strlen
.index:		@DW	index
.strspn:	@DW	strspn
.strcspn:	@DW	strcspn
.memset6:	@DW	memset6
.random:	@DW	random
.between:	@DW	between
.repaint:	@DW	repaint
.fl_read_block: @DW	fl_read_block
.fl_write_block: @DW	fl_write_block
.mouse.x:	@DW	mouse.x
.mouse.y:	@DW	mouse.y
.putpixel:	@DW	putpixel
.getixel:	@DW	getpixel
.putpixel3:	@DW	putpixel3

		@ORG	%100000
inloop:
		LDA 	counter
		JSR 	putnon

		LDA 	counter+1
		JSR 	putnon

		LAD 	prompt			; Print "> "
		JSR 	puts

		JSR 	getstring		; Wait for input
		LDX 	#0

		LDA	counter
		INC	counter+1
		ADD	#0
		STA	counter
.loop:		
		PHX				; Save X
		LAD 	(commands, X)		; Load command string
		CAD	null
		JEQ 	.invalid		; Break loop if null

		PHX				; Push command string
		PHY
		LAD 	getstring.buffer	; Push entered text
		PHX
		PHY
		JSR 	strcmp			; Compare

		PLX				; Restore X
		LDA 	A			; Load A into A (refresh flags)

		JNE 	$$+9			; If not a match, continue
		JSR 	(commands + 2 ,X) 	; Run the function
		JMP 	inloop			; 

						; Increase X by 4
		TXA
		CLC
		ADD 	#4
		TAX
						; Keep comparing
		JMP 	.loop
		
.invalid:
		JSR 	unknown
		JMP 	inloop

commands:	@DW 	help.txt , help
		@DW	charmap.txt , charmap
		@DW	brown.txt, brown
		@DW	mimic.txt, mimic
		@DW	clear.txt, clear
		@DW	memdump.txt, memdump
		@DW	load.txt, load
		@DW	run.txt, run
		@DW	unload.txt, unload
		@DW	fdstat.txt, fdstat
		@DW	rasterdemo3.txt, rasterdemo3
		@DW	rasterdemo729.txt, rasterdemo729
		@DW 	0, 0


; Help function
;
;
;
;
help:
		LAD 	.message
		JSR 	puts
		JSR 	feedscreen
		RST

.message: 	@DT 	'Available commands are=', 2, 2
		@DT	'  HELP (this message)', 2
		@DT	'  CHARMAP (show character map)', 2
		@DT	'  CLEAR (clear screen)', 2 
		@DT	'  MEMDUMP (dump entire system memory to floppy)', 2
		@DT	'  BROWN (brownian motion)   *', 2
		@DT	'  MIMIC (mimic user input)', 2
		@DT	'  RASTERDEMO3 (primitive drawing program,', 2
		@DT	'    3 color mode)   *', 2
		@DT	'  RASTERDEMO729 (primitive drawing program,', 2
		@DT	'    729 color mode)   *', 2
		@DT	2, '*=Does not revert back to text mode when done', 2
		@DT	2, 'Disk related operations are=', 2, 2
		@DT	'  LOAD (load floppy image)', 2
		@DT	'  FDSTAT (check if floppy image is loaded)', 2
		@DT	'  RUN (run loaded floppy image)', 2
		@DT	'  UNLOAD (unload floppy image)', 2
		@DT	0
.txt:		@DT 	'HELP', 0

; Memory dump function. Writes all of memory to disk
;
;
memdump:
		JSR	fl_dump_memory
		RST
.txt:		@DT	'MEMDUMP', 0




; Load subroutine from %DDDxxx on disk to %000xxx in memory
;
;
load:
		LAD	.prompt			; Ask for a filename
		JSR	puts

		JSR	getstring		; Get user input
		LAD	getstring.buffer

		LDA	#floppy.load		; Try to load
		STA	floppy.memory

		JSR	fdstat			; See if it loaded

		RST

.txt:		@DT	'LOAD', 0
.prompt:	@DT	'ENTER IMAGE TO LOAD=', 0


run:				; First, check if a disk is at all loaded
		LDA	#floppy.status
		STA	floppy.memory

		CMP	#0		; If it's loaded, do a local
		JGT	$$+10		; jump 3+3+3+1=10 memory positions
		LAD	.fail
		JSR	puts		; Print failure message
		RST

		LDY	#0		; Run the image
		LDA	#%DDD
		JSR	fl_read_block
		JMP	%000000
.txt:		@DT	'RUN', 0
.fail:		@DT	'NO DISK LOADED', 2, 0

; Unload disk
;
;

unload:		LDA	#floppy.unload
		STA	floppy.memory
		RST
.txt:		@DT	'UNLOAD', 0

; Poll disk status
;
;

fdstat:		LDA	#floppy.status
		STA	floppy.memory		; A is set to 1 if loaded
						;	      0 if not

		CMP	#0			; Compare A with 0
		JGT	.loaded			; Jump to loaded if > 0
		LAD	.emptytxt		; Else print 'Disk not loaded'
		JSR	puts
		RST				; And return

.loaded: 	LAD	.loadedtxt		; A > 0, print 'Disk loaded'
		JSR	puts
		RST				; And return

.txt:		@DT	'FDSTAT', 0
.loadedtxt:	@DT	'DISK LOADED', 2, 0
.emptytxt:	@DT	'DISK NOT LOADED', 2, 0

; Character map function
;
;

charmap:
		LAD	.info			; Print info message
		JSR	puts
		JSR	feedscreen
		JSR	feedscreen

		LDA	#0			; Indent by 1
		JSR	putchar

		LAD	.dec			; Print 0123456789
		JSR	puts
		LDA	#0

.loop:		PSH	A
		TAX
		MOD	#10			
		JNE	.sc
						; Only run if A is 10,20,30,...
		JSR 	feedscreen
		PLL	A
		PSH	A
		DIV	#10
		TAX				; Print left-column index
		LDA	.dec,X
		JSR	putchar
.sc:
		PLL	A
		JSR	putchar			; Print character
		INC	A			; Increase A
		CMP	#100			; Loop till A = 100
		JLT	.loop

		JSR	feedscreen		; Scroll a bit
		JSR	feedscreen

		RST
		
.dec:		@DT	'0123456789', 0
.txt:		@DT	'CHARMAP', 0
.info:		@DT	'Character value = vertical*10+horizontal', 0

; Enable vector mode (this really should be in stdlib)
;
;
;

vectormode:
		LDA	#%003
		STA	redraw
		RST
.txt:		@DT	'VECTORMODE', 0




; Enable raster mode
;
;
;

rastermode:
		JSR	clear
		LDA	redraw
		AND	#%44B
		STA	redraw
		RST

.txt:		@DT	'RASTER', 0




; A very crude drawing program, 729 color version
;
;

rasterdemo729:
		JSR	rastermode

		; Clear screen

		LDA	#%DDB
.cloop:
		; Block set %A:DDD/444 to %DDD
		PGS	#%DDD
		INC	A
		CMP	#{%DDB+108}
		JLT	.cloop
.loop:
		; Cap mouse cursor position
		LDX	#0
		LDY	#raster.width - 1
		LDA	mouse.x
		JSR	between
		STA	mouse.x

		LDX	#0
		LDY	#raster.height - 1
		LDA	mouse.y
		JSR	between
		STA	mouse.y

		; Put pixel
		LDX	mouse.x
		LDY	mouse.y
		LDA	mouse.button
		MLL	#%443
		ADD	#%001
		JSR	putpixel

		JMP	.loop

.txt:		@DT	'RASTERDEMO729', 0




; A very crude drawing program, 3 color version
;
;
;

rasterdemo3:
		LDA	#%01C
		STA 	redraw

		; Clear screen

		LDA	#%DDB
.cloop:
		; Block set %A:DDD/444 to %DDD
		PGS	#%DDD
		INC	A
		CMP	#{%DDB+18}
		JLT	.cloop
.loop:
		; Cap mouse cursor position
		LDX	#0
		LDY	#raster.width - 1
		LDA	mouse.x
		JSR	between
		STA	mouse.x

		LDX	#0
		LDY	#raster.height - 1
		LDA	mouse.y
		JSR	between
		STA	mouse.y

		; Put pixel
		LDX	mouse.x
		LDY	mouse.y
		LDA	mouse.button
		JSR	putpixel3

		JMP	.loop

.txt:		@DT	'RASTERDEMO3', 0


; Mimic user input on screen, quit when user says "BYE"
;
;
;

mimic:
		LAD	.info
		JSR	puts
.loop:
		; Get string
		JSR	getstring
		; Push string buffer onto stack
		LAD	getstring.buffer
		PHX
		PHY
		; Push "bye" onto stack
		LAD	.bye
		PHX
		PHY
		; Compare
		JSR	strcmp
		LDA	A	; Refresh P register

		JNE	$$+4	; If equal, return from subroutine
		RST
				; Otherwise, mimic user input
		LAD	.prefix
		JSR	puts
		LAD	getstring.buffer
		JSR	puts
				; Scroll down one line
		JSR 	feedscreen
				; Loop
		JMP	.loop
.txt:		@DT	'MIMIC', 0
.bye:		@DT	'BYE', 0
.prefix:	@DT	'You said= ', 0
.info:		@DT	'Type BYE to exit', 2 , 0



; Set text mode
;
;

textmode:
		LDA	redraw
		AND	#%441
		STA	redraw
		RST
.txt:		@DT	'TEXTMODE', 0

; A 1 pixel vector that takes a random walk, demonstrates vector graphics
; and a creative use of assembler variables
;
;

brown:
		JSR	clear		; Clear the screen
		JSR	vectormode	; Jump to vector mode

; If it is slow, uncomment the following:
;		LDA	#50
;		STA	clockfrq	; More clock interrupts = more
					; random values :-D

@EQU		.vector1   %DDBDDD	; Establish some pointers to the
@EQU		.vector2   %DDBDDA	; vectors we're gonna use

		; Mouse cursor (4 vectors)
@EQU		.cursor	 %DDBDD2

		; Initialize vectors
		LDA	#0
		STA	.vector1	; Vector from origin to this point,
					; we want it to be black (not drawn)


		STA	.vector1+1	; Set all initial positions to zero
		STA	.vector1+2
		STA	.vector2+1
		STA	.vector2+2
		STA	.cursor

		LDA	#%444		; Set the vector color to white
		STA	.vector2
		STA	.cursor+3
		STA	.cursor+6
		STA	.cursor+9
.loop:

		; Calculate offsets
		JSR	random		; Get a random value

		; Chop the random value into two trits, one in X and one in A
		PSH	A
		EOR	#%001
		TAX
		PLL	A
		LSR
		EOR	#%001
		
		; Add the A trit to the X-coordinate
		ADD	.vector1+1
		STA	.vector1+1
		INC	A		; Need an offset to see the vector
		STA	.vector2+1
		
		; Add the X trit to the Y-coordinate
		TXA
		ADD	.vector1+2
		STA	.vector1+2
		INC	A		; Offset
		STA	.vector2+2
		
		; Paint the mosue cursor vector
		LDA	mouse.x
		STA	.cursor+1
		STA	.cursor+9+1
		STA	.cursor+3+1
		CLC
		ADD	#5
		STA	.cursor+6+1
		LDA	mouse.y
		STA	.cursor+2
		STA	.cursor+9+2
		STA	.cursor+6+2
		CLC
		ADD	#-5
		STA	.cursor+3+2

		JMP	.loop		; Round and round it goes, will it stop,
					; no-one knows...
		RST
.txt:		@DT	'BROWN', 0




; Clear the screen
;

clear:
		LDX	#%DDD
		LDA	#0

.loop:		
		STA	%DDB000, X
		STA	%DDA000, X
		INX
		JVC	.loop
		RST

.txt:		@DT	'CLEAR', 0

; Function called when the user enters an invalid command
;
;

unknown:
		LAD 	.message
		JSR 	puts
		JSR 	feedscreen
		RST
.message: 	@DT 	'Unknown command', 0



; Command counter
;
;
;
counter:	@DW	0
; --

@ORG		%444DDD

; Interrupt handler
; (This needs to be fairly breief, since it only gets just under one page)
;
interrupthandler:

		LDA 	irq			; Load IRQ into A

		CMP	#.intno			; Check if interrupt is within
		JGT	.bad			; bounds

		MLL	#2			; Multiply by two (address size)
		TAX

		LAD	(.interrupts,X)		; Load address from interrupt
		CAD	null			; handler vector, run it if
		JNE	X,Y			; it's different from NULL

		; Unknown interrupt
.bad:
		LAD 	.badmsg
		JSR	puts
		RTI
.badmsg:	@DT	'UNKNOWN ERROR', 2, 0

.interrupts:	@DW	keyboard	;	IRQ = 0
		@DW	clock		; 	IRQ = 1
		@DW	0		;	
		@DW	dbz		;	IRQ = 3
		@DW	0		;
		@DW	mouse		;	IRQ = 5
		@DW	mousepress	;	IRQ = 6
@EQU		.intno	{$$-.interrupts}/2
		NOP



clock: 		
		;@EQU	.interrupt	1
		JSR	repaint

		LDA	irq.data
		STA	random.data

		RTI

; Division By Zero (or modulus by zero)
dbz: 		@EQU	.interrupt	3
		LAD	.errormsg
		JSR	puts
		JSR 	feedscreen
		RTI
.errormsg:	@DT	'ARITHMETIC ERROR (DIV BY ZERO?)', 2, 0

mouse:		@EQU	.interrupt	5
		LDA	irq.data

		; X data in 3 lowest trits
		EOR	#%0AD
		CLC
		ADD	.y
		STA	.y
		
		; Y data in 3 highest trits
		LDA	irq.data
		DIV	#27	; <=> 3xLSR
		CLC
		ADD	.x
		STA	.x

		RTI

.x:		@DT	0
.y:		@DT	0
.button:	@DT	0

mousepress:
		@EQU	.interrupt	6
		LDA	irq.data
		JGT	.press
		JLT	.release
		RTI
.press:		LDA	#1
		STA	mouse.button
		RTI
.release:	LDA	#0
		STA	mouse.button
		RTI
		

; Keyboard interrupt
keyboard: 	@EQU	.interrupt	0
		LDA	redraw			; Don't print stuff on the
						; screen in vector mode
		BIT	#%003
		JNE	.huh
		JLT	.huh

		LDA 	irq.data
		CMP 	#9			; > 9 => Regular symbol
		JGT 	.disp

		CMP 	#1			; Space
		JEQ 	.disp
		CMP 	#2			; New line
		JEQ 	.newline
		CMP	#4
		JEQ	.backspace

.huh:		RTI

.disp:		LDA 	irq.data		; Print character to screen
		JSR	putchar
		LDA	irq.data

		LDA 	#0			; Disable "got a new line"-flag
		STA 	keybuffer.got_line

		LDA 	keybuffer.length	; Buffer only holds 39 chars
		CMP 	#39
		JGT 	.huh
		TAX

		LDA 	irq.data		; Load input to A
		STA 	keybuffer,X		; Put it in buffer
		INX				; Increase buffer length
		LDA 	#0
		STA 	keybuffer,X		; Zero-terminate string
		STX 	keybuffer.length	; Save buffer length

		RTI
.backspace:
		LDA	keybuffer.length		; Don't backspace out of
		CMP	#1				; the keybuffer
		JLT	.huh

		DEC 	keybuffer.length		; Decrement keybuffer

		LDA	cursor				; Don't backspace out
		CMP	#%4AC				; of the line
		JLT	.huh

		DEC	A				; Decrement cursor
		STA	cursor
		TAX					; Transfer it to X
		
		LDA	#0
		STA	screen.page2,X			; Erase on screen

		RTI
.newline:
		JSR 	feedscreen		; Scroll the screen down

		LDX 	keybuffer.length
		LDA 	#0
		STA 	keybuffer,X
		STA 	keybuffer.length	; Reset buf length to zero

		LDA 	#1			; Enable "got a new line" flag
		STA 	keybuffer.got_line

		RTI
		

prompt:		@DT 	'> ', 0

; ----------------------------------------------------------------

@ORG %444441
clockfrq:	@DT 	%444			; Set clock to once every
						; 364 instructions
						; Yes, I know, clock frequency
						; is a misnomer, this is clock
						; INTERVAL. 
@ORG %444442
irvector: 	
		@DW 	interrupthandler	; Interrupt handler vector
@ORG %444444
endcode:					; End of memory



