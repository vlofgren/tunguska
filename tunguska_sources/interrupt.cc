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

#include "interrupt.h"
#include "tryte.h"
#include <stdlib.h>

int mousepress_interrupt::IRQ() { return 6; }
int mousemotion_interrupt::IRQ() { return 5; }
int soft_interrupt::IRQ() { return 4; }
int arith_interrupt::IRQ() { return 3; }
int keybreak_interrupt::IRQ() { return 2; }
int clock_interrupt::IRQ() { return 1; }
int keyboard_interrupt::IRQ() { return 0; }

int mousepress_interrupt::data() { return pdata; }
int mousemotion_interrupt::data() { 
	if(xdata > 13) xdata = 13;
	else if(xdata < -13) xdata = -13;
	if(ydata > 13) ydata = 13;
	else if(ydata < -13) ydata = -13;
	tryte x = xdata;
	tryte y = ydata;
	return ((x << 3) + (y ^ "0AD")).to_int();
}
int soft_interrupt::data() { return 0; }
int arith_interrupt::data() { return 0; }
int keybreak_interrupt::data() { return 0; }
int clock_interrupt::data() { return -364 + random() % 729; }
int keyboard_interrupt::data() { return kdata; }
