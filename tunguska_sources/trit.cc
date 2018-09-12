/* Tunguska, ternary virtual machine
 *
 * Copyright (C) 2007-2009 Viktor Lofgren
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "trit.h"
#include "tryte.h"
#include <stdlib.h>
#include <stdio.h>

const trit::trit_strategy* trit::strategy = new trit_strategy_tuf_calculate();

/* Trit strategy for True-Unknown-False logic employing runtime calculation */

tval_t trit::trit_strategy_tuf_calculate::op_inv(tval_t a) const { return -a; }
tval_t trit::trit_strategy_tuf_calculate::op_and(tval_t a, tval_t b) const { 
	if(a == trit::TRTRUE && b == trit::TRTRUE) return trit::TRTRUE;
	if((a == trit::TRFALSE || b == trit::TRFALSE)) return trit::TRFALSE;
	return trit::TRMU;

}
tval_t trit::trit_strategy_tuf_calculate::op_or(tval_t a, tval_t b) const {
	if(a == trit::TRTRUE || b == trit::TRTRUE) return trit::TRTRUE;
	if(a == trit::TRMU || b == trit::TRMU) return trit::TRMU;
	return trit::TRFALSE;
}
tval_t trit::trit_strategy_tuf_calculate::op_xor(tval_t a, tval_t b) const {
	return -a*b;
}
tval_t trit::trit_strategy_tuf_calculate::op_tsh(tval_t a, tval_t b) const {
	if(a == 1) {
		return abs((1 + a + b) % 3 - 1);
	} else if(a == -1) {
		return -abs((1 + a + b) % 3 - 1);
	} else return b;
}
tval_t trit::trit_strategy_tuf_calculate::op_rol(tval_t a, tval_t b) const {
	return (1 + a + b) % 3 - 1;
}
tval_t trit::trit_strategy_tuf_calculate::op_but(tval_t a, tval_t b) const {
	if(a == trit::TRMU || b == trit::TRMU) return trit::TRMU;
	if(a == trit::TRFALSE|| b == trit::TRFALSE) return trit::TRFALSE;
	return trit::TRTRUE;
}


// Trit class


trit& trit::operator=(const register trit& t) {
	data = t.data;
	return *this;
}

trit& trit::operator=(register int i) {
	data = i;
	return *this;
}

int trit::operator!() const {
	return -1 * data;
}

int trit::operator&(const trit& t) const {
	return strategy->op_and(data, t.data);
}

int trit::operator|(const trit& t) const {
	return strategy->op_or(data, t.data);

}

int trit::operator^(const trit& t) const {
	return strategy->op_xor(data, t.data);
}

trit& trit::operator|=(const trit& t) {
	data = strategy->op_or(data, t.data);

	data = trit(*this | t).data;
	return *this;
}

trit& trit::operator&=(const trit& t) {
	data = strategy->op_and(data, t.data);
	return *this;
}

trit& trit::operator^=(const trit& t) {
	data = strategy->op_xor(data, t.data);
	return *this;
}

int trit::operator==(const trit& t) const {
	if(t.data == this->data) return TRFALSE;
	else return TRTRUE;
}

int trit::operator!=(const trit& t) const {
	return -1 * (*this == t);
}

/* Rotation operator, it shifts the value in the direction t
 *
 * It is sort-of related to binary NOT operator, in the sense that
 * the same permutation in a binary system would result in NOT. 
 *
 * It has some useful properties, 
 * associative and commutative, and 3-cyclic
 *
 * A perm B perm NOT B = A perm B perm B perm B = A
 *
 * Truth table:
 * A | T | A > B 
 * --+---+---------
 * + | + |  -       
 * 0 | + |  +
 * - | + |  0
 * + | 0 |  +
 * 0 | 0 |  0
 * - | 0 |  -
 * + | - |  0
 * 0 | - |  -
 * - | - |  +
 *
 * */
int trit::operator>(const trit& t) const { 
	return strategy->op_rol(data, t.data);

}

/* Same as operator>, only it doesn't roll over 
 *
 * A | T | A > B 
 * --+---+---------
 * + | + |  +       
 * 0 | + |  +
 * - | + |  0
 * + | 0 |  +
 * 0 | 0 |  0
 * - | 0 |  -
 * + | - |  0
 * 0 | - |  -
 * - | - |  -
 *
 * */
int trit::operator>>(const trit& t) const { 
	return strategy->op_tsh(data, t.data);

}

/* Ternary BUT operator, complement to AND and OR
 *
 * Truth table:
 *
 * A | B | A BUT B
 * --+---+--------
 * - | - | -
 * 0 | - | 0
 * + | - | -
 * - | 0 | 0
 * 0 | 0 | 0
 * + | 0 | 0
 * - | + | -
 * 0 | + | 0
 * + | + | +
 *
 *
 */
int trit::but(const trit& t) const {
	return strategy->op_but(data, t.data);

}

trit& trit::but_this(const trit& t) {
	data = strategy->op_but(data, t.data);

	return *this;
}
int trit::comm_op(int op, const trit& t) const {
	tryte operation(op);
	int min = data < t.data ? data : t.data;
	int max = data < t.data ? t.data : data;
	if(min == max) {
		if(min == TRFALSE) return operation[0].to_int();
		else if(min == TRMU) return operation[2].to_int();
		else return operation[4].to_int();
	} else {
		if(min == TRFALSE) {
			if(max == TRMU) 
				return operation[1].to_int();
			else	return operation[5].to_int();
		}
 		return operation[3].to_int();
	}
}

trit& trit::comm_op_this(int op, const trit& t) {
	data = comm_op(op, t);
	return *this;
}
