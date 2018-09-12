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

#ifndef display_h
#define display_h
#include "machine.h"
#include "SDL.h"

 /* The resolution of the window */
#define XRES 640
#define YRES 480

/* The resolution of the emulated screen. These can not be any arbitrary
 * numbers, first of all RASTER_XRES * RASTER_YRES must be evenly divisible
 * by 729, secondly, they must (or rather, ought to) share aspect ratio with
 * the window; so 3 * RASTER_XRES = 4 * RASTER_YRES. Finally, they must be 
 * dimensioned so that they are big enough to be useful, and small enough not
 * to gobble up half the memory of the machine. All these qualifiers boil down
 * to only one resolution: 324x243 (it's also reasonably close to 320x240, a
 * conventional screen resolution)
 * */
#define RASTER_XRES 324  
#define RASTER_YRES 243

/* The width of the window in text mode */
#define ROWS 27
#define COLS 54

typedef enum { TEXT_MODE, VECTOR_MODE, RASTER_MODE } graphics_mode;

class display;

class display_manager { 
	public:
		display_manager() {};
		virtual ~display_manager() {};

		virtual void paint(display* d) = 0;
};

class raster_manager : public display_manager {
	public:
		raster_manager() {};
		virtual ~raster_manager() {};

		virtual void paint(display* d);
};
class vector_manager : public display_manager {
	public:
		vector_manager() {};
		virtual ~vector_manager() {};

		virtual void paint(display* d);
};
class text_manager : public display_manager {
	public:
		text_manager() {
			buffer = new short[ROWS*COLS];
			memset(buffer, 101, sizeof(short) * ROWS * COLS);
		};
		virtual ~text_manager() {
			delete[] buffer;
		};

		virtual void paint(display* d);
	protected:
		void putcxy(display* d, int character, int x, int y);
		short* buffer;
};


class display {
public:
	display(machine& m);
	~display();
	
	void printscreen();
	void fullscreen() { SDL_WM_ToggleFullScreen(window); }

	graphics_mode get_mode() const { return mode; };
	void set_mode(graphics_mode mode) { this->mode = mode; };

	void putpixel(int x, int y, Uint32 color);
	void draw_line(int x0, int y0, int x1, int y1, Uint32 color);
	void wipe();

	machine& get_machine() const { return m; }

	friend class text_manager;
	friend class vector_manager;
	friend class raster_manager;

private:
	machine& m;
	SDL_Surface* window;
	SDL_Surface* symbols;
	SDL_Surface* bg;

	Uint32 getcolor729(tryte& t) { return colorcache[t.to_int()+364]; }
	Uint32* colorcache;

	display_manager* dm;
	graphics_mode mode;

};

class graphics_word_change_hook : public tryte::change_hook {
	public:
		graphics_word_change_hook(display& d) : disp(d), recursion_lock(false) {}
		virtual ~graphics_word_change_hook() {}

		virtual const tryte& change(tryte& self, const tryte& value) { 
			if(recursion_lock) return value;
			recursion_lock = true;
			disp.printscreen();
			recursion_lock = false;
			return value;
		}
	private:
		display& disp;
		double recursion_lock;

};

#endif
