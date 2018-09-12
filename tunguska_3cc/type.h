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

#include <stdio.h>
#include <string.h>
#include <list>
#include <stdexcept>
#include "sllist.h"
#ifndef type_h
#define type_h


/* Type management. 
 *
 * Note that these classes are pretty much disposable. _Don't_bother_
 * _with_deleting_them_(that will infact create more problems than simply
 * tossing them away when you're done.) Modern computers have so much memory
 * there's no point in being a memory nazi in this sort of a short-run
 * application.
 */

class type {
	public:
		class not_a_reference {};
		type() {}
		virtual ~type() {};
		virtual const type* dup() const = 0;
		virtual int size() const = 0;
		virtual void id() const = 0;

		virtual bool operator==(const type& t) const = 0;
		virtual bool operator!=(const type& t) const {
			return !(*this == t);
		}

		virtual const type* dereference() 
			const throw(const not_a_reference*) {
			throw (const not_a_reference*) new not_a_reference();
		}
		virtual bool can_deref() const { 
			try { dereference(); return true; } 
			catch(const not_a_reference* r) { return false; }
		}


};

class t_void : public type {
	public:
		t_void() {}
		virtual int size() const { return 0; }
		virtual void id() const { printf("v"); }
		virtual const type* dup() const { return new t_void(); };

		virtual bool operator==(const type& t) const {
			return dynamic_cast<const t_void*>(&t) != NULL;
		}
};

class t_i6 : public type {
	public:
		t_i6() {}
		virtual const type* dup() const { return new t_i6(); };
		virtual int size() const { return 1; }
		virtual void id() const { printf("i6"); }
		virtual bool operator==(const type& t) const {
			return dynamic_cast<const t_i6*>(&t) != NULL;
		}
};

class t_i12 : public type {
	public:
		t_i12() {}
		virtual const type* dup() const { return new t_i12(); };
		virtual int size() const { return 2; }
		virtual void id() const { printf("i12"); }
		virtual bool operator==(const type& t) const {
			return dynamic_cast<const t_i12*>(&t) != NULL;
		}
};

class t_ptr : public type {
	public:
		t_ptr(const type* t) {
			this->ref = t;
		}
		t_ptr(const t_ptr& t) {
			this->ref = t.ref->dup();
		}
		t_ptr(const t_ptr* t) {
			this->ref = t->ref->dup();
		}

		virtual const type* dup() const { 
			return new t_ptr(ref->dup()); 
		};
		virtual bool operator==(const type& t) const {
			try {
				return *dereference() == *t.dereference();
			} catch (const not_a_reference* e) {
				return false;
			}
		}
		virtual int size() const { return 2; }
		virtual void id() const { printf("&"); ref->id(); }
		virtual const type* dereference() 
			const throw(const not_a_reference*) {
			return ref;
		}
	protected:
		const type* ref;
};

class t_struct : public type {
	public:	
		class field {
		public:
			field(const char* name, const type* typ, int offset) {
				this->name = name;
				this->typ = typ;
				this->offset = offset;
			}
			const char* get_name() const { return name; }
			const type* get_type() const { return typ; }
			int get_offset() const { return offset; }
		private:
			const char* name;
			const type* typ;
			int offset;
		}; 

		t_struct(sllist* fields) {
			sllist* reverse = sllist::reverse(fields);

			int offset = 0;
			while(reverse) {
				this->fields.push_back(new field((const char*) reverse->data, (const type*) reverse->data2, offset));
				offset += ((const type*)reverse->data2)->size();
				reverse = reverse->next;
			}
		}

		virtual int size() const { 
			int sz = 0;

			std::list<const field*>::const_iterator i;
			for(i = fields.begin(); i != fields.end(); i++) {
				sz+=(*i)->get_type()->size();
			}

			return sz;
		}

		virtual const type* dup() const { 
			return new t_struct(*this); 
		}

		virtual bool operator==(const type& t) const {
			const t_struct* ts = dynamic_cast<const t_struct*>(&t);
			if(!ts) return false;

			if(fields.size() != ts->fields.size()) return false;
			std::list<const field*>::const_iterator i;
			std::list<const field*>::const_iterator j;
			for(i = fields.begin(),j=ts->fields.begin(); i != fields.end(); i++,j++) {
				if(strcmp((*i)->get_name(), (*j)->get_name()) != 0) return false;
				if(*(*i)->get_type() != *(*j)->get_type()) return false;
				
			}

			return true;
		}

		virtual void id() const {
			printf("struct { ");
			std::list<const field*>::const_iterator i;
			for(i = fields.begin(); i != fields.end(); i++) {
				(*i)->get_type()->id();
				printf(" %s,", (*i)->get_name());
			}
			printf("\b }");
		}

		virtual const field* get_field(const char* name) const {
			std::list<const field*>::const_iterator i;
			for(i = fields.begin(); i != fields.end(); i++) {
				if(strcmp((*i)->get_name(), name) == 0) return (*i);
			}
			throw new std::runtime_error("Unknown field");
		}
	
		const std::list<const field*>& get_field_list() const { return fields; }
	private:
		t_struct(const t_struct& t) {
			std::list<const field*>::const_iterator i;
			for(i = t.fields.begin(); i != t.fields.end(); i++) {
				fields.push_back(*i);
			}
			
		}
		std::list<const field*> fields;
};

class t_array : public type {
public:
	t_array(type* base, int elements) {
		this->base = base;
		this->elements = elements;
	}

	virtual const type* dup() const {
		return new t_array(base, elements);
	}
	virtual int size() const {
		return base->size()*elements;
	}
	virtual int dimension() const {
		return elements;
	}
	virtual void id() const {
		printf("[%d]", elements);
		base->id();
	}

	virtual bool operator==(const type& t) const {
		const t_array* tarray = dynamic_cast<const t_array*>(&t);
		if(!tarray) return false;
		if(tarray->elements != elements) return false;
		if(tarray->base != base) return false;
		return false;
	}

	virtual const type* dereference() 
		const throw(const not_a_reference*) {
		return base;
	}

	private:
		int elements;
		type* base;
};

#endif
