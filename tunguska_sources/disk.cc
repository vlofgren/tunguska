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

#include "disk.h"
#include "values.h"
#include <stdio.h>

disk::disk(machine& m) : page("DDD"), m(m), filename(NULL), is_loaded(false) {
	m.memrefi(TV_DDD, TV_DDA).set_hook(new disk_change_hook(*this));
}

disk::~disk() {

}

void disk::load(const char* filename) {
	if(is_loaded) unload();
	if(!filename) return;


	try {
		memory::load(filename);
	} catch(std::runtime_error* e) {
		printf("OP_LOAD: Failed to load disk %s (%s)\n", filename, 
			e->what());
		delete e;
		return;
	}

	this->filename = strdup(filename);
	is_loaded = true;
}

void disk::unload() {
	if(!is_loaded) return;

	is_loaded = false;
	if(filename) free(filename);
}

/* Floppy disk communication works through the memory position
 * DDD:DDA. In general, the A register specifies the operation
 * (see enumeration in disk.h), and Y the memory position in the
 * machine. The disk keeps track of it's internal position, which
 * auto increments after reading or writing. It can be manually set
 * with seek, and checked with getpos. It will wrap around from 444:444
 * to DDD:DDD. 
 *
 * Whenever data has been written, a sync call must be
 * made to actually write it to disk.
 *
 */

void disk::heartbeat() {
	static int offset = tryte::word_to_int("DDD", "DDA");
	tryte &d = m.memref(offset);
	switch(d.to_int()) {
		case DISKOP_NOOP: return;
		case DISKOP_READ: read(); break;
		case DISKOP_WRITE: write(); break;
		case DISKOP_SYNC: {
			printf("Saving to %s\n", filename);
			save(filename);
		     } break;
		case DISKOP_SEEK: seek(m.A); break;
		case DISKOP_GETPOS: m.A = getpos(); break;
		case DISKOP_STATUS: m.A = is_loaded; break;
		case DISKOP_UNLOAD: unload(); break;
		case DISKOP_LOAD: do_load(); break;

		default: printf("Unknown disk operation %d\n", d.to_int());
	}

	d = DISKOP_NOOP;
}
void disk::read() {
	for(int o = -364; o <= 364; o++) {
		int diskpos = tryte::word_to_int(page, o);
		int mempos = tryte::word_to_int(m.Y, o);

		m.memref(mempos) = memref(diskpos);
	}

	page = page + 1;
}

void disk::write() {
	for(int o = -364; o <= 364; o++) {
		int diskpos = tryte::word_to_int(page, o);
		int mempos = tryte::word_to_int(m.Y, o);

		memref(diskpos) = m.memref(mempos);
	}

	page = page + 1;
}

void disk::seek(const tryte& t) {
	page = t;
}

void disk::status() {
	m.A = is_loaded;
}

tryte disk::getpos() const {
	return page;
}

void disk::do_load() {
	char name[255];
	int ni = 0;
	int pos = tryte::word_to_int(m.X, m.Y);
	for(ni = 0; ni < 255; ni++) {
		name[ni] = ternarytoascii(m.memref(pos+ni).to_int());
		if(name[ni] == 0) break;
	}

	if(ni == 0 || ni == 255) {
		printf("DO_LOAD failed, filename '%s' not valid", name);
		return;
	}

	disk::load(name);
}
