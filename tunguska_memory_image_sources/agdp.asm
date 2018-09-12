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

; Auxiliary General Data Processor functionality, mostly definitions

@ORG	%340DDD

agdp:
.OP		@EQU	%DDDDD0
.R1		@EQU	%DDDDD1
.R2		@EQU	%DDDDD3
.R3		@EQU	%DDDDCD

.NOOP		@EQU	0		; Much like the goggles, it does nothing

.ITOF		@EQU	1		; Integer to float
.FTOI		@EQU	2		; Float to integer
.FADD		@EQU	3		; Floating point add
.FMUL		@EQU	4		; Floating point multiplication
.FDIV		@EQU	5		; Floating point division
.FEXP		@EQU	6		; Floating point exponential fctn
.FLOG		@EQU	7		; Floating point natural logarithm
.FCOS		@EQU	8		; Floating point cosine
.FSIN		@EQU	9		; Floating point sine

.BLT		@EQU	10		; Block transfer
.BLS		@EQU	11		; Block set
.BLA		@EQU	12		; Blockwise "BIT"
.BLX		@EQU	13		; Blockwise "EOR"
.BLO		@EQU	14		; Blockwise "ORA"
.BSH		@EQU	15		; Blockwise "TSH"
.BLP		@EQU	16		; Blockwise "PRM"

.WHEN		@EQU	17		; Current system time

.IDIVW		@EQU	18		; 12 trit integer division
.IMODW		@EQU	19		; 12 trit integer modulus

; Utility functions
.itof:
		LDA	#.ITOF
		STA	.OP
		RST
.ftoi:
		LDA	#.FTOI
		STA	.OP
		RST

.R1toR2:
		PSH	.R1
		PSH	.R1+1
		PLL	.R2+1
		PLL	.R2
		RST
.R1toR3:
		PSH	.R1
		PSH	.R1+1
		PLL	.R3+1
		PLL	.R3
		RST
.R2toR3:
		PSH	.R2
		PSH	.R2+1
		PLL	.R3+1
		PLL	.R3
		RST
.R2toR1:
		PSH	.R2
		PSH	.R2+1
		PLL	.R1+1
		PLL	.R1
		RST
.R3toR2:
		PSH	.R3
		PSH	.R3+1
		PLL	.R2+1
		PLL	.R2
		RST
.R3toR1:
		PSH	.R3
		PSH	.R3+1
		PLL	.R2+1
		PLL	.R2
		RST


