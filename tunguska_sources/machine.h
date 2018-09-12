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

#include "tryte.h"
#include <stdlib.h>
#include <string.h>
#include <queue>
#include <map>
//#include "display.h"
#include "interrupt.h"
#include "memory.h"
#ifndef machine_h
#define machine_h

char asciitoternary(char a);
char ternarytoascii(int a);

class machine : public memory {
	public:
		machine();
		~machine();

		/* Instruction codes */
		enum {
		CLV=-40,     BRK, RTI, LDA, LDX, LDY, STA, STX, STY, TAX,
			TAY, TXA, TYA, TSX, TXS, PSH, PHP, PLL, PLP, AND,
			EOR, ORA, BIT, ADD, CMP, INC, INX, INY, DEC, DEX,
			DEY, ASL, LSR, ROL, ROR, JMP, JSR, RST, CLC, CLI, 
/*Note NOP=0 b.design*/	NOP, SEC, SEI, MLH, MLL, DIV, MOD, PLX, PLY, PHX,
		       	PHY, JCC, JCT, JCF, JEQ, JNE, JLT, JGT, JVC, JVS, 
			IVC, PRM, TSH, BUT, LAD, PGT, PGS, CAD, XAM, XAX,
			XAY, XYX, PLC, ADW, MLW, DVW, MDW, 

			PAUSE  = 39, DEBUG = 40
		}; 

		/* Addressing modes */
		enum {  ABS=-4, IMMEDIATE=-3, 
			AX=-2, AY=-1, ACC=0, IMPLICIT=0,
	       	  	INDX=1, INDY=2, INDIRECT = 3, XY = 4 };

		/* Process flag shifts */
		enum { C = 0, G = 1, I = 2, B = 3, V = 4, PR = 5 };

		/* Process single instruction */
		void instruction();

		void reset() {
			A = X  = Y = P = S = PCL = PCH = CL = 0;
		}


		/* Quick OP-code-assembly. Takes addressing mode and operation and
		 * bakes it together into a complete opcode */
		static tryte qop(tryte mode, tryte op) { return (mode << 4) + op; }
	
		tryte A, X, Y, P, S, SP, PCL, PCH, CL;

		friend int main(int argc, char* argv[]);

		/* Assembler access */
		friend int addop(int opcode, int shift, machine& m, int pc);

		/* Peripherials */
		friend class display;
		friend class kboard;


		/* Very handy for debugging purposes */
		const char* opcode_to_string(int opcode) const;

		bool trace;

		enum { IRQ_KEYBOARD = 0,
		 	IRQ_CLOCK = 1,
	       		IRQ_KEYBREAK = 2,
			IRQ_ARITH = 3,	/* Arithmetic error, not implemented */
			IRQ_USER = 4,
			IRQ_DEBUG
		};


		/* Add an interrupt to the queue */
		void queue_interrupt(interrupt* i) { interrupt_queue->push(i); }

		class state {
			public:
				virtual ~state() {};
				virtual bool is_running() const = 0;
				virtual bool is_tracing() const = 0;
				virtual void heartbeat() = 0;
			private:
		};

		class paused_state : public state {
			public:
				paused_state() {}
				virtual ~paused_state() {}
				virtual bool is_running() const { return false; }
				virtual bool is_tracing() const { return false; }
				virtual void heartbeat() {}
		};

		class running_state : public state {
			public:
				running_state() {}
				virtual ~running_state() {}
				virtual bool is_running() const { return true; }
				virtual bool is_tracing() const { return false; }
				virtual void heartbeat() {}
		};

		class step_state : public state {
			public:
				step_state(int steps) {
					this->steps = steps;
					this->step = 0;
				}
				virtual ~step_state() {}
				virtual bool is_running() const { return step < steps; }
				virtual bool is_tracing() const { return is_running(); }
				virtual void heartbeat() { if(is_running()) step++; }
			private:
				int step, steps;

		};

		void set_state(state* s) { delete current_state; current_state = s; }
		const state* get_state() const { return current_state; }


	protected:
		std::map<int, const char*> translation_table;
		void init_translation_table();

		void run_interrupt();

		/* set processor flags */
		void pflag(const tryte& t);
		/* ... without touching carry */
		void pflagnoc(const tryte& t);
		/* ... or for a whole word */
		void pflagw(const tryte& t, const tryte& t2);

		/* debug opcode */
		void debug();
		void pause();

		/* All instruction functions */
		inline void nop(); 

		inline void lda(tryte *t);
		inline void ldx(tryte *t); 
		inline void ldy(tryte *t); 
		inline void sta(tryte *t); 
		inline void stx(tryte *t); 
		inline void sty(tryte *t); 
		
		inline void tax(); 
		inline void tay();
		inline void txa(); 
		inline void tya(); 
		inline void tsx(); 
		inline void txs();

		inline void xam(tryte* t); 
		inline void xax(); 
		inline void xay(); 
		inline void xyx();

		inline void psh(tryte* t); 
		inline void php(); 
		inline void phx(); 
		inline void phy();
		inline void pll(tryte* t); 
		inline void plp(); 
		inline void plx(); 
		inline void ply();

		inline void add(tryte* t); 
		inline void cmp(tryte* t); 
		inline void inc(tryte* t);
		inline void inx(); 
		inline void iny();
		inline void dec(tryte* t); 
		inline void dex(); 
		inline void dey();

		inline void _and(tryte *t);
		inline void eor(tryte *t);
		inline void ora(tryte *t);
		inline void bit(tryte* t);
		inline void tsh(tryte* t); 
		inline void prm(tryte* t);
		inline void but(tryte* t);

		inline void asl(tryte* t); inline void lsr(tryte* t);
		inline void rol(tryte* t); inline void ror(tryte* t);
		inline void jmp(const tryte* low, const tryte* high); 
		inline void jsr(const tryte* low, const tryte* high); 
		inline void rst(); 

		inline void jct(const tryte *t, const tryte* t2); 
		inline void jcf(const tryte *t, const tryte* t2); 
		inline void jcc(const tryte* t, const tryte* t2); 
		inline void jeq(const tryte* t, const tryte* t2); 
		inline void jgt(const tryte *t, const tryte* t2); 
		inline void jne(const tryte* t, const tryte* t2); 
		inline void jlt(const tryte* t, const tryte* t2); 
		inline void jvc(const tryte* t, const tryte* t2); 
		inline void jvs(const tryte* t, const tryte* t2);

		inline void clc(); inline void cli(); 
		inline void clv(); inline void ivc();
		inline void sec(); inline void sed(); 
		inline void sei();

		inline void lad(const tryte* t, const tryte* t2);
		inline void cad(const tryte* t, const tryte* t2);

		inline void pgt(const tryte* t);
		inline void pgs(const tryte* t);

		inline void mlh(tryte* t); inline void mll(tryte* t); 
		inline void div(tryte* t); inline void mod(tryte* t);
		inline void plc(tryte* t);
		inline void adw(const tryte* t, const tryte* t2);
		inline void mlw(const tryte* t, const tryte* t2);
		inline void dvw(const tryte* t, const tryte* t2);
		inline void mdw(const tryte* t, const tryte* t2);

		inline void rti();

		void soft_brk();

		/* Instructions end here */
		void brk(); 
		void branch(int p);

		std::queue<interrupt*>* interrupt_queue;

		state* current_state;

	private:
		machine(const machine &m) { throw new std::runtime_error("Machine isn't copyable"); }
};

#endif
