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

;@ORG 	%341DDD

;  Raster graphics functions
;


; This is not a function, but an anchorpoint for variables

raster:
@EQU	.width	324
@EQU	.height	243
@EQU	.mempos	%DDBDDD

; putpixel: Put a pixel on the screen (in raster mode!)
; arguments:
;	A	Pixel color
;	X	X coordinate
;	Y	Y coordinate

putpixel:
		STA	.c	; Store color

		STX	.x	; and coordinates
		STY	.y


		; Trim X coordinate to screen dimensions
		LDA	.x
		LDX	#0
		LDY	#raster.width
		JSR	between
		STA	.x
	
		; Trim Y coordinate to screen dimensions
		LDA	.y
		LDX	#0
		LDY	#raster.height
		JSR	between
		STA	.y

		; Initialize memory pointer
		LAD	raster.mempos
		STX	.mem
		STY	.mem+1

		; Add X offset
		CLC
		LDA	.x
		ADD	.mem+1
		STA	.mem+1
		; Add carry to high tryte
		LDA	#0
		ADD	.mem		; Add carry
		STA	.mem

		; Add Y offset (display.width * .y)
		CLC
		LDA	#raster.width
		MLL	.y
		ADD	.mem+1
		STA	.mem+1
		; Carry
		LDA	#0
		ADD	.mem
		STA	.mem
	
		; Add high tryte off previous y-calculation
		CLC
		LDA	#raster.width
		MLH	.y
		ADD	.mem
		STA	.mem
	
		; Put pixel in memory
		LDA	.c
		STA	(.mem)
		RST
	
.x:		@DT	0
.y:		@DT	0
.c:		@DT	0
.mem:		@DW	0

; getpixel: Get a pixel value from the screen (in raster mode!)
; arguments:
;	X	X coordinate
;	Y	Y coordinate

getpixel:
		STX	.x	; and coordinates
		STY	.y

		; Trim X coordinate to screen dimensions
		LDA	.x
		LDX	#0
		LDY	#raster.width
		JSR	between
		STA	.x
	
		; Trim Y coordinate to screen dimensions
		LDA	.y
		LDX	#0
		LDY	#raster.height
		JSR	between
		STA	.y

		; Initialize memory pointer
		LAD	raster.mempos
		STX	.mem
		STY	.mem+1
	
		; Add X offset
		CLC
		LDA	.x
		ADD	.mem+1
		STA	.mem+1
		; Add carry to high tryte
		LDA	#0
		ADD	.mem		; Add carry
		STA	.mem

		; Add Y offset (display.width * .y)
		CLC
		LDA	#raster.width
		MLL	.y
		ADD	.mem+1
		STA	.mem+1
		; Carry
		LDA	#0
		ADD	.mem
		STA	.mem

		; Add high tryte off previous y-calculation
		CLC
		LDA	#raster.width
		MLH	.y
		ADD	.mem
		STA	.mem

		; Put pixel in memory
		LDA	(.mem)
		RST
	
.x:		@DT	0
.y:		@DT	0
.mem:		@DW	0



putpixel3:
		STA	.c	; Store color

		STX	.x	; and coordinates
		STY	.y


		; Trim X coordinate to screen dimensions
		LDA	.x
		LDX	#0
		LDY	#raster.width
		JSR	between
		STA	.x
	
		; Trim Y coordinate to screen dimensions
		LDA	.y
		LDX	#0
		LDY	#raster.height
		JSR	between
		STA	.y

		; Initialize memory pointer
		LAD	raster.mempos
		STX	.mem
		STY	.mem+1

		; Add X offset
		CLC
		LDA	.x
		DIV	#6
		ADD	.mem+1
		STA	.mem+1
		; Add carry to high tryte
		LDA	#0
		ADD	.mem		; Add carry
		STA	.mem

		; Add Y offset (display.width * .y)
		CLC
		LDA	#raster.width/6
		MLL	.y
		ADD	.mem+1
		STA	.mem+1
		; Carry
		LDA	#0
		ADD	.mem
		STA	.mem

		; Add high tryte off previous y-calculation
		CLC
		LDA	#raster.width/6
		MLH	.y
		ADD	.mem
		STA	.mem

		; Splice trit
		
		; Position in tryte is 6 - x % 6
		LDA	.x
		MOD	#-6
		EOR	#%444
		CLC
		ADD	#6
		TAY

		; Color
		LDA	.c
		TAX
	
		LDA	(.mem)
		JSR	tritsplice
	
		; Put pixel in memory
		STA	(.mem)
		RST

.x:		@DT	0
.y:		@DT	0
.c:		@DT	0
.mem:		@DW	0


