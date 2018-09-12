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
 
#include "compiler.h"
#include "function.h"
#include "conditional.h"
#include <assert.h>
	       
function_prototype::function_prototype(const type* rettype, sllist* argtypes) {
	ret_type = rettype;
	sllist* node = argtypes;
	while(node) {
		this->arg_types.push_front((type*)node->data);
		node = node->next;
	}
}

bool function_prototype::operator==(const function_prototype& p) const {
	if(*p.get_type() != *ret_type) return false;
	const std::list<type*>& p_args = p.get_args();

	if(p_args.size() != arg_types.size()) return false;

	std::list<type*>::const_iterator i = p_args.begin(), 
					 j = arg_types.begin();
	while(i != p_args.end()) {
		if( **(i) != **(j) ) return false;
		i++; j++;
	}
	return true;
	
}

/* *********************************************************
 *                        Scope
 *
 * ******************************************************* */

scope::scope(sllist* variables, int level) {
	while(variables) {
		vars.add_var((variable*)variables->data);
		variables=variables->next;
	} 
	this->level = level;
}

symbol_table_entry* scope::sym_ref(const char* name) {
	if(vars.is_var(name)) {
		char* namebuf = new char[strlen(name)+10];
		sprintf(namebuf, ".%s.s%3.3d", name, level);
		return new symbol_table_entry(namebuf, vars.get_offset(name),  vars.get_variable(name));
	}
	throw new invalid_argument("scope::sym_ref");
}

lvalue* function::get_symbol(const char* name) {
	symbol_table_entry* te = sym_ref(name);
	if(!te) throw new runtime_error("Unable to look up symbol");

	if(te->get_variable()->is_static()) 
		return new symbol(name); 
	else { 
		
		//   This is hideous --------------------------------vvvvvvvvvvvvvvvvv
		return new dereference(new dynamic_variable_addr(te->effective_name()+1, 
			new t_ptr(te->get_type())), te->get_type());

	}

}

/* *********************************************************
 *                      Function
 *
 * ******************************************************* */
function* function::workbench = NULL;

function::function(const function_prototype& fp, const char* name, sllist* args) : prototype(fp)
{
	this->name = name;
	sllist* node = args;
	scope_level = 0;

	while(node) {
		variable* v = new dynamic_variable((const char*)node->data2, 
						  (type*)node->data, 
						  new null_initializer((type*)node->data));

		this->arguments.push_front(v);
		node= node->next;
	}

	cond_mgr::reset_uid();
}

symbol_table_entry* function::sym_ref(const char* name) {
	std::deque<scope*>::iterator i;

	for(i = scopes.begin(); i != scopes.end(); i++) {
		try {
			return (*i)->sym_ref(name);
		} catch (invalid_argument* e) {
			delete e;
		}
	}
	
	if(vars.is_var(name)) {
		char* namebuf = new char[strlen(name)+2];
		sprintf(namebuf, ".%s", name);
		return new symbol_table_entry(namebuf, vars.get_offset(name),  vars.get_variable(name));
	}
	
	if(variable_set::glob_instance()) {
		variable_set* gi = variable_set::glob_instance();
		return new symbol_table_entry(name, gi->get_offset(name), gi->get_variable(name));
	}

	throw new invalid_argument("function::sym_ref");
}

/* Return from function */
void function::do_return(expression* e) {
	if(!e) {
		if(workbench->vars.get_total_offset()) {
			compiler::instance()->printf("\t\tLDA\t__VSS\n");
			compiler::instance()->printf("\t\tCLC\n");
			compiler::instance()->printf("\t\tADD\t#%d\n", workbench->vars.get_total_offset());
			compiler::instance()->printf("\t\tSTA\t__VSS\n");
			compiler::instance()->printf(";\n");
		}

		if(get_type()->size() != 0) 
			throw new std::runtime_error("Returning with no argument from"
						" function returning non-void.");

		compiler::instance()->printf("\t\tRST\n");
		return;
	}

	/* Fold constants */
	e->reduce();

	/* The return address must be on top of the stack in order to return properly.
	 * Now, unfortunately, the return argument comes pushed on top of the return address,
	 * which means there must be a great deal of stack juggling to get it into place. */
	switch(get_type()->size()) {
		case 0:
			throw new std::runtime_error("Returning non-void from void function");
		case 1:
			e->eval6();
			compiler::instance()->pullA();
			compiler::instance()->pullXY();
			compiler::instance()->pushA();
			compiler::instance()->pushXY();
			break;
		case 2:	
			e->eval12();
			compiler::instance()->pullXY();
			compiler::instance()->printf("\t\tPLL\ttmp+1\n");
			compiler::instance()->printf("\t\tPLL\ttmp\n");
			compiler::instance()->pushXY();
			compiler::instance()->printf("\t\tPSH\ttmp\n");
			compiler::instance()->printf("\t\tPSH\ttmp+1\n");
			break;
		default:
			throw new std::runtime_error("Returning impossible type");
	} 

	/* Return */	
	if(workbench->vars.get_total_offset()) {
		compiler::instance()->printf("\n");
		compiler::instance()->printf("\t\tLDA\t__VSS\n");
		compiler::instance()->printf("\t\tCLC\n");
		compiler::instance()->printf("\t\tADD\t#%d\n", workbench->vars.get_total_offset());
		compiler::instance()->printf("\t\tSTA\t__VSS\n");
	}

	compiler::instance()->printf("\t\tRST\n");
	compiler::instance()->printf("\n");
}

void function::add_scope(sllist* variables) {
	scope_level++;
	scopes.push_front(new scope(variables, scope_level));

	int oldsize = vars.get_total_offset();

	sllist* node = variables;
	/* Create a new scope with local variables variables. The local variables
	 * have a name on the form name.s(scope-level) */
	
	/* Iterate over all variables, add them to variable set */
	while(node) {
		variable* v = (variable*) node->data;
		
		/* Create new variable name */
		char* namebuf = new char[strlen(v->get_name())+10];
		sprintf(namebuf, "%s.s%3.3d", v->get_name(), scope_level);
		if(v->is_static()) 
			v = new static_variable(namebuf, v->get_type(), v->get_initial());
		else 
			v = new dynamic_variable(namebuf, v->get_type(), v->get_initial());

		vars.add_var(v);

		node = node->next;
	}

	/* Increment variable stack index if necessary */
	int newsize = vars.get_total_offset();
	if(newsize - oldsize) {
		if(newsize - oldsize == 1) {
			compiler::instance()->printf("\t\tDEC\t__VSS\n");
		} else {
			compiler::instance()->printf("\t\tLDA\t#%d\n", oldsize - newsize);
			compiler::instance()->printf("\t\tCLC\n");
			compiler::instance()->printf("\t\tADD\t__VSS\n");
			compiler::instance()->printf("\t\tSTA\t__VSS\n");
		}
	}

	/* Initialize all new variables */
	node = sllist::reverse(variables);
	while(node) {
		variable* v = (variable*) node->data;
		
		/* Re-resolve symbol to get an accurate offset.
		 *
		 * I have no idea why this is necessary, dynamic_variable_addr
		 * resolves offset on the fly, so theoretically it should already
		 * do this automagically. It does not, however, for some obscure 
		 * reason.
		 *
		 * TODO: Find out why.
		 * */
		symbol_table_entry* te = sym_ref(v->get_name());
		te->get_variable()->initialize();
		node=node->next;
	} 

	
}

/* Increment __VSS as though jumping out of levels scopes */
void function::bail_scopes(int levels) {
	std::deque<scope*>::iterator i = scopes.begin();
	int total_offset = 0;
	while(levels--) {
		if(i == scopes.end()) throw new runtime_error("Bailing too many scopes\n");
		scope* s = (*i);
		total_offset += s->get_variable_set().get_total_offset();
		i++;
	}

	if(total_offset) {
		if(total_offset == 1) {
			compiler::instance()->printf("\t\tINC\t__VSS\n");
		} else {
			compiler::instance()->printf("\t\tLDA\t__VSS\n");
			compiler::instance()->printf("\t\tCLC\n");
			compiler::instance()->printf("\t\tADD\t#%d\n", total_offset);
			compiler::instance()->printf("\t\tSTA\t__VSS\n");
		}
	}
}

void function::end_scope() {
	scope* s = scopes.front();
	const variable_set& scope_vars = s->get_variable_set();

	bail_scopes(1);

	/* This is incredibly ugly, in so many ways.
	 *
	 * XXX: Clean this up!!!! */
	std::map<std::string, variable*>::const_iterator i = scope_vars.get_variables().begin();
	for(; i != scope_vars.get_variables().end(); i++) {
		symbol_table_entry* te = sym_ref((*i).first.c_str());
		if(!te->get_variable()->is_static())
			vars.rem_var(te->effective_name()+1);
	}

	scopes.pop_front();
}

/* Initialize function definition
 */
void function::init_definition(const function_prototype& f, const char* name, const type* rettype, sllist* args, sllist* locals) {


	if(workbench) {
		throw new runtime_error("Function definition within function.");
	}

	function::workbench = new function(f, name, args);

	compiler::instance()->printf(";  Begin function %s\n", name);
	compiler::instance()->printf("%s:\n", name);

	// Load arguments
	//
	//
	

	workbench->init_varstack(locals);

	// Add variables

	sllist* reverse_locals = sllist::reverse(locals); // Want to keep this

	/* Increment value stack index if necessary */

	compiler::instance()->printf(";  Initialize variables \n");

	while(reverse_locals) {
		variable* v = (variable*)reverse_locals->data;
		assert(v);
		v->initialize();
		reverse_locals = reverse_locals->next;
	}

	compiler::instance()->printf(";  Begin code\n");

}

/* Get arguments from stack, initialize variable stack */
int function::init_varstack(sllist* locals) {

	std::list<variable*>::iterator i = arguments.begin();
	for(i = arguments.begin(); i != arguments.end(); i++) {
		vars.add_var((*i));
	}

	while(locals) {
		workbench->vars.add_var((variable*)locals->data);
		locals=locals->next;
	} 

	int to = get_variables()->get_total_offset();
	if(to) {
		if(to == 1) 
			compiler::instance()->printf("\t\tDEC\t__VSS\n");
		else { 
			compiler::instance()->printf("\t\tLDA\t#%d\n", -to);	
			compiler::instance()->printf("\t\tCLC\n");
			compiler::instance()->printf("\t\tADD\t__VSS\n");
			compiler::instance()->printf("\t\tSTA\t__VSS\n");
		}
	}


	if(!arguments.size()) {
		return 0;
	}

	compiler::instance()->printf(";  Load arguments\n");
	/* The return address is first on stack, for the sake of being
	 * able to return from the function, it must be preserved whilst
	 * peeking underneath it to get the relevant arguments out */
	compiler::instance()->printf("\t\tPLY\n");
	compiler::instance()->printf("\t\tPLL\tA\n");

	compiler::instance()->printf("\t\tLDX\t__VSS\n");

	int disp = 0;
	for(i = arguments.begin(); i != arguments.end(); i++) {

		if((*i)->get_type()->size() == 1) {
			compiler::instance()->printf("\t\tPLL\t__VS+%d, X\n", to + disp--);
		} else if((*i)->get_type()->size() == 2)  {
			compiler::instance()->printf("\t\tPLL\t__VS+%d,X\n", to + disp--);
			compiler::instance()->printf("\t\tPLL\t__VS+%d,X\n", to + disp--);
		}
	}

	/* Restore return address */
	compiler::instance()->printf("\t\tPSH\tA\n");
	compiler::instance()->printf("\t\tPHY\n");

	return 0;
}

/* Initialize all local variables */
void function::init_variables() {
	const std::map<std::string, variable*>& variables = vars.get_variables();
	std::map<std::string, variable*>::const_iterator i;

	for(i = variables.begin(); i != variables.end(); i++) {
		variable* v = (*i).second;
		assert(v);
		v->initialize();
	}
}

// ---------------------------------------------------------------
//

/* This is the tail of a function definition, it does the following:
 *
 * (1) Print RST to the sources
 * (2) Define all local variables of all scopes
 */
void function::finish_definition() {
	assert(workbench);

	if(workbench->vars.get_total_offset()) {
		compiler::instance()->printf("\t\tLDA\t#%d\n", workbench->vars.get_total_offset());
		compiler::instance()->printf("\t\tCLC\n");
		compiler::instance()->printf("\t\tADD\t__VSS\n");
		compiler::instance()->printf("\t\tSTA\t__VSS\n");
	}

	compiler::instance()->printf("\t\tRST\n");
	compiler::instance()->printf(";  Function variables\n");

	const std::map<std::string, variable*>& variables = workbench->vars.get_variables();
	std::map<std::string, variable*>::const_iterator i;
	for(i = variables.begin(); i != variables.end(); i++) {
		variable* v = (*i).second;

		char* namep = (char*) malloc(2+strlen(v->get_name()));
		sprintf(namep, ".%s", v->get_name());

		if(v->is_static()) v->get_initial()->define(namep);

		free(namep);

	}
	compiler::instance()->printf("; End function %s\n\n\n\n", workbench->name);

	workbench = NULL;

}
