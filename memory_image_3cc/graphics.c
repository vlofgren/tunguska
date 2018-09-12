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

#include "graphics.3h"
#include "system.3h"
#include "string.3h"

void clear_screen() {
	struct machine_status* ms = get_ms();
	if(ms->video_mode_repaint_trit == VMR_RASTER3) {
		memset(0nDDBDDD, 0nDDD, sizeof(sys_raster_buffer_tri));
	} else if(ms->video_mode_repaint_trit == VMR_RASTER729) {
		memset(0nDDBDDD, 0nDDD, sizeof(sys_raster_buffer));
	} else {
		memset(0nDDBDDD, 0, 0n444);
	}
}
void putpixel(char x, char y, char color) {
	struct machine_status* ms = get_ms();

	x = ~x*x;
	y = ~y*y;
	if(x >= 324 || y >= 243) return;

	if(ms->video_mode_repaint_trit == VMR_RASTER3) {
		putpixel_tricolor(x, y, color);
	} else if(ms->video_mode_repaint_trit == VMR_RASTER729) {
		putpixel_hicolor(x, y, color);
	}
}

void putpixel_tricolor(char x, char y, char color) {
	char mask[] = { 243, 81, 27, 9, 3, 1 };

	/* Make sure color is only one trit, and
	 * shift it to the trit indicated by mask[x%6] */
	color = (~color) * mask[x%6];

	/* Clear the relevant trit */
	/* XOR is inverted tritwise multiplication, so shifting the mask down one
	 * bit effectively leaves every trit but the masked one negative 
	 * (the masked is zero) thus allowing us to clear that trit */
	sys_raster_buffer_tri[y][x/6] ^= (mask[x%6] <S> 0nDDD);

	/* Shift the masked trit in the direction of color */
	sys_raster_buffer_tri[y][x/6] = sys_raster_buffer_tri[y][x/6]<S>color;
}

void putpixel_hicolor(char x, char y, char color) {
	sys_raster_buffer[y][x] = color;
}
