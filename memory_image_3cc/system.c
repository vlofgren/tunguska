/* Tunguska, ternary virtual machine
 *
 * Copyright (C) 2007,2008 Viktor Lofgren
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

#include "system.3h"
#include "stddef.3h"

void main();

/* This rather unsightly blob of code allows direct memory access
 * to several useful memory registers. While it might look very primordial,
 * just defining global variables like this in specific memory locations,
 * it sure beats the even more arcane idea of seemingly grabbing memory
 * locations out of thin air (well, the murky depths of the tunguka specs), 
 * assigning them to pointers and using those to access this data. */

#pragma SCL // Save current location

# pragma ORIGIN 0nDDDDDD
 char sys_irqno;
 char sys_irqdata;
 char sys_graphics_register;
 char sys_disk_op;
 char sys_agdp_operation;
 int sys_agdp_registers[3];
# pragma ORIGIN 0nDDBDDD
 char sys_text_buffer[27][54];
# pragma ORIGIN 0nDDBDDD
 char sys_raster_buffer[243][324];
# pragma ORIGIN 0nDDBDDD
 char sys_raster_buffer_tri[243][54];
# pragma ORIGIN 0nDDBDDD
 struct vector sys_vector_buffer[121];

# pragma ORIGIN 0
/* This is run at the very beginning */
void boot() {
	static int bootstrapped = 0;
	asm("SEI");
  	*(int*)(SYS_ISR) = 0n444DDD;	// Interrupt Service Routine address
	*(char*)(SYS_IRQFRQ) = 0n444;	// Interrupt delay
	*(char*)(SYS_CHARBUF) = 0;	// Character buffer
  	asm("CLI");
	get_ms()->echo_input = TRUE;
	get_ms()->video_mode_repaint_trit = VMR_TEXT;

	if(bootstrapped) puts("Possible jump to nonsensical address\n");
	bootstrapped = 1;

        main();
}

# pragma ORIGIN 0n444DDD

/* Set up the interrupt service routine 
 *
 * !!!It is of utmost importance that the value of tmp
 * is NOT changed during ISR.  
 *
 * */

void isr() {
	static int count = 0;
	static struct machine_status* ms;

	// Push tmp before evaluating any addresses
	asm(	"PSH tmp",
	    	"PSH tmp+1");
	ms = get_ms();

	if(sys_irqno == 0) { // IRQ 0 => Keyboard input
		ms->key = sys_irqdata;
	} else if(sys_irqno == 1) { // IRQ 1 => Clock input
		// Repaint the screen every 4,000 non-ISR instructions or so.
		ms->random = sys_irqdata;
		ms->random_update = 1;

		if(count++ > 10) { // Count is static
			repaint();
			count = 0;
		}
	} else if(sys_irqno == 5) {
		ms->mouse_x += sys_irqdata / 27;
		ms->mouse_y += sys_irqdata ^ 0t000NNN;
	} else { // Other IRQs
	//	asm("DEBUG");
	}

	asm(	"PLL tmp+1",
		"PLL tmp",
		"RTI");		// Return with RTI, and not RST!
}
#pragma RCL // Restore old location

#define MALL_MAX 1000

int memory_nodes[MALL_MAX];

#define MALL_FREE 0
#define MALL_BEGIN 1
#define MALL_CONTIN 2
#define MALL_MEMORY (void*) 0nC00000
#define MALL_CHUNK 27

//#define MALLOC_VERBOSE

void malloc_init() {
	int i;
	for(i = 0; i < MALL_MAX; i++) memory_nodes[i] = MALL_FREE;

#ifdef MALLOC_VERBOSE
	puts("COMPILED WITH    MALLOC=VERBOSE\n");
#endif
}

void* malloc(int size) {
	int start;
	size = size / MALL_CHUNK + 1;
#ifdef MALLOC_VERBOSE
	puts("MALLOC   WORDS = ");
	putnum(size);
	puts("  ");
#endif

	for(start = 0; start < MALL_MAX - size; start++) {
		if(memory_nodes[start] == MALL_FREE) {
			int seek = 0; int good = 1;

			for(seek = 1; seek < size; seek++) {
				if(memory_nodes[start + seek] != MALL_FREE) {
					good = 0;
					start += seek;
					break;
				}
			}

			if(good) {
#ifdef MALLOC_VERBOSE
				puts("  START = ");
				putnum(start);
				puts(" \n");
#endif
				memory_nodes[start] = MALL_BEGIN;
				for(seek = 1; seek < size; seek++) {
					memory_nodes[start+seek] = MALL_CONTIN;
				}
				return MALL_MEMORY + start * MALL_CHUNK;
			} 	
		}
	}

	return NULL;
}

void free(void* memory) {
	int offset = memory - MALL_MEMORY;
	int i = 1;

	if(offset % 27 != 0) puts("Invalid free\n");
	if(memory_nodes[offset / MALL_CHUNK] != MALL_BEGIN) puts("Invalid free!\n");
	
	memory_nodes[offset / MALL_CHUNK] = MALL_FREE;
	while(memory_nodes[offset/MALL_CHUNK + i] == MALL_CONTIN) {
		memory_nodes[offset / MALL_CHUNK + i] = MALL_FREE;
		i++;
	}
#ifdef MALLOC_VERBOSE
	puts("FREE     WORDS = ");
	putnum(i);
	puts("    START = ");
	putnum(offset / MALL_CHUNK);
	puts("\n");
#endif
}
