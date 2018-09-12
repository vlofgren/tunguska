/* Tunguska, ternary virtual machine
 *
 * Copyright (C) 2007-2009 Viktor Lofgren
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "machine.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "tryte.h"
#include "values.h"

#include <SDL/SDL.h>

machine::machine() {
	init_translation_table();

	S = "444";	// Stack page is "DDC"
	SP = "DDC";

	A = X = Y = CL = 0;

	P = 0; /* All flags false */
	PCH = 0; PCL = 0;

	/* For sanity's sake, set a RTI interrupt vector!!! */
	memref("444", "444") = qop(IMPLICIT, RTI); 
	memref("444", "443") = "444";
	memref("444", "442") = "444";
	memref("444", "441") = 100; // Clock frequency

	current_state = new running_state();
	trace = false;

	interrupt_queue = new std::queue<interrupt*>();
	init_translation_table();
	assert(interrupt_queue);
}

machine::~machine() {
	while(!interrupt_queue->empty()) {
		delete interrupt_queue->front();
		interrupt_queue->pop();
	}
	delete interrupt_queue;
}

void machine::init_translation_table() {
	/* This is a really ugly hack, but it does keep maintenance in check,
	 * and makes debugging and assembling so much easier. */
#define STRTOOPC(X) translation_table[machine::X] = strdup(#X);
	STRTOOPC(NOP); STRTOOPC(TAX); STRTOOPC(TAY); STRTOOPC(TXA); 
	STRTOOPC(TYA); STRTOOPC(PSH); STRTOOPC(PHP); STRTOOPC(PLL); 
	STRTOOPC(PLP); STRTOOPC(PHX); STRTOOPC(PHY); STRTOOPC(PLX);
	STRTOOPC(PLY); STRTOOPC(STA); STRTOOPC(STX); STRTOOPC(STY); 
	STRTOOPC(LDA); STRTOOPC(LDX); STRTOOPC(LDY); STRTOOPC(AND); 
	STRTOOPC(EOR); STRTOOPC(PRM); STRTOOPC(ORA); STRTOOPC(ADD);
	STRTOOPC(CMP); STRTOOPC(INC); STRTOOPC(INX); STRTOOPC(INY); 
	STRTOOPC(DEC); STRTOOPC(DEX); STRTOOPC(DEY); STRTOOPC(ASL); 
	STRTOOPC(LSR); STRTOOPC(ROL); STRTOOPC(ROR); STRTOOPC(JCC); 
	STRTOOPC(JCT); STRTOOPC(JCF); STRTOOPC(JNE); STRTOOPC(JLT); 
	STRTOOPC(JGT); STRTOOPC(JVS); STRTOOPC(JVC); STRTOOPC(CLC); 
	STRTOOPC(CLI); STRTOOPC(IVC); STRTOOPC(CLV); STRTOOPC(SEC); 
	STRTOOPC(SEI); STRTOOPC(BRK); STRTOOPC(BRK); STRTOOPC(RTI); 
	STRTOOPC(MLH); STRTOOPC(MLL); STRTOOPC(MOD); STRTOOPC(DIV); 
	STRTOOPC(JMP); STRTOOPC(JSR); STRTOOPC(RST); STRTOOPC(TSX);
	STRTOOPC(TXS); STRTOOPC(TSH); STRTOOPC(BUT); STRTOOPC(JEQ); 
	STRTOOPC(LAD); STRTOOPC(BIT); STRTOOPC(PGT); STRTOOPC(PGS);
	STRTOOPC(CAD); STRTOOPC(XAM); STRTOOPC(XAX); STRTOOPC(XAY);
	STRTOOPC(XYX); STRTOOPC(PLC); STRTOOPC(ADW); STRTOOPC(MLW);
	STRTOOPC(DVW); STRTOOPC(MDW); 

	STRTOOPC(PAUSE); STRTOOPC(DEBUG);
#undef STRTOOPC
}

/* see share/symbols.bmp */
char asciitoternary(char a) {
	char translation[] = {  0, ' ','\n','\t','\b',   0,  0,   0,   0,   0,
			    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
			    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
			    'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3',
			    '4', '5', '6', '7', '8', '9', '.', ',', '!', '?',
			    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
			    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
			    'u', 'v', 'w', 'x', 'y', 'z',  0 ,   0,   0,   0,
			      0,   0,   0,   0,   0,   0, '=', '-', '*', '/',
			    '%', '<', '>',   0,   0, '(', ')', '$', '+', '#' };

	for(int c  = 0; c < 100; c++) {
		if(translation[c] == a) return c;

	}

	return 0;
}

char ternarytoascii(int a) {
	char translation[] = {  0, ' ','\n','\t','\b',   0,  0,   0,   0,   0,
			    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
			    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
			    'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3',
			    '4', '5', '6', '7', '8', '9', '.', ',', '!', '?',
			    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
			    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
			    'u', 'v', 'w', 'x', 'y', 'z',  0 ,   0,   0,   0,
			      0,   0,   0,   0,   0,   0, '=', '-', '*', '/',
			    '%', '<', '>',   0,   0, '(', ')', '$', '+', '#' };

	if(a > 0 && a < 101) {
		return translation[a];
	}
	return 0;
}

const char* machine::opcode_to_string(int opcode) const {
	if(opcode < -40 || opcode > 40) {
		return "DEBUG";
	}
	std::map<int, const char*>::const_iterator i = translation_table.find(opcode);
	if(i != translation_table.end()) return (*i).second;

	else return "DEBUG";
}

/* Try to run an interrupt */
void machine::run_interrupt() {
	if(P[B].to_int() != 0 || P[I].to_int() != 0 ||
			interrupt_queue->empty()) return;
	interrupt* i = interrupt_queue->front();
	interrupt_queue->pop();

	memrefi(TV_DDD, TV_DDD) = i->IRQ();
	memrefi(TV_DDD, TV_DDC) = i->data();
	brk();
	delete i;
}

/* Process a single instruction */
void machine::instruction() {
	/* Check for interrupt */
	run_interrupt();

	register int ip = tryte::word_to_int(PCH, PCL);
//	tryte instruction = memref(ip);
//	tryte highbits = instruction >> 4;
//	instruction = (instruction << 2) >> 2;

	int instruction = memref(ip).to_int();
	int highbits;
	if(instruction > 0) {
		highbits = ((instruction + 40) / 81);
	} else {
		highbits = ((instruction-40) / 81);

	}
	instruction -= 81 * highbits;
	

	tryte* arg = &A;
	tryte* abs1 = &PCL,* abs2 = &PCH;
	
	/* This allows reference passing when there isn't a distinct
	 * memory address specified (absolute-X addressing for an instance) */
	static tryte cheatarg1, cheatarg2;

	if(trace || get_state()->is_tracing()) { 
		const char* opname = translation_table[instruction];
		printf("%.3X:%.3X %s\t", PCH.nonaryhex(), PCL.nonaryhex(), opname);
		switch(highbits) {
			case ACC: printf("A / Implicit\n"); break;
			case ABS: printf("%.3X:%.3X\n", memref(ip+1).nonaryhex(), memref(ip+2).nonaryhex()); break;
			case INDIRECT: printf("(%.3X:%.3X)\n", memref(ip+1).nonaryhex(), memref(ip+2).nonaryhex()); break;
			case AX: printf("%.3X:%.3X,X\n", memref(ip+1).nonaryhex(), memref(ip+2).nonaryhex()); break;
			case AY: printf("%.3X:%.3X,Y\n", memref(ip+1).nonaryhex(), memref(ip+2).nonaryhex()); break;
			case IMMEDIATE: printf("#%.3X\n", memref(ip+1).nonaryhex()); break;
			case INDX: printf("(%.3X:%.3X,X)\n", memref(ip+1).nonaryhex(), memref(ip+2).nonaryhex()); break;
			case INDY: printf("(%.3X:%.3X),Y\n", memref(ip+1).nonaryhex(), memref(ip+2).nonaryhex()); break;
			case XY: printf("X,Y\n"); break;
		}
	}

	/* Increase clock */
	CL = CL + 1;
	if((CL > memrefi(TV_444, TV_441)) == 1) CL = 0;

	/* Address translation (a bit of a mess)*/
	switch(highbits) {
		case ABS: abs1 = &memref(ip+1);
			  abs2 = &memref(ip+2);
			  
			  arg = &memref(*abs1,*abs2); 
			tryte::int_to_word(ip+3, PCH, PCL);
			break;
		case IMMEDIATE: 
			arg = &memref(ip+1); 
			tryte::int_to_word(ip+2, PCH, PCL);
			break;
		case AX: 
			tryte::int_to_word(tryte::word_to_int(memref(ip+1), memref(ip+2)) + X.to_int(),
						cheatarg1, cheatarg2);
			abs1 = &cheatarg1;
			abs2 = &cheatarg2;
			arg = &memref(*abs1, *abs2);
			tryte::int_to_word(ip+3, PCH, PCL);
			break;
		case AY:
			tryte::int_to_word(tryte::word_to_int(memref(ip+1), memref(ip+2)) + Y.to_int(),
						cheatarg1, cheatarg2);
			abs1 = &cheatarg1;
			abs2 = &cheatarg2;
			arg = &memref(*abs1, *abs2);
			tryte::int_to_word(ip+3, PCH, PCL);
			break;
		case ACC: 
			arg = &A; 
			tryte::int_to_word(ip+1, PCH, PCL);
			break;
		case INDX: 
			tryte::int_to_word(tryte::word_to_int(memref(ip+1), memref(ip+2)) + X.to_int(),
					cheatarg1, cheatarg2);
			abs1 = &memref(cheatarg1, cheatarg2);
			abs2 = &memref(tryte::word_to_int(cheatarg1, cheatarg2) + 1);
			arg = &memref(*abs1, *abs2);
			tryte::int_to_word(ip+3, PCH, PCL);
			break;
		case INDY: 
			tryte::int_to_word(tryte::word_to_int(memref(ip+1), memref(ip+2)),
					cheatarg1, cheatarg2);
			abs1 = &memref(cheatarg1, cheatarg2);
			abs2 = &memref(tryte::word_to_int(cheatarg1, cheatarg2) + 1);
			arg = &memref(tryte::word_to_int(*abs1, *abs2) + Y.to_int());
			tryte::int_to_word(ip+3, PCH, PCL);
			break;	
		case INDIRECT:
			abs1 = &memref(tryte::word_to_int(memref(ip+1), memref(ip+2)));
			abs2 = &memref(tryte::word_to_int(memref(ip+1), memref(ip+2))+1);
			arg = &memref(*abs1,*abs2); 
			tryte::int_to_word(ip+3, PCH, PCL);
			break;

		case XY:
			abs1 = &X;
			abs2 = &Y;
			arg = &memref(*abs1, *abs2);
			tryte::int_to_word(ip+1, PCH, PCL);
			break;
	}

	//switch(instruction.to_int()) {
	switch(instruction) {
		case CLV: clv(); break;
		case BRK: soft_brk(); break;
		case RTI: rti(); break;
		case LDA: lda(arg); break;
		case LDX: ldx(arg); break;
		case LDY: ldy(arg); break;
		case STA: sta(arg); break;
		case STX: stx(arg); break;
		case STY: sty(arg); break;
		case TAX: tax(); break;
		case TAY: tay(); break;
		case TXA: txa(); break;
		case TYA: tya(); break;
		case TSX: tsx(); break;
		case TXS: txs(); break;
		case PSH: psh(arg); break;
		case PHP: php(); break;
		case PLL: pll(arg); break;
		case PLP: plp(); break;
		case AND: _and(arg); break;
		case EOR: eor(arg); break;
		case ORA: ora(arg); break;
		case BIT: bit(arg); break;
		case ADD: add(arg); break;
		case CMP: cmp(arg); break;
		case INC: inc(arg); break;
		case INX: inx(); break;
		case INY: iny(); break;
		case DEC: dec(arg); break;
		case DEX: dex(); break;
		case DEY: dey(); break;
		case ASL: asl(arg); break;
		case LSR: lsr(arg); break;
		case ROL: rol(arg); break;
		case ROR: ror(arg); break;
		case JMP: jmp(abs1, abs2); break;
		case JSR: jsr(abs1, abs2); break;
		case RST: rst(); break;
		case CLC: clc(); break;
		case CLI: cli(); break;
		case NOP: nop(); break;
		case SEC: sec(); break;
		case SEI: sei(); break;
		case MLH: mlh(arg); break;
		case MLL: mll(arg); break;
		case DIV: div(arg); break;
		case MOD: mod(arg); break;
		case PLX: plx(); break;
		case PLY: ply(); break;
		case PHX: phx(); break;
		case PHY: phy(); break;
		case JCC: jcc(abs1, abs2); break;
	       	case JCT: jct(abs1, abs2); break;
	       	case JCF: jcf(abs1, abs2); break;
	       	case JEQ: jeq(abs1, abs2); break;
	       	case JNE: jne(abs1, abs2); break; 
		case JLT: jlt(abs1, abs2); break; 
		case JGT: jgt(abs1, abs2); break; 
		case JVC: jvc(abs1, abs2); break; 
		case JVS: jvs(abs1, abs2); break;
		case IVC: ivc(); break; /* invert carry */
		case PRM: prm(arg); break;
		case TSH: tsh(arg); break;
		case BUT: but(arg); break;
		case LAD: lad(abs1, abs2); break;
		case PGT: pgt(arg); break;
		case PGS: pgs(arg); break;
		case CAD: cad(abs1, abs2); break;
		case XAM: xam(arg); break;
		case XAY: xay(); break;
		case XAX: xax(); break;
		case XYX: xyx(); break;
		case PLC: plc(arg); break;
		case ADW: adw(abs1, abs2); break;
		case MLW: mlw(abs1, abs2); break;
		case DVW: dvw(abs1, abs2); break;
		case MDW: mdw(abs1, abs2); break;

		case PAUSE: pause(); break;
		case DEBUG: debug(); break;

		default: printf("%d: unknown opcode %d\n", ip, instruction/*.to_int()*/);
	}

	current_state->heartbeat();

}

void machine::pflag(const tryte& t) {
	if(t.to_int() == 0) P[G] = trit::TRMU;
	else if(t.to_int() > 0) P[G] = trit::TRTRUE;
	else P[G] = trit::TRFALSE;
	P[PR] = t.parity();

	P[C] = t.carry;
}

void machine::pflagnoc(const tryte& t) {
	register int tv = t.to_int();
	if(tv > 0) P[G] = trit::TRTRUE;
	else if(tv == 0) P[G] = trit::TRMU;
	else P[G] = trit::TRFALSE;

	P[PR] = t.parity();
}

void machine::pflagw(const tryte& t, const tryte& t2) {
	register int tv = tryte::word_to_int(t, t2);

	if(tv > 0) P[G] = trit::TRTRUE;
	else if(tv == 0) P[G] = trit::TRMU;
	else P[G] = trit::TRFALSE;

	//TODO: parity?

}
void machine::pause() {
	printf(" -- PAUSE by instruction (before %%%.3X%.3X)\n"
	       "    (resume by pressing pause on keyboard)\n", 
			PCH.nonaryhex(), PCL.nonaryhex());
	set_state(new paused_state());
}

void machine::debug() {
	tryte instruction = memref(PCH, PCL);
	instruction = (instruction << 2) >> 2;
	printf("---DEBUG---\n"
		"PC:%d:%d(%s)\tX:%d/%X\tY:%d/%X\tACC:%d/%X\n", PCH.to_int(), PCL.to_int(), opcode_to_string(instruction.to_int()), X.to_int(), X.nonaryhex(), Y.to_int(), Y.nonaryhex(), A.to_int(), A.nonaryhex());
	printf("P: \tP%d\t V%d\t B%d\t I%d\t G%d\t C%d\n", P[PR].to_int(), P[V].to_int(), P[B].to_int(), P[I].to_int(), P[G].to_int(), P[C].to_int());
	printf("S: %d\n", S.to_int());
	printf("CL: %d\n", CL.to_int());
	printf("TICK: %d\n", SDL_GetTicks());
	fflush(NULL);
}

/* Truck load of nothing */
void machine::nop() { }

/* Loaders */
void machine::lda(tryte *t) { A = *t; pflagnoc(A); }
void machine::ldx(tryte *t) { X = *t; pflagnoc(X); }
void machine::ldy(tryte *t) { Y = *t; pflagnoc(Y); }

/* Storers */
void machine::sta(tryte *t) { *t = A; pflagnoc(*t); }
void machine::stx(tryte *t) { *t = X; pflagnoc(*t); }
void machine::sty(tryte *t) { *t = Y; pflagnoc(*t); }

/* Transferrers */
void machine::tax() { X = A; pflagnoc(X); }
void machine::tay() { Y = A; pflagnoc(Y); }
void machine::txa() { A = X; pflagnoc(A); }
void machine::tya() { A = Y; pflagnoc(A); }
void machine::tsx() { X = S; pflagnoc(X); }
void machine::txs() { S = X; pflagnoc(S); }

/* Exchange operations */
void machine::xam(tryte* t) { 
	tryte tmp = A;
	A = *t;
	*t = tmp;
	pflagnoc(A);
}

void machine::xax() {
	tryte tmp = A;
	A = X;
	X = tmp;
	pflagnoc(A);
}
void machine::xay() {
	tryte tmp = A;
	A = Y;
	Y = tmp;
	pflagnoc(A);
}
void machine::xyx() {
	tryte tmp = X;
	X = Y;
	Y = tmp;
}


/* Pushers */
void machine::psh(tryte* t) { S += -1; memref(SP,S)=*t; pflagnoc(*t); }
void machine::phx() { S += -1; memref(SP,S)=X; pflagnoc(X); }
void machine::phy() { S += -1; memref(SP,S)=Y; pflagnoc(Y); }
void machine::php() { S += -1; memref(SP,S)=P; }

/* Pullers */
void machine::pll(tryte* t) { *t = memref(SP,S); S += 1; pflagnoc(*t); }
void machine::plx() { X = memref(SP,S); S += 1; pflagnoc(X); }
void machine::ply() { Y = memref(SP,S); S += 1; pflagnoc(Y); }
void machine::plp() { P = memref(SP,S); S += 1; }

/* Logical operations */
void machine::_and(tryte* t) { A = A & (*t); pflag(A); }
void machine::eor(tryte* t) { A = A ^ (*t); pflag(A); }
void machine::prm(tryte* t) { A.permute_this(*t); pflag(A); }
void machine::but(tryte* t) { A.but_this(*t); pflag(A); }
void machine::tsh(tryte* t) { A.shift_this(*t); pflag(A); }
void machine::ora(tryte* t) { A = A | *t; pflag(A); }
void machine::bit(tryte* t) { pflag(A & *t); }

/* Arithmetics */
void machine::add(tryte* t) { 
	if((t->to_int() + P[C].to_int()) > 0 && (A + *t + P[C].to_int()) < A) 
		P[V] = trit::TRTRUE;
	else if((t->to_int() + P[C].to_int()) < 0 && (A + *t + P[C].to_int()) > A)
		P[V] = trit::TRFALSE;
	else P[V] = trit::TRMU;
	A = A.to_int() + t->to_int() + P[C].to_int(); 
	pflag(A); 
}

void machine::cmp(tryte* t) {
	if(*t > 0 && (A + *t) < A) 
		P[V] = trit::TRFALSE;
	else if(*t < 0 && (A + *t) > A)
		P[V] = trit::TRTRUE;
	else P[V] = trit::TRMU;

	pflag(A + -(t->to_int()));
}

void machine::inc(tryte* t) { *t = *t + 1; pflag(*t); 
	P[V] = (*t == -364)>0 ? trit::TRTRUE : trit::TRMU; }
void machine::inx() { X = X + 1; pflag(X); 
	P[V] = (X == -364)>0 ? trit::TRTRUE : trit::TRMU; }
void machine::iny() { Y = Y + 1; pflag(Y); 
	P[V] = (Y == -364)>0 ? trit::TRTRUE : trit::TRMU; }
void machine::dec(tryte* t) { *t = *t + -1; pflag(*t);
	P[V] = (*t == 364)>0 ? trit::TRFALSE: trit::TRMU; }
void machine::dex() { X = X + -1; pflag(X);
	P[V] = (X == 364)>0 ? trit::TRFALSE: trit::TRMU; }
void machine::dey() { Y = Y + -1; pflag(Y);
	P[V] = (Y == 364)>0 ? trit::TRFALSE: trit::TRMU; }

/* Shifts and rolls */
void machine::asl(tryte* t) { t->carry = (*t)[5]; *t = *t << 1; pflag(*t);}
void machine::lsr(tryte* t) { t->carry = (*t)[0]; *t = *t >> 1; pflag(*t); }
void machine::ror(tryte* t) {
	trit carry = (*t)[5];
	*t = (*t >> 1) + P[C].to_int()*243; 
	t->carry = carry; 
	pflag(*t); 
}
void machine::rol(tryte* t) { 
	trit carry = (*t)[0];
	*t = (*t << 1) + P[C].to_int(); 
	t->carry = carry; 
	pflag(*t); 
}

/* Jumps */
void machine::jmp(const tryte* high, const tryte* low) { 
	PCH = *high; PCL = *low; 

}
void machine::jsr(const tryte* high, const tryte* low) {
	S += -1; memref(SP, S) = PCH;
	S += -1; memref(SP, S) = PCL;
	PCH = *high; PCL = *low; 
}
void machine::rst() { PCL = memref(SP, S); S += 1;
	PCH = memref(SP, S); S += 1;
}

void machine::jct(const tryte* t, const tryte* t2) {
	if((P[C]).to_int() == 1) jmp(t, t2);
}
void machine::jcf(const tryte* t, const tryte* t2) {
	if((P[C]).to_int() == -1) jmp(t, t2);
}
void machine::jcc(const tryte* t, const tryte* t2) {
	if((P[C]).to_int() == 0) jmp(t, t2);
}
void machine::jeq(const tryte* t, const tryte* t2) {
	if((P[G]).to_int() == 0) jmp(t, t2);
}
void machine::jne(const tryte* t, const tryte* t2) {
	if((P[G]).to_int() != 0) jmp(t, t2);
}
void machine::jgt(const tryte* t, const tryte* t2) {
	if((P[G]).to_int() == 1) jmp(t, t2);
}
void machine::jlt(const tryte* t, const tryte* t2) {
	if((P[G]).to_int() == -1) jmp(t, t2);
}
void machine::jvs(const tryte* t, const tryte* t2) {
	if((P[V]).to_int() == 1) jmp(t, t2);
}
void machine::jvc(const tryte* t, const tryte* t2) {
	if((P[V]).to_int() <= 0) jmp(t, t2);
}

/* Flag manipulation */
void machine::clc() { P[C] = trit::TRMU; }
void machine::cli() { P[I] = trit::TRMU; }
void machine::ivc() { P[C] = !P[C]; }
void machine::clv() { P[V] = trit::TRMU; }
void machine::sec() { P[C] = trit::TRTRUE; }
void machine::sei() { P[I] = trit::TRTRUE; }

/* More arithemetics */
void machine::mlh(tryte* t) { A = A.mlh(*t); pflagnoc(A); }
void machine::mll(tryte* t) { A = (*t)*A; pflagnoc(A); }
void machine::div(tryte* t) { 
	if((*t).to_int() == 0) {
		queue_interrupt(new arith_interrupt());
		return;
	}
	A = A / (*t); pflag(A); 
}
void machine::mod(tryte* t) {
	if((*t).to_int() == 0) {
		queue_interrupt(new arith_interrupt());
		return;
	}
	A = A % (*t); pflag(A);
}

/* Load ADdress */
void machine::lad(const tryte* t, const tryte* t2) {
	X = *t;
	Y = *t2;
}

/* Compare ADdress */
void machine::cad(const tryte* t, const tryte* t2) {
	int V =  tryte::word_to_int(*t, *t2);
	int V2 = tryte::word_to_int(X, Y);

	int diff = V2 - V;
	if(diff != 0) diff = diff / abs(diff);

	pflag(diff);
}

/* BLock (well, page) Transfer 
 *
 * Copy the contents of t:xxx to A:xxx
 * 
 * This, (t=>A), is a bit unintuitive at first, but it really is a lot
 * more useful than the other way around.
 *
 * */
void machine::pgt(const tryte* t) {
	tryte offset = "DDD";

	do {
		memref(A, offset) = memref(*t, offset);
		offset = offset + 1;
	} while(offset.to_int() != -364);
}

/* BLock Set. Set all the trytes of page *t to A. */
void machine::pgs(const tryte* t) {
	tryte offset = "DDD";

	do {
		memref(A, offset) = *t;
		offset = offset + 1;
	} while(offset.to_int() != -364);
}

void machine::plc(tryte* t) {
	tryte& op = memref(SP,S); S += 1; 
	A.comm_op_this(op.to_int(), *t);
	pflagnoc(A); 
}

void machine::adw(const tryte* t, const tryte* t2) {
	tryte& s1 = memref(SP,S); S += 1; 
	tryte& s2 = memref(SP,S); S += 1; 

	int a = tryte::word_to_int(*t, *t2);
	int b = tryte::word_to_int(s2, s1);
	
	tryte::int_to_word(a+b, X, Y);

	if(a+b > 265720) P[C] = trit::TRTRUE;
	else if(a+b < 265720) P[C] = trit::TRFALSE;
	else P[C] = trit::TRMU;

	pflagw(X, Y);
}

void machine::mlw(const tryte* t, const tryte* t2) {
	tryte& s1 = memref(SP,S); S += 1; 
	tryte& s2 = memref(SP,S); S += 1; 

	int a = tryte::word_to_int(*t, *t2);
	int b = tryte::word_to_int(s2, s1);

	tryte::int_to_word(a*b, X, Y);
	pflagw(X, Y);
}

void machine::dvw(const tryte* t, const tryte* t2) {
	tryte& s1 = memref(SP,S); S += 1; 
	tryte& s2 = memref(SP,S); S += 1; 

	int a = tryte::word_to_int(*t, *t2);
	int b = tryte::word_to_int(s2, s1);

	if(b == 0) {
		queue_interrupt(new arith_interrupt());
		return;
	}
	tryte::int_to_word(a/b, X, Y);
	pflagw(X, Y);
}

void machine::mdw(const tryte* t, const tryte* t2) {
	tryte& s1 = memref(SP,S); S += 1; 
	tryte& s2 = memref(SP,S); S += 1; 

	int a = tryte::word_to_int(*t, *t2);
	int b = tryte::word_to_int(s2, s1);

	if(b == 0) {
		queue_interrupt(new arith_interrupt());
		return;
	}
	tryte::int_to_word(a%b, X, Y);
	pflagw(X, Y);
}


void machine::soft_brk() {
	if(P[I].to_int() == 0 && P[B].to_int() == 0) {
		memref("DDD", "DDD") = IRQ_USER;
		brk();
	} /* STUB */
}


/* Break (hardware or software) */
void machine::brk() {
	S += -1; memref(SP,S) = PCH;
	S += -1; memref(SP,S) = PCL;
	S += -1; memref(SP,S) = P;

	S += -1; memref(SP,S) = X;
	S += -1; memref(SP,S) = Y;
	S += -1; memref(SP,S) = A;

	P[B]= trit::TRTRUE;
	P[I]= trit::TRTRUE;

	PCL = memref(364, 363);
	PCH = memref(364, 362);
}

/* Return from Interrupt */
void machine::rti() {
	if((P[B].to_int()) <= 0) printf("RTI outside BRK\n");

	A   = memref(SP,S); S += 1;
	Y   = memref(SP,S); S += 1;
	X   = memref(SP,S); S += 1;
	P   = memref(SP,S); S += 1;
	PCL = memref(SP,S); S += 1;
	PCH = memref(SP,S); S += 1;
	CL = 1;
}


