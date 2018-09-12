/*  3cc - Ternary C Compiler for Tunguska
 *  Copyright (C) 2008  Viktor Lofgren
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "expression.h"
#include "type.h"
#include "function.h"
#include "memory.h"
#include "sllist.h"
#include <map>
#include <string>

#include <FlexLexer.h>
#ifndef compiler_h
#define compiler_h

class yyFlexLexer;

class type;

namespace cio {
	void op(const char* text, const char* arg = 0); 
	void label(const char* name);
}



/* The compiler class is a singleton (yet again because of parser convenience),
 * and as close to a "main class" as you're going to get. */
class compiler {
	public:
		compiler();
		virtual ~compiler();
		static compiler* instance() {
			if(!compiler_instance) {
				compiler_instance = new compiler();
			}
			return compiler_instance;
		}

		void compile_file(const char* file);
		void set_output(const char* outfile);

		void pragma(const char* command, int data);
		void pragma(const char* command);
		void begin();

		/* These functions keep track of pushes and pops of
		 * A and XY, so that tautological constructs like
		 * PSH  A, PLL  A
		 * can be filtered out
		 *
		 * TODO: Extend this to filter out more complex
		 * stuff, PSH foo, PLL A, STA bar => LDA foo, STA bar, etc. */
		void pushA(); 
		void pushXY(); 
		void pullA(); 
		void pullXY();

		/* File output */
		int printf(const char* arg, ...); 

		/* Declare a function, can be done repeatedly for any given
		 * function, so long as the prototype is the same */
		void decl_fun(const char* name, const function_prototype& fp);

		/* Access function prototype by name */
		const function_prototype& fun_ref(const char* name) const;

		void decl_struct(const char* name, t_struct* typ);
		t_struct* struct_ref(const char* name) const;

		/* Access symbol by name */
		symbol* sym_ref(const char* name);

		memory_mgr* get_mmgr() { return &mmgr; }
		yyFlexLexer* get_lexer() { return lexer; }

		const char* get_file() const { return filename; }
		int get_line() const { return line; }
		int advance_line() { return ++line; }
		void set_effective_location(const char* file, int lineno) {
			if(file) effective_filename = strdup(file);
			line = lineno;
		}

		const char* get_effective_file() { return effective_filename; }
	private:

		memory_mgr mmgr;
		int origin;
		std::map<std::string, function_prototype> functions;
		std::map<std::string, t_struct*> structs;
		static compiler* compiler_instance;
		bool PA, PXY;

		int line;
		const char* effective_filename;

		const char* filename;
		yyFlexLexer* lexer;

		FILE* output;
};

#endif
