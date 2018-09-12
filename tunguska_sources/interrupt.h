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

#ifndef interrupt_h
#define interrupt_h

class interrupt {
	public:
		interrupt() {};
		virtual ~interrupt() {};
		virtual int IRQ() = 0;
		virtual int data() = 0;
};

class soft_interrupt : public interrupt {
	public:
		soft_interrupt() {}
		virtual ~soft_interrupt() {}
		virtual int IRQ();
		virtual int data();
};

class arith_interrupt : public interrupt {
	public:
		arith_interrupt() { }
		virtual ~arith_interrupt() {}
		virtual int IRQ();
		virtual int data();
	private:
};

class keybreak_interrupt : public interrupt {
	public:
		keybreak_interrupt() {} 
		virtual ~keybreak_interrupt() {}
		virtual int IRQ();
		virtual int data();
};

class clock_interrupt : public interrupt {
	public:
		clock_interrupt() {} 
		virtual ~clock_interrupt() {}
		virtual int IRQ();
		virtual int data();
};

class keyboard_interrupt : public interrupt {
	public:
		keyboard_interrupt(int data) {this->kdata = data;} 
		virtual ~keyboard_interrupt() {}
		virtual int IRQ();
		virtual int data();
	private:
		int kdata;
};

class mousemotion_interrupt : public interrupt {
	public:
		mousemotion_interrupt(int xdata, int ydata) {
			this->xdata = xdata;
			this->ydata = ydata;
		} 
		virtual ~mousemotion_interrupt() {}
		virtual int IRQ();
		virtual int data();
	private:
		int xdata, ydata;
};

class mousepress_interrupt : public interrupt {
	public:
		mousepress_interrupt(int data) { this->pdata = data;} 
		virtual ~mousepress_interrupt() {}
		virtual int IRQ();
		virtual int data();
	private:
		int pdata;

};

#endif
