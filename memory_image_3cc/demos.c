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

#include "demos.3h"
#include "system.3h"
#include "stddef.3h"
#include "stdio.3h"
#include "math.3h"

/* Mandelbrot fractal renderer */
char mandelbrot_test(int x, int y, int limit) {
	int cx = x;
	int cy = y;
	int cx2 = cx*cx/81;
	int cy2 = cy*cy/81;
	int iters = 0; 
	for(;cx2 + cy2 < 4*81; iters++) {
		cy = 2 * cx * cy / 81 + y;
		cx = cx2 - cy2 + x;
		cx2 = cx * cx / 81;
		cy2 = cy * cy / 81;

		if(iters > limit) return 0n444;
	}
	return 0nDDD;
}

void mandelbrot() {
	int x;
	int y;
	int limit;
	char buffer[10];
	struct machine_status* ms = get_ms();

	puts("MANDELBROT. Press Any Key to exit once rendering.\n\n");
	puts("How deep would you like to iterate? (Anything over 200 will "
	     "take AGES! 50 is a good suggestion)\n\nIteration limit = ");
	gets(buffer, 10);
	limit = atoi(buffer);
	while(limit <= 0) {
		puts("Thats no good. Positive integer please...\nIteration limit = ");
		gets(buffer, 10);
		limit = atoi(buffer);
	}

	ms->video_mode_repaint_trit = VMR_RASTER729;
	clear_screen();
	ms->echo_input = FALSE;

	/* Interlace for speed and great justice */
	for(x = 0; x < 324; x++) {
		int count = 0;
		int v0 = mandelbrot_test(x-162, 0, limit);
		sys_raster_buffer[121][x] = v0;

		for(y = 0; y < 121; y++) {
			char value;
			if(count > 10 && v0 == 0n444) value = 0n444;
			else { 
				value = mandelbrot_test(x-162, 4*(y-121)/3, limit);
				if(value == 0n444) count++;
			}

			/* Speed is of the essence, and the coordinates
			 * are by definition within the screen area,
			 * so direct memory hacking is due. The
			 * mandelbrot fractal is also symmetric around
			 * the X axis, which cuts calculations down to half.*/
			sys_raster_buffer[y][x] = value;
			sys_raster_buffer[242-y][x] = value;
				
		}
		if(getc_noblock() != 0) break; 
	}
	
	if(x == 324) getc();
	ms->echo_input = TRUE;
	ms->video_mode_repaint_trit = VMR_TEXT;
	
}

/* Brownian motion */
void brown() {
	char x = 100;
	char y = 100;
	struct machine_status* ms = get_ms();
	char iters = 0;

	ms->video_mode_repaint_trit = VMR_RASTER729;
	clear_screen();
	ms->echo_input = FALSE;

	while(getc_noblock() == 0)  {
		char rand = random();
		x+=rand^1; // LSB
		y+=~rand;  // MSB
		iters+=~(char)(3*rand);

		x=~x*x;
		y=~y*y;
		if(x >= 324) x = 323;
		if(y >= 243) y = 242;

		putpixel(x, y, iters);
	}
	
	ms->echo_input = TRUE;
	ms->video_mode_repaint_trit = VMR_TEXT;
	clear_screen();
}



void paint() {
	struct machine_status* ms = get_ms();
	char color = 0n444;

	ms->video_mode_repaint_trit = VMR_RASTER729;
	clear_screen();
	ms->echo_input = FALSE;

	for(;;) {
		char x = ms->mouse_x;
		char y = ms->mouse_y;
		char key = getc_noblock();

		putpixel(x, y, color);

		if(key == 'q' || key == 'Q') {
			break;
		}
		else if(key == '0') color = 0n444;
		else if(key == '1') color = 0n333;
		else if(key == '2') color = 0n222;
		else if(key == '3') color = 0n111;
		else if(key == '4') color = 0n000;
		else if(key == '5') color = 0nAAA;
		else if(key == '6') color = 0nBBB;
		else if(key == '7') color = 0nCCC;
		else if(key == '8') color = 0nDDD;

	}

	ms->echo_input = TRUE;
	clear_screen();
	ms->video_mode_repaint_trit = VMR_TEXT;
}

void spin() {
	struct machine_status* ms = get_ms();
	char x = 0; char y = 0;
	char xn[5];
	char yn[5];
	char dx[4];
	char dy[5];
	int angle = 0;
	int dangle = 0;
	int d;
	int run = TRUE;
	clear_screen();
	ms->video_mode_repaint_trit = VMM_VECTOR;
	ms->echo_input = FALSE;

	for(x = 0; x < 5; x++) {
		sys_vector_buffer[5*x].color = 0nDDD;
		sys_vector_buffer[5*x+1].color = 0n400;
		sys_vector_buffer[5*x+2].color = 0n004;
		sys_vector_buffer[5*x+3].color = 0n400;
		sys_vector_buffer[5*x+4].color = 0n004;
	}

	for(dangle = 0;run;dangle++) {
		d = cos(dangle)/9;
		for(angle = 0; angle < 0n2000; angle++) {
			int i;
			int j;

			for(i = 0; i < 4; i++) {
				dx[i] = (int)(d*cos(angle/2 + i*182)/364);
				dy[i] = (int)(d*sin(angle/2 + i*182)/364);
				sys_vector_buffer[i].x = xn[i] = cos(-angle + i*182)/9;
				sys_vector_buffer[i].y = yn[i] = sin(-angle + i*182)/9;
			}
			sys_vector_buffer[4].x = xn[4] = xn[0];
			sys_vector_buffer[4].y = yn[4] = yn[0];

			for(i = 1; i < 5; i++) {
				for(j = 0; j < 5; j++) {
					sys_vector_buffer[j+5*i].x = xn[j] + dx[i-1];
					sys_vector_buffer[j+5*i].y = yn[j] + dy[i-1];
				}
			}
		
			if(getc_noblock()) { 
				run = FALSE;
				break;
			}
			sys_graphics_register = VMR_VECTOR;
		}

	}


	ms->echo_input = TRUE;
	ms->video_mode_repaint_trit = VMR_TEXT;
	clear_screen();
}
