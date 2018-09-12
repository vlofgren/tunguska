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

#include "tryte.h"
#include <stdexcept>

#ifndef memory_h
#define memory_h

#define MEMSIZ 729*729

class memory { 
	public:
		memory() {
			mem = new tryte[MEMSIZ];
		}
		~memory() { delete[] mem; }
		tryte& memref(int pos);
		tryte& memref(const tryte& low, const tryte& high);
		tryte& memrefi(int low, int high);

		/* Memory image management */
		void load(const char* filename) throw(std::runtime_error*);
		void save(const char* filename);
	protected:
		/* Virtual memory */
		tryte* mem;
};


#endif
