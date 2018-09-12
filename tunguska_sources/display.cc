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

#include "share_dir.h"
#include "display.h"
#include "values.h"

display::display(machine& mach) : m(mach) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_WM_SetCaption("TernCPU", 0);
	window = SDL_SetVideoMode(XRES, YRES, 16, SDL_HWSURFACE);

	if(window == NULL) {
   		printf("Unable to open a window (SDL_SetVideoMode fails).\n"
		"* SDL says the problem is \"%s.\"\n", SDL_GetError());
		printf("This is a critical error. Bailing out. \n\n");
		exit(EXIT_FAILURE);
	}

	symbols = SDL_LoadBMP(SHARE_DIR "symbols.bmp");
	bg = SDL_LoadBMP(SHARE_DIR "bg.bmp");

	if(symbols == NULL) {
		printf("'symbols.bmp' not found in '%s'\n", 
				SHARE_DIR);
		printf("Falling back to 'share/symbols.bmp' ...");
		symbols = SDL_LoadBMP("share/symbols.bmp");
		if(symbols == NULL) {
			printf(" failure :-(\n");
			exit(EXIT_FAILURE);
		} else printf(" success!\n");
	}
	if(bg == NULL) {
		printf("'bg.bmp' not found in '%s'\n", 
				SHARE_DIR);
		printf("Falling back to 'share/bg.bmp' ...");
		bg = SDL_LoadBMP("share/bg.bmp");
		if(bg == NULL) {
			printf(" failure :-(\n");
			exit(EXIT_FAILURE);
		} else printf(" success!\n");
	}

	SDL_BlitSurface(bg, NULL, window, NULL);
	SDL_Flip(window);

	mode = TEXT_MODE;

	/* Cache SDL_MapRGB's output for speed */
	colorcache = new Uint32[730];
	for(int i = 0; i <= 729; i++) {
		tryte t = i - 364;
		int c1 = 4 + t[0].to_int() * 3 + t[1].to_int();
		int c2 = 4 + t[2].to_int() * 3 + t[3].to_int();
		int c3 = 4 + t[4].to_int() * 3 + t[5].to_int();

		colorcache[i] = SDL_MapRGB(window->format, 28*c1, 28*c2, 28*c3);
	}

	dm = new text_manager();
	wipe();

	m.memrefi(TV_DDD, TV_DDB).set_hook(new graphics_word_change_hook(*this));
}

display::~display() {
	delete[] colorcache;
	SDL_Quit();

}

void display::wipe() {
	SDL_Rect r2 = {XRES/2 - COLS*4, YRES/2-ROWS*5, COLS*8, ROWS*10};
	SDL_FillRect(window, &r2, SDL_MapRGB(window->format, 0, 0, 0));
	SDL_UpdateRect(window, XRES/2 - COLS*4, YRES/2-ROWS*5,
				COLS*8, ROWS*10);
}


/* Screen printing function, called every time LST of DDD:DDB is
 * set to 1. Does mode switching if appropriate, otherwise hands
 * the torch over to the display manager.
 */

void display::printscreen() {
	tryte& displaytryte = m.memrefi(TV_DDD, TV_DDB);
	int auxflag = displaytryte[3].to_int();
	int modeflag = displaytryte[4].to_int();
	int refresh = displaytryte[5].to_int();

	// Yes, there's no mutex in this, but it's no big cause
	// of concern. One screendraw more or less isn't much of
	// an issue. Or, in other words: 
	//   "What could possibly go wrong?"
	if(refresh == 1) { displaytryte[5] = 0; }
	else return;

	SDL_Rect r = { 10, 10, 10, 10 };
	if(m.get_state()->is_running()) SDL_FillRect(window, &r, SDL_MapRGB(window->format, 0, 255, 0));
	else SDL_FillRect(window, &r, SDL_MapRGB(window->format, 128, 128, 0));

	r.x = 30;
	if(mode == TEXT_MODE) SDL_FillRect(window, &r, SDL_MapRGB(window->format, 0, 0, 0));
	else if(mode == VECTOR_MODE) SDL_FillRect(window, &r, SDL_MapRGB(window->format, 128, 128, 0));
	else if(mode == RASTER_MODE) SDL_FillRect(window, &r, SDL_MapRGB(window->format, 0, 255, 0));

	r.x = 50;
	if(auxflag == -1) SDL_FillRect(window, &r, SDL_MapRGB(window->format, 0, 0, 0));
	else if(auxflag == 0) SDL_FillRect(window, &r, SDL_MapRGB(window->format, 128, 128, 0));
	else if(auxflag == 1) SDL_FillRect(window, &r, SDL_MapRGB(window->format, 0, 255, 0));

	SDL_UpdateRect(window, 5, 5, XRES-10, 20);

	/* Assign a new display manager if modeflag does not match
	 * current mode
	 */
	switch(modeflag) {
		case 0: if(mode != TEXT_MODE) {
				printf("Setting text mode\n");
				wipe();
				SDL_UpdateRect(window, 30, 30, XRES-60, YRES-60);
				delete dm;
				dm = new text_manager();
				mode = TEXT_MODE; 
			}
			break;
		case 1: if(mode != VECTOR_MODE) {
				printf("Setting vector mode\n");
				wipe();
				SDL_UpdateRect(window, 30, 30, XRES-60, YRES-60);
				delete dm;
				dm = new vector_manager();
				mode = VECTOR_MODE;
			}
			break;
		case -1: if(mode != RASTER_MODE) {
				printf("Setting raster mode\n");
				wipe();
				SDL_UpdateRect(window, 30, 30, XRES-60, YRES-60);
			 	delete dm;
				dm = new raster_manager();
			 	mode = RASTER_MODE;
			 } 
			 break;
	}

	/* Paint the contents of the screen
	 * */
	dm->paint(this);
}

/* Put a single pixel on the screen.
 *
 * Ignores off-screen coordinates.
 */
void display::putpixel(int x, int y, Uint32 color) {
	if(x < 0 || x > XRES || y < 0 || y > YRES) return;

	Uint8 *pixel = (Uint8*)window->pixels + y * window->pitch + x *
		window->format->BytesPerPixel;
	switch(window->format->BytesPerPixel) {
		case 1: *pixel = color; break;
		case 2: *(Uint16 *) pixel = color; break;
		case 3: 
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				pixel[0] = (color >> 16) & 0xff;
				pixel[1] = (color >> 8) & 0xff;
				pixel[2] = color & 0xff;
			} else {
				pixel[2] = (color >> 16) & 0xff;
				pixel[1] = (color >> 8) & 0xff;
				pixel[0] = color & 0xff;
			}
			break;
		case 4: *(Uint32 *) pixel = color; break;
	}

}

/* Bresenham's line algorithm */
void display::draw_line(int x0, int y0, int x1, int y1, Uint32 color) {
	bool s = abs(y1 - y0) > abs(x1 - x0); int tmp; int x;
	if(s) { 
		tmp = x0; x0 = y0; y0 = tmp;
		tmp = x1; x1 = y1; y1 = tmp;
	}
	if(x0 > x1) {
		tmp = x1; x1 = x0; x0 = tmp;
		tmp = y1; y1 = y0; y0 = tmp;
	}

	int dx = x1 - x0;
	int dy = abs(y1 - y0);
	int error = - (dx + 1) / 2;
	int step;
	int y = y0;
	if(y0 < y1) step = 1; else step = -1;
	for(x = x0; x < x1; x++) {
		if(s) putpixel(y, x, color);
	       		else putpixel(x, y, color);
		error += dy;
		if(error >= 0) {
			y += step;
			error = error - dx;
		}
	}
}

/* Text mode paint function
 *
 * Iterate through the cells of the screen, update where
 * necessary */
void text_manager::paint(display* d) {
	for(int x = 0; x < COLS; x++) {
		for(int y = 0; y < ROWS; y++) {
			putcxy(d, d->m.memref(-264262 + x + COLS*y).to_int(), x, y);
		}
	}
}

/* Text mode character display function
 *
 * Does caching so not to unnecessarily re-draw
 * the same character over and over on the screen */

void text_manager::putcxy(display* d, int character, int x, int y) {
	int c = (y * COLS + x) % (ROWS*COLS);
	if(buffer[c] == character) return;
	buffer[c] = character;
	if(character < 0 || character > 100) return;
	
	Sint16 SX = 8 * (character % 10);
	Sint16 SY = (character / 10) * 10;
	SDL_Rect src = { SX, SY, 8, 10 };

	Sint16 DX = XRES / 2 - COLS * 4 + 8 * x;
	Sint16 DY = YRES / 2 - ROWS * 5 + 10 * y;
   	SDL_Rect dst = { DX, DY, 8, 10 };
   	SDL_BlitSurface(d->symbols, &src, d->window, &dst);

	/* Updating just a small segment of the screen is
	 * way, WAY much faster than flipping the entire screen.
	 */
	SDL_UpdateRect(d->window, dst.x, dst.y, dst.w, dst.h);
}


/* Vector mode drawing function */

void vector_manager::paint(display* d) {
	SDL_Rect r2 = {XRES/2 - COLS*4, YRES/2-ROWS*5, COLS*8, ROWS*10};
	SDL_FillRect(d->window, &r2, SDL_MapRGB(d->window->format, 0, 0, 0));

	if(SDL_MUSTLOCK(d->window)) SDL_LockSurface(d->window);
	int xold = 0, yold = 0;
	for(int idx = -364; idx <= 364; idx+=3) {
		tryte& d1 = d->m.memrefi(TV_DDB, idx+0);
		tryte& d2 = d->m.memrefi(TV_DDB, idx+1);
		tryte& d3 = d->m.memrefi(TV_DDB, idx+2);

		int x = int(4*COLS/121.0*((d2 ^ TV_ADD).to_int()));
		int y = int(5*ROWS/121.0*((d3 ^ TV_ADD).to_int()));

		if(d1.to_int() && d1.to_int() != -364) d->draw_line(XRES/2+xold, YRES/2+yold, XRES/2+x, YRES/2+y, d->getcolor729(d1));
		xold = x; yold = y;
	}
	if(SDL_MUSTLOCK(d->window)) SDL_UnlockSurface(d->window);
	SDL_Flip(d->window);
}

/* Raster mode drawing function (both color modes) */

void raster_manager::paint(display* d) {
	tryte& displaytryte = d->m.memrefi(TV_DDD, TV_DDB);
	int modeaux = displaytryte[3].to_int();

	int x = 0; int y = 0;
	if(SDL_MUSTLOCK(d->window)) SDL_LockSurface(d->window);
	int offset = PODWORD_TO_INT(TV_DDB, TV_DDD); 

	if(modeaux == 0) { /* 729 color mode */
		for(x = 0; x < RASTER_XRES; x++) {
			for(y = 0; y < RASTER_YRES; y++) {
				d->putpixel(x-RASTER_XRES/2+XRES/2, 
					 y-RASTER_YRES/2+YRES/2, 
					 d->getcolor729(d->m.memref(offset + x + 
							 RASTER_XRES*y)));
			}
		}
	} else if(modeaux == 1) {
		for(x = 0; x < RASTER_XRES; x++) {
			for(y = 0; y < RASTER_YRES; y++) {
				tryte color = 364 * d->m.memref(offset + x / 6 + RASTER_XRES/6 * y)[x % 6].to_int();

			
				d->putpixel(x-RASTER_XRES/2+XRES/2, 
				 y-RASTER_YRES/2+YRES/2, 
				 d->getcolor729(color));
			}
		}
					
	}

	if(SDL_MUSTLOCK(d->window)) SDL_UnlockSurface(d->window);
	SDL_Flip(d->window);
}


