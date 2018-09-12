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

#ifndef kboard_h
#define kboard_h
#include "SDL.h"
#include "machine.h"
#include <queue>


class kboard {
public:
	kboard(machine& m);
	bool has_input() { return !keyqueue.empty(); }
	char get_input() { char c = keyqueue.front(); keyqueue.pop(); return c;}
	bool has_break() { bool k = key_break; key_break = false; return k; }
	bool has_mouse_motion() { return mouse_rel_x != 0 || mouse_rel_y != 0; }

	bool has_click();
	bool has_release(); 

	int get_mouse_rel_x();
	int get_mouse_rel_y();
		
	void grab() { SDL_WM_GrabInput(SDL_GRAB_ON); }
	void poll();
private:
	bool input;
	bool key_break;
	

	void got_key(SDL_Event* e);
	char c;
	machine& m;

	bool click;
	bool release;

	int mouse_rel_x;
	int mouse_rel_y;

	std::queue<char> keyqueue;

};

#endif
