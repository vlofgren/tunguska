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
#ifndef tryte_h
#define tryte_h

class tryte {
public:	
	class change_hook {
		public:
			change_hook() {}
			virtual ~change_hook() {};

			virtual const tryte& change(tryte& self, const tryte& value) = 0;
	};

protected:
	trit trits[6];
	trit carry;

	change_hook* hook;

	bool changed;
	int cache;
	friend class machine;
	static tryte intval_lookup[730];

	void updatecache() {
		cache = trits[5].to_int() + 3*trits[4].to_int() +
		      	9 * trits[3].to_int() +	27 * trits[2].to_int() + 
			81 * trits[1].to_int() + 243 * trits[0].to_int();
		changed = false;
	}
	static tryte* from_int(register int i);


public:
	tryte();
	tryte(const tryte& t);
	tryte(trit* trits);
	tryte(const char* s);
	tryte(int i);

	int to_int() const {
		if(changed) return trits[5].to_int() + 3*trits[4].to_int() +
		      	9 * trits[3].to_int() +	27 * trits[2].to_int() + 
			81 * trits[1].to_int() + 243 * trits[0].to_int();
		else return cache;
	}
	int to_int() {
		if(changed) updatecache();
		return cache;
	}
	bool is_changed() { return changed; }

	/* Translate two trytes into an integer */
	static int word_to_int(const tryte& high, const tryte& low) {
		return low.to_int() + 3*243*high.to_int();
	}

	/* Translata an integer into two trytes */
	static void int_to_word(int value, tryte& high, tryte& low) {
		low = value;
		high = (value - low.to_int())/729;
	}


	/* Return value in nonary, in hexadecimal notation for use with printf's
	 * %x */
	int nonaryhex() {
		char l1 = (*this)[5].to_int() + (*this)[4].to_int() * 3, 
		     l2 = (*this)[3].to_int() +  (*this)[2].to_int() * 3, 
		     l3 = (*this)[1].to_int() + (*this)[0].to_int() * 3;
		int returnval = 0;
		switch(l1) {
			case -4: returnval += 0xd; break;
			case -3: returnval += 0xc; break;
			case -2: returnval += 0xb; break;
			case -1: returnval += 0xa; break;
			case 0: returnval += 0; break;
			case 1: returnval += 0x1; break;
			case 2: returnval += 0x2; break;
			case 3: returnval += 0x3; break;
			case 4: returnval += 0x4; break;
		}
		switch(l2) {
			case -4: returnval += 0xd0; break;
			case -3: returnval += 0xc0; break;
			case -2: returnval += 0xb0; break;
			case -1: returnval += 0xa0; break;
			case 0: returnval += 0; break;
			case 1: returnval += 0x10; break;
			case 2: returnval += 0x20; break;
			case 3: returnval += 0x30; break;
			case 4: returnval += 0x40; break;
		}
		switch(l3) {
			case -4: returnval += 0xd00; break;
			case -3: returnval += 0xc00; break;
			case -2: returnval += 0xb00; break;
			case -1: returnval += 0xa00; break;
			case 0: returnval += 0; break;
			case 1: returnval += 0x100; break;
			case 2: returnval += 0x200; break;
			case 3: returnval += 0x300; break;
			case 4: returnval += 0x400; break;
		}

		return returnval;

	}

	void set_hook(change_hook* hook) { this->hook = hook; }
	change_hook* get_hook() const { return hook; }

	const trit& operator[](int i) const { return trits[i]; }
	trit& operator[](int i) { changed = true; return trits[i]; }
	tryte operator~() const;
	tryte operator&(const tryte a) const;
	tryte operator|(const tryte a) const;
	tryte operator^(const tryte a) const;

	tryte& operator&=(tryte &a);
	tryte& operator|=(tryte &a);
	tryte& operator=(const tryte &a);
	tryte& operator=(const int v);
	int operator==(const tryte &a) const;
	int operator!=(const tryte &a) const;
	int operator<(const tryte &a) const;
	int operator>(const tryte &a) const;

	tryte operator+(const tryte& a) const;
	tryte operator+(const int v) const;
	tryte& operator+=(const tryte& a);
	tryte& operator+=(int v);
	tryte operator*(const tryte& a) const;
//	tryte operator,(const tryte& a) const; <-- deprecated!
	tryte mlh(const tryte& a) const;
	tryte operator%(const tryte& a) const;
	tryte operator/(const tryte& a) const;
	tryte operator%(const int v) const;
	tryte operator/(const int v) const;

	trit parity() const;

	tryte permute(const tryte& t) const;
	tryte& permute_this(const tryte& t);
	tryte shift(const tryte& t) const;
	tryte& shift_this(const tryte& t);
	tryte but(const tryte& t) const;
	tryte& but_this(const tryte& t);
	tryte comm_op(int op, const tryte& t) const;
	tryte& comm_op_this(int op, const tryte& t);

	tryte operator<<(int s) const;
	tryte operator>>(int s) const;

	const trit get_carry() const { return carry; } 

};

#endif
