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

#include "type.h"
#include "expression.h"

#ifndef variable_h
#define variable_h

class expression;
class function;
class sllist;
#include <map>
#include <string>

/*  INITIALIZERS 
 * =====================================
 *  Initializing and defining variables of most types,
 *  plain old data, stucts, arrays, etc.
 */

class initializer {
public:
	initializer() {}
	virtual ~initializer() {}

	virtual void initialize(const char* name) = 0;
	virtual void define(const char* name) = 0;
	virtual bool is_const() = 0;
};

/* Non-initializer, just does define */
class null_initializer : public initializer {
public:
	null_initializer(const type* t) { typ = t; }
	virtual ~null_initializer() {}

	virtual void initialize(const char* name) {}
	virtual void define(const char* name);
	virtual bool is_const() { return true; }
private:
	const type* typ;
};

/* Plain old data initializer, that is: int, char or pointer */
class pod_initializer : public initializer {
public:
	pod_initializer(const type* t, expression* e) { 
		typ = t; 
		value = e;
	}
	virtual ~pod_initializer() {}

	virtual void initialize(const char* name);
	virtual void define(const char* name);
	virtual bool is_const();
private:
	const type* typ;
	expression* value;
};

/* Struct, array initializer */
class composite_initializer : public initializer {
public:
	composite_initializer(const type* t, sllist* l) { 
		typ = t; 
		values = l;
	}
	virtual ~composite_initializer() {}

	virtual void initialize(expression* e);
	virtual void initialize(const char* name);
	virtual void define(const char* name);
	virtual bool is_const();
private:
	virtual void define();
	virtual bool check_const(sllist* values);
	const type* typ;
	sllist* values;
};

/*  VARIABLE
 * =====================================
 * Mostly translates between internal and external representation
 * of varaibles, both dynamic and static variables.
 */

class variable {
	public:
		variable(const char* name, const type* t, expression* initial);
		variable(const char* name, const type* t, initializer* initial = NULL);
		virtual ~variable() {}

		virtual const char* get_name() const { return name; }
		virtual const type* get_type() const { return t; }

		virtual initializer* get_initial() const { return initial; }

		
		virtual bool is_static() const = 0;

		virtual void set_extern(bool externness)
			{ Extern = externness; }
		virtual bool is_extern() const { return Extern; }

		virtual void initialize();

		virtual int get_displacement() const = 0;
	protected:
		const char* name;
		const type* t;
		bool Extern;
		initializer* initial;
};

class static_variable : public variable {
	public:
		static_variable(const char* name, const type* t, expression* initial) : variable(name, t, initial) {
			if(!this->initial->is_const()) throw new std::runtime_error("Not a constant initializer");
		};
		static_variable(const char* name, const type* t, initializer* initial = NULL) : variable(name, t, initial) {
			if(!this->initial->is_const()) throw new std::runtime_error("Not a constant initializer");
		}; 
		virtual ~static_variable() {}

		virtual bool is_static() const { return true; }
		virtual void initialize();

		virtual int get_displacement() const { return 0; }

};

class dynamic_variable : public variable {
	public:	
		dynamic_variable(const char* name, const type* t, expression* initial) : variable(name, t, initial) {};
		dynamic_variable(const char* name, const type* t, initializer* initial = NULL) : variable(name, t, initial) {}; 
		virtual ~dynamic_variable() {}

		virtual bool is_static() const { return false; }

		virtual int get_displacement() const { return t->size(); }
};

class expression;

/*  VARIABLE SET
 * =====================================
 * Quasi stack frame / variable collection.
 */


class variable_set {
	public:
		variable_set() { offset = 0; }

		void add_var(variable* v);
		bool is_var(const char* name);

		void rem_var(const char* name); 

		variable* get_variable(const char* name);

		void debug();

		static variable_set* glob_instance() {
			if(g_instance == NULL) {
				g_instance = new variable_set();
			}
			return g_instance;
		}

		int get_offset(const char* name) const { return offsets[name]; }
		int get_total_offset() const { return offset; }
	protected:
		friend class function; 	
		std::map<std::string, variable*>& get_variables() { 
			return variables;
		}
		const std::map<std::string, variable*>& get_variables() const { 
			return variables;
		}
	private:
		int offset;
		static variable_set* g_instance;

		std::map<std::string, variable*> variables;
		mutable std::map<std::string, int> offsets;
};

#endif
