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

#include "variable.h"

#include <stdio.h>
#include <map>
#include "compiler.h"

void null_initializer::define(const char* name) {
	compiler::instance()->printf("%s:\t@REST %d\n", name, typ->size());
}

void pod_initializer::define(const char* name) {
	if(value->is_const()) {
		switch(typ->size()) {
			case 1:
				compiler::instance()->printf(
					"%s:\t@DT %d\n", name, 
					value->const_val());
				break;
			case 2:
				compiler::instance()->printf(
					"%s:\t@DW %d\n", name, 
					value->const_val());
				break;
			}
	} else {
		compiler::instance()->printf("%s:\t@REST %d\n", name, typ->size());
	}
}

bool pod_initializer::is_const() {
	return value->is_const();
}

void pod_initializer::initialize(const char* name) {
	lvalue* sym = function::get_current()->get_symbol(name);
	symbol_table_entry* entry = function::get_current()->sym_ref(name);

	/* This isn't quite proper behavior, but it's more desirable than
	 * the alternative. Initialize a static variable if it's initializer
	 * is not constant. Otherwise, only define it's value in memory */
	if(entry->get_variable()->is_static() && value->is_const()) return;

	expression* e = new cleaner(new assigner(sym, value));
	e->autoeval();


	// FIXME: "e" can't be safely deleted, or else "value" will be deleted,
	// and "value" is needed in define(...)
	//
	// Proposed fix: Add a blacklist to safe_delete so that it's possible
	// to filter out deeply nested pointers from the deletion cascade.
}

void composite_initializer::initialize(const char* name) {
	initialize(function::get_current()->get_symbol(name));
}

/* Initialize a struct or an array.
 *
 * This is an extremely long and nasty function.
 */
void composite_initializer::initialize(expression *sym) {
	sllist* node = sllist::reverse(values);	
	const t_struct* ts = dynamic_cast<const t_struct*>(typ);
	const t_array* ta = dynamic_cast<const t_array*>(typ);

	if(ts) { /* Initialize a struct */
		const std::list<const t_struct::field*>& fields = ts->get_field_list();
		std::list<const t_struct::field*>::const_iterator ri = fields.begin();

		lvalue* symlval = dynamic_cast<lvalue*>(sym);

		while(node) {
			if(ri == fields.end()) throw new runtime_error("Mismatching struct size/initializer field count");
			if(node->data) { /* Initialze struct element */
				lvalue* field = symlval->struct_field((*ri)->get_name());
				expression* e = new cleaner(new assigner(field, (expression*)node->data));
				e->autoeval();
			} else { /* Initialize nested struct/array */
				lvalue* field = symlval->struct_field((*ri)->get_name());
				composite_initializer* ci = new composite_initializer(field->typ(), (sllist*)node->data2);
				ci->initialize(field);
			}

			ri++;
			node = node->next;
		}
		if(ri != fields.end()) throw new runtime_error("Mismatching struct size/initializer field count");
	} else if(ta) { /* Initialize an array */
		int element = 0; 
		for(; node; element++,node=node->next) {
			if(element >= ta->dimension()) throw new runtime_error("Array initializer too big");

			if(node->data) { /* Initialize element */
				lvalue* field = array_index(sym, new constant(element));
				expression* e = new cleaner(new assigner(field, (expression*)node->data));
				e->autoeval();
			} else { /* Initialize nested struct/array */
				expression* field = array_index(sym, new constant(element));
				composite_initializer* ci = new composite_initializer(field->typ(), (sllist*)node->data2);
				ci->initialize(field);
			}
		}
		
	} else throw new std::runtime_error("What are you trying to initialize?!");
}

bool composite_initializer::is_const() {  return check_const(values);  }
bool composite_initializer::check_const(sllist* values) {
	while(values) {
		if(values->data) 
			if(!((expression*)values->data)->is_const()) return false;
		if(values->data2) 
			if(!check_const((sllist*)values->data2)) return false;

		values = values->next;
	}

	return true;
}

void composite_initializer::define(const char* name) {
	if(!is_const()) {
		compiler::instance()->printf("%s:\t@REST %d\n", name, typ->size());
	} else {
		compiler::instance()->printf("%s:\n", name);
		define();
	}
}

void composite_initializer::define() {
	sllist* node = sllist::reverse(values);	
	const t_struct* ts = dynamic_cast<const t_struct*>(typ);
	const t_array* ta = dynamic_cast<const t_array*>(typ);

	if(ts) { /* Initialize a struct */
		const std::list<const t_struct::field*>& fields = ts->get_field_list();
		std::list<const t_struct::field*>::const_iterator ri = fields.begin();

		while(node) {
			if(ri == fields.end()) throw new runtime_error("Mismatching struct size/initializer field count");
			if(node->data) { /* Initialze struct element */
				expression* dv = (expression*)node->data;
				if((*ri)->get_type()->size() == 1) 
					compiler::instance()->printf("@DT %d\n", dv->const_val());
				else
					compiler::instance()->printf("@DW %d\n", dv->const_val());

			} else { /* Initialize nested struct/array */
				composite_initializer* ci = new composite_initializer((*ri)->get_type(), (sllist*)node->data2);
				ci->define();
			}

			ri++;
			node = node->next;
		}
		if(ri != fields.end()) throw new runtime_error("Mismatching struct size/initializer field count");
	} else if(ta) { /* Initialize an array */
		int element = 0;
		for(; node && element < ta->dimension(); element++,node=node->next) {

			if(node->data) { /* Initialize element */
				expression* dv = (expression*)node->data;
				if(ta->dereference()->size() == 1) 
					compiler::instance()->printf("@DT %d\n", dv->const_val());
				else
					compiler::instance()->printf("@DW %d\n", dv->const_val());
			} else if(node->data2) { /* Initialize nested struct/array */
				composite_initializer* ci = new composite_initializer(ta->dereference(), (sllist*)node->data2);
				ci->define();
			}
		}
		if(element < ta->dimension()) compiler::instance()->printf("@REST %d\n", (ta->dimension() - element) * ta->dereference()->size());
	} else {
		typ->id();
		throw new std::runtime_error("What are you trying to define?!");
	}

}

variable::variable(const char* name, const type* t, expression* initial) {
	this->name = name;
	this->t = t;
	this->initial = new pod_initializer(t, initial);
	this->Extern = false;
}

variable::variable(const char* name, const type* t, initializer* initial) {
	this->name = name;
	this->t = t;
	if(initial)
		this->initial = initial;
	else this->initial = new null_initializer(t);
	this->Extern = false;
}


/* Initialize variable */
void variable::initialize() {
	initial->initialize(name);
}

void static_variable::initialize() {
	// Define initializes. 
}


variable_set* variable_set::g_instance = NULL;

void variable_set::add_var(variable* v) {
	assert(v);
	if(is_var(v->get_name())) {
		if(v->is_extern()) return;
		else if(variables[v->get_name()]->is_extern()) {
			goto aaah_it_was_just_external;
		}

		char* buffer = new char[255];
	       	sprintf(buffer, "Variable %s added twice!", v->get_name());

		throw new runtime_error(buffer);
	}
aaah_it_was_just_external:
	variables[v->get_name()] = v;
	offsets[v->get_name()] = offset;
	offset += v->get_displacement();
}

void variable_set::debug() {
	std::map<std::string, variable*>::iterator i = variables.begin();
	for(; i != variables.end(); i++) {
		printf("\t%s[%d]\n", (*i).first.c_str(), offsets[(*i).first.c_str()]);
	}
}

bool variable_set::is_var(const char* name) {
	if(variables.find(name) == variables.end()) return false;
	else return true;
}

void variable_set::rem_var(const char* name) {
	std::map<std::string, variable*>::iterator vi = variables.find(name);
	std::map<std::string, int>::iterator oi = offsets.find(name);
	if(vi != variables.end()) {
		offset-=(*vi).second->get_displacement();
		variables.erase(vi);
	} else {
		printf("rem_var('%s'): Failed\n", name);
	}
	if(oi != offsets.end()) {
		offsets.erase(oi);
	} else {
		printf("rem_var('%s'): Failed\n", name);
	}

	
}

variable* variable_set::get_variable(const char* name) {
	if(variables.find(name) != variables.end()) {
		return variables[name];
	}
	char* emsg = new char[100];
	snprintf(emsg, 100, "Failed to look up variable %s", name);
//	*(char*)NULL = 1;
	throw new invalid_argument(emsg);
}
