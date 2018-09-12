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

#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include "machine.h"
#include "disk.h"
#include "display.h"
#include "keyboard.h"
#include "agdp.h"
#include "SDL_thread.h"
#include "values.h"

/* Poll events every POLL_INTERAL instructions. 
 *
 * 5000 is a good compromise between a short interval in which
 * most of the time is spent polling I/O, and a long interval,
 * in which there is a pronounced delay between keypress and response.
 */
#define POLL_INTERVAL 5000

/* Uncomment NOSLEEP this to disable calls to usleep. Enabling usleep
 * may improve program flow, but then again, that may just be
 * cargo cult programming.
 */
//#define NOSLEEP

/* Uncomment FLAT to disable threading. Threading gives a pronounced speed
 * boost on multi-core systems, but has no tangible effect on single core
 * systems, so if you experience weird behavior, you may wish to consider
 * running in flat mode. */
#define FLAT

int do_exit = 0;

void usage(char* app) {
	printf("Usage: %s [-h] [-f] [-t] [-c] [-e] [-F disk] [image]\n"
		"\t-f Fullscreen\n"
		"\t-t Trace\n"
		"\t-F Specify floppy disk name\n"
		"\t-c Grab cursor\n"
		"\t-e Run testcase\n"
		"\t-Z Don't try to load the floppy, assume it's zero\n"
		"\t-h Show this message\n\n", app);
}

int display_thread(void* dispp) {
	display* disp = (display*) dispp;

	while(!do_exit) {
		disp->printscreen();
#ifndef NOSLEEP
		usleep(10); // Give the CPU some breathing room
#endif
	}

	return 0;
}

inline void cycle(machine &m, display &disp, disk &floppy, kboard &keyboard, 
		agdp &agd) {
	static int ioclock = 0;
	if(ioclock++ > POLL_INTERVAL) {
	       	keyboard.poll();
		ioclock = 0;
	}

	if(m.get_state()->is_running()) {
		while(keyboard.has_input())
			m.queue_interrupt(new keyboard_interrupt(keyboard.get_input()));
		if(keyboard.has_break())
			m.queue_interrupt(new keybreak_interrupt());
		if(keyboard.has_click())
			m.queue_interrupt(new mousepress_interrupt(1));
		if(keyboard.has_release())
			m.queue_interrupt(new mousepress_interrupt(-1));
		if(keyboard.has_mouse_motion()) {
			int mouse_rel_x = keyboard.get_mouse_rel_x();
			int mouse_rel_y = keyboard.get_mouse_rel_y();

			while(mouse_rel_x != 0 || mouse_rel_y != 0)
			{
				m.queue_interrupt(new mousemotion_interrupt(
						mouse_rel_x,
						mouse_rel_y));
				if(mouse_rel_x > 13) mouse_rel_x -= 13;
				else if(mouse_rel_x < -13) mouse_rel_x += 13;
				else mouse_rel_x = 0;

				if(mouse_rel_y > 13) mouse_rel_y -= 13;
				else if(mouse_rel_y < -13) mouse_rel_y += 13;
				else mouse_rel_y = 0;
			}
		}
				

		/* XXX: This is dubious behavior, theoretically, if a ISR fails
		 * to return properly, or SEI is left on, the memory will fill up
		 * with clock interrupts :-( */

		if(m.CL.to_int() == 0)
			m.queue_interrupt(new clock_interrupt());

//		floppy->heartbeat();

#ifdef FLAT
//		disp->printscreen();
#endif

//		agd->heartbeat(*m);

		m.instruction();
	}
}


void testcase(machine& m) {
	printf("Running testcase\n");
	tryte t;
#define DOTEST(X,E,T) X; printf("'%s' => %d (expect %d)\n", #X, T, E);
	DOTEST(t = 10, 10, t.to_int());
	DOTEST(t = t+10, 20, t.to_int());
	DOTEST(t = -365, 364, t.to_int());
	DOTEST(t = 365, -364, t.to_int());
	DOTEST(t = 5; t = t * 2, 10, t.to_int());
	DOTEST(t = 5; t = t / 2, 2, t.to_int());
	DOTEST(t = 5; t = t % 3, 2, t.to_int());
	DOTEST(m.memref(25) = 10; m.memref(0, 25) += 5, 15,
			m.memref(0, 25).to_int());
	DOTEST(m.memref("DDD", "DDD") = 10; m.memref(-729*729/2) += 5, 15,
			m.memref("DDD", "DDD").to_int());
	DOTEST(t = 1000; t.to_int();, 271, t.to_int());
	DOTEST(t = -1000; t.to_int();, -271, t.to_int());
#undef  DOTEST

}

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO); // Initialize video (and keyboard)

	int fullscreen = 0;
	int optret;
	int trace = 0;
	int grabcursor = 0;
	int zerodisk = 0;
	int dotest = 0;

	const char* floppyname = NULL;

	while((optret = getopt(argc, argv, "hftcF:Ze")) != -1) {
		switch(optret) {
			case 'h': usage(argv[0]); exit(EXIT_SUCCESS); 
			case 'f': fullscreen = 1; break;
			case 't': trace = 1; break;
			case 'c': grabcursor = 1; break;
			case 'e': dotest = 1; break;
			case 'F': floppyname = strdup(optarg); break;
			case 'Z': zerodisk = 1; break;
			case '?': usage(argv[1]); exit(EXIT_FAILURE);
			case ':': usage(argv[1]); exit(EXIT_FAILURE);
		}
	}

	trit::set_strategy(new trit::trit_strategy_tuf_table());

	if(floppyname == NULL) {
		zerodisk = 1;
		floppyname = "disk.ternobj";
	}

	machine m;
	kboard keyboard(m);
	display disp(m);;
	disk floppy(m);

	agdp agd;
	m.memrefi(TV_DDD, TV_DD0).set_hook(new agdp_change_hook(agd, m));

	if(!zerodisk || !floppyname) {
		try {
			printf("Opening disk '%s'\n", floppyname);
			floppy.load(floppyname);
		} catch(std::runtime_error* e) {
			printf("Failed to load disk '%s' (%s)\n", floppyname, e->what());
			delete e;
		}
	}
	
	m.trace = trace;

	if(grabcursor) keyboard.grab();

	if(fullscreen) disp.fullscreen();

	if(argc > optind) {
		printf("LOADING %s\n", argv[optind]);
		try {
			m.load(argv[optind]);
		} catch(std::runtime_error* e) {
			printf("Unable to load %s (%s)\n", argv[optind], e->what());
			return EXIT_FAILURE;
		}
		printf("...DONE\n");
	} else {
		printf("Tunguska loaded null memory, basically\n"
		       "rendering it a drooling vegetable :-(\n"
		       "\n"
		       "It's a lot more fun with something in the\n"
		       "memory to execute...");
	}

	srandom(time(NULL));

	/* Run repainting in a separate thread, huge speed boost, especially for
	 * multi-core systems */
#ifndef FLAT
	SDL_Thread* dthread = 
		SDL_CreateThread(display_thread, disp);
#endif
	/* Initialization is done */

	if(dotest) {
		testcase(m);
	}

	/* Warm up for 500k instructions 
	 * (wait for static lookup tables to be initialized
	 * and whatnot)*/
	for(int i = 0; i < 500000; i++) {
		cycle(m, disp, floppy, keyboard, agd);
	}

	/* Do 10 benchmark tests during 50k insturctions each */
	double kspeed_total = 0;
	double bench_max = -1;
	double bench_min = -1;
	for(int bench = 0; bench < 10; bench++) {
		long timestart = SDL_GetTicks();
		for(int i = 0; i < 50000; i++) 
			cycle(m, disp, floppy, keyboard, agd);
		long dt = SDL_GetTicks() - timestart;

		double kspeed = 50000.0 / dt;
		kspeed_total += kspeed;
		if(bench_max == -1) bench_max = kspeed;
		if(bench_min == -1) bench_min = kspeed;
		else {
			if(kspeed > bench_max) bench_max = kspeed;
			if(kspeed < bench_min) bench_min = kspeed;
		}
	}
	printf("Initial benchmark: %.3f thousand operations per second\n"
	       "\tSpread: %.3f\n\n", kspeed_total / 10, bench_max - bench_min);

	long operation = 0;

	/* Main loop */
	while(!do_exit) {
		operation++;
		cycle(m, disp, floppy, keyboard, agd); 

		/* Sleep every 5000 instructions */
#ifndef NOSLEEP
		if(operation % 5000 == 0) usleep(10);
#endif
	}

	/* cleanup */
#ifndef FLAT
	SDL_WaitThread(dthread, NULL);
#endif
	SDL_Quit();

	return 0;
}
