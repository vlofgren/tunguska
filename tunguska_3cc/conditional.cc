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

#include "conditional.h"
#include <stack>


int cond_mgr::uid;
std::stack<conditional*>* cond_mgr::conditional_heap = NULL;

void cond_mgr::init_condheap() {
	if(conditional_heap == NULL) 
		conditional_heap = new std::stack<conditional*>();
}

void cond_mgr::init_cond(conditional* c) {
	init_condheap();
	conditional_heap->push(c);
	c->init();
}

void cond_mgr::intermediate_cond() {
	init_condheap();
	conditional_heap->top()->intermediate();
}

void cond_mgr::cont_cond() { 
	init_condheap();
	std::stack<conditional*> pile;
	while(!conditional_heap->empty()) {
		if(conditional_heap->top()->does_continue()) {
			conditional_heap->top()->cont();
			break;
		}
		pile.push(conditional_heap->top());
		conditional_heap->pop();
	}
	if(conditional_heap->empty()) {
		throw new runtime_error("Break outside of loop.");
	}
	while(!pile.empty()) {
		conditional_heap->push(pile.top());
		pile.pop();
	}
}

void cond_mgr::brk_cond() { 
	init_condheap();

	std::stack<conditional*> pile;

	int levels = 0;
	while(!conditional_heap->empty()) {
		levels+=conditional_heap->top()->scope_count();
		if(conditional_heap->top()->does_break()) {
			function::get_current()->bail_scopes(levels);
			conditional_heap->top()->brk();
			break;
		}
		pile.push(conditional_heap->top());
		conditional_heap->pop();
	}
	if(conditional_heap->empty()) {
		throw new runtime_error("Continue outside of loop.");
	}
	while(!pile.empty()) {
		conditional_heap->push(pile.top());
		pile.pop();
	}
}

void cond_mgr::add_scope() {
	init_condheap();
	if(conditional_heap->empty()) return;

	conditional_heap->top()->add_scope();
}
void cond_mgr::end_scope() {
	init_condheap();
	if(conditional_heap->empty()) return;
	conditional_heap->top()->end_scope();
}

void cond_mgr::dispose_cond() {
	init_condheap();
	conditional_heap->top()->dispose();
	conditional_heap->pop();
}

/* ************************************************************
 *
 *                             I F
 *
 * ************************************************************ */

void if_cnd::init() {
	if(condition->typ()->size() == 1) {
		condition->eval6();
		compiler::instance()->pullA();
		compiler::instance()->printf("\t\tCMP\t#0\n");
	} else {
		condition->eval12();
		compiler::instance()->pullXY();
		compiler::instance()->printf("\t\tCAD\t0\n");
	}
	compiler::instance()->printf("\t\tJLT\t.if%delse\n", id);
	compiler::instance()->printf("\t\tJEQ\t.if%delse\n", id);
}
void if_cnd::intermediate() { 
	compiler::instance()->printf("\t\tJMP\t.if%ddone\n", id);
	compiler::instance()->printf(".if%delse:\n", id);
}
void if_cnd::cont() { 
	/* Not defined */
}
void if_cnd::brk() { 
	/* Not defined */
}
void if_cnd::dispose() {
	compiler::instance()->printf(".if%ddone:\n", id);
}

/* ************************************************************
 *
 *                           W H I L E
 *
 * ************************************************************ */

void while_cnd::init() {
	compiler::instance()->printf(".while%dbegin:\n", id);
	if(condition->typ()->size() == 1) {
		condition->eval6();
		compiler::instance()->pullA();
		compiler::instance()->printf("\t\tCMP\t#0\n");
	} else {
		condition->eval12();
		compiler::instance()->pullXY();
		compiler::instance()->printf("\t\tCAD\t0\n");
	}
	compiler::instance()->printf("\t\tJLT\t.while%ddone\n", id);
	compiler::instance()->printf("\t\tJEQ\t.while%ddone\n", id);
}
void while_cnd::cont() { 
	compiler::instance()->printf("\t\tJMP\t.while%dbegin\n", id);
}
void while_cnd::brk() { 
	compiler::instance()->printf("\t\tJMP\t.while%ddone\n", id);
}

void while_cnd::intermediate() { }

void while_cnd::dispose() {
	compiler::instance()->printf("\t\tJMP\t.while%dbegin\n", id);
	compiler::instance()->printf(".while%ddone:\n", id);
}

/* ************************************************************
 *
 *                             F O R
 *
 * ************************************************************ */

void for_cnd::init() {
	if(initial) {
		if(initial->typ()->size() == 1) {
			initial->eval6();
			compiler::instance()->pullA();
		} else {
			initial->eval12();
			compiler::instance()->pullXY();
		}
	}
	compiler::instance()->printf(".for%dbegin:\n", id);
	if(condition) {
		if(condition->typ()->size() == 1) {
			condition->eval6();
			compiler::instance()->pullA();
			compiler::instance()->printf("\t\tCMP\t#0\n");
		} else {
			condition->eval12();
			compiler::instance()->pullXY();
			compiler::instance()->printf("\t\tCAD\t0\n");
		}
		compiler::instance()->printf("\t\tJLT\t.for%ddone\n", id);
		compiler::instance()->printf("\t\tJEQ\t.for%ddone\n", id);
	}
}

void for_cnd::intermediate() { }

void for_cnd::cont() { 
	compiler::instance()->printf("\t\tJMP\t.for%dcontinue\n", id);
}
void for_cnd::brk() { 
	compiler::instance()->printf("\t\tJMP\t.for%ddone\n", id);
}


void for_cnd::dispose() {
	compiler::instance()->printf(".for%dcontinue:\n", id);
	if(incrementer) {
		if(incrementer->typ()->size() == 1) {
			incrementer->eval6();
			compiler::instance()->pullA();
		} else {
			incrementer->eval12();
			compiler::instance()->pullXY();
		}
	}

	compiler::instance()->printf("\t\tJMP\t.for%dbegin\n", id);
	compiler::instance()->printf(".for%ddone:\n", id);
}


