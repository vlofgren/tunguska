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

/* !!!! NOTICE !!!!
 *
 * ONLY USE THIS ERROR CLASS WITH THE ASSEMBLER.
 *  Throws are SLOW. It's a case of speed vs. fancy error
 *  management, speed is vastly preferable in this case.
 */

#ifndef error_h
#define error_h

#include <string>
using namespace std;
#define WHERE new error::where(__LINE__,__FILE__)
class error {
	public:
		class where;
		error(where* place, string desc, error* child = 0) {
			this->desc = desc;
			this->place = place;
			this->child = child;
		}

		void trace() {
			if(child) child->trace();
			printf("* %s[%d]: %s\n", place->get_file().c_str(),
						place->get_line(),
						desc.c_str());
		}
		class where {
			public:
				where(int line, string file) {
					this->line = line;
					this->file = file;
				}
				int get_line() { return line; }
				string get_file() { return file; }
			protected:
				int line;
				string file;
		};
	protected:
		string desc;
		where* place;
		error* child;
};

#endif
