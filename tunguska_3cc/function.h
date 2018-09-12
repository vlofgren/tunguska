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

/* I'm really sorry what horrible mess the function code
 * really is. 
 */

#include "type.h"
#include "variable.h"
#include "tables.h"
#ifndef function_h
#define function_h

#include "expression.h"
#include <list>
#include <stdexcept>
#include <deque>

class sllist;
class variable_set;
class function;
class symbol;
class lvalue;


/* The function prototype contains only information about the sort of
 * function a function is. It doesn't care about specifics like name or
 * parameter names. */
class function_prototype {
	public:
		function_prototype(const type* rettype, sllist* argtypes);
		function_prototype(const function_prototype& fp) {
			ret_type = fp.get_type();
			arg_types = fp.get_args();	
		}
		bool operator==(const function_prototype& fp) const;
		bool operator!=(const function_prototype& fp) const {
			return !(*this == fp);
		}
		const type* get_type() const { return ret_type; }

		friend class function_call;
		friend class function;
		friend class std::map<std::string, function_prototype>;
	protected:
		function_prototype() { // Default constructor should only be used by std::map
			ret_type = new t_void();
		}
		std::list<type*>& get_args() { return arg_types; }
		const std::list<type*>& get_args() const { return arg_types; }
	private:
		std::list<type*> arg_types;
		const type* ret_type;
};

/* Scope, with local variables
 *
 *
 */

class scope {
	public:
		scope(sllist* variables, int level);
		symbol_table_entry* sym_ref(const char* name);

		const variable_set& get_variable_set() const { return vars; }
	private:
		variable_set vars;		
		int level;
};

/* The function class is a sort of pseudo-singleton that allows the
 * parser to access the current function without having to pass around 
 * a reference to it.
 */

class function {
	public:
		~function() {}

		/* Non-static, instance related stuff */
		const type* get_type() { return prototype.get_type(); }
		variable_set* get_variables() { return &vars; }

		/* Static, singleton stuff */
		static void init_definition(const function_prototype& f, 
					const char* name, 
					const type* rettype, 
					sllist* args, 
					sllist* locals);
		static void finish_definition();

		static function* get_current() { 
			if(!workbench) throw new std::runtime_error("function::get_current() = NULL");
			return workbench; 
		}

		static void set_reentrant() { get_current()->reentrant = true; }

		void do_return(expression* e = 0);

		void add_scope(sllist* variables); 
		void end_scope();

		void bail_scopes(int levels);


		friend class function_call;
		friend class compiler;

		symbol_table_entry* sym_ref(const char* name);

		lvalue* get_symbol(const char* name);
	protected: 
		int init_varstack(sllist* locals); // Returns stack displacement
		void init_variables();
		
	private:
		function(const function_prototype& fp, 
			const char* name, sllist* args);

		variable_set vars;

		int scope_level;
		std::deque<scope*> scopes;

		std::list<variable*> arguments;
		const function_prototype prototype;

		bool reentrant;
		static function* workbench;
		const char* name;
};

#endif
