#include <gtk/gtk.h>
#include "machine.h"
#include "agdp.h"
#include "disk.h"
#include "values.h"
#ifndef TUNGUSKA_WRAPPER
#define TUNGUSKA_WRAPPER

/* Some systems (e.g. x86_64) seem to suffer from a bug
 * (in GTK+?) where running the machine_cycle idle task while
 * opening a file chooser dialog causes the file chooser dialog
 * to behave erratically. */
#define HAS_DIALOG_BUG

#ifdef HAS_DIALOG_BUG
# define ABOUT_TO_OPEN_FILE_DIALOG	run_wrapper(FALSE)
# define FILE_DIALOG_NO_LONGER_OPEN	run_wrapper(TRUE)
#else
# define ABOUT_TO_OPEN_FILE_DIALOG
# define FILE_DIALOG_NO_LONGER_OPEN
#endif


#define ROWS 27
#define COLS 54

void run_wrapper(gboolean state);

void load_image();
void load_disk();
void save_disk();
void reset_machine();
void step_instruction();
void pause_machine();
void run_machine();
void set_keyboard_grabbed(gboolean grab);
void init_wrapper();

GtkWidget* create_wrapper();

#endif
