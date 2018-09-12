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

#include "machine.h"

#ifndef disk_h
#define disk_h

class disk : public memory {
public:
	disk(machine& m);
	virtual ~disk();

	void seek(const tryte& t);
	tryte getpos() const;

	void write();	/* Write to -disk-, read from -memory- */
	void read();	/* Read from -disk-, write to -memory- */
	virtual void load(const char* filename);
	void unload();
	void status();

	void heartbeat();
	void do_load();

	enum {
		DISKOP_NOOP = 0, 
		DISKOP_READ = 1, 
		DISKOP_WRITE = 2,
		DISKOP_SYNC = 3, 
		DISKOP_SEEK= 4, 
		DISKOP_GETPOS = 5,

		DISKOP_STATUS = 6,
		DISKOP_UNLOAD = 7,
		DISKOP_LOAD = 8,

	};
private:
	tryte page;

	machine& m;
	char* filename;
	bool is_loaded;
};

class disk_change_hook : public tryte::change_hook {
	public:
		disk_change_hook(disk& d) : floppy(d), recursion_lock(false) {}
		virtual ~disk_change_hook() {}

		virtual const tryte& change(tryte& self, const tryte& value) { 
			if(recursion_lock) return value;
			recursion_lock = true;
			floppy.heartbeat();
			recursion_lock = false;
			return value;
		}
	private:
		disk& floppy;
		double recursion_lock;

};

#endif
