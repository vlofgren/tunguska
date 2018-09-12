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
#include <stdlib.h>
#include "compiler.h"
#include <exception>
#include <vector>

/* FIXME: This function is ugly in so many ways, design wise
 * and implementation wise */
std::list<expression*> expression::allocated_expressions;
expression* expression::dearray(expression* a) {
	const t_array* t = dynamic_cast<const t_array*>(a->typ());
	if(!t) return a;

	word_caster* wc = dynamic_cast<word_caster*>(a);
	if(wc) a = wc->get_base();

	symbol* oldsym = dynamic_cast<symbol*>(a);
	if(oldsym) {
		symbol* newsym = new symbol(oldsym->get_ambiguous_name(), t->dereference(), oldsym->get_offset());
		return new reference(newsym);
	}

	dereference* der = dynamic_cast<dereference*>(a);
	if(der) {
		return new word_caster(der->source(), new t_ptr(t->dereference()));
	}

	a->debug_print_tree();
	puts(":");

	throw new runtime_error("Dearray failed");
}

lvalue* ptr_index(expression* a, expression* b) {
	const type* base = a->typ()->dereference();
	return new dereference(new adder(a, new mul(b, new constant(base->size()))));
}

lvalue* array_index(expression* a, expression* b) {
	const type* base = a->typ()->dereference();

	if(b->is_const()) {
		lvalue* lva = dynamic_cast<lvalue*>(a);
		if(!lva) throw new runtime_error("Bad array");

		return lva->shift(b->const_val()*base->size(), base);
	}

	expression* source = expression::dearray(a);

	return new dereference(new adder(source, new mul(b, new constant(base->size()))), base);
}

lvalue* struct_ptr_field(expression* e, const char* field) {
	if(!e->typ()->can_deref())
		throw new std::runtime_error("Bad field dereference");

	const t_struct* source_type = dynamic_cast<const t_struct*>(e->typ()->dereference());
	const type* field_type = source_type->get_field(field)->get_type();
	expression* offset = new constant(source_type->get_field(field)->get_offset());
	return new dereference(new adder(e, offset), field_type);
}

void cleaner::eval6() const {
	if(a) {
		a->eval6();
		compiler::instance()->pullA();
	}
}
void cleaner::eval12() const {
	if(a) {
		a->eval12();
		compiler::instance()->pullXY();
	}
}

void word_caster::eval12() const {
	a->eval12();
}
void word_caster::eval6() const {
	a->eval12();
	compiler::instance()->pullXY();
	compiler::instance()->printf("\t\tPHY\n");
}

void char_caster::eval6() const {
	a->eval6();

}
void char_caster::eval12() const {
	a->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPSH\t#0\n");
	compiler::instance()->printf("\t\tPSH\tA\n");

}

void constant::eval6() const {
	compiler::instance()->printf("\t\tPSH\t#%d\n", value);
}

void constant::eval12() const {
	compiler::instance()->printf("\t\tLAD\t%d\t; const word\n", value);
	compiler::instance()->pushXY();
}

void string_constant::eval12() const {
	compiler::instance()->printf("\t\tLAD\t%s+%d\t; const char*\n", compiler::instance()->get_mmgr()->name(), offset);
	compiler::instance()->pushXY();
}

void string_constant::eval6() const {
	eval12();
	compiler::instance()->pullXY();
	compiler::instance()->printf("\t\tPHY\n");
}

function_call::function_call(const char* name, const function_prototype& f, sllist* arg) : fun(f) {
	sllist* node = arg;
	this->name = name;

	while(node) {
		args.push_back((expression*) node->data);
		node = node->next;
	}

	if(args.size() != fun.get_args().size()) {
		printf("Call to %s with mismatching number of arguments\n",
			name);
	} 
}

void function_call::eval6() const {
	std::list<expression*>::const_iterator i = args.begin();
	std::list<type*>::const_reverse_iterator j = fun.get_args().rbegin();
	while(j != fun.get_args().rend() && i!=args.end()) {
		expression* e = dearray(*i);

		if((*j)->size() == 1) {
			e->eval6();
		} if((*j)->size() == 2) {
			e->eval12();
		}
		j++; i++;
	}

	compiler::instance()->printf("\t\tJSR\t%s\n", name);
	if(fun.get_type()->size() == 2) {
		compiler::instance()->pullXY();
		compiler::instance()->printf("\t\tPHY\n");
	} else if(fun.get_type()->size() == 0) {
		compiler::instance()->pushA();
	}
}

void function_call::eval12() const {
	std::list<expression*>::const_iterator i = args.begin();
	std::list<type*>::const_iterator j = fun.get_args().begin();
	while(j != fun.get_args().end() && i!=args.end()) {
		if((*j)->size() == 1) {
			(*i)->eval6();
		} if((*j)->size() == 2) {
			(*i)->eval12();
		}
		j++; i++;
	}

	compiler::instance()->printf("\t\tJSR\t%s\n", name);
	if(fun.get_type()->size() == 1) {
		compiler::instance()->pullA();
		compiler::instance()->printf("\t\tPSH\t#0\n");
		compiler::instance()->printf("\t\tPSH\tA\n");
	} else if(fun.get_type()->size() == 0) {
		compiler::instance()->pushXY(); // Cleaner removes this automagically
	}

}


void dynamic_variable_addr::eval6() const {
	compiler::instance()->printf("\t\tLDX\t__VSS\n");
	compiler::instance()->printf("\t\tLAD\t__VS+%d,X\n", get_offset());
	compiler::instance()->printf("\t\tPHY\n");
}

void dynamic_variable_addr::eval12() const {
	compiler::instance()->printf("\t\tLDX\t__VSS\n");
	compiler::instance()->printf("\t\tLAD\t__VS+%d,X\n", get_offset());
	compiler::instance()->pushXY();
}

const char* symbol::get_name() const {
	const symbol_table_entry* info = symbol_table::lookup(name);
	const char* buf = info->effective_name();
	char* ename;
	if(offset) {
		ename = new char[strlen(buf)+10];
		sprintf(ename, "%s+%d", buf, offset);
	} else {
		ename = strdup(buf);
	}
	delete info;
	return ename; 
}

const type* symbol::typ() const {
	if(sym_type) return sym_type;

	const symbol_table_entry* info = symbol_table::lookup(name);
	const type* typ = info->get_type();
	delete info;
	return typ; 
}

lvalue* symbol::shift(int trytes, const type* t) {
	if(!t) t = sym_type;
	return new symbol(name, t, offset + trytes);
}

expression* symbol::refer() { 
	return new reference(this); 
}

lvalue* symbol::struct_field(const char* field) {
	const t_struct* struct_type = dynamic_cast<const t_struct*>(typ());
	if(!struct_type) throw new runtime_error("Not a struct.");
	const type* field_type = struct_type->get_field(field)->get_type();

	int offset = struct_type->get_field(field)->get_offset();
	return new symbol(get_ambiguous_name(), field_type, offset + get_offset());
}

void symbol::eval6() const {
	switch(typ()->size()) {
		case 0:
			printf("Error: Evaluating void %s\n", get_name());
			exit(EXIT_FAILURE);
		case 1:
			compiler::instance()->printf("\t\tPSH\t%s\n", get_name());
			break;
		case 2:
			compiler::instance()->printf("\t\tPSH\t%s+1\n", get_name());
			break;
	}
}

void symbol::eval12() const {
	switch(typ()->size()) {
		case 0:
			printf("Error: Evaluating void %s\n", get_name());
			exit(EXIT_FAILURE);
		case 1:
			compiler::instance()->printf("\t\tPSH\t#0\n");
			compiler::instance()->printf("\t\tPSH\t%s\n", get_name());
			break;
		case 2:
			compiler::instance()->printf("\t\tPSH\t%s\n", get_name());
			compiler::instance()->printf("\t\tPSH\t%s+1\n", get_name());
			break;
	}
}

void reference::eval6() const {
	compiler::instance()->printf("\t\tLAD\t%s\n", sym->get_name());
	compiler::instance()->printf("\t\tPHY\n");
}

void reference::eval12() const {
	compiler::instance()->printf("\t\tLAD\t%s\n", sym->get_name());
	compiler::instance()->pushXY();
}

lvalue* dereference::shift(int trytes, const type* t) {
	if(!t) t = Type;
	
	dynamic_variable_addr* dva = dynamic_cast<dynamic_variable_addr*>(a);
	if(dva) { /* DVAs can be optimized in a very nice fashion */
		dva = new dynamic_variable_addr(dva->get_name(), new t_ptr(t), dva->get_displacement() + trytes);
		return new dereference(dva, t);
	}
	expression* source = new word_caster(a, new t_ptr(t));

	return new dereference(new adder(source, new constant(trytes)), t);
}

lvalue* dereference::struct_field(const char* field) {
	const t_struct* struct_type = dynamic_cast<const t_struct*>(typ());
	if(!struct_type) throw new runtime_error("Not a struct.");
	const type* field_type = struct_type->get_field(field)->get_type();

	return shift(struct_type->get_field(field)->get_offset(), field_type);
}
void dereference::assign6(expression* e) const {
	dynamic_variable_addr* da = dynamic_cast<dynamic_variable_addr*>(source());
	if(da) {
		int offset = da->get_offset();

		switch(typ()->size()) {
			case 1:
				e->eval6();
				compiler::instance()->pullA();
				compiler::instance()->printf("\t\tLDX\t__VSS\n");
				compiler::instance()->printf("\t\tSTA\t__VS+%d,X\n", offset);
				compiler::instance()->pushA();
				break;
			case 2:
				e->eval12();
				compiler::instance()->pullXY();
				compiler::instance()->printf("\t\tTXA\n");
				compiler::instance()->printf("\t\tLDX\t__VSS\n");
				compiler::instance()->printf("\t\tSTA\t__VS+%d,X\n", offset);
				compiler::instance()->printf("\t\tSTY\t__VS+%d,X\n", offset+1);
				compiler::instance()->printf("\t\tPHY\n");
				break;
			default: 
				throw new runtime_error("Assigning non-plain-data");
		}
	} else {
		if(typ()->size() == 1) {
			e->eval6();
			source()->eval12();
			compiler::instance()->printf("\t\tPLL\ttmp+1\n");
			compiler::instance()->printf("\t\tPLL\ttmp\n");
			compiler::instance()->printf("\t\tPLL\t(tmp)\n");
			compiler::instance()->printf("\t\tPSH\t(tmp)\n");
		} else if(typ()->size() == 2) {
			e->eval12();
			source()->eval12();
			compiler::instance()->printf("\t\tPLL\ttmp+1\n");
			compiler::instance()->printf("\t\tPLL\ttmp\n");
	
			compiler::instance()->printf("\t\tLDY\t#1\n");
			compiler::instance()->printf("\t\tPLL\t(tmp),Y\n");
			compiler::instance()->printf("\t\tPLL\t(tmp)\n");
			compiler::instance()->printf("\t\tPSH\t(tmp),Y\n");
		} else throw new runtime_error("Assigning non-plain-data");
	}
}

void dereference::assign12(expression* e) const {
	dynamic_variable_addr* da = dynamic_cast<dynamic_variable_addr*>(source());

	if(da) {
		int offset = da->get_offset();

		switch(typ()->size()) {
			case 1:
				e->eval6();
				compiler::instance()->pullA();
				compiler::instance()->printf("\t\tLDX\t__VSS\n");
				compiler::instance()->printf("\t\tSTA\t__VS+%d,X\n", offset);
				compiler::instance()->printf("\t\tPSH\t#0\n");
				compiler::instance()->pushA();
				break;
			case 2:
				e->eval12();
				compiler::instance()->pullXY();
				compiler::instance()->printf("\t\tTXA\n");
				compiler::instance()->printf("\t\tLDX\t__VSS\n");
				compiler::instance()->printf("\t\tSTA\t__VS+%d,X\n", offset);
				compiler::instance()->printf("\t\tSTY\t__VS+%d,X\n", offset+1);
				compiler::instance()->printf("\t\tTAX\n");
				compiler::instance()->pushXY();
				break;
			default: 
				throw new runtime_error("Assigning non-plain-data");
		}
	} else {
		if(typ()->size() == 1) {
			e->eval6();
			source()->eval12();
			compiler::instance()->printf("\t\tPLL\ttmp+1\n");
			compiler::instance()->printf("\t\tPLL\ttmp\n");
			compiler::instance()->printf("\t\tPLL\t(tmp)\n");
			compiler::instance()->printf("\t\tPSH\t#0\n");
			compiler::instance()->printf("\t\tPSH\t(tmp)\n");

		} else if(typ()->size() == 2) {
			e->eval12();
			source()->eval12();
			compiler::instance()->printf("\t\tPLL\ttmp+1\n");
			compiler::instance()->printf("\t\tPLL\ttmp\n");
		
			compiler::instance()->printf("\t\tLDY\t#1\n");
			compiler::instance()->printf("\t\tPLL\t(tmp),Y\n");
			compiler::instance()->printf("\t\tPLL\t(tmp)\n");
			compiler::instance()->printf("\t\tPSH\t(tmp)\n");
			compiler::instance()->printf("\t\tPSH\t(tmp),Y\n");
		} else throw new runtime_error("Assigning non-plain-data");
	}
}

void dereference::eval6() const {
	dynamic_variable_addr* da = dynamic_cast<dynamic_variable_addr*>(a);
	if(da) {
		if(Type->size() == 1) {
			compiler::instance()->printf("\t\tLDY\t__VSS\n");
			compiler::instance()->printf("\t\tLDA\t__VS+%d,Y\n", da->get_offset());
			compiler::instance()->pushA();
		} else {
			compiler::instance()->printf("\t\tLDY\t__VSS\n");
			compiler::instance()->printf("\t\tLDA\t__VS+%d,Y\n", da->get_offset()+1);
			compiler::instance()->pushA();
		}
	} else {
		a->eval12();
		
		if(Type->size() == 1) {
			compiler::instance()->printf("\t\tPLL\ttmp+1\n");
			compiler::instance()->printf("\t\tPLL\ttmp\n");
			compiler::instance()->printf("\t\tPSH\t(tmp)\n");
		} else {
			compiler::instance()->printf("\t\tPLL\ttmp+1\n");
			compiler::instance()->printf("\t\tPLL\ttmp\n");
			compiler::instance()->printf("\t\tLDY\t#1\n");
			compiler::instance()->printf("\t\tLDA\t(tmp),Y\n");
			compiler::instance()->pushA();
		}
	}
}

void dereference::eval12() const {
	dynamic_variable_addr* da = dynamic_cast<dynamic_variable_addr*>(a);
	if(da) {
		if(Type->size() == 1) {
			compiler::instance()->printf("\t\tLDY\t__VSS\n");
			compiler::instance()->printf("\t\tLDX\t#0\n");
			compiler::instance()->printf("\t\tLDY\t__VS+%d,Y\n", da->get_offset());
			compiler::instance()->pushXY();
		} else {
			compiler::instance()->printf("\t\tLDY\t__VSS\n");
			compiler::instance()->printf("\t\tLDX\t__VS+%d,Y\n", da->get_offset());
			compiler::instance()->printf("\t\tLDY\t__VS+%d,Y\n", da->get_offset()+1);
			compiler::instance()->pushXY();
		}
	} else {
		if(Type->size() == 1) {
			compiler::instance()->printf("\t\tPSH\t#0\n");
			eval6();
		} else {
			a->eval12();
			compiler::instance()->printf("\t\tPLL\ttmp+1\n");
			compiler::instance()->printf("\t\tPLL\ttmp\n");
			compiler::instance()->printf("\t\tLDY\t#1\n");
			compiler::instance()->printf("\t\tLDY\t(tmp),Y\n");
			compiler::instance()->printf("\t\tLDX\t(tmp)\n");
			compiler::instance()->pushXY();
		}
	}
}

void symbol::assign6(expression* e) const {
	if(e->is_const()) { e->reduce(); } 

	if(typ()->size() == 1) {
		e->eval6();
		compiler::instance()->pullA();
		compiler::instance()->printf("\t\tSTA\t%s\n", get_name());
		compiler::instance()->pushA();
	} else if(typ()->size() == 2) {
		e->eval12();
		compiler::instance()->pullXY();
		compiler::instance()->printf("\t\tSTX\t%s\n", get_name());
		compiler::instance()->printf("\t\tSTY\t%s+1\n", get_name());
		compiler::instance()->printf("\t\tPHY\n");
	} else {
		printf("Assigning void???");
	}
}
void symbol::assign12(expression* e) const {
	if(e->is_const()) { e->reduce(); } 

	if(typ()->size() == 1) {
		e->eval6();
		compiler::instance()->pullA();
		compiler::instance()->printf("\t\tSTA\t%s\n", get_name());
		compiler::instance()->printf("\t\tPSH\t#0\n");
		compiler::instance()->pushA();
	} else if(typ()->size() == 2) {
		e->eval12();
		compiler::instance()->pullXY();
		compiler::instance()->printf("\t\tSTX\t%s\n", get_name());
		compiler::instance()->printf("\t\tSTY\t%s+1\n", get_name());
		compiler::instance()->pushXY();
	} else {
		printf("Assigning void???");
	}
}

void inverter::eval6() const {
	a->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tEOR\t#%%444\n");
	compiler::instance()->pushA();
}

void inverter::eval12() const {
	a->eval12();
	compiler::instance()->printf("\t\tPLX\n");
	compiler::instance()->printf("\t\tPLL\tA\n");
	compiler::instance()->printf("\t\tEOR\t#%%444\n");
	compiler::instance()->printf("\t\tPSH\tA\n");
	compiler::instance()->printf("\t\tTXA\n");
	compiler::instance()->printf("\t\tEOR\t#%%444\n");
	compiler::instance()->pushA();
}


void sp_test::eval6() const {
	compiler::instance()->printf("\t\tTSX\n");
	compiler::instance()->printf("\t\tPHX\n");
	a->autoeval();
	compiler::instance()->printf("\t\tTSX\n");
	compiler::instance()->printf("\t\tTXA\n");
	compiler::instance()->printf("\t\tPLL\ttmp\n");
	compiler::instance()->printf("\t\tMLL\t#%%00A\n");
	compiler::instance()->printf("\t\tCLC\n");
	compiler::instance()->printf("\t\tADD\ttmp\n");
	compiler::instance()->pushA();
}

void sp_test::eval12() const {
	compiler::instance()->printf("\t\tPSH\t#0\n");
	compiler::instance()->printf("\t\tTSX\n");
	compiler::instance()->printf("\t\tPHX\n");
	a->autoeval();
	compiler::instance()->printf("\t\tTSX\n");
	compiler::instance()->printf("\t\tTXA\n");
	compiler::instance()->printf("\t\tPLL\ttmp\n");
	compiler::instance()->printf("\t\tMLL\t#%%00A\n");
	compiler::instance()->printf("\t\tCLC\n");
	compiler::instance()->printf("\t\tADD\ttmp\n");
	compiler::instance()->pushA();
}


/* Incrementers & decrementers */





void symbol::pre_increment6() const {
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tINC\t%s\n", get_name());
		compiler::instance()->printf("\t\tPSH\t%s\n", get_name());
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tINC\t%s+1\n", get_name());
		compiler::instance()->printf("\t\tLDA\t%s\n", get_name());
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t%s\n", get_name());
		compiler::instance()->printf("\t\tPSH\t%s+1\n", get_name());
	}

}

void symbol::pre_increment12() const {
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tINC\t%s\n", get_name());
		compiler::instance()->printf("\t\tPSH\t#0\n");
		compiler::instance()->printf("\t\tPSH\t%s\n", get_name());
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tINC\t%s+1\n", get_name());
		compiler::instance()->printf("\t\tLDA\t%s\n", get_name());
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t%s\n", get_name());
		compiler::instance()->pushA();
		compiler::instance()->printf("\t\tPSH\t%s+1\n", get_name());
	}

}

void symbol::post_increment6() const {
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tPSH\t%s\n", get_name());
		compiler::instance()->printf("\t\tINC\t%s\n", get_name());
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tPSH\t%s+1\n", get_name());
		compiler::instance()->printf("\t\tINC\t%s+1\n", get_name());
		compiler::instance()->printf("\t\tLDA\t%s\n", get_name());
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t%s\n", get_name());
	}

}

void symbol::post_increment12() const {
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tPSH\t#0\n");
		compiler::instance()->printf("\t\tPSH\t%s\n", get_name());
		compiler::instance()->printf("\t\tINC\t%s\n", get_name());
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tPSH\t%s\n", get_name());
		compiler::instance()->printf("\t\tPSH\t%s+1\n", get_name());
		compiler::instance()->printf("\t\tINC\t%s+1\n", get_name());
		compiler::instance()->printf("\t\tLDA\t%s\n", get_name());
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t%s\n", get_name());
	}

}
void symbol::pre_decrement6() const {
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tDEC\t%s\n", get_name());
		compiler::instance()->printf("\t\tPSH\t%s\n", get_name());
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tDEC\t%s+1\n", get_name());
		compiler::instance()->printf("\t\tLDA\t%s\n", get_name());
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t%s\n", get_name());
		compiler::instance()->printf("\t\tPSH\t%s+1\n", get_name());
	}

}

void symbol::pre_decrement12() const {
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tDEC\t%s\n", get_name());
		compiler::instance()->printf("\t\tPSH\t#0\n");
		compiler::instance()->printf("\t\tPSH\t%s\n", get_name());
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tDEC\t%s+1\n", get_name());
		compiler::instance()->printf("\t\tLDA\t%s\n", get_name());
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t%s\n", get_name());
		compiler::instance()->pushA();
		compiler::instance()->printf("\t\tPSH\t%s+1\n", get_name());
	}

}
void symbol::post_decrement6() const {
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tPSH\t%s\n", get_name());
		compiler::instance()->printf("\t\tDEC\t%s\n", get_name());
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tPSH\t%s+1\n", get_name());
		compiler::instance()->printf("\t\tDEC\t%s+1\n", get_name());
		compiler::instance()->printf("\t\tLDA\t%s\n", get_name());
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t%s\n", get_name());
	}

}

void symbol::post_decrement12() const {
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tPSH\t#0\n");
		compiler::instance()->printf("\t\tPSH\t%s\n", get_name());
		compiler::instance()->printf("\t\tDEC\t%s\n", get_name());
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tPSH\t%s\n", get_name());
		compiler::instance()->printf("\t\tPSH\t%s+1\n", get_name());
		compiler::instance()->printf("\t\tDEC\t%s+1\n", get_name());
		compiler::instance()->printf("\t\tLDA\t%s\n", get_name());
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t%s\n", get_name());
	}

}

void dereference::pre_increment6() const {
	source()->eval12();
	compiler::instance()->pullXY();
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tINC\t(tmp)\n");
		compiler::instance()->printf("\t\tPSH\t(tmp)\n");
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tLDY\t#1\n");
		compiler::instance()->printf("\t\tINC\t(tmp),Y\n");
		compiler::instance()->printf("\t\tLDA\t(tmp)\n");
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t(tmp)\n");
		compiler::instance()->printf("\t\tPSH\t(tmp),Y\n");
	}

}

void dereference::pre_increment12() const {
	source()->eval12();
	compiler::instance()->pullXY();
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tINC\t(tmp)\n");
		compiler::instance()->printf("\t\tPSH\t#0\n");
		compiler::instance()->printf("\t\tPSH\t(tmp)\n");
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tLDY\t#1\n");
		compiler::instance()->printf("\t\tINC\t(tmp),Y\n");
		compiler::instance()->printf("\t\tLDA\t(tmp)\n");
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t(tmp)\n");
		compiler::instance()->printf("\t\tPSH\t(tmp)\n");
		compiler::instance()->printf("\t\tPSH\t(tmp),Y\n");
	}

}


void dereference::post_increment6() const {
	source()->eval12();
	compiler::instance()->pullXY();
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tPSH\t(tmp)\n");
		compiler::instance()->printf("\t\tINC\t(tmp)\n");
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tLDY\t#1\n");
		compiler::instance()->printf("\t\tPSH\t(tmp),Y\n");
		compiler::instance()->printf("\t\tINC\t(tmp),Y\n");
		compiler::instance()->printf("\t\tLDA\t(tmp)\n");
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t(tmp)\n");
	}

}

void dereference::post_increment12() const {
	source()->eval12();
	compiler::instance()->pullXY();
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tPSH\t#0\n");
		compiler::instance()->printf("\t\tPSH\t(tmp)\n");
		compiler::instance()->printf("\t\tINC\t(tmp)\n");
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tLDY\t#1\n");
		compiler::instance()->printf("\t\tPSH\t(tmp)\n");
		compiler::instance()->printf("\t\tPSH\t(tmp),Y\n");
		compiler::instance()->printf("\t\tINC\t(tmp),Y\n");
		compiler::instance()->printf("\t\tLDA\t(tmp)\n");
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t(tmp)\n");
	}
}


void dereference::pre_decrement6() const {
	source()->eval12();
	compiler::instance()->pullXY();
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tDEC\t(tmp)\n");
		compiler::instance()->printf("\t\tPSH\t(tmp)\n");
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tLDY\t#1\n");
		compiler::instance()->printf("\t\tDEC\t(tmp),Y\n");
		compiler::instance()->printf("\t\tLDA\t(tmp)\n");
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t(tmp)\n");
		compiler::instance()->printf("\t\tPSH\t(tmp),Y\n");
	}

}

void dereference::pre_decrement12() const {
	source()->eval12();
	compiler::instance()->pullXY();
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tDEC\t(tmp)\n");
		compiler::instance()->printf("\t\tPSH\t#0\n");
		compiler::instance()->printf("\t\tPSH\t(tmp)\n");
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tLDY\t#1\n");
		compiler::instance()->printf("\t\tDEC\t(tmp),Y\n");
		compiler::instance()->printf("\t\tLDA\t(tmp)\n");
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t(tmp)\n");
		compiler::instance()->printf("\t\tPSH\t(tmp)\n");
		compiler::instance()->printf("\t\tPSH\t(tmp),Y\n");
	}

}

void dereference::post_decrement6() const {
	source()->eval12();
	compiler::instance()->pullXY();
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tPSH\t(tmp)\n");
		compiler::instance()->printf("\t\tDEC\t(tmp)\n");
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tLDY\t#1\n");
		compiler::instance()->printf("\t\tPSH\t(tmp),Y\n");
		compiler::instance()->printf("\t\tDEC\t(tmp),Y\n");
		compiler::instance()->printf("\t\tLDA\t(tmp)\n");
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t(tmp)\n");
	}

}

void dereference::post_decrement12() const {
	source()->eval12();
	compiler::instance()->pullXY();
	if(typ()->size() == 1) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tPSH\t#0\n");
		compiler::instance()->printf("\t\tPSH\t(tmp)\n");
		compiler::instance()->printf("\t\tDEC\t(tmp)\n");
	} else if(typ()->size() == 2) {
		compiler::instance()->printf("\t\tSTX\ttmp\n");
		compiler::instance()->printf("\t\tSTY\ttmp+1\n");
		compiler::instance()->printf("\t\tLDY\t#1\n");
		compiler::instance()->printf("\t\tPSH\t(tmp)\n");
		compiler::instance()->printf("\t\tPSH\t(tmp),Y\n");
		compiler::instance()->printf("\t\tDEC\t(tmp),Y\n");
		compiler::instance()->printf("\t\tLDA\t(tmp)\n");
		compiler::instance()->printf("\t\tADD\t#0\n");
		compiler::instance()->printf("\t\tSTA\t(tmp)\n");
	}
}

void sign::eval6() const {
	if(a->typ()->size() == 1) {
		a->eval6();
		compiler::instance()->pullA();
		compiler::instance()->printf("\t\tCMP\t#0\n");
	} else {
		a->eval12();
		compiler::instance()->pullXY();
		compiler::instance()->printf("\t\tCAD\t0\n");
	}
	compiler::instance()->printf("\t\tPHP\n");
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tEOR\t#%%A00\n"); 
	compiler::instance()->printf("\t\tDIV\t#%%100\n"); 
	compiler::instance()->pushA();
}

void sign::eval12() const {
	compiler::instance()->printf("\t\tPSH\t#0\n");
	eval6();
}

