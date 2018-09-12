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

#include "SDL.h"
#include "keyboard.h"

extern int do_exit; 

kboard::kboard(machine& m) : input(false), key_break(false), m(m) { 
	SDL_EnableUNICODE(true);
	mouse_rel_x = mouse_rel_y = 0;
	release = click = false;
}

/* Poll SDL events and do accordingly */
void kboard::poll() {
	SDL_Event e;
	while(SDL_PollEvent(&e)) {
		switch(e.type) {
			case SDL_QUIT:
				do_exit = 1; break;
			case SDL_KEYDOWN:
				got_key(&e); break;
			case SDL_MOUSEMOTION:
				mouse_rel_x += e.motion.xrel;
				mouse_rel_y += e.motion.yrel;
				break;
			case SDL_MOUSEBUTTONDOWN: click = true; break;
			case SDL_MOUSEBUTTONUP: release = true; break;
		}
	}
}


/* Key press handler, basically a large translation mechanism from ASCII to
 * Tunguska's internal character system */

void kboard::got_key(SDL_Event* e) {
	if((e->key.keysym.unicode & 0xff80) != 0) return; // Non-unicode
	
	char ascii = e->key.keysym.unicode & 0x7f;
	
	char c = 0;

	switch(ascii) { 
		case ' ': c = 1; break;
		case 'A': c = 10; break;
		case 'B': c = 11; break;
		case 'C': c = 12; break;
		case 'D': c = 13; break;
		case 'E': c = 14; break;
		case 'F': c = 15; break;
		case 'G': c = 16; break;
		case 'H': c = 17; break;
		case 'I': c = 18; break;
		case 'J': c = 19; break;
		case 'K': c = 20; break;
		case 'L': c = 21; break;
		case 'M': c = 22; break;
		case 'N': c = 23; break;
		case 'O': c = 24; break;
		case 'P': c = 25; break;
		case 'Q': c = 26; break;
		case 'R': c = 27; break;
		case 'S': c = 28; break;
		case 'T': c = 29; break;
		case 'U': c = 30; break;
		case 'V': c = 31; break;
		case 'W': c = 32; break;
		case 'X': c = 33; break;
		case 'Y': c = 34; break;
		case 'Z': c = 35; break;
		case '0': c = 36; break;
		case '1': c = 37; break;
		case '2': c = 38; break;
		case '3': c = 39; break;
		case '4': c = 40; break;
		case '5': c = 41; break;
		case '6': c = 42; break;
		case '7': c = 43; break;
		case '8': c = 44; break;
		case '9': c = 45; break;
		case '.': c = 46; break;
		case ',': c = 47; break;
		case '!': c = 48; break;
		case '?': c = 49; break;
		case 'a': c = 50; break;
		case 'b': c = 51; break;
		case 'c': c = 52; break;
		case 'd': c = 53; break;
		case 'e': c = 54; break;
		case 'f': c = 55; break;
		case 'g': c = 56; break;
		case 'h': c = 57; break;
		case 'i': c = 58;  break;
		case 'j': c = 59;  break;
		case 'k': c = 60;  break;
		case 'l': c = 61;  break;
		case 'm': c = 62;  break;
		case 'n': c = 63;  break;
		case 'o': c = 64;  break;
		case 'p': c = 65;  break;
		case 'q': c = 66;  break;
		case 'r': c = 67;  break;
		case 's': c = 68;  break;
		case 't': c = 69;  break;
		case 'u': c = 70;  break;
		case 'v': c = 71;  break;
		case 'w': c = 72;  break;
		case 'x': c = 73;  break;
		case 'y': c = 74; break;
		case 'z': c = 75; break;
		case '=': c = 86;  break;
		case '-': c = 87;  break;
		case '*': c = 88;  break;
		case '/': c = 89;  break;
		case '%': c = 90;  break;
		case '<': c = 91;  break;
		case '>': c = 92;  break;
		case '(': c = 95; break;
		case ')': c = 96; break;
		case '$': c = 97;  break;
		case '+': c = 98;  break;
		case '#': c = 99; break;
	}

	/* Handle special keys */
	switch(e->key.keysym.sym) {
		case SDLK_RETURN: c = 2; break;
		case SDLK_TAB: c = 3; break;
		case SDLK_BACKSPACE: c = 4; break;

		case SDLK_PAUSE: if(m.get_state()->is_running()) m.set_state(new machine::paused_state()); 
				else m.set_state(new machine::running_state()); return;
		case SDLK_F9: m.trace = !m.trace; return;
		case SDLK_F10: m.debug(); return;
		case SDLK_F11: m.instruction(); m.debug(); return;
		case SDLK_F12: key_break = true; return;
		case SDLK_ESCAPE: do_exit = 1; return;
		default: break;
	}

	if(c != 0) keyqueue.push(c);
}

/*  ************************************
 *
 *
 * Mouse functionality 
 *
 *
 * */

int kboard::get_mouse_rel_x() {
	int tmp = mouse_rel_x;
	mouse_rel_x = 0;
	return tmp;
}
int kboard::get_mouse_rel_y() {
	int tmp = mouse_rel_y;
	mouse_rel_y = 0;
	return tmp;

}

bool kboard::has_click() {
	if(click) { click = false; return true; }
	return false;
}

bool kboard::has_release() {
	if(release) { release = false; return true; }
	return false;
}

