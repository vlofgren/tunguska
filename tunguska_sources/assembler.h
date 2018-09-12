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


#ifndef assembler_h
#define assembler_h
#include "machine.h"

#include <map>
#include <stack>
#include <string>
using namespace std;

typedef enum {
	OP_IMPLICIT,
	OP_ACC,
	OP_XY,
	OP_IMMEDIATE,
	OP_ABSOLUTE,
	OP_ABSOLUTE_X,
	OP_ABSOLUTE_Y,
	OP_INDIRECT,
	OP_INDIRECT_X,
	OP_INDIRECT_Y
} op_mode;

typedef enum { INITIAL_SWEEP, FINAL_SWEEP } state_t;


/* Internal representation for source files, used in
 * inclusion. */
class source {
	public:
		source(const char* filename);
		source(const source& s);

		int lineno() const { return line; }
		void inc_line() { line ++; }

		ifstream* get_fs() { return filestream; }
		const string& get_filename() { return filename; }
	private:
		string filename;
		ifstream* filestream;
		int line;
};

class assembler {
	public:
		assembler() { 
			pc = 0; 
	       	}
		~assembler();
		

		/* interface used by the parser */
		void org(int p);
		void equ(const char* c, int p);
		void ldef(const char* c);
		void dt(int v);
		void dtstring(const char* s);
		void dw(int v);
		int label_eval(const char* c);
		void addop(char* c, op_mode mode, int val);
		void inc/*lude*/(const char* s);

		/* Program counter */
		int get_pc() const { return pc; }
		void set_pc(int p) { this->pc = p; }

		
		unsigned int strtoopc(const char* s) const;

		void save(const char* outfile) { m.save(outfile); }
		void verbose_info();

		/* Read source files from argument list */
		void read_files(int argc, int find, char** argv);

		/* During INITIAL_SWEEP, label lookup failures are ignored */
		state_t get_state() const { return state; }
		void set_state(state_t s) { state = s; }

		int lineno();
		void inc_line();

		/* Only one assembler, lexer instance at a time -- accessible
		 * through these functions*/
		static assembler* instance();
		static yyFlexLexer* lexer();

	private:
		map<string, int> labels;	/* Labels */
		map<string, int> variables;	/* Variables */
		multimap<string, string> label_tracker;
		multimap<string, string> variable_tracker;

		machine m;			/* The machine */
		int pc;				/* Program counter */
		state_t state;			/* Assembler sweep (INITIAL or FINAL) */

		static yyFlexLexer* l_instance;
		static assembler* a_instance;

		/* Used in @INC */
		stack<source> sources;
};

int nontriplet(char* s);
int nonsextet(char* s);
int floatval(char* s);
int lowtryte(int v);
int hightryte(int v);

#endif
