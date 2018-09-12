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
#include "compiler.h"

#ifndef conditional_h
#define conditional_h

#include <stack>

class conditional {

	public:
		conditional() { scopec = 0; }
		virtual ~conditional() {}
		virtual void init() = 0;
		virtual void intermediate() = 0;
		virtual void dispose() = 0;
		virtual void cont() = 0;
		virtual void brk() = 0;
		virtual bool does_break() const = 0;
		virtual bool does_continue() const = 0;
		virtual int scope_count() const { return scopec; }
		virtual void add_scope() { scopec++; }
		virtual void end_scope() { scopec--; }
	private:
		int scopec;
};

class cond_mgr { 
	public:
		static void init_cond(conditional* c);
		static void cont_cond();
		static void brk_cond();
		static void intermediate_cond();
		static void dispose_cond();
		static int  get_uid() { return uid++; }
		static void reset_uid() { uid = 0; }
		static void add_scope();
		static void end_scope();
	private:
		static void init_condheap();
		static std::stack<conditional*>* conditional_heap;
		static int uid;
		
};

class if_cnd : public conditional {
	public:
		if_cnd(expression* condition) {
			this->condition = condition;
			this->id = cond_mgr::get_uid();
		}
	protected:
		virtual void init();
		virtual void intermediate();
		virtual void dispose();
		virtual void cont();
		virtual void brk();
		virtual bool does_break() const { return false; }
		virtual bool does_continue() const { return false; }
	private:
		expression* condition;
		int id;
};

class while_cnd: public conditional {
	public:
		while_cnd(expression* condition) {
			this->condition = condition;
			this->id = cond_mgr::get_uid();
		}
	protected: 
		virtual void init();
		virtual void intermediate();
		virtual void dispose();
		virtual void cont();
		virtual void brk();
		virtual bool does_break() const { return true; }
		virtual bool does_continue() const { return true; }
	private:
		expression* condition;
		int id;
};

class for_cnd: public conditional {
	public:
		for_cnd(expression* initial,
			  expression* condition,
			  expression* incrementer) {
			this->initial= initial;
			this->condition= condition;
			this->incrementer= incrementer;
			this->id = cond_mgr::get_uid();
		}
	protected: 
		virtual void init();
		virtual void intermediate();
		virtual void dispose();
		virtual void cont();
		virtual void brk();
		virtual bool does_break() const { return true; }
		virtual bool does_continue() const { return true; }
	private:
		expression* initial, *condition, *incrementer;
		int id;
};


#endif
