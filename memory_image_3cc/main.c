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


#define TEST(TEST) puts(#TEST " = "); putnum(TEST); putc(2);

#include "string.3h"
#include "stddef.3h"
#include "graphics.3h"
#include "stdio.3h"
#include "agdp.3h"
#include "system.3h"
#include "math.3h"
#include "demos.3h"

void repaint();
void scroll();
void mandelbrot();
char user_input(char* s);

char test(int value) {
	TEST(value);
	return value;
}

int fib(int n) {
	if(n <= 1) return 1;
	return fib(n-2) + fib(n-1);
}


void main() {
	int line = 0;
	static int p[10];
	char x = 1;
	puts("Welcome to the 3CC memory image!\n\n");
	malloc_init();

	for(;;) {
		static char[50] s;

		putnum(line++);
		puts(" PROMPT> ");

		if(gets(&s, 50)) { // Read user string
			if(-user_input(s)) {
				puts(s); // Print user string
				puts(" -- Huh?\n");
			}
		}
	}
}

char user_input(char* s) {
	struct machine_status* ms = get_ms();

	if(strcmp(s, "HELP") == 0) {
		puts("If you by HELP mean <I want to help>, press 1.\n"
		     "If you by HELP mean <I need help>, press 2.\n"
		     "If you by HELP mean <I dont need help>, press 3.\n"
		     "Nah, seriously though, available commands are=\n"
		     "    MOUSE -- Print mouse coordinates. \n"
		     "    RANDOM -- Print a random number. \n"
		     "    PAINT -- Unleash your inner artist. Anykey exits.\n"
		     "    MANDELBROT -- Mandelbrot fractal. \n"
		     "    BROWN -- Random walk (brownian motion). \n"
		     "    SPIN -- Mildly nauseating vector gfx demo. \n\n");
	} else if(strcmp(s, "MOUSE") == 0) {
		puts("Mouse coordinates=");
		putnum(ms->mouse_x); putc('x');
		putnum(ms->mouse_y); putc('\n');
	} else if(strcmp(s, "RANDOM") == 0) {
		TEST(random());
	} else if(strcmp(s, "PAINT") == 0) {
		paint();
	} else if(strcmp(s, "MANDELBROT") == 0) {
		mandelbrot();
	} else if(strcmp(s, "BROWN") == 0) {
		brown();
	} else if(strcmp(s, "SPIN") == 0) {
		spin();
	} else return -1;
	return 1;
}


void sys_agdp(char op, int R1, int R2, int R3) {
	asm("PHP", "SEI");
	sys_agdp_registers[0] = R1;
	sys_agdp_registers[1] = R2;
	sys_agdp_registers[2] = R3;
	sys_agdp_operation = op;
	asm("PLP");
}

void repaint() {
	static char* video_mode_register = 0nDDDDDB;
	struct machine_status* ms = get_ms();
	*video_mode_register = ms->video_mode_repaint_trit;
}

char random() {
	struct machine_status* ms = get_ms();
	while(ms->random_update == 0);
	ms->random_update = 0;
	return ms->random;
}

struct machine_status* get_ms() {
	static struct machine_status ms;
	return &ms;
}


