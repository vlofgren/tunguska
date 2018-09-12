#include <gtk/gtk.h>

#include "tunguska_wrapper.h"
#include "../share_dir.h"

#include <gdk/gdkkeysyms.h>

GtkWidget* 	screen 		= NULL;
GdkPixbuf* 	symbols 	= NULL;
GdkGC* 		black 		= NULL;
GdkGC* 		white 		= NULL;
extern GtkWidget* main_window;
extern GtkToolItem* runbutton;

gboolean 	do_repaint	= TRUE;

machine*   	mac		= NULL;
agdp* 	   	agd		= NULL;
disk*		dis		= NULL;

gboolean machine_cycle(gpointer not_used);
void init_gcs();

/* * * * * * * * * * * * * * * * * * * * * * * * *
 *						 *
 *						 *
 *						 *
 * Image and disk loading/saving 		 *
 *						 *
 *						 *
 *						 *
 * * * * * * * * * * * * * * * * * * * * * * * * */

void add_filters_to_image_dialog(GtkFileChooser* dialog) {
	GtkFileFilter* image_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(GTK_FILE_FILTER(image_filter), "Tunguska images (*.ternobj)");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(image_filter), "*.ternobj");
	gtk_file_chooser_add_filter(dialog, image_filter);

	image_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(GTK_FILE_FILTER(image_filter), "All files");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(image_filter), "*");
	gtk_file_chooser_add_filter(dialog, image_filter);
}



void load_image() {
	GtkWidget* dialog = gtk_file_chooser_dialog_new("Load Image",
								GTK_WINDOW(main_window),
								GTK_FILE_CHOOSER_ACTION_OPEN,
								GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
								GTK_STOCK_OPEN,	GTK_RESPONSE_ACCEPT,
								NULL);
	ABOUT_TO_OPEN_FILE_DIALOG;
	add_filters_to_image_dialog(GTK_FILE_CHOOSER(dialog));
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char* image;
		image = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		mac->reset();
		mac->load(image);
		g_free(image);
	}
	gtk_widget_destroy(dialog);
	FILE_DIALOG_NO_LONGER_OPEN;
}

void load_disk() {
	GtkWidget* dialog = gtk_file_chooser_dialog_new("Load Disk",
								GTK_WINDOW(main_window),
								GTK_FILE_CHOOSER_ACTION_OPEN,
								GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
								GTK_STOCK_OPEN,	GTK_RESPONSE_ACCEPT,
								NULL);
	ABOUT_TO_OPEN_FILE_DIALOG;
	add_filters_to_image_dialog(GTK_FILE_CHOOSER(dialog));
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char* image;
		image = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		dis->load(image);
		g_free(image);
	}
	gtk_widget_destroy(dialog);
	FILE_DIALOG_NO_LONGER_OPEN;
}

void save_disk() {
	GtkWidget* dialog = gtk_file_chooser_dialog_new("Save Disk",
								GTK_WINDOW(main_window),
								GTK_FILE_CHOOSER_ACTION_SAVE,
								GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
								GTK_STOCK_OPEN,	GTK_RESPONSE_ACCEPT,
								NULL);
	add_filters_to_image_dialog(GTK_FILE_CHOOSER(dialog));
	ABOUT_TO_OPEN_FILE_DIALOG;
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char* image;
		image = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		dis->load(image);
		g_free(image);
	}
	gtk_widget_destroy(dialog);
	FILE_DIALOG_NO_LONGER_OPEN;
}

/* If complete == TRUE, then the entire screen will be redrawn whether
 * it has been changed in memory or not, but if complete == FALSE, it will
 * only redraw the portions that have changed. */
void redraw(gboolean complete) {
	static int cache[COLS][ROWS];
	int x, y;
	int w, h;
	int x_offset, y_offset;

	gdk_window_get_size(screen->window, &w, &h);

	x_offset = (w - COLS*8)*0.5;
	y_offset = (h - ROWS*10)*0.5;

	g_assert(screen);
	g_assert(mac);
	g_assert(symbols);

	for(x = 0; x < COLS; x++) {
		for(y = 0; y < ROWS && (y_offset+y*10) < h; y++) {
			int val = mac->memref(-264262 + x + COLS*y).to_int();
			if(complete || val != cache[x][y]) {
				cache[x][y] = val;
				if(val < 0 || val >= 100) val = 0;
				gdk_draw_pixbuf(screen->window, 
					NULL, symbols, 8*(val%10), (val/10)*10, x_offset + 8*x, y_offset + 10*y, 
					8, 10, GDK_RGB_DITHER_NONE, 0, 0);
				
			}
		}
	}
}

/* Repaint the screen */
gboolean expose_screen(GtkWidget* w, GdkEventExpose* evt, gpointer unused) {
	
	int width,height;
	if(!black) init_gcs();
	g_assert(black);
	gdk_window_get_size(w->window, &width, &height);
	gdk_draw_rectangle(w->window, black, TRUE, 0, 0, width, height);
	redraw(TRUE);

	return TRUE;
}





/* If approperiate, add a key interrupt to the machine */
gboolean tunguska_keypress(GtkWidget* widget,
			GdkEventKey* event,
			gpointer data) {
	char* table ="\a \a\a\a\a\a\a\a\a"
			  "ABCDE" "FGHIJ"
			  "KLMNO" "PQRST"
			  "UVWXY" "Z0123"
			  "45678" "9.,!?"
			  "abcde" "fghij"
			  "klmno" "pqrst"
			  "uvwxy" "z=-*/"
			  "%<>\a\a()" "$+#\a";
	char c = 0;
	int i = 0;
	for(i = 0; i < 100; i++) 
		if(*(event->string) == table[i]) {
			c = i;
			break;
		}
	if(strlen(event->string) != 1 || i == 100) switch(event->keyval) {
		case GDK_Return: c = 2; break;
		case GDK_BackSpace: c = 4; break;
		default: {
			return FALSE;
		}
	}

	mac->queue_interrupt(new keyboard_interrupt(c));

	return TRUE;
}

gboolean keyboard_is_grabbed = FALSE;
gboolean keyboard_handler = 0;

/* The keyboard needs to be grabbed when the machine is running so that
 * it can process keyboard input, but it's not necessary to intercept
 * keys when the machine isn't running since this will interfere with
 * key shortcuts and whatnot */
void set_keyboard_grabbed(gboolean grab) {
	if(keyboard_is_grabbed) {
		if(!grab) {
			g_signal_handler_disconnect(G_OBJECT(main_window), keyboard_handler);
			keyboard_is_grabbed = FALSE;
		}
	} else {
		if(grab) {
			keyboard_handler = g_signal_connect(G_OBJECT(main_window), "key-press-event",
				G_CALLBACK(tunguska_keypress), NULL);
			keyboard_is_grabbed = TRUE;
		}
	}
}


void pause_machine() {
	mac->set_state(new machine::paused_state());
}

void run_machine() {
	g_assert(mac);
	mac->set_state(new machine::running_state());
}


/* Execute one instruction, and the pause */
void step_instruction() {
	g_assert(mac);
	mac->set_state(new machine::step_state(1));
}

/* Set all registers to 0 */
void reset_machine() {
	mac->reset();
}

void run_wrapper(gboolean state) {
	static gboolean current_state = FALSE;
	if(current_state != state) switch(state) {
			case 0: g_idle_remove_by_data(mac); break;
			case 1: g_idle_add(machine_cycle, mac); break;
	}
	current_state = state;
}

void init_gcs() {
	GdkColormap* screen_cm = gtk_widget_get_colormap(screen);
	
	
	black = gdk_gc_new(screen->window);
	white = gdk_gc_new(screen->window);

	GdkColor black_color = { 0, 0, 0, 0 };
	GdkColor white_color = { 0, 65535, 65535, 65535 };
	gdk_gc_set_rgb_fg_color(black, &black_color);
	gdk_gc_set_rgb_bg_color(black, &white_color);
	gdk_gc_set_rgb_fg_color(white, &white_color);
	gdk_gc_set_rgb_bg_color(white, &black_color);
	
}

void init_wrapper() {
	/* Load symbols from file */
	symbols = gdk_pixbuf_new_from_file(SHARE_DIR "/symbols.bmp", NULL);
	if(!symbols) {
                 /* Look in other places */
                 symbols = gdk_pixbuf_new_from_file("symbols.bmp", NULL);
                 if(!symbols) /* Shaos-08.08.2009 */
                 {
		    printf("Couldn't open symbols.bmp :(\n");
		    exit(EXIT_FAILURE);
		 }
	}

	/* Create tunguska components */
	mac = new machine();
	agd = new agdp();
	dis = new disk(mac);
	mac->set_state(new machine::paused_state());



	g_signal_connect(G_OBJECT(screen), "expose_event",
				G_CALLBACK(expose_screen), NULL);


	run_wrapper(TRUE);
}

GtkWidget* create_wrapper() {
	/* Create drawing area for Tunguska to paint in */


	screen = gtk_drawing_area_new();
	gtk_widget_set_size_request(screen, 432, 270);
	gtk_widget_show_all(screen);
	init_wrapper();

	return screen;
}

gboolean scheduled_expose(gpointer unused) {
	redraw(FALSE);
	return FALSE;
}

gboolean machine_cycle(gpointer not_used) {

	if(mac->get_state()->is_running()) {
		if(mac->CL.to_int() == 0) mac->queue_interrupt(new clock_interrupt());
		if(mac->memrefi(TV_DDD,TV_DDB)[5].to_int() == 1) {
			mac->memrefi(TV_DDD,TV_DDB)[5] = 0;
			//expose_screen(screen, NULL, NULL);
			g_timeout_add(10, &scheduled_expose, NULL); // Repaint ever so often
		}
		dis->heartbeat();
		agd->heartbeat(*mac);
		mac->instruction();
	} else {
		if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(runbutton))) {
			set_keyboard_grabbed(FALSE);
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(runbutton), FALSE);
		}
	}
	
	return TRUE;
}


