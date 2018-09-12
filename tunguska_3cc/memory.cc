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
 
#include "memory.h"
#include "compiler.h"
#include <list>

void string_node::define() const {
	int quote = 0;
	const char* s = str;
	compiler::instance()->printf("\t@DT\t");
	while(*s) {
		if(*s == '\\') {
			if(*(s+1) == '0') {
				if(quote) { compiler::instance()->printf("',"); quote = 0; }
				else if(s != str) compiler::instance()->printf(",");
				compiler::instance()->printf("0 ");
				s+=2; continue;
			} else if(*(s+1) == 'n') {
				if(quote) { compiler::instance()->printf("',"); quote = 0; }
				else if(s != str) compiler::instance()->printf(",");
				compiler::instance()->printf("2 ");
				s+=2; continue;
			}
		}
		if(!quote) { 
			if(s != str) compiler::instance()->printf(",");
			compiler::instance()->printf(" '"); 
			quote = 1;
		}
		compiler::instance()->printf("%c", *s);
		s++;
	}
	if(quote) compiler::instance()->printf("'");
	if(s != str) compiler::instance()->printf(",");
	compiler::instance()->printf("0\n");
	
}

int string_node::size() const { 
	const char* s = str;
	int len = 0;
	while(*s) {
		if(*s == '\\') {
			if(!*(s+1)) { len++; break; }
			else if(*(s+1) == 'n') { len++; s+=2; continue; }
			else if(*(s+1) == '0') { len++; s+=2; continue; }
		}
		len++;
		s++;
	}
	return len+1;
}

void array6_node::define() const {
	compiler::instance()->printf("\t@REST\t%d\n", length);
}
void array12_node::define() const {
	compiler::instance()->printf("\t@REST\t%d\n", 2*length);
}

expression* memory_mgr::add(memory_node* n) {
	expression* e = new string_constant(size());
	nodes.push_back(n);
	return e;
}	

int memory_mgr::size() const {
	int sz = 0;
	std::list<memory_node*>::const_iterator i;
	for(i = nodes.begin(); i != nodes.end(); i++) {
		sz += (*i)->size();
	}
	return sz;
}

void memory_mgr::define() const {
	compiler::instance()->printf("%s:\n", name());
	std::list<memory_node*>::const_iterator i;
	for(i = nodes.begin(); i != nodes.end(); i++) {
		(*i)->define();
	}
	
}
