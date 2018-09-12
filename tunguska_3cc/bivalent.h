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

#ifndef bivalent_h
#define bivalent_h

#include "expression.h"
#include <assert.h>

/* Bivalent expressions, see expression.h for more information */

class bivalent : public expression {
	public:
		bivalent(expression* a, expression* b) { 
			try {
				this->a = dearray(a); 
			} catch (std::invalid_argument* e) {
				delete e;
				this->a = a;
			}
			try {
				this->b = dearray(b); 
			} catch (std::invalid_argument* e) {
				delete e;
				this->b = b;
			}

			try {
				reduce();
			} catch (std::invalid_argument* e) {
				delete e;
			}
		}

		virtual ~bivalent() {
			expression::safe_delete(a);
			expression::safe_delete(b);
		}

		virtual void debug_print_tree() {
			printf("%s (", typeid(*this).name());
			a->debug_print_tree();
			printf(",");
			b->debug_print_tree();
			printf(")");
		}
		virtual const type* typ() const;
		virtual void reduce() { if(a->is_const()) { a = new constant(a->const_val()); }
					else a->reduce(); 
					if(b->is_const()) { b = new constant(b->const_val()); }
					else b->reduce(); }
	protected:
		expression* a;
		expression* b;
};

class logical_bivalent : public bivalent {
	public:
		logical_bivalent(expression* a, expression* b) : bivalent(a,b) {
		}
};

class ptr_assignment : public bivalent {
	public:
		ptr_assignment(expression* a, expression* b) : bivalent(a, b) {};
		virtual void eval6() const;
		virtual void eval12() const;
};

/* *****************************
 *
 * Comparisons
 *
 * **************************** */

class greater_than : public bivalent {
	public:
		greater_than(expression* a, expression* b) 
			: bivalent(a, b){ }
		virtual void eval6() const;
		virtual void eval12() const;
		virtual const type* typ() const { return new t_i6(); }
		virtual bool is_const() { return a->is_const() && b->is_const(); }
		virtual int const_val() { return a->const_val() > b->const_val(); }
		virtual bool is_logical() const { return true; }

};

class greater_or_equal : public bivalent {
	public:
		greater_or_equal(expression* a, expression* b) 
			: bivalent(a, b) { }
		virtual void eval6() const;
		virtual void eval12() const;
		virtual const type* typ() const { return new t_i6(); }
		virtual bool is_const() { return a->is_const() && b->is_const(); }
		virtual int const_val() { return a->const_val() >= b->const_val(); }
		virtual bool is_logical() const { return true; }
};

class equals : public bivalent {
	public:
		equals(expression* a, expression* b) : bivalent(a, b) {}
		virtual void eval6() const;
		virtual void eval12() const;
		virtual const type* typ() const { return new t_i6(); }
		virtual bool is_const() { return a->is_const() && b->is_const(); }
		virtual int const_val() { return a->const_val() == b->const_val(); }
		virtual bool is_logical() const { return true; }

};

class differs : public bivalent {
	public:
		differs(expression* a, expression* b) : bivalent(a, b) { };
		virtual void eval6() const;
		virtual void eval12() const;
		virtual const type* typ() const { return new t_i6(); }
		virtual bool is_const() { return a->is_const() && b->is_const(); }
		virtual int const_val() { return a->const_val() != b->const_val(); }
		virtual bool is_logical() const { return true; }
};

class less_than : public bivalent {
	public:
		less_than(expression* a, expression* b) : bivalent(a,b) {};
		virtual void eval6() const;
		virtual void eval12() const;
		virtual const type* typ() const { return new t_i6(); }
		virtual bool is_const() { return a->is_const() && b->is_const(); }
		virtual int const_val() { return a->const_val() < b->const_val(); }
		virtual bool is_logical() const { return true; }

};

class less_or_equal : public bivalent {
	public:
		less_or_equal(expression* a, expression* b) : bivalent(a,b) {}
		virtual void eval6() const;
		virtual void eval12() const;
		virtual const type* typ() const { return new t_i6(); }
		virtual bool is_const() { return a->is_const() && b->is_const(); }
		virtual int const_val() { return a->const_val() <= b->const_val(); }
		virtual bool is_logical() const { return true; }
};

/* *****************************
 *
 * Operations
 *
 * **************************** */

class adder : public bivalent {
	public:
		adder(expression* a, expression * b) : bivalent(a,b) {}
		virtual void eval6() const;
		virtual void eval12() const;
		/* Addition defaults to be of integer type, even if
 		 * both a and b are chars. */
		virtual const type* typ() const { 
			const type* t = bivalent::typ();
			if(t->size() == 1) return new t_i12(); 
			else return t;
		}
		virtual bool is_const() { return a->is_const() && b->is_const(); }
		virtual int const_val() { return a->const_val() + b->const_val(); }
};

class mul : public bivalent {
	public:
		mul(expression* a, expression* b) : bivalent(a,b) { }
		virtual void eval6() const;
		virtual void eval12() const;
		/* Multiplication defaults to be of integer type, even if
 		 * both a and b are chars. */
		virtual const type* typ() const { 
			const type* t = bivalent::typ();
			if(t->size() == 1) return new t_i12(); 
			else return t;
		}
		virtual bool is_const() { return a->is_const() && b->is_const(); }
		virtual int const_val() { return a->const_val() * b->const_val(); }
		
};

class divider : public bivalent {
	public:
		divider(expression* a, expression* b) : bivalent(a,b) { }
		virtual void eval6() const;
		virtual void eval12() const;

		virtual bool is_const() { return a->is_const() && b->is_const(); }
					// TODO: Make sure b != NULL
		virtual int const_val() { return a->const_val() / b->const_val(); }
		
};

class modulo : public bivalent {
	public:
		modulo(expression* a, expression* b) : bivalent(a,b) { }
		virtual void eval6() const;
		virtual void eval12() const;

		virtual bool is_const() { return a->is_const() && b->is_const(); }
					// TODO: Make sure b != NULL
		virtual int const_val() { return a->const_val() / b->const_val(); }
		
};

/* & */
class ander : public bivalent {
	public:
		ander(expression* a, expression* b) : bivalent(a,b) { }
		virtual void eval6() const;
		virtual void eval12() const;
};

/* && */

/* TODO: 
 * Logical And should only evaluate b if a is true.
 */

class logical_ander : public ander {
	public:
		logical_ander(expression* a, expression* b) : ander(a,b) {
			if(!a->is_logical()) a = new sign(a);
			if(!b->is_logical()) b = new sign(b);
		}
		virtual void eval6() const;// { ander::eval6(); }
		virtual void eval12() const;// { ander::eval12(); }
		virtual bool is_logical() const { return true; }
};

/* | */
class orer : public bivalent {
	public:
		orer(expression* a, expression* b) : bivalent(a,b) {}
		virtual void eval6() const;
		virtual void eval12() const;
};

/* || */

/* TODO: 
 * Logical And should only evaluate b if a is false.
 */
class logical_orer : public orer {
	public:
		logical_orer(expression* a, expression* b) : orer(a,b) { 
			if(!a->is_logical()) a = new sign(a);
			if(!b->is_logical()) b = new sign(b);
		
		}
		virtual void eval6() const;// { orer::eval6(); }
		virtual void eval12() const;// { orer::eval12(); }
		virtual bool is_logical() const { return true; }
};

class xorer : public bivalent {
	public:
		xorer(expression* a, expression* b) : bivalent(a,b) {}
		virtual void eval6() const;
		virtual void eval12() const;
};
class prm: public bivalent {
	public:
		prm(expression* a, expression* b) : bivalent(a,b) {}
		virtual void eval6() const;
		virtual void eval12() const;
};
class tsh: public bivalent {
	public:
		tsh(expression* a, expression* b) : bivalent(a,b) {}
		virtual void eval6() const;
		virtual void eval12() const;
};

class comm_op: public bivalent {
	public:
		comm_op(int op, expression* a, expression* b) : bivalent(a,b) { this->op = op; }
		virtual void eval6() const;
		virtual void eval12() const;
	private:
		int op;
};

#endif
