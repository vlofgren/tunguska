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
 
#include "bivalent.h"
#include "expression.h"
#include "conditional.h"
#include "compiler.h"
#include <math.h>

const type* bivalent::typ() const {
	const type* atyp = a->typ();
	const type* btyp = b->typ();
	int asize = atyp->size();
	int bsize = btyp->size();
	bool aderef = atyp->can_deref();
	bool bderef = btyp->can_deref();

	if(aderef && bderef) {
		if(*atyp == *btyp) return atyp;
		atyp->id(); putchar(' ');
		btyp->id(); putchar(':');
		fflush(NULL);
		throw new runtime_error("Operation on different pointer-types"
				" (ambiguity of type).");
	}

	if(aderef) return atyp;
	if(bderef) return btyp;

	if(asize > bsize) return atyp;
	return btyp;
}

void adder::eval6() const {
	a->eval6();
	b->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tCLC\n");	
	compiler::instance()->printf("\t\tADD\ttmp\n");	
	compiler::instance()->pushA();
}

void adder::eval12() const {

	b->eval12();
	a->eval12();
	compiler::instance()->pullXY();

	compiler::instance()->printf("\t\tADW\tX,Y\n");
	compiler::instance()->pushXY();
}

void mul::eval6() const {
	a->eval6();
	b->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tMLL\ttmp\n");	
	compiler::instance()->printf("\t\tPSH\tA\n");	
}

void mul::eval12() const {
	if(a->typ()->size() == 1 && b->typ()->size() == 1) {
		a->eval6();
		b->eval6();
		compiler::instance()->pullXY();
		compiler::instance()->printf("\t\tTXA\n");	
		compiler::instance()->printf("\t\tSTY\ttmp\n");	
		compiler::instance()->printf("\t\tMLH\ttmp\n");	
		compiler::instance()->printf("\t\tPSH\tA\n");	
		compiler::instance()->printf("\t\tTXA\n");	
		compiler::instance()->printf("\t\tMLL\ttmp\n");	
		compiler::instance()->printf("\t\tPSH\tA\n");	
	} else {
		/* First check if one of the arguments is a constant power of 3
		 *  -- if so, use left-shifting to perform the multiplication */
		if(a->is_const()) {
			if(a->const_val() == 0) { // Multiplication by 0
				compiler::instance()->printf("\t\tPSH #0\n");
				compiler::instance()->printf("\t\tPSH #0\n");
				return;
			}
		} else if(b->is_const()) {	
		
			if(b->const_val() == 0) { // Multiplication by 0
				compiler::instance()->printf("\t\tPSH #0\n");
				compiler::instance()->printf("\t\tPSH #0\n");
				return;
			}
		}

		/* No satisfactory optimization? All that is left is to do it
		 * the hard way */
		
		a->eval12();
		b->eval12();

		compiler::instance()->pullXY();

		compiler::instance()->printf("\t\tMLW\tX,Y\n");
		compiler::instance()->pushXY();
	}
}

void divider::eval6() const {
	b->eval6();
	a->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tDIV\ttmp\n");	
	compiler::instance()->pushA();
}

void divider::eval12() const {
	b->eval12();
	a->eval12();
	compiler::instance()->pullXY();
	compiler::instance()->printf("\t\tDVW\tX,Y\n");
	compiler::instance()->pushXY();
}

void modulo::eval6() const {
	b->eval6();
	a->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tMOD\ttmp\n");	
	compiler::instance()->pushA();
}

void modulo::eval12() const {
	b->eval12();
	a->eval12();
	compiler::instance()->pullXY();
	compiler::instance()->printf("\t\tMDW\tX,Y\n");
	compiler::instance()->pushXY();
}




void ander::eval6() const {
	a->eval6();
	b->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tAND\ttmp\n");	
	compiler::instance()->pushA();
}

void ander::eval12() const {
	a->eval12();
	b->eval12();
	compiler::instance()->pullXY();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tPLL\ttmp+1\n");	
	compiler::instance()->printf("\t\tTXA\n");	
	compiler::instance()->printf("\t\tAND\ttmp\n");	
	compiler::instance()->printf("\t\tTAX\n");	
	compiler::instance()->printf("\t\tTYA\n");	
	compiler::instance()->printf("\t\tAND\ttmp\n");	
	compiler::instance()->printf("\t\tTAY\n");	
	compiler::instance()->pushXY();
}

void logical_ander::eval6() const {
	int uid = cond_mgr::get_uid();
	a->eval6();
	compiler::instance()->pullA();
	compiler::instance()->pushA();
	compiler::instance()->printf("\t\tCMP\t#0\n");
	compiler::instance()->printf("\t\tJLT\t.and%d_false\n", uid);
	b->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");
	compiler::instance()->printf("\t\tAND\ttmp\n");
	compiler::instance()->pushA();
	compiler::instance()->printf("\t\tJMP\t.and%d_done\n", uid);
	compiler::instance()->printf(".and%d_false:\n", uid);
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPSH\t#%%00A\n");
	compiler::instance()->printf(".and%d_done:\n", uid);
}

void logical_ander::eval12() const {
	int uid = cond_mgr::get_uid();
	a->eval6();
	compiler::instance()->pullA();
	compiler::instance()->pushA();
	compiler::instance()->printf("\t\tCMP\t#0\n");
	compiler::instance()->printf("\t\tJLT\t.and%d_false\n", uid);
	b->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");
	compiler::instance()->printf("\t\tAND\ttmp\n");
	compiler::instance()->printf("\t\tPSH\t#%%000\n");
	compiler::instance()->pushA();
	compiler::instance()->printf("\t\tJMP\t.and%d_done\n", uid);
	compiler::instance()->printf(".and%d_false:\n", uid);
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPSH\t#%%000\n");
	compiler::instance()->printf("\t\tPSH\t#%%00A\n");
	compiler::instance()->printf(".and%d_done:\n", uid);
}

void orer::eval6() const {
	a->eval6();
	b->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tORA\ttmp\n");	
	compiler::instance()->pushA();
}

void orer::eval12() const {
	a->eval12();
	b->eval12();
	compiler::instance()->pullXY();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tPLL\ttmp+1\n");	
	compiler::instance()->printf("\t\tTXA\n");	
	compiler::instance()->printf("\t\tORA\ttmp\n");	
	compiler::instance()->printf("\t\tTAX\n");	
	compiler::instance()->printf("\t\tTYA\n");	
	compiler::instance()->printf("\t\tORA\ttmp\n");	
	compiler::instance()->printf("\t\tTAY\n");	
	compiler::instance()->pushXY();
}

void logical_orer::eval6() const {
	int uid = cond_mgr::get_uid();
	a->eval6();
	compiler::instance()->pullA();
	compiler::instance()->pushA();
	compiler::instance()->printf("\t\tCMP\t#0\n");
	compiler::instance()->printf("\t\tJGT\t.or%d_true\n", uid);
	b->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");
	compiler::instance()->printf("\t\tORA\ttmp\n");
	compiler::instance()->pushA();
	compiler::instance()->printf("\t\tJMP\t.or%d_done\n", uid);
	compiler::instance()->printf(".or%d_true:\n", uid);
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPSH\t#%%001\n");
	compiler::instance()->printf(".or%d_done:\n", uid);
}

void logical_orer::eval12() const {
	int uid = cond_mgr::get_uid();
	a->eval6();
	compiler::instance()->pullA();
	compiler::instance()->pushA();
	compiler::instance()->printf("\t\tCMP\t#0\n");
	compiler::instance()->printf("\t\tJGT\t.or%d_true\n", uid);
	b->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");
	compiler::instance()->printf("\t\tORA\ttmp\n");
	compiler::instance()->printf("\t\tPSH\t#%%000\n");
	compiler::instance()->pushA();
	compiler::instance()->printf("\t\tJMP\t.or%d_done\n", uid);
	compiler::instance()->printf(".or%d_true:\n", uid);
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPSH\t#%%000\n");
	compiler::instance()->printf("\t\tPSH\t#%%001\n");
	compiler::instance()->printf(".or%d_done:\n", uid);
}
void xorer::eval6() const {
	a->eval6();
	b->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tEOR\ttmp\n");	
	compiler::instance()->pushA();
}

void xorer::eval12() const {
	a->eval12();
	b->eval12();
	compiler::instance()->pullXY();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tPLL\ttmp+1\n");	
	compiler::instance()->printf("\t\tTXA\n");	
	compiler::instance()->printf("\t\tEOR\ttmp\n");	
	compiler::instance()->printf("\t\tTAX\n");	
	compiler::instance()->printf("\t\tTYA\n");	
	compiler::instance()->printf("\t\tEOR\ttmp\n");	
	compiler::instance()->printf("\t\tTAY\n");	
	compiler::instance()->pushXY();
}

void tsh::eval6() const {
	a->eval6();
	b->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tTSH\ttmp\n");	
	compiler::instance()->pushA();
}

void tsh::eval12() const {
	a->eval12();
	b->eval12();
	compiler::instance()->pullXY();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tPLL\ttmp+1\n");	
	compiler::instance()->printf("\t\tTXA\n");	
	compiler::instance()->printf("\t\tTSH\ttmp\n");	
	compiler::instance()->printf("\t\tTAX\n");	
	compiler::instance()->printf("\t\tTYA\n");	
	compiler::instance()->printf("\t\tTSH\ttmp\n");	
	compiler::instance()->printf("\t\tTAY\n");	
	compiler::instance()->pushXY();
}

void prm::eval6() const {
	a->eval6();
	b->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tPRM\ttmp\n");	
	compiler::instance()->pushA();
}

void prm::eval12() const {
	a->eval12();
	b->eval12();
	compiler::instance()->pullXY();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tPLL\ttmp+1\n");	
	compiler::instance()->printf("\t\tTXA\n");	
	compiler::instance()->printf("\t\tPRM\ttmp\n");	
	compiler::instance()->printf("\t\tTAX\n");	
	compiler::instance()->printf("\t\tTYA\n");	
	compiler::instance()->printf("\t\tPRM\ttmp\n");	
	compiler::instance()->printf("\t\tTAY\n");	
	compiler::instance()->pushXY();
}

void comm_op::eval6() const {
	compiler::instance()->printf("\t\tPSH\t#%d\n", op);
	a->eval6();
	b->eval6();
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tPLC\ttmp\n");	
	compiler::instance()->pushA();
}
void comm_op::eval12() const {
	compiler::instance()->printf("\t\tPSH\t#%d\n", op);
	compiler::instance()->printf("\t\tPSH\t#%d\n", op);
	a->eval12();
	b->eval12();
	compiler::instance()->pullXY();
	compiler::instance()->printf("\t\tPLL\ttmp\n");	
	compiler::instance()->printf("\t\tPLL\ttmp+1\n");	
	compiler::instance()->printf("\t\tTXA\n");	
	compiler::instance()->printf("\t\tPLC\ttmp\n");	
	compiler::instance()->printf("\t\tTAX\n");	
	compiler::instance()->printf("\t\tTYA\n");	
	compiler::instance()->printf("\t\tPLC\ttmp\n");	
	compiler::instance()->printf("\t\tTAY\n");	
	compiler::instance()->pushXY();
}



/*
 * Comparisons
 *
 *
 *
 *
 */

/* -- CHAR -- */

void greater_than::eval6() const {
	if(b->typ()->size() == a->typ()->size() && a->typ()->size() == 1) {
		a->eval6();
		b->eval6();
		compiler::instance()->pullA();
		compiler::instance()->printf("\t\tPLL\ttmp\n");
		compiler::instance()->printf("\t\tCMP\ttmp\n");
	} else {
		a->eval12();
		b->eval12();
		compiler::instance()->pullXY();
		compiler::instance()->printf("\t\tPLL\ttmp+1\n");
		compiler::instance()->printf("\t\tPLL\ttmp\n");
		compiler::instance()->printf("\t\tCAD\t(tmp)\n");
	}
	compiler::instance()->printf("\t\tPHP\n");
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tEOR\t#%%100\n"); 
	compiler::instance()->printf("\t\tDIV\t#%%100\n"); 
	compiler::instance()->pushA();
}

void greater_or_equal::eval6() const {
	if(b->typ()->size() == a->typ()->size() && a->typ()->size() == 1) {
		a->eval6();
		b->eval6();
		compiler::instance()->pullA();
		compiler::instance()->printf("\t\tPLL\ttmp\n");
		compiler::instance()->printf("\t\tCMP\ttmp\n");
	} else {
		a->eval12();
		b->eval12();
		compiler::instance()->pullXY();
		compiler::instance()->printf("\t\tPLL\ttmp+1\n");
		compiler::instance()->printf("\t\tPLL\ttmp\n");
		compiler::instance()->printf("\t\tCAD\t(tmp)\n");
	}
	compiler::instance()->printf("\t\tPHP\n");
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tEOR\t#%%100\n");
	compiler::instance()->printf("\t\tDIV\t#%%100\n");
	compiler::instance()->printf("\t\tTSH\t#%%001\n"); // ! Returns 0 on false
	compiler::instance()->pushA();
}

void equals::eval6() const {
	if(b->typ()->size() == a->typ()->size() && a->typ()->size() == 1) {
		b->eval6();
		a->eval6();
		compiler::instance()->pullA();
		compiler::instance()->printf("\t\tPLL\ttmp\n");
		compiler::instance()->printf("\t\tCMP\ttmp\n");
	} else {
		b->eval12();
		a->eval12();
		compiler::instance()->pullXY();
		compiler::instance()->printf("\t\tPLL\ttmp+1\n");
		compiler::instance()->printf("\t\tPLL\ttmp\n");
		compiler::instance()->printf("\t\tCAD\t(tmp)\n"); // FAIL
	}
	compiler::instance()->printf("\t\tPHP\n");
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tEOR\t#%%100\n");
	compiler::instance()->printf("\t\tDIV\t#%%100\n");
	compiler::instance()->printf("\t\tEOR\tA\n");
	compiler::instance()->printf("\t\tPRM\t#%%001\n");
	compiler::instance()->printf("\t\tPRM\t#%%001\n");
	compiler::instance()->printf("\t\tEOR\t#%%001\n");
	compiler::instance()->pushA();
}

void differs::eval6() const {	
	if(b->typ()->size() == a->typ()->size() && a->typ()->size() == 1) {
		a->eval6();
		b->eval6();
		compiler::instance()->pullA();
		compiler::instance()->printf("\t\tPLL\ttmp\n");
		compiler::instance()->printf("\t\tCMP\ttmp\n");
	} else {
		a->eval12();
		b->eval12();
		compiler::instance()->pullXY();
		compiler::instance()->printf("\t\tPLL\ttmp+1\n");
		compiler::instance()->printf("\t\tPLL\ttmp\n");
		compiler::instance()->printf("\t\tCAD\t(tmp)\n");
	}
	compiler::instance()->printf("\t\tPHP\n");
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tEOR\t#%%100\n");
	compiler::instance()->printf("\t\tDIV\t#%%100\n");
	compiler::instance()->printf("\t\tEOR\tA\n");
	compiler::instance()->printf("\t\tPRM\t#1\n");
	compiler::instance()->printf("\t\tPRM\t#1\n");
	compiler::instance()->pushA();
}

void less_than::eval6() const {
	if(b->typ()->size() == a->typ()->size() && a->typ()->size() == 1) {
		a->eval6();
		b->eval6();
		compiler::instance()->pullA();
		compiler::instance()->printf("\t\tPLL\ttmp\n");
		compiler::instance()->printf("\t\tCMP\ttmp\n");
	} else {
		a->eval12();
		b->eval12();
		compiler::instance()->pullXY();
		compiler::instance()->printf("\t\tPLL\ttmp+1\n");
		compiler::instance()->printf("\t\tPLL\ttmp\n");
		compiler::instance()->printf("\t\tCAD\t(tmp)\n");
	}
	compiler::instance()->printf("\t\tPHP\n");
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tEOR\t#%%A00\n"); 
	compiler::instance()->printf("\t\tDIV\t#%%100\n"); 
	compiler::instance()->pushA();
}

void less_or_equal::eval6() const {
	if(b->typ()->size() == a->typ()->size() && a->typ()->size() == 1) {
		b->eval6();
		a->eval6();
		compiler::instance()->pullA();
		compiler::instance()->printf("\t\tPLL\ttmp\n");
		compiler::instance()->printf("\t\tCMP\ttmp\n");
	} else {
		b->eval12();
		a->eval12();
		compiler::instance()->pullXY();
		compiler::instance()->printf("\t\tPLL\ttmp+1\n");
		compiler::instance()->printf("\t\tPLL\ttmp\n");
		compiler::instance()->printf("\t\tCAD\t(tmp)\n");
	}
	compiler::instance()->printf("\t\tPHP\n");
	compiler::instance()->pullA();
	compiler::instance()->printf("\t\tEOR\t#%%100\n"); 
	compiler::instance()->printf("\t\tDIV\t#%%100\n"); 
	compiler::instance()->printf("\t\tTSH\t#%%001\n");
	compiler::instance()->pushA();
}


/* -- WORD -- */

void greater_than::eval12() const {
	compiler::instance()->printf("\t\tPSH\t#0\n");
	eval6();
}

void greater_or_equal::eval12() const {
	compiler::instance()->printf("\t\tPSH\t#0\n");
	eval6();
}

void equals::eval12() const {
	compiler::instance()->printf("\t\tPSH\t#0\n");
	eval6();
}

void differs::eval12() const {
	compiler::instance()->printf("\t\tPSH\t#0\n");
	eval6();
}

void less_than::eval12() const {
	compiler::instance()->printf("\t\tPSH\t#0\n");
	eval6();
}

void less_or_equal::eval12() const {
	compiler::instance()->printf("\t\tPSH\t#0\n");
	eval6();
}


