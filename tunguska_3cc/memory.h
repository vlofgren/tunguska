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

#ifndef memory_h
#define memory_h

#include <list>

/* This code mostly regulates string constants */

class memory_node;

class memory_mgr {
public:
	expression* add(memory_node* n);
	int size() const;
	void define() const;	
	const char* name() const { return "__DATA"; }
private:
	std::list<memory_node*> nodes;
};

class memory_node {
public:
	virtual ~memory_node() {}
	virtual int size() const = 0;
	virtual void define() const = 0;
};

class string_node : public memory_node {
public:
	string_node(const char* str) { this->str = str; }
	virtual int size() const; 
	virtual void define() const;
private:
	const char* str;
};

/* The following code is deprecated, and is a sort proto array
 * code that is replaced by initializers.
 */
class array6_node : public memory_node {
public:
	array6_node(int length) { this->length = length; }
	virtual int size() const { return length; }
	virtual void define() const;
private:
	int length;
};

class array12_node : public memory_node {
public:
	array12_node(int length) { this->length = length; }
	virtual int size() const { return length*2; }
	virtual void define() const;
private:
	int length;
};


#endif
