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
#include "memory.h"
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include "values.h"

#define IF_BIG_ENDIAN \
	union { short s; unsigned char c[2]; }; \
	s = 0x1234; if(c[0]==0x12 && c[1]==0x34)

#define TG_SWAP(t) \
	((((t)>>8)&0xFF)|(((t)<<8)&0xFF00))

/* Functions for loading and saving memory dumps */
/* Since most of the time these are mainly full of zeros,
 * compression can yield 100-fold or more size reduction */
void memory::load(const char* filename) throw(std::runtime_error*) {
	gzFile f = gzopen(filename, "rb");
	if(f == NULL) { 
		printf("Couldn't open '%s' for reading :-(\n", 
			filename);
		throw new std::runtime_error("memory::load failed");
	}

	short* data = new short[MEMSIZ];
	int memdata_len = MEMSIZ;

	memdata_len = gzread(f, data, memdata_len * sizeof(short));

	int be=0; IF_BIG_ENDIAN be=1;

	for(unsigned int t = 0; t < memdata_len / sizeof(short); t++) {
		if(be) data[t]=TG_SWAP(data[t]);
		mem[t] = data[t];
	}
	gzclose(f);

	delete[] data;
}

void memory::save(const char* filename) {
	gzFile f = gzopen(filename, "wb9");

	if(f == NULL) { 
		printf("Couldn't open '%s' for writing:-(\n", 
			filename);
		return;
	}

	short* data = new short[MEMSIZ];
	
	int be=0; IF_BIG_ENDIAN be=1;

	for(int t = 0; t < MEMSIZ; t++) {
		data[t] = mem[t].to_int();
		if(be) data[t]=TG_SWAP(data[t]);
	}

	gzwrite(f, ((char*) data), MEMSIZ * sizeof(short));

	gzclose(f);

	delete[] data;
}

/* Memory referencer */
tryte& memory::memref(register int pos) {
	if(pos < -MEMSIZ/2) {
		pos = -((-pos + MEMSIZ/2) % MEMSIZ - MEMSIZ/2);
	} else if(pos > MEMSIZ/2) {
		pos = ((pos + MEMSIZ/2) % MEMSIZ - MEMSIZ/2);
	}

	return mem[pos+MEMSIZ/2];
}

/* Integer memory referencer (for speed, use with vaues.h) */
tryte& memory::memrefi(register int low,  register int high) {
	return memref(PODWORD_TO_INT(low, high));
}

/* Tryte memory referencer */
tryte& memory::memref(const tryte& high, const tryte& low) {
	int pos = tryte::word_to_int(high, low); 
	return memref(pos);
}


